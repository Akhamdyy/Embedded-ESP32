/******************************************************************************
 *
 * Application: Robot Car — Basic Obstacle Test (no MPU6050)
 *
 * File Name: main.c
 *
 * Description: Simple obstacle avoidance using front ultrasonic sensor only.
 *              No MPU6050 — turns are time-based.
 *
 *              Behaviour:
 *                1. Drive forward
 *                2. Obstacle detected → reverse for REVERSE_MS
 *                3. Turn right for TURN_MS
 *                4. Resume forward
 *
 *******************************************************************************/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"
#include "ultrasonic.h"

/*******************************************************************************
 *                         Tuning Parameters                                   *
 *******************************************************************************/

#define OBSTACLE_CM         25u     /* Front sensor: trigger avoidance (cm)    */

#define DRIVE_SPEED         155u    /* Forward speed (0-255)                   */
#define REVERSE_SPEED       140u    /* Reverse speed                           */
#define TURN_SPEED          140u    /* Speed during right turn                 */

#define SENSOR_INTERVAL_MS  50u     /* Ultrasonic fire interval (ms)           */
#define SENSOR_SETTLE_MS    500u    /* Wait for first readings after boot      */
#define REVERSE_MS          600u    /* How long to reverse (ms)                */
#define TURN_MS             550u    /* How long to turn right (ms)             */
#define LOOP_DELAY_MS       50u     /* Main loop period (ms)                   */

/*******************************************************************************
 *                         Application Entry Point                             *
 *******************************************************************************/

void app_main(void)
{
    uint16_t front;

    /* --- Init --- */
    Motor_init();
    Motor_stopAll();

    Ultrasonic_initAll(SENSOR_INTERVAL_MS);

    /* Wait for first sensor readings */
    vTaskDelay(pdMS_TO_TICKS(SENSOR_SETTLE_MS));

    printf("Basic obstacle test started.\n\n");

    /* --- Main Loop --- */
    while (1)
    {
        front = Ultrasonic_getDistance(ULTRASONIC_FRONT);

        /* Treat out-of-range as clear (no obstacle) */
        if (front == ULTRASONIC_OUT_OF_RANGE)
            front = 400u;

        if (front <= OBSTACLE_CM)
        {
            /* --- Obstacle detected --- */
            printf("Obstacle %u cm — reversing then turning right\n", front);

            /* Step 1: reverse */
            Car_moveBackward(REVERSE_SPEED, REVERSE_SPEED);
            vTaskDelay(pdMS_TO_TICKS(REVERSE_MS));

            /* Step 2: turn right in place */
            Car_turnRight(DRIVE_SPEED, 0);
            vTaskDelay(pdMS_TO_TICKS(TURN_MS));

            /* Step 3: stop briefly before resuming */
            Motor_brakeAll();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
        {
            /* --- Clear path — drive forward --- */
            Car_moveForward(DRIVE_SPEED, DRIVE_SPEED);
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
