---
name: Current Project Status
description: Architecture state, completed work, and what still needs attention
type: project
---

## Status as of 2026-05-11 — All Modules Working, FSM Next

All individual hardware modules are tested and working on the physical robot.

### Module Status

| Module | File | Status | Notes |
|--------|------|--------|-------|
| GPIO | `gpio.c` | DONE | Direct registers, no driver/gpio.h |
| PWM | `pwm.c` | DONE | Direct LEDC registers |
| Timer | `timer.c` | DONE | TIMG1_T0 free-running µs counter + FreeRTOS sw-timers |
| Ultrasonic | `ultrasonic.c` | DONE | Interrupt-driven, 3 sensors (Front/Left/Right) |
| I2C | `i2c.c` | DONE — uses `driver/i2c.h` | Register-level rewrite attempted twice and abandoned due to ESP32 FIFO bus quirks; `driver/i2c.h` is the accepted permanent exception |
| MPU6050 | `mpu6050.c` | DONE | Gyro Z working, 90° turn at TURN_SPEED=90 |
| PID | `pid.c` | IMPLEMENTED, not field-tested | KP=2.0, KI=0, KD=2.0, BASE_SPEED=80, THRESHOLD=12 cm |
| Motor | `motor.c` | DONE | 4-wheel H-bridge via L298N |
| Bluetooth | `bluetooth.c` | DONE | Bluedroid SPP, Android tested |
| BT MCAL | `bt.c` | DONE | Uses Bluedroid stack (accepted exception, no register-level alternative) |

### Current `main.c`

Currently a Bluetooth gyro-angle streaming harness (10 Hz, reset on 'R').  
Will be replaced by the FSM driver once FSM is implemented.

---

## Next Deliverable: FSM

Implement `src/fsm.c` + `include/fsm.h` combining all modules.

- FSM plan is in `.claude/memory/FSM.md`
- States run sequentially in a loop
- Uses: `ultrasonic.h`, `motor.h`, `mpu6050.h`, `pid.h`, `bluetooth.h`
- `main.c` will call `FSM_init()` then `FSM_run()` (or loop calling `FSM_step()`)

**Why:** Replace the test harness in main.c with autonomous wall-following behavior.
**How to apply:** When implementing FSM, put it in `src/fsm.c` / `include/fsm.h`. Do not put FSM logic directly in `main.c`.
