/******************************************************************************
 *
 * Module: Finite State Machine
 *
 * File Name: fsm.h
 *
 * Description: Wall-following robot FSM.
 *
 *   IDLE ──init──► WALL_FOLLOW ◄──────────────────────────────────┐
 *                     │   ▲                                        │
 *              lost   │   │ re-found / turn done                  │
 *                     ▼   │                                        │
 *                 WALL_LOST ──5 s──► STOP               TURN_LEFT ┤
 *                                                       TURN_RIGHT ┘
 *   WALL_FOLLOW ──L>35,R>35──► REALIGN ──front──► TURN_LEFT / RIGHT
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
    FSM_STATE_WALL_FOLLOW,
    FSM_STATE_WALL_LOST,
    FSM_STATE_STOP,
    FSM_STATE_REALIGN,
    FSM_STATE_TURN_LEFT,
    FSM_STATE_TURN_RIGHT
} FSM_StateID;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

void FSM_init(void);
FSM_StateID FSM_getState(void);
void fsm_task(void *pv);

#endif /* FSM_H_ */
