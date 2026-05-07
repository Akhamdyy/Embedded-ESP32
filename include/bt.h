/******************************************************************************
 *
 * Module: BT (Bluetooth Controller + Bluedroid SPP)
 *
 * File Name: bt.h
 *
 * Description: MCAL header for Bluetooth Classic SPP.
 *              Wraps ESP-IDF controller init, Bluedroid stack, GAP, and SPP
 *              so upper HAL layers have zero direct ESP-IDF dependency.
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

/* Callbacks fired from the Bluedroid task — keep them short, no blocking. */
typedef void (*BT_ConnectedCallbackType)(void);
typedef void (*BT_DisconnectedCallbackType)(void);

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description:
 * Initialise NVS, BT controller, Bluedroid stack, GAP, and SPP server.
 * The device becomes discoverable and connectable asynchronously after this
 * call returns (driven by Bluedroid internal tasks).
 * Must be called once before any other BT function.
 */
BT_StatusType BT_initSPP(const char *device_name);

/*
 * Description:
 * Register optional application callbacks for connect / disconnect events.
 * Call before BT_initSPP or immediately after — safe to call at any time.
 */
void BT_setSPPCallbacks(BT_ConnectedCallbackType on_connect,
                        BT_DisconnectedCallbackType on_disconnect);

/*
 * Description:
 * Send len bytes over the active SPP connection.
 * No-op if no client is connected.
 */
void BT_sendSPP(const uint8 *data, uint16 len);

/*
 * Description:
 * Return TRUE if an SPP client is currently connected.
 */
boolean BT_isSPPConnected(void);

#endif /* BT_H_ */
