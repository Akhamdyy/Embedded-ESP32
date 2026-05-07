#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ultrasonic.h"

void app_main(void)
{
    printf("=== Ultrasonic Sensor Test (New Pins: 34/35/36) ===\n\n");

    Ultrasonic_initAll(70);
    printf("All 3 sensors initialized — Front:GPIO34 Left:GPIO35 Right:GPIO36\n\n");

    while (1)
    {
        uint16 front = Ultrasonic_getDistance(ULTRASONIC_FRONT);
        uint16 left  = Ultrasonic_getDistance(ULTRASONIC_LEFT);
        uint16 right = Ultrasonic_getDistance(ULTRASONIC_RIGHT);

        char fs[8], ls[8], rs[8];
        snprintf(fs, sizeof(fs), front == ULTRASONIC_OUT_OF_RANGE ? "OOR" : "%u cm", front);
        snprintf(ls, sizeof(ls), left  == ULTRASONIC_OUT_OF_RANGE ? "OOR" : "%u cm", left);
        snprintf(rs, sizeof(rs), right == ULTRASONIC_OUT_OF_RANGE ? "OOR" : "%u cm", right);
        printf("Front: %6s | Left: %6s | Right: %6s\n", fs, ls, rs);

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
