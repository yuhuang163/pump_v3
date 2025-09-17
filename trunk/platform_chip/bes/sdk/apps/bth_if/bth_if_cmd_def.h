
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

/****************************************************************************
 *
 * File Description: this file defines the UI calls client interface which needs
 * data struct.
 *
 ****************************************************************************/

#ifndef __BTH_IF_CMD_DEFINITION_H__
#define __BTH_IF_CMD_DEFINITION_H__
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t addr[6];
} __attribute__((packed)) bt_dev_addr_t;

typedef enum {
    BTH_IF_COMM_TYPE_RPC = 0,
    BTH_IF_COMM_TYPE_INNER,
    BTH_IF_COMM_TYPE_IPC,
    BTH_IF_COMM_TYPE_QTY,
} app_bth_if_comm_type_t;

// BTH interface ctrl cmd definition
typedef enum {
    BTH_IF_CORE_COMM_SUBCMD_INVALID = 0,
    BTH_IF_CORE_COMM_SUBCMD_INIT,
    BTH_IF_CORE_COMM_SUBCMD_BT_OPEN_INQUERY_SCAN,
    BTH_IF_CORE_COMM_SUBCMD_BT_START_SEARCH_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SERACH_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_START_CONN_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_CANCEL_CONN_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_START_SNIFF,
    BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SNIFF,
    BTH_IF_CORE_COMM_SUBCMD_BT_SCAN_ENABLE,
    BTH_IF_CORE_COMM_SUBCMD_BT_CONN_FIXED_ADDR_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_DISCONN_FIXED_ADDR_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_BT_DISCONN_CURRENT_ADDR_DEVICE,
    BTH_IF_CORE_COMM_SUBCMD_COREMARK_START,
    BTH_IF_CORE_COMM_SUBCMD_BLE_ADV_ENABLE,
    BTH_IF_CORE_COMM_SUBCMD_BLE_CON_UPDATE,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CONN_INIT,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_STREAM_CTRL,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CODEC_CFG,
    BTH_IF_CORE_COMM_SUBCMD_HFP_SET_CALL_ACTION,    //10
    BTH_IF_CORE_COMM_SUBCMD_GATT_DP_SEND_NTF_REQ,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_INIT,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_DEINIT,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_OPEN_SCAN,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_STOP_SCAN,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_START_CAPTURE,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_STOP_CAPTURE,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_AUDIO_INIT_DONE,
    BTH_IF_CORE_COMM_SUBCMD_HFP_START_SCO_STREAM,
    BTH_IF_CORE_COMM_SUBCMD_HFP_STOP_SCO_STREAM,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_START,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_VOL_SET,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_STOP,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PLAY,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PAUSE,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_NEXT_TRACK,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PREVIOUS_TRACK,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_VOLUME,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_MUTE,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_NAME,
    BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_ADDR,
    //M55 response to BTH in multi image OTA
    BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_CMD,
    BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_OTA_RSP_DATA,
    BTH_IF_CORE_COMM_SUBCMD_M55_TO_BTH_DATA,
    BTH_IF_CORE_COMM_SUBCMD_QTY,
} app_bth_if_comm_subcmd_t;

// BTH state result report cmd definition
typedef enum {
    BTH_IF_CORE_COMM_RESULT_INVALID = 0,
    // module init complete state report event.
    BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT,

    /*************************************
     ******* bt basic report evt *********
     *************************************/
    // bt inquiry device result report evt.
    BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT,
    // bt inquiry procedure complete report evt.
    BTH_IF_CORE_COMM_RESULT_BT_SEARCH_CMP_EVT,
    // bt acl connect status evt.
    BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT,
    // bt acl disconnect status evt.
    BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT,
    // bt acl connect status evt.
    BTH_IF_CORE_COMM_RESULT_BT_SCO_CONNECT_EVT,
    // bt acl disconnect status evt.
    BTH_IF_CORE_COMM_RESULT_BT_SCO_DISCONN_EVT,
    // bt acl authenticated status evt.
    BTH_IF_CORE_COMM_RESULT_BT_LINK_AUTH_EVT,
    // bt acl encrypted status evt.
    BTH_IF_CORE_COMM_RESULT_BT_LINK_ENC_EVT,

    /**************************************
     ****** a2dp profile report evt *******
     **************************************/
    BTH_IF_CORE_COMM_RESULT_A2DP_CONNECT_EVT,
    BTH_IF_CORE_COMM_RESULT_A2DP_DISCONN_EVT,
    BTH_IF_CORE_COMM_RESULT_A2DP_STREAM_STATUS_CHANGE_EVT,
    BTH_IF_CORE_COMM_RESULT_A2DP_VOL_CHANGE_EVT,
    /**************************************
     ****** hfp profile report evt ********
     **************************************/
    BTH_IF_CORE_COMM_RESULT_HFP_CONNECT_EVT,
    BTH_IF_CORE_COMM_RESULT_HFP_DISCONN_EVT,
    BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND,
    BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE,
    BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE,
    BTH_IF_CORE_COMM_RESULT_HFP_START_SCO_STREAM,
    BTH_IF_CORE_COMM_RESULT_HFP_STOP_SCO_STREAM,

    /**************************************
     ****** ble gatt report evt ********
     **************************************/
    BTH_IF_CORE_COMM_RESULT_GATT_DP_CON_STATE_EVT,
    BTH_IF_CORE_COMM_RESULT_GATT_DP_DATA_RCV_EVT,
    /**************************************
     ****** ble walkie talkie  evt ********
     **************************************/
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_STREAM_EVT,   //20
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_DATA_EVT,
    BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_NABOR_CHANGED_EVT,


    //BTH to M55 in multi image OTA
    BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_CMD,
    BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DATA,
    BTH_IF_CORE_COMM_RESULT_BTH_TO_M55_DONE,

    BTH_IF_CORE_COMM_RESULT_QTY,
}app_bth_if_comm_result_evt_t;

typedef enum {
    A2DP_IF_STREAM_CODEC_TYPE_SBC = 0x00,
    A2DP_IF_STREAM_CODEC_TYPE_AAC,
    A2DP_IF_STREAM_CODEC_TYPE_LC3,
    A2DP_IF_STREAM_CODEC_TYPE_PCM,
    A2DP_IF_STREAM_CODEC_TYPE_SCALABLE,
    A2DP_IF_STREAM_CODEC_TYPE_LDAC,
    A2DP_IF_STREAM_CODEC_TYPE_LHAC,
    A2DP_IF_STREAM_CODEC_TYPE_QTY,
} a2dp_if_stream_codec_type_e;

typedef struct {
    uint8_t     codec_type;
    uint8_t     sample_bits;
    uint8_t     channel_num;
    uint32_t    sample_rate;
} __attribute__((packed)) codec_info_t;
/***************************************************************************
*
************* BTH send cmd struct definition *******************************
*
****************************************************************************/
typedef struct {
    uint32_t    search_last_time;       //unit of second
} __attribute__((packed)) app_bt_search_device_cmd_t;

typedef struct {
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_create_conn_cmd_t;

typedef struct {
    uint16_t maxInterval;
    uint16_t minInterval;
    uint16_t attempt;
    uint16_t timeout;
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_sniff_cmd_t;

typedef struct {
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_cancel_conn_cmd_t;

enum {
    A2DP_STREAM_CTRL_NULL = 0,
    A2DP_STREAM_CTRL_STOP,
    A2DP_STREAM_CTRL_PLAY,
    A2DP_STREAM_CTRL_PAUSE,
    A2DP_STREAM_CTRL_FORWARD,
    A2DP_STREAM_CTRL_BACKWARD,
    A2DP_STREAM_CTRL_VOLUME_UP,
    A2DP_STREAM_CTRL_VOLUME_DOWN,
};

typedef struct {
    uint32_t    buff_ptr;
    uint32_t    buff_size;
} __attribute__((packed)) app_a2dp_src_connect_init_cmd_t;

typedef struct {
    bt_dev_addr_t   bt_addr;            //multipoint support
    uint8_t         ctrl_key;
} __attribute__((packed)) app_a2dp_src_stream_ctrl_cmd_t;

typedef struct {
    bt_dev_addr_t       bt_addr;
    uint32_t            latency_ms;
    codec_info_t        codec_info;
} __attribute__((packed)) app_a2dp_src_stream_codec_cmd_t;

typedef struct {
    int             number_len;
    char            number[20];
    bt_dev_addr_t   remote;
} __attribute__((packed)) app_hfp_caller_ind_cmd_t;

typedef struct {
    uint8_t         action;
    bt_dev_addr_t   remote;
} __attribute__((packed)) app_hfp_call_action_cmd_t;

typedef struct {
    uint8_t         call_state;
    bt_dev_addr_t   remote;
} __attribute__((packed)) app_hfp_call_state_cmd_t;

typedef struct {
    uint8_t         callsetup_state;
    bt_dev_addr_t   remote;
} __attribute__((packed)) app_hfp_callsetup_state_cmd_t;


typedef struct {
    void *kfifo;
}__attribute__((packed)) app_ble_walkie_talkie_start_capture_cmd_t;

/***************************************************************************
*
************* BTH report result struct definition **************************
*
****************************************************************************/
typedef struct {
    uint32_t    module_interface_addr;
    uint8_t     local_device_name[64];
} __attribute__((packed)) app_bt_init_cmp_evt_t;

typedef struct {
    bt_dev_addr_t   bt_addr;
    uint32_t        cod;            //class of device, the lower 24bits are valid.
    int8_t          rssi_value;
    uint8_t         device_name[64];
} __attribute__((packed)) app_bt_discover_result_evt_t;

typedef struct {
    uint8_t     error_code;
} __attribute__((packed)) app_bt_discover_complete_evt_t;

typedef struct {
    uint8_t     error_code;
    bt_dev_addr_t   bt_addr;
    uint8_t     device_id;
} __attribute__((packed)) app_bt_acl_connect_evt_t;

typedef struct {
    uint8_t     error_code;
    bt_dev_addr_t   bt_addr;
    uint8_t     device_id;
} __attribute__((packed)) app_bt_acl_discnt_evt_t;

typedef struct {
    uint8_t         error_code;
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_acl_auth_evt_t;

typedef struct {
    uint8_t         error_code;
    bt_dev_addr_t   bt_addr;
    uint8_t         enc_value;
} __attribute__((packed)) app_bt_acl_encrypt_evt_t;

typedef struct {
    uint8_t     onoff;
    uint8_t     codec_type;
} __attribute__((packed)) app_bt_sco_stream_evt_t;

typedef struct {
    uint8_t     error_code;
    bt_dev_addr_t   bt_addr;
    uint8_t     codec_type;
} __attribute__((packed)) app_bt_a2dp_conn_state_evt_t;

typedef struct {
    uint8_t     error_code;
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_a2dp_disc_state_evt_t;

typedef struct {
    uint8_t         onoff;
    bt_dev_addr_t   bt_addr;
    uint8_t         codec_type;
} __attribute__((packed)) app_bt_a2dp_stream_state_evt_t;

typedef struct {
    uint8_t     volume;
    bt_dev_addr_t   bt_addr;
} __attribute__((packed)) app_bt_a2dp_sink_volume_evt_t;

typedef struct {
    uint8_t     connect_state;  //1 connexted, 0 closed
    uint8_t     conidx;
} __attribute__((packed)) app_gatt_dp_connection_state_evt_t;

typedef struct {
    uint16_t        data_len;
    uint8_t         data[116];
} __attribute__((packed)) app_gatt_dp_receive_data_evt_t;

typedef struct {
    uint8_t state;  //0 --closed, 1--opened
    uint8_t device_id;
    void *kfifo;
}__attribute__((packed)) app_ble_walkie_talkie_recv_stream_evt_t;

typedef struct {
    uint8_t device_id;
}__attribute__((packed)) app_ble_walkie_talkie_recv_data_evt_t;

typedef struct {
    uint8_t nebor_count;
}__attribute__((packed)) app_ble_walkie_talkie_nabor_changed_evt_t;

typedef union {
    struct
    {
        uint8_t onoff;
        uint8_t codec_type;
        uint32_t sample_rate;
        uint32_t kfifo_addr;
    }onoff_param;
    struct
    {
        uint8_t volume_level;
    }vol_set;
} __attribute__((packed)) app_bt_a2dp_sink_param_t;

typedef struct {
    uint32_t op_code;
    uint32_t address;
    uint32_t ilength;
}__attribute__((packed)) app_ota_bth_to_m55_cmd_t;

typedef struct {
    uint8_t     data[120];
} __attribute__((packed)) app_ota_bth_to_m55_data_t;
#ifdef __cplusplus
}
#endif
#endif // end of __BTH_IF_CMD_DEFINITION_H__
