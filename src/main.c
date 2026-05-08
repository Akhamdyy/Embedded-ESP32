/******************************************************************************
 *
 * Application: MPU-6050 Turn Test
 *
 * File Name: main.c
 *
 * Description: Tests MPU6050_turn() — gyro-guided 90-degree pivot turns.
 *
 *              Sequence (repeated forever):
 *                1. Calibrate (keep robot still)
 *                2. 3-second countdown — place robot on floor
 *                3. Turn RIGHT 90°  → stop 1 s
 *                4. Turn LEFT  90°  → stop 1 s
 *
 *******************************************************************************/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"
#include "mpu6050.h"

#define TURN_SPEED      90u     /* 0-255 — lower = more accurate, slower */

void app_main(void)
{
    Motor_init();
    Motor_stopAll();

    MPU6050_init();
    MPU6050_calibrate();

    printf("\nPlacing robot on floor — turning in 3 s...\n");
    vTaskDelay(pdMS_TO_TICKS(3000));

    while (1)
    {
        printf("Turning RIGHT 90 degrees\n");
        MPU6050_turn(MPU6050_TURN_RIGHT, TURN_SPEED);
        printf("Done\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Turning LEFT 90 degrees\n");
        MPU6050_turn(MPU6050_TURN_LEFT, TURN_SPEED);
        printf("Done\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
