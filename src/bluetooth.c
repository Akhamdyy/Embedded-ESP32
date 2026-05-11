/******************************************************************************
 *
 * Module: UART Serial (HAL) — replaces Bluetooth SPP
 *
 * File Name: bluetooth.c
 *
 * Description: HAL source for UART2 serial communication.
 *              Thin wrapper over the UART MCAL (bt.h). Exposes the four
 *              application-facing functions defined in bluetooth.h without
 *              any direct ESP-IDF dependency.
 *
 *******************************************************************************/

#include "bluetooth.h"
#include "bt.h"
#include <string.h>

boolean bluetooth_init(const char *device_name)
{
    return (BT_initSPP(device_name) == BT_OK) ? TRUE : FALSE;
}

void bluetooth_send(const char *msg)
{
    size_t len = strlen(msg);
    if (len == 0 || len > 251) return;
    BT_sendSPP((const uint8 *)msg, (uint16)len);
}

boolean bluetooth_is_connected(void)
{
    return BT_isSPPConnected();
}

uint16 bluetooth_recv(uint8 *buf, uint16 maxlen)
{
    return BT_recvSPP(buf, maxlen);
}

void bluetooth_setRxCallback(bluetooth_rx_cb_t cb)
{
    BT_setRxCallback((BT_RxCallbackType)cb);
}
