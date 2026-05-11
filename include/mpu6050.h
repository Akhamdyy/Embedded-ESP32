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
 * Call once per FSM tick while the robot is stationary.
 * Accumulates gyro Z samples for zero-rate offset estimation.
 * Returns TRUE when enough samples have been collected and the
 * offset has been applied.  No blocking delays inside.
 */
boolean MPU6050_calibrateStep(void);

/*
 * Description :
 * Return the calibrated gyro Z rate in degrees per second.
 */
float32 MPU6050_getGyroZ(void);

/*
 * Description :
 * Start a 90-degree pivot turn.  Sets motor directions and captures
 * the initial timestamp.  Call once when entering the turn state.
 * No blocking delays inside.
 */
void MPU6050_turnBegin(MPU6050_TurnDir dir, uint8 speed);

/*
 * Description :
 * Advance the turn by one FSM tick.  Reads gyro Z, integrates the
 * angle, and returns TRUE when TARGET_ANGLE_DEG is reached (motors
 * are braked automatically).  Returns FALSE while still turning.
 * No blocking delays inside.
 */
boolean MPU6050_turnStep(void);

#endif /* MPU6050_H_ */
