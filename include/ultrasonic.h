/******************************************************************************
 *
 * Module: Ultrasonic Sensor (Multi-Sensor)
 *
 * File Name: ultrasonic.h
 *
 * Description: Header file for HC-SR04 ultrasonic distance sensor driver.
 *              Supports 3 sensors (Front, Left, Right).
 *              Fully timer and interrupt driven - no blocking delays.
 *
 * Wiring:
 *   All sensors:
 *     VCC  -> 5V
 *     GND  -> GND
 *
 *   Front Sensor (GPIO 26/27):
 *     TRIG -> GPIO 26 (PORTC PIN0)  direct connection
 *     ECHO -> voltage divider     -> GPIO 27 (PORTC PIN1)
 *
 *   Left Sensor (GPIO 32/33):
 *     TRIG -> GPIO 32 (PORTC PIN2)  direct connection
 *     ECHO -> voltage divider     -> GPIO 33 (PORTC PIN3)
 *
 *   Right Sensor (GPIO 4/5):
 *     TRIG -> GPIO 4  (PORTA PIN2)  direct connection
 *     ECHO -> voltage divider     -> GPIO 5  (PORTA PIN3)
 *
 *   Echo voltage divider (5V -> 3.3V) for EACH sensor:
 *     HC-SR04 ECHO ── 1kΩ ── GPIOxx
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
 *                              Type Definitions                               *
 *******************************************************************************/

typedef enum
{
    ULTRASONIC_FRONT = 0,
    ULTRASONIC_LEFT  = 1,
    ULTRASONIC_RIGHT = 2,
    ULTRASONIC_COUNT = 3
} Ultrasonic_SensorID;

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define ULTRASONIC_OUT_OF_RANGE    0xFFFFU   /* Returned when no echo received */

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize all 3 ultrasonic sensors (Front, Left, Right).
 * Must be called once before use.
 * measurement_interval_ms: how often to trigger each sensor (per sensor, min 60ms).
 * Full cycle = measurement_interval_ms * 3 (staggered firing)
 */
void Ultrasonic_initAll(uint32 measurement_interval_ms);

/*
 * Description :
 * Return the last successfully measured distance for the specified sensor (cm).
 * Returns ULTRASONIC_OUT_OF_RANGE if no valid measurement is available.
 * Safe to call from any context.
 */
uint16 Ultrasonic_getDistance(Ultrasonic_SensorID sensor);

/*
 * Description :
 * Return TRUE if a new measurement has completed for the specified sensor
 * since the last call to Ultrasonic_getDistance() for that sensor.
 */
boolean Ultrasonic_isDataReady(Ultrasonic_SensorID sensor);

#endif /* ULTRASONIC_H_ */
