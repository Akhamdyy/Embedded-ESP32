/******************************************************************************
 *
 * Module: UART Serial (HAL) — replaces Bluetooth SPP
 *
 * File Name: bluetooth.h
 *
 * Description: HAL header for serial communication over UART2.
 *              Thin wrapper over the UART MCAL (bt.h).
 *              API is kept identical to the former Bluetooth HAL so that
 *              app-layer callers (main.c, fsm.c) require zero changes.
 *
 *******************************************************************************/

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "std_types.h"

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialise UART2 at 115200 8N1.
 * Returns TRUE on success, FALSE on failure.
 */
boolean bluetooth_init(const char *device_name);

/*
 * Description :
 * Transmit a null-terminated string over UART2 TX (polling).
 * No-op if msg is empty or longer than 251 bytes.
 */
void bluetooth_send(const char *msg);

/*
 * Description :
 * Return TRUE always — UART has no connection state.
 */
boolean bluetooth_is_connected(void);

/*
 * Description :
 * Copy up to maxlen bytes from the UART2 RX ring buffer into buf.
 * Returns the number of bytes actually copied (0 if buffer is empty).
 */
uint16 bluetooth_recv(uint8 *buf, uint16 maxlen);

typedef void (*bluetooth_rx_cb_t)(const uint8 *data, uint16 len);
void bluetooth_setRxCallback(bluetooth_rx_cb_t cb);

#endif /* BLUETOOTH_H_ */
