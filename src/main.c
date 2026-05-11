/******************************************************************************
 *
 * Application: Autonomous Wall-Following Robot
 *
 * File Name: main.c
 *
 * Description: Entry point. Delegates entirely to the FSM module.
 *
 *******************************************************************************/

#include "fsm.h"

void app_main(void)
{
    FSM_init();
    FSM_run();   /* never returns */
}
