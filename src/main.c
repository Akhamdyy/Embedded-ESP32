
#include "fsm.h"
#include "intr.h"
#include "ultrasonic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

/* Set to 0 to skip the one-shot interrupt-driver validation at boot. */
#define INTR_VALIDATE_AT_BOOT   1

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

#if INTR_VALIDATE_AT_BOOT
/*
 * Validates the manual Intr_install path end-to-end. Runs once from
 * app_main AFTER fsm_task has invoked Ultrasonic_initAll() (which is what
 * actually calls GPIO_enableInterrupt -> Intr_install). Steps:
 *   1. Wait long enough for fsm_task to reach Ultrasonic_initAll.
 *   2. Intr_diagnose — static checks: DPORT map readback, INTENABLE bit,
 *      core ID, BT-controller CPU-int collision scan.
 *   3. Dynamic check — count echo ISR-driven measurements over 3 s. A
 *      non-zero hit count per sensor proves the full chain works:
 *         DPORT matrix -> Xtensa vector -> GPIO_sharedIsr ->
 *         per-pin dispatch -> echo_isr_handler -> Timer_getTimeUs.
 *   4. Print PASS/FAIL. FSM keeps running afterwards.
 *
 * Delete this function and its call site once you trust the refactor.
 */
static void intr_validation_run(void)
{
    printf("\n=== INTR REFACTOR VALIDATION ===\n");

    /* fsm_task is priority 5 and the first thing it does is Ultrasonic_initAll.
     * Give it a comfortable head-start regardless of scheduling jitter. */
    vTaskDelay(pdMS_TO_TICKS(500u));

    Intr_diagnose();

    /* End-to-end ISR liveness check. measurement_timer_cb fires every 60 ms
     * (Ultrasonic_initAll(60u) in fsm.c) and round-robins across 3 sensors,
     * so each sensor's echo ISR should produce ~16 measurements per 3 s. */
    uint32 hits[3] = {0u, 0u, 0u};
    TickType_t end = xTaskGetTickCount() + pdMS_TO_TICKS(3000u);
    while (xTaskGetTickCount() < end)
    {
        for (int s = 0; s < 3; s++)
        {
            if (Ultrasonic_isDataReady((Ultrasonic_SensorID)s))
            {
                hits[s]++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5u));
    }

    printf("[VALIDATE] hits FRONT=%lu LEFT=%lu RIGHT=%lu  (expect ~16 each)\n",
           (unsigned long)hits[0], (unsigned long)hits[1], (unsigned long)hits[2]);
    boolean pass = (hits[0] > 0u) && (hits[1] > 0u) && (hits[2] > 0u);
    printf("[VALIDATE] echo-ISR liveness: %s\n", pass ? "PASS" : "FAIL");
    printf("=== END VALIDATION ===\n\n");
}
#endif

void app_main(void)
{
    printf("\n=== ESP32 Wall-Follow FSM starting ===\n");

    FSM_init();
    xTaskCreate(fsm_task,     "fsm",     8192u, NULL, 5u, NULL);
    xTaskCreate(monitor_task, "monitor", 2048u, NULL, 3u, NULL);

#if INTR_VALIDATE_AT_BOOT
    intr_validation_run();
#endif
}
