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
#include "cmsis_os.h"
#include "stdlib.h"
#include "string.h"
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "bth_if_evt_def.h"
#include "bth_if_thread_client.h"
#include "bth_if_cmd_def.h"
#include "bth_bt_if_client.h"
#include "bth_bt_custom_evt.h"
#include "bth_ota_rx_handler.h"

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/************************private variable defination************************/
bt_dev_addr_t current_bt_connect_addr = {0};
bt_bdname_t local_bt_device_name;
static const custom_bt_callbacks_t* custom_ui_bt_evt_if_cbs = NULL;
static uint32_t bt_if_ptr = 0;

/**********************private function declaration*************************/

int32_t _btif_client_bt_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    app_bth_if_payload_param_info_t *result_info = (app_bth_if_payload_param_info_t*)buf;
    TRACE(2, "custom [%s] sub_cmd=%s", __func__, bt_ret2str(sub_cmd));

    if (!result_info) {
        TRACE(1, "[%s] ERROR!!!", __func__);
        return -1;
    }
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT: {
            bt_if_ptr = result_info->init_cmp_evt_info.module_interface_addr;
            TRACE(1, "bluetooth_interface_ptr:0x%x", bt_if_ptr);
            strcpy((char *) &local_bt_device_name, (char *)result_info->init_cmp_evt_info.local_device_name);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT: {
            app_bt_discover_result_evt_t* search_ptr = &(result_info->serach_result_evt_info);
            TRACE(8, "dev_add=%02x:%02x:%02x:%02x:%02x:%02x dev_name=%s rssi=%d",
                        search_ptr->bt_addr.addr[0], search_ptr->bt_addr.addr[1],
                        search_ptr->bt_addr.addr[2], search_ptr->bt_addr.addr[3],
                        search_ptr->bt_addr.addr[4], search_ptr->bt_addr.addr[5],
                        search_ptr->device_name, search_ptr->rssi_value);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_CMP_EVT: {
            app_bt_discover_complete_evt_t* search_cmp = &(result_info->search_cmp_evt_info);
            TRACE(1, "bt inquiry complete error_code=%d", search_cmp->error_code);
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT: {
            app_bt_acl_connect_evt_t* acl_con_evt = &(result_info->acl_link_con_evt_info);

            BTH_PairedDevice bth_paired_device;
            bth_paired_device.index = acl_con_evt->device_id;
            bth_paired_device.state = bth_device_state_connected;
            memcpy((void *)&current_bt_connect_addr, (bt_rawaddr_t*)&acl_con_evt->bt_addr, 6);
            memcpy((void *)bth_paired_device.address, (bt_rawaddr_t*)&acl_con_evt->bt_addr, 6);
            if (custom_ui_bt_evt_if_cbs && custom_ui_bt_evt_if_cbs->deviceConnectedCB) {
                custom_ui_bt_evt_if_cbs->deviceConnectedCB(&bth_paired_device);
            }

        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT: {
            app_bt_acl_discnt_evt_t* acl_dis_evt = &(result_info->acl_link_dis_evt_info);

            BTH_PairedDevice bth_paired_device;
            bth_paired_device.index = acl_dis_evt->device_id;
            bth_paired_device.state = bth_device_state_disconnected;
            memset((void *)&current_bt_connect_addr, 0, 6);
            memcpy((void *)bth_paired_device.address, (void*)&acl_dis_evt->bt_addr, 6);
            if (custom_ui_bt_evt_if_cbs && custom_ui_bt_evt_if_cbs->deviceConnectedCB) {
                custom_ui_bt_evt_if_cbs->deviceConnectedCB(&bth_paired_device);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_A2DP_VOL_CHANGE_EVT: {
            app_bt_a2dp_sink_volume_evt_t* volume_evt = &(result_info->a2dp_sink_volume_evt_info);
            if (custom_ui_bt_evt_if_cbs && custom_ui_bt_evt_if_cbs->volumeChangedCB) {
                custom_ui_bt_evt_if_cbs->volumeChangedCB(volume_evt->volume);
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

int32_t app_bth_bt_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] custom callback_if_ptr=0x%p", __func__, ui_callback);
    custom_ui_bt_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_BT, (uint32_t)ui_callback, comm_type);
    return 0;
}

