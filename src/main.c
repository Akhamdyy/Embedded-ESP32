  #include "pwm.h"
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"

  void app_main(void)
  {
      Pwm_TimerConfig tcfg = {
          .timer     = PWM_TIMER_0,
          .freq_hz   = 1000u,
          .duty_bits = 8u,
      };
      Pwm_timerInit(&tcfg);

      Pwm_ChannelConfig ccfg = {
          .channel  = PWM_CHANNEL_0,
          .timer    = PWM_TIMER_0,
          .gpio_num = 2,            /* on-board LED */
      };
      Pwm_channelInit(&ccfg);

      while (1)
      {
          for (int d = 0; d <= 255; d += 5)
          {
              Pwm_setDuty(PWM_CHANNEL_0, (uint8)d);
              vTaskDelay(pdMS_TO_TICKS(10));
          }
          for (int d = 255; d >= 0; d -= 5)
          {
              Pwm_setDuty(PWM_CHANNEL_0, (uint8)d);
              vTaskDelay(pdMS_TO_TICKS(10));
          }
      }
  }