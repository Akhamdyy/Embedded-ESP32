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

#define FRONT_BLOCKED_CM    55u                 /* critical override threshold — between 50 (too late) and 65 (too early) */
#define FRONT_SLOWDOWN_CM   75u                 /* below this distance → cruise slow */
#define WALL_OPEN_CM        35u                 /* side ≥ 35 cm = open (10 cm buffer over the 25 cm in-corridor max) */
#define WALL_PID_CM         30u                 /* both sides ≤ 30 cm → PID centres */
#define WALL_NEAR_CM        13u                 /* PID centre setpoint             */
/* Speeds tuned for the 16 V battery — these are PWM duty values; the actual
 * physical motor voltage = (value / 255) * battery_voltage.
 * SLOW_SPEED must stay ≥ ~60 or the motor stalls and the driver beeps. */
#define BASE_SPEED          90u                 /* normal cruise                   */
#define SLOW_SPEED          70u                 /* approach-the-wall cruise        */
#define TURN_SPEED          90u                 /* rotation speed                  */
#define TICK_MS             50u
#define BLANK_DEBOUNCE      10u                 /* 500 ms all-blank → STOP         */
#define LOST_TIMEOUT_TICKS  (5000u / TICK_MS)
#define TURN_SETTLE_TICKS    6u                 /* 300 ms active brake before spin */
#define POST_TURN_MIN_TICKS (500u / TICK_MS)    /* 500 ms — short corridors        */
#define CORRIDOR_CONFIRM     4u                 /* both walls stable for 200 ms    */
#define BT_PERIOD_TICKS      5u                 /* bluetooth telemetry @ 4 Hz      */
#define TURN_CONFIRM_TICKS   4u                 /* 200 ms — exceeds side sensor refresh (280 ms not quite, but kills single spikes) */
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
static volatile boolean    s_stop_signal   = FALSE;   /* '2' from BT — sticky */
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

/* Gentle post-turn re-centring controller.
 *
 *   error = left - right (cm).
 *   correction = error / GAIN_DIV, clamped to ±MAX_CORRECT.
 *   left_speed  = BASE_SPEED - correction
 *   right_speed = BASE_SPEED + correction
 *
 * Compared to the regular PID:
 *   - Pure P (no I, no D) — no integrator wind-up, no derivative kick
 *   - Wider dead-band (DEADBAND_CM) than ultrasonic noise floor
 *   - Strict input gate: skips correction unless BOTH sides see in-corridor walls
 *   - Max correction limited to ±5 PWM out of BASE_SPEED=90 (≈5.5 %)
 *
 * The previous PID failed because its ±80 PWM correction angled the car so
 * hard that the front sensor saw corridor walls obliquely at the next corner.
 * This version's correction is too small to angle the car appreciably over
 * the few hundred ms POST_TURN lasts, but enough to nudge it toward centre. */
static void apply_recenter(uint16 l, uint16 r)
{
    const sint16 DEADBAND_CM = 5;
    const sint16 MAX_CORRECT = 5;
    const sint16 GAIN_DIV    = 4;

    /* Skip correction if a side is out of corridor range — straight is safer */
    if (l > 30u || r > 30u || l < 3u || r < 3u)
    {
        Car_moveForward(BASE_SPEED, BASE_SPEED);
        return;
    }

    sint16 err = (sint16)l - (sint16)r;
    if (err > -DEADBAND_CM && err < DEADBAND_CM)
    {
        Car_moveForward(BASE_SPEED, BASE_SPEED);
        return;
    }

    sint16 corr = err / GAIN_DIV;
    if (corr >  MAX_CORRECT) corr =  MAX_CORRECT;
    if (corr < -MAX_CORRECT) corr = -MAX_CORRECT;

    sint16 left  = (sint16)BASE_SPEED - corr;
    sint16 right = (sint16)BASE_SPEED + corr;
    if (left  < 0)   left  = 0;
    if (right < 0)   right = 0;
    if (left  > 255) left  = 255;
    if (right > 255) right = 255;
    Car_moveForward((uint8)left, (uint8)right);
}

/* Sliding-window minimum filter for the front sensor.
 *
 * Why minimum (not median, not average):
 *   The HC-SR04 occasionally misses an echo and returns OOR (=400 here) at
 *   angled walls or weak reflections.  An average or asymmetric filter would
 *   eventually accept those false-far readings — the FSM then thinks the
 *   wall is gone, cruises, and crashes.
 *
 *   The min-window is collision-safe: a single valid close read anywhere
 *   in the last WIN_TICKS samples keeps the FSM seeing "wall close" until
 *   the close read ages out.  A run of false-OOR can't fool it.
 *
 * Trade-off: when the car turns into a genuinely open corridor, the filter
 *   holds the previous close value for one window (~250 ms = ~6 cm of
 *   travel at race speed).  That's fine — it just keeps the SLOW phase
 *   active for an extra ~6 cm at the start of the new corridor. */
#define FRONT_FILTER_WIN_TICKS 5u   /* 250 ms — exceeds one front refresh (140 ms) */

static uint16 read_front_filtered(void)
{
    static uint16 hist[FRONT_FILTER_WIN_TICKS] = {400u, 400u, 400u, 400u, 400u};
    static uint8  idx                          = 0u;

    uint16 raw = read_cm(ULTRASONIC_FRONT);
    hist[idx]  = raw;
    idx        = (uint8)((idx + 1u) % FRONT_FILTER_WIN_TICKS);

    uint16 min_val = hist[0];
    uint8  i;
    for (i = 1u; i < FRONT_FILTER_WIN_TICKS; i++)
    {
        if (hist[i] < min_val) min_val = hist[i];
    }
    return min_val;
}

/* Milliseconds since the FSM first entered IDLE.  Useful for correlating
 * Bluetooth output with what you see the car doing. */
static uint32 ms_since_boot(void)
{
    sint64 now = Timer_getTimeUs();
    return (uint32)((now - s_boot_us) / 1000);
}

/* Poll the UART/BT RX ring buffer every FSM tick.
 *   '1' → start signal (released by IDLE state)
 *   '2' → sticky stop signal (FSM bounces straight to STOP from anywhere) */
static void poll_bt_commands(void)
{
    uint8  rx[16];
    uint16 n = bluetooth_recv(rx, (uint16)sizeof(rx));
    uint16 i;
    for (i = 0u; i < n; i++)
    {
        if (rx[i] == (uint8)'1') s_start_signal = TRUE;
        if (rx[i] == (uint8)'2') s_stop_signal  = TRUE;
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
        /* 70 ms round-robin → each sensor reads once per 210 ms (70×3) */
        Ultrasonic_initAll(70u);
        MPU6050_init();
        PID_init(&k_pid);
        bluetooth_init("ESP32-Car");
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

    /* poll_bt_commands runs at the fsm_task level; we just check the flag here */
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

    uint16 f = read_front_filtered();
    uint16 l = read_cm(ULTRASONIC_LEFT);
    uint16 r = read_cm(ULTRASONIC_RIGHT);

    /* Approaching a wall — require TURN_CONFIRM_TICKS consecutive ticks
     * with a side ≥ WALL_OPEN_CM before committing.  Plus a critical-front
     * safety net that forces a turn before the bumper reaches the wall. */
    if (f <= FRONT_SLOWDOWN_CM)
    {
        if (l >= WALL_OPEN_CM) s_l_open_ticks++; else s_l_open_ticks = 0u;
        if (r >= WALL_OPEN_CM) s_r_open_ticks++; else s_r_open_ticks = 0u;

        boolean l_open = (s_l_open_ticks >= TURN_CONFIRM_TICKS);
        boolean r_open = (s_r_open_ticks >= TURN_CONFIRM_TICKS);

        if (l_open && r_open) return (l > r) ? FSM_STATE_TURN_LEFT : FSM_STATE_TURN_RIGHT;
        if (l_open)           return FSM_STATE_TURN_LEFT;
        if (r_open)           return FSM_STATE_TURN_RIGHT;

        /* === CRITICAL FRONT OVERRIDE ===
         * Front is at the commit threshold but neither side debounced open in
         * time.  Don't sit there crawling into the wall — pick the more-open
         * side and commit the turn NOW.  This is the safety net for the case
         * where the corner geometry hides the open side from the ultrasonic
         * until the car is almost on top of the front wall. */
        if (f <= FRONT_BLOCKED_CM)
        {
            char buf[64];
            FSM_StateID dir = (l > r) ? FSM_STATE_TURN_LEFT : FSM_STATE_TURN_RIGHT;
            (void)snprintf(buf, sizeof(buf),
                           "[CRITICAL] F=%u L=%u R=%u -> %s (forced)\n",
                           (unsigned)f, (unsigned)l, (unsigned)r,
                           (dir == FSM_STATE_TURN_LEFT) ? "TURN_LEFT" : "TURN_RIGHT");
            bluetooth_send(buf);
            return dir;
        }

        /* Not critical yet — keep crawling slowly */
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
        char done_buf[64];
        (void)snprintf(done_buf, sizeof(done_buf),
                       "[ROT] done  ===== TURN #%lu (%c) COMPLETE =====\n",
                       (unsigned long)s_turn_count,
                       (dir == MPU6050_TURN_LEFT) ? 'L' : 'R');
        bluetooth_send(done_buf);
        printf("%s", done_buf);
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
    f = read_front_filtered();
    l = read_cm(ULTRASONIC_LEFT);
    r = read_cm(ULTRASONIC_RIGHT);

    /* Hard guard — no state transitions, but apply the gentle recenter so the
     * car starts straightening out immediately after the rotation. */
    if (s_post_ticks < POST_TURN_MIN_TICKS)
    {
        s_confirm_ticks = 0u;
        apply_recenter(l, r);
        tag = "PT/REC";
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

    /* Post-guard: keep recentering until corridor confirms.
     * apply_recenter is bounded to ±5 PWM differential so it can't angle the
     * car the way the previous PID did. */
    apply_recenter(l, r);
    tag = "PT/REC";

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
    /* Sticky when triggered by the user via Bluetooth '2'.  Otherwise (any
     * vestigial automatic STOP path) bounce back so the car keeps running. */
    if (s_stop_signal)
    {
        Motor_brakeAll();
        return FSM_STATE_STOP;
    }
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
    s_stop_signal    = FALSE;
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
        /* Drain the BT RX ring every tick — sets s_start_signal / s_stop_signal */
        poll_bt_commands();

        /* '2' from BT is an unconditional STOP from any state */
        if (s_stop_signal && s_state != FSM_STATE_STOP)
        {
            bluetooth_send("[STOP] '2' received -> halting\n");
            bt_transition(s_state, FSM_STATE_STOP);
            on_entry(FSM_STATE_STOP);
            s_state = FSM_STATE_STOP;
            Motor_brakeAll();
            vTaskDelay(pdMS_TO_TICKS(TICK_MS));
            continue;
        }

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
