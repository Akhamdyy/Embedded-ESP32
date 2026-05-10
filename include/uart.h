/******************************************************************************
 *
 * Module: UART (MCAL)
 *
 * File Name: uart.h
 *
 * Description: Register-level UART2 driver for ESP32.
 *              All hardware access is via direct MMIO register reads/writes.
 *              No driver/uart.h, no esp_timer.h, no Arduino libraries.
 *
 *              Design:
 *                TX — polling: spins until TX FIFO has room, then writes.
 *                RX — interrupt-driven: ISR drains FIFO into a 256-byte
 *                     ring buffer; UART_recv() pops from the ring buffer.
 *
 *              Only UART2 is supported (hardwired in uart.c). The baud
 *              rate and GPIO pins are configurable at init time.
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

typedef enum
{
    UART_OK   = 0,
    UART_FAIL = 1
} UART_StatusType;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description:
 * Initialise UART2 at the requested baud rate.
 * Clocks and resets the UART2 peripheral, routes TX and RX through the GPIO
 * matrix (IO_MUX MCU_SEL = 2), resets both FIFOs, programs the baud
 * divisor, sets 8N1 framing, and registers the RX ISR via esp_intr_alloc.
 * Returns UART_OK on success or UART_FAIL if either GPIO is unsupported.
 */
UART_StatusType UART_init(uint32 baud, uint8 tx_gpio, uint8 rx_gpio);

/*
 * Description:
 * Transmit len bytes over UART2 TX (polling).
 * Spins on TXFIFO_CNT until there is room, then writes one byte at a time
 * into the TX FIFO register.
 */
void UART_send(const uint8 *data, uint16 len);

/*
 * Description:
 * Copy up to maxlen bytes from the UART2 RX ring buffer into buf.
 * Returns the number of bytes actually copied (0 if the buffer is empty).
 * Non-blocking — returns immediately if no data is available.
 */
uint16 UART_recv(uint8 *buf, uint16 maxlen);

#endif /* UART_H_ */
