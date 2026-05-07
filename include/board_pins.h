/******************************************************************************
 *
 * Module: Board Pin Configuration
 *
 * File Name: board_pins.h
 *
 * Description: Centralized pin and peripheral channel assignments for the
 *              ESP32 DOIT DevKit V1 robot car board.
 *
 *              This is the ONLY file that may contain hardware pin numbers.
 *              All HAL drivers (motor, ultrasonic, mpu6050) must import pin
 *              constants from here instead of hardcoding values locally.
 *
 *              Changing wiring only requires edits in this file.
 *
 * Peripherals covered:
 *   - 4x DC motor H-bridge (L298N x2, LEDC PWM + GPIO direction)
 *   - 3x HC-SR04 ultrasonic sensors (GPIO trigger + interrupt echo)
 *   - MPU-6050 IMU (I2C)
 *
 *******************************************************************************/

#ifndef BOARD_PINS_H_
#define BOARD_PINS_H_

#include "gpio.h"   /* PORT/PIN ID definitions                        */
#include "pwm.h"    /* Pwm_ChannelID, Pwm_TimerID                     */

/*******************************************************************************
 *                     Motor PWM — LEDC timer shared config                    *
 *******************************************************************************/

#define MOTOR_PWM_TIMER        PWM_TIMER_0
#define MOTOR_PWM_FREQ_HZ      1000u
#define MOTOR_PWM_DUTY_BITS    8u           /* 8-bit: duty range 0-255 */

/*******************************************************************************
 *                     Motor PWM — per-motor GPIO and channel                  *
 *                                                                              *
 *  H-Bridge 1 (Left):  ENA=GPIO12, ENB=GPIO13                                 *
 *  H-Bridge 2 (Right): ENA=GPIO14, ENB=GPIO15                                 *
 *******************************************************************************/

/* Front Left motor */
#define MOTOR_FL_PWM_GPIO      12
#define MOTOR_FL_PWM_CHANNEL   PWM_CHANNEL_0

/* Rear Left motor */
#define MOTOR_RL_PWM_GPIO      13
#define MOTOR_RL_PWM_CHANNEL   PWM_CHANNEL_1

/* Front Right motor */
#define MOTOR_FR_PWM_GPIO      14
#define MOTOR_FR_PWM_CHANNEL   PWM_CHANNEL_2

/* Rear Right motor */
#define MOTOR_RR_PWM_GPIO      15
#define MOTOR_RR_PWM_CHANNEL   PWM_CHANNEL_3

/*******************************************************************************
 *                     Motor Direction — IN1/IN2 GPIO (PORTB)                  *
 *                                                                              *
 *  H-Bridge 1:  IN1=GPIO16, IN2=GPIO17, IN3=GPIO18, IN4=GPIO19                *
 *  H-Bridge 2:  IN1=GPIO21, IN2=GPIO22, IN3=GPIO23, IN4=GPIO25                *
 *******************************************************************************/

/* Front Left: IN1=GPIO16 (PORTB P0), IN2=GPIO17 (PORTB P1) */
#define MOTOR_FL_IN1_PORT      PORTB_ID
#define MOTOR_FL_IN1_PIN       PIN0_ID
#define MOTOR_FL_IN2_PORT      PORTB_ID
#define MOTOR_FL_IN2_PIN       PIN1_ID

/* Rear Left: IN3=GPIO18 (PORTB P2), IN4=GPIO19 (PORTB P3) */
#define MOTOR_RL_IN1_PORT      PORTB_ID
#define MOTOR_RL_IN1_PIN       PIN2_ID
#define MOTOR_RL_IN2_PORT      PORTB_ID
#define MOTOR_RL_IN2_PIN       PIN3_ID

/* Front Right: IN1=GPIO21 (PORTB P4), IN2=GPIO22 (PORTB P5) */
#define MOTOR_FR_IN1_PORT      PORTB_ID
#define MOTOR_FR_IN1_PIN       PIN4_ID
#define MOTOR_FR_IN2_PORT      PORTB_ID
#define MOTOR_FR_IN2_PIN       PIN5_ID

/* Rear Right: IN3=GPIO23 (PORTB P6), IN4=GPIO25 (PORTB P7) */
#define MOTOR_RR_IN1_PORT      PORTB_ID
#define MOTOR_RR_IN1_PIN       PIN6_ID
#define MOTOR_RR_IN2_PORT      PORTB_ID
#define MOTOR_RR_IN2_PIN       PIN7_ID

/*******************************************************************************
 *                     Ultrasonic Sensors — HC-SR04                            *
 *                                                                              *
 *  Echo pins are input-only GPIOs (34/35/36). No pull resistors available.    *
 *  A 1kΩ/2kΩ voltage divider must be placed on each echo line (5V→3.3V).     *
 *******************************************************************************/

/* Front sensor */
#define US_FRONT_TRIG_GPIO     26
#define US_FRONT_ECHO_GPIO     34

/* Left sensor */
#define US_LEFT_TRIG_GPIO      32
#define US_LEFT_ECHO_GPIO      35

/* Right sensor */
#define US_RIGHT_TRIG_GPIO     4
#define US_RIGHT_ECHO_GPIO     36

/*******************************************************************************
 *                     MPU-6050 — I2C                                          *
 *                                                                              *
 *  AD0 tied to GND → I2C address 0x68                                         *
 *******************************************************************************/

#define MPU6050_I2C_SDA_GPIO   27   /* PORTC PIN1 */
#define MPU6050_I2C_SCL_GPIO   33   /* PORTC PIN3 */
#define MPU6050_I2C_FREQ_HZ    400000u
#define MPU6050_I2C_ADDR       0x68u

#endif /* BOARD_PINS_H_ */
