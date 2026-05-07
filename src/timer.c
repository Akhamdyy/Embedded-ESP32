/******************************************************************************
 *
 * Module: Timer
 *
 * File Name: timer.c
 *
 * Description: Source file for the ESP32 esp_timer-based software timer
 *              driver (MCAL layer). Wraps ESP-IDF esp_timer so that higher-
 *              level HAL drivers have no direct ESP-IDF dependency.
 *
 *******************************************************************************/

#include "timer.h"
#include "esp_timer.h"
#include "esp_attr.h"

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Create a one-shot software timer.
 */
void Timer_createOneShot(const char *name, Timer_CallbackType callback,
                         void *arg, Timer_HandleType *out)
{
    const esp_timer_create_args_t args = {
        .callback        = callback,
        .arg             = arg,
        .name            = name,
        .dispatch_method = ESP_TIMER_TASK,
    };
    esp_timer_create(&args, (esp_timer_handle_t *)out);
}

/*
 * Description :
 * Create a periodic software timer.
 */
void Timer_createPeriodic(const char *name, Timer_CallbackType callback,
                          void *arg, Timer_HandleType *out)
{
    const esp_timer_create_args_t args = {
        .callback        = callback,
        .arg             = arg,
        .name            = name,
        .dispatch_method = ESP_TIMER_TASK,
    };
    esp_timer_create(&args, (esp_timer_handle_t *)out);
}

/*
 * Description :
 * Start a one-shot timer to fire after delay_us microseconds.
 */
void Timer_startOnce(Timer_HandleType handle, uint32 delay_us)
{
    esp_timer_start_once((esp_timer_handle_t)handle, (uint64_t)delay_us);
}

/*
 * Description :
 * Start a periodic timer to fire every period_us microseconds.
 */
void Timer_startPeriodic(Timer_HandleType handle, uint32 period_us)
{
    esp_timer_start_periodic((esp_timer_handle_t)handle, (uint64_t)period_us);
}

/*
 * Description :
 * Return time since boot in microseconds. ISR-safe (placed in IRAM).
 */
IRAM_ATTR sint64 Timer_getTimeUs(void)
{
    return (sint64)esp_timer_get_time();
}
