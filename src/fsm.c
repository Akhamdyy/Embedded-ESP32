/******************************************************************************
 *
 * Module: Finite State Machine
 *
 * File Name: fsm.c
 *
 * Description: Wall-following robot FSM implementation.
 *
 * Transition table:
 *
 *  From          Condition                              To
 *  ─────────     ────────────────────────────────────── ─────────────
 *  IDLE          always (init complete)                 WALL_FOLLOW
 *  WALL_FOLLOW   all blank for 1 s                      WALL_LOST
 *  WALL_FOLLOW   F≤15, L>35                             TURN_LEFT
 *  WALL_FOLLOW   F≤15, R>35                             TURN_RIGHT
 *  WALL_FOLLOW   L>35 AND R>35 (sides open, F clear)    REALIGN
 *  WALL_FOLLOW   otherwise                              WALL_FOLLOW (PID)
 *  WALL_LOST     any sensor < 35 cm                     WALL_FOLLOW
 *  WALL_LOST     timer > 5 s                            STOP
 *  STOP          (terminal — power-cycle to restart)    STOP
 *  REALIGN       L≤35 OR R≤35 (wall reacquired)        WALL_FOLLOW
 *  REALIGN       F≤15, L≥R                              TURN_LEFT
 *  REALIGN       F≤15, R>L                              TURN_RIGHT
 *  REALIGN       all>35 (completely open)               WALL_LOST
 *  TURN_LEFT     turn complete (MPU6050 blocking)        WALL_FOLLOW
 *  TURN_RIGHT    turn complete (MPU6050 blocking)        WALL_FOLLOW
 *
 * Track: 45 cm wide, car: 20 cm wide → 12.5 cm clearance per side.
 * PID setpoint (threshold_cm) = 13 cm.
 *
 *******************************************************************************/

#include "fsm.h"
#include "motor.h"
#include "ultrasonic.h"
#include "pid.h"
#include "bluetooth.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

/*******************************************************************************
 *                         Tunable Constants                                   *
 *******************************************************************************/

#define FRONT_BLOCKED_CM    15u   /* front wall: too close to continue forward */
#define WALL_FAR_CM         35u   /* side / front: "wall lost" threshold        */
#define WALL_NEAR_CM        13u   /* PID setpoint — half of (45-20) cm         */
#define REALIGN_SPEED       30u   /* slow creep while re-centering             */
#define BASE_SPEED          80u
#define TURN_SPEED          90u
#define TICK_MS             50u   /* FSM update period (20 Hz)                 */
#define LOST_TIMEOUT_TICKS  (5000u / TICK_MS)   /* 5 s = 100 ticks            */
#define BLANK_DEBOUNCE      20u   /* ticks before declaring WALL_LOST (1 s)   */
#define TURN_DURATION_MS    650u  /* timed 90° substitute — tune on the bench */

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static const PID_Config k_pid = {
    .kp           = 2.0f,
    .ki           = 0.0f,
    .kd           = 2.0f,
    .base_speed   = BASE_SPEED,
    .threshold_cm = WALL_NEAR_CM
};

static FSM_StateID       s_state;
static uint32            s_turn_count;
static uint32            s_lost_ticks;

static const char *const k_state_str[] = {
    "IDLE", "WALL_FOLLOW", "WALL_LOST", "STOP", "REALIGN", "TURN_LEFT", "TURN_RIGHT"
};

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static uint16 read_cm(Ultrasonic_SensorID id)
{
    uint16 d = Ultrasonic_getDistance(id);
    return (d == ULTRASONIC_OUT_OF_RANGE) ? 400u : d;
}

static void on_entry(FSM_StateID s)
{
    char buf[64];

    printf("[FSM] ──► %s\n", k_state_str[s]);
    switch (s)
    {
        case FSM_STATE_IDLE:
            Motor_stopAll();
            break;

        case FSM_STATE_WALL_FOLLOW:
            PID_reset();
            bluetooth_send("[FSM] WALL_FOLLOW\n");
            break;

        case FSM_STATE_WALL_LOST:
            Motor_stopAll();
            s_lost_ticks = 0u;
            bluetooth_send("[FSM] WALL_LOST\n");
            break;

        case FSM_STATE_STOP:
            Motor_stopAll();
            (void)snprintf(buf, sizeof(buf),
                           "[FSM] STOP turns=%lu\n",
                           (unsigned long)s_turn_count);
            bluetooth_send(buf);
            printf("%s", buf);
            break;

        case FSM_STATE_REALIGN:
            Car_moveForward(REALIGN_SPEED, REALIGN_SPEED);
            bluetooth_send("[FSM] REALIGN\n");
            break;

        case FSM_STATE_TURN_LEFT:
            Motor_stopAll();
            s_turn_count++;
            bluetooth_send("[FSM] TURN_LEFT\n");
            break;

        case FSM_STATE_TURN_RIGHT:
            Motor_stopAll();
            s_turn_count++;
            bluetooth_send("[FSM] TURN_RIGHT\n");
            break;

        default:
            break;
    }
}

/* ---- State handlers: each returns the next FSM state ---- */

static FSM_StateID state_idle(void)
{
    Motor_init();
    Ultrasonic_initAll(60u);
    PID_init(&k_pid);
    bluetooth_init("ESP32-Car");
    printf("[FSM] Init done — entering WALL_FOLLOW\n");
    return FSM_STATE_WALL_FOLLOW;
}

static FSM_StateID state_wall_follow(void)
{
    static uint32 dbg_tick    = 0u;
    static uint32 blank_ticks = 0u;   /* consecutive ticks with all sensors blank */

    uint16 f = read_cm(ULTRASONIC_FRONT);
    uint16 l = read_cm(ULTRASONIC_LEFT);
    uint16 r = read_cm(ULTRASONIC_RIGHT);

    if (++dbg_tick >= 20u)   /* sensor print every 1 s */
    {
        dbg_tick = 0u;
        printf("[WALL_FOLLOW] F=%u L=%u R=%u cm  turns=%lu\n",
               (unsigned)f, (unsigned)l, (unsigned)r, (unsigned long)s_turn_count);
    }

    /* All sensors blank — drive forward and debounce before declaring lost.
     * Prevents a single OUT_OF_RANGE reading from stopping the car. */
    if (f > WALL_FAR_CM && l > WALL_FAR_CM && r > WALL_FAR_CM)
    {
        if (++blank_ticks >= BLANK_DEBOUNCE)
        {
            blank_ticks = 0u;
            return FSM_STATE_WALL_LOST;
        }
        PID_update((float32)TICK_MS / 1000.0f);
        return FSM_STATE_WALL_FOLLOW;
    }
    blank_ticks = 0u;

    /* Front blocked — pick turn direction from whichever side is open */
    if (f <= FRONT_BLOCKED_CM)
    {
        if (l > WALL_FAR_CM) return FSM_STATE_TURN_LEFT;
        if (r > WALL_FAR_CM) return FSM_STATE_TURN_RIGHT;
        return FSM_STATE_WALL_LOST;   /* boxed in */
    }

    /* Both side walls gone, front still clear → slow and realign */
    if (l > WALL_FAR_CM && r > WALL_FAR_CM)
        return FSM_STATE_REALIGN;

    /* Normal corridor: PID reads sensors and drives motors */
    PID_update((float32)TICK_MS / 1000.0f);
    return FSM_STATE_WALL_FOLLOW;
}

static FSM_StateID state_wall_lost(void)
{
    s_lost_ticks++;

    if (s_lost_ticks >= LOST_TIMEOUT_TICKS)
        return FSM_STATE_STOP;

    if (read_cm(ULTRASONIC_FRONT) < WALL_FAR_CM ||
        read_cm(ULTRASONIC_LEFT)  < WALL_FAR_CM ||
        read_cm(ULTRASONIC_RIGHT) < WALL_FAR_CM)
    {
        bluetooth_send("[FSM] wall re-found\n");
        return FSM_STATE_WALL_FOLLOW;
    }

    return FSM_STATE_WALL_LOST;
}

static FSM_StateID state_stop(void)
{
    return FSM_STATE_STOP;
}

static FSM_StateID state_realign(void)
{
    uint16 f = read_cm(ULTRASONIC_FRONT);
    uint16 l = read_cm(ULTRASONIC_LEFT);
    uint16 r = read_cm(ULTRASONIC_RIGHT);

    if (l <= WALL_FAR_CM || r <= WALL_FAR_CM)
        return FSM_STATE_WALL_FOLLOW;

    if (f <= FRONT_BLOCKED_CM)
        return (l >= r) ? FSM_STATE_TURN_LEFT : FSM_STATE_TURN_RIGHT;

    if (f > WALL_FAR_CM)
        return FSM_STATE_WALL_LOST;

    return FSM_STATE_REALIGN;
}

static FSM_StateID state_turn_left(void)
{
    Car_turnLeft(TURN_SPEED, TURN_SPEED);
    vTaskDelay(pdMS_TO_TICKS(TURN_DURATION_MS));
    Motor_stopAll();
    bluetooth_send("[FSM] TURN_LEFT done\n");
    return FSM_STATE_WALL_FOLLOW;
}

static FSM_StateID state_turn_right(void)
{
    Car_turnRight(TURN_SPEED, TURN_SPEED);
    vTaskDelay(pdMS_TO_TICKS(TURN_DURATION_MS));
    Motor_stopAll();
    bluetooth_send("[FSM] TURN_RIGHT done\n");
    return FSM_STATE_WALL_FOLLOW;
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

void FSM_init(void)
{
    s_state      = FSM_STATE_IDLE;
    s_turn_count = 0u;
    s_lost_ticks = 0u;
}

FSM_StateID FSM_getState(void)
{
    return s_state;
}

void fsm_task(void *pv)
{
    (void)pv;
    FSM_StateID next;

    for (;;)
    {
        switch (s_state)
        {
            case FSM_STATE_IDLE:        next = state_idle();        break;
            case FSM_STATE_WALL_FOLLOW: next = state_wall_follow(); break;
            case FSM_STATE_WALL_LOST:   next = state_wall_lost();   break;
            case FSM_STATE_STOP:        next = state_stop();        break;
            case FSM_STATE_REALIGN:     next = state_realign();     break;
            case FSM_STATE_TURN_LEFT:   next = state_turn_left();   break;
            case FSM_STATE_TURN_RIGHT:  next = state_turn_right();  break;
            default:                    next = FSM_STATE_IDLE;      break;
        }

        if (next != s_state)
        {
            on_entry(next);
            s_state = next;
        }

        vTaskDelay(pdMS_TO_TICKS(TICK_MS));
    }
}
