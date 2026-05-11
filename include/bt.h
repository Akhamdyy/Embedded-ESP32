/******************************************************************************
 *
 * Module: UART (MCAL) — replaces Bluetooth SPP
 *
 * File Name: bt.h
 *
 * Description: MCAL header for UART2 serial communication.
 *              Implements a register-level UART2 driver on ESP32.
 *              Exposes the same API shape as the former Bluetooth MCAL so
 *              the HAL layer (bluetooth.c) requires zero changes to its
 *              #include list or call sites — only its init call changes.
 *
 *              Hardware assignment:
 *                UART2  TX → GPIO 5  (PORTA PIN3)
 *                UART2  RX → GPIO 0  (PORTA PIN0, boot button — idle HIGH)
 *                Baud       115200, 8N1, APB 80 MHz clock
 *
 *******************************************************************************/

#ifndef BT_H_
#define BT_H_

#include "std_types.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

typedef enum
{
    BT_OK   = 0,
    BT_FAIL = 1
} BT_StatusType;

typedef void (*BT_ConnectedCallbackType)(void);
typedef void (*BT_DisconnectedCallbackType)(void);
typedef void (*BT_RxCallbackType)(const uint8 *data, uint16 len);

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description:
 * Initialise UART2 at 115200 8N1 via direct register access.
 * Clocks UART2 peripheral, routes GPIO matrix, resets FIFOs, sets baud
 * rate divisor, enables RX FIFO + timeout interrupts.
 * device_name is accepted for API compatibility but is unused.
 */
BT_StatusType BT_initSPP(const char *device_name);

/*
 * Description:
 * Store optional connect/disconnect callbacks.
 * Because UART is always "connected", on_connect is fired immediately.
 */
void BT_setSPPCallbacks(BT_ConnectedCallbackType on_connect,
                        BT_DisconnectedCallbackType on_disconnect);

/*
 * Description:
 * Transmit len bytes over UART2 TX FIFO (polling — blocks until sent).
 */
void BT_sendSPP(const uint8 *data, uint16 len);

/*
 * Description:
 * Return TRUE always — UART has no connection state.
 */
boolean BT_isSPPConnected(void);

/*
 * Description:
 * Copy up to maxlen bytes from the RX ring buffer into buf.
 * Returns the number of bytes actually copied (0 if buffer is empty).
 */
uint16 BT_recvSPP(uint8 *buf, uint16 maxlen);

#endif /* BT_H_ */
