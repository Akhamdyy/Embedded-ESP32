#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"
#include "ultrasonic.h"

#define BASE_SPEED              200   /* Base motor speed (0-255) */
#define OBSTACLE_DISTANCE       30    /* Stop if anything closer than 30cm */
#define VERY_CLOSE_DISTANCE     15    /* Reverse if too close */

void app_main(void)
{
    printf("=== Motor + Obstacle Avoidance Test ===\n\n");

    Motor_init();
    printf("Motors initialized\n");

    Ultrasonic_initAll(70);
    printf("All 3 ultrasonic sensors initialized\n\n");

    printf("Starting forward movement with obstacle avoidance...\n");
    printf("Place obstacles in front of the car to test.\n\n");

    uint8_t state = 0;  /* 0=moving, 1=obstacle detected, 2=reversing */

    while (1)
    {
        /* Read all sensors */
        uint16 front = Ultrasonic_getDistance(ULTRASONIC_FRONT);
        uint16 left  = Ultrasonic_getDistance(ULTRASONIC_LEFT);
        uint16 right = Ultrasonic_getDistance(ULTRASONIC_RIGHT);

        printf("[Sensors] Front: %3u cm | Left: %3u cm | Right: %3u cm | ",
               front, left, right);

        /* Obstacle detection and avoidance */
        if (front != ULTRASONIC_OUT_OF_RANGE && front < VERY_CLOSE_DISTANCE)
        {
            printf("VERY CLOSE - REVERSING!\n");
            Motor_driveAll(MOTOR_BACKWARD, BASE_SPEED);
            state = 2;
            vTaskDelay(pdMS_TO_TICKS(800));
            continue;
        }
        else if (front != ULTRASONIC_OUT_OF_RANGE && front < OBSTACLE_DISTANCE)
        {
            printf("OBSTACLE AHEAD - STOPPING!\n");
            Motor_stopAll();
            state = 1;
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        else
        {
            printf("MOVING FORWARD\n");
            Motor_driveAll(MOTOR_FORWARD, BASE_SPEED);
            state = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
