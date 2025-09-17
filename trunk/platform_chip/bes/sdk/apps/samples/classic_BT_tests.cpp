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
#include "classic_BT_tests.h"

#include "hal_trace.h"
#include "bt_common_define.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_conn.h"
#include "nvrecord_extension.h"
#include "app_ibrt_middleware.h"
#include "app_tws_ibrt_core_type.h"
#include "app_tws_ibrt_ui_test.h"
#include "app_ui_evt.h"
#include "factory_section.h"
#include "app_ibrt_customif_ui.h"
#include "app_ui_api.h"
#include "earbud_profiles_api.h"
#include "earbud_ux_api.h"
#include "earbud_ux_duplicate_api.h"

static const uint8_t testBtAddr[6] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
static const uint8_t testSlaveBtAddr[6] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

static void app_ibrt_global_state_changed(ibrt_global_state_change_event *state);

void app_ibrt_if_get_bt_local_address_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t* localBtAddr = app_ibrt_if_get_bt_local_address();
    TRACE(0, "app_ibrt_if_get_bt_local_address: ");
    DUMP8("%02x ",localBtAddr, 6);
}

void app_ibrt_if_get_bt_peer_address_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t* peerAddr = app_ibrt_if_get_bt_peer_address();
    TRACE(0, "app_ibrt_if_get_bt_local_address: ");
    DUMP8("%02x ",peerAddr, 6);
}

void app_ibrt_if_write_bt_local_address_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_ibrt_if_write_bt_local_address((uint8_t *)testBtAddr);
}

void app_ibrt_if_get_a2dp_state_test(char* cmdParam, uint32_t cmdParam_len)
{
    const char* a2dp_state_strings[] = {
        "IDLE",
        "CODEC_CONFIGURED",
        "OPEN",
        "STREAMING",
        "CLOSED",
        "ABORTING"
    };
    bt_bdaddr_t remote_mobile_addr_list[BT_DEVICE_NUM];
    bt_bdaddr_t remote_mobile_addr;

    uint8_t connected_mobile_count = app_ibrt_if_get_mobile_connected_dev_list(remote_mobile_addr_list);
    TRACE(0, "%s: connected_mobile_count: %d", __func__, connected_mobile_count);
    remote_mobile_addr = *remote_mobile_addr_list;
    TRACE(0, "%s: remote_mobile_addr: ", __func__);
    DUMP8("%02x ", remote_mobile_addr.address, BTIF_BD_ADDR_SIZE);

    AppIbrtA2dpState a2dp_state;
    AppIbrtStatus app_ibrt_status = app_ibrt_if_get_a2dp_state(&remote_mobile_addr, &a2dp_state);

    if (IBRT_PROFILE_STATUS_SUCCESS == app_ibrt_status)
    {
        TRACE(0,"ibrt_ui_log: a2dp_state = %s",a2dp_state_strings[a2dp_state]);
    }
    else
    {
        TRACE(0,"ibrt_ui_log: get a2dp state error");
    }
}

void app_ibrt_if_get_avrcp_state_test(char* cmdParam, uint32_t cmdParam_len)
{
    const char* avrcp_state_strings[] = {
        "DISCONNECTED",
        "CONNECTED",
        "PLAYING",
        "PAUSED",
        "VOLUME_UPDATED"
    };

    bt_bdaddr_t remote_mobile_addr_list[BT_DEVICE_NUM];
    bt_bdaddr_t remote_mobile_addr;

    uint8_t connected_mobile_count = app_ibrt_if_get_mobile_connected_dev_list(remote_mobile_addr_list);
    TRACE(0, "%s: connected_mobile_count: %d", __func__, connected_mobile_count);
    remote_mobile_addr = *remote_mobile_addr_list;
    TRACE(0, "%s: remote_mobile_addr: ", __func__);
    DUMP8("%02x ", remote_mobile_addr.address, BTIF_BD_ADDR_SIZE);

    AppIbrtAvrcpState avrcp_state;
    AppIbrtStatus app_ibrt_status = app_ibrt_if_get_avrcp_state(&remote_mobile_addr, &avrcp_state);
    if (IBRT_PROFILE_STATUS_SUCCESS == app_ibrt_status)
    {
        TRACE(0, "ibrt_ui_log: avrcp_state = %s", avrcp_state_strings[avrcp_state]);
    }
    else
    {
        TRACE(0,"ibrt_ui_log: get avrcp state error");
    }
}

void app_ibrt_if_get_hfp_state_test(char* cmdParam, uint32_t cmdParam_len)
{
    const char* hfp_state_strings[] = {
        "SLC_DISCONNECTED",
        "CLOSED",
        "SCO_CLOSED",
        "PENDING",
        "SLC_OPEN",
        "NEGOTIATE",
        "CODEC_CONFIGURED",
        "SCO_OPEN",
        "INCOMING_CALL",
        "OUTGOING_CALL",
        "RING_INDICATION"
    };

    bt_bdaddr_t remote_mobile_addr_list[BT_DEVICE_NUM];
    bt_bdaddr_t remote_mobile_addr;

    uint8_t connected_mobile_count = app_ibrt_if_get_mobile_connected_dev_list(remote_mobile_addr_list);
    TRACE(0, "%s: connected_mobile_count: %d", __func__, connected_mobile_count);
    remote_mobile_addr = *remote_mobile_addr_list;
    TRACE(0, "%s: remote_mobile_addr: ", __func__);
    DUMP8("%02x ", remote_mobile_addr.address, BTIF_BD_ADDR_SIZE);

    AppIbrtHfpState hfp_state;
    AppIbrtStatus app_ibrt_status = app_ibrt_if_get_hfp_state(&remote_mobile_addr, &hfp_state);
    if (IBRT_PROFILE_STATUS_SUCCESS == app_ibrt_status)
    {
        TRACE(0,"ibrt_ui_log: hfp_state = %s", hfp_state_strings[hfp_state]);
    }
    else
    {
        TRACE(0,"ibrt_ui_log: get hfp state error");
    }
}

void app_ibrt_if_get_hfp_call_status_test(char* cmdParam, uint32_t cmdParam_len)
{
    const char* call_status_strings[] = {
        "NO_CALL",
        "CALL_ACTIVE",
        "HOLD",
        "INCOMMING",
        "OUTGOING",
        "ALERT"
    };

    bt_bdaddr_t remote_mobile_addr_list[BT_DEVICE_NUM];
    bt_bdaddr_t remote_mobile_addr;

    uint8_t connected_mobile_count = app_ibrt_if_get_mobile_connected_dev_list(remote_mobile_addr_list);
    TRACE(0, "%s: connected_mobile_count: %d", __func__, connected_mobile_count);
    remote_mobile_addr = *remote_mobile_addr_list;
    TRACE(0, "%s: remote_mobile_addr: ", __func__);
    DUMP8("%02x ", remote_mobile_addr.address, BTIF_BD_ADDR_SIZE);

    AppIbrtCallStatus call_status;
    AppIbrtStatus app_ibrt_status = app_ibrt_if_get_hfp_call_status(&remote_mobile_addr, &call_status);
    if (IBRT_PROFILE_STATUS_SUCCESS == app_ibrt_status)
    {
        TRACE(0,"ibrt_ui_log: hfp_call_status = %s", call_status_strings[call_status]);
    }
    else
    {
        TRACE(0,"ibrt_ui_log: get hfp call status error");
    }
}

void app_ibrt_if_get_connected_mobile_count_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t result = app_ibrt_if_get_connected_mobile_count();
    TRACE(0, "app_ibrt_if_get_connected_mobile_count = %d", result);
}

void app_ibrt_if_get_mobile_connected_dev_list_test(char* cmdParam, uint32_t cmdParam_len)
{
    bt_bdaddr_t remote_mobile_addr_list[BT_DEVICE_NUM];
    uint8_t mobile_count = app_ibrt_if_get_mobile_connected_dev_list(remote_mobile_addr_list);
    TRACE(0, "%s: currently connected mobile device num = %d", __func__, mobile_count);
    ASSERT(BT_DEVICE_NUM >= mobile_count, "current mobile_count larger than size of mobile_addr_list");
    for (uint8_t i = 0; i < mobile_count; i++)
    {
        TRACE(0, "remote_mobile_addr_list[%d] = ", i);
        DUMP8("%02x ", remote_mobile_addr_list[i].address, BTIF_BD_ADDR_SIZE);
    }
}

void app_ibrt_if_is_tws_role_switch_on_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result;
    result = app_ibrt_if_is_tws_role_switch_on();
    TRACE(1, "%s: result = %d", __func__, result);
}

void app_ibrt_if_tws_role_switch_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ibrt_if_tws_role_switch_request();
}

void app_ibrt_if_nvrecord_get_mobile_addr_test(char* cmdParam, uint32_t cmdParam_len)
{
    bt_bdaddr_t remote_addr_test[BT_DEVICE_NUM];
    uint8_t pairedMobileNum = 0;
    app_ibrt_if_nvrecord_get_mobile_addr(remote_addr_test, &pairedMobileNum);

    TRACE(0, "%s: pairedMobileNum = %d", __func__, pairedMobileNum);

    for (uint8_t i = 0; i < pairedMobileNum; i++)
    {
        TRACE(0, "remote_addr_test[%d] =", i);
        DUMP8("%02x ", remote_addr_test[i].address, BTIF_BD_ADDR_SIZE);
    }
}

void app_ibrt_if_nvrecord_delete_all_mobile_record_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);

    bt_bdaddr_t remote_addr_test[BT_DEVICE_NUM];
    uint8_t pairedMobileNum = 0;
    app_ibrt_if_nvrecord_get_mobile_addr(remote_addr_test, &pairedMobileNum);
    TRACE(0, "mobile record num = %d, before deleting all moible record", pairedMobileNum);

    app_ibrt_if_nvrecord_delete_all_mobile_record();

    app_ibrt_if_nvrecord_get_mobile_addr(remote_addr_test, &pairedMobileNum);
    TRACE(0, "mobile record num = %d, after deleting all moible record", pairedMobileNum);
}

void app_ibrt_if_nvrecord_get_mobile_paired_dev_list_test(char* cmdParam, uint32_t cmdParam_len)
{
    nvrec_btdevicerecord p_nv_record[BT_DEVICE_NUM];
    uint8_t count = 0;
    app_ibrt_if_nvrecord_get_mobile_paired_dev_list(p_nv_record, &count);
    TRACE(0, "%s: count = %d", __func__, count);
    for (uint8_t i = 0; i < count; i++)
    {
        TRACE(0, "nvrec_btdevicerecord: p_nv_record[%d]:", i);
        TRACE(0, "bdAddr =");
        DUMP8("%02x ", p_nv_record[i].record.bdAddr.address, BTIF_BD_ADDR_SIZE);
        TRACE(0, "trusted = %d, for_bt_source = %d",
              p_nv_record[i].record.trusted,
              p_nv_record[i].record.for_bt_source);
        TRACE(0, "linkKey =");
        DUMP8("%02x ", p_nv_record[i].record.linkKey, 16);
    }
}

void app_ibrt_if_is_tws_link_connected_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result = false;
    result = app_ibrt_if_is_tws_link_connected();
    TRACE(0, "%s: is_tws_link_connected = %d", __func__, result);
}

void app_ibrt_if_is_in_freeman_mode_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint8_t result = false;
    result = app_ibrt_if_is_in_freeman_mode();
    TRACE(0, "%s: is_in_freeman_mode = %d", __func__, result);
}

void app_ibrt_if_tws_disconnect_request_test(char* cmdParam, uint32_t cmdParam_len)
{
    const char* ibrt_status_strings[] = {
        "IBRT_STATUS_SUCCESS",
        "IBRT_STATUS_PENDING",
        "IBRT_STATUS_ERROR_INVALID_PARAMETERS",
        "IBRT_STATUS_ERROR_NO_CONNECTION",
        "IBRT_STATUS_ERROR_CONNECTION_EXISTS",
        "IBRT_STATUS_IN_PROGRESS",
        "IBRT_STATUS_ERROR_DUPLICATE_REQUEST",
        "IBRT_STATUS_ERROR_INVALID_STATE",
        "IBRT_STATUS_ERROR_TIMEOUT",
        "IBRT_STATUS_ERROR_ROLE_SWITCH_FAILED",
        "IBRT_STATUS_ERROR_UNEXPECTED_VALUE",
        "IBRT_STATUS_ERROR_OP_NOT_ALLOWED",
        "IBRT_STATUS_VENDOR_START"
    };

    ibrt_status_t ibrt_status;
    uint8_t ibrt_status_index;

    ibrt_status = app_ibrt_if_tws_disconnect_request();
    if (0x80 == ibrt_status)
    {
        ibrt_status_index = sizeof(ibrt_status_strings) - 1;
    }
    else
    {
        ibrt_status_index = (uint8_t)ibrt_status;
    }

    TRACE(0, "%s: ibrt_status = %s", __func__, ibrt_status_strings[ibrt_status_index]);
}

void app_ibrt_conn_is_tws_in_pairing_state_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result;
    result  = app_ibrt_conn_is_tws_in_pairing_state();
    TRACE(0, "%s: is_tws_in_pairing_state = %d", __func__, result);
}

void app_ibrt_if_is_left_side_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result = app_ibrt_if_is_left_side();
    TRACE(0, "%s: is_left_side = %d", __func__, result);
}

void app_ibrt_if_is_right_side_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result = app_ibrt_if_is_right_side();
    TRACE(0, "%s: is_right_side = %d", __func__, result);
}

void app_ibrt_if_is_unknown_side_test(char* cmdParam, uint32_t cmdParam_len)
{
    bool result = app_ibrt_if_unknown_side();
    TRACE(0, "%s: unknown_side = %d", __func__, result);
}

void app_ibrt_if_set_side_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: set tws side to EAR_SIDE_LEFT", __func__);
    APP_TWS_SIDE_T app_tws_side = EAR_SIDE_LEFT;
    app_ibrt_if_set_side(app_tws_side);
}

void app_ibrt_if_get_ui_role_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    if (TWS_UI_MASTER == app_ibrt_if_get_ui_role())
    {
        TRACE(0, "ui role=MASTER");
    }
    else if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        TRACE(0, "ui role=SLAVE");
    }
    else
    {
        TRACE(0, "ui role=UNKOWN");
    }
}

void app_ibrt_if_uirole2str_test(char* cmdParam, uint32_t cmdParam_len)
{
    TWS_UI_ROLE_E tws_ui_role = app_ibrt_if_get_ui_role();

    TRACE(0, "%s: get_ui_role = %d", __func__, tws_ui_role);
    TRACE(0, "current role is %s", app_ibrt_if_uirole2str(tws_ui_role));
}

void app_slave_update_tws_pair_info_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: ibrt_role_e = IBRT_SLAVE, peerAddr = ", __func__);
    DUMP8("%02x", testBtAddr, sizeof(testBtAddr));

    app_ibrt_if_update_tws_pairing_info(IBRT_SLAVE, (uint8_t *)testBtAddr);
}

void app_master_update_tws_pair_info_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: ibrt_role_e = IBRT_MASTER, peerAddr = ", __func__);
    DUMP8("%02x", testSlaveBtAddr, sizeof(testSlaveBtAddr));

    app_ibrt_if_update_tws_pairing_info(IBRT_MASTER, (uint8_t *)testSlaveBtAddr);
}

void app_master_start_tws_pairing_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: ibrt_role_e = IBRT_MASTER, peerAddr = ", __func__);
    DUMP8("%02x", testSlaveBtAddr, sizeof(testSlaveBtAddr));

    app_ibrt_if_init_open_box_state_for_evb();
    app_ibrt_if_start_tws_pairing(IBRT_MASTER, (uint8_t *)testSlaveBtAddr);
}

void app_slave_start_tws_pairing_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: ibrt_role_e = IBRT_SLAVE, peerAddr = ", __func__);
    DUMP8("%02x", testBtAddr, sizeof(testBtAddr));

    app_ibrt_if_init_open_box_state_for_evb();
    app_ibrt_if_start_tws_pairing(IBRT_SLAVE, (uint8_t *)testBtAddr);
}

void app_ibrt_if_event_entry_open_box_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_CASE_OPEN", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
}

void app_ibrt_if_event_entry_fetch_out_box_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_CASE_CLOSE", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);
}

void app_ibrt_if_event_entry_put_in_box_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_DOCK", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_DOCK);
}

void app_ibrt_if_event_entry_close_box_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_UNDOCK", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_UNDOCK);
}

void app_ibrt_if_event_entry_wear_up_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_WEAR_UP", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_WEAR_UP);
}

void app_ibrt_if_event_entry_wear_down_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: APP_UI_EV_WEAR_DOWN", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_WEAR_DOWN);
}

void app_ibrt_if_pairing_mode_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
#ifndef FREE_TWS_PAIRING_ENABLED
    app_ibrt_if_init_open_box_state_for_evb();
    app_ibrt_if_event_entry(APP_UI_EV_TWS_PAIRING);
#else
    app_ibrt_if_init_open_box_state_for_evb();
    uint8_t btAddr[6];
    factory_section_original_btaddr_get(btAddr);
    btAddr[0] ^= 0x01;
    // the last bit 0 of the bt address's first byte clarifies the tws addresses
    // master if it's 1, slave if it's 0
    app_ibrt_if_start_tws_pairing(app_tws_ibrt_get_bt_ctrl_ctx()->nv_role,
        btAddr);
#endif
}

void app_ibrt_if_event_entry_freeman_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ibrt_if_event_entry(APP_UI_EV_FREE_MAN_MODE);
}

void app_ibrt_if_event_entry_dump_info_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ibrt_if_dump_ui_status();
}

void app_ibrt_customif_ui_tws_switch_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ibrt_customif_ui_tws_switch();
}

void app_ibrt_customif_ui_is_tws_switching_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    if (app_ibrt_customif_ui_is_tws_switching())
    {
        TRACE(0, "doing tws switch now");
    }
    else
    {
        TRACE(0, "not doing tws switch now");
    }
}

static void app_ibrt_global_state_changed(ibrt_global_state_change_event *state)
{
    TRACE(0, "%s", __func__);
}

void app_ibrt_if_register_client_callback_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK custom_client_callback_test_cb = {
        .ibrt_global_state_changed = app_ibrt_global_state_changed,
    };
    app_ibrt_if_register_client_callback(&custom_client_callback_test_cb);
}

void app_ibrt_if_prevent_sniff_set_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    // exit bt sniff mode
    app_tws_ibrt_exit_sniff_with_tws();
    uint8_t* mobileAddr = app_ibrt_if_get_bt_local_address();
    app_ibrt_if_exit_sniff_with_mobile(mobileAddr);
    app_ibrt_middleware_prevent_sniff_set(mobileAddr, AVRCP_STATUS_CHANING);
}

void app_ibrt_if_prevent_sniff_clear_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s", __func__);
    app_ibrt_middleware_reset_prevent_sniff_clear(app_ibrt_if_get_bt_local_address());
}

#endif
