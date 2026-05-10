/******************************************************************************
 *
 * Module: UART (MCAL)
 *
 * File Name: uart.c
 *
 * Description: ESP32 UART2 driver via DIRECT register access.
 *              No driver/uart.h, no esp_timer.h, no Arduino libraries.
 *              All addresses and bit positions are taken from the ESP32 TRM
 *              §13 UART Controller and verified against soc/uart_reg.h.
 *
 *              Pin assignment is set by the caller of UART_init():
 *                Default project use: TX = GPIO5, RX = GPIO0, 115200 8N1
 *
 *******************************************************************************/

#include "uart.h"
#include "platform.h"

#include "soc/soc.h"            /* DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE, etc. */
#include "soc/dport_reg.h"      /* DPORT_PERIP_CLK_EN_REG, DPORT_PERIP_RST_EN_REG */
#include "soc/gpio_sig_map.h"   /* U2TXD_OUT_IDX, U2RXD_IN_IDX                    */
#include "intr.h"               /* Intr_install, INTR_SOURCE_UART2                 */

#include "freertos/FreeRTOS.h"

#include <stdio.h>

/*******************************************************************************
 *                           Register Helper Macro                             *
 *******************************************************************************/

#define REG32(addr)     (*((volatile uint32 *)(addr)))

/*******************************************************************************
 *        UART2 Register Map  (ESP32 TRM §13, confirmed from soc/uart_reg.h)  *
 *                                                                             *
 *  UART base addresses (TRM §4.1 System Address Map):                         *
 *    UART0 = DR_REG_UART_BASE + 0*0x10000          = 0x3FF40000              *
 *    UART1 = DR_REG_UART_BASE + 1*0x10000          = 0x3FF50000              *
 *    UART2 = DR_REG_UART_BASE + 2*0x10000 + 0xE000 = 0x3FF6E000              *
 *******************************************************************************/

#define UART2_BASE      (DR_REG_UART_BASE + 2u*0x10000u + 0xE000u)

#define UART_FIFO       (UART2_BASE + 0x00u)  /* TX: push byte;  RX: pop byte      */
#define UART_INT_RAW    (UART2_BASE + 0x04u)  /* RO: raw interrupt flags            */
#define UART_INT_ST     (UART2_BASE + 0x08u)  /* RO: masked interrupt status        */
#define UART_INT_ENA    (UART2_BASE + 0x0Cu)  /* R/W: interrupt enable mask         */
#define UART_INT_CLR    (UART2_BASE + 0x10u)  /* WO: write 1 to clear flag (W1C)    */
#define UART_CLKDIV     (UART2_BASE + 0x14u)  /* R/W: baud rate divisor             */
#define UART_STATUS     (UART2_BASE + 0x1Cu)  /* RO: TX/RX FIFO byte counts         */
#define UART_CONF0      (UART2_BASE + 0x20u)  /* R/W: frame format + FIFO control   */
#define UART_CONF1      (UART2_BASE + 0x24u)  /* R/W: FIFO threshold + RX timeout   */

/*------------------------------------------------------------------------------
 * UART_INT_* shared bit positions  (TRM §13.3 Interrupt Registers)
 *----------------------------------------------------------------------------*/
#define UART_INT_RXFIFO_FULL    (1u << 0)  /* RX FIFO depth >= threshold       */
#define UART_INT_RXFIFO_OVF     (1u << 4)  /* RX FIFO overflowed               */
#define UART_INT_RXFIFO_TOUT    (1u << 8)  /* RX idle timeout elapsed          */

/*------------------------------------------------------------------------------
 * UART_STATUS register  (TRM Table 13-4)
 *  bits [ 7: 0]  RXFIFO_CNT — bytes in RX FIFO
 *  bits [23:16]  TXFIFO_CNT — bytes in TX FIFO
 *----------------------------------------------------------------------------*/
#define UART_RXFIFO_CNT_S       0u
#define UART_RXFIFO_CNT_MASK    0xFFu
#define UART_TXFIFO_CNT_S       16u
#define UART_TXFIFO_CNT_MASK    0xFFu
#define UART_FIFO_DEPTH         128u

/*------------------------------------------------------------------------------
 * UART_CONF0 register  (TRM Table 13-5, confirmed from soc/uart_reg.h)
 *  bits [3:2]   BIT_NUM          data bits: 0=5b 1=6b 2=7b 3=8b
 *  bits [5:4]   STOP_BIT_NUM     stop bits: 1=1b  2=1.5b  3=2b
 *  bit  17      RXFIFO_RST       write 1 then 0 to flush RX FIFO
 *  bit  18      TXFIFO_RST       write 1 then 0 to flush TX FIFO
 *  bit  27      TICK_REF_ALWAYS_ON  1=APB 80 MHz clock, 0=REF_TICK 1 MHz
 *----------------------------------------------------------------------------*/
#define UART_BIT_NUM_S          2u
#define UART_STOP_BIT_NUM_S     4u
#define UART_RXFIFO_RST         (1u << 17)
#define UART_TXFIFO_RST         (1u << 18)
#define UART_TICK_REF_ALWAYS_ON (1u << 27)

/*------------------------------------------------------------------------------
 * UART_CONF1 register  (TRM Table 13-6, confirmed from soc/uart_reg.h)
 *  bits [ 6: 0]  RXFIFO_FULL_THRHD  fire RXFIFO_FULL_INT when count >= this
 *  bits [30:24]  RX_TOUT_THRHD      idle timeout in UART symbol periods
 *  bit  31       RX_TOUT_EN         enable RX idle timeout interrupt
 *----------------------------------------------------------------------------*/
#define UART_RXFIFO_FULL_THRHD_S    0u
#define UART_RX_TOUT_THRHD_S        24u
#define UART_RX_TOUT_EN             (1u << 31)

/*******************************************************************************
 *        DPORT — Peripheral Clock / Reset  (ESP32 TRM §3, soc/dport_reg.h)  *
 *******************************************************************************/
#define UART2_CLK_EN        (1u << 23)   /* DPORT_PERIP_CLK_EN_REG bit 23 */
#define UART2_RST           (1u << 23)   /* DPORT_PERIP_RST_EN_REG  bit 23 */
#define UART_MEM_CLK_EN     (1u << 24)   /* shared UART FIFO memory clock  */

/*******************************************************************************
 *        GPIO Matrix  (ESP32 TRM §4.3, soc/gpio_reg.h)                      *
 *                                                                             *
 *  Output routing: GPIO_FUNCgpio_OUT_SEL_CFG                                 *
 *    address = DR_REG_GPIO_BASE + 0x530 + gpio * 4                           *
 *  Input routing: GPIO_FUNCsig_IN_SEL_CFG                                    *
 *    address = DR_REG_GPIO_BASE + 0x130 + signal * 4                         *
 *                                                                             *
 *  Output-enable set/clear (W1S / W1C):                                      *
 *    GPIO 0-31 : GPIO_ENABLE_W1TS (0x024), GPIO_ENABLE_W1TC (0x028)         *
 *    GPIO 32+  : GPIO_ENABLE1_W1TS (0x030), GPIO_ENABLE1_W1TC (0x034)       *
 *******************************************************************************/
#define GPIO_FUNC_OUT_CFG(gpio)  (DR_REG_GPIO_BASE + 0x530u + (uint32)(gpio) * 4u)
#define GPIO_FUNC_IN_CFG(sig)    (DR_REG_GPIO_BASE + 0x130u + (uint32)(sig)  * 4u)
#define GPIO_ENABLE_W1TS         (DR_REG_GPIO_BASE + 0x024u)
#define GPIO_ENABLE_W1TC         (DR_REG_GPIO_BASE + 0x028u)
#define GPIO_ENABLE1_W1TS        (DR_REG_GPIO_BASE + 0x030u)
#define GPIO_ENABLE1_W1TC        (DR_REG_GPIO_BASE + 0x034u)
#define GPIO_SIG_IN_SEL          (1u << 6)   /* route through GPIO matrix */

/*******************************************************************************
 *        IO_MUX — Per-pad Config  (ESP32 TRM §4.4 Table 4-3)                *
 *******************************************************************************/
#define IOMUX_MCU_SEL_SHIFT     12u
#define IOMUX_MCU_SEL_MASK      (0x7u << IOMUX_MCU_SEL_SHIFT)
#define IOMUX_MCU_SEL_GPIO      (0x2u << IOMUX_MCU_SEL_SHIFT)  /* GPIO matrix routing */
#define IOMUX_FUN_IE            (1u << 9)   /* input buffer enable (RX pads)   */
#define IOMUX_FUN_WPU           (1u << 8)   /* pull-up enable                  */
#define IOMUX_FUN_WPD           (1u << 7)   /* pull-down enable                */

/* IO_MUX pad offsets relative to DR_REG_IO_MUX_BASE (TRM Table 4-3) */
static const struct { uint8 gpio; uint16 offset; } s_iomux_map[] =
{
    {  0u, 0x44u }, {  1u, 0x88u }, {  2u, 0x40u }, {  3u, 0x84u },
    {  4u, 0x48u }, {  5u, 0x6Cu }, {  6u, 0x60u }, {  7u, 0x64u },
    {  8u, 0x68u }, {  9u, 0x54u }, { 10u, 0x58u }, { 11u, 0x5Cu },
    { 12u, 0x34u }, { 13u, 0x38u }, { 14u, 0x30u }, { 15u, 0x3Cu },
    { 16u, 0x4Cu }, { 17u, 0x50u }, { 18u, 0x70u }, { 19u, 0x74u },
    { 21u, 0x7Cu }, { 22u, 0x80u }, { 23u, 0x8Cu },
    { 25u, 0x24u }, { 26u, 0x28u }, { 27u, 0x2Cu },
    { 32u, 0x1Cu }, { 33u, 0x20u }, { 34u, 0x14u }, { 35u, 0x18u },
    { 36u, 0x04u }, { 39u, 0x10u },
};

/*******************************************************************************
 *                            RX Ring Buffer                                   *
 *******************************************************************************/

#define RX_BUF_SIZE     256u

static uint8           s_rx_buf[RX_BUF_SIZE];
static volatile uint16 s_rx_head = 0u;   /* ISR writes here (producer)  */
static volatile uint16 s_rx_tail = 0u;   /* task reads here (consumer)  */

static void prv_rxbuf_push(uint8 c)
{
    uint16 next = (uint16)((s_rx_head + 1u) % RX_BUF_SIZE);
    if (next != s_rx_tail)          /* drop byte silently when full */
    {
        s_rx_buf[s_rx_head] = c;
        s_rx_head = next;
    }
}

static boolean prv_rxbuf_pop(uint8 *c)
{
    if (s_rx_head == s_rx_tail) return FALSE;
    *c = s_rx_buf[s_rx_tail];
    s_rx_tail = (uint16)((s_rx_tail + 1u) % RX_BUF_SIZE);
    return TRUE;
}

/*******************************************************************************
 *                      UART2 Receive ISR (IRAM)                              *
 *******************************************************************************/

static ISR_ATTR void uart2_isr(void *arg)
{
    (void)arg;

    /* Read masked status: INT_ST = INT_RAW & INT_ENA */
    uint32 st = REG32(UART_INT_ST);

    if (st & (UART_INT_RXFIFO_FULL | UART_INT_RXFIFO_TOUT | UART_INT_RXFIFO_OVF))
    {
        uint32 cnt = (REG32(UART_STATUS) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT_MASK;
        while (cnt > 0u)
        {
            prv_rxbuf_push((uint8)(REG32(UART_FIFO) & 0xFFu));
            cnt--;
        }
    }

    /* Acknowledge all pending flags (W1C) */
    REG32(UART_INT_CLR) = st;
}

/*******************************************************************************
 *               Private Helpers: IO_MUX + GPIO Matrix                        *
 *******************************************************************************/

/* Return IO_MUX register address for a GPIO, or 0 if not in the map */
static uint32 prv_iomux_addr(uint8 gpio)
{
    uint32 i;
    for (i = 0u; i < sizeof(s_iomux_map) / sizeof(s_iomux_map[0]); i++)
    {
        if (s_iomux_map[i].gpio == gpio)
            return (uint32)DR_REG_IO_MUX_BASE + s_iomux_map[i].offset;
    }
    return 0u;
}

/*
 * Configure IO_MUX and GPIO matrix for UART2 TX (output) and RX (input).
 * Supports GPIO 0-39 (output enable uses the correct bank register).
 * Returns FALSE if either GPIO is not in the IO_MUX map.
 */
static boolean prv_gpio_init(uint8 tx_gpio, uint8 rx_gpio)
{
    uint32 tx_iomux = prv_iomux_addr(tx_gpio);
    uint32 rx_iomux = prv_iomux_addr(rx_gpio);
    uint32 v;

    if (tx_iomux == 0u || rx_iomux == 0u) return FALSE;

    /*--------------------------------------------------------------------------
     * TX pin: output, GPIO matrix function (MCU_SEL = 2 → write 1s at [14:12])
     *   1. Set MCU_SEL = 2 in IO_MUX
     *   2. Disable input buffer (FUN_IE = 0)
     *   3. Clear pull resistors
     *   4. Route U2TXD_OUT signal to this GPIO via GPIO matrix output table
     *   5. Enable output driver (W1S to GPIO_ENABLE or GPIO_ENABLE1)
     *------------------------------------------------------------------------*/
    v  = REG32(tx_iomux);
    v &= ~IOMUX_MCU_SEL_MASK;
    v |=  IOMUX_MCU_SEL_GPIO;          /* write 1s: MCU_SEL = 2          */
    v &= ~IOMUX_FUN_IE;                 /* disable input buffer on TX pad */
    v &= ~(IOMUX_FUN_WPU | IOMUX_FUN_WPD);
    REG32(tx_iomux) = v;

    REG32(GPIO_FUNC_OUT_CFG(tx_gpio)) = U2TXD_OUT_IDX;

    /* Write 1 to enable output driver for this GPIO */
    if (tx_gpio < 32u)
        REG32(GPIO_ENABLE_W1TS)  = (1u << tx_gpio);
    else
        REG32(GPIO_ENABLE1_W1TS) = (1u << (tx_gpio - 32u));

    /*--------------------------------------------------------------------------
     * RX pin: input, GPIO matrix function (MCU_SEL = 2 → write 1s at [14:12])
     *   1. Set MCU_SEL = 2 in IO_MUX
     *   2. Enable input buffer (FUN_IE = 1 → write 1 at bit 9)
     *   3. Clear pull resistors
     *   4. Disable output driver (W1C)
     *   5. Route this GPIO into U2RXD_IN via GPIO matrix input table
     *------------------------------------------------------------------------*/
    v  = REG32(rx_iomux);
    v &= ~IOMUX_MCU_SEL_MASK;
    v |=  IOMUX_MCU_SEL_GPIO;          /* write 1s: MCU_SEL = 2          */
    v |=  IOMUX_FUN_IE;                 /* write 1: enable input buffer   */
    v &= ~(IOMUX_FUN_WPU | IOMUX_FUN_WPD);
    REG32(rx_iomux) = v;

    /* Write 1 to disable output driver for RX pin */
    if (rx_gpio < 32u)
        REG32(GPIO_ENABLE_W1TC)  = (1u << rx_gpio);
    else
        REG32(GPIO_ENABLE1_W1TC) = (1u << (rx_gpio - 32u));

    /* Route GPIO → U2RXD_IN: write GPIO number | SIG_IN_SEL into input table */
    REG32(GPIO_FUNC_IN_CFG(U2RXD_IN_IDX)) = (uint32)rx_gpio | GPIO_SIG_IN_SEL;

    return TRUE;
}

/*******************************************************************************
 *                             Public API                                      *
 *******************************************************************************/

UART_StatusType UART_init(uint32 baud, uint8 tx_gpio, uint8 rx_gpio)
{
    uint32 div_x16, clkdiv, frag;

    /*--------------------------------------------------------------------------
     * Step 1: Enable UART2 peripheral clock and shared UART FIFO memory clock.
     *         Write 1 to bit 23 (UART2_CLK_EN) and bit 24 (UART_MEM_CLK_EN)
     *         in DPORT_PERIP_CLK_EN_REG.
     *------------------------------------------------------------------------*/
    REG32(DPORT_PERIP_CLK_EN_REG) |= UART2_CLK_EN;       /* write 1: bit 23 */
    REG32(DPORT_PERIP_CLK_EN_REG) |= UART_MEM_CLK_EN;    /* write 1: bit 24 */

    /*--------------------------------------------------------------------------
     * Step 2: Pulse UART2 reset.
     *         Write 1 to assert, then write 0 to release — leaves the
     *         peripheral registers in a known-clean state.
     *------------------------------------------------------------------------*/
    REG32(DPORT_PERIP_RST_EN_REG) |=  UART2_RST;   /* write 1: assert reset   */
    REG32(DPORT_PERIP_RST_EN_REG) &= ~UART2_RST;   /* write 0: release reset  */

    /*--------------------------------------------------------------------------
     * Step 3: Configure IO_MUX and GPIO matrix for TX and RX pins.
     *------------------------------------------------------------------------*/
    if (!prv_gpio_init(tx_gpio, rx_gpio)) return UART_FAIL;

    /*--------------------------------------------------------------------------
     * Step 4: Flush TX and RX FIFOs.
     *         Write 1 to TXFIFO_RST (bit 18) and RXFIFO_RST (bit 17) in
     *         UART_CONF0, then write 0 to complete the strobe.
     *------------------------------------------------------------------------*/
    REG32(UART_CONF0) |=  UART_TXFIFO_RST;   /* write 1: assert TX flush  */
    REG32(UART_CONF0) &= ~UART_TXFIFO_RST;   /* write 0: complete strobe  */
    REG32(UART_CONF0) |=  UART_RXFIFO_RST;   /* write 1: assert RX flush  */
    REG32(UART_CONF0) &= ~UART_RXFIFO_RST;   /* write 0: complete strobe  */

    /*--------------------------------------------------------------------------
     * Step 5: Program baud rate divisor.
     *         Formula: baud = APB_CLK / (CLKDIV + FRAG/16)
     *         Compute fixed-point x16 divisor, then split into integer/frac.
     *         Example at 115200 baud / 80 MHz APB:
     *           div_x16 = 80000000 * 16 / 115200 = 11111
     *           CLKDIV  = 11111 >> 4  = 694
     *           FRAG    = 11111 & 0xF = 7
     *           Actual baud: 80000000 / (694 + 7/16) = 115195 (error < 0.005%)
     *------------------------------------------------------------------------*/
    div_x16 = (80000000UL * 16UL) / baud;
    clkdiv  = div_x16 >> 4u;
    frag    = div_x16 & 0xFu;
    REG32(UART_CLKDIV) = (frag << 20u) | clkdiv;

    /*--------------------------------------------------------------------------
     * Step 6: Set frame format — 8 data bits, 1 stop bit, no parity.
     *         Use APB (80 MHz) as reference clock (TICK_REF_ALWAYS_ON = 1).
     *
     *   BIT_NUM      [3:2] = 3  → write 1s at bits 3 and 2
     *   STOP_BIT_NUM [5:4] = 1  → write 1 at bit 4
     *   TICK_REF_ALWAYS_ON = 1  → write 1 at bit 27
     *------------------------------------------------------------------------*/
    REG32(UART_CONF0) = (3u << UART_BIT_NUM_S)        /* 8 data bits           */
                      | (1u << UART_STOP_BIT_NUM_S)   /* 1 stop bit            */
                      | UART_TICK_REF_ALWAYS_ON;       /* APB 80 MHz reference  */

    /*--------------------------------------------------------------------------
     * Step 7: Configure RX FIFO threshold and idle timeout.
     *
     *   RXFIFO_FULL_THRHD [6:0]  = 1   → interrupt after every received byte
     *   RX_TOUT_THRHD    [30:24] = 10  → 10 symbol-period idle timeout
     *   RX_TOUT_EN        [31]   = 1   → write 1 to enable timeout interrupt
     *------------------------------------------------------------------------*/
    REG32(UART_CONF1) = (1u  << UART_RXFIFO_FULL_THRHD_S)
                      | (10u << UART_RX_TOUT_THRHD_S)
                      | UART_RX_TOUT_EN;

    /*--------------------------------------------------------------------------
     * Step 8: Clear all pending interrupt flags, then enable the three
     *         RX interrupt sources.
     *         Write 1 to each bit in INT_CLR to acknowledge (W1C).
     *         Write 1 to each bit in INT_ENA to enable.
     *------------------------------------------------------------------------*/
    REG32(UART_INT_ENA) = 0u;
    REG32(UART_INT_CLR) = 0xFFFFFFFFu;
    REG32(UART_INT_ENA) = UART_INT_RXFIFO_FULL
                        | UART_INT_RXFIFO_TOUT
                        | UART_INT_RXFIFO_OVF;

    /*--------------------------------------------------------------------------
     * Step 9: Route UART2 interrupt source through the DPORT matrix to a
     *         free CPU interrupt line and install the ISR handler.
     *------------------------------------------------------------------------*/
    Intr_install(INTR_SOURCE_UART2, uart2_isr, NULL);

    printf("[UART] UART2 ready: TX=GPIO%u RX=GPIO%u %u baud 8N1\n",
           (unsigned)tx_gpio, (unsigned)rx_gpio, (unsigned)baud);

    return UART_OK;
}

void UART_send(const uint8 *data, uint16 len)
{
    uint16 i;
    for (i = 0u; i < len; i++)
    {
        /* Wait until TX FIFO has at least one empty slot.
         * TXFIFO_CNT is bits [23:16] of UART_STATUS; mask to 8 bits. */
        while (((REG32(UART_STATUS) >> UART_TXFIFO_CNT_S) & UART_TXFIFO_CNT_MASK)
               >= UART_FIFO_DEPTH)
        {
            /* spin — at 115200 baud each byte drains in ~87 µs */
        }
        /* Write 1 byte to TX FIFO — bits [7:0] are used */
        REG32(UART_FIFO) = (uint32)data[i];
    }
}

uint16 UART_recv(uint8 *buf, uint16 maxlen)
{
    uint16 count = 0u;
    while (count < maxlen)
    {
        uint8 c;
        if (!prv_rxbuf_pop(&c)) break;
        buf[count++] = c;
    }
    return count;
}
