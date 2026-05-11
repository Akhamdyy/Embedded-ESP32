#include "fsm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void app_main(void)
{
    printf("\n=== ESP32 Wall-Follow FSM starting ===\n");

    FSM_init();
    xTaskCreate(fsm_task, "fsm", 8192u, NULL, 5u, NULL);
}
