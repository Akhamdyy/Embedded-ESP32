#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor.h"
#include "ultrasonic.h"

/* Measure and log distance every 100ms */
#define ULTRASONIC_INTERVAL_MS  100

void app_main(void)
{
    Motor_init();
    Ultrasonic_init(ULTRASONIC_INTERVAL_MS);

    printf("System initialized\n");

    /* All logic is timer/interrupt driven - nothing to do in the main loop */
    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}
