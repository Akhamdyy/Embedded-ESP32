/******************************************************************************
 *
 * Module: BT (Bluetooth Controller)
 *
 * File Name: bt.h
 *
 * Description: Header file for the ESP32 Bluetooth Classic controller driver
 *              (MCAL layer). Wraps the ESP-IDF BT controller and VHCI
 *              interface so that higher-level HAL drivers have no direct
 *              ESP-IDF BT dependency.
 *
 *              Responsibilities:
 *                - NVS flash initialisation (required by BT controller)
 *                - BT controller power-on in Classic BT mode
 *                - VHCI send / receive interface
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

/*
 * Callback invoked when a raw HCI packet is received from the controller.
 * data : pointer to raw packet bytes (packet-type byte is data[0])
 * len  : total byte count
 * Returns 0 (reserved for future use by ESP-IDF VHCI interface).
 */
typedef int (*BT_RecvCallbackType)(uint8 *data, uint16 len);

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialise NVS, power on the BT controller in Classic BT mode, and register
 * the internal VHCI receive shim. Must be called once before any other BT
 * or bluetooth HAL function. Returns BT_OK on success.
 */
BT_StatusType BT_init(void);

/*
 * Description :
 * Register the HAL callback that will be called for every inbound HCI packet.
 * Must be called before BT_init so the callback is in place before the
 * controller begins forwarding events.
 */
void BT_registerRecvCallback(BT_RecvCallbackType callback);

/*
 * Description :
 * Return TRUE if the VHCI transmit buffer is ready to accept a new packet.
 */
boolean BT_isSendAvailable(void);

/*
 * Description :
 * Send a raw HCI packet (buf, len bytes) to the BT controller via VHCI.
 * Caller must ensure BT_isSendAvailable() is TRUE before calling.
 */
void BT_sendPacket(const uint8 *buf, uint16 len);

#endif /* BT_H_ */
