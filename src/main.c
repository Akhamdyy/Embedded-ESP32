#include "ultrasonic.h"
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include <stdio.h>

  void app_main(void)
  {
      /* round-robin every 500 ms across the 3 sensors */
      Ultrasonic_initAll(500u);

      while (1)
      {
          uint16 f = Ultrasonic_getDistance(ULTRASONIC_FRONT);
          uint16 l = Ultrasonic_getDistance(ULTRASONIC_LEFT);
          uint16 r = Ultrasonic_getDistance(ULTRASONIC_RIGHT);
          printf("F=%u  L=%u  R=%u  cm\n",
                 (unsigned)f, (unsigned)l, (unsigned)r);
        //   vTaskDelay(pdMS_TO_TICKS(500));
      }
  }