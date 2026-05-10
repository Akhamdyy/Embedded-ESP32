/******************************************************************************
 *
 * Module: Interrupts (MCAL)
 *
 * File Name: intr.h
 *
 * Description: Header file for the manual CPU interrupt allocator. Routes
 *              ESP32 peripheral interrupt sources to Xtensa LX6 CPU
 *              interrupt lines via direct writes to the DPORT interrupt
 *              matrix (TRM Chapter 7). Replaces esp_intr_alloc().
 *
 *******************************************************************************/

#ifndef INTR_H_
#define INTR_H_

#include "std_types.h"

/*******************************************************************************
 *                            Peripheral Source IDs                            *
 *
 * Subset of the ESP32 peripheral interrupt source table (TRM Table 7-1 and
 * soc/interrupts.h). The numeric value is the source's slot in the DPORT
 * interrupt-matrix mapping register block — Intr_install indexes it
 * directly. Add more entries here as additional peripherals start using
 * interrupts.
 ******************************************************************************/
#define INTR_SOURCE_GPIO        22u   /* ETS_GPIO_INTR_SOURCE              */

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

/* ISR handler signature. Runs in CPU interrupt context — keep short and
 * only call ISR-safe APIs (e.g. FreeRTOS *FromISR helpers).               */
typedef void (*Intr_HandlerType)(void *arg);

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Allocate a CPU interrupt for the given peripheral source, route the
 * source to it through the DPORT interrupt matrix, install the handler
 * in the Xtensa vector dispatch table, and unmask the CPU interrupt line.
 *
 * Notes:
 *  - Not thread-safe; intended to be called from app init only.
 *  - One-shot per source — calling twice for the same source consumes
 *    another CPU interrupt slot. Callers must guard.
 *  - The CPU-interrupt pool is fixed (see intr.c); call returns silently
 *    if exhausted.
 */
void Intr_install(uint32 intr_source, Intr_HandlerType handler, void *arg);

#endif /* INTR_H_ */
