/******************************************************************************
 *
 * Module: MPU-6050 IMU Driver
 *
 * File Name: mpu6050.c
 *
 * Description: Non-blocking MPU-6050 / MPU-6500 gyroscope driver.
 *              Calibration and turn are step functions — called once per
 *              FSM tick with no vTaskDelay inside.
 *
 *******************************************************************************/

#include "mpu6050.h"
#include "motor.h"
#include "i2c.h"
#include "timer.h"
#include "bluetooth.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define MPU_ADDR            0x68

#define REG_SMPLRT_DIV      0x19
#define REG_CONFIG          0x1A
#define REG_GYRO_CONFIG     0x1B
#define REG_PWR_MGMT_1      0x6B
#define REG_GYRO_ZOUT_H     0x47

#define GYRO_SENSITIVITY    131.0f   /* ±250 °/s → 131 LSB per °/s */

/* Calibration: 20 ticks × 50 ms FSM tick = 1 second of samples */
#define CALIBRATION_SAMPLES 20u

/* Per-direction target — motor / gyro asymmetry means one direction can
 * over- or under-rotate even with the same target.  Tune each one against
 * the physical 90° mark independently:
 *   - left rotates too far  → lower TARGET_ANGLE_LEFT
 *   - left rotates too short → raise TARGET_ANGLE_LEFT
 *   - same for right.
 * Inertia after the brake makes up the rest of the 90°. */
#define TARGET_ANGLE_LEFT   62.0f
#define TARGET_ANGLE_RIGHT  62.0f

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static float32 gyro_z_offset  = 0.0f;

/* Calibration state */
static float32 s_calib_sum    = 0.0f;
static uint16  s_calib_count  = 0u;

/* Turn state */
static float32         s_turn_angle   = 0.0f;
static sint64          s_turn_prev_us = 0;
static MPU6050_TurnDir s_turn_dir     = MPU6050_TURN_LEFT;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static sint16 read_gyro_z_raw(void)
{
    uint8 buf[2] = {0u, 0u};
    I2C_readBytes(MPU_ADDR, REG_GYRO_ZOUT_H, buf, 2u);
    return (sint16)((buf[0] << 8) | buf[1]);
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

void MPU6050_init(void)
{
    I2C_init();
    I2C_writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00u);
    I2C_writeReg(MPU_ADDR, REG_SMPLRT_DIV, 0x09u);
    I2C_writeReg(MPU_ADDR, REG_CONFIG,      0x01u);
    I2C_writeReg(MPU_ADDR, REG_GYRO_CONFIG, 0x00u);

    /* Reset calibration state so a fresh calibrateStep sequence starts */
    s_calib_sum   = 0.0f;
    s_calib_count = 0u;
    gyro_z_offset = 0.0f;

    printf("MPU6050: init done — keep robot still for calibration\n");
}

boolean MPU6050_calibrateStep(void)
{
    s_calib_sum += (float32)read_gyro_z_raw() / GYRO_SENSITIVITY;
    s_calib_count++;

    if (s_calib_count >= CALIBRATION_SAMPLES)
    {
        gyro_z_offset = s_calib_sum / (float32)CALIBRATION_SAMPLES;
        s_calib_sum   = 0.0f;
        s_calib_count = 0u;
        printf("MPU6050: ready  offset=%.3f dps\n", gyro_z_offset);
        return TRUE;
    }
    return FALSE;
}

float32 MPU6050_getGyroZ(void)
{
    return ((float32)read_gyro_z_raw() / GYRO_SENSITIVITY) - gyro_z_offset;
}

void MPU6050_turnBegin(MPU6050_TurnDir dir, uint8 speed)
{
    s_turn_angle   = 0.0f;
    s_turn_prev_us = Timer_getTimeUs();
    s_turn_dir     = dir;

    if (dir == MPU6050_TURN_RIGHT)
    {
        Motor_drive(MOTOR_FRONT_LEFT,  MOTOR_FORWARD,  speed);
        Motor_drive(MOTOR_REAR_LEFT,   MOTOR_FORWARD,  speed);
        Motor_drive(MOTOR_FRONT_RIGHT, MOTOR_BACKWARD, speed);
        Motor_drive(MOTOR_REAR_RIGHT,  MOTOR_BACKWARD, speed);
    }
    else
    {
        Motor_drive(MOTOR_FRONT_LEFT,  MOTOR_BACKWARD, speed);
        Motor_drive(MOTOR_REAR_LEFT,   MOTOR_BACKWARD, speed);
        Motor_drive(MOTOR_FRONT_RIGHT, MOTOR_FORWARD,  speed);
        Motor_drive(MOTOR_REAR_RIGHT,  MOTOR_FORWARD,  speed);
    }
}

boolean MPU6050_turnStep(void)
{
    sint64  now_us = Timer_getTimeUs();
    float32 dt     = (float32)(now_us - s_turn_prev_us) / 1000000.0f;
    float32 rate   = MPU6050_getGyroZ();
    s_turn_prev_us = now_us;
    s_turn_angle  += rate * dt;

    float32 target = (s_turn_dir == MPU6050_TURN_LEFT)
                     ? TARGET_ANGLE_LEFT
                     : TARGET_ANGLE_RIGHT;

    printf("[TURN] dir=%c rate=%.1f dps  angle=%.1f / %.1f deg\n",
           (s_turn_dir == MPU6050_TURN_LEFT) ? 'L' : 'R',
           (double)rate, (double)s_turn_angle, (double)target);

    if (fabsf(s_turn_angle) >= target)
    {
        Motor_brakeAll();
        printf("[TURN] brake @ %.1f deg (target %.1f)\n",
               (double)s_turn_angle, (double)target);
        return TRUE;
    }
    return FALSE;
}
