/******************************************************************************
 *
 * Module: Bluetooth Classic SPP
 *
 * File Name: bluetooth.h
 *
 * Description: Header file for the Bluetooth Classic SPP driver (HAL layer).
 *              Provides device discovery, connection tracking, and raw HCI
 *              ACL data transmission over the BT MCAL driver (bt.h).
 *
 *              The Android "Serial Bluetooth Terminal" app initiates the
 *              RFCOMM/SPP channel; this driver makes the device discoverable
 *              and connectable via raw HCI commands.
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
 * Initialise Bluetooth Classic. Powers on the controller, sets the device
 * name, Class of Device, page timeout, and enables inquiry + page scan.
 * Returns TRUE on success, FALSE on any failure.
 * Must be called once before bluetooth_send.
 */
boolean bluetooth_init(const char *device_name);

/*
 * Description :
 * Send a null-terminated string as a raw HCI ACL packet over the active
 * connection. No-op if not connected or msg is empty / longer than 251 bytes.
 */
void bluetooth_send(const char *msg);

/*
 * Description :
 * Return TRUE if a remote device is currently connected.
 */
boolean bluetooth_is_connected(void);

#endif /* BLUETOOTH_H_ */
