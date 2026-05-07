/******************************************************************************
 *
 * Module: PWM (MCAL)
 *
 * File Name: pwm.c
 *
 * Description: ESP-IDF LEDC-backed implementation of the MCAL PWM driver.
 *              This is the ONLY source file that includes "driver/ledc.h".
 *              All upper-layer modules (motor HAL, etc.) must use pwm.h only.
 *
 *              All channels are configured in LEDC_HIGH_SPEED_MODE.
 *              Duty cycle is assumed to be 8-bit (0-255) for setDuty calls.
 *
 *******************************************************************************/

#include "pwm.h"
#include "driver/ledc.h"

/*******************************************************************************
 *                         Private Helpers                                     *
 *******************************************************************************/

/* Maps our Pwm_TimerID to ESP-IDF ledc_timer_t */
static ledc_timer_t prv_toIdfTimer(Pwm_TimerID timer)
{
    switch (timer)
    {
        case PWM_TIMER_1: return LEDC_TIMER_1;
        case PWM_TIMER_2: return LEDC_TIMER_2;
        case PWM_TIMER_3: return LEDC_TIMER_3;
        case PWM_TIMER_0:
        default:          return LEDC_TIMER_0;
    }
}

/* Maps our Pwm_ChannelID to ESP-IDF ledc_channel_t */
static ledc_channel_t prv_toIdfChannel(Pwm_ChannelID channel)
{
    switch (channel)
    {
        case PWM_CHANNEL_1: return LEDC_CHANNEL_1;
        case PWM_CHANNEL_2: return LEDC_CHANNEL_2;
        case PWM_CHANNEL_3: return LEDC_CHANNEL_3;
        case PWM_CHANNEL_4: return LEDC_CHANNEL_4;
        case PWM_CHANNEL_5: return LEDC_CHANNEL_5;
        case PWM_CHANNEL_6: return LEDC_CHANNEL_6;
        case PWM_CHANNEL_7: return LEDC_CHANNEL_7;
        case PWM_CHANNEL_0:
        default:             return LEDC_CHANNEL_0;
    }
}

/*******************************************************************************
 *                         Function Definitions                                *
 *******************************************************************************/

/*
 * Description :
 * Configure a LEDC timer with the given frequency and duty resolution.
 */
void Pwm_timerInit(const Pwm_TimerConfig *cfg)
{
    if (cfg == NULL_PTR)
    {
        return;
    }

    ledc_timer_config_t idf_timer = {
        .speed_mode      = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)cfg->duty_bits,
        .timer_num       = prv_toIdfTimer(cfg->timer),
        .freq_hz         = cfg->freq_hz,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&idf_timer);
}

/*
 * Description :
 * Configure a single LEDC channel: attach GPIO and timer, set initial duty 0.
 */
void Pwm_channelInit(const Pwm_ChannelConfig *cfg)
{
    if (cfg == NULL_PTR)
    {
        return;
    }

    ledc_channel_config_t idf_ch = {
        .gpio_num   = cfg->gpio_num,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = prv_toIdfChannel(cfg->channel),
        .timer_sel  = prv_toIdfTimer(cfg->timer),
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&idf_ch);
}

/*
 * Description :
 * Set the duty cycle of a channel. duty: 0-255 for 8-bit resolution.
 */
void Pwm_setDuty(Pwm_ChannelID channel, uint8 duty)
{
    ledc_channel_t idf_ch = prv_toIdfChannel(channel);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, idf_ch, (uint32_t)duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, idf_ch);
}
