---
name: Current Project Status
description: Architecture state, completed work, and what still needs attention
type: project
---

## Register-Level Refactor — IN PROGRESS

Goal: eliminate every `driver/*` and `esp_*` ESP-IDF dependency from MCAL,
replacing it with direct ESP32 register access per the TRM. Only allowed
external dependency is FreeRTOS.

Branch: `refactor/step1-gpio` (continuing on the same branch through all steps).

### Steps

| Step | Module | Status | Notes |
|------|--------|--------|-------|
| 1 | `gpio.c` | DONE & tested | Direct GPIO matrix + IO_MUX. Uses `ISR_ATTR` (was `IRAM_ATTR`). |
| 2 | `pwm.c`  | DONE & tested | Direct LEDC + GPIO matrix routing (signals 71–78), Q20.4 duty, Q10.8 divisor. LED-sweep test on GPIO2 confirmed working. |
| 3A | `timer.c` | **DONE, build OK, runtime broken** — see "Open issue" below |
| 3B | `ultrasonic.c` / `platform.h` | DONE — `ultrasonic.c` unchanged (already MCAL-clean). `platform.h` rewritten to define `ISR_ATTR` directly via `__attribute__((section(".iram1")))` — no `esp_attr.h`. |
| 4 | `i2c.c`, `bt.c` | NOT STARTED |

---

## Step 3 Implementation Detail (`src/timer.c`)

- **Create / Start one-shot & periodic** → backed by **FreeRTOS software timers**
  (`xTimerCreate`, `xTimerChangePeriod`). User callback + arg stored in a static
  `ctx_pool[8]`; `prv_dispatch` adapts FreeRTOS' `void(TimerHandle_t)` signature
  to our `void(void*)`.
- **`Timer_getTimeUs`** → reads **TIMG1_T0** as a free-running 64-bit counter
  ticking at 1 MHz (APB / 80). Latch via `TIMG_T0UPDATE_REG`, then read
  `TIMG_T0LO_REG` + `TIMG_T0HI_REG`. Marked `ISR_ATTR`.
- **TIMG1, not TIMG0**: ESP-IDF's `esp_timer` LAC driver owns TIMG0_T0 on ESP32
  and reconfigures the prescaler at boot. Using TIMG1_T0 avoids that conflict.
- **Sub-ms one-shot fast path**: `Timer_startOnce` with `delay_us < 1000` busy-
  waits on `Timer_getTimeUs` and dispatches the callback synchronously. This is
  required because FreeRTOS sw-timers can't go below 1 tick (10 ms at the
  default `CONFIG_FREERTOS_HZ=100`), and the HC-SR04 trigger pulse needs a real
  ~10 µs pulse, not a 10 ms one.
- DPORT clock: `DPORT_TIMERGROUP1_CLK_EN` ungated, `DPORT_TIMERGROUP1_RST`
  pulsed once on first `Timer_create*` call (`prv_timgInit`, idempotent).

### `include/platform.h` (final)

Defines only `ISR_ATTR` as `__attribute__((section(".iram1")))`. Does NOT
re-define `IRAM_ATTR` — `esp_attr.h` leaks in transitively via
`freertos/FreeRTOS.h` and would clash. All project files that previously used
the bare `IRAM_ATTR` spelling have been switched to `ISR_ATTR` (only `gpio.c`
needed updating: `GPIO_sharedIsr` and `GPIO_readPin`).

---

## Open Issue — Ultrasonic returns OOR after Step 3 refactor

### Symptom

After flashing the Step-3 build, `pio device monitor` shows:

```
F=65535  L=65535  R=65535  cm
[LEFT] OOR
F=65535  L=65535  R=65535  cm
[RIGHT] OOR
[FRONT] OOR
```

`measurement_timer_cb` IS firing (the round-robin `[FRONT/LEFT/RIGHT] OOR`
prints prove it), but `last_distance` never updates from its initial
`ULTRASONIC_OUT_OF_RANGE` value. So either the echo ISR isn't firing, or it
fires but `duration_us` fails the `> 0 && < ECHO_TIMEOUT_US` check.

### Fixes already applied (in current `timer.c`)

1. Switched the µs counter from TIMG0_T0 to **TIMG1_T0** to avoid ESP-IDF's
   `esp_timer` LAC driver reconfiguring our divider.
2. Added the **busy-wait fast path** in `Timer_startOnce` so the HC-SR04 TRIG
   pulse is a real ~10 µs (was being stretched to ~10 ms by FreeRTOS tick
   quantisation).

User has not yet rebuilt + flashed after these two fixes — that is the next
thing to do before deeper debugging.

### Potential remaining problems if OOR persists after the rebuild

1. **TIMG1_T0 not actually counting** — verify by printing
   `Timer_getTimeUs()` in `app_main`'s loop; it should increment by ~500_000
   per 500 ms iteration. If it's frozen at 0, `prv_timgInit` isn't writing
   the CONFIG register correctly (re-check the divider bit position 13 and
   `TIMG_T0_EN` bit 31).
2. **DPORT_TIMERGROUP1_CLK_EN already gated off** by ESP-IDF startup —
   unlikely (default is enabled), but a sanity read of
   `DPORT_PERIP_CLK_EN_REG` would confirm bit 14 is set after init.
3. **GPIO ISR not firing on echo edges** — check by toggling an LED inside
   `echo_isr_handler`. If the ISR doesn't fire, the GPIO interrupt config in
   the Step-1 `gpio.c` may have regressed when we changed `IRAM_ATTR` →
   `ISR_ATTR` on `GPIO_sharedIsr`. (Both should resolve to placement in
   `.iram1`, but the ESP-IDF `IRAM_ATTR` macro uses unique subsections like
   `.iram1.<counter>` while our `ISR_ATTR` uses bare `.iram1` — both are
   matched by the linker script `*(.iram1 .iram1.*)`, but worth verifying.)
4. **HC-SR04 wiring** — voltage divider on the echo line (1 kΩ / 2 kΩ for
   5 V → 3.3 V) must be present and the idle level must be LOW. GPIO34–36 have
   no internal pull resistors, so a missing divider means the line floats.
5. **TIMG1 watchdog accidentally enabled** — when we pulsed
   `DPORT_TIMERGROUP1_RST`, if any TIMG1 register layout assumption is off,
   we could have armed `TIMG1_WDT` which would reset the chip. (No reset
   loop has been observed yet, so probably not the issue.)

### Next debug step (recommended)

In `main.c`, replace the test loop with:

```c
while (1) {
    sint64 t = Timer_getTimeUs();
    printf("t=%lld  F=%u  L=%u  R=%u\n",
           t,
           Ultrasonic_getDistance(ULTRASONIC_FRONT),
           Ultrasonic_getDistance(ULTRASONIC_LEFT),
           Ultrasonic_getDistance(ULTRASONIC_RIGHT));
    vTaskDelay(pdMS_TO_TICKS(500));
}
```

If `t` is stuck at 0 → TIMG1_T0 init is broken.
If `t` advances correctly but distances stay OOR → echo ISR or wiring.

---

## Architecture Layer Map (post-Step-3)

| File | Layer | What it depends on now |
|------|-------|-----------------------|
| `gpio.c` | MCAL | `soc/*` register headers + `platform.h` (no `driver/gpio.h`) |
| `pwm.c`  | MCAL | `soc/ledc_reg.h`, `soc/gpio_reg.h`, `soc/gpio_sig_map.h`, `soc/io_mux_reg.h`, `soc/dport_reg.h` |
| `timer.c` | MCAL | `soc/dport_reg.h` + `freertos/timers.h` + `platform.h` (no `esp_timer.h`, no `esp_attr.h`) |
| `bt.c` | MCAL | `esp_bt.h`, `nvs_flash.h` (Step 4 will refactor) |
| `i2c.c` | MCAL | `driver/i2c.h` (Step 4 will refactor) |
| `platform.h` | Platform | none — defines `ISR_ATTR` directly |
| `motor.c` | HAL | `gpio.h` + `pwm.h` |
| `ultrasonic.c` | HAL | `gpio.h` + `timer.h` + `platform.h` |
| `mpu6050.c` | HAL | `i2c.h` + `timer.h` + `motor.h` |
| `pid.c` | HAL | `ultrasonic.h` + `motor.h` |
| `bluetooth.c` | HAL | `bt.h` + FreeRTOS |

---

## What Still Needs Refactoring

- **`bt.c`** — still wraps `esp_bt.h` + `nvs_flash.h`. Bluedroid stack is huge;
  going register-level is impractical, so the plan here may be to leave the
  Bluedroid SPP stack as the only ESP-IDF dependency in MCAL and document the
  exception.
- **`i2c.c`** — currently absent; `main.c` still uses `driver/i2c.h` directly.
  Needs an MCAL wrapper using direct I2C peripheral registers (TRM §11).

---

## Build & Test

- Build: `pio run`
- Flash + monitor: `pio run -t upload && pio device monitor`
- Monitor speed: 115200, port COM3.
- Board: `esp32doit-devkit-v1` (note: 4 MB vs 2 MB flash mismatch warning is
  cosmetic and pre-existing).
