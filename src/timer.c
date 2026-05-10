/******************************************************************************
 *
 * Module: Timer (MCAL)
 *
 * File Name: timer.c
 *
 * Description: Register-level + FreeRTOS implementation of the MCAL timer
 *              driver. Replaces the previous esp_timer.h dependency.
 *
 *              Implementation strategy:
 *                - Timer_createOneShot / Timer_createPeriodic
 *                  are backed by FreeRTOS software timers (the only allowed
 *                  external dependency). A small dispatch trampoline adapts
 *                  the FreeRTOS callback signature to ours.
 *                - Timer_getTimeUs reads ESP32 Timer Group 0, timer 0,
 *                  configured at boot as a free-running 64-bit counter
 *                  ticking at 1 MHz (i.e. each tick = 1 µs). All access
 *                  is via direct register writes per ESP32 TRM §17.
 *
 *              Note: FreeRTOS software timers run from the timer service
 *              task at FreeRTOS tick resolution, so callback timing is
 *              tick-quantised (typically 1 ms or 10 ms depending on
 *              CONFIG_FREERTOS_HZ). This is acceptable for the existing
 *              callers (HC-SR04 trigger pulse just needs >=10 µs HIGH).
 *
 *******************************************************************************/

#include "timer.h"
#include "platform.h"

#include "soc/soc.h"
#include "soc/dport_reg.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

/*******************************************************************************
 *                              Local Macros                                   *
 *******************************************************************************/

#define REG32(addr)             (*((volatile uint32 *)(addr)))

/* We use Timer Group 1, timer 0 (TIMG1_T0).
 *
 * Important: ESP-IDF's esp_timer driver (LAC) owns TIMG0_T0 on ESP32 and
 * reconfigures its divider at boot, so we deliberately use TIMG1_T0 to
 * avoid that conflict. TIMG1 is otherwise untouched by ESP-IDF v5.x in the
 * default build (its watchdog is disabled by default).
 *
 * Register offsets (ESP32 TRM §17 / soc/timer_group_reg.h)
 */
#define TIMG1_BASE              0x3FF60000u
#define TIMG_T0CONFIG_REG       (TIMG1_BASE + 0x0000u)
#define TIMG_T0LO_REG           (TIMG1_BASE + 0x0004u)
#define TIMG_T0HI_REG           (TIMG1_BASE + 0x0008u)
#define TIMG_T0UPDATE_REG       (TIMG1_BASE + 0x000Cu)
#define TIMG_T0LOADLO_REG       (TIMG1_BASE + 0x0018u)
#define TIMG_T0LOADHI_REG       (TIMG1_BASE + 0x001Cu)
#define TIMG_T0LOAD_REG         (TIMG1_BASE + 0x0020u)

/* TIMG_T0CONFIG_REG fields */
#define TIMG_T0_EN              (1u << 31)        /* enable counter            */
#define TIMG_T0_INCREASE        (1u << 30)        /* 1 = count up              */
#define TIMG_T0_AUTORELOAD      (1u << 29)        /* 0 = free-running          */
#define TIMG_T0_DIVIDER_S       13u               /* 16-bit prescaler [28:13]  */
#define TIMG_T0_DIVIDER_M       0xFFFFu
#define TIMG_T0_ALARM_EN        (1u << 10)        /* keep cleared              */

/* APB_CLK is 80 MHz. Divider 80 → 1 MHz tick → 1 µs per tick. */
#define TIMG_DIVIDER_FOR_1MHZ   80u

/* Maximum number of MCAL timers we ever allocate (matches project usage:
 * 3 ultrasonic one-shots + 1 periodic measurement timer). Bumped for headroom. */
#define TIMER_POOL_SIZE         8u

/*******************************************************************************
 *                            Private Types                                    *
 *******************************************************************************/

typedef struct
{
    Timer_CallbackType cb;
    void              *arg;
} Timer_Ctx;

/*******************************************************************************
 *                           Private Data                                      *
 *******************************************************************************/

static Timer_Ctx ctx_pool[TIMER_POOL_SIZE];
static uint32    ctx_count          = 0u;
static boolean   timg0_initialized  = FALSE;

/*******************************************************************************
 *                          Private Helpers                                    *
 *******************************************************************************/

/* Configure TIMG0_T0 as a 64-bit free-running 1 MHz counter. Idempotent. */
static void prv_timgInit(void)
{
    if (timg0_initialized)
    {
        return;
    }

    /* Ungate Timer Group 1 peripheral clock and pulse its reset. */
    REG32(DPORT_PERIP_CLK_EN_REG) |= DPORT_TIMERGROUP1_CLK_EN;
    REG32(DPORT_PERIP_RST_EN_REG) |= DPORT_TIMERGROUP1_RST;
    REG32(DPORT_PERIP_RST_EN_REG) &= ~DPORT_TIMERGROUP1_RST;

    /* Disable while configuring */
    REG32(TIMG_T0CONFIG_REG) = 0u;

    /* Reset counter to 0 by latching the load registers */
    REG32(TIMG_T0LOADLO_REG) = 0u;
    REG32(TIMG_T0LOADHI_REG) = 0u;
    REG32(TIMG_T0LOAD_REG)   = 1u;

    /* Divider 80 → 1 MHz, count up, no auto-reload, no alarm, enable */
    REG32(TIMG_T0CONFIG_REG) =
          ((TIMG_DIVIDER_FOR_1MHZ & TIMG_T0_DIVIDER_M) << TIMG_T0_DIVIDER_S)
        | TIMG_T0_INCREASE
        | TIMG_T0_EN;

    timg0_initialized = TRUE;
}

/* FreeRTOS timer callback adapter — extracts our context and dispatches. */
static void prv_dispatch(TimerHandle_t t)
{
    Timer_Ctx *ctx = (Timer_Ctx *)pvTimerGetTimerID(t);
    if ((ctx != NULL_PTR) && (ctx->cb != NULL_PTR))
    {
        ctx->cb(ctx->arg);
    }
}

/* Convert microseconds → FreeRTOS ticks (round up, minimum 1 tick). */
static TickType_t prv_usToTicks(uint32 us)
{
    uint32 ms = (us + 999u) / 1000u;
    if (ms == 0u)
    {
        ms = 1u;
    }
    TickType_t t = pdMS_TO_TICKS(ms);
    if (t == 0u)
    {
        t = 1u;
    }
    return t;
}

/* Shared create path — allocates a context slot and a FreeRTOS timer. */
static void prv_create(const char *name, Timer_CallbackType cb, void *arg,
                       Timer_HandleType *out, BaseType_t auto_reload)
{
    if (out == NULL_PTR)
    {
        return;
    }
    if (ctx_count >= TIMER_POOL_SIZE)
    {
        *out = NULL_PTR;
        return;
    }

    prv_timgInit();

    Timer_Ctx *ctx = &ctx_pool[ctx_count++];
    ctx->cb  = cb;
    ctx->arg = arg;

    TimerHandle_t h = xTimerCreate(name,
                                   1u,                /* placeholder period   */
                                   auto_reload,
                                   (void *)ctx,
                                   prv_dispatch);
    *out = (Timer_HandleType)h;
}

/*******************************************************************************
 *                         Function Definitions                                *
 *******************************************************************************/

/*
 * Description :
 * Create a one-shot software timer.
 */
void Timer_createOneShot(const char *name, Timer_CallbackType callback,
                         void *arg, Timer_HandleType *out)
{
    prv_create(name, callback, arg, out, pdFALSE);
}

/*
 * Description :
 * Create a periodic software timer.
 */
void Timer_createPeriodic(const char *name, Timer_CallbackType callback,
                          void *arg, Timer_HandleType *out)
{
    prv_create(name, callback, arg, out, pdTRUE);
}

/*
 * Description :
 * Start a one-shot timer to fire after delay_us microseconds.
 */
void Timer_startOnce(Timer_HandleType handle, uint32 delay_us)
{
    if (handle == NULL_PTR)
    {
        return;
    }

    /* Fast path: sub-millisecond one-shots (e.g. HC-SR04 10 µs trigger pulse).
     * FreeRTOS software timers can't go below 1 tick (10 ms at the default
     * CONFIG_FREERTOS_HZ=100), so we busy-wait on the 1 MHz TIMG counter and
     * dispatch the callback synchronously. The wait is short enough that
     * spinning is cheaper than a context switch. */
    if (delay_us < 1000u)
    {
        sint64 deadline = Timer_getTimeUs() + (sint64)delay_us;
        while (Timer_getTimeUs() < deadline)
        {
            /* spin */
        }
        Timer_Ctx *ctx = (Timer_Ctx *)pvTimerGetTimerID((TimerHandle_t)handle);
        if ((ctx != NULL_PTR) && (ctx->cb != NULL_PTR))
        {
            ctx->cb(ctx->arg);
        }
        return;
    }

    /* xTimerChangePeriod also implicitly starts the timer. */
    (void)xTimerChangePeriod((TimerHandle_t)handle, prv_usToTicks(delay_us), 0);
}

/*
 * Description :
 * Start a periodic timer to fire every period_us microseconds.
 */
void Timer_startPeriodic(Timer_HandleType handle, uint32 period_us)
{
    if (handle == NULL_PTR)
    {
        return;
    }
    (void)xTimerChangePeriod((TimerHandle_t)handle, prv_usToTicks(period_us), 0);
}

/*
 * Description :
 * Return time since boot in microseconds. ISR-safe (placed in IRAM).
 * Reads the 64-bit TIMG0_T0 free-running counter (1 MHz tick).
 */
ISR_ATTR sint64 Timer_getTimeUs(void)
{
    /* Latch the current 64-bit counter value into the LO/HI shadow regs.
     * Bit 31 (TIMG_T0_UPDATE) is the trigger field; writing bit 0 does nothing. */
    REG32(TIMG_T0UPDATE_REG) = (1u << 31);
    uint32 lo = REG32(TIMG_T0LO_REG);
    uint32 hi = REG32(TIMG_T0HI_REG);
    return (sint64)(((uint64)hi << 32) | (uint64)lo);
}
