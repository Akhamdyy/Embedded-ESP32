/******************************************************************************
 *
 * Module: BT (MCAL)
 *
 * File Name: bt.c
 *
 * Description: Bluedroid Classic SPP server on ESP32.
 *              Uses ESP-IDF Bluetooth stack (the only way to access the
 *              ESP32 BT radio — no register-level alternative exists for
 *              the Bluetooth co-processor).
 *
 *              Exposes the same BT_* API as bt.h so the HAL layer
 *              (bluetooth.c) requires zero changes.
 *
 *******************************************************************************/

#include "bt.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>

/*******************************************************************************
 *                          Private Definitions                                *
 *******************************************************************************/

#define RX_BUF_SIZE     256u
#define SPP_SERVER_NAME "SPP_SERVER"
#define SPP_PIN_CODE    "1234"

#define BT_CHECK(fn, label)                                                  \
    do {                                                                     \
        esp_err_t _e = (fn);                                                 \
        if (_e != ESP_OK) {                                                  \
            printf("[BT] FAIL %s: %s\n", (label), esp_err_to_name(_e));     \
            return BT_FAIL;                                                  \
        }                                                                    \
        printf("[BT] OK: %s\n", (label));                                    \
    } while (0)

/*******************************************************************************
 *                          Private Data                                       *
 *******************************************************************************/

static char    s_device_name[32]  = "ESP32";
static uint32  s_spp_handle       = 0u;
static volatile boolean s_connected       = FALSE;

static BT_ConnectedCallbackType    s_on_connect    = NULL;
static BT_DisconnectedCallbackType s_on_disconnect = NULL;

static uint8           s_rx_buf[RX_BUF_SIZE];
static volatile uint16 s_rx_head = 0u;
static volatile uint16 s_rx_tail = 0u;

/*******************************************************************************
 *                          Ring Buffer                                        *
 *******************************************************************************/

static void prv_rxbuf_push(uint8 c)
{
    uint16 next = (uint16)((s_rx_head + 1u) % RX_BUF_SIZE);
    if (next != s_rx_tail) { s_rx_buf[s_rx_head] = c; s_rx_head = next; }
}

static boolean prv_rxbuf_pop(uint8 *c)
{
    if (s_rx_head == s_rx_tail) return FALSE;
    *c = s_rx_buf[s_rx_tail];
    s_rx_tail = (uint16)((s_rx_tail + 1u) % RX_BUF_SIZE);
    return TRUE;
}

/*******************************************************************************
 *                          GAP Callback                                       *
 *******************************************************************************/

static void prv_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
            printf("[BT] Paired with: %s\n", param->auth_cmpl.device_name);
        else
            printf("[BT] Pairing FAILED status=%d\n", param->auth_cmpl.stat);
        break;

    case ESP_BT_GAP_PIN_REQ_EVT:
        /* Legacy pairing — send our fixed PIN */
        printf("[BT] PIN requested (min_16=%d)\n", param->pin_req.min_16_digit);
        {
            esp_bt_pin_code_t pin;
            memset(pin, 0, sizeof(pin));
            memcpy(pin, SPP_PIN_CODE, strlen(SPP_PIN_CODE));
            esp_bt_gap_pin_reply(param->pin_req.bda, true, (uint8_t)strlen(SPP_PIN_CODE), pin);
        }
        break;

    case ESP_BT_GAP_CFM_REQ_EVT:
        /* Secure Simple Pairing — auto-confirm */
        printf("[BT] SSP confirm num=%lu\n", (unsigned long)param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;

    case ESP_BT_GAP_KEY_NOTIF_EVT:
        printf("[BT] Passkey: %lu\n", (unsigned long)param->key_notif.passkey);
        break;

    case ESP_BT_GAP_MODE_CHG_EVT:
        printf("[BT] Mode changed: %d\n", param->mode_chg.mode);
        break;

    default:
        break;
    }
}

/*******************************************************************************
 *                          SPP Callback                                       *
 *******************************************************************************/

static void prv_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        if (param->init.status == ESP_SPP_SUCCESS)
        {
            printf("[BT] SPP init OK — starting server\n");
            esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        }
        else
        {
            printf("[BT] SPP init FAILED status=%d\n", param->init.status);
        }
        break;

    case ESP_SPP_START_EVT:
        if (param->start.status == ESP_SPP_SUCCESS)
        {
            printf("[BT] SPP server ready (scn=%d) — discoverable as \"%s\"\n",
                   param->start.scn, s_device_name);
            /* Make discoverable only once the server is listening */
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        }
        else
        {
            printf("[BT] SPP server start FAILED status=%d\n", param->start.status);
        }
        break;

    case ESP_SPP_SRV_OPEN_EVT:
        if (param->srv_open.status == ESP_SPP_SUCCESS)
        {
            s_spp_handle = param->srv_open.handle;
            s_connected  = TRUE;
            printf("[BT] Connected (handle=%lu)\n", (unsigned long)s_spp_handle);
            if (s_on_connect != NULL) s_on_connect();
        }
        break;

    case ESP_SPP_CLOSE_EVT:
        s_spp_handle = 0u;
        s_connected  = FALSE;
        printf("[BT] Disconnected — restarting server\n");
        if (s_on_disconnect != NULL) s_on_disconnect();
        /* Stop the old slot first so the SCN doesn't keep incrementing;
           the resulting UNINIT_EVT is ignored (server name won't match). */
        esp_spp_stop_srv();
        break;

    case ESP_SPP_UNINIT_EVT:
        /* Fired after esp_spp_stop_srv() — re-register with the same SCN slot. */
        esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        break;

    case ESP_SPP_DATA_IND_EVT:
        {
            uint16 i;
            for (i = 0u; i < (uint16)param->data_ind.len; i++)
                prv_rxbuf_push(param->data_ind.data[i]);
        }
        break;

    case ESP_SPP_WRITE_EVT:
        break;

    default:
        break;
    }
}

/*******************************************************************************
 *                          Public API                                         *
 *******************************************************************************/

BT_StatusType BT_initSPP(const char *device_name)
{
    esp_err_t err;

    /* NVS required by Bluetooth stack */
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    printf("[BT] NVS OK\n");

    /* Free BLE controller memory — Classic BT only */
    err = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    printf("[BT] mem_release(BLE): %s\n", esp_err_to_name(err));

    /* BT controller */
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    BT_CHECK(esp_bt_controller_init(&bt_cfg),                        "bt_controller_init");
    BT_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT),       "bt_controller_enable");

    /* Bluedroid */
    BT_CHECK(esp_bluedroid_init(),   "bluedroid_init");
    BT_CHECK(esp_bluedroid_enable(), "bluedroid_enable");

    /* Callbacks must be registered before SPP init */
    BT_CHECK(esp_bt_gap_register_callback(prv_gap_callback), "gap_register_callback");
    BT_CHECK(esp_spp_register_callback(prv_spp_callback),    "spp_register_callback");

    /* Secure Simple Pairing — Just Works (no confirmation dialog on either side) */
    esp_bt_sp_param_t sp = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t   iocap = ESP_BT_IO_CAP_NONE;
    esp_bt_gap_set_security_param(sp, &iocap, sizeof(iocap));
    printf("[BT] SSP IO cap: Just Works\n");

    /* Legacy PIN pairing fallback */
    esp_bt_pin_type_t  pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t  pin_code;
    memset(pin_code, 0, sizeof(pin_code));
    memcpy(pin_code, SPP_PIN_CODE, strlen(SPP_PIN_CODE));
    esp_bt_gap_set_pin(pin_type, (uint8_t)strlen(SPP_PIN_CODE), pin_code);
    printf("[BT] Legacy PIN set to %s\n", SPP_PIN_CODE);

    /* Device name — set before SPP init */
    const char *name = (device_name != NULL) ? device_name : "ESP32";
    strncpy(s_device_name, name, sizeof(s_device_name) - 1u);
    s_device_name[sizeof(s_device_name) - 1u] = '\0';
    esp_bt_gap_set_device_name(s_device_name);
    printf("[BT] Name set to \"%s\"\n", s_device_name);

    /* SPP init — START_EVT callback sets scan mode once server is ready */
    esp_spp_cfg_t spp_cfg = {
        .mode              = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size    = 0u,
    };
    BT_CHECK(esp_spp_enhanced_init(&spp_cfg), "spp_enhanced_init");

    return BT_OK;
}

void BT_setSPPCallbacks(BT_ConnectedCallbackType    on_connect,
                        BT_DisconnectedCallbackType on_disconnect)
{
    s_on_connect    = on_connect;
    s_on_disconnect = on_disconnect;
}

void BT_sendSPP(const uint8 *data, uint16 len)
{
    if (!s_connected || s_spp_handle == 0u || len == 0u) return;
    esp_spp_write(s_spp_handle, (int)len, (uint8_t *)data);
}

boolean BT_isSPPConnected(void)
{
    return s_connected;
}

uint16 BT_recvSPP(uint8 *buf, uint16 maxlen)
{
    uint16 count = 0u;
    while (count < maxlen)
    {
        uint8 c;
        if (!prv_rxbuf_pop(&c)) break;
        buf[count++] = c;
    }
    return count;
}
