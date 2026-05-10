# Project Overview

ESP32-based autonomous robotic car.

Framework:
- NO ESP IDF APIs ONLY REGISTER LEVEL ACCESS
- FreeRTOS (only allowed external dependency)
- PlatformIO (`esp32doit-devkit-v1` board)
- No Arduino libraries

Main Components:
- ESP32-WROOM-32 (DOIT DevKit V1)
- L298N dual H-bridge motor driver
- HC-SR04 ultrasonic sensors (x3 — Front, Left, Right)
- MPU-6050 IMU (I2C: SDA=GPIO27, SCL=GPIO33)
- Bluetooth Classic SPP (Bluedroid SPP — no Arduino libraries)

# Ground Rules

These rules are non-negotiable and must be followed in every response:

## File Layout
- Headers go in: `include/`
- Sources go in: `src/`

## Forbidden Headers (use direct register access instead)
- NO `driver/gpio.h`
- NO `driver/i2c.h`
- NO `driver/ledc.h`
- NO `esp_timer.h`
- NO `esp_rom_*` headers
- Implement all hardware access via **direct register writes/reads per the ESP32 TRM**

## Required Preservation
- Keep ALL existing type aliases from `include/std_types.h`
  (`uint8`, `uint16`, `uint32`, `sint16`, `sint64`, `float32`, `boolean`, etc.)
- Keep ALL existing macros from `include/common_macros.h`
- Keep the existing port/pin abstraction (`PORTA`/`PORTB`/`PORTC`, `PIN0`–`PIN7`)
  and the `gpio_map[][]` lookup table in `gpio.c`
- Keep every existing `.h` API signature **UNCHANGED** — HAL modules
  (`motor.c`, `ultrasonic.c`, `mpu6050.c`, `pid.c`, `bluetooth.c`) must not need
  to change their `#include` or function calls

## ISR / Platform
- `platform.h` exposes `ISR_ATTR` — keep it working
  (define directly via `__attribute__((section(".iram1")))` or xtensa intrinsics;
  wrapping `esp_attr.h` is acceptable as a last resort)

## Build
- Build command: `pio run`
- After writing code always show the exact `pio run` command and the expected output

# Architecture

Task-based FreeRTOS design with strict MCAL/HAL layering.

## Layer Rules

- **MCAL** (`gpio.c`, `pwm.c`, `timer.c`, `i2c.c`, `bt.c`):
  Implements hardware access via **direct register access** only.
  May include `soc/` peripheral headers (register definitions) and FreeRTOS.
  Must NOT include `driver/*` headers.
- **HAL** (`motor.c`, `ultrasonic.c`, `mpu6050.c`, `pid.c`, `bluetooth.c`):
  Includes only MCAL headers + FreeRTOS + standard C.
  HAL modules may include other HAL modules (e.g. `mpu6050.c` includes `motor.h`).
- **App** (`main.c`, `fsm.c`):
  May include FreeRTOS and HAL directly.

## MCAL Modules

| Module   | Files               | Implements                                    |
|----------|---------------------|-----------------------------------------------|
| GPIO     | `gpio.h` / `gpio.c` | Port/pin abstraction, ISR — direct registers  |
| PWM      | `pwm.h` / `pwm.c`   | Channel-based PWM — direct LEDC registers     |
| Timer    | `timer.h` / `timer.c` | One-shot + periodic timers — direct registers |
| BT       | `bt.h` / `bt.c`     | Bluedroid SPP server                          |
| I2C      | `i2c.h` / `i2c.c`   | I2C master — direct registers                 |
| Platform | `platform.h`        | Exposes `ISR_ATTR` macro                      |

## HAL Modules

| Module        | Files                       | Uses                              |
|---------------|-----------------------------|-----------------------------------|
| Motor         | `motor.h` / `motor.c`       | `gpio.h`, `pwm.h`                 |
| Ultrasonic    | `ultrasonic.h` / `ultrasonic.c` | `gpio.h`, `timer.h`, `platform.h` |
| Bluetooth     | `bluetooth.h` / `bluetooth.c` | `bt.h`, FreeRTOS                 |
| MPU6050       | `mpu6050.h` / `mpu6050.c`   | `i2c.h`, `timer.h`, `motor.h`, FreeRTOS |
| PID           | `pid.h` / `pid.c`           | `ultrasonic.h`, `motor.h`         |

# Important Constraints

- GPIO34–39 are input-only — no output, no internal pull resistors
- GPIO6–11 reserved for internal SPI flash — never use
- GPIO1, 3 reserved for UART0 (USB serial)
- GPIO12 must be LOW at boot (flash voltage select)
- Avoid blocking delays — use FreeRTOS or timer callbacks
- No Arduino framework
- ESP-IDF v5+ compatibility required

# Pin Assignment Summary

| Function          | GPIO(s)                          |
|-------------------|----------------------------------|
| Motor PWM         | 12, 13, 14, 15                   |
| Motor Direction   | 16, 17, 18, 19, 21, 22, 23, 25  |
| Ultrasonic TRIG   | 26, 32, 4                        |
| Ultrasonic ECHO   | 34, 35, 36 (input-only)          |
| MPU-6050 SDA      | 27                               |
| MPU-6050 SCL      | 33                               |
| On-board LED      | 2                                |
| Bluetooth         | Built-in radio — no GPIO         |

# Current Focus (as of 2026-05-08)

All individual modules tested and working:
- Ultrasonic (front/left/right) — interrupt-driven, timer-based
- MPU6050 / MPU6500 gyro Z — I2C via MCAL, 90-degree turn working at TURN_SPEED=90
- PID lane centering — implemented, not yet field-tested (KP=2.0, KI=0, KD=2.0, BASE_SPEED=80, THRESHOLD=12 cm)
- `main.c` is currently a PID test harness

# Next Steps

Implement a **Finite State Machine (FSM)** combining all modules:
- States to be defined by the user
- FSM captures each state sequentially, selects next state based on sensor input
- Will replace the current test harness in `main.c`
- Suggested files: `app/fsm.h` / `app/fsm.c` or directly in `main.c`

# Hardware Notes

- Physical MPU6050 module contains an **MPU-6500** (WHO_AM_I = 0x70, not 0x68)
  — fully compatible, same register map for gyro Z
- Track width: 45 cm, car width: 15 cm → 15 cm clearance each side
- PID threshold: 12 cm per side
