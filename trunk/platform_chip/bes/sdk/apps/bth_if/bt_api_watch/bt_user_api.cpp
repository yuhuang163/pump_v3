/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#include <string>
#include <vector>
#include "bt_user_api.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <algorithm>
#include "plat_types.h"
#include "hal_trace.h"
#include "eshell.h"
#include "bth_if_evt_def.h"
#include "bth_bt_if_client.h"
#include "bth_if_core.h"
#include "bth_if_cmd_def.h"
#include "bt_user_api_callback.h"

namespace Watch {
namespace BlueTooth {

void BtDeviceFoundReportCb(int num_properties, bt_property_t *properties)
{
    if (NULL == properties || num_properties <= 0) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    BtDiscoverInfo search_info;
    if (properties->type == BT_PROPERTY_INQUIRY_EIR_RESULT) {
        search_info.inq_result = *properties;
        BtUserApiCallback::GetIstance().StorageApiResult(&search_info,BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT);
    }
}

void BtAclStateReportCb(bt_status_ui_t status, bt_rawaddr_t *remote_bd_addr, bt_acl_state_ui_t state)
{
    if (NULL == remote_bd_addr) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    BtAclConnectInfo con_info;
    BtAclDisconnectInfo dis_info;
    if (state == BT_ACL_STATE_CONNECTED) {
        con_info.status = status;
        memcpy(con_info.bt_addr.addr, remote_bd_addr->addr, 6);
        con_info.state = state;
        BtUserApiCallback::GetIstance().StorageApiResult(&con_info,BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT);
    } else if (state == BT_ACL_STATE_CONNECTED) {
        dis_info.status = status;
        memcpy(dis_info.bt_addr.addr, remote_bd_addr->addr, 6);
        dis_info.state = state;
        BtUserApiCallback::GetIstance().StorageApiResult(&dis_info,BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT);
    }
}

void BtHfpCallerInfoReportCb(const bt_rawaddr_t *bd_addr, const uint8_t* number, int number_len)
{
    if (NULL == bd_addr || NULL == number) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    BtCallerInfo call_info;
    call_info.number_len =  number_len;
    memcpy(call_info.remote.addr, bd_addr->addr, 6);
    memcpy(call_info.number, number, number_len);
    BtUserApiCallback::GetIstance().StorageApiResult(&call_info,BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND);
}

void BtHfpCallStateReportCb(const bt_rawaddr_t *bd_addr, bthf_client_call_state_t state)
{
    if (NULL == bd_addr) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    BtCallStateInfo call_state_info;
    call_state_info.call_state = state;
    memcpy(call_state_info.remote.addr, bd_addr->addr,6);
    BtUserApiCallback::GetIstance().StorageApiResult(&call_state_info,BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE);
}

void BtHfpCallSetupStateReportCb(const bt_rawaddr_t *bd_addr, bthf_client_call_setup_state_t state)
{
    if (NULL == bd_addr) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    BtCallSetupStateInfo call_setup_state_info;
    call_setup_state_info.callsetup_state = state;
    memcpy(call_setup_state_info.remote.addr, bd_addr->addr,6);
    BtUserApiCallback::GetIstance().StorageApiResult(&call_setup_state_info,BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE);
}

static const bt_callbacks_t bt_state_report_if_cbs = {
    .size                           = sizeof(bt_callbacks_t),
    .adapter_state_changed_cb       = NULL,
    .adapter_properties_cb          = NULL,
    .remote_device_properties_cb    = NULL,
    .device_found_cb                = BtDeviceFoundReportCb,
    .discovery_state_changed_cb     = NULL,
    .pin_request_cb                 = NULL,
    .ssp_request_cb                 = NULL,
    .bond_state_changed_cb          = NULL,
    .acl_state_changed_cb           = BtAclStateReportCb,
};

static const btav_source_callbacks_t a2dp_source_report_if_cbs = {
    .size                           = sizeof(btav_source_callbacks_t),
    .connection_state_cb            = NULL,
    .audio_state_cb                 = NULL,
};

static const bthf_client_callbacks_t hfp_client_report_if_cbs = {
    .size                           = sizeof(bthf_client_callbacks_t),
    .connection_state_cb            = NULL,
    .audio_state_cb                 = NULL,
    .caller_info_cb                 = BtHfpCallerInfoReportCb,
    .call_state_cb                  = BtHfpCallStateReportCb,
    .call_setup_state_cb            = BtHfpCallSetupStateReportCb,
};

void BtUserApi::Enable(void)
{
    app_bth_bt_if_client_open((void*)&bt_state_report_if_cbs, BTH_IF_COMM_TYPE_RPC);
    app_bth_a2dp_if_client_open((void*)&a2dp_source_report_if_cbs, BTH_IF_COMM_TYPE_RPC);
    app_bth_hfp_cli_if_client_open((void*)&hfp_client_report_if_cbs, BTH_IF_COMM_TYPE_RPC);
}

void BtUserApi::BtConnectDevice(const void *addr)
{
    if (NULL == addr){
        TRACE(1, "[%s] Fail to connect device due to the bad parameter ", __func__);
        return;
    }
    bd_addr_t conn_info;
    memcpy(conn_info.addr, addr, 6);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_CONN_DEVICE,
                                            (uint8_t*)&conn_info, sizeof(bd_addr_t));
}

void BtUserApi::BtDisconnectDevice(const void *addr)
{
    if (NULL == addr){
        TRACE(1, "[%s] Fail to connect device due to the bad parameter ", __func__);
        return;
    }
    bd_addr_t conn_info;
    memcpy(conn_info.addr, addr, 6);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_CANCEL_CONN_DEVICE,
                                            (uint8_t*)&conn_info, sizeof(app_bt_create_conn_cmd_t));
}

void BtUserApi::BtSearchDevice(bool start, uint32_t search_last_time)
{
    if (start) {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_SEARCH_DEVICE,
                                                    (uint8_t*)&search_last_time, 4);
    } else {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SERACH_DEVICE, NULL, 0);
        return;
    }
}

void BtUserApi::BtA2dpSourceConnectInit(uint32_t buf_ptr, uint32_t buf_size)
{
    if (buf_ptr <=0 || buf_size <= 0) {
         TRACE(1, "[%s] Fail to init due to the bad parameter ", __func__);
        return;
    }
    app_a2dp_src_connect_init_cmd_t connect_init;
    connect_init.buff_ptr = buf_ptr;
    connect_init.buff_size = buf_size;
    app_bth_a2dp_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CONN_INIT,
                                                (uint8_t*)&connect_init,sizeof(app_a2dp_src_connect_init_cmd_t));
}

void BtUserApi::BtA2dpSourceStreamCtrl(const bd_addr_t addr, uint8_t key)
{
    if ((key != A2DP_STREAM_CTRL_PLAY) && (key != A2DP_STREAM_CTRL_PAUSE) && (key != A2DP_STREAM_CTRL_STOP)) {
         TRACE(1, "[%s] Fail to ctrl due to the bad parameter ", __func__);
        return;
    }
    app_a2dp_src_stream_ctrl_cmd_t stream_ctrl;
    stream_ctrl.bt_addr = addr;
    stream_ctrl.ctrl_key = key;
    app_bth_a2dp_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_STREAM_CTRL,
                                                (uint8_t*)&stream_ctrl,sizeof(app_a2dp_src_stream_ctrl_cmd_t));
}

void BtUserApi::BtHfpSetCallAciton(uint8_t actn, bd_addr_t current_call_remote_addr)
{
    if ((actn != BT_HFP_ANSWER_CALL) && (actn != BT_HFP_HANGUP_CALL) && (actn != BT_HFP_REDIAL_CALL)) {
         TRACE(1, "[%s] Fail to ctrl due to the bad parameter ", __func__);
        return;
    }
    app_hfp_call_action_cmd_t call_action;
    call_action.action = actn;
    memcpy(call_action.remote.addr, current_call_remote_addr.addr, 6);
    app_bth_hfp_cli_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_HFP_SET_CALL_ACTION,
                                                (uint8_t*)&call_action, sizeof(app_hfp_call_action_cmd_t));
}

}  // namespace BlueTooth
}  // namespace Watch


