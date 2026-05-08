/******************************************************************************
 *
 * Module: PID Lane Centering
 *
 * File Name: pid.c
 *
 * Description: Source file for the PID lane-centering controller.
 *              Reads left/right ultrasonic distances, computes a PID
 *              correction, and applies it as a differential speed to the
 *              left and right motors while driving forward.
 *
 *              error  = left_dist - right_dist
 *                       positive → car too close to right wall → arc left
 *                       negative → car too close to left wall  → arc right
 *
 *              left_speed  = base_speed - output
 *              right_speed = base_speed + output
 *
 *******************************************************************************/

#include "pid.h"
#include "ultrasonic.h"
#include "motor.h"
#include <math.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

/* Maximum correction applied to either side (PWM counts) */
#define PID_MAX_OUTPUT      80.0f

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static PID_Config pid_cfg;
static float32    integral   = 0.0f;
static float32    prev_error = 0.0f;
static boolean    initialised = FALSE;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static float32 clamp(float32 val, float32 min_val, float32 max_val)
{
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

static uint8 clamp_speed(float32 val)
{
    if (val < 0.0f)   return 0u;
    if (val > 255.0f) return 255u;
    return (uint8)val;
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize the PID controller with the given configuration.
 */
void PID_init(const PID_Config *cfg)
{
    pid_cfg     = *cfg;
    integral    = 0.0f;
    prev_error  = 0.0f;
    initialised = TRUE;
}

/*
 * Description :
 * Run one PID iteration and apply the result to the motors.
 */
void PID_update(float32 dt_s)
{
    if (!initialised || dt_s <= 0.0f)
    {
        return;
    }

    uint16 left_dist  = Ultrasonic_getDistance(ULTRASONIC_LEFT);
    uint16 right_dist = Ultrasonic_getDistance(ULTRASONIC_RIGHT);

    /* Treat out-of-range as clear — use a large value so no correction fires */
    if (left_dist  == ULTRASONIC_OUT_OF_RANGE) left_dist  = 400u;
    if (right_dist == ULTRASONIC_OUT_OF_RANGE) right_dist = 400u;

    /* Only correct when at least one side is inside the threshold */
    if (left_dist >= pid_cfg.threshold_cm && right_dist >= pid_cfg.threshold_cm)
    {
        Car_moveForward(pid_cfg.base_speed, pid_cfg.base_speed);
        PID_reset();
        return;
    }

    /* --- PID computation --- */
    float32 error      = (float32)left_dist - (float32)right_dist;
    integral          += error * dt_s;
    float32 derivative = (error - prev_error) / dt_s;
    prev_error         = error;

    float32 output = (pid_cfg.kp * error)
                   + (pid_cfg.ki * integral)
                   + (pid_cfg.kd * derivative);

    output = clamp(output, -PID_MAX_OUTPUT, PID_MAX_OUTPUT);

    uint8 left_speed  = clamp_speed((float32)pid_cfg.base_speed - output);
    uint8 right_speed = clamp_speed((float32)pid_cfg.base_speed + output);

    Car_moveForward(left_speed, right_speed);
}

/*
 * Description :
 * Reset integrator and derivative state.
 */
void PID_reset(void)
{
    integral   = 0.0f;
    prev_error = 0.0f;
}
