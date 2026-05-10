
#include "fsm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *const k_state_str[] = {
    "IDLE", "WALL_FOLLOW", "WALL_LOST", "STOP", "REALIGN", "TURN_LEFT", "TURN_RIGHT"
};

/* Prints the current FSM state to serial every 500 ms. */
static void monitor_task(void *pv)
{
    (void)pv;
    for (;;)
    {
        printf("[MONITOR] state = %s\n", k_state_str[FSM_getState()]);
        vTaskDelay(pdMS_TO_TICKS(500u));
    }
}

void app_main(void)
{
    printf("\n=== ESP32 Wall-Follow FSM starting ===\n");

    FSM_init();
    xTaskCreate(fsm_task,     "fsm",     8192u, NULL, 5u, NULL);
    xTaskCreate(monitor_task, "monitor", 2048u, NULL, 3u, NULL);
}
