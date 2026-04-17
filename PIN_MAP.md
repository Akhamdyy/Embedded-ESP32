# ESP32 DOIT DevKit V1 — Project Pin Map

Board: **ESP32-WROOM-32**
Framework: **ESP-IDF**
Last updated: 2026-04-16

---

## Port/Pin to GPIO Mapping (gpio.h)

The GPIO driver uses a port+pin abstraction. The table below shows the physical
GPIO number each port/pin resolves to.

| Port     | PIN0 | PIN1 | PIN2 | PIN3 | PIN4  | PIN5  | PIN6  | PIN7  |
|----------|------|------|------|------|-------|-------|-------|-------|
| PORTA(0) |  0   |  2   |  4   |  5   |  12   |  13   |  14   |  15   |
| PORTB(1) |  16  |  17  |  18  |  19  |  21   |  22   |  23   |  25   |
| PORTC(2) |  26  |  27  |  32  |  33  | 34 ★ | 35 ★ | 36 ★ | 39 ★ |

★ = **INPUT ONLY** — cannot be used as output

> PORTD is not available: GPIO 24, 28–31 do not exist on the ESP32-WROOM-32.

---

## Used Pins

### On-board

| GPIO | Port/Pin     | Connected To        | Notes                        |
|------|--------------|---------------------|------------------------------|
|  2   | PORTA PIN1   | On-board LED        | Built-in LED, active HIGH    |

---

### H-Bridge 1 — Left Side Motors (L298N)

| GPIO | Port/Pin     | H-Bridge Pin | Function                  |
|------|--------------|--------------|---------------------------|
|  12  | PORTA PIN4   | ENA (PWM)    | Front Left — speed        |
|  13  | PORTA PIN5   | ENB (PWM)    | Rear Left — speed         |
|  16  | PORTB PIN0   | IN1          | Front Left — direction    |
|  17  | PORTB PIN1   | IN2          | Front Left — direction    |
|  18  | PORTB PIN2   | IN3          | Rear Left — direction     |
|  19  | PORTB PIN3   | IN4          | Rear Left — direction     |

> GPIO 12 is a strapping pin — must be LOW during boot. Safe after boot.

---

### H-Bridge 2 — Right Side Motors (L298N)

| GPIO | Port/Pin     | H-Bridge Pin | Function                  |
|------|--------------|--------------|---------------------------|
|  14  | PORTA PIN6   | ENA (PWM)    | Front Right — speed       |
|  15  | PORTA PIN7   | ENB (PWM)    | Rear Right — speed        |
|  21  | PORTB PIN4   | IN1          | Front Right — direction   |
|  22  | PORTB PIN5   | IN2          | Front Right — direction   |
|  23  | PORTB PIN6   | IN3          | Rear Right — direction    |
|  25  | PORTB PIN7   | IN4          | Rear Right — direction    |

> GPIO 15 is a strapping pin — has internal pull-up during boot. Safe after boot.

---

### Ultrasonic Sensor (HC-SR04)

| GPIO | Port/Pin     | Sensor Pin | Notes                                        |
|------|--------------|------------|----------------------------------------------|
|  26  | PORTC PIN0   | TRIG       | Direct connection                            |
|  27  | PORTC PIN1   | ECHO       | Via voltage divider: 1kΩ + 2kΩ (5V → 3.3V) |

---

## Available Pins

These pins are not currently assigned to any peripheral.

| GPIO | Port/Pin     | Direction    | Notes                                         |
|------|--------------|--------------|-----------------------------------------------|
|  0   | PORTA PIN0   | Input/Output | Strapping pin — internal pull-up, boot button |
|  4   | PORTA PIN2   | Input/Output | General purpose                               |
|  5   | PORTA PIN3   | Input/Output | Strapping pin — internal pull-up during boot  |
|  32  | PORTC PIN2   | Input/Output | General purpose                               |
|  33  | PORTC PIN3   | Input/Output | General purpose                               |
|  34  | PORTC PIN4   | Input ONLY   | No output driver                              |
|  35  | PORTC PIN5   | Input ONLY   | No output driver                              |
|  36  | PORTC PIN6   | Input ONLY   | No output driver (labelled VP on board)       |
|  39  | PORTC PIN7   | Input ONLY   | No output driver (labelled VN on board)       |

---

## Reserved / Do Not Use

| GPIO    | Reason                                              |
|---------|-----------------------------------------------------|
|  1      | UART0 TX — used by USB serial monitor               |
|  3      | UART0 RX — used by USB serial monitor               |
|  6–11   | Connected to internal SPI flash — forbidden         |
|  20     | Does not exist on ESP32-WROOM-32                    |
|  24     | Does not exist on ESP32-WROOM-32                    |
|  28–31  | Does not exist on ESP32-WROOM-32                    |

---

## Summary

| Category          | GPIO Pins                          | Count |
|-------------------|------------------------------------|-------|
| Motors (PWM)      | 12, 13, 14, 15                     | 4     |
| Motors (Direction)| 16, 17, 18, 19, 21, 22, 23, 25    | 8     |
| Ultrasonic        | 26, 27                             | 2     |
| On-board LED      | 2                                  | 1     |
| **Used total**    |                                    | **15**|
| Available I/O     | 0, 4, 5, 32, 33                    | 5     |
| Available input   | 34, 35, 36, 39                     | 4     |
| **Available total**|                                   | **9** |
| Reserved          | 1, 3, 6–11, 20, 24, 28–31         | —     |
