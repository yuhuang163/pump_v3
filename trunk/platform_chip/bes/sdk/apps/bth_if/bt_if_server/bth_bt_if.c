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
#include "hal_sysfreq.h"
#include "hal_bootmode.h"
#include "bth_if_core.h"
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"
#include "besbt.h"

#ifdef OTA_SEC_IN_M55
#include "ota_multi_image_upgrade.h"
#endif

#ifdef APP_COREMARK_TEST
#include "app_rpc_api.h"
#include "hwtest.h"
#include "watchdog.h"
#endif

static bool btif_module_enable = false;
static uint8_t  bth_if_comm_type = BTH_IF_COMM_TYPE_RPC;
static bt_callbacks_t *bth_bt_if_cb = NULL;

typedef struct
{
    bool coremark_start;
    uint16_t sys_feq;
} POWER_CONSUMPTION_TEST_INFO_T;

//TODO: parse the EID data, it need be moved to bes_bth_api layer.
extern uint8_t btif_me_get_ext_inq_data(uint8_t * eir, uint8_t type,
                                                            uint8_t * outBuffer, uint8_t Length);

static const char* bt_evt2str(uint16_t event)
{
    switch (event) {
        case BT_EVENT_ACL_OPENED:
            return "BT_ACL_OPENED";
        break;
        case BT_EVENT_ACL_CLOSED:
            return "BT_ACL_CLOSED";
        break;
        case BT_EVENT_SCO_OPENED:
            return "BT_SCO_OPENED";
        break;
        case BT_EVENT_SCO_CLOSED:
            return "BT_SCO_CLOSED";
        break;
        case BT_EVENT_ACCESS_CHANGE:
            return "BT_ACCESS_CHANGE";
        break;
        case BT_EVENT_ROLE_DISCOVER:
            return "BT_ROLE_DISCOVER";
        break;
        case BT_EVENT_ROLE_CHANGE:
            return "BT_ROLE_CHANGE";
        break;
        case BT_EVENT_MODE_CHANGE:
            return "BT_MODE_CHANGE";
        break;
        case BT_EVENT_AUTHENTICATED:
            return "BT_AUTHENTICATED";
        break;
        case BT_EVENT_ENC_CHANGE:
            return "BT_ENCRYPT_CHANGE";
        break;
        case BT_EVENT_INQUIRY_RESULT:
            return "BT_INQUIRY_RESULT";
        break;
        case BT_EVENT_INQUIRY_COMPLETE:
            return "BT_INQUIRY_COMPLETE";
        break;
#ifdef OTA_SEC_IN_M55
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD:
            return "M55_TO_BTH_OTA_RSP_CMD";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_DATA:
            return "M55_TO_BTH_OTA_RSP_DATA";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_DATA:
            return "M55_TO_BTH_DATA";
        break;
#endif
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static const char* bt_cmd2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_BT_START_SEARCH_DEVICE:
            return "BT_START_SEARCH";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SERACH_DEVICE:
            return "BT_STOP_SEARCH";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_START_CONN_DEVICE:
            return "BT_START_CONN";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_CANCEL_CONN_DEVICE:
            return "BT_CANCEL_CONN";
        break;
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static int btif_bt_evt_customif_cb(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    if (memcmp(bt_evt2str(event), "UNKNOW", strlen("UNKNOW"))) {
        TRACE(3, "[%s] event=%s btif_enable=%d", __func__, bt_evt2str(event), btif_module_enable);
    }

    if (!btif_module_enable) {
        return 0;
    }
    if ((BTH_IF_COMM_TYPE_RPC != bth_if_comm_type) && (bth_bt_if_cb == NULL)) {
        TRACE(0, "Warning: callback not set!");
        return 0;
    }
    app_bth_if_payload_param_info_t report_param;
    switch (event) {
        case BT_EVENT_ACL_OPENED: {
            bt_adapter_acl_opened_param_t *con_ret = param.bt.acl_opened;
            memcpy(report_param.acl_link_con_evt_info.bt_addr.addr, bd_addr->address, 6);
            report_param.acl_link_con_evt_info.error_code = con_ret->error_code;
            report_param.acl_link_con_evt_info.device_id = con_ret->device_id;
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_acl_connect_evt_t), BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                bth_bt_if_cb->acl_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)bd_addr->address, BT_ACL_STATE_CONNECTED);
            }
        }
        break;
        case BT_EVENT_ACL_CLOSED: {
            bt_adapter_acl_closed_param_t *dis_ret = param.bt.acl_closed;
            memcpy(report_param.acl_link_dis_evt_info.bt_addr.addr, bd_addr->address, 6);
            report_param.acl_link_dis_evt_info.error_code = dis_ret->error_code;
            report_param.acl_link_dis_evt_info.device_id = dis_ret->device_id;
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_acl_discnt_evt_t), BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                bth_bt_if_cb->acl_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)bd_addr->address, BT_ACL_STATE_DISCONNECTED);
            }
        }
        break;
        case BT_EVENT_SCO_OPENED: {

        }
        break;
        case BT_EVENT_SCO_CLOSED: {

        }
        break;
        case BT_EVENT_ACCESS_CHANGE: {

        }
        break;
        case BT_EVENT_ROLE_CHANGE: {

        }
        break;
        case BT_EVENT_MODE_CHANGE: {

        }
        break;
        case BT_EVENT_AUTHENTICATED: {
            bt_adapter_authenticated_param_t *auth_param = param.bt.authenticated;
            report_param.acl_link_auth_evt_info.error_code = auth_param->error_code;
            memcpy(report_param.acl_link_auth_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_acl_auth_evt_t), BTH_IF_CORE_COMM_RESULT_BT_LINK_AUTH_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                bth_bt_if_cb->bond_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                    (bt_rawaddr_t*)bd_addr->address, BT_BOND_STATE_BONDED);
            }
        }
        break;
        case BT_EVENT_ENC_CHANGE: {
            bt_adapter_enc_change_param_t *enc_param = param.bt.enc_change;
            report_param.acl_link_enc_evt_info.error_code = enc_param->error_code;
            report_param.acl_link_enc_evt_info.enc_value = enc_param->encrypted;
            memcpy(report_param.acl_link_enc_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_acl_encrypt_evt_t), BTH_IF_CORE_COMM_RESULT_BT_LINK_ENC_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                if (report_param.acl_link_enc_evt_info.enc_value != 0) {
                    bth_bt_if_cb->enc_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                        (bt_rawaddr_t*)bd_addr->address, BT_ENC_ENABLE);
                } else {
                    bth_bt_if_cb->enc_state_changed_cb(BT_STATUS_SUCCESS_UI,
                                        (bt_rawaddr_t*)bd_addr->address, BT_ENC_DISABLE);
                }
            }
        }
        break;
        case BT_EVENT_INQUIRY_RESULT: {
            bt_adapter_inquiry_result_param_t *inq_ret = param.bt.inq_result;
            memcpy((uint8_t*)&report_param.serach_result_evt_info.bt_addr,
                    (uint8_t*)&inq_ret->remote, sizeof(bd_addr_t));
            report_param.serach_result_evt_info.rssi_value = inq_ret->rssi;
            uint8_t dev_name[240] = {0};
            uint8_t dev_name_len = 0;
            if (inq_ret->eir != NULL) {
                dev_name_len = btif_me_get_ext_inq_data(inq_ret->eir, 0x09, dev_name, sizeof(dev_name));
            }
            if (dev_name_len > 0) {
                memcpy(report_param.serach_result_evt_info.device_name, dev_name, 64);
            } else {
                memcpy(report_param.serach_result_evt_info.device_name, "UNKNOW", 7);
            }
            report_param.serach_result_evt_info.cod = inq_ret->class_of_device;
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_discover_result_evt_t), BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                if (bth_bt_if_cb) {
                    bth_bt_if_cb->device_found_cb(1, NULL);
                }
            }
        }
        break;
        case BT_EVENT_INQUIRY_COMPLETE: {
            bt_adapter_inquiry_complete_param_t *inq_cmp = param.bt.inq_complete;
            report_param.search_cmp_evt_info.error_code = inq_cmp->error_code;
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_param.context,
                                sizeof(app_bt_discover_complete_evt_t), BTH_IF_CORE_COMM_RESULT_BT_SEARCH_CMP_EVT);
            } else if (BTH_IF_COMM_TYPE_INNER == bth_if_comm_type) {
                if (bth_bt_if_cb) {
                    bth_bt_if_cb->discovery_state_changed_cb(BT_DISCOVERY_STOPPED);
                }
            }
        }
        break;
        default:
        break;
    }

    return 0;
}

uint32_t app_bth_bt_if_enable(void *callback_func_if, uint8_t comm_type)
{
    bt_add_event_callback(btif_bt_evt_customif_cb, BT_EVENT_MASK_LINK_GROUP);

    bth_bt_if_cb = (bt_callbacks_t *)callback_func_if;
    bth_if_comm_type = comm_type;
    btif_module_enable = true;

    //return the bluetooth interface ptr to client side
    app_bt_init_cmp_evt_t report_ret;
    report_ret.module_interface_addr = (uint32_t)NULL;
    const char *localname = bt_get_local_name();
    strcpy((char *) report_ret.local_device_name,localname);
    bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)&report_ret,
                                sizeof(app_bt_init_cmp_evt_t), BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT);
#ifndef BT_NOT_RECONNECT_WHEN_ESHELL_OPEN
    bes_bt_me_opening_reconnect();
#endif
    return 0;
}

uint32_t app_bth_bt_if_disable(void)
{
    btif_module_enable = false;
    bth_bt_if_cb = NULL;
    return 0;
}

uint32_t app_bth_bt_if_restart(void)
{
    return 0;
}

uint32_t app_bth_bt_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{
    if (!btif_module_enable) {
        TRACE(0, "[Warnning] BTH_IF module has not been enabled!!!");
        return -1;
    }
    static bool start_inquiry = false;
    TRACE(3, "[%s] sub_cmd=%s payload_len=%d", __func__, bt_cmd2str(sub_cmd), ptr_len);
    POSSIBLY_UNUSED uint8_t rsp_temp[4]={0};
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_BT_OPEN_INQUERY_SCAN: {
            //set accessmode = 3
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_START_SEARCH_DEVICE: {
            POSSIBLY_UNUSED app_bt_search_device_cmd_t* search_info =
                                    (app_bt_search_device_cmd_t*)ptr;
            bes_bt_app_start_inquiry();
            start_inquiry = true;
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SERACH_DEVICE: {
            bes_bt_app_stop_inquiry();
            start_inquiry = false;
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_START_CONN_DEVICE: {
            POSSIBLY_UNUSED app_bt_create_conn_cmd_t* conn_req_info =
                                (app_bt_create_conn_cmd_t*)ptr;
            if (start_inquiry) {
                bes_bt_app_stop_inquiry();
                start_inquiry = false;
            }
            //bes_bt_me_acl_connect_req((const bt_bdaddr_t *)&conn_req_info->bt_addr);
#ifdef BT_SOURCE
            bes_bt_source_reconnect_a2dp_profile((const bt_bdaddr_t *)&conn_req_info->bt_addr);
#endif
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_CANCEL_CONN_DEVICE: {
            app_bt_create_conn_cmd_t* conn_remove_info =
                                    (app_bt_create_conn_cmd_t*)ptr;
            bes_bt_me_acl_disconnect_req((const bt_bdaddr_t *)&conn_remove_info->bt_addr);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_CONN_FIXED_ADDR_DEVICE: {
            app_bt_create_conn_cmd_t* conn_remove_info =
                                   (app_bt_create_conn_cmd_t*)ptr;
            bes_bt_me_acl_connect_req((bt_bdaddr_t *)conn_remove_info);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_DISCONN_FIXED_ADDR_DEVICE: {
            app_bt_cancel_conn_cmd_t* conn_remove_info =
                                   (app_bt_cancel_conn_cmd_t*)ptr;
            bes_bt_me_acl_disconnect_req((bt_bdaddr_t *)conn_remove_info);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_START_SNIFF: {
            app_bt_sniff_cmd_t* sniff_info =
                                    (app_bt_sniff_cmd_t*)ptr;
            btif_sniff_info_t bt_sniff_info;
            bt_sniff_info.maxInterval = sniff_info->maxInterval;
            bt_sniff_info.minInterval = sniff_info->minInterval;
            bt_sniff_info.attempt = sniff_info->attempt;
            bt_sniff_info.timeout = sniff_info->timeout;
            extern uint16_t btif_me_get_connhandle_by_addr(const bt_bdaddr_t *addr);
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            DUMP8("%02x ", addr_t, 6);
            bes_bt_me_acl_start_sniff(btif_me_get_connhandle_by_addr((const bt_bdaddr_t *)addr_t),
                                                            &bt_sniff_info);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SNIFF: {
            extern uint16_t btif_me_get_connhandle_by_addr(const bt_bdaddr_t *addr);
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            bes_bt_me_acl_stop_sniff(btif_me_get_connhandle_by_addr((const bt_bdaddr_t *)addr_t));
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BT_SCAN_ENABLE: {
            uint8_t access_mode = *ptr;
            bes_bt_me_write_access_mode(access_mode, 1);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PLAY: {
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            bes_bt_avrcp_send_play_cmd(addr_t);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PAUSE: {
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            bes_bt_avrcp_send_pause_cmd(addr_t);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_NEXT_TRACK: {
            bes_bt_a2dp_key_handler(AVRCP_KEY_BACKWARD);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PREVIOUS_TRACK: {
            bes_bt_a2dp_key_handler(AVRCP_KEY_FORWARD);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_VOLUME: {
            uint8_t local_volume = *ptr;
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            int device_id = bes_bt_me_get_device_id_from_address(addr_t);
            bes_bt_a2dp_set_local_vol(device_id,local_volume);
            local_volume = bes_bt_a2dp_get_local_vol(device_id);

            app_bt_a2dp_sink_volume_evt_t volume_evt;
            volume_evt.volume = local_volume;
            bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t *)&volume_evt,
                sizeof(app_bt_a2dp_sink_volume_evt_t), BTH_IF_CORE_COMM_RESULT_A2DP_VOL_CHANGE_EVT);

        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_MUTE: {
            bt_bdaddr_t *addr_t = &(bt_adapter_get_device(BT_DEVICE_ID_1)->remote);
            int device_id = bes_bt_me_get_device_id_from_address(addr_t);
            bes_bt_a2dp_set_local_vol(device_id, 0);
        }
        break;

        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_NAME: {
            char name_ptr[MAX_BTH_STRING] = "0";
            memcpy((void *)name_ptr, ptr, ptr_len);
            strcpy(&name_ptr[ptr_len],"\n");
            bes_bt_me_set_bt_local_name((const unsigned char *)name_ptr, strlen(name_ptr));
        }
        break;

        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_ADDR: {
            const uint8_t *local_addr= ptr;
            bes_bt_me_set_bt_address(local_addr);
        }
        break;
#ifdef APP_COREMARK_TEST
        case BTH_IF_CORE_COMM_SUBCMD_COREMARK_START: {
            POWER_CONSUMPTION_TEST_INFO_T *power_c_test = (POWER_CONSUMPTION_TEST_INFO_T *)ptr;
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, power_c_test->sys_feq -1);
            if (power_c_test->coremark_start) {
                app_rpc_ctx_deinit(APP_RPC_CORE_BTH_DSP);
                app_wdt_close();
                while (1) {
                    coremark_test();
                }
            } else {
                TRACE(0, "------wfi test started------");
                TRACE(0, "CPU frequency changed to: %uM", (hal_sys_timer_calc_cpu_freq(5, 0) / 1000000));
                while (1) {
                    __WFI();
                }
            }
        }
        break;
#endif
#ifdef OTA_SEC_IN_M55
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_DATA: {
            ota_read_m55_flash_done((uint8_t*)ptr , ptr_len);
            bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)(&rsp_temp),
                                sizeof(rsp_temp), BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DONE);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD: {
            ota_write_m55_flash_done_handle();
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_DATA: {
            bth_if_send_result_to_app(APP_BTH_IF_USER_BT, (uint8_t*)(&rsp_temp),
                                sizeof(rsp_temp), BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DONE);
        }
        break;
#endif
        default:
        break;
    }

    return 0;
}

static const app_bth_if_ipc_msg_t bth_btif_ipc_handle_table = {
    .open           = app_bth_bt_if_enable,
    .close          = app_bth_bt_if_disable,
    .restart        = app_bth_bt_if_restart,
    .ctrl           = app_bth_bt_if_ctrl,
};

void app_bth_bt_if_init(void)
{
    bth_if_thread_server_register(APP_BTH_IF_USER_BT, &bth_btif_ipc_handle_table);
}

