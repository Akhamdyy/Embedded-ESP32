/******************************************************************************
 *
 * Module: GPIO
 *
 * File Name: gpio.c
 *
 * Description: Source file for the ESP32 GPIO driver.
 *              Implements the same API as the original AVR driver using
 *              the ESP-IDF gpio driver underneath.
 *
 *******************************************************************************/

#include "gpio.h"
#include "driver/gpio.h"
#include "esp_attr.h"

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/*
 * Lookup table: maps port_num + pin_num to the physical ESP32 GPIO number.
 * Only the 24 GPIO pins that are safe to use on the ESP32-WROOM-32 are included.
 * See gpio.h for the full mapping and exclusion reasons.
 */
static const gpio_num_t gpio_map[NUM_OF_PORTS][NUM_OF_PINS_PER_PORT] = {
    /* PORTA */ { GPIO_NUM_0,  GPIO_NUM_2,  GPIO_NUM_4,  GPIO_NUM_5,
                  GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15 },
    /* PORTB */ { GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19,
                  GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_25 },
    /* PORTC */ { GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_32, GPIO_NUM_33,
                  GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_39 },
};

/*
 * Tracks the configured direction of each pin so GPIO_writePin can decide
 * whether to drive the output level or control the pull-up resistor.
 * All pins default to PIN_INPUT (matches ESP32 reset state).
 */
static GPIO_PinDirectionType pin_direction[NUM_OF_PORTS][NUM_OF_PINS_PER_PORT];

/* Prevents calling gpio_install_isr_service more than once */
static boolean isr_service_installed = FALSE;

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

    gpio_num_t gpio = gpio_map[port_num][pin_num];
    gpio_reset_pin(gpio);

    if (direction == PIN_OUTPUT)
    {
        gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    }
    else
    {
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
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

    gpio_num_t gpio = gpio_map[port_num][pin_num];

    if (pin_direction[port_num][pin_num] == PIN_OUTPUT)
    {
        gpio_set_level(gpio, value);
    }
    else
    {
        /* Input pin: enable/disable internal pull-up resistor */
        if (value == LOGIC_HIGH)
        {
            gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
        }
        else
        {
            gpio_set_pull_mode(gpio, GPIO_FLOATING);
        }
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

    gpio_num_t gpio = gpio_map[port_num][pin_num];
    return (uint8)gpio_get_level(gpio);
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

    gpio_num_t gpio = gpio_map[port_num][pin_num];

    switch (pull)
    {
    case GPIO_PULL_UP:
        gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
        break;
    case GPIO_PULL_DOWN:
        gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
        break;
    case GPIO_PULL_NONE:
    default:
        gpio_set_pull_mode(gpio, GPIO_FLOATING);
        break;
    }
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

    gpio_num_t gpio = gpio_map[port_num][pin_num];

    gpio_int_type_t esp_intr;
    switch (trigger)
    {
    case GPIO_INTR_RISING_EDGE:  esp_intr = GPIO_INTR_POSEDGE; break;
    case GPIO_INTR_FALLING_EDGE: esp_intr = GPIO_INTR_NEGEDGE; break;
    case GPIO_INTR_ANY_EDGE:
    default:                     esp_intr = GPIO_INTR_ANYEDGE; break;
    }

    gpio_set_intr_type(gpio, esp_intr);

    if (!isr_service_installed)
    {
        gpio_install_isr_service(0);
        isr_service_installed = TRUE;
    }

    gpio_isr_handler_add(gpio, callback, arg);
}
