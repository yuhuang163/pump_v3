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
#include "bth_if_evt_def.h"
#include "bth_sco_stream.h"

static const bthf_client_callbacks_t* ui_hfp_cli_evt_if_cbs = NULL;

static const char* hfp_ret2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_HFP_START_SCO_STREAM:
            return "HFP_STREAM_START_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_HFP_STOP_SCO_STREAM:
            return "HFP_STREAM_STOP_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND:
            return "HFP_CALLER_IND_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE:
            return "HFP_CALL_STATE_EVT";
            break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE:
            return "HFP_CALLSETUP_STATE_EVT";
            break;
        default:
            break;
    }
    return "UNKNOW";
}

static int32_t _btif_client_hfp_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd)
{
    TRACE(2, "[%s] recv hfp rsp sub_cmd=%s len=%d", __func__, hfp_ret2str(sub_cmd), len);
    switch(sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_HFP_START_SCO_STREAM: {
             bt_sco_player_start(*((uint8_t*)buf+1));
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_STOP_SCO_STREAM: {
             bt_sco_player_stop();
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND: {
            app_hfp_caller_ind_cmd_t *caller_ind = (app_hfp_caller_ind_cmd_t*)buf;
            if(ui_hfp_cli_evt_if_cbs->caller_info_cb) {
                ui_hfp_cli_evt_if_cbs->caller_info_cb((bt_rawaddr_t*)&caller_ind->remote,
                                   (const uint8_t *)caller_ind->number, caller_ind->number_len);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE: {
            app_hfp_call_state_cmd_t *call_state = (app_hfp_call_state_cmd_t*)buf;
            if(ui_hfp_cli_evt_if_cbs->call_state_cb) {
                ui_hfp_cli_evt_if_cbs->call_state_cb((bt_rawaddr_t*)&call_state->remote, call_state->call_state);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE: {
            app_hfp_callsetup_state_cmd_t *call_setup = (app_hfp_callsetup_state_cmd_t*)buf;
            if(ui_hfp_cli_evt_if_cbs->call_setup_state_cb) {
                ui_hfp_cli_evt_if_cbs->call_setup_state_cb((bt_rawaddr_t*)&call_setup->remote, call_setup->callsetup_state);
            }
        }
        break;
        default:
        break;
    }

    return 0;
}

int32_t app_bth_hfp_cli_if_client_init(void)
{
    app_bth_if_thread_client_result_register(APP_BTH_IF_USER_HFP, _btif_client_hfp_result_rsp_callback);
    return 0;
}

int32_t app_bth_hfp_cli_if_client_open(void* ui_callback, uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, ui_callback);
    ui_hfp_cli_evt_if_cbs = ui_callback;
    bth_if_thread_client_open(APP_BTH_IF_USER_HFP, (uint32_t)ui_callback, comm_type);
    return 0;
}

int32_t app_bth_hfp_cli_if_client_close(void)
{
    bth_if_thread_client_close(APP_BTH_IF_USER_HFP);
    ui_hfp_cli_evt_if_cbs = NULL;
    return 0;
}

int32_t app_bth_hfp_cli_if_client_reset(void)
{
    bth_if_thread_client_reset(APP_BTH_IF_USER_HFP);
    return 0;
}

int32_t app_bth_hfp_cli_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len)
{
    bth_if_thread_client_ctrl(APP_BTH_IF_USER_HFP, ctrl_cmd, payload, payload_len);
    return 0;
}