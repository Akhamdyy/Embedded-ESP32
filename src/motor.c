/******************************************************************************
 *
 * Module: Motor Control (HAL)
 *
 * File Name: motor.c
 *
 * Description: HAL source file for 4WD motor control.
 *              - Speed: Pwm_setDuty() via MCAL pwm driver (no LEDC direct use)
 *              - Direction: GPIO_writePin() via MCAL gpio driver
 *              - Pin/channel mapping: board_pins.h (single source of truth)
 *
 *******************************************************************************/

#include "motor.h"
#include "gpio.h"
#include "pwm.h"
#include "board_pins.h"

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define MOTOR_COUNT     4

/*******************************************************************************
 *                         Private Data Types                                  *
 *******************************************************************************/

/* Per-motor static configuration resolved from board_pins.h */
typedef struct
{
    Pwm_ChannelID pwm_channel;
    int           pwm_gpio;
    uint8         in1_port;
    uint8         in1_pin;
    uint8         in2_port;
    uint8         in2_pin;
} Motor_Config;

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/*
 * Motor configuration table — all pin/channel assignments come from
 * board_pins.h. Changing wiring only requires editing that one file.
 *
 * Index matches Motor_ID enum values.
 */
static const Motor_Config motor_cfg[MOTOR_COUNT] = {
    /* FRONT_LEFT  */ { MOTOR_FL_PWM_CHANNEL, MOTOR_FL_PWM_GPIO,
                        MOTOR_FL_IN1_PORT, MOTOR_FL_IN1_PIN,
                        MOTOR_FL_IN2_PORT, MOTOR_FL_IN2_PIN },
    /* REAR_LEFT   */ { MOTOR_RL_PWM_CHANNEL, MOTOR_RL_PWM_GPIO,
                        MOTOR_RL_IN1_PORT, MOTOR_RL_IN1_PIN,
                        MOTOR_RL_IN2_PORT, MOTOR_RL_IN2_PIN },
    /* FRONT_RIGHT */ { MOTOR_FR_PWM_CHANNEL, MOTOR_FR_PWM_GPIO,
                        MOTOR_FR_IN1_PORT, MOTOR_FR_IN1_PIN,
                        MOTOR_FR_IN2_PORT, MOTOR_FR_IN2_PIN },
    /* REAR_RIGHT  */ { MOTOR_RR_PWM_CHANNEL, MOTOR_RR_PWM_GPIO,
                        MOTOR_RR_IN1_PORT, MOTOR_RR_IN1_PIN,
                        MOTOR_RR_IN2_PORT, MOTOR_RR_IN2_PIN },
};

/*******************************************************************************
 *                         Private Helper Functions                            *
 *******************************************************************************/

static void Motor_applyDirection(Motor_ID motor, Motor_Direction direction)
{
    const Motor_Config *cfg = &motor_cfg[motor];

    switch (direction)
    {
    case MOTOR_FORWARD:
        GPIO_writePin(cfg->in1_port, cfg->in1_pin, LOGIC_HIGH);
        GPIO_writePin(cfg->in2_port, cfg->in2_pin, LOGIC_LOW);
        break;

    case MOTOR_BACKWARD:
        GPIO_writePin(cfg->in1_port, cfg->in1_pin, LOGIC_LOW);
        GPIO_writePin(cfg->in2_port, cfg->in2_pin, LOGIC_HIGH);
        break;

    case MOTOR_BRAKE:
        /* Both IN pins HIGH = active brake on L298N */
        GPIO_writePin(cfg->in1_port, cfg->in1_pin, LOGIC_HIGH);
        GPIO_writePin(cfg->in2_port, cfg->in2_pin, LOGIC_HIGH);
        break;

    case MOTOR_STOP:
    default:
        GPIO_writePin(cfg->in1_port, cfg->in1_pin, LOGIC_LOW);
        GPIO_writePin(cfg->in2_port, cfg->in2_pin, LOGIC_LOW);
        break;
    }
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize all 4 motors: configure PWM channels and direction GPIO pins.
 */
void Motor_init(void)
{
    uint8 i;

    /* Configure the shared PWM timer via MCAL */
    const Pwm_TimerConfig timer_cfg = {
        .timer     = MOTOR_PWM_TIMER,
        .freq_hz   = MOTOR_PWM_FREQ_HZ,
        .duty_bits = MOTOR_PWM_DUTY_BITS,
    };
    Pwm_timerInit(&timer_cfg);

    /* Configure each motor's PWM channel and direction GPIO pins */
    for (i = 0; i < MOTOR_COUNT; i++)
    {
        const Motor_Config *cfg = &motor_cfg[i];

        /* PWM channel via MCAL — no LEDC types visible here */
        const Pwm_ChannelConfig ch_cfg = {
            .channel  = cfg->pwm_channel,
            .timer    = MOTOR_PWM_TIMER,
            .gpio_num = cfg->pwm_gpio,
        };
        Pwm_channelInit(&ch_cfg);

        /* Direction pins via MCAL GPIO */
        GPIO_setupPinDirection(cfg->in1_port, cfg->in1_pin, PIN_OUTPUT);
        GPIO_setupPinDirection(cfg->in2_port, cfg->in2_pin, PIN_OUTPUT);

        /* Start braked (both IN pins HIGH) */
        GPIO_writePin(cfg->in1_port, cfg->in1_pin, LOGIC_HIGH);
        GPIO_writePin(cfg->in2_port, cfg->in2_pin, LOGIC_HIGH);
    }
}

/*
 * Description :
 * Set the speed of a specific motor (0-255).
 */
void Motor_setSpeed(Motor_ID motor, uint8 speed)
{
    Pwm_setDuty(motor_cfg[motor].pwm_channel, speed);
}

/*
 * Description :
 * Set the direction of a specific motor without changing its speed.
 */
void Motor_setDirection(Motor_ID motor, Motor_Direction direction)
{
    Motor_applyDirection(motor, direction);
}

/*
 * Description :
 * Drive a motor at a given speed in a given direction.
 */
void Motor_drive(Motor_ID motor, Motor_Direction direction, uint8 speed)
{
    Motor_applyDirection(motor, direction);
    Pwm_setDuty(motor_cfg[motor].pwm_channel, speed);
}

/*
 * Description :
 * Coast stop a specific motor.
 */
void Motor_stop(Motor_ID motor)
{
    Pwm_setDuty(motor_cfg[motor].pwm_channel, MOTOR_MIN_SPEED);
    Motor_applyDirection(motor, MOTOR_STOP);
}

/*
 * Description :
 * Coast stop all 4 motors.
 */
void Motor_stopAll(void)
{
    uint8 i;
    for (i = 0; i < MOTOR_COUNT; i++)
    {
        Motor_stop((Motor_ID)i);
    }
}

/*
 * Description :
 * Drive all 4 motors at the same speed and direction.
 */
void Motor_driveAll(Motor_Direction direction, uint8 speed)
{
    uint8 i;
    for (i = 0; i < MOTOR_COUNT; i++)
    {
        Motor_drive((Motor_ID)i, direction, speed);
    }
}

/*
 * Description :
 * Active brake all 4 motors.
 */
void Motor_brakeAll(void)
{
    uint8 i;
    for (i = 0; i < MOTOR_COUNT; i++)
    {
        Motor_setSpeed((Motor_ID)i, MOTOR_MIN_SPEED);
        Motor_setDirection((Motor_ID)i, MOTOR_BRAKE);
    }
}

/*
 * Description :
 * Move car forward. Left/right speeds set independently for drift correction.
 */
void Car_moveForward(uint8 left_speed, uint8 right_speed)
{
    Motor_drive(MOTOR_FRONT_LEFT,  MOTOR_FORWARD, left_speed);
    Motor_drive(MOTOR_REAR_LEFT,   MOTOR_FORWARD, left_speed);
    Motor_drive(MOTOR_FRONT_RIGHT, MOTOR_FORWARD, right_speed);
    Motor_drive(MOTOR_REAR_RIGHT,  MOTOR_FORWARD, right_speed);
}

/*
 * Description :
 * Move car backward. Left/right speeds set independently.
 */
void Car_moveBackward(uint8 left_speed, uint8 right_speed)
{
    Motor_drive(MOTOR_FRONT_LEFT,  MOTOR_BACKWARD, left_speed);
    Motor_drive(MOTOR_REAR_LEFT,   MOTOR_BACKWARD, left_speed);
    Motor_drive(MOTOR_FRONT_RIGHT, MOTOR_BACKWARD, right_speed);
    Motor_drive(MOTOR_REAR_RIGHT,  MOTOR_BACKWARD, right_speed);
}

/*
 * Description :
 * Turn left: left side slower, right side faster (differential drive).
 * Pass (left_speed < right_speed) for a gradual curve.
 */
void Car_turnLeft(uint8 left_speed, uint8 right_speed)
{
    Car_moveForward(left_speed, right_speed);
}

/*
 * Description :
 * Turn right: right side slower, left side faster (differential drive).
 * Pass (right_speed < left_speed) for a gradual curve.
 */
void Car_turnRight(uint8 left_speed, uint8 right_speed)
{
    Car_moveForward(left_speed, right_speed);
}
