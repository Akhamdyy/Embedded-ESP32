/******************************************************************************
 *
 * Module: Motor Control
 *
 * File Name: motor.c
 *
 * Description: Source file for 4WD motor control (HAL layer).
 *              - Speed: PWM MCAL driver (pwm.h)
 *              - Direction: GPIO MCAL driver (gpio.h)
 *
 *******************************************************************************/

#include "motor.h"
#include "gpio.h"
#include "pwm.h"

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/
#define MOTOR_PWM_FREQ_HZ   1000U
#define MOTOR_COUNT         4U

/*******************************************************************************
 *                         Private Data Types                                  *
 *******************************************************************************/

/* Holds the pin configuration for one motor */
typedef struct
{
    PWM_ChannelType pwm_channel;
    uint8           pwm_gpio;

    uint8 in1_port;
    uint8 in1_pin;
    uint8 in2_port;
    uint8 in2_pin;
} Motor_Config;

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/*
 *  MOTOR_FRONT_LEFT  [0]: PWM=GPIO12, IN1=GPIO16(PORTB,P0), IN2=GPIO17(PORTB,P1)
 *  MOTOR_REAR_LEFT   [1]: PWM=GPIO13, IN1=GPIO18(PORTB,P2), IN2=GPIO19(PORTB,P3)
 *  MOTOR_FRONT_RIGHT [2]: PWM=GPIO14, IN1=GPIO21(PORTB,P4), IN2=GPIO22(PORTB,P5)
 *  MOTOR_REAR_RIGHT  [3]: PWM=GPIO15, IN1=GPIO23(PORTB,P6), IN2=GPIO25(PORTB,P7)
 */
static const Motor_Config motor_cfg[MOTOR_COUNT] = {
    /* FRONT_LEFT  */ { PWM_CHANNEL_0, 12, PORTB_ID, PIN0_ID, PORTB_ID, PIN1_ID },
    /* REAR_LEFT   */ { PWM_CHANNEL_1, 13, PORTB_ID, PIN2_ID, PORTB_ID, PIN3_ID },
    /* FRONT_RIGHT */ { PWM_CHANNEL_2, 14, PORTB_ID, PIN4_ID, PORTB_ID, PIN5_ID },
    /* REAR_RIGHT  */ { PWM_CHANNEL_3, 15, PORTB_ID, PIN6_ID, PORTB_ID, PIN7_ID },
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

    for (i = 0; i < MOTOR_COUNT; i++)
    {
        const Motor_Config *cfg = &motor_cfg[i];

        PWM_initChannel(cfg->pwm_channel, cfg->pwm_gpio, MOTOR_PWM_FREQ_HZ);

        GPIO_setupPinDirection(cfg->in1_port, cfg->in1_pin, PIN_OUTPUT);
        GPIO_setupPinDirection(cfg->in2_port, cfg->in2_pin, PIN_OUTPUT);

        /* Start with active brake */
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
    PWM_setDuty(motor_cfg[motor].pwm_channel, speed);
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
    PWM_setDuty(motor_cfg[motor].pwm_channel, speed);
}

/*
 * Description :
 * Coast stop a specific motor.
 */
void Motor_stop(Motor_ID motor)
{
    PWM_setDuty(motor_cfg[motor].pwm_channel, MOTOR_MIN_SPEED);
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
