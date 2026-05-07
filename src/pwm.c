/******************************************************************************
 *
 * Module: PWM
 *
 * File Name: pwm.c
 *
 * Description: Source file for the ESP32 LEDC-based PWM driver (MCAL layer).
 *              Wraps the ESP-IDF LEDC peripheral so that higher-level HAL
 *              drivers (e.g. motor.c) have no direct ESP-IDF dependency.
 *
 *******************************************************************************/

#include "pwm.h"
#include "driver/ledc.h"

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/
#define PWM_SPEED_MODE      LEDC_HIGH_SPEED_MODE
#define PWM_TIMER           LEDC_TIMER_0
#define PWM_RESOLUTION      LEDC_TIMER_8_BIT

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/* Maps logical PWM_ChannelType to ESP-IDF LEDC channel IDs */
static const ledc_channel_t ledc_ch_map[PWM_NUM_CHANNELS] = {
    LEDC_CHANNEL_0,
    LEDC_CHANNEL_1,
    LEDC_CHANNEL_2,
    LEDC_CHANNEL_3,
};

/* The shared LEDC timer is configured only once regardless of channel order */
static boolean timer_ready = FALSE;

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize a PWM channel on the given physical GPIO pin at the specified
 * frequency. The shared LEDC timer is configured on the first call.
 */
void PWM_initChannel(PWM_ChannelType channel, uint8 gpio_num, uint32 freq_hz)
{
    if (channel >= PWM_NUM_CHANNELS)
    {
        return;
    }

    if (!timer_ready)
    {
        ledc_timer_config_t timer_cfg = {
            .speed_mode      = PWM_SPEED_MODE,
            .duty_resolution = PWM_RESOLUTION,
            .timer_num       = PWM_TIMER,
            .freq_hz         = freq_hz,
            .clk_cfg         = LEDC_AUTO_CLK,
        };
        ledc_timer_config(&timer_cfg);
        timer_ready = TRUE;
    }

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = (int)gpio_num,
        .speed_mode = PWM_SPEED_MODE,
        .channel    = ledc_ch_map[channel],
        .timer_sel  = PWM_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&ch_cfg);
}

/*
 * Description :
 * Set the duty cycle of a previously initialized PWM channel.
 * duty: 0 (always-off) to 255 (always-on).
 */
void PWM_setDuty(PWM_ChannelType channel, uint8 duty)
{
    if (channel >= PWM_NUM_CHANNELS)
    {
        return;
    }
    ledc_set_duty(PWM_SPEED_MODE, ledc_ch_map[channel], duty);
    ledc_update_duty(PWM_SPEED_MODE, ledc_ch_map[channel]);
}
