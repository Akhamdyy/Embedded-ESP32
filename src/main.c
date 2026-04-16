#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BLINK_GPIO 2  // Most ESP32 DevKits have an LED on GPIO 2

void app_main(void) {
    // 1. Configure the peripheral (GPIO)
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    printf("Starting ESP-IDF Blink (Pure C)\n");

    while (1) {
        // 2. Set the GPIO level to 1 (High)
        gpio_set_level(BLINK_GPIO, 1);
        printf("LED ON\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second

        // 3. Set the GPIO level to 0 (Low)
        gpio_set_level(BLINK_GPIO, 0);
        printf("LED OFF\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}