/******************************************************************************
 *
 * Module: Platform
 *
 * File Name: platform.h
 *
 * Description: Platform-specific attribute macros for ESP32.
 *              Provides portable aliases so MCAL/HAL modules never include
 *              ESP-IDF headers directly for compiler attributes.
 *
 *              Implements IRAM placement directly via the GCC section
 *              attribute, eliminating the dependency on <esp_attr.h>.
 *
 *******************************************************************************/

#ifndef PLATFORM_H_
#define PLATFORM_H_

/* Place a function/symbol in IRAM (.iram1 section).
 * Required for ISR handlers and any code that may run while the flash
 * cache is disabled (e.g. during SPI flash operations).
 */
#define ISR_ATTR    __attribute__((section(".iram1")))

#endif /* PLATFORM_H_ */
