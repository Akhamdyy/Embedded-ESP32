/******************************************************************************
 *
 * Module: Ultrasonic Sensor (Multi-Sensor)
 *
 * File Name: ultrasonic.c
 *
 * Description: Source file for HC-SR04 ultrasonic distance sensor driver
 *              (HAL layer). Supports 3 independent sensors with staggered
 *              firing. Fully timer and interrupt driven - no blocking delays.
 *
 *              MCAL dependencies:
 *                - gpio.h  : trigger output, echo input, echo ISR registration
 *                - timer.h : one-shot trigger pulse timer, periodic measurement
 *                            timer, ISR timestamp source
 *
 * Pin mapping (matched to project pin layout):
 *
 *   Sensor  | TRIG              | ECHO
 *   --------|-------------------|-----------------------------
 *   FRONT   | GPIO26 PORTC/PIN0 | GPIO34 PORTC/PIN4 (input-only)
 *   LEFT    | GPIO32 PORTC/PIN2 | GPIO35 PORTC/PIN5 (input-only)
 *   RIGHT   | GPIO4  PORTA/PIN2 | GPIO36 PORTC/PIN6 (input-only)
 *
 *   Echo pins GPIO34-36 are input-only (no internal pull resistors).
 *   Use a 1kΩ/2kΩ voltage divider on each echo line (5V → 3.3V).
 *
 *******************************************************************************/

#include "ultrasonic.h"
#include "gpio.h"
#include "timer.h"
#include "platform.h"
#include <stdio.h>
#include <stdint.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

/* Trigger pulse width per HC-SR04 datasheet */
#define TRIG_PULSE_US       10U

/* Echo timeout: 400 cm max range ≈ 23 ms round trip; 30 ms gives margin */
#define ECHO_TIMEOUT_US     30000U

/* Distance formula: duration(µs) / 58 = distance(cm) */
#define SOUND_DIVISOR       58

/*******************************************************************************
 *                         Private Type Definitions                            *
 *******************************************************************************/

typedef struct
{
    uint8       trig_port;
    uint8       trig_pin;
    uint8       echo_port;
    uint8       echo_pin;
    const char *name;
} Ultrasonic_PinConfig;

typedef struct
{
    volatile sint64  echo_start_us;
    volatile boolean echo_active;
    volatile uint16  last_distance;
    volatile boolean data_ready;
    Timer_HandleType trig_pulse_timer;
} Ultrasonic_State;

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/*
 * Pin assignments — must match project pin layout doc.
 * ECHO pins are GPIO34/35/36 (PORTC PIN4/5/6, input-only, no internal pull).
 */
static const Ultrasonic_PinConfig sensor_cfg[ULTRASONIC_COUNT] = {
    /* FRONT */ { PORTC_ID, PIN0_ID, PORTC_ID, PIN4_ID, "FRONT" },
    /* LEFT  */ { PORTC_ID, PIN2_ID, PORTC_ID, PIN5_ID, "LEFT"  },
    /* RIGHT */ { PORTA_ID, PIN2_ID, PORTC_ID, PIN6_ID, "RIGHT" },
};

static Ultrasonic_State    sensor_state[ULTRASONIC_COUNT];
static Timer_HandleType    measurement_timer;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

/*
 * One-shot timer callback — fires 10 µs after TRIG went HIGH.
 * Pulls TRIG LOW to complete the trigger pulse for this sensor.
 */
static void trig_pulse_timer_cb(void *arg)
{
    uint32_t idx = (uint32_t)(uintptr_t)arg;
    GPIO_writePin(sensor_cfg[idx].trig_port, sensor_cfg[idx].trig_pin, LOGIC_LOW);
}

/*
 * Round-robin pattern.  FRONT gets priority — fired twice per pattern cycle so
 * the front distance refreshes at 2× the rate of the sides.  This matters for
 * collision avoidance while approaching corners.
 *
 *   slot 0  → FRONT
 *   slot 1  → LEFT
 *   slot 2  → FRONT
 *   slot 3  → RIGHT
 *
 * With a 70 ms slot the front refreshes every 140 ms, sides every 280 ms.
 * Each sensor still has ≥140 ms gap → safely above the HC-SR04 60 ms minimum.
 */
static const uint8_t round_robin_pattern[] = {
    (uint8_t)ULTRASONIC_FRONT,
    (uint8_t)ULTRASONIC_LEFT,
    (uint8_t)ULTRASONIC_FRONT,
    (uint8_t)ULTRASONIC_RIGHT,
};
#define ROUND_ROBIN_LEN  (sizeof(round_robin_pattern) / sizeof(round_robin_pattern[0]))

/*
 * Periodic timer callback — fires every measurement_interval_ms.
 * Triggers one sensor per tick following the round-robin pattern above.
 */
static void measurement_timer_cb(void *arg)
{
    (void)arg;
    static uint32_t step = 0;
    uint32_t current = (uint32_t)round_robin_pattern[step];
    step = (step + 1U) % ROUND_ROBIN_LEN;

    /* Log previous result for this slot */
    uint16 d = sensor_state[current].last_distance;
    if (d == ULTRASONIC_OUT_OF_RANGE)
        printf("[%s] OOR\n", sensor_cfg[current].name);
    else
        printf("[%s] %u cm\n", sensor_cfg[current].name, (unsigned int)d);

    /* Start trigger pulse: HIGH → one-shot timer will pull it LOW after 10 µs */
    GPIO_writePin(sensor_cfg[current].trig_port, sensor_cfg[current].trig_pin, LOGIC_HIGH);
    Timer_startOnce(sensor_state[current].trig_pulse_timer, TRIG_PULSE_US);
}

/*
 * GPIO ISR — called on both edges of each Echo pin.
 * Rising  : record start timestamp.
 * Falling : compute duration, convert to cm, store result.
 */
static ISR_ATTR void echo_isr_handler(void *arg)
{
    uint32_t idx = (uint32_t)(uintptr_t)arg;
    Ultrasonic_State *s = &sensor_state[idx];

    if (GPIO_readPin(sensor_cfg[idx].echo_port, sensor_cfg[idx].echo_pin) == LOGIC_HIGH)
    {
        s->echo_start_us = Timer_getTimeUs();
        s->echo_active   = TRUE;
    }
    else
    {
        if (s->echo_active)
        {
            sint64 duration_us = Timer_getTimeUs() - s->echo_start_us;

            if (duration_us > 0 && duration_us < (sint64)ECHO_TIMEOUT_US)
            {
                s->last_distance = (uint16)(duration_us / SOUND_DIVISOR);
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
    uint32_t i;
    char timer_name[20];

    /* Create all one-shot timers first — this initialises TIMG1 via prv_timgInit()
     * so Timer_getTimeUs() is ready before any echo ISR can fire. */
    for (i = 0; i < (uint32_t)ULTRASONIC_COUNT; i++)
    {
        sensor_state[i].echo_start_us = 0;
        sensor_state[i].echo_active   = FALSE;
        sensor_state[i].last_distance = ULTRASONIC_OUT_OF_RANGE;
        sensor_state[i].data_ready    = FALSE;

        snprintf(timer_name, sizeof(timer_name), "us_trig_%s", sensor_cfg[i].name);
        Timer_createOneShot(timer_name, trig_pulse_timer_cb,
                            (void *)(uintptr_t)i, &sensor_state[i].trig_pulse_timer);
    }

    /* TIMG1 is now live. Configure GPIO and register echo ISRs. */
    for (i = 0; i < (uint32_t)ULTRASONIC_COUNT; i++)
    {
        /* TRIG pin: output, start LOW */
        GPIO_setupPinDirection(sensor_cfg[i].trig_port, sensor_cfg[i].trig_pin, PIN_OUTPUT);
        GPIO_writePin(sensor_cfg[i].trig_port, sensor_cfg[i].trig_pin, LOGIC_LOW);

        /* ECHO pin: input, both-edge ISR
         * GPIO34-36 have no internal pull resistors; external voltage divider
         * holds the line LOW when idle — no pull configuration needed. */
        GPIO_setupPinDirection(sensor_cfg[i].echo_port, sensor_cfg[i].echo_pin, PIN_INPUT);
        GPIO_enableInterrupt(sensor_cfg[i].echo_port, sensor_cfg[i].echo_pin,
                             GPIO_INTR_ANY_EDGE, echo_isr_handler, (void *)(uintptr_t)i);
    }

    /* Periodic timer: triggers measurements in round-robin */
    Timer_createPeriodic("ultrasonic_meas", measurement_timer_cb, NULL, &measurement_timer);
    Timer_startPeriodic(measurement_timer, (uint32)(measurement_interval_ms * 1000UL));
}

/*
 * Description :
 * Return the last successfully measured distance for the specified sensor (cm).
 */
uint16 Ultrasonic_getDistance(Ultrasonic_SensorID sensor)
{
    if (sensor >= ULTRASONIC_COUNT)
    {
        return ULTRASONIC_OUT_OF_RANGE;
    }
    return sensor_state[sensor].last_distance;
}

/*
 * Description :
 * Return TRUE if a new measurement has completed since the last call for
 * this sensor. Clears the flag on read.
 */
boolean Ultrasonic_isDataReady(Ultrasonic_SensorID sensor)
{
    if (sensor >= ULTRASONIC_COUNT)
    {
        return FALSE;
    }
    if (sensor_state[sensor].data_ready)
    {
        sensor_state[sensor].data_ready = FALSE;
        return TRUE;
    }
    return FALSE;
}
