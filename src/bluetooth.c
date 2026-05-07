/******************************************************************************
 *
 * Module: Bluetooth Classic SPP
 *
 * File Name: bluetooth.c
 *
 * Description: Source file for the Bluetooth Classic SPP driver (HAL layer).
 *              Implements raw HCI command sequences and ACL data framing
 *              using only the BT MCAL driver (bt.h). No ESP-IDF BT headers
 *              are included here.
 *
 *              Layer map:
 *                bluetooth_send / bluetooth_is_connected  ← application
 *                ──────────────────────────────────────────────────────
 *                HCI commands + ACL packets  (this file)  ← HAL
 *                bt.h / bt.c                              ← MCAL
 *                ESP32 BT radio hardware                  ← silicon
 *
 *******************************************************************************/

#include "bluetooth.h"
#include "bt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdint.h>

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

/* HCI packet type indicators */
#define HCI_PKT_CMD  0x01
#define HCI_PKT_ACL  0x02
#define HCI_PKT_EVT  0x04

/* HCI command opcodes (OGF<<10 | OCF, little-endian on wire) */
#define HCI_RESET                     0x0C03
#define HCI_SET_EVENT_MASK            0x0C01
#define HCI_WRITE_LOCAL_NAME          0x0C13
#define HCI_WRITE_COD                 0x0C24
#define HCI_WRITE_PAGE_TO             0x0C18
#define HCI_WRITE_SCAN_ENABLE         0x0C1A
#define HCI_WRITE_SIMPLE_PAIRING_MODE 0x0C56
#define HCI_ACCEPT_CONN               0x0409
#define HCI_IO_CAPABILITY_REPLY       0x042B
#define HCI_USER_CONFIRM_REPLY        0x042C
#define HCI_LINK_KEY_NEG_REPLY        0x040C
#define HCI_PIN_CODE_NEG_REPLY        0x040E

/* HCI event codes */
#define HCI_EVT_CMD_COMPLETE          0x0E
#define HCI_EVT_CMD_STATUS            0x0F
#define HCI_EVT_CONN_REQUEST          0x04
#define HCI_EVT_CONN_COMPLETE         0x03
#define HCI_EVT_DISC_COMPLETE         0x05
#define HCI_EVT_PIN_CODE_REQUEST      0x16
#define HCI_EVT_LINK_KEY_REQUEST      0x17
#define HCI_EVT_IO_CAPABILITY_REQUEST 0x31
#define HCI_EVT_USER_CONFIRM_REQUEST  0x33

/* ACL handle mask and Packet Boundary flag */
#define ACL_HANDLE_MASK  0x0FFF
#define ACL_PB_FIRST     0x20

/*******************************************************************************
 *                         Private Data                                        *
 *******************************************************************************/

static volatile boolean  connected  = FALSE;
static volatile uint16   acl_handle = 0xFFFF;
static SemaphoreHandle_t cmd_sem;

/*******************************************************************************
 *                         Private Functions                                   *
 *******************************************************************************/

/*
 * Build and send a raw HCI command packet via the BT MCAL.
 */
static void hci_send_cmd(uint16 opcode, const uint8 *params, uint8 plen)
{
    /* HCI command packet:
     *   [0]    packet type = 0x01
     *   [1..2] opcode little-endian
     *   [3]    parameter length
     *   [4..N] parameters            */
    uint8 buf[260];
    buf[0] = HCI_PKT_CMD;
    buf[1] = (uint8)(opcode & 0xFF);
    buf[2] = (uint8)(opcode >> 8);
    buf[3] = plen;
    if (plen && params)
        memcpy(&buf[4], params, plen);

    while (!BT_isSendAvailable())
        vTaskDelay(pdMS_TO_TICKS(1));

    BT_sendPacket(buf, (uint16)(4u + plen));
}

/* Send command and block until Command Complete event is received.
 * Returns pdTRUE if the response arrived, pdFALSE on timeout. */
static BaseType_t hci_cmd(uint16 opcode, const uint8 *params, uint8 plen)
{
    hci_send_cmd(opcode, params, plen);
    BaseType_t ok = xSemaphoreTake(cmd_sem, pdMS_TO_TICKS(2000));
    if (ok != pdTRUE)
        printf("[BT] WARNING: opcode 0x%04X timed out — no CMD_COMPLETE received\n", opcode);
    return ok;
}

/*
 * HCI event handler — registered with the BT MCAL as the receive callback.
 * Called from the BT controller task whenever an HCI packet arrives.
 */
static int hci_event_handler(uint8 *data, uint16 len)
{
    if (len < 3 || data[0] != HCI_PKT_EVT)
        return 0;

    uint8 evt = data[1];

    switch (evt)
    {
    case HCI_EVT_CMD_COMPLETE:
    case HCI_EVT_CMD_STATUS:
        xSemaphoreGive(cmd_sem);
        break;

    case HCI_EVT_CONN_REQUEST:
        /* bytes 3..8 = BD_ADDR of the remote device requesting connection */
        if (len >= 13) {
            uint8 params[7];
            memcpy(params, &data[3], 6);  /* BD_ADDR */
            params[6] = 0x01;             /* Role: remain slave */
            printf("[BT] Connection request — accepting\n");
            hci_send_cmd(HCI_ACCEPT_CONN, params, 7);
        }
        break;

    case HCI_EVT_CONN_COMPLETE:
        /* byte 3 = status, bytes 4-5 = connection handle */
        if (len >= 6 && data[3] == 0x00)
        {
            acl_handle = ((uint16)data[4] | ((uint16)data[5] << 8)) & ACL_HANDLE_MASK;
            connected  = TRUE;
            printf("[BT] HCI_EVT_CONN_COMPLETE — handle=0x%04X, connected\n", acl_handle);
        }
        else if (len >= 4)
        {
            printf("[BT] HCI_EVT_CONN_COMPLETE — status=0x%02X (failed)\n", data[3]);
        }
        break;

    case HCI_EVT_DISC_COMPLETE:
        connected  = FALSE;
        acl_handle = 0xFFFF;
        printf("[BT] Disconnected\n");
        break;

    case HCI_EVT_IO_CAPABILITY_REQUEST:
        /* Reply: NoInputNoOutput — forces Android into "Just Works" (no PIN) */
        if (len >= 9) {
            uint8 params[9];
            memcpy(params, &data[3], 6);  /* BD_ADDR */
            params[6] = 0x03;             /* IO_Capability: NoInputNoOutput */
            params[7] = 0x00;             /* OOB_Data_Present: No */
            params[8] = 0x00;             /* Authentication_Requirements: No bonding */
            printf("[BT] IO capability request — replying NoInputNoOutput\n");
            hci_send_cmd(HCI_IO_CAPABILITY_REPLY, params, 9);
        }
        break;

    case HCI_EVT_USER_CONFIRM_REQUEST:
        /* Auto-accept numeric comparison — Just Works, no PIN needed */
        if (len >= 9) {
            uint8 params[6];
            memcpy(params, &data[3], 6);
            printf("[BT] User confirmation request — auto-accepting\n");
            hci_send_cmd(HCI_USER_CONFIRM_REPLY, params, 6);
        }
        break;

    case HCI_EVT_LINK_KEY_REQUEST:
        /* No stored keys — reply negative to trigger fresh pairing */
        if (len >= 9) {
            uint8 params[6];
            memcpy(params, &data[3], 6);
            hci_send_cmd(HCI_LINK_KEY_NEG_REPLY, params, 6);
        }
        break;

    case HCI_EVT_PIN_CODE_REQUEST:
        /* Reject legacy PIN fallback — we use SSP only */
        if (len >= 9) {
            uint8 params[6];
            memcpy(params, &data[3], 6);
            hci_send_cmd(HCI_PIN_CODE_NEG_REPLY, params, 6);
        }
        break;

    default:
        break;
    }
    return 0;
}

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialise Bluetooth Classic. Powers on the controller via BT MCAL,
 * then issues the HCI sequence to set the device name, CoD, and enable scan.
 */
boolean bluetooth_init(const char *device_name)
{
    cmd_sem = xSemaphoreCreateBinary();

    /* Register HCI event handler before controller init so no events are lost */
    BT_registerRecvCallback(hci_event_handler);

    if (BT_init() != BT_OK)
    {
        printf("[BT] MCAL BT_init failed\n");
        return FALSE;
    }
    printf("[BT] MCAL init OK — controller powered on\n");

    /* 1. Reset — bring controller to a clean known state */
    if (hci_cmd(HCI_RESET, NULL, 0) != pdTRUE) return FALSE;
    printf("[BT] HCI_RESET OK\n");
    vTaskDelay(pdMS_TO_TICKS(200));

    /* 2. Event mask — enable all standard events + SSP events (0x31-0x38, byte 6) */
    uint8 evt_mask[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F };
    if (hci_cmd(HCI_SET_EVENT_MASK, evt_mask, 8) != pdTRUE) return FALSE;
    printf("[BT] Event mask set\n");

    /* 3. Enable Secure Simple Pairing — host opts in so controller uses SSP not legacy PIN */
    uint8 ssp = 0x01;
    if (hci_cmd(HCI_WRITE_SIMPLE_PAIRING_MODE, &ssp, 1) != pdTRUE) return FALSE;
    printf("[BT] Simple Pairing Mode enabled\n");

    /* 4. Local device name — 248 bytes, zero-padded (HCI spec Vol 4 §7.3.11) */
    uint8 name_buf[248];
    memset(name_buf, 0, sizeof(name_buf));
    strncpy((char *)name_buf, device_name, sizeof(name_buf) - 1);
    if (hci_cmd(HCI_WRITE_LOCAL_NAME, name_buf, sizeof(name_buf)) != pdTRUE) return FALSE;
    printf("[BT] Local name set: %s\n", device_name);

    /* 3. Class of Device: 0x002540 — Toy / Robot controller
     *    Transmitted little-endian: { 0x40, 0x25, 0x00 } */
    uint8 cod[3] = { 0x40, 0x25, 0x00 };
    if (hci_cmd(HCI_WRITE_COD, cod, 3) != pdTRUE) return FALSE;
    printf("[BT] Class of Device set\n");

    /* 4. Page timeout: 0x2000 slots × 0.625 ms ≈ 20 s */
    uint8 page_to[2] = { 0x00, 0x20 };
    if (hci_cmd(HCI_WRITE_PAGE_TO, page_to, 2) != pdTRUE) return FALSE;
    printf("[BT] Page timeout set\n");

    /* 5. Scan enable = 0x03 → inquiry scan ON + page scan ON */
    uint8 scan = 0x03;
    if (hci_cmd(HCI_WRITE_SCAN_ENABLE, &scan, 1) != pdTRUE) return FALSE;
    printf("[BT] Scan enabled — device is now discoverable as \"%s\"\n", device_name);

    return TRUE;
}

/*
 * Description :
 * Send a null-terminated string as a raw HCI ACL packet.
 */
void bluetooth_send(const char *msg)
{
    if (!connected || acl_handle == 0xFFFF)
        return;

    size_t dlen = strlen(msg);
    if (dlen == 0 || dlen > 251)
        return;

    /* HCI ACL data packet:
     *   [0]    packet type = 0x02
     *   [1]    handle low byte
     *   [2]    handle high nibble | PB flag
     *   [3..4] data length little-endian
     *   [5..N] payload               */
    uint8 buf[256];
    buf[0] = HCI_PKT_ACL;
    buf[1] = (uint8)(acl_handle & 0xFF);
    buf[2] = (uint8)(((acl_handle >> 8) & 0x0F) | ACL_PB_FIRST);
    buf[3] = (uint8)(dlen & 0xFF);
    buf[4] = (uint8)(dlen >> 8);
    memcpy(&buf[5], msg, dlen);

    while (!BT_isSendAvailable())
        vTaskDelay(pdMS_TO_TICKS(1));

    BT_sendPacket(buf, (uint16)(5u + dlen));
}

/*
 * Description :
 * Return TRUE if a remote device is currently connected.
 */
boolean bluetooth_is_connected(void)
{
    return connected;
}
