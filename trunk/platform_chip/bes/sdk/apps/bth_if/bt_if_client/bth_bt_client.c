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

#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_client.h"
#include "bth_bt_if_client.h"
#include "bth_if_evt_def.h"
#include "bth_ota_rx_handler.h"

static const bt_callbacks_t* ui_bt_evt_if_cbs = NULL;

//TODO: it can redefine the actual bluetooth interface data type, eg: bes_bt_me_t.
static uint32_t bt_if_ptr = 0;

const char* bt_ret2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT:
            return "BT_INIT_DONE_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT:
            return "BT_SEARCH_RESULT_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_CMP_EVT:
            return "BT_SEARCH_COMPLETE_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT:
            return "BT_ACL_CONN_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT:
            return "BT_ACL_DISC_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_SCO_CONNECT_EVT:
            return "BT_SCO_CONN_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_SCO_DISCONN_EVT:
            return "BT_SCO_DISC_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_LINK_AUTH_EVT:
            return "BT_AUTH_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BT_LINK_ENC_EVT:
            return "BT_ENC_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD:
            return "BTH_TO_M55_CMD";
            break;
        case BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DATA:
            return "BTH_TO_M55_DATA";
            break;
        default:
            break;
    }
    return "UNKNOW";
}

WEAK int32_t _btif_client_bt_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    app_bth_if_payload_param_info_t *result_info = (app_bth_if_payload_param_info_t*)buf;
    TRACE(2, "[%s] sub_cmd=%s", __func__, bt_ret2str(sub_cmd));

    if (!result_info) {
        TRACE(1, "[%s] ERROR!!!", __func__);
        return -1;
    }
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT: {
            bt_if_ptr = result_info->init_cmp_evt_info.module_interface_addr;
            TRACE(1, "bluetooth_interface_ptr:0x%x", bt_if_ptr);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT: {
            app_bt_discover_result_evt_t* search_ptr = &(result_info->serach_result_evt_info);
            TRACE(8, "dev_add=%02x:%02x:%02x:%02x:%02x:%02x dev_name=%s rssi=%d",
                        search_ptr->bt_addr.addr[0], search_ptr->bt_addr.addr[1],
                        search_ptr->bt_addr.addr[2], search_ptr->bt_addr.addr[3],
                        search_ptr->bt_addr.addr[4], search_ptr->bt_addr.addr[5],
                        search_ptr->device_name, search_ptr->rssi_value);

            if (ui_bt_evt_if_cbs && ui_bt_evt_if_cbs->device_found_cb) {
                bt_property_t inq_result_prop;
                inq_result_prop.type = BT_PROPERTY_INQUIRY_EIR_RESULT;
                inq_result_prop.len = sizeof(app_bt_discover_result_evt_t);
                inq_result_prop.val = search_ptr;
                ui_bt_evt_if_cbs->device_found_cb(1, &inq_result_prop);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_CMP_EVT: {
            app_bt_discover_complete_evt_t* search_cmp = &(result_info->search_cmp_evt_info);
            TRACE(1, "bt inquiry complete error_code=%d", search_cmp->error_code);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT: {
            app_bt_acl_connect_evt_t* acl_con_evt = &(result_info->acl_link_con_evt_info);
            if (ui_bt_evt_if_cbs && ui_bt_evt_if_cbs->acl_state_changed_cb) {
                ui_bt_evt_if_cbs->acl_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)&acl_con_evt->bt_addr, BT_ACL_STATE_CONNECTED);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT: {
            app_bt_acl_discnt_evt_t* acl_dis_evt = &(result_info->acl_link_dis_evt_info);
            if (ui_bt_evt_if_cbs && ui_bt_evt_if_cbs->acl_state_changed_cb) {
                ui_bt_evt_if_cbs->acl_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)&acl_dis_evt->bt_addr, BT_ACL_STATE_DISCONNECTED);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_LINK_AUTH_EVT: {
            app_bt_acl_auth_evt_t* acl_auth_evt = &(result_info->acl_link_auth_evt_info);
            if (ui_bt_evt_if_cbs && ui_bt_evt_if_cbs->bond_state_changed_cb) {
                ui_bt_evt_if_cbs->bond_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)&acl_auth_evt->bt_addr, BT_BOND_STATE_BONDED);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_LINK_ENC_EVT: {
            app_bt_acl_encrypt_evt_t* acl_enc_evt = &(result_info->acl_link_enc_evt_info);
            if (ui_bt_evt_if_cbs && ui_bt_evt_if_cbs->bond_state_changed_cb) {
                if (acl_enc_evt->enc_value != 0) {
                    ui_bt_evt_if_cbs->enc_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                        (bt_rawaddr_t*)&acl_enc_evt->bt_addr, BT_ENC_ENABLE);
                } else {
                    ui_bt_evt_if_cbs->enc_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                        (bt_rawaddr_t*)&acl_enc_evt->bt_addr, BT_ENC_DISABLE);
                }
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD: {
            //handle cmd from bth in maerge OTA
            app_ota_bth_to_m55_cmd_t* bth_to_m55_cmd = &(result_info->ota_bth_to_m55_cmd);
            app_bth_ota_op_code_send((uint8_t*)bth_to_m55_cmd , len);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DATA: {
            //handle data from bth in maerge OTA
            app_ota_bth_to_m55_data_t* bth_to_m55_data = &(result_info->ota_bth_to_m55_data);
            app_bth_ota_data_send((uint8_t*)bth_to_m55_data, len);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DONE:{
        }
        break;
        default:
        break;
    }

    return 0;
}

int32_t app_bth_bt_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_BT, _btif_client_bt_result_rsp_callback);
    return 0;
}

WEAK int32_t app_bth_bt_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_bt_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_BT, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_bt_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_BT);
    ui_bt_evt_if_cbs = NULL;
    return 0;
}

int32_t app_bth_bt_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_BT);
    return 0;
}

int32_t app_bth_bt_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_BT, ctrl_cmd, payload, payload_len);
    return 0;
}
