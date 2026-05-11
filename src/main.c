#include "fsm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *const k_state_str[] = {
    "IDLE", "WALL_FOLLOW", "WALL_LOST", "STOP",
    "TURN_LEFT", "TURN_RIGHT", "POST_TURN"
};

/* Prints the current FSM state to serial every 500 ms. */
static void monitor_task(void *pv)
{
    TickType_t ticks = xTaskGetTickCount();
    uint32 total_s   = (uint32)((ticks * portTICK_PERIOD_MS) / 1000u);
    uint32 h         = total_s / 3600u;
    uint32 m         = (total_s % 3600u) / 60u;
    uint32 s         = total_s % 60u;
    snprintf(buf, 12, "%02u:%02u:%02u", (unsigned)h, (unsigned)m, (unsigned)s);
}

void app_main(void)
{
    bluetooth_init("wallrobot");

    printf("[MAIN] Bluetooth SPP test — device visible as 'wallrobot'\n");

    uint8  rx_buf[64];
    uint32 counter = 0u;
    char   ts[12];

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));

        counter++;
        uptime_str(ts);

        /* Heartbeat — only when phone is connected */
        if (bluetooth_is_connected())
        {
            char msg[64];
            snprintf(msg, sizeof(msg), "[%s] heartbeat #%u\r\n", ts, (unsigned)counter);
            bluetooth_send(msg);
            printf("[TX] %s", msg);
        }

        /* Echo received data with a timestamp prefix */
        uint16 n = bluetooth_recv(rx_buf, (uint16)(sizeof(rx_buf) - 1u));
        if (n > 0u)
        {
            rx_buf[n] = '\0';
            uptime_str(ts);
            printf("[RX] %s  %u bytes: %s\n", ts, (unsigned)n, (char *)rx_buf);

            char echo[96];
            snprintf(echo, sizeof(echo), "[%s] ECHO: %s\r\n", ts, (char *)rx_buf);
            bluetooth_send(echo);
        }
    }

}
