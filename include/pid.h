/******************************************************************************
 *
 * Module: PID Lane Centering
 *
 * File Name: pid.h
 *
 * Description: Header file for the PID lane-centering controller.
 *              Uses left and right ultrasonic sensors to keep the car
 *              centered in a fixed-width track.
 *
 *              Error  = left_distance - right_distance
 *              Output = PID(error) → differential speed applied to motors
 *
 *              Correction only activates when either side drops below
 *              threshold_cm. Above threshold both sides run at base_speed.
 *
 *******************************************************************************/

#ifndef PID_H_
#define PID_H_

#include "std_types.h"

/*******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************/

typedef struct
{
    float32 kp;             /* Proportional gain                              */
    float32 ki;             /* Integral gain (set 0 to disable)               */
    float32 kd;             /* Derivative gain (set 0 to disable)             */
    uint8   base_speed;     /* Forward speed when centered (0-255)            */
    uint16  threshold_cm;   /* Activate correction when either side < this    */
} PID_Config;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize the PID controller with the given configuration.
 * Must be called after Ultrasonic_initAll() and Motor_init().
 */
void PID_init(const PID_Config *cfg);

/*
 * Description :
 * Run one PID iteration: read sensors, compute output, apply to motors.
 * Call periodically from the main loop at a fixed interval.
 * dt_s: time elapsed since last call in seconds.
 */
void PID_update(float32 dt_s);

/*
 * Description :
 * Reset the integrator and derivative state.
 * Call when resuming after a stop to prevent windup carry-over.
 */
void PID_reset(void);

#endif /* PID_H_ */
