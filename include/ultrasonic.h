/******************************************************************************
 *
 * Module: Ultrasonic Sensor
 *
 * File Name: ultrasonic.h
 *
 * Description: Header file for the HC-SR04 ultrasonic distance sensor driver.
 *              Fully timer and interrupt driven - no blocking delays.
 *
 * Wiring:
 *   VCC  -> 5V
 *   GND  -> GND
 *   TRIG -> GPIO 26 (PORTC PIN0)  direct connection
 *   ECHO -> voltage divider     -> GPIO 27 (PORTC PIN1)
 *
 *   Echo voltage divider (5V -> 3.3V):
 *     HC-SR04 ECHO ── 1kΩ ── GPIO 27
 *                        |
 *                       2kΩ
 *                        |
 *                       GND
 *
 * Range    : 2 cm to 400 cm
 * Accuracy : ~3 mm
 *
 *******************************************************************************/

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define ULTRASONIC_OUT_OF_RANGE    0xFFFFU   /* Returned when no echo received */

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize the ultrasonic sensor GPIO pins, interrupts, and timers.
 * Must be called once before use.
 * measurement_interval_ms: how often to trigger a measurement and log (min 60ms).
 */
void Ultrasonic_init(uint32 measurement_interval_ms);

/*
 * Description :
 * Return the last successfully measured distance in centimeters.
 * Returns ULTRASONIC_OUT_OF_RANGE if no valid measurement is available.
 * Safe to call from any context.
 */
uint16 Ultrasonic_getDistance(void);

/*
 * Description :
 * Return TRUE if a new measurement has completed since the last call
 * to Ultrasonic_getDistance().
 */
boolean Ultrasonic_isDataReady(void);

#endif /* ULTRASONIC_H_ */
