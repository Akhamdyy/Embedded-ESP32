# Project Overview

ESP32-based autonomous robotic car.

Framework:
- ESP-IDF v5+
- FreeRTOS
- PlatformIO
- No Arduino libraries

Main Components:
- ESP32-WROOM-32 (DOIT DevKit V1)
- L298N dual H-bridge motor driver
- HC-SR04 ultrasonic sensors (x3 — Front, Left, Right)
- MPU-6050 IMU (I2C: SDA=GPIO27, SCL=GPIO33)
- Bluetooth Classic SPP (Bluedroid SPP — no Arduino libraries)

# Architecture

Task-based FreeRTOS design with strict MCAL/HAL layering.

## Layer Rules

- **MCAL** (`gpio.c`, `pwm.c`, `timer.c`, `bt.c`): Only layer that may include ESP-IDF hardware headers.
- **HAL** (`motor.c`, `ultrasonic.c`, `bluetooth.c`): Includes only MCAL headers + FreeRTOS + standard C.
- **App** (`main.c`): May include FreeRTOS and ESP-IDF directly.

## MCAL Modules (completed)

| Module | Files | Wraps |
|--------|-------|-------|
| GPIO | `gpio.h` / `gpio.c` | `driver/gpio.h` — port/pin abstraction, ISR support |
| PWM | `pwm.h` / `pwm.c` | `driver/ledc.h` — channel-based PWM |
| Timer | `timer.h` / `timer.c` | `esp_timer.h` — one-shot and periodic timers |
| BT Controller | `bt.h` / `bt.c` | `esp_bt.h` + Bluedroid + `esp_spp_api.h` — SPP server |
| Platform | `platform.h` | `esp_attr.h` — exposes `ISR_ATTR` macro |

## HAL Modules (completed)

| Module | Files | Uses |
|--------|-------|------|
| Motor | `motor.h` / `motor.c` | `gpio.h`, `pwm.h` |
| Ultrasonic | `ultrasonic.h` / `ultrasonic.c` | `gpio.h`, `timer.h`, `platform.h` |
| Bluetooth | `bluetooth.h` / `bluetooth.c` | `bt.h`, FreeRTOS |
| MPU6050 | `mpu6050.h` / `mpu6050.c` | `i2c.h`, `timer.h`, `motor.h`, FreeRTOS |
| PID Lane Centering | `pid.h` / `pid.c` | `ultrasonic.h`, `motor.h` |

# Important Constraints

- GPIO34-39 are input-only — no output, no internal pull resistors
- GPIO6-11 reserved for internal SPI flash — never use
- GPIO1, 3 reserved for UART0 (USB serial)
- GPIO12 must be LOW at boot (flash voltage select)
- Avoid blocking delays — use FreeRTOS or timer callbacks
- No Arduino framework
- ESP-IDF v5+ compatibility required
- All drivers must follow MCAL/HAL architecture

# Pin Assignment Summary

| Function              | GPIO(s)                         |
|-----------------------|---------------------------------|
| Motor PWM             | 12, 13, 14, 15                  |
| Motor Direction       | 16, 17, 18, 19, 21, 22, 23, 25 |
| Ultrasonic TRIG       | 26, 32, 4                       |
| Ultrasonic ECHO       | 34, 35, 36 (input-only)         |
| MPU-6050 SDA          | 27                              |
| MPU-6050 SCL          | 33                              |
| On-board LED          | 2                               |
| Bluetooth             | Built-in radio — no GPIO        |

# Current Focus (as of 2026-05-08)

- All individual modules tested and working:
  - Ultrasonic (front/left/right) — interrupt-driven, timer-based
  - MPU6050 / MPU6500 gyro Z — I2C via MCAL, 90-degree turn working at TURN_SPEED=90, TARGET_ANGLE_DEG tuning ongoing
  - PID lane centering — implemented, not yet field-tested (KP=2.0, KI=0, KD=2.0, BASE_SPEED=80, THRESHOLD=12cm)
- `main.c` is currently a PID test harness

# Next Steps

- Implement a **Finite State Machine (FSM)** that combines all modules:
  - States to be defined in the next session
  - FSM captures each state sequentially and selects the next state based on sensor input
  - Will replace the current test harness in `main.c`
  - Suggested file: `app/fsm.h` / `app/fsm.c` or directly in `main.c`

# Hardware Notes

- The physical MPU6050 module contains an **MPU-6500** (WHO_AM_I = 0x70, not 0x68)
  - Fully compatible — same register map for gyro Z
- Track width: 45 cm, car width: 15 cm → 15 cm clearance each side
- PID threshold: 12 cm per side
