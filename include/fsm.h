/******************************************************************************
 *
 * Module: FSM (App)
 *
 * File Name: fsm.h
 *
 * Description: Finite State Machine for autonomous corridor navigation.
 *
 *  States
 *  ──────
 *  IDLE        → auto-starts into WALL_FOLLOW
 *  WALL_FOLLOW → PID centering; transitions on front obstacle / wall loss
 *  RECENTER    → post-turn settling at reduced speed; NO turn exits
 *  TURN_LEFT   → blocking 90° left turn via MPU6050_turn()
 *  TURN_RIGHT  → blocking 90° right turn via MPU6050_turn()
 *  WALL_LOST   → rotate-in-place to re-acquire; timeout → STOP
 *  STOP        → motors braked; resume on Bluetooth 'R'
 *
 *******************************************************************************/

#ifndef FSM_H_
#define FSM_H_

#include "std_types.h"

/*******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************/

typedef enum
{
    FSM_STATE_IDLE        = 0,
    FSM_STATE_WALL_FOLLOW = 1,
    FSM_STATE_RECENTER    = 2,
    FSM_STATE_TURN_LEFT   = 3,
    FSM_STATE_TURN_RIGHT  = 4,
    FSM_STATE_WALL_LOST   = 5,
    FSM_STATE_STOP        = 6
} FSM_StateID;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialise all hardware modules (motors, ultrasonics, gyro, Bluetooth),
 * calibrate the gyro zero-rate offset, and set the FSM to IDLE.
 * Must be called once before FSM_run().
 */
void FSM_init(void);

/*
 * Description :
 * Enter the FSM main loop. Reads sensors, evaluates transitions, and drives
 * motors every FSM_TICK_MS milliseconds. Never returns.
 */
void FSM_run(void);

/*
 * Description :
 * Return the current FSM state (for diagnostics / telemetry).
 */
FSM_StateID FSM_getState(void);

#endif /* FSM_H_ */
