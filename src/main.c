/******************************************************************************
 *
 * Application: PID Lane Centering Test
 *
 * File Name: main.c
 *
 * Description: Tests the PID lane-centering controller.
 *              Car drives forward and corrects its position using left/right
 *              ultrasonic sensors. Correction activates when either side
 *              drops below THRESHOLD_CM.
 *
 *******************************************************************************/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"
#include "ultrasonic.h"
#include "pid.h"

/*******************************************************************************
 *                         Tuning Parameters                                   *
 *******************************************************************************/

#define BASE_SPEED          80u    /* Forward speed when centered (0-255)    */
#define THRESHOLD_CM        12u     /* Correction activates below this (cm)   */

#define PID_KP              2.0f    /* Proportional gain — tune first         */
#define PID_KI              0.0f    /* Integral gain    — add after P is good */
#define PID_KD              2.0f    /* Derivative gain  — reduces overshoot   */

#define SENSOR_INTERVAL_MS  50u     /* Ultrasonic fire interval per sensor    */
#define LOOP_INTERVAL_MS    50u     /* PID update interval (dt = 0.05 s)      */
#define SENSOR_SETTLE_MS    500u    /* Wait for first readings after boot     */

#define LOOP_DT_S           (LOOP_INTERVAL_MS / 1000.0f)

/*******************************************************************************
 *                         Application Entry Point                             *
 *******************************************************************************/

void app_main(void)
{
    Motor_init();
    Motor_stopAll();

    Ultrasonic_initAll(SENSOR_INTERVAL_MS);
    vTaskDelay(pdMS_TO_TICKS(SENSOR_SETTLE_MS));

    const PID_Config cfg = {
        .kp           = PID_KP,
        .ki           = PID_KI,
        .kd           = PID_KD,
        .base_speed   = BASE_SPEED,
        .threshold_cm = THRESHOLD_CM,
    };
    PID_init(&cfg);

    printf("PID lane centering started.\n\n");

    while (1)
    {
        PID_update(LOOP_DT_S);
        vTaskDelay(pdMS_TO_TICKS(LOOP_INTERVAL_MS));
    }
}
