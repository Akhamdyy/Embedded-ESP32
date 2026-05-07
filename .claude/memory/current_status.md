---
name: Current Project Status
description: Architecture state, completed work, and what still needs attention
type: project
---

## MCAL / HAL Architecture Refactor — COMPLETE (all drivers)

All three hardware subsystems (motor, ultrasonic, Bluetooth) now comply with
the MCAL/HAL layer separation. No HAL file includes any ESP-IDF header directly.

### Layer boundary rule
- **MCAL files** are the ONLY files that include ESP-IDF hardware headers.
- **HAL files** include only project MCAL headers + FreeRTOS + standard C.
- **App layer** (`main.c`) may include FreeRTOS and ESP-IDF directly.

---

## Full MCAL / HAL File Map

| File | Layer | ESP-IDF dependency it wraps |
|------|-------|-----------------------------|
| `gpio.c` | MCAL | `driver/gpio.h`, `esp_attr.h` |
| `pwm.c` | MCAL | `driver/ledc.h` |
| `timer.c` | MCAL | `esp_timer.h`, `esp_attr.h` |
| `bt.c` | MCAL | `esp_bt.h`, `nvs_flash.h`, `esp_err.h` |
| `platform.h` | Platform | `esp_attr.h` → exposes `ISR_ATTR` macro |
| `motor.c` | HAL | uses `gpio.h` + `pwm.h` |
| `ultrasonic.c` | HAL | uses `gpio.h` + `timer.h` + `platform.h` |
| `bluetooth.c` | HAL | uses `bt.h` + FreeRTOS + standard C |

---

## Session Changes — Bluetooth (2026-05-07)

### Files added
| File | Purpose |
|------|---------|
| `include/bt.h` | MCAL BT API: `BT_init`, `BT_registerRecvCallback`, `BT_isSendAvailable`, `BT_sendPacket` |
| `src/bt.c` | Wraps `esp_bt.h` + `nvs_flash.h`; internal VHCI shim forwards events to HAL callback |

### Files modified
| File | Change |
|------|--------|
| `include/bluetooth.h` | Removed `esp_err.h` / `<stdbool.h>`; uses `std_types.h` only; `bluetooth_init` returns `boolean` |
| `src/bluetooth.c` | Removed all ESP-IDF BT includes; uses `bt.h`; all types now use project types (`uint8`, `uint16`, `boolean`, `TRUE/FALSE`) |
| `src/main.c` | `bluetooth_init(...) != ESP_OK` → `bluetooth_init(...) == FALSE` |

### Key design decision — callback registration order
`BT_registerRecvCallback(hci_event_handler)` is called BEFORE `BT_init()` in
`bluetooth_init`. This ensures the HAL callback is in place before the VHCI
interface is registered with the ESP-IDF controller, so no HCI events are lost.

---

## Session Changes — Ultrasonic (2026-05-07)

### Files added
| File | Purpose |
|------|---------|
| `include/timer.h` | MCAL timer API (one-shot + periodic + ISR timestamp) |
| `src/timer.c` | Wraps `esp_timer.h`; `Timer_getTimeUs` is `IRAM_ATTR` |
| `include/platform.h` | Defines `ISR_ATTR = IRAM_ATTR` for use in HAL ISR callbacks |

### Files modified
| File | Change |
|------|--------|
| `include/gpio.h` | Added `GPIO_PullType`, `GPIO_IntrTrigger`, `GPIO_IsrCallback`, `GPIO_setPullMode`, `GPIO_enableInterrupt` |
| `src/gpio.c` | Implemented `GPIO_setPullMode` + `GPIO_enableInterrupt`; `GPIO_readPin` marked `IRAM_ATTR` |
| `src/ultrasonic.c` | Full rewrite — no ESP-IDF; echo pins corrected to GPIO34/35/36 |
| `include/ultrasonic.h` | Wiring comments updated to match new pin layout |

### Echo pin change
Echo pins moved from GPIO27/33/5 → GPIO34/35/36 to free GPIO27/33 for MPU-6050 I2C.
GPIO34-36 are input-only with no internal pull resistors; external voltage dividers hold the line LOW when idle.

---

## Session Changes — Motor (2026-05-07)

### Files added
| File | Purpose |
|------|---------|
| `include/pwm.h` | MCAL PWM API wrapping LEDC |
| `src/pwm.c` | Wraps `driver/ledc.h`; shared LEDC timer guarded by `timer_ready` flag |

### Files modified
| File | Change |
|------|--------|
| `src/motor.c` | Removed `driver/ledc.h`; uses `PWM_initChannel` / `PWM_setDuty` |
| `include/motor.h` | Updated description comment |

---

## Current Focus (as of 2026-05-07)

- **MPU-6050 90-degree turn** — not yet implemented.
  GPIO27 (SDA) and GPIO33 (SCL) are reserved and confirmed in pin layout.
  `main.c` already has a working I2C + MPU-6050 read loop as a hardware test.
  Next step: build an I2C MCAL (`i2c.h`/`i2c.c`) and an MPU-6050 HAL driver,
  then implement the FreeRTOS turn task using gyro Z integration.

- **`main.c`** — currently a breadboard test harness. Will need to be replaced
  with the final FreeRTOS task architecture once all drivers are in place.

## What Still Needs MCAL/HAL Review

- `main.c` uses `driver/i2c.h` directly. When the MPU-6050 driver is added,
  an I2C MCAL should wrap this.
