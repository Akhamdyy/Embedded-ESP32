GROUND RULES — include in every response:
• Project path: c:\Users\alikh\Documents\PlatformIO\Projects\Embedded-ESP
• Headers go in: include/    Sources go in: src/
• Only allowed external dependency: FreeRTOS
• No driver/gpio.h, no driver/i2c.h, no driver/ledc.h, no esp_timer.h,
  no esp_rom_* — implement via direct register access per ESP32 TRM
• Keep ALL existing type aliases from include/std_types.h
  (uint8, uint16, uint32, sint16, sint64, float32, boolean, etc.)
• Keep ALL existing macros from include/common_macros.h
• Keep the existing port/pin abstraction (PORTA/PORTB/PORTC, PIN0-PIN7)
  and the gpio_map[][] lookup table in gpio.c
• Keep the existing API signatures in every .h file UNCHANGED so that
  HAL modules (motor.c, ultrasonic.c, mpu6050.c, pid.c, bluetooth.c)
  do NOT need to change their #include or function calls
• platform.h currently wraps esp_attr.h for IRAM_ATTR — keep ISR_ATTR
  working (define it directly or via xtensa intrinsics)
• Build system: PlatformIO + ESP-IDF, board = esp32doit-devkit-v1
• Build command: pio run
• After writing code, always show exact pio run command and expected output
