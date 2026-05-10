/******************************************************************************
 *
 * Module: PWM (MCAL)
 *
 * File Name: pwm.c
 *
 * Description: Register-level LEDC implementation of the MCAL PWM driver.
 *              Uses direct ESP32 LEDC, GPIO matrix, IO_MUX and DPORT
 *              register access per the ESP32 TRM (Section 14 — LEDC).
 *
 *              All channels run in LEDC_HIGH_SPEED_MODE.
 *              Timer clock source: APB_CLK (80 MHz).
 *              Duty range follows the timer's duty_bits resolution.
 *
 *******************************************************************************/

#include "pwm.h"

#include "soc/soc.h"
#include "soc/ledc_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "soc/dport_reg.h"

/*******************************************************************************
 *                              Local Macros                                   *
 *******************************************************************************/

#define REG32(addr)                 (*((volatile uint32 *)(addr)))

/* APB_CLK is fixed at 80 MHz on this board */
#define LEDC_APB_CLK_HZ             80000000u

/* High-speed timer block: TIMERx_CONF at +0x140 + x*8, TIMERx_VALUE at +0x144 + x*8 */
#define LEDC_HSTIMER_CONF_REG(t)    (DR_REG_LEDC_BASE + 0x0140u + ((t) * 0x08u))

/* High-speed channel block: 5 regs * 4 bytes = 0x14 stride per channel.
 * CONF0 +0x00, HPOINT +0x04, DUTY +0x08, CONF1 +0x0C, DUTY_R +0x10           */
#define LEDC_HSCH_CONF0_REG(c)      (DR_REG_LEDC_BASE + 0x0000u + ((c) * 0x14u))
#define LEDC_HSCH_HPOINT_REG(c)     (DR_REG_LEDC_BASE + 0x0004u + ((c) * 0x14u))
#define LEDC_HSCH_DUTY_REG(c)       (DR_REG_LEDC_BASE + 0x0008u + ((c) * 0x14u))
#define LEDC_HSCH_CONF1_REG(c)      (DR_REG_LEDC_BASE + 0x000Cu + ((c) * 0x14u))

/* Channel CONF0 fields (per HSCH0_CONF0 layout, mirrored in all 8 channels) */
#define LEDC_HSCH_TIMER_SEL_S       0u    /* bits [1:0] : timer selector  */
#define LEDC_HSCH_TIMER_SEL_M       0x03u
#define LEDC_HSCH_SIG_OUT_EN        (1u << 2)   /* output enable (channel) */
#define LEDC_HSCH_IDLE_LV           (1u << 3)   /* idle level when disabled*/

/* Channel CONF1 fields */
#define LEDC_HSCH_DUTY_INC          (1u << 30)  /* default direction = up  */
#define LEDC_HSCH_DUTY_START        (1u << 31)  /* commit (a.k.a. para_up) */

/* Timer CONF fields */
#define LEDC_HSTIMER_DUTY_RES_S     0u    /* bits [4:0]   : duty resolution */
#define LEDC_HSTIMER_DUTY_RES_M     0x1Fu
#define LEDC_HSTIMER_DIV_NUM_S      5u    /* bits [22:5]  : Q10.8 divisor   */
#define LEDC_HSTIMER_DIV_NUM_M      0x3FFFFu
#define LEDC_HSTIMER_PAUSE          (1u << 23)
#define LEDC_HSTIMER_RST            (1u << 24)
#define LEDC_HSTIMER_TICK_SEL       (1u << 25)  /* 1 = APB_CLK, 0 = REF_TICK*/

/* GPIO matrix output config: GPIO_FUNCx_OUT_SEL_CFG_REG @ DR_REG_GPIO_BASE+0x530+4*x */
#define GPIO_FUNC_OUT_SEL_CFG_REG(g) (DR_REG_GPIO_BASE + 0x0530u + ((g) * 4u))

/* IO_MUX FUNC select bits */
#define IOMUX_MCU_SEL_S             12u
#define IOMUX_MCU_SEL_M             (0x07u << IOMUX_MCU_SEL_S)
#define IOMUX_FUNC_GPIO             (2u  << IOMUX_MCU_SEL_S)  /* route via GPIO matrix */
#define IOMUX_FUN_IE                (1u << 9)
#define IOMUX_FUN_WPU               (1u << 8)
#define IOMUX_FUN_WPD               (1u << 7)

/* IO_MUX register address per GPIO (TRM Table 4-3) — offsets are NOT in GPIO order */
static const uint16 io_mux_offset[40] = {
    [0]  = 0x44, [1]  = 0x88, [2]  = 0x40, [3]  = 0x84,
    [4]  = 0x48, [5]  = 0x6C,
    /* 6..11 reserved for SPI flash — left zero, never used */
    [12] = 0x34, [13] = 0x38, [14] = 0x30, [15] = 0x3C,
    [16] = 0x4C, [17] = 0x50, [18] = 0x70, [19] = 0x74,
    [20] = 0x78,
    [21] = 0x7C, [22] = 0x80, [23] = 0x8C,
    [25] = 0x24, [26] = 0x28, [27] = 0x2C,
    [32] = 0x1C, [33] = 0x20,
    [34] = 0x14, [35] = 0x18, [36] = 0x04, [37] = 0x08, [38] = 0x0C, [39] = 0x10,
};
#define IOMUX_REG_ADDR(g)           (DR_REG_IO_MUX_BASE + io_mux_offset[(g)])

/* GPIO 34..39 are input-only — must not enable output */
#define IS_INPUT_ONLY_GPIO(g)       (((g) >= 34) && ((g) <= 39))

/*******************************************************************************
 *                            Private Helpers                                  *
 *******************************************************************************/

/* Ungate the LEDC peripheral clock and pulse its reset (idempotent) */
static void prv_ledcEnablePeripheral(void)
{
    static boolean ledc_periph_initialized = FALSE;
    if (ledc_periph_initialized)
    {
        return;
    }
    REG32(DPORT_PERIP_CLK_EN_REG) |= DPORT_LEDC_CLK_EN;
    REG32(DPORT_PERIP_RST_EN_REG) |= DPORT_LEDC_RST;
    REG32(DPORT_PERIP_RST_EN_REG) &= ~DPORT_LEDC_RST;
    ledc_periph_initialized = TRUE;
}

/* Route a LEDC channel's output signal to a GPIO pad through the GPIO matrix
 * and configure the IO_MUX so the pad is driven by the GPIO matrix.
 */
static void prv_attachChannelToGpio(uint8 channel, uint8 gpio_num)
{
    if ((gpio_num >= 40u) || IS_INPUT_ONLY_GPIO(gpio_num))
    {
        return;
    }

    /* IO_MUX: select GPIO matrix function, drop pulls and input enable */
    uint32 iomux = REG32(IOMUX_REG_ADDR(gpio_num));
    iomux &= ~(IOMUX_MCU_SEL_M | IOMUX_FUN_IE | IOMUX_FUN_WPU | IOMUX_FUN_WPD);
    iomux |= IOMUX_FUNC_GPIO;
    REG32(IOMUX_REG_ADDR(gpio_num)) = iomux;

    /* GPIO matrix: route LEDC_HS_SIG_OUTx_IDX (71 + channel) to the pad,
     * with no inversion and matrix-controlled output enable.            */
    REG32(GPIO_FUNC_OUT_SEL_CFG_REG(gpio_num)) =
        (uint32)(LEDC_HS_SIG_OUT0_IDX + channel);

    /* Enable output driver on the pad */
    if (gpio_num < 32u)
    {
        REG32(GPIO_ENABLE_W1TS_REG) = (1u << gpio_num);
    }
    else
    {
        REG32(GPIO_ENABLE1_W1TS_REG) = (1u << (gpio_num - 32u));
    }
}

/*******************************************************************************
 *                         Function Definitions                                *
 *******************************************************************************/

/*
 * Description :
 * Configure a high-speed LEDC timer with the given frequency and duty
 * resolution.  Clock source = APB_CLK (80 MHz).
 *   div_num = (APB_CLK * 256) / (freq * 2^duty_bits)   (Q10.8 fixed point)
 */
void Pwm_timerInit(const Pwm_TimerConfig *cfg)
{
    if ((cfg == NULL_PTR) || (cfg->duty_bits == 0u) || (cfg->duty_bits > 20u)
            || (cfg->freq_hz == 0u))
    {
        return;
    }

    prv_ledcEnablePeripheral();

    uint32 t          = (uint32)cfg->timer & 0x03u;
    uint32 duty_steps = (1u << cfg->duty_bits);
    /* Q10.8 divisor: shift APB by 8, divide by (freq * duty_steps).
     * Use 64-bit math to avoid overflow when APB_CLK is shifted.       */
    uint32 div_num =
        (uint32)(((uint64)LEDC_APB_CLK_HZ << 8) / ((uint64)cfg->freq_hz * duty_steps));

    if (div_num > LEDC_HSTIMER_DIV_NUM_M)
    {
        div_num = LEDC_HSTIMER_DIV_NUM_M;
    }

    uint32 conf = ((div_num    & LEDC_HSTIMER_DIV_NUM_M) << LEDC_HSTIMER_DIV_NUM_S)
                | (((uint32)cfg->duty_bits & LEDC_HSTIMER_DUTY_RES_M)
                       << LEDC_HSTIMER_DUTY_RES_S)
                | LEDC_HSTIMER_TICK_SEL;        /* APB_CLK */

    REG32(LEDC_HSTIMER_CONF_REG(t)) = conf;

    /* Pulse reset to apply new divisor cleanly */
    REG32(LEDC_HSTIMER_CONF_REG(t)) = conf | LEDC_HSTIMER_RST;
    REG32(LEDC_HSTIMER_CONF_REG(t)) = conf;
}

/*
 * Description :
 * Configure a LEDC channel: bind to a timer, attach to a GPIO via the
 * GPIO matrix, set initial duty 0, hpoint 0, output enabled.
 */
void Pwm_channelInit(const Pwm_ChannelConfig *cfg)
{
    if ((cfg == NULL_PTR) || (cfg->gpio_num < 0) || (cfg->gpio_num >= 40))
    {
        return;
    }

    prv_ledcEnablePeripheral();

    uint32 ch = (uint32)cfg->channel & 0x07u;
    uint32 t  = (uint32)cfg->timer   & 0x03u;

    /* HPOINT = 0  (counter value at which the output goes HIGH) */
    REG32(LEDC_HSCH_HPOINT_REG(ch)) = 0u;

    /* Initial duty = 0 (off). DUTY register is Q20.4 — duty value << 4. */
    REG32(LEDC_HSCH_DUTY_REG(ch)) = 0u;

    /* CONF0: bind to timer, enable signal output, idle low */
    REG32(LEDC_HSCH_CONF0_REG(ch)) =
          (t & LEDC_HSCH_TIMER_SEL_M)
        | LEDC_HSCH_SIG_OUT_EN;

    /* CONF1: no fading — set DUTY_INC and pulse DUTY_START to commit duty */
    REG32(LEDC_HSCH_CONF1_REG(ch)) = LEDC_HSCH_DUTY_INC | LEDC_HSCH_DUTY_START;

    /* Route channel signal to the requested GPIO pad */
    prv_attachChannelToGpio((uint8)ch, (uint8)cfg->gpio_num);
}

/*
 * Description :
 * Set the duty cycle of a channel and commit it immediately.
 * `duty` is interpreted in the channel timer's resolution (e.g. 0..255 for 8-bit).
 * The DUTY register format is Q20.4, so the value must be left-shifted by 4.
 */
void Pwm_setDuty(Pwm_ChannelID channel, uint8 duty)
{
    uint32 ch = (uint32)channel & 0x07u;

    REG32(LEDC_HSCH_DUTY_REG(ch)) = ((uint32)duty) << 4;

    /* Re-arm DUTY_START so the new duty value latches on the next cycle */
    uint32 conf1 = REG32(LEDC_HSCH_CONF1_REG(ch));
    REG32(LEDC_HSCH_CONF1_REG(ch)) = conf1 | LEDC_HSCH_DUTY_START;
}
