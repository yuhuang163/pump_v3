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

#ifndef __BTH_IF_CORE_H__
#define __BTH_IF_CORE_H__
#include "plat_types.h"
#include "bth_if_cmd_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*app_bth_if_ipc_enable_t)(void *callback_func_if, uint8_t comm_type);
typedef uint32_t (*app_bth_if_ipc_disable_t)(void);
typedef uint32_t (*app_bth_if_ipc_restart_t)(void);
typedef uint32_t (*app_bth_if_ipc_ctrl_t)(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len);

typedef int32_t (*bth_if_user_rsp_result_callback_t)(void *buf, uint32_t len, void *other, uint32_t sub_cmd);

typedef struct {
    app_bth_if_ipc_enable_t           open;
    app_bth_if_ipc_disable_t          close;
    app_bth_if_ipc_restart_t          restart;
    app_bth_if_ipc_ctrl_t             ctrl;
} app_bth_if_ipc_msg_t;

typedef enum {
    APP_BTH_IF_USER_BT = 0,
    APP_BTH_IF_USER_A2DP,
    APP_BTH_IF_USER_HFP,
    APP_BTH_IF_USER_AVRCP,
    APP_BTH_IF_USER_BLE,
    APP_BTH_IF_USER_GATT,
    APP_BTH_IF_USER_A2DP_SINK,
    APP_BTH_IF_USER_QTY
} app_bth_if_user_t;

typedef enum {
    APP_BTH_IF_CORE_CMD_CLOSE = 0,
    APP_BTH_IF_CORE_CMD_OPEN,
    APP_BTH_IF_CORE_CMD_RESTART,
    APP_BTH_IF_CORE_CMD_CTRL,
    APP_BTH_IF_CORE_RESULT_RSP,
    APP_BTH_IF_CORE_CMD_QTY,
} app_bth_if_cmd_t;

typedef union {
    //send cmd to BTH core
    app_bt_search_device_cmd_t                          search_device_cmd_info;
    app_bt_create_conn_cmd_t                            create_conn_cmd_info;
    app_a2dp_src_connect_init_cmd_t                     a2dp_src_init_cmd_info;
    app_a2dp_src_stream_ctrl_cmd_t                      a2dp_src_stream_ctrl_cmd_info;
    app_a2dp_src_stream_codec_cmd_t                     a2dp_src_codec_cfg_cmd_info;
    app_hfp_caller_ind_cmd_t                            hfp_caller_ind_cmd_info;
    app_hfp_call_state_cmd_t                            hfp_call_state_cmd_info;
    app_hfp_callsetup_state_cmd_t                       hfp_callsetup_status_cmd_info;
    //BTH report event
    app_bt_init_cmp_evt_t                               init_cmp_evt_info;
    app_bt_discover_result_evt_t                        serach_result_evt_info;
    app_bt_discover_complete_evt_t                      search_cmp_evt_info;
    app_bt_acl_connect_evt_t                            acl_link_con_evt_info;
    app_bt_acl_discnt_evt_t                             acl_link_dis_evt_info;
    app_bt_acl_auth_evt_t                               acl_link_auth_evt_info;
    app_bt_acl_encrypt_evt_t                            acl_link_enc_evt_info;
    app_bt_a2dp_conn_state_evt_t                        a2dp_conn_state_evt_info;
    app_bt_a2dp_disc_state_evt_t                        a2dp_disc_state_evt_info;
    app_bt_a2dp_stream_state_evt_t                      a2dp_stream_state_evt_info;
    app_bt_a2dp_sink_volume_evt_t                       a2dp_sink_volume_evt_info;
    app_gatt_dp_connection_state_evt_t                  gatt_datapath_connection_evt_info;
    app_gatt_dp_receive_data_evt_t                      gatt_datapath_reveive_evt_info;
    app_ble_walkie_talkie_recv_stream_evt_t             ble_walkie_talkie_recv_stream_evt_info;
    app_ble_walkie_talkie_recv_data_evt_t               ble_walkie_talkie_recv_data_evt_info;
    app_ble_walkie_talkie_nabor_changed_evt_t           ble_walkie_talkie_nabor_changed_evt_info;
    app_ota_bth_to_m55_cmd_t                            ota_bth_to_m55_cmd;
    app_ota_bth_to_m55_data_t                           ota_bth_to_m55_data;
    //reserved space
    uint8_t                                             context[128];
} app_bth_if_payload_param_info_t;

typedef struct {
    uint32_t                            sub_cmd;
    app_bth_if_payload_param_info_t     param;
} __attribute__((packed)) app_bth_if_ipc_cmd_msg_context_info_t;

typedef struct {
    uint16_t                                    user;
    uint16_t                                    cmd;
    uint32_t                                    payload_len;
    app_bth_if_ipc_cmd_msg_context_info_t       payload;
} __attribute__((packed)) app_bth_if_ipc_cmd_msg_info_t;

#ifdef __cplusplus
}
#endif
#endif  //end of __BTH_IF_CORE_H__