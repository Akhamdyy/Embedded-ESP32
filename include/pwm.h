/******************************************************************************
 *
 * Module: PWM
 *
 * File Name: pwm.h
 *
 * Description: Header file for the ESP32 LEDC-based PWM driver (MCAL layer).
 *              Provides a hardware-agnostic channel API for PWM output.
 *              All LEDC-specific details are confined to pwm.c.
 *
 * PWM Parameters:
 *   Resolution : 8-bit (duty range 0–255)
 *   Timer      : LEDC_TIMER_0, LEDC_HIGH_SPEED_MODE
 *   Frequency  : set per-channel at init time
 *
 *******************************************************************************/

#ifndef PWM_H_
#define PWM_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define PWM_MAX_DUTY        (255U)
#define PWM_MIN_DUTY        (0U)

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
    PWM_CHANNEL_0 = 0,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_3,
    PWM_NUM_CHANNELS
} PWM_ChannelType;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize a PWM channel on the given physical GPIO pin at the specified
 * frequency. The shared LEDC timer is configured on the first call.
 * Must be called once per channel before PWM_setDuty.
 * gpio_num : physical ESP32 GPIO number (e.g. 12, 13, 14, 15).
 * freq_hz  : PWM output frequency in Hz.
 */
void PWM_initChannel(PWM_ChannelType channel, uint8 gpio_num, uint32 freq_hz);

/*
 * Description :
 * Set the duty cycle of a previously initialized PWM channel.
 * duty: 0 (always-off) to 255 (always-on), 8-bit resolution.
 */
void PWM_setDuty(PWM_ChannelType channel, uint8 duty);

#endif /* PWM_H_ */
