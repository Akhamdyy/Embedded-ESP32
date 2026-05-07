/******************************************************************************
 *
 * Module: PWM (MCAL)
 *
 * File Name: pwm.h
 *
 * Description: MCAL-level PWM driver for ESP32.
 *              Wraps the ESP-IDF LEDC peripheral behind a hardware-independent
 *              interface. Upper layers (motor HAL, etc.) must use only this
 *              API and must NOT include "driver/ledc.h" directly.
 *
 *              Supports up to 4 timers and 8 channels.
 *              All channels operate in high-speed mode internally.
 *              Duty cycle is always 8-bit (0–255) unless duty_bits is changed.
 *
 *******************************************************************************/

#ifndef PWM_H_
#define PWM_H_

#include "std_types.h"

/*******************************************************************************
 *                                Types                                        *
 *******************************************************************************/

typedef enum
{
    PWM_TIMER_0 = 0,
    PWM_TIMER_1 = 1,
    PWM_TIMER_2 = 2,
    PWM_TIMER_3 = 3
} Pwm_TimerID;

typedef enum
{
    PWM_CHANNEL_0 = 0,
    PWM_CHANNEL_1 = 1,
    PWM_CHANNEL_2 = 2,
    PWM_CHANNEL_3 = 3,
    PWM_CHANNEL_4 = 4,
    PWM_CHANNEL_5 = 5,
    PWM_CHANNEL_6 = 6,
    PWM_CHANNEL_7 = 7
} Pwm_ChannelID;

/* Timer configuration — shared by all channels attached to this timer */
typedef struct
{
    Pwm_TimerID timer;       /* Which LEDC timer to configure         */
    uint32      freq_hz;     /* PWM frequency in Hz (e.g. 1000)       */
    uint8       duty_bits;   /* Resolution: 8 = 0-255, 10 = 0-1023   */
} Pwm_TimerConfig;

/* Channel configuration — one channel drives one GPIO at the timer's frequency */
typedef struct
{
    Pwm_ChannelID channel;   /* LEDC channel ID                       */
    Pwm_TimerID   timer;     /* Timer this channel is attached to     */
    int           gpio_num;  /* Physical ESP32 GPIO number            */
} Pwm_ChannelConfig;

/*******************************************************************************
 *                             Definitions                                     *
 *******************************************************************************/

#define PWM_DUTY_MAX    255u
#define PWM_DUTY_MIN    0u

/*******************************************************************************
 *                           Function Prototypes                               *
 *******************************************************************************/

/*
 * Description :
 * Configure a LEDC timer (frequency + resolution).
 * Must be called before any channel that references this timer is initialized.
 */
void Pwm_timerInit(const Pwm_TimerConfig *cfg);

/*
 * Description :
 * Configure a single LEDC channel (GPIO assignment, timer selection).
 * Initial duty is set to 0 (off). Call Pwm_timerInit for the referenced
 * timer before calling this function.
 */
void Pwm_channelInit(const Pwm_ChannelConfig *cfg);

/*
 * Description :
 * Update the duty cycle of a channel immediately.
 * duty: 0 (0 %) to 255 (100 %) when using 8-bit resolution.
 */
void Pwm_setDuty(Pwm_ChannelID channel, uint8 duty);

#endif /* PWM_H_ */
