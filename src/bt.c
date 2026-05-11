/******************************************************************************
 *
 * Module: BT (Bluetooth Controller + Bluedroid SPP)
 *
 * File Name: bt.c
 *
 * Description: MCAL source for Bluetooth Classic SPP.
 *              Owns all ESP-IDF BT headers. Initialises the BT controller,
 *              Bluedroid stack, GAP (Just Works SSP), and an SPP server slot.
 *              Exposes a minimal API so the HAL layer (bluetooth.c) has no
 *              direct ESP-IDF dependency.
 *
 *******************************************************************************/

#include "bt.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include "esp_err.h"

#include <string.h>
#include <stdio.h>

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static BT_ConnectedCallbackType    s_on_connect    = NULL;
static BT_DisconnectedCallbackType s_on_disconnect = NULL;
static BT_RxCallbackType           s_on_rx         = NULL;
static volatile boolean            s_connected     = FALSE;
static volatile uint32_t           s_spp_handle    = 0;
static const char                 *s_device_name   = NULL;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
            printf("[BT] Paired with: %s\n", param->auth_cmpl.device_name);
        else
            printf("[BT] Pairing failed, status=%d\n", param->auth_cmpl.stat);
        break;

    case ESP_BT_GAP_CFM_REQ_EVT:
        /* Numeric comparison — auto-accept (NoInputNoOutput = Just Works) */
        printf("[BT] SSP confirm — auto-accepting\n");
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;

    case ESP_BT_GAP_KEY_NOTIF_EVT:
        printf("[BT] SSP passkey: %06lu\n", (unsigned long)param->key_notif.passkey);
        break;

    default:
        break;
    }
}

static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        if (param->init.status == ESP_SPP_SUCCESS) {
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
        } else {
            printf("[BT] SPP init error, status=%d\n", param->init.status);
        }
        break;

    case ESP_SPP_START_EVT:
        if (param->start.status == ESP_SPP_SUCCESS) {
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            printf("[BT] SPP server ready — discoverable as \"%s\"\n",
                   s_device_name ? s_device_name : "");
        }
        break;

    case ESP_SPP_SRV_OPEN_EVT:
        s_spp_handle = param->srv_open.handle;
        s_connected  = TRUE;
        printf("[BT] Client connected, handle=%lu\n", (unsigned long)s_spp_handle);
        if (s_on_connect) s_on_connect();
        break;

    case ESP_SPP_CLOSE_EVT:
        s_connected  = FALSE;
        s_spp_handle = 0;
        printf("[BT] Client disconnected\n");
        if (s_on_disconnect) s_on_disconnect();
        break;

    case ESP_SPP_DATA_IND_EVT:
        if (s_on_rx && param->data_ind.data && param->data_ind.len > 0)
        {
            s_on_rx((const uint8 *)param->data_ind.data,
                    (uint16)param->data_ind.len);
        }
        break;

    case ESP_SPP_WRITE_EVT:
        break;

    default:
        break;
    }
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

BT_StatusType BT_initSPP(const char *device_name)
{
    s_device_name = device_name;
    esp_err_t ret;

    /* NVS — required by the BT controller */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) { printf("[BT] NVS init failed\n"); return BT_FAIL; }

    /* Release BLE memory — Classic BT only */
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    /* BT controller */
    esp_bt_controller_config_t hw_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&hw_cfg) != ESP_OK) {
        printf("[BT] Controller init failed\n"); return BT_FAIL;
    }
    if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
        printf("[BT] Controller enable failed\n"); return BT_FAIL;
    }
    printf("[BT] Controller OK\n");

    /* Bluedroid host stack */
    if (esp_bluedroid_init() != ESP_OK) {
        printf("[BT] Bluedroid init failed\n"); return BT_FAIL;
    }
    if (esp_bluedroid_enable() != ESP_OK) {
        printf("[BT] Bluedroid enable failed\n"); return BT_FAIL;
    }
    printf("[BT] Bluedroid OK\n");

    /* GAP — device name + NoInputNoOutput IO capability (Just Works pairing) */
    if (esp_bt_gap_register_callback(gap_callback) != ESP_OK) {
        printf("[BT] GAP callback failed\n"); return BT_FAIL;
    }
    esp_bt_gap_set_device_name(device_name);
    esp_bt_io_cap_t io_cap = ESP_BT_IO_CAP_NONE;
    esp_bt_gap_set_security_param(ESP_BT_SP_IOCAP_MODE, &io_cap, sizeof(io_cap));

    /* SPP */
    if (esp_spp_register_callback(spp_callback) != ESP_OK) {
        printf("[BT] SPP callback failed\n"); return BT_FAIL;
    }

    esp_spp_cfg_t spp_cfg = {
        .mode             = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size   = 0,
    };
    if (esp_spp_enhanced_init(&spp_cfg) != ESP_OK) {
        printf("[BT] SPP enhanced init failed\n"); return BT_FAIL;
    }

    printf("[BT] Init sequence done — waiting for SPP server start\n");
    return BT_OK;
}

void BT_setSPPCallbacks(BT_ConnectedCallbackType on_connect,
                        BT_DisconnectedCallbackType on_disconnect)
{
    s_on_connect    = on_connect;
    s_on_disconnect = on_disconnect;
}

void BT_sendSPP(const uint8 *data, uint16 len)
{
    if (!s_connected || s_spp_handle == 0) return;
    esp_spp_write(s_spp_handle, (int)len, (uint8_t *)data);
}

boolean BT_isSPPConnected(void)
{
    return s_connected;
}

void BT_setSPPRxCallback(BT_RxCallbackType cb)
{
    s_on_rx = cb;
}
