---
name: GPIO Pin Layout
description: Current pin assignments for all ESP32-WROOM-32 peripherals on the robot
type: project
---

## ESP32-WROOM-32 — Current Pin Assignment (as of 2026-05-07)

| Function              | GPIO(s)                        | Port/Pin (project abstraction) | Count |
|-----------------------|--------------------------------|-------------------------------|-------|
| Motors PWM (ENA/ENB)  | 12, 13, 14, 15                 | PORTA PIN4-7                  | 4     |
| Motors Direction (IN) | 16, 17, 18, 19, 21, 22, 23, 25| PORTB PIN0-7                  | 8     |
| Ultrasonic TRIG       | 26, 32, 4                      | PORTC/PIN0, PORTC/PIN2, PORTA/PIN2 | 3 |
| Ultrasonic ECHO       | 34, 35, 36                     | PORTC PIN4-6 (input-only⚠)   | 3     |
| MPU-6050 SDA (I2C)    | 27                             | PORTC PIN1                    | 1     |
| MPU-6050 SCL (I2C)    | 33                             | PORTC PIN3                    | 1     |
| On-board LED          | 2                              | PORTA PIN1                    | 1     |
| Bluetooth Classic     | Built-in radio — no GPIO used  | —                             | —     |
| **Used total**        |                                |                               | **21**|
| Available I/O         | 0, 5                           | PORTA PIN0, PORTA PIN3        | 2     |
| Available input-only  | 39                             | PORTC PIN7                    | 1     |
| **Available total**   |                                |                               | **3** |
| Reserved (no-touch)   | 1, 3, 6–11, 20, 24, 28–31     | UART0 / SPI flash / non-existent | —  |

## Sensor Wiring Detail

### Ultrasonic HC-SR04 (3 sensors)

| Sensor | TRIG GPIO | ECHO GPIO | ECHO note |
|--------|-----------|-----------|-----------|
| FRONT  | GPIO 26   | GPIO 34   | Input-only, no internal pull — external 1kΩ/2kΩ divider required |
| LEFT   | GPIO 32   | GPIO 35   | Input-only, no internal pull — external 1kΩ/2kΩ divider required |
| RIGHT  | GPIO 4    | GPIO 36   | Input-only, no internal pull — external 1kΩ/2kΩ divider required |

Echo voltage divider per sensor (5 V HC-SR04 → 3.3 V ESP32):

```
HC-SR04 ECHO ── 1 kΩ ── GPIO3x
                    |
                   2 kΩ
                    |
                   GND
```

### Motor Driver L298N

| Motor        | PWM GPIO | IN1 GPIO | IN2 GPIO |
|--------------|----------|----------|----------|
| Front Left   | 12       | 16       | 17       |
| Rear Left    | 13       | 18       | 19       |
| Front Right  | 14       | 21       | 22       |
| Rear Right   | 15       | 23       | 25       |

### MPU-6050 (I2C)

| Signal | GPIO | Note                                  |
|--------|------|---------------------------------------|
| SDA    | 27   | 4.7 kΩ pull-up to 3.3 V (on breakout)|
| SCL    | 33   | 4.7 kΩ pull-up to 3.3 V (on breakout)|
| AD0    | GND  | Sets I2C address = 0x68               |
| VCC    | 3.3 V| ⚠ Do NOT connect to 5 V              |

## Important Constraints

- GPIO 34–39 are **input-only** — no output, no internal pull resistors.
- GPIO 12 must be LOW at boot (flash voltage select) — motor PWM starts at 0.
- GPIO 6–11 are connected to internal SPI flash — never use.
- GPIO 1, 3 are UART0 (USB serial) — avoid for I/O.
