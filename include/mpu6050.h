/******************************************************************************
 *
 * Module: MPU-6050 IMU Driver
 *
 * File Name: mpu6050.h
 *
 * Description: Header file for MPU-6050 gyroscope driver.
 *              Uses gyro Z-axis integration for 90-degree turn detection.
 *              I2C master on GPIO 27 (SDA) / GPIO 33 (SCL) at 400 kHz.
 *
 * Wiring:
 *   VCC -> 3.3V
 *   GND -> GND
 *   SDA -> GPIO 27  (PORTC PIN1)
 *   SCL -> GPIO 33  (PORTC PIN3)
 *   AD0 -> GND      (I2C address 0x68)
 *   INT -> Not connected
 *   XDA -> Not connected
 *   XCL -> Not connected
 *
 * Assumptions:
 *   - MPU-6050 module is mounted flat (PCB parallel to ground).
 *   - Gyro Z-axis points up — measures yaw (horizontal rotation).
 *   - Robot is stationary during MPU6050_calibrate().
 *
 *******************************************************************************/

#ifndef MPU6050_H_
#define MPU6050_H_

#include "std_types.h"

/*******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************/

typedef enum
{
    MPU6050_TURN_LEFT  = 0,
    MPU6050_TURN_RIGHT = 1
} MPU6050_TurnDir;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize I2C bus and configure MPU-6050.
 * Sets gyro range to ±250 °/s with 188 Hz DLPF.
 * Must be called once before any other MPU function.
 */
void MPU6050_init(void);

/*
 * Description :
 * Average 100 gyro Z samples while the robot is stationary to compute
 * the zero-rate offset. Takes approximately 1 second.
 * Must be called after MPU6050_init() and before MPU6050_turn().
 * Robot must not be moving during this call.
 */
void MPU6050_calibrate(void);

/*
 * Description :
 * Return the calibrated gyro Z rate in degrees per second.
 * Positive = counter-clockwise, Negative = clockwise (Z-up convention).
 * Useful for verifying sensor output before using MPU6050_turn().
 */
float32 MPU6050_getGyroZ(void);

/*
 * Description :
 * Perform a blocking 90-degree pivot turn.
 * Drives all 4 motors for an in-place rotation, integrates gyro Z at 100 Hz,
 * and stops all motors when 90 degrees is reached.
 * speed: 0-255 (motor speed during the turn)
 */
void MPU6050_turn(MPU6050_TurnDir dir, uint8 speed);

#endif /* MPU6050_H_ */
