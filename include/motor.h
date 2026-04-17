/******************************************************************************
 *
 * Module: Motor Control
 *
 * File Name: motor.h
 *
 * Description: Header file for 4WD motor control using dual H-bridge drivers.
 *              PWM speed control is handled via the ESP32 LEDC peripheral.
 *              Direction control uses the project GPIO driver.
 *
 * Wiring (per H-bridge, e.g. L298N):
 *
 *   H-Bridge 1 (Left side):
 *     ENA (PWM)  -> GPIO 12   (PORTA PIN4) - Front Left speed
 *     IN1        -> GPIO 16   (PORTB PIN0) - Front Left dir
 *     IN2        -> GPIO 17   (PORTB PIN1) - Front Left dir
 *     ENB (PWM)  -> GPIO 13   (PORTA PIN5) - Rear Left speed
 *     IN3        -> GPIO 18   (PORTB PIN2) - Rear Left dir
 *     IN4        -> GPIO 19   (PORTB PIN3) - Rear Left dir
 *
 *   H-Bridge 2 (Right side):
 *     ENA (PWM)  -> GPIO 14   (PORTA PIN6) - Front Right speed
 *     IN1        -> GPIO 21   (PORTB PIN4) - Front Right dir
 *     IN2        -> GPIO 22   (PORTB PIN5) - Front Right dir
 *     ENB (PWM)  -> GPIO 15   (PORTA PIN7) - Rear Right speed
 *     IN3        -> GPIO 23   (PORTB PIN6) - Rear Right dir
 *     IN4        -> GPIO 25   (PORTB PIN7) - Rear Right dir
 *
 * PWM: 1 kHz, 8-bit resolution (speed range 0-255)
 *
 *******************************************************************************/

#ifndef MOTOR_H_
#define MOTOR_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define MOTOR_MAX_SPEED     255
#define MOTOR_MIN_SPEED     0

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
    MOTOR_FRONT_LEFT  = 0,
    MOTOR_REAR_LEFT   = 1,
    MOTOR_FRONT_RIGHT = 2,
    MOTOR_REAR_RIGHT  = 3
} Motor_ID;

typedef enum
{
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_STOP,    /* Coast - motor spins freely */
    MOTOR_BRAKE    /* Active brake - motor resists movement */
} Motor_Direction;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize all 4 motors: configure LEDC PWM channels and direction GPIO pins.
 * Must be called once before any other motor function.
 */
void Motor_init(void);

/*
 * Description :
 * Set the speed of a specific motor.
 * speed: 0 (stopped) to 255 (full speed)
 */
void Motor_setSpeed(Motor_ID motor, uint8 speed);

/*
 * Description :
 * Set the direction of a specific motor without changing its speed.
 */
void Motor_setDirection(Motor_ID motor, Motor_Direction direction);

/*
 * Description :
 * Drive a motor at a given speed in a given direction in one call.
 * speed: 0-255
 */
void Motor_drive(Motor_ID motor, Motor_Direction direction, uint8 speed);

/*
 * Description :
 * Coast stop a specific motor (PWM = 0, direction pins LOW).
 */
void Motor_stop(Motor_ID motor);

/*
 * Description :
 * Coast stop all 4 motors.
 */
void Motor_stopAll(void);

/*
 * Description :
 * Drive all 4 motors at the same speed and direction.
 * Useful for straight forward/backward movement.
 */
void Motor_driveAll(Motor_Direction direction, uint8 speed);

#endif /* MOTOR_H_ */
