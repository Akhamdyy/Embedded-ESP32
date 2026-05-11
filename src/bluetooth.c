/******************************************************************************
 *
 * Module: Bluetooth Classic SPP
 *
 * File Name: bluetooth.c
 *
 * Description: HAL source for Bluetooth Classic SPP.
 *              Thin wrapper over the BT MCAL (bt.h). Exposes the three
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

void bluetooth_setRxCallback(bluetooth_rx_callback_t cb)
{
    BT_setSPPRxCallback((BT_RxCallbackType)cb);
}
