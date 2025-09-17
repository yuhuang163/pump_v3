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
#ifdef AOB_MOBILE_ENABLED
#ifdef BLE_USB_AUDIO_SUPPORT
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "ble_audio_dbg.h"
#include "ble_audio_core.h"
#include "ble_audio_mobile_info.h"
#include "app_ble_usb_stream_stm.h"
#include "app_ble_usb_audio.h"
#include "aob_media_api.h"
#include "aob_pacs_api.h"
#include "list.h"
#include "app_gaf_custom_api.h"
#include "gaf_mobile_usb_audio.h"

#define INVALID_LID         (0xFF)

static usb_stream_stm_t ble_usb_stream_stm;

static const char* usb_stream_sm_event_to_string(int event)
{
    #define CASES(s) case s:return "["#s"]";

    switch(event)
    {
        CASES(USB_REQ_STREAM_PLAYBACK_START)
        CASES(USB_REQ_STREAM_CAPTURE_START)
        CASES(USB_REQ_STREAM_PLAYBACK_STOP)
        CASES(USB_REQ_STREAM_CAPTURE_STOP)
        CASES(EVT_PLAYBACK_STREAM_STARTED)
        CASES(EVT_CAPTURE_STREAM_STARTED)
        CASES(START_EVT)
        CASES(ENTRY_EVT)
        CASES(EXIT_EVT)
    }
    return "[UNDEFINE EVENT]";
}

static void ble_audio_bidirectional_stream_start_handler(uint8_t con_lid)
{
    LOG_I("%s", __func__);

    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start[] =
    {
        {APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 40, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_GAME},
        {APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 40, APP_GAF_DIRECTION_SRC, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_GAME},
    };

    aob_media_mobile_start_stream(ase_to_start, con_lid, true);
}

void ble_usb_stream_resume_ble_audio(uint8_t con_lid)
{
    BLE_USB_STREAM_STATE_E stream_state = ble_usb_stream_stm_get_cur_state();
    LOG_I("%s stream_state=%d", __func__,stream_state);
    ble_audio_bidirectional_stream_start_handler(con_lid);
}

static bool ble_usb_stream_ase_enable_availability_check(uint8_t con_lid, uint8_t direction, uint16_t ctx)
{
    uint16_t ava_ctx = aob_pacs_mobile_get_peer_ava_context(con_lid, direction);

    if (ava_ctx & ctx)
    {
        LOG_W("%s [%d] ready for enable, ctx = 0x%x", __func__, con_lid, ctx);
        return true;
    }

    LOG_I("%s [%d] not ready for enable, ava ctx = 0x%x, enable ctx = 0x%x", 
          __func__, con_lid, ava_ctx, ctx);
    return false;
}

static bool ble_usb_stream_check_ava_ctx_and_enable_stream(uint8_t con_lid, uint8_t direction)
{
    uint8_t nb_ase = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCC_NB_ASE_CFG] = {0};
    nb_ase = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                APP_BAP_UC_ASE_STATE_QOS_CONFIGURED,
                                                                ase_lid_list);
    if (nb_ase == 0)
    {
        LOG_I("USB-STREAM link(%d) ase is not ready", con_lid);
        return false;
    }
    /// enable all ase in qos configured state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase; ase_idx++)
    {
        ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid_list[ase_idx]);
        if (p_ase_sm && ble_usb_stream_ase_enable_availability_check(con_lid, direction,
                                                        p_ase_sm->ase_info.ase_cfg_info.context_type))
        {
            aob_media_mobile_enable_stream(ase_lid_list[ase_idx]);
        }
        else
        {
            return false;
        }
    }

    return true;
}

static void ble_usb_stream_disable_stream(uint8_t con_lid, uint8_t direction)
{
    uint8_t nb_ase_streaming = 0;
    uint8_t nb_ase_enabling = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCC_NB_ASE_CFG] = {0};

    nb_ase_streaming = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                APP_BAP_UC_ASE_STATE_STREAMING,
                                                                ase_lid_list);
    /// disable all ase in streaming state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_streaming; ase_idx++)
    {
        aob_media_mobile_disable_stream(ase_lid_list[ase_idx]);
    }

    nb_ase_enabling = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                APP_BAP_UC_ASE_STATE_ENABLING,
                                                                ase_lid_list);
    /// disable all ase in enabling state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase_enabling; ase_idx++)
    {
        aob_media_mobile_disable_stream(ase_lid_list[ase_idx]);
    }

    if (nb_ase_streaming + nb_ase_enabling == 0)
    {
        LOG_W("USB-STREAM link(%d) no ase to disable", con_lid);
    }
}

Msg const* ble_usb_stream_sm_super_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: SUPER state on event: %s", usb_stream_sm_event_to_string(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            STATE_START(me, &me->idle);
            return 0;
        case ENTRY_EVT:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_idle_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: IDLE State on event: %s", usb_stream_sm_event_to_string(msg->evt));

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {   
                ble_usb_stream_check_ava_ctx_and_enable_stream(i, APP_GAF_DIRECTION_SINK);
            }
            STATE_TRAN(me, &me->playback_start);
            return 0;
        case USB_REQ_STREAM_CAPTURE_START:
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_check_ava_ctx_and_enable_stream(i, APP_GAF_DIRECTION_SRC);
            }
            STATE_TRAN(me, &me->capture_start);
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_playback_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: playback state on event: %s", usb_stream_sm_event_to_string(msg->evt));

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case EVT_PLAYBACK_STREAM_STARTED:
            gap_mobile_start_usb_audio_receiving_dma();
            return 0;
        case USB_REQ_STREAM_PLAYBACK_STOP:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_disable_stream(i, APP_GAF_DIRECTION_SINK);
            }
        }
            STATE_TRAN(me, &me->idle);
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
            return 0;
        case USB_REQ_STREAM_CAPTURE_START:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_check_ava_ctx_and_enable_stream(i, APP_GAF_DIRECTION_SRC);
            }
        }
            STATE_TRAN(me, &me->both_start);
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_capture_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: CAPTURE state on event: %s", usb_stream_sm_event_to_string(msg->evt));

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case EVT_CAPTURE_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_transmission_dma();
        }
            return 0;
        case USB_REQ_STREAM_CAPTURE_STOP:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_disable_stream(i, APP_GAF_DIRECTION_SRC);
            }
        }
            STATE_TRAN(me, &me->idle);
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_check_ava_ctx_and_enable_stream(i, APP_GAF_DIRECTION_SRC);
            }
        }
            STATE_TRAN(me, &me->both_start);
            return 0;
        case USB_REQ_STREAM_CAPTURE_START:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_both_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: BOTH stream state on event: %s", usb_stream_sm_event_to_string(msg->evt));

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
        case USB_REQ_STREAM_CAPTURE_START:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_STOP:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_disable_stream(i, APP_GAF_DIRECTION_SINK);
            }
        }
            STATE_TRAN(me, &me->capture_start);
            return 0;
        case USB_REQ_STREAM_CAPTURE_STOP:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ble_usb_stream_disable_stream(i, APP_GAF_DIRECTION_SRC);
            }
        }
            STATE_TRAN(me, &me->playback_start);
            return 0;
        case EVT_CAPTURE_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_transmission_dma();
        }
            return 0;
        case EVT_PLAYBACK_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_receiving_dma();
        }
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

int ble_usb_stream_sm_on_event(uint32_t stream_stm_ptr,uint32_t event,uint32_t param0,uint32_t param1)
{
    Msg message;
    usb_stream_stm_t *p_stream_sm = NULL;

    message.evt = event;
    message.param0 = param0;
    message.param1 = param1;
    message.param2 = 0;

    p_stream_sm = (usb_stream_stm_t*)stream_stm_ptr;
    HsmOnEvent((Hsm *)p_stream_sm, &message);

    return 0;
}

void ble_usb_stream_send_message(usb_stream_stm_t *p_stream_sm, BLE_USB_STREAM_EVENT_E event, uint32_t param0, uint32_t param1)
{
    app_bt_call_func_in_bt_thread((uint32_t)p_stream_sm,(uint32_t)event,param0,param1,(uint32_t)ble_usb_stream_sm_on_event);
}

usb_stream_stm_t* ble_usb_get_stream_sm(void)
{
    usb_stream_stm_t* iter = &ble_usb_stream_stm;

    return iter;

}

BLE_USB_STREAM_STATE_E ble_usb_stream_stm_get_cur_state(void)
{
    usb_stream_stm_t *p_stream_sm = &ble_usb_stream_stm;

    BLE_USB_STREAM_STATE_E state = BLE_USB_STREAM_STATE_UNKNOW;
    if (!p_stream_sm->used) {
        return state;
    }

    if (p_stream_sm->super.curr == &p_stream_sm->idle)
    {
        state = BLE_USB_STREAM_STATE_IDLE;
    } else if (p_stream_sm->super.curr == &p_stream_sm->playback_start) {
        state = BLE_USB_STREAM_STATE_PLAYBACK_STARTED;
    } else if (p_stream_sm->super.curr == &p_stream_sm->capture_start) {
        state = BLE_USB_STREAM_STATE_CAPTURE_STARTED;
    } else if (p_stream_sm->super.curr == &p_stream_sm->both_start) {
        state = BLE_USB_STREAM_STATE_BOTH_STARTED;
    }

    return state;
}

void ble_usb_stream_stm_startup(usb_stream_stm_t *ble_usb_stream_sm)
{
    LOG_I("USB-STREAM: %s", __func__);
    ble_usb_stream_stm.used = true;

    HsmCtor((Hsm *)ble_usb_stream_sm, "stream_super", (EvtHndlr)ble_usb_stream_sm_super_state);
    AddState(&ble_usb_stream_sm->idle, "idle",  &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_idle_state);
    AddState(&ble_usb_stream_sm->playback_start, "playback_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_playback_start_state);
    AddState(&ble_usb_stream_sm->capture_start, "capture_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_capture_start_state);
    AddState(&ble_usb_stream_sm->both_start, "both_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_both_start_state);

    HsmOnStart((Hsm *)ble_usb_stream_sm);
}

static void ble_usb_stream_enable_ase_with_no_check(uint8_t con_lid, uint8_t direction)
{
    uint8_t nb_ase = 0;
    uint8_t ase_lid_list[APP_BAP_DFT_ASCC_NB_ASE_CFG] = {0};
    nb_ase = app_bap_uc_cli_get_specific_state_ase_lid_list(con_lid, direction,
                                                                APP_BAP_UC_ASE_STATE_QOS_CONFIGURED,
                                                                ase_lid_list);
    if (nb_ase == 0)
    {
        LOG_I("USB-STREAM link(%d) ase is not ready", con_lid);
    }
    /// enable all ase in qos configured state
    for (uint32_t ase_idx = 0; ase_idx < nb_ase; ase_idx++)
    {
        aob_media_mobile_enable_stream(ase_lid_list[ase_idx]);
    }
}

static void ble_usb_stream_stm_ava_ctx_changed_cb(uint8_t con_lid,
                                                  uint16_t sink_ava_ctx,
                                                  uint16_t src_ava_ctx)
{
    BLE_USB_STREAM_STATE_E usb_stream_state = ble_usb_stream_stm_get_cur_state();

    /// usb are both on and sink and src ava ctx contains conversational
    if ((usb_stream_state == BLE_USB_STREAM_STATE_BOTH_STARTED) &&
        (src_ava_ctx & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL) &&
        (sink_ava_ctx & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL))
    {
        ble_usb_stream_enable_ase_with_no_check(con_lid, APP_GAF_DIRECTION_SINK);
        ble_usb_stream_enable_ase_with_no_check(con_lid, APP_GAF_DIRECTION_SRC);
    }
    /// or usb playback is on and sink ava ctx contains media
    else if ((usb_stream_state == BLE_USB_STREAM_STATE_PLAYBACK_STARTED) &&
                (sink_ava_ctx & AOB_AUDIO_CONTEXT_TYPE_MEDIA))
    {
        ble_usb_stream_enable_ase_with_no_check(con_lid, APP_GAF_DIRECTION_SINK);
    }
    /// or usb capture is one and src ava ctx contains conversational
    else if ((usb_stream_state == BLE_USB_STREAM_STATE_CAPTURE_STARTED) &&
                (src_ava_ctx & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL))
    {
        ble_usb_stream_enable_ase_with_no_check(con_lid, APP_GAF_DIRECTION_SRC);
    }
    else
    {
        LOG_I("USB-STREAM: [%d] stream can not start here, stream state: %d",
                                                        con_lid, usb_stream_state);
    }
}

void ble_usb_stream_stm_init(void)
{
    memset(&ble_usb_stream_stm, 0xFF, sizeof(usb_stream_stm_t));
    ble_usb_stream_stm.used = false;
    aob_mobile_ava_ctx_changed_cb_init(ble_usb_stream_stm_ava_ctx_changed_cb);
}

#endif
#endif //AOB_MOBILE_ENABLED

