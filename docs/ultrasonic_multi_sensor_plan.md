# Plan: Multi-Sensor Ultrasonic Driver

Status: **PENDING TEAM DECISION**
Created: 2026-04-16

---

## Context

The current ultrasonic driver (`ultrasonic.h` / `ultrasonic.c`) supports
one sensor only (front-facing HC-SR04 on GPIO 26/27). This plan covers
expanding it to 3 sensors — front, left, and right — to enable maze
centering with a known road width of 45 cm and car width of 20 cm.

---

## Centering Math

```
Road width  : 45 cm
Car width   : 20 cm
Target gap  : (45 - 20) / 2 = 12.5 cm from each wall
```

When left sensor reads 12.5 cm and right sensor reads 12.5 cm, the car
is perfectly centered. Any deviation is corrected by adjusting motor
speeds on the drifted side.

---

## Pin Assignment

| Sensor | Signal | GPIO | Port/Pin     |
|--------|--------|------|--------------|
| Front  | TRIG   |  26  | PORTC PIN0   |
| Front  | ECHO   |  27  | PORTC PIN1   |
| Left   | TRIG   |  32  | PORTC PIN2   |
| Left   | ECHO   |  33  | PORTC PIN3   |
| Right  | TRIG   |   4  | PORTA PIN2   |
| Right  | ECHO   |   5  | PORTA PIN3   |

All 6 pins are currently available (confirmed in PIN_MAP.md).

---

## Why the Driver Must Change

The current driver has hardcoded GPIO numbers and a single set of static
variables (timestamps, distance, flags, timers). A second or third sensor
cannot be added without a full redesign. Specifically:

1. `#define TRIG_GPIO 26` and `#define ECHO_GPIO 27` are hardcoded
2. There is one ISR with no way to identify which sensor fired
3. There is one `esp_timer_handle_t` for the trigger pulse
4. All state variables (`echo_start_us`, `last_distance`, etc.) are
   global singletons — they would be overwritten by multiple sensors

---

## Proposed Changes

### 1. New Sensor ID Enum (ultrasonic.h)

```c
typedef enum
{
    ULTRASONIC_FRONT = 0,
    ULTRASONIC_LEFT  = 1,
    ULTRASONIC_RIGHT = 2,
    ULTRASONIC_COUNT = 3
} Ultrasonic_SensorID;
```

### 2. Internal Config Struct (ultrasonic.c)

Holds the fixed hardware config per sensor:

```c
typedef struct
{
    int         trig_gpio;
    int         echo_gpio;
    const char *name;
} Ultrasonic_Config;

static const Ultrasonic_Config sensor_cfg[ULTRASONIC_COUNT] = {
    /* FRONT */ { 26, 27, "FRONT" },
    /* LEFT  */ { 32, 33, "LEFT"  },
    /* RIGHT */ {  4,  5, "RIGHT" },
};
```

### 3. Internal State Struct (ultrasonic.c)

Each sensor gets its own independent runtime state:

```c
typedef struct
{
    volatile int64_t   echo_start_us;
    volatile boolean   echo_active;
    volatile uint16_t  last_distance;
    volatile boolean   data_ready;
    esp_timer_handle_t trig_pulse_timer;
} Ultrasonic_State;

static Ultrasonic_State sensor_state[ULTRASONIC_COUNT];
```

### 4. Shared ISR with Sensor Index

The ISR receives the sensor index as its argument so it updates the
correct state struct:

```c
static void IRAM_ATTR echo_isr_handler(void *arg)
{
    uint32_t idx = (uint32_t)arg;
    Ultrasonic_State *s = &sensor_state[idx];

    if (gpio_get_level(sensor_cfg[idx].echo_gpio) == 1)
    {
        s->echo_start_us = esp_timer_get_time();
        s->echo_active   = TRUE;
    }
    else
    {
        if (s->echo_active)
        {
            int64_t duration = esp_timer_get_time() - s->echo_start_us;
            s->last_distance = (duration > 0 && duration < 30000)
                               ? (uint16_t)(duration / 58)
                               : ULTRASONIC_OUT_OF_RANGE;
            s->data_ready  = TRUE;
            s->echo_active = FALSE;
        }
    }
}
```

### 5. Staggered Firing (no sensor interference)

All 3 sensors share one periodic timer but are fired one at a time.
A static index cycles through them each tick:

```c
// Interval per sensor slot: 70ms  →  full cycle: 210ms (~5 readings/sec)
static void measurement_timer_cb(void *arg)
{
    static uint32_t current = 0;

    // Log previous result for this slot
    uint16_t d = sensor_state[current].last_distance;
    if (d == ULTRASONIC_OUT_OF_RANGE)
        printf("[%s] OUT OF RANGE\n", sensor_cfg[current].name);
    else
        printf("[%s] %u cm\n", sensor_cfg[current].name, d);

    // Fire this sensor
    gpio_set_level(sensor_cfg[current].trig_gpio, 1);
    esp_timer_start_once(sensor_state[current].trig_pulse_timer, 10);

    // Advance to next sensor
    current = (current + 1) % ULTRASONIC_COUNT;
}
```

### 6. Updated Public API (ultrasonic.h)

```c
// Init all sensors, measurement_interval_ms is per sensor slot
void Ultrasonic_initAll(uint32 measurement_interval_ms);

// Get last distance for a specific sensor (cm)
uint16 Ultrasonic_getDistance(Ultrasonic_SensorID sensor);

// Check if new data is available for a specific sensor
boolean Ultrasonic_isDataReady(Ultrasonic_SensorID sensor);
```

Usage example:

```c
Ultrasonic_initAll(70);   // 70ms per slot = 210ms full cycle

uint16 front = Ultrasonic_getDistance(ULTRASONIC_FRONT);
uint16 left  = Ultrasonic_getDistance(ULTRASONIC_LEFT);
uint16 right = Ultrasonic_getDistance(ULTRASONIC_RIGHT);
```

---

## What Stays the Same

- Timer-based approach — no blocking delays
- 10µs trigger pulse via one-shot `esp_timer`
- Echo measurement via GPIO interrupt on both edges
- `duration / 58` distance formula
- `ULTRASONIC_OUT_OF_RANGE` return value on timeout

---

## Files to Modify

| File                  | Change                                          |
|-----------------------|-------------------------------------------------|
| `include/ultrasonic.h`| Add `Ultrasonic_SensorID` enum, update API      |
| `src/ultrasonic.c`    | Full rewrite to support 3 sensor instances      |
| `PIN_MAP.md`          | Add Left and Right sensor pins, mark as used    |
| `src/main.c`          | Update init call and distance reads             |

---

## Open Questions for the Team

1. Is 3 ultrasonic sensors the final decision or are IR sensors being
   considered for the side walls instead?
2. Is a 210ms full cycle fast enough for the car speed being planned?
   (At 0.5 m/s the car travels ~10cm per cycle — acceptable?)
3. Should the centering correction logic live in `ultrasonic.c` or in
   a separate `navigation.c` module?
4. What is the correction threshold — at what deviation (e.g. ±2 cm
   from 12.5 cm) should the car start correcting?
