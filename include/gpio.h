/******************************************************************************
 *
 * Module: GPIO
 *
 * File Name: gpio.h
 *
 * Description: Header file for the ESP32 GPIO driver.
 *              Provides the same API as the original AVR driver.
 *
 * Board: ESP32 DOIT DevKit V1 (ESP32-WROOM-32)
 *
 * Port/Pin to ESP32 GPIO mapping:
 *
 *   PORTA (0):
 *     PIN0=GPIO0   PIN1=GPIO2   PIN2=GPIO4   PIN3=GPIO5
 *     PIN4=GPIO12  PIN5=GPIO13  PIN6=GPIO14  PIN7=GPIO15
 *
 *   PORTB (1):
 *     PIN0=GPIO16  PIN1=GPIO17  PIN2=GPIO18  PIN3=GPIO19
 *     PIN4=GPIO21  PIN5=GPIO22  PIN6=GPIO23  PIN7=GPIO25
 *
 *   PORTC (2):
 *     PIN0=GPIO26  PIN1=GPIO27  PIN2=GPIO32  PIN3=GPIO33
 *     PIN4=GPIO34* PIN5=GPIO35* PIN6=GPIO36* PIN7=GPIO39*
 *     (* INPUT ONLY - do not use as output)
 *
 *   PORTD: NOT AVAILABLE on ESP32-WROOM-32
 *          (GPIO 24, 28-31 do not exist on this chip)
 *
 * Pins excluded from mapping:
 *   GPIO 6-11  : internal SPI flash (forbidden)
 *   GPIO 1, 3  : UART0 TX/RX used by USB serial
 *   GPIO 20,24 : do not exist on ESP32
 *   GPIO 28-31 : do not exist on ESP32
 *
 * Strapping pins (work normally after boot, but affect boot mode):
 *   GPIO 0 (PORTA PIN0): internal pull-up, used for boot button
 *   GPIO 2 (PORTA PIN1): internal pull-down, on-board LED on DOIT DevKit
 *   GPIO 5 (PORTA PIN3): internal pull-up during boot
 *   GPIO 12 (PORTA PIN4): must be LOW during boot (flash voltage select)
 *   GPIO 15 (PORTA PIN7): internal pull-up during boot
 *
 *******************************************************************************/

#ifndef GPIO_H_
#define GPIO_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define NUM_OF_PORTS           3   /* Only 3 usable ports on ESP32-WROOM-32 */
#define NUM_OF_PINS_PER_PORT   8

#define PORTA_ID               0
#define PORTB_ID               1
#define PORTC_ID               2
#define PORTD_ID               3   /* NOT AVAILABLE on this board */

#define PIN0_ID                0
#define PIN1_ID                1
#define PIN2_ID                2
#define PIN3_ID                3
#define PIN4_ID                4
#define PIN5_ID                5
#define PIN6_ID                6
#define PIN7_ID                7

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
    PIN_INPUT, PIN_OUTPUT
} GPIO_PinDirectionType;

typedef enum
{
    PORT_INPUT, PORT_OUTPUT = 0xFF
} GPIO_PortDirectionType;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Setup the direction of the required pin input/output.
 * If the input port number or pin number are not correct, The function will not handle the request.
 */
void GPIO_setupPinDirection(uint8 port_num, uint8 pin_num, GPIO_PinDirectionType direction);

/*
 * Description :
 * Write the value Logic High or Logic Low on the required pin.
 * If the input port number or pin number are not correct, The function will not handle the request.
 * If the pin is input, this function will enable/disable the internal pull-up resistor.
 */
void GPIO_writePin(uint8 port_num, uint8 pin_num, uint8 value);

/*
 * Description :
 * Read and return the value for the required pin, it should be Logic High or Logic Low.
 * If the input port number or pin number are not correct, The function will return Logic Low.
 */
uint8 GPIO_readPin(uint8 port_num, uint8 pin_num);

/*
 * Description :
 * Setup the direction of the required port all pins input/output.
 * If the direction value is PORT_INPUT all pins in this port should be input pins.
 * If the direction value is PORT_OUTPUT all pins in this port should be output pins.
 * If the input port number is not correct, The function will not handle the request.
 */
void GPIO_setupPortDirection(uint8 port_num, GPIO_PortDirectionType direction);

/*
 * Description :
 * Write the value on the required port.
 * If any pin in the port is output pin the value will be written.
 * If any pin in the port is input pin this will activate/deactivate the internal pull-up resistor.
 * If the input port number is not correct, The function will not handle the request.
 */
void GPIO_writePort(uint8 port_num, uint8 value);

/*
 * Description :
 * Read and return the value of the required port.
 * If the input port number is not correct, The function will return ZERO value.
 */
uint8 GPIO_readPort(uint8 port_num);

#endif /* GPIO_H_ */
