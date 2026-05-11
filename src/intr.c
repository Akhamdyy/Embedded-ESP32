/******************************************************************************
 *
 * Module: Interrupts (MCAL)
 *
 * File Name: intr.c
 *
 * Description: Manual CPU interrupt allocator — replaces esp_intr_alloc().
 *
 *              The ESP32 routes peripheral interrupt sources through the
 *              DPORT interrupt matrix (TRM Chapter 7): writing a 5-bit CPU
 *              interrupt number into the source's PRO_*_INTR_MAP_REG makes
 *              the source raise that CPU interrupt line on PRO_CPU. The
 *              Xtensa LX6 core then dispatches through a vector table that
 *              the FreeRTOS Xtensa port installs at boot, looking each
 *              handler up in _xt_interrupt_table.
 *
 *              Pipeline performed by Intr_install():
 *                1. DPORT mapping register — direct register write
 *                2. Xtensa vector slot     — xt_set_interrupt_handler
 *                                            (FreeRTOS Xtensa port API)
 *                3. CPU interrupt unmask   — xt_ints_on (FreeRTOS port API,
 *                                            wraps RSR/WSR.INTENABLE)
 *
 *              xt_set_interrupt_handler and xt_ints_on belong to the
 *              FreeRTOS Xtensa port (the only allowed external dependency)
 *              and not to ESP-IDF. Forward-declared below so this file
 *              does not depend on the (relocatable) xtensa_api.h path.
 *
 *******************************************************************************/

#include "intr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"      /* xPortGetCoreID for Intr_diagnose          */
#include <stdio.h>              /* printf for Intr_diagnose                  */

/*******************************************************************************
 *                FreeRTOS Xtensa Port — Forward Declarations                  *
 *******************************************************************************/

typedef void (*xt_handler_t)(void *);
extern xt_handler_t xt_set_interrupt_handler(int n, xt_handler_t f, void *arg);
extern void         xt_ints_on(unsigned int mask);

/*******************************************************************************
 *                              Register Helper                                *
 *******************************************************************************/

#define REG32(addr)                 (*((volatile uint32 *)(addr)))

/*******************************************************************************
 *                  DPORT Interrupt Matrix (TRM Chapter 7)                     *
 *
 * One 32-bit register per peripheral source. Lower 5 bits select the CPU
 * interrupt number (0..31) raised on PRO_CPU. Source 0 (WIFI_MAC) is at
 * +0x104; source N at +0x104 + (N * 4). APP_CPU map block is at +0x218
 * and is unused here — all interrupts are routed to PRO_CPU.
 ******************************************************************************/
#define DPORT_PRO_INTR_MAP_BASE     0x3FF00104u
#define INTR_MAP_REG(source)        (DPORT_PRO_INTR_MAP_BASE + ((source) * 4u))

/*******************************************************************************
 *                          CPU Interrupt Pool                                 *
 *
 * Xtensa LX6 has 32 interrupt lines. Lines reserved by FreeRTOS / ESP-IDF
 * on this chip (TRM Section 7.4 + ESP-IDF interrupt_allocator):
 *      6  - SYSTICK   (FreeRTOS tick)
 *      7  - SW0       (FreeRTOS yield)
 *      11 - PROFILING
 *      14 - NMI                       (cannot run a C handler)
 *      15 - TIMER1    (esp_timer)
 *      16 - TIMER2    (systimer alarm)
 *      29 - SW1       (cross-core IPC)
 * Lines 0/1 are conventionally used by Wi-Fi (WMAC/BB) — Wi-Fi is unused
 * in this project. BT controller dynamically allocates a few mid-range
 * lines via esp_intr_alloc when bt.c initialises; the pool below picks
 * level-1 external lines that BT's allocator is unlikely to grab first.
 *
 * Extend cpu_int_pool[] if more peripherals start requesting interrupts.
 ******************************************************************************/
static const uint8 cpu_int_pool[] = { 12u, 13u, 17u, 18u };
static uint8       pool_next      = 0u;

/* Bookkeeping for Intr_diagnose: which source got which CPU int. */
typedef struct
{
    uint32 source;
    uint8  cpu_int;
} Intr_InstalledEntry;

static Intr_InstalledEntry installed[sizeof(cpu_int_pool) / sizeof(cpu_int_pool[0])];
static uint8               installed_count = 0u;

/*******************************************************************************
 *                         Function Definitions                                *
 *******************************************************************************/

/*
 * Description :
 * See header.
 */
void Intr_install(uint32 intr_source, Intr_HandlerType handler, void *arg)
{
    if (handler == NULL_PTR)
    {
        return;
    }
    if (pool_next >= (uint8)(sizeof(cpu_int_pool) / sizeof(cpu_int_pool[0])))
    {
        /* Pool exhausted — extend cpu_int_pool[] above. */
        return;
    }

    uint8 cpu_int = cpu_int_pool[pool_next++];

    /* 1. Route the peripheral source to the chosen CPU interrupt line. */
    REG32(INTR_MAP_REG(intr_source)) = (uint32)cpu_int;

    /* 2. Install the handler in the Xtensa vector dispatch table. */
    (void)xt_set_interrupt_handler((int)cpu_int, (xt_handler_t)handler, arg);

    /* 3. Unmask the CPU interrupt line (RMW of INTENABLE special reg). */
    xt_ints_on(1u << cpu_int);

    /* 4. Record for Intr_diagnose. */
    installed[installed_count].source  = intr_source;
    installed[installed_count].cpu_int = cpu_int;
    installed_count++;
}

/*******************************************************************************
 *                          Diagnostic / Validation                            *
 *******************************************************************************/

/* Sources allocated dynamically by the BT controller's internal esp_intr_alloc.
 * Range 3..8 covers ETS_BT_MAC, ETS_BT_BB, ETS_BT_BB_NMI, ETS_RWBT, ETS_RWBLE,
 * ETS_RWBT_NMI on ESP32. Used by Intr_diagnose to scan for CPU-int collisions
 * with our pool. */
#define BT_SOURCE_FIRST     3u
#define BT_SOURCE_LAST      8u

/* Read Xtensa INTENABLE special register (per-core). */
static inline uint32 read_intenable(void)
{
    uint32 v;
    __asm__ __volatile__ ("rsr.intenable %0" : "=r"(v));
    return v;
}

void Intr_diagnose(void)
{
    BaseType_t core = xPortGetCoreID();
    uint32     ie   = read_intenable();
    uint8      i;
    uint8      failures = 0u;

    printf("\n=== Intr_diagnose ===\n");
    printf("core_id        : %d (expect 0 = PRO_CPU)\n", (int)core);
    if (core != 0)
    {
        printf("  !! FAIL — Intr_diagnose must run on PRO_CPU "
               "(INTENABLE is per-core)\n");
        failures++;
    }

    printf("INTENABLE      : 0x%08lx\n", (unsigned long)ie);
    printf("pool_next      : %u / %u\n",
           (unsigned)pool_next,
           (unsigned)(sizeof(cpu_int_pool) / sizeof(cpu_int_pool[0])));
    printf("installed_count: %u\n", (unsigned)installed_count);

    /* Per-installation checks: DPORT map read-back + INTENABLE bit set. */
    for (i = 0u; i < installed_count; i++)
    {
        uint32 source  = installed[i].source;
        uint8  cpu_int = installed[i].cpu_int;
        uint32 mapreg  = REG32(INTR_MAP_REG(source)) & 0x1Fu;
        boolean ena    = (ie & (1u << cpu_int)) != 0u;

        printf("  [%u] src=%lu cpu_int=%u map_reg=%lu enabled=%s",
               (unsigned)i, (unsigned long)source, (unsigned)cpu_int,
               (unsigned long)mapreg, ena ? "Y" : "N");

        if (mapreg != cpu_int)
        {
            printf("  !! FAIL (map mismatch)");
            failures++;
        }
        if (!ena)
        {
            printf("  !! FAIL (INTENABLE bit clear)");
            failures++;
        }
        printf("\n");
    }

    /* BT-controller collision scan. If bt.c is initialised before
     * Intr_diagnose runs, the BT source map registers hold non-zero CPU int
     * numbers assigned by ESP-IDF's esp_intr_alloc. Any overlap with our
     * pool means two ISRs sharing one CPU int — silent corruption. */
    printf("BT collision scan (sources %u..%u):\n",
           (unsigned)BT_SOURCE_FIRST, (unsigned)BT_SOURCE_LAST);
    for (i = BT_SOURCE_FIRST; i <= BT_SOURCE_LAST; i++)
    {
        uint32 bt_cpu_int = REG32(INTR_MAP_REG(i)) & 0x1Fu;
        boolean collision = FALSE;
        uint8 j;
        for (j = 0u; j < installed_count; j++)
        {
            if (installed[j].cpu_int == (uint8)bt_cpu_int)
            {
                collision = TRUE;
                break;
            }
        }
        if (bt_cpu_int != 0u)
        {
            printf("  bt_src=%u -> cpu_int=%lu%s\n",
                   (unsigned)i, (unsigned long)bt_cpu_int,
                   collision ? "  !! FAIL (overlaps our pool)" : "");
            if (collision) failures++;
        }
    }

    printf("--- result: %s (%u failures) ---\n\n",
           (failures == 0u) ? "PASS" : "FAIL", (unsigned)failures);
}
