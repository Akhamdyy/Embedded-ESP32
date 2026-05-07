/******************************************************************************
 *
 * Module: MPU-6050 IMU Driver
 *
 * File Name: mpu6050.c
 *
 * Description: Source file for MPU-6050 gyroscope driver.
 *              Uses gyro Z-axis integration for 90-degree turn detection.
 *
 *******************************************************************************/

#include "mpu6050.h"
#include "motor.h"
#include "driver/i2c.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define I2C_PORT            I2C_NUM_0
#define I2C_SDA_GPIO        27
#define I2C_SCL_GPIO        33
#define I2C_FREQ_HZ         400000

#define MPU_ADDR            0x68

/* MPU-6050 registers */
#define REG_SMPLRT_DIV      0x19    /* Sample rate divider */
#define REG_CONFIG          0x1A    /* DLPF config */
#define REG_GYRO_CONFIG     0x1B    /* Gyro full-scale range */
#define REG_PWR_MGMT_1      0x6B    /* Power management */
#define REG_GYRO_ZOUT_H     0x47    /* Gyro Z high byte (low byte follows) */

/* ±250 °/s → 131 LSB per °/s */
#define GYRO_SENSITIVITY    131.0f

#define CALIBRATION_SAMPLES 100
#define POLL_INTERVAL_MS    10      /* 100 Hz integration rate */
#define TARGET_ANGLE_DEG    90.0f

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static float32 gyro_z_offset = 0.0f;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static esp_err_t write_reg(uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static int16_t read_gyro_z_raw(void)
{
    uint8_t buf[2] = {0, 0};

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, REG_GYRO_ZOUT_H, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    return (int16_t)((buf[0] << 8) | buf[1]);
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize I2C bus and configure MPU-6050.
 */
void MPU6050_init(void)
{
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_SDA_GPIO,
        .scl_io_num       = I2C_SCL_GPIO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    write_reg(REG_PWR_MGMT_1,  0x00);  /* clear sleep bit — wake up */
    write_reg(REG_SMPLRT_DIV,  0x09);  /* 100 Hz: 1 kHz / (1 + 9) */
    write_reg(REG_CONFIG,       0x01);  /* DLPF 188 Hz bandwidth */
    write_reg(REG_GYRO_CONFIG,  0x00);  /* ±250 °/s */
}

/*
 * Description :
 * Calibrate zero-rate offset by averaging samples while robot is stationary.
 */
void MPU6050_calibrate(void)
{
    printf("MPU6050: calibrating — keep robot still...\n");

    float32 sum = 0.0f;
    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
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
    float32 angle  = 0.0f;
    int64_t prev_us = esp_timer_get_time();

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

        int64_t now_us = esp_timer_get_time();
        float32 dt     = (float32)(now_us - prev_us) / 1000000.0f;
        prev_us        = now_us;

        angle += MPU6050_getGyroZ() * dt;
    }

    Motor_stopAll();
}
