#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mpu6050.h"

void app_main(void)
{
    printf("=== MPU-6050 Gyro Z Test ===\n\n");

    MPU6050_init();
    MPU6050_calibrate();

    printf("Rotate the robot and watch gyro Z change.\n");
    printf("Still = near 0.00 dps  |  Spinning = non-zero dps\n\n");

    while (1)
    {
        float gz = MPU6050_getGyroZ();
        printf("Gyro Z: %7.2f dps\n", gz);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
