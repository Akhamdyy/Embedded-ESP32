/******************************************************************************
 *
 * Module: Ultrasonic Sensor
 *
 * File Name: ultrasonic.c
 *
 * Description: Source file for the HC-SR04 ultrasonic distance sensor driver.
 *              Fully timer and interrupt driven - no blocking delays.
 *
 *******************************************************************************/

#include "ultrasonic.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"
#include <stdio.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define TRIG_GPIO           26
#define ECHO_GPIO           27

/* Trigger pulse width per HC-SR04 datasheet */
#define TRIG_PULSE_US       10

/* Echo timeout: 400cm max range = ~23ms round trip, 30ms gives margin */
#define ECHO_TIMEOUT_US     30000

/* Sound speed formula: duration(us) / 58 = distance(cm) */
#define SOUND_DIVISOR       58

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static esp_timer_handle_t trig_pulse_timer;    /* One-shot: ends the trigger pulse */
static esp_timer_handle_t measurement_timer;   /* Periodic: starts each measurement */

static volatile int64_t  echo_start_us  = 0;
static volatile int64_t  echo_end_us    = 0;
static volatile uint16_t last_distance  = ULTRASONIC_OUT_OF_RANGE;
static volatile boolean  data_ready     = FALSE;
static volatile boolean  echo_active    = FALSE;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

/*
 * One-shot timer callback — fires 10µs after trigger went HIGH.
 * Pulls TRIG LOW to complete the trigger pulse.
 */
static void trig_pulse_timer_cb(void *arg)
{
    gpio_set_level(TRIG_GPIO, 0);
}

/*
 * Periodic timer callback — fires every measurement_interval_ms.
 * Logs the last distance then starts a new measurement.
 */
static void measurement_timer_cb(void *arg)
{
    /* Log previous result */
    if (last_distance == ULTRASONIC_OUT_OF_RANGE)
    {
        printf("[Ultrasonic] Distance: OUT OF RANGE\n");
    }
    else
    {
        printf("[Ultrasonic] Distance: %u cm\n", last_distance);
    }

    /* Reset state for new measurement */
    echo_active   = FALSE;
    data_ready    = FALSE;
    echo_start_us = 0;
    echo_end_us   = 0;

    /* Start trigger pulse: pull HIGH then start one-shot timer to pull LOW */
    gpio_set_level(TRIG_GPIO, 1);
    esp_timer_start_once(trig_pulse_timer, TRIG_PULSE_US);
}

/*
 * GPIO ISR — called on both rising and falling edges of the Echo pin.
 * Rising edge : record start time.
 * Falling edge: record end time, compute and store distance.
 */
static void IRAM_ATTR echo_isr_handler(void *arg)
{
    if (gpio_get_level(ECHO_GPIO) == 1)
    {
        /* Rising edge: echo pulse started */
        echo_start_us = esp_timer_get_time();
        echo_active   = TRUE;
    }
    else
    {
        /* Falling edge: echo pulse ended */
        if (echo_active)
        {
            echo_end_us = esp_timer_get_time();
            int64_t duration_us = echo_end_us - echo_start_us;

            if (duration_us > 0 && duration_us < ECHO_TIMEOUT_US)
            {
                last_distance = (uint16_t)(duration_us / SOUND_DIVISOR);
            }
            else
            {
                last_distance = ULTRASONIC_OUT_OF_RANGE;
            }

            data_ready  = TRUE;
            echo_active = FALSE;
        }
    }
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize the ultrasonic sensor GPIO pins, interrupts, and timers.
 */
void Ultrasonic_init(uint32 measurement_interval_ms)
{
    /* --- Trigger pin: output, start LOW --- */
    gpio_config_t trig_cfg = {
        .pin_bit_mask = (1ULL << TRIG_GPIO),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&trig_cfg);
    gpio_set_level(TRIG_GPIO, 0);

    /* --- Echo pin: input, interrupt on both edges --- */
    gpio_config_t echo_cfg = {
        .pin_bit_mask = (1ULL << ECHO_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&echo_cfg);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(ECHO_GPIO, echo_isr_handler, NULL);

    /* --- One-shot timer: ends the 10µs trigger pulse --- */
    const esp_timer_create_args_t trig_timer_args = {
        .callback = trig_pulse_timer_cb,
        .arg      = NULL,
        .name     = "ultrasonic_trig",
    };
    esp_timer_create(&trig_timer_args, &trig_pulse_timer);

    /* --- Periodic timer: triggers measurements and logs results --- */
    const esp_timer_create_args_t meas_timer_args = {
        .callback = measurement_timer_cb,
        .arg      = NULL,
        .name     = "ultrasonic_meas",
    };
    esp_timer_create(&meas_timer_args, &measurement_timer);
    esp_timer_start_periodic(measurement_timer,
                             (uint64_t)measurement_interval_ms * 1000ULL);
}

/*
 * Description :
 * Return the last successfully measured distance in centimeters.
 */
uint16 Ultrasonic_getDistance(void)
{
    return last_distance;
}

/*
 * Description :
 * Return TRUE if a new measurement has completed since the last call.
 */
boolean Ultrasonic_isDataReady(void)
{
    if (data_ready)
    {
        data_ready = FALSE;
        return TRUE;
    }
    return FALSE;
}
