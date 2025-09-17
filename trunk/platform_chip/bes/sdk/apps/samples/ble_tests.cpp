/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "ble_tests.h"

#include "factory_section.h"
#include "hal_trace.h"
#include <string.h>
#include "ble_core_common.h"
#include "app_ble_mode_switch.h"
#include "gapm_le.h"
#include "app_ble_param_config.h"
#include "app_ble_custom_api.h"
#include "app.h"
#include <cstdlib>
#include "aob_dts_api.h"
#include "aob_dtc_api.h"

#define APP_DEMO_UUID           "\x03\x17\x03\xFE"
#define APP_DEMO_UUID_LEN       (4)
#define APP_DEMO_DATA           "\x02\x01\x06"
#define APP_DEMO_DATA_LEN       (3)
#define FLAG_DATA               "\x02\x01\x01"
#define FLAG_DATA_LEN           (3)
#define TEST_DTS_SPSM_0         0x0028

static void ble_demo_user_data_fill_handler(void *param);

void get_ble_addr_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t *ble_addr = factory_section_get_ble_address();
    TRACE(0, "ble address:");
    DUMP8("0x%02x ", ble_addr, 6);
}

void get_ble_name_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t *ble_name = factory_section_get_ble_name();
    TRACE(0, "device ble name: %s", (unsigned char *)ble_name);
}

void set_ble_address_test(char* cmdParam, uint32_t cmdParam_len)
{
    unsigned char new_ble_address[7] = "\x50\x33\x33\x22\x11\x11";
    int isSuccessful = factory_section_set_ble_address((uint8_t *)new_ble_address);

    if (!isSuccessful)
    {
        TRACE(0, "set ble address success");
        uint8_t *new_ble_addr = factory_section_get_ble_address();

        TRACE(0, "ble address:");
        DUMP8("0x%02x ", new_ble_addr, 6);
    }
    else
    {
        TRACE(0, "set ble address failed");
    }
}

void set_ble_name_test(char* cmdParam, uint32_t cmdParam_len)
{
    char *new_ble_name = (char *)"new_BLE_name";
    int len = strlen(new_ble_name);
    TRACE(0, "%s: new name = %s, len = %d", __func__, new_ble_name, len);

    int isSuccessful = factory_section_set_ble_name(new_ble_name);

    if (0 == isSuccessful)
    {
        TRACE(0, "set ble name success");
        uint8_t *ble_name = factory_section_get_ble_name();
        TRACE(0, "device ble name: %s", (unsigned char *)ble_name);
    }
    else
    {
        TRACE(0, "set ble name failed");
    }
}

static void ble_demo_user_data_fill_handler(void *param)
{
    BLE_ADV_PARAM_T *cmd = (BLE_ADV_PARAM_T*)param;

    memcpy(&cmd->advData[cmd->advDataLen], APP_DEMO_UUID, APP_DEMO_UUID_LEN);
    cmd->advDataLen += APP_DEMO_UUID_LEN;

    memcpy(&cmd->scanRspData[cmd->scanRspDataLen], APP_DEMO_DATA, APP_DEMO_DATA_LEN);
    cmd->scanRspDataLen += APP_DEMO_DATA_LEN;

    cmd->advType = ADV_TYPE_CONN_EXT_ADV;
    cmd->advMode = ADV_MODE_EXTENDED;
    cmd->advInterval = BLE_ADVERTISING_INTERVAL;
    cmd->advUserInterval[USER_BLE_DEMO0] = BLE_ADVERTISING_INTERVAL + 20;
    cmd->advTxPwr = BLE_ADV_TX_POWER_LEVEL_1;

    cmd->localAddr[0] = 0x31;
    cmd->localAddr[1] = 0x32;
    cmd->localAddr[2] = 0x33;
    cmd->localAddr[3] = 0x24;
    cmd->localAddr[4] = 0x11;
    cmd->localAddr[5] = 0x11;
    cmd->localAddrType = 1;

    //Add flag data
    cmd->isBleFlagsAdvDataConfiguredByAppLayer = true;
    memcpy(&cmd->advData[cmd->advDataLen], FLAG_DATA, FLAG_DATA_LEN);
    cmd->advDataLen += FLAG_DATA_LEN;

    app_ble_data_fill_enable(USER_BLE_DEMO0, true);
}

void ble_adv_user_register_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ble_register_data_fill_handle(USER_BLE_DEMO0, (BLE_DATA_FILL_FUNC_T)ble_demo_user_data_fill_handler, true);
}

void ble_audio_close_adv_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0,"%s", __func__);
    app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
}

void ble_audio_open_adv_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0,"%s", __func__);
    app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, true);
}

void ble_set_adv_type_test(char *cmdParam, uint32_t cmdParam_len)
{
    BLE_ADV_TYPE_E adv_type = ADV_TYPE_UNDIRECT;

    int value = 0;
    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &value);
    }

    switch(value)
    {
        case 0:
            adv_type = ADV_TYPE_UNDIRECT;
            break;
        case 1:
            adv_type = ADV_TYPE_DIRECT_LDC;
            break;
        case 2:
            adv_type = ADV_TYPE_DIRECT_HDC;
            break;
        case 3:
            adv_type = ADV_TYPE_NON_CONN_SCAN;
            break;
        case 4:
            adv_type = ADV_TYPE_NON_CONN_NON_SCAN;
            break;
        case 5:
            adv_type = ADV_TYPE_CONN_EXT_ADV;
            break;
        case 6:
            adv_type = ADV_TYPE_EXT_CON_DIRECT;
            break;
        default:
            break;
    }

    ble_bdaddr_t peer_addr = {{0x51, 0x33, 0x33, 0x22, 0x11, 0x11}, GAPM_GEN_RSLV_ADDR};

    app_ble_set_adv_type(adv_type, &peer_addr);
}

void ble_param_set_adv_interval_test(char *cmdParam, uint32_t cmdParam_len)
{
    BLE_ADV_INTERVALREQ_USER_E adv_intv_user = BLE_ADV_INTERVALREQ_USER_TWS_STM;
    BLE_ADV_USER_E adv_user = USER_BLE_DEMO0;
    int interval = 100;
    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &interval);
    }

    app_ble_param_set_adv_interval(adv_intv_user, adv_user, (uint32_t)interval);
}

void ble_set_adv_txpwr_by_actv_user_test(char *cmdParam, uint32_t cmdParam_len)
{
    int txpwr_dbm = 5;
    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &txpwr_dbm);
    }

    BLE_ADV_ACTIVITY_USER_E actv_user = BLE_ADV_ACTIVITY_USER_0;

    app_ble_set_adv_txpwr_by_actv_user(actv_user, txpwr_dbm);
}

void ble_set_adv_txpwr_by_adv_user_test(char *cmdParam, uint32_t cmdParam_len)
{
    int txpwr_dbm = 5;
    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &txpwr_dbm);
    }

    BLE_ADV_USER_E ble_adv_user = USER_BLE_DEMO1;

    app_ble_set_adv_txpwr_by_adv_user(ble_adv_user, txpwr_dbm);
}

void ble_set_all_adv_txpwr_test(char *cmdParam, uint32_t cmdParam_len)
{
    int txpwr_dbm = 5;
    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &txpwr_dbm);
    }

    app_ble_set_all_adv_txpwr(txpwr_dbm);
}

void ble_set_adv_local_addr_by_actv_user_test(char *cmdParam, uint32_t cmdParam_len)
{
    BLE_ADV_ACTIVITY_USER_E actv_user = BLE_ADV_ACTIVITY_USER_0;
    uint8_t address[6]  = {0x41,0x33, 0x33, 0x22, 0x11, 0x11};
    app_ble_set_adv_local_addr_by_actv_user(actv_user, address);
}

void ble_set_adv_local_addr_by_adv_user_test(char *cmdParam, uint32_t cmdParam_len)
{
    BLE_ADV_USER_E adv_user = USER_BLE_DEMO1;
    uint8_t address[6] = {0x61, 0x33, 0x33, 0x22, 0x11, 0x11};
    app_ble_set_adv_local_addr_by_adv_user(adv_user, address);
}

void ble_user_adv_start_test(char *cmdParam, uint32_t cmdParam_len)
{
    app_ble_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
}

void ble_custom_adv_start_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint8_t adv_address[6] = {0x31, 0x33, 0x33, 0x22, 0x11, 0x11};

    app_ble_custom_init();
    app_ble_custom_adv_write_data(BLE_ADV_ACTIVITY_USER_0,
                                  true,
                                  BLE_ADV_PRIVATE_STATIC,
                                  (uint8_t *)adv_address,
                                  NULL,
                                  BLE_ADVERTISING_INTERVAL,
                                  ADV_TYPE_UNDIRECT,
                                  ADV_MODE_LEGACY,
                                  12,
                                  (uint8_t *)APP_DEMO_DATA, APP_DEMO_DATA_LEN,
                                  NULL, 0);

    app_ble_custom_adv_start(BLE_ADV_ACTIVITY_USER_0);
}

void ble_start_scan_test(char *cmdParam, uint32_t cmdParam_len)
{
    BLE_SCAN_PARAM_T scan_param = {0};

    scan_param.scanFolicyType = BLE_SCAN_ALLOW_ADV_ALL_AND_INIT_RPA;
    scan_param.scanWindowMs   = 20;
    scan_param.scanIntervalMs   = 60;
    app_ble_start_scan(&scan_param);
}

void ble_stop_scan_test(char *cmdParam, uint32_t cmdParam_len)
{
    app_ble_stop_scan();
}

void ble_start_connect_test(char *cmdParam, uint32_t cmdParam_len)
{
    POSSIBLY_UNUSED ble_bdaddr_t ble_bdaddr =
    {
        .addr = {0x51,0x33,0x33,0x22,0x11,0x11},    // Default address
        .addr_type = 1,                             // private static address
    };

    int value[6];

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%x %x %x %x %x %x", &value[0], &value[1],
            &value[2], &value[3], &value[4], &value[5]);

        for (uint8_t i = 0; i < 6; i++)
        {
            ble_bdaddr.addr[i] = value[i];
        }
    }

    app_ble_start_connect(&ble_bdaddr, APP_GAPM_STATIC_ADDR);
}

void ble_start_connect_with_init_type_test(char *cmdParam, uint32_t cmdParam_len)
{
    POSSIBLY_UNUSED ble_bdaddr_t ble_bdaddr =
    {
        .addr = {0x51,0x33,0x33,0x22,0x11,0x11},    // Default address
        .addr_type = 0,
    };

    int value[6];

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%x %x %x %x %x %x", &value[0], &value[1],
            &value[2], &value[3], &value[4], &value[5]);

        for (uint8_t i = 0; i < 6; i++)
        {
            ble_bdaddr.addr[i] = value[i];
        }
    }

    app_ble_start_connect_with_init_type(APP_BLE_INIT_TYPE_DIRECT_CONN_EST, &ble_bdaddr, APP_GAPM_STATIC_ADDR, 0);
}

void ble_is_connection_on_by_addr_test(char *cmdParam, uint32_t cmdParam_len)
{
    POSSIBLY_UNUSED uint8_t peer_address[6] = {0x51, 0x33, 0x33, 0x22, 0x11, 0x11};

    int value[6];

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%x %x %x %x %x %x", &value[0], &value[1],
            &value[2], &value[3], &value[4], &value[5]);

        for (uint8_t i = 0; i < 6; i++)
        {
            peer_address[i] = (uint8_t)value[i];
        }
    }

    bool isConnected = app_ble_is_connection_on_by_addr(peer_address);

    if (isConnected)
    {
        TRACE(0, "tws_connected");
    }
    else
    {
        TRACE(0, "tws_disconnected");
    }
}

void ble_is_connection_on_by_conIdx_test(char *cmdParam, uint32_t cmdParam_len)
{
    int conIdx = 0;

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &conIdx);
    }

    bool isConnected = app_ble_is_connection_on((uint8_t)conIdx);

    if (isConnected)
    {
        TRACE(0, "tws_connected");
    }
    else
    {
        TRACE(0, "tws_disconnected");
    }
}

void ble_cancel_connecting_test(char *cmdParam, uint32_t cmdParam_len)
{
    app_ble_cancel_connecting();
}

void ble_start_disconnect_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint8_t conIdx = 0;
    app_ble_start_disconnect(conIdx);
}

void ble_update_conn_param_mode_test(char *cmdParam, uint32_t cmdParam_len)
{
    int connextion_mode;

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%d", &connextion_mode);
    }

    BLE_CONN_PARAM_MODE_E ble_conn_param_mode = BLE_CONN_PARAM_MODE_E(connextion_mode);

    app_ble_update_conn_param_mode(ble_conn_param_mode, true);
}

void ble_get_conn_param_test(char *cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    uint8_t conidx = 0;
    APP_BLE_CONN_PARAM_T *pConnParam = (APP_BLE_CONN_PARAM_T *)malloc(sizeof(APP_BLE_CONN_PARAM_T));

    bool isSuccess = app_ble_get_conn_param(conidx, pConnParam);

    if (isSuccess)
    {
        TRACE(0, "Get connection param success");
        TRACE(0, "Connection param:\n interval: %d latency: %d", pConnParam->con_interval, pConnParam->con_latency);
    }
    else
    {
        TRACE(0, "Get connection param failed");
    }

    free(pConnParam);
}

void aob_dts_configure_test(char *cmdParam, uint32_t cmdParam_len)
{
    aob_dts_register_spsm(TEST_DTS_SPSM_0, 6);
}

void aob_dts_send_data_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint8_t a[10];
    memset(a, 0x28, 10);
    aob_dts_send_data(0, TEST_DTS_SPSM_0, 10, (const uint8_t *)a);
}

void aob_dts_disconnect_test(char *cmdParam, uint32_t cmdParam_len)
{
    aob_dts_disconnect(0, TEST_DTS_SPSM_0);
}

#ifdef AOB_MOBILE_ENABLED
void aob_dtc_connect_test(char *cmdParam, uint32_t cmdParam_len)
{
    aob_dtc_connect(0, 512, TEST_DTS_SPSM_0);
}

void aob_dtc_send_data_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint8_t a[10];
    memset(a, 0x28, 10);
    aob_dtc_send_data(0, TEST_DTS_SPSM_0, 10, a);
}

void aob_dtc_disconnect_test(char *cmdParam, uint32_t cmdParam_len)
{
    aob_dtc_disconnect(0, TEST_DTS_SPSM_0);
}
#endif
