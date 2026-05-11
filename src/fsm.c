/******************************************************************************
 *
 * Module: FSM (App)
 *
 * File Name: fsm.c
 *
 * Description: Autonomous corridor-following FSM.
 *
 *  Transition table (priority top → bottom within each state):
 *
 *  IDLE          always                               → WALL_FOLLOW
 *
 *  WALL_FOLLOW   BT 'X'                               → STOP
 *                front < 25 cm (≥3 consecutive ticks)
 *                  left ≥ right                       → TURN_LEFT
 *                  right > left                       → TURN_RIGHT
 *                left > 45 AND right > 45             → WALL_LOST
 *                left < 7  OR  right < 7              → RECENTER
 *                otherwise                            → WALL_FOLLOW (PID)
 *
 *  RECENTER      BT 'X'                               → STOP
 *                left > 45 AND right > 45             → WALL_LOST
 *                |left − right| < 3 cm AND safe       → WALL_FOLLOW
 *                otherwise                            → RECENTER (PID slow)
 *
 *  TURN_LEFT     turn complete + brake + settle        → RECENTER
 *  TURN_RIGHT    turn complete + brake + settle        → RECENTER
 *
 *  WALL_LOST     BT 'X'                               → STOP
 *                left < 35  OR  right < 35            → WALL_FOLLOW
 *                timeout (3 s)                        → STOP
 *                otherwise (rotate in place)          → WALL_LOST
 *
 *  STOP          BT 'R'                               → IDLE
 *
 *  Three real-world issues addressed by design:
 *   1. Overturning  → Motor_brakeAll() + 150 ms hold after MPU6050_turn()
 *   2. No turn fired → sensors read FIRST; front threshold = 25 cm; hysteresis
 *   3. Re-triggers  → TURN always exits to RECENTER; RECENTER has no TURN exits
 *
 *******************************************************************************/

#include "fsm.h"
#include "motor.h"
#include "ultrasonic.h"
#include "pid.h"
#include "mpu6050.h"
#include "bluetooth.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>

/*******************************************************************************
 *                         Tunable Constants                                   *
 *******************************************************************************/

#define FSM_TICK_MS              20u     /* main loop period (50 Hz)           */
#define FSM_FRONT_OBSTACLE_CM    35u     /* front < this → plan a turn  (was 25, raised for earlier detection) */
#define FSM_TURN_HYSTERESIS      3u      /* N consecutive ticks before turning  */
#define FSM_TOO_CLOSE_CM         13u     /* side < this → RECENTER (sensors 10 cm from car edge; was 7, unreachable) */
#define FSM_WALL_LOST_CM         50u     /* both sides > this → WALL_LOST (was 45) */
#define FSM_WALL_FOUND_CM        40u     /* any side < this → wall re-acquired (was 35) */
#define FSM_CENTERED_TOLERANCE   8u      /* |L-R| < this cm → centered (was 3, too tight for sensor noise) */
#define FSM_RECENTER_MAX_MS      800u    /* force-exit RECENTER after this time regardless of tolerance */
#define FSM_WALL_LOST_TIMEOUT_MS 3000u   /* no wall → STOP after this many ms  */
#define FSM_POST_TURN_BRAKE_MS   200u    /* active-brake hold after each turn (was 150) */
#define FSM_BASE_SPEED           55u     /* PID base speed in WALL_FOLLOW (was 80, too fast) */
#define FSM_RECENTER_SPEED       35u     /* PID base speed in RECENTER (was 50) */
#define FSM_TURN_SPEED           60u     /* speed passed to MPU6050_turn() (was 70) */
#define FSM_WALL_LOST_ROT_SPEED  35u     /* rotate-in-place during WALL_LOST (was 45) */
#define FSM_ULTRASONIC_PERIOD_MS 60u     /* HC-SR04 measurement period          */

#define FSM_LOST_TIMEOUT_TICKS    (FSM_WALL_LOST_TIMEOUT_MS / FSM_TICK_MS)
#define FSM_RECENTER_MAX_TICKS    (FSM_RECENTER_MAX_MS      / FSM_TICK_MS)

/*******************************************************************************
 *                         PID Configurations                                  *
 *******************************************************************************/

/* error = left_dist - right_dist; positive → arc left, negative → arc right  */
static const PID_Config s_wall_follow_cfg = { 2.0f, 0.0f, 2.0f, FSM_BASE_SPEED,    12u };
static const PID_Config s_recenter_cfg    = { 2.0f, 0.0f, 2.0f, FSM_RECENTER_SPEED, 12u };

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static FSM_StateID s_state;
static uint32      s_lost_ticks;     /* ticks elapsed in WALL_LOST            */
static uint32      s_recenter_ticks; /* ticks elapsed in RECENTER             */
static uint8       s_front_hits;     /* consecutive ticks with front obstacle */
static uint32      s_turn_count;     /* total turns executed (telemetry)      */

static const char *const k_state_str[] = {
    "IDLE", "WALL_FOLLOW", "RECENTER",
    "TURN_LEFT", "TURN_RIGHT", "WALL_LOST", "STOP"
};

/*******************************************************************************
 *                         Private Helpers                                     *
 *******************************************************************************/

/* Substitute OOR with 400 cm so all arithmetic stays valid */
static uint16 prv_read_cm(Ultrasonic_SensorID id)
{
    uint16 d = Ultrasonic_getDistance(id);
    return (d == ULTRASONIC_OUT_OF_RANGE) ? 400u : d;
}

/* Read one Bluetooth command byte; return 0 if nothing pending */
static uint8 prv_bt_cmd(void)
{
    uint8 buf[4];
    return (bluetooth_recv(buf, 1u) > 0u) ? buf[0] : 0u;
}

/* Absolute difference between two uint16 values */
static uint16 prv_abs_diff(uint16 a, uint16 b)
{
    return (a >= b) ? (a - b) : (b - a);
}

/* Run entry actions for a state and update s_state */
static void prv_transition(FSM_StateID next)
{
    if (next == s_state) return;

    printf("[FSM] %s → %s\n",
           k_state_str[(uint8)s_state],
           k_state_str[(uint8)next]);

    switch (next)
    {
        case FSM_STATE_IDLE:
            Motor_brakeAll();
            break;

        case FSM_STATE_WALL_FOLLOW:
            PID_init(&s_wall_follow_cfg);
            PID_reset();
            s_front_hits = 0u;
            bluetooth_send("[FSM] WALL_FOLLOW\n");
            break;

        case FSM_STATE_RECENTER:
            PID_init(&s_recenter_cfg);
            PID_reset();
            s_front_hits    = 0u;
            s_recenter_ticks = 0u;
            bluetooth_send("[FSM] RECENTER\n");
            break;

        case FSM_STATE_TURN_LEFT:
            Motor_brakeAll();
            s_turn_count++;
            bluetooth_send("[FSM] TURN_LEFT\n");
            break;

        case FSM_STATE_TURN_RIGHT:
            Motor_brakeAll();
            s_turn_count++;
            bluetooth_send("[FSM] TURN_RIGHT\n");
            break;

        case FSM_STATE_WALL_LOST:
            Motor_brakeAll();
            s_lost_ticks = 0u;
            bluetooth_send("[FSM] WALL_LOST\n");
            break;

        case FSM_STATE_STOP:
        {
            char msg[48];
            Motor_brakeAll();
            (void)snprintf(msg, sizeof(msg),
                           "[FSM] STOP turns=%lu\n",
                           (unsigned long)s_turn_count);
            bluetooth_send(msg);
            printf("%s", msg);
            break;
        }

        default:
            break;
    }

    s_state = next;
}

/*******************************************************************************
 *                         State Handlers                                      *
 *******************************************************************************/

static void prv_state_idle(void)
{
    Motor_brakeAll();
    prv_transition(FSM_STATE_WALL_FOLLOW);   /* auto-start */
}

static void prv_state_wall_follow(uint16 f, uint16 l, uint16 r, uint8 cmd)
{
    /* P1: Bluetooth emergency stop */
    if (cmd == 'X' || cmd == 'x')
    {
        prv_transition(FSM_STATE_STOP);
        return;
    }

    /* P2: Front obstacle with hysteresis (evaluated BEFORE wall-lost check so a
     *     front wall in open space still causes a turn rather than WALL_LOST)  */
    if (f < FSM_FRONT_OBSTACLE_CM)
    {
        if (++s_front_hits >= FSM_TURN_HYSTERESIS)
        {
            s_front_hits = 0u;
            prv_transition((l >= r) ? FSM_STATE_TURN_LEFT : FSM_STATE_TURN_RIGHT);
            return;
        }
    }
    else
    {
        s_front_hits = 0u;
    }

    /* P3: Both side walls absent → WALL_LOST */
    if (l > FSM_WALL_LOST_CM && r > FSM_WALL_LOST_CM)
    {
        prv_transition(FSM_STATE_WALL_LOST);
        return;
    }

    /* P4: Dangerously close to a side wall → RECENTER */
    if (l < FSM_TOO_CLOSE_CM || r < FSM_TOO_CLOSE_CM)
    {
        prv_transition(FSM_STATE_RECENTER);
        return;
    }

    /* Default: PID handles motor control and centering */
    PID_update((float32)FSM_TICK_MS / 1000.0f);
}

static void prv_state_recenter(uint16 f, uint16 l, uint16 r, uint8 cmd)
{
    (void)f;   /* front not used in RECENTER — no TURN exits */

    /* P1: Bluetooth emergency stop */
    if (cmd == 'X' || cmd == 'x')
    {
        prv_transition(FSM_STATE_STOP);
        return;
    }

    /* P2: Both walls lost */
    if (l > FSM_WALL_LOST_CM && r > FSM_WALL_LOST_CM)
    {
        prv_transition(FSM_STATE_WALL_LOST);
        return;
    }

    /* P3: Centred enough and not dangerously close → resume WALL_FOLLOW */
    if (prv_abs_diff(l, r) < FSM_CENTERED_TOLERANCE &&
        l > FSM_TOO_CLOSE_CM &&
        r > FSM_TOO_CLOSE_CM)
    {
        prv_transition(FSM_STATE_WALL_FOLLOW);
        return;
    }

    /* P4: Timeout — force exit even if not perfectly centred.
     *     Prevents robot getting stuck in RECENTER indefinitely due to
     *     sensor noise or asymmetric corridor geometry post-turn.        */
    if (++s_recenter_ticks >= FSM_RECENTER_MAX_TICKS)
    {
        prv_transition(FSM_STATE_WALL_FOLLOW);
        return;
    }

    /* Default: PID at reduced base speed; NO turn exits from here */
    PID_update((float32)FSM_TICK_MS / 1000.0f);
}

static void prv_state_turn(MPU6050_TurnDir dir)
{
    /* Blocking — does not return until the gyro reports ~83° */
    MPU6050_turn(dir, FSM_TURN_SPEED);

    /* Active brake arrests mechanical overshoot (fixes overturning) */
    Motor_brakeAll();
    vTaskDelay(pdMS_TO_TICKS(FSM_POST_TURN_BRAKE_MS));

    /* Always enter RECENTER after a turn — prevents immediate re-trigger
     * because RECENTER has no transitions to TURN states              */
    prv_transition(FSM_STATE_RECENTER);
}

static void prv_state_wall_lost(uint16 f, uint16 l, uint16 r, uint8 cmd)
{
    (void)f;

    /* P1: Bluetooth emergency stop */
    if (cmd == 'X' || cmd == 'x')
    {
        prv_transition(FSM_STATE_STOP);
        return;
    }

    /* P2: Wall re-acquired on either side */
    if (l < FSM_WALL_FOUND_CM || r < FSM_WALL_FOUND_CM)
    {
        prv_transition(FSM_STATE_WALL_FOLLOW);
        return;
    }

    /* P3: Timeout → give up and stop */
    if (++s_lost_ticks >= FSM_LOST_TIMEOUT_TICKS)
    {
        prv_transition(FSM_STATE_STOP);
        return;
    }

    /* Default: rotate slowly to scan for walls */
    Car_turnLeft(FSM_WALL_LOST_ROT_SPEED, FSM_WALL_LOST_ROT_SPEED);
}

static void prv_state_stop(uint8 cmd)
{
    Motor_brakeAll();

    /* Resume on 'R' — goes through IDLE (re-runs auto-start) */
    if (cmd == 'R' || cmd == 'r')
        prv_transition(FSM_STATE_IDLE);
}

/*******************************************************************************
 *                          Public Functions                                   *
 *******************************************************************************/

void FSM_init(void)
{
    s_state          = FSM_STATE_IDLE;
    s_lost_ticks     = 0u;
    s_recenter_ticks = 0u;
    s_front_hits     = 0u;
    s_turn_count     = 0u;

    Motor_init();
    Ultrasonic_initAll(FSM_ULTRASONIC_PERIOD_MS);
    bluetooth_init("wallrobot");

    printf("[FSM] MPU6050 init...\n");
    MPU6050_init();
    printf("[FSM] Calibrating gyro — keep robot still...\n");
    MPU6050_calibrate();

    PID_init(&s_wall_follow_cfg);

    printf("[FSM] Ready. Starting in 1 s...\n");
    vTaskDelay(pdMS_TO_TICKS(1000u));
}

void FSM_run(void)
{
    uint16 f, l, r;
    uint8  cmd;

    for (;;)
    {
        /* Step 1 — read all sensors BEFORE evaluating any transition */
        f   = prv_read_cm(ULTRASONIC_FRONT);
        l   = prv_read_cm(ULTRASONIC_LEFT);
        r   = prv_read_cm(ULTRASONIC_RIGHT);
        cmd = prv_bt_cmd();

        /* Step 2 — evaluate transitions and execute state action */
        switch (s_state)
        {
            case FSM_STATE_IDLE:
                prv_state_idle();
                break;

            case FSM_STATE_WALL_FOLLOW:
                prv_state_wall_follow(f, l, r, cmd);
                break;

            case FSM_STATE_RECENTER:
                prv_state_recenter(f, l, r, cmd);
                break;

            case FSM_STATE_TURN_LEFT:
                prv_state_turn(MPU6050_TURN_LEFT);
                break;

            case FSM_STATE_TURN_RIGHT:
                prv_state_turn(MPU6050_TURN_RIGHT);
                break;

            case FSM_STATE_WALL_LOST:
                prv_state_wall_lost(f, l, r, cmd);
                break;

            case FSM_STATE_STOP:
                prv_state_stop(cmd);
                break;

            default:
                prv_transition(FSM_STATE_IDLE);
                break;
        }

        /* Step 3 — fixed tick delay (TURN states have their own blocking delay) */
        vTaskDelay(pdMS_TO_TICKS(FSM_TICK_MS));
    }
}

FSM_StateID FSM_getState(void)
{
    return s_state;
}
