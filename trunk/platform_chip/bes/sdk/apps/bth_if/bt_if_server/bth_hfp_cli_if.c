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
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"
#include "cmsis_os.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
//#include "audio_player_adapter.h"

static bthf_client_callbacks_t *bth_hf_client_if_cb = NULL;
static uint8_t  bth_if_comm_type = BTH_IF_COMM_TYPE_RPC;

#define SCO_STREAM_OFF_CODE  (0x00)
#define SCO_STREAM_ON_CODE   (0x01)

extern uint16_t app_bt_get_handle_by_device_id(uint8_t device_id);

static void app_bth_hfp_stream_btpcm_start(int dev_id)
{
    uint32_t curr_ticks = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    uint16_t conhdl = app_bt_get_handle_by_device_id(dev_id);

    uint32_t lock;

    lock = int_lock();
    curr_ticks = bt_syn_get_curr_ticks(conhdl);
    tg_acl_trigger_offset_time = (curr_ticks+0x180) - ((curr_ticks+0x180)%192);

    btdrv_syn_clr_trigger(0);
    btdrv_enable_playback_triggler(SCO_TRIGGLE_MODE);
    bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, conhdl, BT_TRIG_SLAVE_ROLE,0,false);

    int_unlock(lock);
}

static void app_bth_hfp_stream_btpcm_stop(int dev_id)
{
    (void)dev_id;
    //Notes: adc/dac codec manage is done on the sys core.
    //hal_codec_sync_dac_disable();
    //hal_codec_sync_adc_disable();
}

static void bth_req_sys_start_sco_stream(int dev_id)
{
    app_bt_sco_stream_evt_t bt_sco_stream_req_param;

    bt_sco_stream_req_param.onoff = SCO_STREAM_ON_CODE;

    bt_sco_stream_req_param.codec_type = (uint8_t)bt_adapter_get_hfp_sco_codec_type(dev_id);

#if defined(PCM_FAST_MODE)
    btdrv_open_pcm_fast_mode_enable();
#ifdef PCM_PRIVATE_DATA_FLAG
    bt_drv_reg_op_set_pcm_flag();
#endif
#endif

    btdrv_disable_playback_triggler();
    bth_if_send_result_to_app(APP_BTH_IF_USER_HFP, (uint8_t*)(&bt_sco_stream_req_param),
                                    sizeof(app_bt_sco_stream_evt_t), BTH_IF_CORE_COMM_RESULT_HFP_START_SCO_STREAM);
    osDelay(10);
    bt_drv_reg_op_set_btpcm_trig_flag(true);
    app_bth_hfp_stream_btpcm_start(dev_id);
    TRACE(0, "%s", __func__);
}

static void bth_req_sys_stop_sco_stream(int dev_id)
{
    app_bt_sco_stream_evt_t bt_sco_stream_req_param;

    bt_sco_stream_req_param.onoff = SCO_STREAM_OFF_CODE;
    app_bth_hfp_stream_btpcm_stop(dev_id);

    bth_if_send_result_to_app(APP_BTH_IF_USER_HFP, (uint8_t*)(&bt_sco_stream_req_param),
                                    sizeof(app_bt_sco_stream_evt_t), BTH_IF_CORE_COMM_RESULT_HFP_STOP_SCO_STREAM);
    osDelay(10);
    bt_drv_reg_op_set_btpcm_trig_flag(false);
#if defined(PCM_FAST_MODE)
    btdrv_open_pcm_fast_mode_disable();
#endif
    TRACE(0, "%s",__func__);
}

static int app_bt_hfp_cli_user_hook_func(app_hfp_hf_event_t event, app_hfp_hf_event_param_t *param)
{
    int device_id = BT_DEVICE_INVALID_ID;
    switch (event)
    {
    case APP_HFP_HF_EVENT_AUDIO_CONNECTED:
    {
        if (param)
            device_id = param->p.audio_connected.device_id;
        bth_req_sys_start_sco_stream(device_id);
    }
    break;
    case APP_HFP_HF_EVENT_SERVICE_DISCONNECTED:
    case APP_HFP_HF_EVENT_AUDIO_DISCONNECTED:
    //fall through
    {
        if (param)
            device_id = param->p.audio_disconnected.device_id;
        bth_req_sys_stop_sco_stream(device_id);
    }
    break;
    default:
    break;
    }
    return 0;
}

static void bt_rmt_sco_stream_init(void)
{
    bes_bt_register_hfp_hf_callback(app_bt_hfp_cli_user_hook_func);
}

static const char* hfp_cli_evt2str(uint16_t event)
{
    switch (event) {
        case BT_EVENT_HF_OPENED:
            return "HF_OPENED";
        break;
        case BT_EVENT_HF_CLOSED:
            return "HF_CLOSED";
        break;
        case BT_EVENT_HF_RING_IND:
            return "HF_RING_IND";
        break;
        case BT_EVENT_HF_CLIP_IND:
            return "HF_CLIP_IND";
        break;
        case BT_EVENT_HF_CALL_IND:
            return "HF_CALL_IND";
        break;
        case BT_EVENT_HF_CALLSETUP_IND:
            return "HF_CALLSETUP_IND";
        break;
        case BT_EVENT_HF_CALLHELD_IND:
            return "HF_CALLHELD_IND";
        break;
        case BT_EVENT_HF_VOLUME_CHANGE:
            return "HF_VOLUME_CHANGE";
        break;
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static const char* hfpcli_cmd2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_HFP_SET_CALL_ACTION:
            return "HFP_CLI_CALL_ACTION_SET";
        break;
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static int btif_hfp_evt_customif_cb(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    if (memcmp(hfp_cli_evt2str(event), "UNKNOW", strlen("UNKNOW"))) {
        TRACE(2, "[%s] event=%s", __func__, hfp_cli_evt2str(event));
    }

    app_bth_if_payload_param_info_t info;
    switch (event) {
        case BT_EVENT_HF_OPENED:
        break;
        case BT_EVENT_HF_CLOSED:
        break;
        case BT_EVENT_HF_RING_IND:
        break;
        case BT_EVENT_HF_CLIP_IND: {
            //TRACE(2, "[%s] BT_EVENT_HFP_CALLER_IND caller number %s", __func__, param->hfp_caller_ind.number);
            bt_hf_clip_ind_param_t *clip_ind = param.hf.clip_ind;
            info.hfp_caller_ind_cmd_info.number_len = clip_ind->number_len;
            if(clip_ind->number_len) {
                memcpy(info.hfp_caller_ind_cmd_info.number, clip_ind->number, clip_ind->number_len);
            }
            memcpy(info.hfp_caller_ind_cmd_info.remote.addr, bd_addr->address, 6);
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_HFP, (uint8_t*)&info.context,
                                            sizeof(app_hfp_caller_ind_cmd_t), BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND);
            }
        }
        break;
        case BT_EVENT_HF_CALL_IND: {
            info.hfp_call_state_cmd_info.call_state = (uint8_t)param.hf.call_ind->call;
            memcpy(&info.hfp_call_state_cmd_info.remote.addr, bd_addr->address, 6);
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_HFP, (uint8_t*)&info.context,
                                    sizeof(app_hfp_call_state_cmd_t), BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE);
            }
        }
        break;
        case BT_EVENT_HF_CALLSETUP_IND: {
             info.hfp_callsetup_status_cmd_info.callsetup_state = (uint8_t)param.hf.callsetup_ind->callsetup;
             memcpy(&info.hfp_callsetup_status_cmd_info.remote.addr, bd_addr->address, 6);
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_HFP, (uint8_t*)&info.context,
                                    sizeof(app_hfp_callsetup_state_cmd_t), BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE);
            }
        }
        break;
        case BT_EVENT_HF_CALLHELD_IND:
        break;
        case BT_EVENT_HF_VOLUME_CHANGE:
        break;
        default:
        break;
    }
    return 0;
}

uint32_t app_bth_hfp_cli_if_enable(void *callback_func_if, uint8_t comm_type)
{
    bt_add_event_callback(btif_hfp_evt_customif_cb, BT_EVENT_MASK_HFP_HF_GROUP);
    bth_hf_client_if_cb = (bthf_client_callbacks_t*)callback_func_if;
    return 0;
}

uint32_t app_bth_hfp_cli_if_disable(void)
{
    return 0;
}

uint32_t app_bth_hfp_cli_if_restart(void)
{
    return 0;
}

uint32_t app_bth_hfp_cli_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{
    TRACE(3, "[%s] sub_cmd=%s payload_len=%d", __func__, hfpcli_cmd2str(sub_cmd), ptr_len);
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_HFP_SET_CALL_ACTION: {
            app_hfp_call_action_cmd_t *call_action = (app_hfp_call_action_cmd_t*)ptr;
            uint8_t device_id = bes_bt_me_select_device(BT_SELECT_CURR_HFP_DEVICE);
            bes_bt_hfp_call_action(device_id, call_action->action);
        }
        break;
    }
    return 0;
}

static const app_bth_if_ipc_msg_t bth_hfp_cli_btif_ipc_handle_table = {
    .open           = app_bth_hfp_cli_if_enable,
    .close          = app_bth_hfp_cli_if_disable,
    .restart        = app_bth_hfp_cli_if_restart,
    .ctrl           = app_bth_hfp_cli_if_ctrl,
};

void app_bth_hfp_cli_if_init(void)
{
    bt_rmt_sco_stream_init();
    bth_if_thread_server_register(APP_BTH_IF_USER_HFP, &bth_hfp_cli_btif_ipc_handle_table);
}