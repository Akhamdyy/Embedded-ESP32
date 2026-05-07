/******************************************************************************
 *
 * Module: Ultrasonic Sensor (Multi-Sensor)
 *
 * File Name: ultrasonic.c
 *
 * Description: Source file for HC-SR04 ultrasonic distance sensor driver.
 *              Supports 3 independent sensors with staggered firing.
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

/* Trigger pulse width per HC-SR04 datasheet */
#define TRIG_PULSE_US       10

/* Echo timeout: 400cm max range = ~23ms round trip, 30ms gives margin */
#define ECHO_TIMEOUT_US     30000

/* Sound speed formula: duration(us) / 58 = distance(cm) */
#define SOUND_DIVISOR       58

/*******************************************************************************
 *                         Private Type Definitions                            *
 *******************************************************************************/

typedef struct
{
    int         trig_gpio;
    int         echo_gpio;
    const char *name;
} Ultrasonic_Config;

typedef struct
{
    volatile int64_t   echo_start_us;
    volatile boolean   echo_active;
    volatile uint16_t  last_distance;
    volatile boolean   data_ready;
    esp_timer_handle_t trig_pulse_timer;
} Ultrasonic_State;

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static const Ultrasonic_Config sensor_cfg[ULTRASONIC_COUNT] = {
    /* FRONT */ { 26, 34, "FRONT" },
    /* LEFT  */ { 32, 35, "LEFT"  },
    /* RIGHT */ {  4, 36, "RIGHT" },
};

static Ultrasonic_State sensor_state[ULTRASONIC_COUNT];

static esp_timer_handle_t measurement_timer;   /* Periodic: triggers measurements */

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

/*
 * One-shot timer callback — fires 10µs after trigger went HIGH.
 * Pulls TRIG LOW to complete the trigger pulse for the specified sensor.
 */
static void trig_pulse_timer_cb(void *arg)
{
    uint32_t idx = (uint32_t)arg;
    gpio_set_level(sensor_cfg[idx].trig_gpio, 0);
}

/*
 * Periodic timer callback — fires every measurement_interval_ms.
 * Fires one sensor per tick in round-robin fashion.
 */
static void measurement_timer_cb(void *arg)
{
    static uint32_t current = 0;

    /* Log previous result for this slot */
    uint16_t d = sensor_state[current].last_distance;
    if (d == ULTRASONIC_OUT_OF_RANGE)
        printf("[%s] OOR\n", sensor_cfg[current].name);
    else
        printf("[%s] %u cm\n", sensor_cfg[current].name, d);

    /* Start trigger pulse: pull HIGH then start one-shot timer to pull LOW */
    gpio_set_level(sensor_cfg[current].trig_gpio, 1);
    esp_timer_start_once(sensor_state[current].trig_pulse_timer, TRIG_PULSE_US);

    /* Advance to next sensor */
    current = (current + 1) % ULTRASONIC_COUNT;
}

/*
 * GPIO ISR — called on both rising and falling edges of any Echo pin.
 * The sensor index is passed as arg to identify which sensor fired.
 * Rising edge : record start time.
 * Falling edge: record end time, compute and store distance.
 */
static void IRAM_ATTR echo_isr_handler(void *arg)
{
    uint32_t idx = (uint32_t)arg;
    Ultrasonic_State *s = &sensor_state[idx];

    if (gpio_get_level(sensor_cfg[idx].echo_gpio) == 1)
    {
        /* Rising edge: echo pulse started */
        s->echo_start_us = esp_timer_get_time();
        s->echo_active   = TRUE;
    }
    else
    {
        /* Falling edge: echo pulse ended */
        if (s->echo_active)
        {
            int64_t duration_us = esp_timer_get_time() - s->echo_start_us;

            if (duration_us > 0 && duration_us < ECHO_TIMEOUT_US)
            {
                s->last_distance = (uint16_t)(duration_us / SOUND_DIVISOR);
            }
            else
            {
                s->last_distance = ULTRASONIC_OUT_OF_RANGE;
            }

            s->data_ready  = TRUE;
            s->echo_active = FALSE;
        }
    }
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize all 3 ultrasonic sensors (Front, Left, Right).
 */
void Ultrasonic_initAll(uint32 measurement_interval_ms)
{
    for (uint32_t i = 0; i < ULTRASONIC_COUNT; i++)
    {
        /* Initialize state */
        sensor_state[i].echo_start_us  = 0;
        sensor_state[i].echo_active    = FALSE;
        sensor_state[i].last_distance  = ULTRASONIC_OUT_OF_RANGE;
        sensor_state[i].data_ready     = FALSE;

        /* --- Trigger pin: output, start LOW --- */
        gpio_config_t trig_cfg = {
            .pin_bit_mask = (1ULL << sensor_cfg[i].trig_gpio),
            .mode         = GPIO_MODE_OUTPUT,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&trig_cfg);
        gpio_set_level(sensor_cfg[i].trig_gpio, 0);

        /* --- Echo pin: input, interrupt on both edges --- */
        /* GPIO 34/35/36 are input-only and have no internal pull resistors.
           The voltage divider (2kΩ to GND) provides the passive pull-down. */
        gpio_config_t echo_cfg = {
            .pin_bit_mask = (1ULL << sensor_cfg[i].echo_gpio),
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_ANYEDGE,
        };
        gpio_config(&echo_cfg);

        gpio_install_isr_service(0);
        gpio_isr_handler_add(sensor_cfg[i].echo_gpio, echo_isr_handler, (void *)i);

        /* --- One-shot timer: ends the 10µs trigger pulse for this sensor --- */
        char timer_name[20];
        snprintf(timer_name, sizeof(timer_name), "us_trig_%s", sensor_cfg[i].name);
        const esp_timer_create_args_t trig_timer_args = {
            .callback = trig_pulse_timer_cb,
            .arg      = (void *)i,
            .name     = timer_name,
        };
        esp_timer_create(&trig_timer_args, &sensor_state[i].trig_pulse_timer);
    }

    /* --- Periodic timer: triggers measurements in round-robin fashion --- */
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
 * Return the last successfully measured distance for the specified sensor (cm).
 */
uint16 Ultrasonic_getDistance(Ultrasonic_SensorID sensor)
{
    if (sensor >= ULTRASONIC_COUNT)
        return ULTRASONIC_OUT_OF_RANGE;

    return sensor_state[sensor].last_distance;
}

/*
 * Description :
 * Return TRUE if a new measurement has completed for the specified sensor.
 */
boolean Ultrasonic_isDataReady(Ultrasonic_SensorID sensor)
{
    if (sensor >= ULTRASONIC_COUNT)
        return FALSE;

    if (sensor_state[sensor].data_ready)
    {
        sensor_state[sensor].data_ready = FALSE;
        return TRUE;
    }
    return FALSE;
}
