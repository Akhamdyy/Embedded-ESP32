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
}
