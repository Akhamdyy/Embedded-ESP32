/******************************************************************************
 *
 * Module: MPU-6050 IMU Driver
 *
 * File Name: mpu6050.c
 *
 * Description: Source file for MPU-6050 / MPU-6500 gyroscope driver.
 *              Uses gyro Z-axis integration for 90-degree turn detection.
 *
 *******************************************************************************/

#include "mpu6050.h"
#include "motor.h"
#include "i2c.h"
#include "timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define MPU_ADDR            0x68

/* MPU-6050 / MPU-6500 registers */
#define REG_SMPLRT_DIV      0x19    /* Sample rate divider */
#define REG_CONFIG          0x1A    /* DLPF config */
#define REG_GYRO_CONFIG     0x1B    /* Gyro full-scale range */
#define REG_PWR_MGMT_1      0x6B    /* Power management */
#define REG_GYRO_ZOUT_H     0x47    /* Gyro Z high byte (low byte follows) */

/* ±250 °/s → 131 LSB per °/s */
#define GYRO_SENSITIVITY    131.0f

#define CALIBRATION_SAMPLES 100
#define POLL_INTERVAL_MS    10      /* 100 Hz integration rate */
#define TARGET_ANGLE_DEG    83.0f

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static float32 gyro_z_offset = 0.0f;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static sint16 read_gyro_z_raw(void)
{
    uint8 buf[2] = {0, 0};
    I2C_readBytes(MPU_ADDR, REG_GYRO_ZOUT_H, buf, 2);
    return (sint16)((buf[0] << 8) | buf[1]);
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize I2C bus and configure MPU-6050 / MPU-6500.
 */
void MPU6050_init(void)
{
    I2C_init();

    I2C_writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00);  /* clear sleep bit — wake up */
    I2C_writeReg(MPU_ADDR, REG_SMPLRT_DIV, 0x09);  /* 100 Hz: 1 kHz / (1 + 9) */
    I2C_writeReg(MPU_ADDR, REG_CONFIG,      0x01);  /* DLPF 188 Hz bandwidth */
    I2C_writeReg(MPU_ADDR, REG_GYRO_CONFIG, 0x00);  /* ±250 °/s */
}

/*
 * Description :
 * Calibrate zero-rate offset by averaging samples while robot is stationary.
 */
void MPU6050_calibrate(void)
{
    printf("MPU6050: calibrating — keep robot still...\n");

    float32 sum = 0.0f;
    int i;
    for (i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        sum += (float32)read_gyro_z_raw() / GYRO_SENSITIVITY;
        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
    gyro_z_offset = sum / (float32)CALIBRATION_SAMPLES;

    printf("MPU6050: ready (zero-rate offset = %.3f dps)\n", gyro_z_offset);
}

/*
 * Description :
 * Return calibrated gyro Z rate in degrees per second.
 */
float32 MPU6050_getGyroZ(void)
{
    return ((float32)read_gyro_z_raw() / GYRO_SENSITIVITY) - gyro_z_offset;
}

/*
 * Description :
 * Blocking 90-degree pivot turn using gyro Z integration.
 */
void MPU6050_turn(MPU6050_TurnDir dir, uint8 speed)
{
    float32 angle   = 0.0f;
    sint64  prev_us = Timer_getTimeUs();

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

    while (fabsf(angle) < TARGET_ANGLE_DEG)
    {
        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));

        sint64  now_us = Timer_getTimeUs();
        float32 dt     = (float32)(now_us - prev_us) / 1000000.0f;
        prev_us        = now_us;

        angle += MPU6050_getGyroZ() * dt;
    }

    Motor_stopAll();
}
