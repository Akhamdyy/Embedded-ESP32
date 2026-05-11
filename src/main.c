#include "fsm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    FSM_init();
    xTaskCreate(fsm_task, "fsm", 4096, NULL, 5, NULL);
}
