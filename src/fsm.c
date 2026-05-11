/******************************************************************************
 *
 * Module: Finite State Machine
 *
 * File Name: fsm.c
 *
 * Description: Wall-following robot FSM — zero blocking delays.
 *              Every state handler returns in one FSM tick (50 ms).
 *              Gyro integration and calibration are tick-stepped.
 *
 * Transition table:
 *
 *  From          Condition                               To
 *  ───────────   ─────────────────────────────────────── ────────────
 *  IDLE          calibrateStep() returns TRUE            WALL_FOLLOW
 *  WALL_FOLLOW   F ≤ FRONT_BLOCKED_CM, L open            TURN_LEFT
 *  WALL_FOLLOW   F ≤ FRONT_BLOCKED_CM, R open            TURN_RIGHT
 *  WALL_FOLLOW   all blank > 1 s                         WALL_LOST
 *  TURN_LEFT     settle ticks done → turnBegin; turnStep TRUE  POST_TURN
 *  TURN_RIGHT    settle ticks done → turnBegin; turnStep TRUE  POST_TURN
 *  POST_TURN     F ≤ FRONT_BLOCKED_CM                   WALL_FOLLOW
 *  POST_TURN     ≥ POST_TURN_MIN_TICKS AND both walls    WALL_FOLLOW
 *  WALL_LOST     any sensor < WALL_FAR_CM               WALL_FOLLOW
 *  WALL_LOST     5 s timeout                            STOP
 *  STOP          —                                       STOP
 *
 *******************************************************************************/

#include "fsm.h"
#include "motor.h"
#include "ultrasonic.h"
#include "mpu6050.h"
#include "pid.h"
#include "bluetooth.h"
#include "timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

/*******************************************************************************
 *                         Tunable Constants                                   *
 *******************************************************************************/

#define FRONT_BLOCKED_CM    35u                 /* decide-turn threshold           */
#define FRONT_SLOWDOWN_CM   75u                 /* below this distance → cruise slow */
#define WALL_OPEN_CM        40u                 /* side ≥ 40 cm = truly open → can turn */
#define WALL_PID_CM         30u                 /* both sides ≤ 30 cm → PID centres */
#define WALL_NEAR_CM        13u                 /* PID centre setpoint             */
/* Speeds tuned for the 16 V battery — these are PWM duty values; the actual
 * physical motor voltage = (value / 255) * battery_voltage. */
#define BASE_SPEED          90u                 /* normal cruise (16 V eq. of 70 @ 12 V) */
#define SLOW_SPEED          70u                 /* approach-the-wall cruise        */
#define TURN_SPEED          90u                 /* rotation speed                  */
#define TICK_MS             50u
#define BLANK_DEBOUNCE      10u                 /* 500 ms all-blank → STOP         */
#define LOST_TIMEOUT_TICKS  (5000u / TICK_MS)
#define TURN_SETTLE_TICKS    6u                 /* 300 ms active brake before spin */
#define POST_TURN_MIN_TICKS (500u / TICK_MS)    /* 500 ms — short corridors        */
#define CORRIDOR_CONFIRM     4u                 /* both walls stable for 200 ms    */
#define BT_PERIOD_TICKS      5u                 /* bluetooth telemetry @ 4 Hz      */
#define TURN_CONFIRM_TICKS   2u                 /* require N consecutive open reads */
#define STUCK_TIMEOUT_TICKS (15000u / TICK_MS)  /* 15 s without transition → STOP   */

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static const PID_Config k_pid = {
    .kp           = 1.2f,
    .ki           = 0.0f,
    .kd           = 0.4f,
    .base_speed   = BASE_SPEED,
    .threshold_cm = WALL_NEAR_CM
};

static FSM_StateID s_state;
static uint32      s_turn_count;
static uint32      s_lost_ticks;
static sint64      s_boot_us;       /* timestamp at first idle entry */

/* IDLE sub-state: calibration first, then wait for '1' from Bluetooth */
static boolean             s_calib_done    = FALSE;
static volatile boolean    s_start_signal  = FALSE;
static boolean             s_armed_msg_sent = FALSE;

/* WALL_FOLLOW sub-state — reset every time WF is entered */
static uint32              s_wf_ticks      = 0u;
static uint32              s_l_open_ticks  = 0u;
static uint32              s_r_open_ticks  = 0u;

/* Turn sub-state */
static boolean     s_turn_started;
static uint32      s_turn_settle;

/* POST_TURN sub-state */
static uint32      s_post_ticks;
static uint32      s_confirm_ticks;

/* IDLE sub-state */
static boolean     s_idle_init_done;

static const char *const k_state_str[] = {
    "IDLE", "WALL_FOLLOW", "WALL_LOST", "STOP",
    "TURN_LEFT", "TURN_RIGHT", "POST_TURN"
};

/*******************************************************************************
 *                         Private Helpers                                     *
 *******************************************************************************/

static uint16 read_cm(Ultrasonic_SensorID id)
{
    uint16 d = Ultrasonic_getDistance(id);
    return (d == ULTRASONIC_OUT_OF_RANGE) ? 400u : d;
}

/* Milliseconds since the FSM first entered IDLE.  Useful for correlating
 * Bluetooth output with what you see the car doing. */
static uint32 ms_since_boot(void)
{
    sint64 now = Timer_getTimeUs();
    return (uint32)((now - s_boot_us) / 1000);
}

/* BT-RX callback — runs in the Bluedroid task context.  Sets a flag the
 * IDLE state checks.  Any byte equal to ASCII '1' arms the start. */
static void on_bt_rx(const uint8 *data, uint16 len)
{
    uint16 i;
    for (i = 0u; i < len; i++)
    {
        if (data[i] == (uint8)'1')
        {
            s_start_signal = TRUE;
            break;
        }
    }
}

/* Stream one telemetry line.  Tag identifies which branch of the FSM produced
 * the line.  Format is deliberately short so the Bluetooth app can keep up:
 *
 *   t=12345 WF/PID F=22 L=14 R=15 T=1
 *   ^time   ^state ^sensors        ^turn count */
static void bt_telemetry(const char *tag, uint16 f, uint16 l, uint16 r)
{
    char buf[96];
    (void)snprintf(buf, sizeof(buf),
                   "t=%lu %s/%s F=%u L=%u R=%u T=%lu\n",
                   (unsigned long)ms_since_boot(),
                   k_state_str[s_state], tag,
                   (unsigned)f, (unsigned)l, (unsigned)r,
                   (unsigned long)s_turn_count);
    bluetooth_send(buf);
}

/* Loud transition marker — these print only when the state actually changes
 * so they're easy to spot in the Bluetooth log. */
static void bt_transition(FSM_StateID from, FSM_StateID to)
{
    char buf[80];
    (void)snprintf(buf, sizeof(buf),
                   "t=%lu ===> %s -> %s (T=%lu)\n",
                   (unsigned long)ms_since_boot(),
                   k_state_str[from], k_state_str[to],
                   (unsigned long)s_turn_count);
    bluetooth_send(buf);
}

static void on_entry(FSM_StateID s)
{
    char buf[64];
    /* Transition message is sent from fsm_task via bt_transition() — keep
     * this one only for serial monitor. */
    printf("[FSM] -> %s @ %lu ms\n", k_state_str[s], (unsigned long)ms_since_boot());

    switch (s)
    {
        case FSM_STATE_IDLE:
            break;

        case FSM_STATE_WALL_FOLLOW:
            PID_reset();
            s_wf_ticks     = 0u;
            s_l_open_ticks = 0u;
            s_r_open_ticks = 0u;
            bluetooth_send("[FSM] WALL_FOLLOW\n");
            break;

        case FSM_STATE_WALL_LOST:
            Motor_stopAll();
            s_lost_ticks = 0u;
            bluetooth_send("[FSM] WALL_LOST\n");
            break;

        case FSM_STATE_STOP:
            Motor_stopAll();
            (void)snprintf(buf, sizeof(buf), "[FSM] STOP turns=%lu\n",
                           (unsigned long)s_turn_count);
            bluetooth_send(buf);
            printf("%s", buf);
            break;

        case FSM_STATE_TURN_LEFT:
        case FSM_STATE_TURN_RIGHT:
            Motor_brakeAll();      /* active brake — kills forward inertia */
            s_turn_started = FALSE;
            s_turn_settle  = 0u;
            s_turn_count++;
            bluetooth_send((s == FSM_STATE_TURN_LEFT) ? "[FSM] TURN_LEFT\n"
                                                       : "[FSM] TURN_RIGHT\n");
            break;

        case FSM_STATE_POST_TURN:
            s_post_ticks    = 0u;
            s_confirm_ticks = 0u;
            PID_reset();
            Car_moveForward(BASE_SPEED, BASE_SPEED);
            bluetooth_send("[FSM] POST_TURN\n");
            break;

        default:
            break;
    }
}

/*******************************************************************************
 *                         State Handlers                                      *
 *******************************************************************************/

/* Hardware init on first tick, then one calibration sample per tick. */
static FSM_StateID state_idle(void)
{
    if (!s_idle_init_done)
    {
        Motor_init();
        Ultrasonic_initAll(60u);
        MPU6050_init();
        PID_init(&k_pid);
        bluetooth_init("ESP32-Car");
        bluetooth_setRxCallback(on_bt_rx);
        s_idle_init_done = TRUE;

        bluetooth_send("\n========= ESP32-Car ready =========\n");
        bluetooth_send("Calibrating gyro - keep robot still...\n");
    }

    /* Phase 1 — gyro calibration */
    if (!s_calib_done)
    {
        if (MPU6050_calibrateStep())
        {
            s_calib_done = TRUE;
            bluetooth_send("Calibration done.\n");
            bluetooth_send("Send '1' over Bluetooth to start the run.\n");
            bluetooth_send("===================================\n\n");
        }
        return FSM_STATE_IDLE;
    }

    /* Phase 2 — wait for start signal */
    if (!s_armed_msg_sent)
    {
        bluetooth_send("[ARMED] waiting for '1' ...\n");
        s_armed_msg_sent = TRUE;
    }

    if (s_start_signal)
    {
        bluetooth_send("[GO] start command received\n");
        return FSM_STATE_WALL_FOLLOW;
    }

    return FSM_STATE_IDLE;
}

static FSM_StateID state_wall_follow(void)
{
    static uint32 bt_tick = 0u;
    const char   *tag     = "STRAIGHT";

    /* PID + STOP disabled — turn-only test mode.
     * Car drives straight at BASE_SPEED, only transitions on corner detection.
     * No blank-STOP, no stuck-STOP, no dead-end-STOP. */

    uint16 f = read_cm(ULTRASONIC_FRONT);
    uint16 l = read_cm(ULTRASONIC_LEFT);
    uint16 r = read_cm(ULTRASONIC_RIGHT);

    /* Approaching a wall — require TURN_CONFIRM_TICKS consecutive ticks
     * with a side ≥ WALL_OPEN_CM before committing. */
    if (f <= FRONT_SLOWDOWN_CM)
    {
        if (l >= WALL_OPEN_CM) s_l_open_ticks++; else s_l_open_ticks = 0u;
        if (r >= WALL_OPEN_CM) s_r_open_ticks++; else s_r_open_ticks = 0u;

        boolean l_open = (s_l_open_ticks >= TURN_CONFIRM_TICKS);
        boolean r_open = (s_r_open_ticks >= TURN_CONFIRM_TICKS);

        if (l_open && r_open) return (l > r) ? FSM_STATE_TURN_LEFT : FSM_STATE_TURN_RIGHT;
        if (l_open)           return FSM_STATE_TURN_LEFT;
        if (r_open)           return FSM_STATE_TURN_RIGHT;

        /* No side open — keep crawling slowly forward (no STOP) */
        Car_moveForward(SLOW_SPEED, SLOW_SPEED);
        tag = "SLOW";
        goto telemetry;
    }
    s_l_open_ticks = 0u;
    s_r_open_ticks = 0u;

    /* Always drive straight — PID disabled */
    Car_moveForward(BASE_SPEED, BASE_SPEED);
    tag = "STRAIGHT";

telemetry:
    if (++bt_tick >= BT_PERIOD_TICKS)
    {
        bt_tick = 0u;
        bt_telemetry(tag, f, l, r);
        printf("[WF/%s] F=%u L=%u R=%u T=%lu\n",
               tag, (unsigned)f, (unsigned)l, (unsigned)r,
               (unsigned long)s_turn_count);
    }
    return FSM_STATE_WALL_FOLLOW;
}

/* Shared handler for both turn directions.
 * Phase 1: active-brake settle (TURN_SETTLE_TICKS × 50 ms).
 * Phase 2: MPU6050_turnBegin once, then MPU6050_turnStep each tick. */
static FSM_StateID state_turn(MPU6050_TurnDir dir)
{
    FSM_StateID self = (dir == MPU6050_TURN_LEFT) ? FSM_STATE_TURN_LEFT
                                                  : FSM_STATE_TURN_RIGHT;
    char buf[64];

    if (!s_turn_started)
    {
        if (++s_turn_settle < TURN_SETTLE_TICKS)
        {
            (void)snprintf(buf, sizeof(buf), "[SETTLE] %lu/%u\n",
                           (unsigned long)s_turn_settle, TURN_SETTLE_TICKS);
            bluetooth_send(buf);
            return self;
        }

        MPU6050_turnBegin(dir, TURN_SPEED);
        s_turn_started = TRUE;
        bluetooth_send("[ROT] begin\n");
        return self;
    }

    if (MPU6050_turnStep())
    {
        bluetooth_send("[ROT] done\n");
        return FSM_STATE_POST_TURN;
    }

    return self;
}

static FSM_StateID state_turn_left(void)  { return state_turn(MPU6050_TURN_LEFT);  }
static FSM_StateID state_turn_right(void) { return state_turn(MPU6050_TURN_RIGHT); }

/* Drives forward after a turn.
 * Guarantees POST_TURN_MIN_TICKS (2 s) of forward motion regardless of sensors.
 * After the guard, exits to WALL_FOLLOW when either a front wall is seen OR
 * both side walls have been stable for CORRIDOR_CONFIRM ticks.
 * The 2 s guard is what prevents the infinite-turn loop when a turn overshoots
 * and leaves the front sensor pointing at a wall. */
static FSM_StateID state_post_turn(void)
{
    static uint32 bt_tick = 0u;
    uint16 f, l, r;
    const char *tag;

    s_post_ticks++;
    f = read_cm(ULTRASONIC_FRONT);
    l = read_cm(ULTRASONIC_LEFT);
    r = read_cm(ULTRASONIC_RIGHT);

    /* Hard guard — drive forward, no exits whatsoever (PID disabled) */
    if (s_post_ticks < POST_TURN_MIN_TICKS)
    {
        s_confirm_ticks = 0u;
        Car_moveForward(BASE_SPEED, BASE_SPEED);
        tag = "PT/STR";
        goto telemetry;
    }

    /* Guard elapsed — sensors decide */
    if (f <= FRONT_BLOCKED_CM)
    {
        char buf[64];
        s_post_ticks    = 0u;
        s_confirm_ticks = 0u;
        (void)snprintf(buf, sizeof(buf),
                       "[CORRIDOR %lu] entered (front wall imminent)\n",
                       (unsigned long)(s_turn_count + 1u));
        bluetooth_send(buf);
        return FSM_STATE_WALL_FOLLOW;
    }

    if (l < WALL_OPEN_CM && r < WALL_OPEN_CM)
    {
        if (++s_confirm_ticks >= CORRIDOR_CONFIRM)
        {
            char buf[64];
            s_post_ticks    = 0u;
            s_confirm_ticks = 0u;
            (void)snprintf(buf, sizeof(buf),
                           "[CORRIDOR %lu] entered (both walls confirmed)\n",
                           (unsigned long)(s_turn_count + 1u));
            bluetooth_send(buf);
            return FSM_STATE_WALL_FOLLOW;
        }
    }
    else
    {
        s_confirm_ticks = 0u;
    }

    Car_moveForward(BASE_SPEED, BASE_SPEED);
    tag = "PT/STR";

telemetry:
    if (++bt_tick >= BT_PERIOD_TICKS)
    {
        bt_tick = 0u;
        bt_telemetry(tag, f, l, r);
    }
    return FSM_STATE_POST_TURN;
}

static FSM_StateID state_wall_lost(void)
{
    if (++s_lost_ticks >= LOST_TIMEOUT_TICKS) return FSM_STATE_STOP;

    if (read_cm(ULTRASONIC_FRONT) < WALL_OPEN_CM ||
        read_cm(ULTRASONIC_LEFT)  < WALL_OPEN_CM ||
        read_cm(ULTRASONIC_RIGHT) < WALL_OPEN_CM)
    {
        bluetooth_send("[FSM] wall re-found\n");
        return FSM_STATE_WALL_FOLLOW;
    }

    return FSM_STATE_WALL_LOST;
}

static FSM_StateID state_stop(void)
{
    /* Turn-only test mode: never stay in STOP, bounce back to WALL_FOLLOW. */
    return FSM_STATE_WALL_FOLLOW;
}

/*******************************************************************************
 *                         Public API                                          *
 *******************************************************************************/

void FSM_init(void)
{
    s_state          = FSM_STATE_IDLE;
    s_turn_count     = 0u;
    s_lost_ticks     = 0u;
    s_turn_started   = FALSE;
    s_turn_settle    = 0u;
    s_post_ticks     = 0u;
    s_confirm_ticks  = 0u;
    s_idle_init_done = FALSE;
    s_calib_done     = FALSE;
    s_start_signal   = FALSE;
    s_armed_msg_sent = FALSE;
    s_boot_us        = Timer_getTimeUs();
}

FSM_StateID FSM_getState(void)
{
    return s_state;
}

void fsm_task(void *pv)
{
    (void)pv;
    FSM_StateID next;
    uint32      hb_ticks = 0u;

    for (;;)
    {
        /* Heartbeat — every 100 ticks (5 s) regardless of state */
        if (++hb_ticks >= (5000u / TICK_MS))
        {
            char hb_buf[80];
            hb_ticks = 0u;
            (void)snprintf(hb_buf, sizeof(hb_buf),
                           "[HB] t=%lu state=%s T=%lu conn=%d\n",
                           (unsigned long)ms_since_boot(),
                           k_state_str[s_state],
                           (unsigned long)s_turn_count,
                           bluetooth_is_connected() ? 1 : 0);
            bluetooth_send(hb_buf);
        }

        switch (s_state)
        {
            case FSM_STATE_IDLE:        next = state_idle();        break;
            case FSM_STATE_WALL_FOLLOW: next = state_wall_follow(); break;
            case FSM_STATE_TURN_LEFT:   next = state_turn_left();   break;
            case FSM_STATE_TURN_RIGHT:  next = state_turn_right();  break;
            case FSM_STATE_POST_TURN:   next = state_post_turn();   break;
            case FSM_STATE_WALL_LOST:   next = state_wall_lost();   break;
            case FSM_STATE_STOP:        next = state_stop();        break;
            default:                    next = FSM_STATE_IDLE;      break;
        }

        if (next != s_state)
        {
            bt_transition(s_state, next);
            on_entry(next);
            s_state = next;
        }

        vTaskDelay(pdMS_TO_TICKS(TICK_MS));  /* only delay in the system */
    }
}
