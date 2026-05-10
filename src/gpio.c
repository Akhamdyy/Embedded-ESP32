/******************************************************************************
 *
 * Module: GPIO
 *
 * File Name: gpio.c
 *
 * Description: ESP32 GPIO driver implemented via DIRECT register access
 *              (no driver/gpio.h). Same API as the original AVR driver.
 *              Register layout per ESP32 TRM section 4 (IO_MUX & GPIO).
 *
 *******************************************************************************/

#include "gpio.h"
#include "platform.h"           /* IRAM_ATTR / ISR_ATTR                     */

#include "soc/gpio_reg.h"       /* GPIO_*_REG addresses                     */
#include "soc/io_mux_reg.h"     /* DR_REG_IO_MUX_BASE                       */
#include "soc/soc.h"            /* ETS_GPIO_INTR_SOURCE                     */
#include "esp_intr_alloc.h"     /* esp_intr_alloc, intr_handle_t            */

/*******************************************************************************
 *                              Register Helper                                *
 *******************************************************************************/

#define REG32(addr)                 (*((volatile uint32 *)(addr)))

/*******************************************************************************
 *                  IO_MUX bit fields  (ESP32 TRM Table 4-4)                   *
 *******************************************************************************/
#define IOMUX_FUN_WPD_BIT           7   /* function pull-down enable        */
#define IOMUX_FUN_WPU_BIT           8   /* function pull-up   enable        */
#define IOMUX_FUN_IE_BIT            9   /* function input     enable        */
#define IOMUX_MCU_SEL_SHIFT         12
#define IOMUX_MCU_SEL_MASK          (0x7u  << IOMUX_MCU_SEL_SHIFT)
#define IOMUX_MCU_SEL_GPIO          (0x2u  << IOMUX_MCU_SEL_SHIFT) /* matrix */

/*******************************************************************************
 *                  GPIO_PINn_REG bit fields  (TRM 4.11)                       *
 *******************************************************************************/
#define GPIO_PIN_INT_TYPE_SHIFT     7
#define GPIO_PIN_INT_TYPE_MASK      (0x7u  << GPIO_PIN_INT_TYPE_SHIFT)
#define GPIO_PIN_INT_ENA_SHIFT      13
#define GPIO_PIN_INT_ENA_MASK       (0x1Fu << GPIO_PIN_INT_ENA_SHIFT)
#define GPIO_PIN_INT_ENA_PRO_CPU    (0x1u  << GPIO_PIN_INT_ENA_SHIFT)

#define GPIO_INT_TYPE_DISABLE       0u
#define GPIO_INT_TYPE_POSEDGE       1u
#define GPIO_INT_TYPE_NEGEDGE       2u
#define GPIO_INT_TYPE_ANYEDGE       3u

/*******************************************************************************
 *                  GPIO_FUNCx_OUT_SEL_CFG_REG (TRM 4.11)                      *
 *
 * Default value 0x100 means: out_sel = 256 (signal index "use GPIO_OUT_REG"),
 * inv_sel = 0, oen_sel = 0 (output enable controlled by GPIO_ENABLE_REG).
 ******************************************************************************/
#define GPIO_FUNC_OUT_SEL_CFG_REG(g)  (DR_REG_GPIO_BASE + 0x530u + ((g) * 4u))
#define GPIO_FUNC_OUT_SEL_GPIO_OUT    0x100u

/*******************************************************************************
 *                  GPIO_PINn_REG address                                      *
 ******************************************************************************/
#define GPIO_PIN_REG_ADDR(g)          (DR_REG_GPIO_BASE + 0x88u + ((g) * 4u))

/*******************************************************************************
 *           IO_MUX register offsets per ESP32 TRM Table 4-3                   *
 *
 * These are NOT in GPIO numerical order — the table must be looked up.
 * Indexed by physical GPIO number (0..39). Slots for GPIOs that don't
 * exist on ESP32-WROOM-32 (28..31) are zeroed and never read; gpio_map
 * filters them out.
 ******************************************************************************/
static const uint16 io_mux_offset[40] = {
    [0]  = 0x44u,
    [1]  = 0x88u,    /* U0TXD     - reserved (UART0)                         */
    [2]  = 0x40u,
    [3]  = 0x84u,    /* U0RXD     - reserved (UART0)                         */
    [4]  = 0x48u,
    [5]  = 0x6Cu,
    [6]  = 0x60u,    /* SD_CLK    - reserved (SPI flash)                     */
    [7]  = 0x64u,    /* SD_DATA0  - reserved (SPI flash)                     */
    [8]  = 0x68u,    /* SD_DATA1  - reserved (SPI flash)                     */
    [9]  = 0x54u,    /* SD_DATA2  - reserved (SPI flash)                     */
    [10] = 0x58u,    /* SD_DATA3  - reserved (SPI flash)                     */
    [11] = 0x5Cu,    /* SD_CMD    - reserved (SPI flash)                     */
    [12] = 0x34u,    /* MTDI                                                 */
    [13] = 0x38u,    /* MTCK                                                 */
    [14] = 0x30u,    /* MTMS                                                 */
    [15] = 0x3Cu,    /* MTDO                                                 */
    [16] = 0x4Cu,
    [17] = 0x50u,
    [18] = 0x70u,
    [19] = 0x74u,
    [20] = 0x78u,
    [21] = 0x7Cu,
    [22] = 0x80u,
    [23] = 0x8Cu,
    [24] = 0x90u,
    [25] = 0x24u,
    [26] = 0x28u,
    [27] = 0x2Cu,
    /* 28..31 do not exist on ESP32                                          */
    [32] = 0x1Cu,
    [33] = 0x20u,
    [34] = 0x14u,    /* input only - no internal pulls                       */
    [35] = 0x18u,    /* input only - no internal pulls                       */
    [36] = 0x04u,    /* input only - no internal pulls                       */
    [37] = 0x08u,    /* input only - no internal pulls                       */
    [38] = 0x0Cu,    /* input only - no internal pulls                       */
    [39] = 0x10u,    /* input only - no internal pulls                       */
};

#define IOMUX_REG_ADDR(g)   (DR_REG_IO_MUX_BASE + io_mux_offset[(g)])

/* GPIO34..39 are input-only — they cannot drive an output and have no
 * internal pull resistors. */
#define IS_INPUT_ONLY_GPIO(g)   ((g) >= 34u && (g) <= 39u)

/*******************************************************************************
 *                                Private Data                                 *
 *******************************************************************************/

/*
 * Lookup table: maps port_num + pin_num to the physical ESP32 GPIO number.
 * Only the 24 GPIO pins that are safe to use on the ESP32-WROOM-32 are
 * included. See gpio.h for the full mapping and exclusion reasons.
 *
 * Type is plain uint8 (was gpio_num_t from driver/gpio.h). Numeric values
 * are unchanged so all callers behave identically.
 */
static const uint8 gpio_map[NUM_OF_PORTS][NUM_OF_PINS_PER_PORT] = {
    /* PORTA */ {  0u,  2u,  4u,  5u, 12u, 13u, 14u, 15u },
    /* PORTB */ { 16u, 17u, 18u, 19u, 21u, 22u, 23u, 25u },
    /* PORTC */ { 26u, 27u, 32u, 33u, 34u, 35u, 36u, 39u },
};

/*
 * Tracks the configured direction of each pin so GPIO_writePin can decide
 * whether to drive the output level or control the pull-up resistor.
 * All pins default to PIN_INPUT (matches ESP32 reset state).
 */
static GPIO_PinDirectionType pin_direction[NUM_OF_PORTS][NUM_OF_PINS_PER_PORT];

/*******************************************************************************
 *                            ISR Dispatch Table                               *
 *
 * One entry per physical GPIO (0..39). The shared ISR reads the latched
 * GPIO_STATUS / GPIO_STATUS1 registers, fans out to the matching callback,
 * then clears the status bits.
 ******************************************************************************/
typedef struct
{
    GPIO_IsrCallback callback;
    void            *arg;
} gpio_isr_entry_t;

static gpio_isr_entry_t isr_table[40];
static boolean          gpio_intr_installed = FALSE;

/*
 * Shared GPIO ISR — placed in IRAM so it can run while flash cache is
 * disabled (e.g. during SPI flash writes).
 */
static IRAM_ATTR void GPIO_sharedIsr(void *unused)
{
    (void)unused;

    uint32 status0 = REG32(GPIO_STATUS_REG);
    uint32 status1 = REG32(GPIO_STATUS1_REG);

    /* Acknowledge first so a fresh edge during the callback is not lost. */
    REG32(GPIO_STATUS_W1TC_REG)  = status0;
    REG32(GPIO_STATUS1_W1TC_REG) = status1;

    uint8 i;
    for (i = 0u; i < 32u; i++)
    {
        if ((status0 & (1u << i)) && (isr_table[i].callback != NULL_PTR))
        {
            isr_table[i].callback(isr_table[i].arg);
        }
    }
    for (i = 0u; i < 8u; i++)
    {
        uint8 idx = (uint8)(32u + i);
        if ((status1 & (1u << i)) && (isr_table[idx].callback != NULL_PTR))
        {
            isr_table[idx].callback(isr_table[idx].arg);
        }
    }
}

/*******************************************************************************
 *                          Internal Helpers                                   *
 *******************************************************************************/

/*
 * Configure IO_MUX for a given GPIO:
 *   - select function 3 (MCU_SEL = 2) so the pad is routed through the
 *     GPIO matrix
 *   - enable input buffer for input pins so GPIO_IN_REG reflects the level
 *   - clear pull-up / pull-down (callers may reapply via GPIO_setPullMode
 *     or the legacy GPIO_writePin behaviour)
 */
static void IOMUX_configure(uint8 gpio_num, GPIO_PinDirectionType direction)
{
    volatile uint32 *iomux = (volatile uint32 *)IOMUX_REG_ADDR(gpio_num);
    uint32 v = *iomux;

    v &= ~IOMUX_MCU_SEL_MASK;
    v |=  IOMUX_MCU_SEL_GPIO;

    v &= ~((1u << IOMUX_FUN_WPU_BIT) | (1u << IOMUX_FUN_WPD_BIT));

    if (direction == PIN_INPUT)
    {
        v |=  (1u << IOMUX_FUN_IE_BIT);
    }
    else
    {
        v &= ~(1u << IOMUX_FUN_IE_BIT);
    }

    *iomux = v;
}

static void GPIO_outputEnable(uint8 gpio)
{
    /* Route GPIO_OUT_REG to the pad (default function-out signal = 256). */
    REG32(GPIO_FUNC_OUT_SEL_CFG_REG(gpio)) = GPIO_FUNC_OUT_SEL_GPIO_OUT;

    if (gpio < 32u)
    {
        REG32(GPIO_ENABLE_W1TS_REG)  = (1u << gpio);
    }
    else
    {
        REG32(GPIO_ENABLE1_W1TS_REG) = (1u << (gpio - 32u));
    }
}

static void GPIO_outputDisable(uint8 gpio)
{
    if (gpio < 32u)
    {
        REG32(GPIO_ENABLE_W1TC_REG)  = (1u << gpio);
    }
    else
    {
        REG32(GPIO_ENABLE1_W1TC_REG) = (1u << (gpio - 32u));
    }
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Setup the direction of the required pin input/output.
 * If the input port number or pin number are not correct, The function will not handle the request.
 */
void GPIO_setupPinDirection(uint8 port_num, uint8 pin_num, GPIO_PinDirectionType direction)
{
    if ((pin_num >= NUM_OF_PINS_PER_PORT) || (port_num >= NUM_OF_PORTS))
    {
        /* Do Nothing */
        return;
    }

    uint8 gpio = gpio_map[port_num][pin_num];

    /* Tear down any previously configured interrupt on this pin. */
    REG32(GPIO_PIN_REG_ADDR(gpio)) &= ~(GPIO_PIN_INT_TYPE_MASK | GPIO_PIN_INT_ENA_MASK);

    IOMUX_configure(gpio, direction);

    if (direction == PIN_OUTPUT)
    {
        /* GPIO34..39 are input-only — silently reject output requests. */
        if (!IS_INPUT_ONLY_GPIO(gpio))
        {
            GPIO_outputEnable(gpio);
        }
    }
    else
    {
        GPIO_outputDisable(gpio);
    }

    pin_direction[port_num][pin_num] = direction;
}

/*
 * Description :
 * Write the value Logic High or Logic Low on the required pin.
 * If the input port number or pin number are not correct, The function will not handle the request.
 * If the pin is input, this function will enable/disable the internal pull-up resistor.
 */
void GPIO_writePin(uint8 port_num, uint8 pin_num, uint8 value)
{
    if ((pin_num >= NUM_OF_PINS_PER_PORT) || (port_num >= NUM_OF_PORTS))
    {
        /* Do Nothing */
        return;
    }

    uint8 gpio = gpio_map[port_num][pin_num];

    if (pin_direction[port_num][pin_num] == PIN_OUTPUT)
    {
        if (IS_INPUT_ONLY_GPIO(gpio))
        {
            /* Cannot drive an input-only pin. */
            return;
        }

        if (gpio < 32u)
        {
            if (value == LOGIC_HIGH)
            {
                REG32(GPIO_OUT_W1TS_REG) = (1u << gpio);
            }
            else
            {
                REG32(GPIO_OUT_W1TC_REG) = (1u << gpio);
            }
        }
        else
        {
            if (value == LOGIC_HIGH)
            {
                REG32(GPIO_OUT1_W1TS_REG) = (1u << (gpio - 32u));
            }
            else
            {
                REG32(GPIO_OUT1_W1TC_REG) = (1u << (gpio - 32u));
            }
        }
    }
    else
    {
        /* Input pin: legacy AVR semantics — value enables the pull-up. */
        if (IS_INPUT_ONLY_GPIO(gpio))
        {
            /* GPIO34..39 have no internal pulls. */
            return;
        }

        volatile uint32 *iomux = (volatile uint32 *)IOMUX_REG_ADDR(gpio);
        uint32 v = *iomux;

        if (value == LOGIC_HIGH)
        {
            v |=  (1u << IOMUX_FUN_WPU_BIT);
            v &= ~(1u << IOMUX_FUN_WPD_BIT);
        }
        else
        {
            v &= ~((1u << IOMUX_FUN_WPU_BIT) | (1u << IOMUX_FUN_WPD_BIT));
        }

        *iomux = v;
    }
}

/*
 * Description :
 * Read and return the value for the required pin, it should be Logic High or Logic Low.
 * If the input port number or pin number are not correct, The function will return Logic Low.
 */
IRAM_ATTR uint8 GPIO_readPin(uint8 port_num, uint8 pin_num)
{
    if ((pin_num >= NUM_OF_PINS_PER_PORT) || (port_num >= NUM_OF_PORTS))
    {
        return LOGIC_LOW;
    }

    uint8 gpio = gpio_map[port_num][pin_num];

    if (gpio < 32u)
    {
        return (uint8)((REG32(GPIO_IN_REG)  >> gpio)        & 1u);
    }
    else
    {
        return (uint8)((REG32(GPIO_IN1_REG) >> (gpio - 32u)) & 1u);
    }
}

/*
 * Description :
 * Setup the direction of the required port all pins input/output.
 * If the direction value is PORT_INPUT all pins in this port should be input pins.
 * If the direction value is PORT_OUTPUT all pins in this port should be output pins.
 * If the input port number is not correct, The function will not handle the request.
 */
void GPIO_setupPortDirection(uint8 port_num, GPIO_PortDirectionType direction)
{
    if (port_num >= NUM_OF_PORTS)
    {
        /* Do Nothing */
        return;
    }

    uint8 pin;
    for (pin = 0; pin < NUM_OF_PINS_PER_PORT; pin++)
    {
        GPIO_setupPinDirection(port_num, pin,
            (direction == PORT_OUTPUT) ? PIN_OUTPUT : PIN_INPUT);
    }
}

/*
 * Description :
 * Write the value on the required port.
 * If any pin in the port is output pin the value will be written.
 * If any pin in the port is input pin this will activate/deactivate the internal pull-up resistor.
 * If the input port number is not correct, The function will not handle the request.
 */
void GPIO_writePort(uint8 port_num, uint8 value)
{
    if (port_num >= NUM_OF_PORTS)
    {
        /* Do Nothing */
        return;
    }

    uint8 pin;
    for (pin = 0; pin < NUM_OF_PINS_PER_PORT; pin++)
    {
        GPIO_writePin(port_num, pin, (value >> pin) & 1U);
    }
}

/*
 * Description :
 * Read and return the value of the required port.
 * If the input port number is not correct, The function will return ZERO value.
 */
uint8 GPIO_readPort(uint8 port_num)
{
    if (port_num >= NUM_OF_PORTS)
    {
        return LOGIC_LOW;
    }

    uint8 value = 0;
    uint8 pin;
    for (pin = 0; pin < NUM_OF_PINS_PER_PORT; pin++)
    {
        if (GPIO_readPin(port_num, pin) == LOGIC_HIGH)
        {
            value |= (uint8)(1U << pin);
        }
    }
    return value;
}

/*
 * Description :
 * Set the pull-up or pull-down resistor for a pin already configured as input.
 */
void GPIO_setPullMode(uint8 port_num, uint8 pin_num, GPIO_PullType pull)
{
    if ((pin_num >= NUM_OF_PINS_PER_PORT) || (port_num >= NUM_OF_PORTS))
    {
        return;
    }

    uint8 gpio = gpio_map[port_num][pin_num];

    /* GPIO34..39 are input-only and have no internal pull resistors. */
    if (IS_INPUT_ONLY_GPIO(gpio))
    {
        return;
    }

    volatile uint32 *iomux = (volatile uint32 *)IOMUX_REG_ADDR(gpio);
    uint32 v = *iomux;

    /* Always start by clearing both pulls so the switch is clean. */
    v &= ~((1u << IOMUX_FUN_WPU_BIT) | (1u << IOMUX_FUN_WPD_BIT));

    switch (pull)
    {
    case GPIO_PULL_UP:
        v |= (1u << IOMUX_FUN_WPU_BIT);
        break;
    case GPIO_PULL_DOWN:
        v |= (1u << IOMUX_FUN_WPD_BIT);
        break;
    case GPIO_PULL_NONE:
    default:
        /* Both bits already cleared above. */
        break;
    }

    *iomux = v;
}

/*
 * Description :
 * Enable edge-triggered interrupt on an input pin and register an ISR handler.
 */
void GPIO_enableInterrupt(uint8 port_num, uint8 pin_num, GPIO_IntrTrigger trigger,
                          GPIO_IsrCallback callback, void *arg)
{
    if ((pin_num >= NUM_OF_PINS_PER_PORT) || (port_num >= NUM_OF_PORTS))
    {
        return;
    }

    uint8 gpio = gpio_map[port_num][pin_num];

    /* Translate API trigger to the chip's INT_TYPE field encoding. */
    uint32 int_type;
    switch (trigger)
    {
    case GPIO_INTR_RISING_EDGE:  int_type = GPIO_INT_TYPE_POSEDGE; break;
    case GPIO_INTR_FALLING_EDGE: int_type = GPIO_INT_TYPE_NEGEDGE; break;
    case GPIO_INTR_ANY_EDGE:
    default:                     int_type = GPIO_INT_TYPE_ANYEDGE; break;
    }

    /* Register the user callback BEFORE enabling the interrupt at the
     * pin so a spurious edge during configuration cannot fire into a
     * NULL callback. */
    isr_table[gpio].callback = callback;
    isr_table[gpio].arg      = arg;

    /* Clear any latched edge before re-arming. */
    if (gpio < 32u)
    {
        REG32(GPIO_STATUS_W1TC_REG)  = (1u << gpio);
    }
    else
    {
        REG32(GPIO_STATUS1_W1TC_REG) = (1u << (gpio - 32u));
    }

    /* Program GPIO_PINn_REG: set INT_TYPE and route the interrupt to PRO CPU. */
    volatile uint32 *pin_reg = (volatile uint32 *)GPIO_PIN_REG_ADDR(gpio);
    uint32 v = *pin_reg;
    v &= ~(GPIO_PIN_INT_TYPE_MASK | GPIO_PIN_INT_ENA_MASK);
    v |=  (int_type << GPIO_PIN_INT_TYPE_SHIFT);
    v |=  GPIO_PIN_INT_ENA_PRO_CPU;
    *pin_reg = v;

    /* Allocate the shared GPIO peripheral interrupt the first time only. */
    if (!gpio_intr_installed)
    {
        (void)esp_intr_alloc(ETS_GPIO_INTR_SOURCE,
                             0,                  /* default level (1)        */
                             GPIO_sharedIsr,
                             NULL_PTR,
                             NULL_PTR);
        gpio_intr_installed = TRUE;
    }
}
