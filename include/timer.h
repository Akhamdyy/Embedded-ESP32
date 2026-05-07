/******************************************************************************
 *
 * Module: Timer
 *
 * File Name: timer.h
 *
 * Description: Header file for the ESP32 esp_timer-based software timer
 *              driver (MCAL layer). Provides a hardware-agnostic API for
 *              one-shot and periodic callbacks. All esp_timer details are
 *              confined to timer.c.
 *
 *******************************************************************************/

#ifndef TIMER_H_
#define TIMER_H_

#include "std_types.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

/* Callback signature: matches esp_timer internally */
typedef void (*Timer_CallbackType)(void *arg);

/* Opaque handle — internally an esp_timer_handle_t (pointer type) */
typedef void *Timer_HandleType;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Create a one-shot software timer. The handle is written to *out.
 * Start the timer with Timer_startOnce after creation.
 * name: short label used by the esp_timer profiler.
 */
void Timer_createOneShot(const char *name, Timer_CallbackType callback,
                         void *arg, Timer_HandleType *out);

/*
 * Description :
 * Create a periodic software timer. The handle is written to *out.
 * Start the timer with Timer_startPeriodic after creation.
 */
void Timer_createPeriodic(const char *name, Timer_CallbackType callback,
                          void *arg, Timer_HandleType *out);

/*
 * Description :
 * Start a one-shot timer. Fires callback once after delay_us microseconds.
 * If the timer is already running it is restarted.
 */
void Timer_startOnce(Timer_HandleType handle, uint32 delay_us);

/*
 * Description :
 * Start a periodic timer. Fires callback every period_us microseconds.
 */
void Timer_startPeriodic(Timer_HandleType handle, uint32 period_us);

/*
 * Description :
 * Return the time in microseconds since boot.
 * Safe to call from ISR context (placed in IRAM).
 */
sint64 Timer_getTimeUs(void);

#endif /* TIMER_H_ */
