/******************************************************************************
 *
 * Module: BT (Bluetooth Controller)
 *
 * File Name: bt.c
 *
 * Description: Source file for the ESP32 Bluetooth Classic controller driver
 *              (MCAL layer). Wraps ESP-IDF esp_bt.h and nvs_flash.h so that
 *              higher-level HAL drivers have no direct ESP-IDF dependency.
 *
 *******************************************************************************/

#include "bt.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "esp_err.h"

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

/* HAL-registered callback — forwarded from the internal VHCI shim */
static BT_RecvCallbackType user_recv_cb = NULL;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

/*
 * Internal VHCI receive shim — matches esp_vhci_host_callback_t signature.
 * Forwards every inbound HCI packet to the HAL-registered callback.
 */
static int internal_recv(uint8_t *data, uint16_t len)
{
    if (user_recv_cb != NULL)
    {
        return user_recv_cb((uint8 *)data, (uint16)len);
    }
    return 0;
}

static const esp_vhci_host_callback_t vhci_cb = {
    .notify_host_send_available = NULL,
    .notify_host_recv           = internal_recv,
};

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialise NVS, power on the BT controller in Classic BT mode, and
 * register the internal VHCI receive shim.
 */
BT_StatusType BT_init(void)
{
    /* NVS is required by the BT controller internals */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) return BT_FAIL;

    /* Release BLE memory — Classic BT only */
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    /* Power on and configure the BT radio */
    esp_bt_controller_config_t hw_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&hw_cfg);
    if (ret != ESP_OK) return BT_FAIL;

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) return BT_FAIL;

    /* Register raw HCI callback — bypasses Bluedroid stack entirely */
    ret = esp_vhci_host_register_callback(&vhci_cb);
    if (ret != ESP_OK) return BT_FAIL;

    return BT_OK;
}

/*
 * Description :
 * Register the HAL callback for inbound HCI packets.
 */
void BT_registerRecvCallback(BT_RecvCallbackType callback)
{
    user_recv_cb = callback;
}

/*
 * Description :
 * Return TRUE if the VHCI transmit buffer is ready.
 */
boolean BT_isSendAvailable(void)
{
    return esp_vhci_host_check_send_available() ? TRUE : FALSE;
}

/*
 * Description :
 * Send a raw HCI packet to the BT controller via VHCI.
 */
void BT_sendPacket(const uint8 *buf, uint16 len)
{
    esp_vhci_host_send_packet((uint8_t *)buf, (uint16_t)len);
}
