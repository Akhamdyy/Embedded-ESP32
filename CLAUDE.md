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

# Current Focus (as of 2026-05-07)

- MPU-6050 90-degree turn: gyro Z integration in a FreeRTOS task
- Next MCAL needed: `i2c.h` / `i2c.c` to wrap `driver/i2c.h`
- `main.c` is currently a breadboard test harness — replace with final task architecture
