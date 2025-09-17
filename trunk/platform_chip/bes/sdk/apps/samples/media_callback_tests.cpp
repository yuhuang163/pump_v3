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
#include "media_callback_tests.h"

#include "app_media_player.h"
#include "hal_trace.h"
#include "bt_common_define.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_profiles_api.h"
#endif

static void app_prompt_finish_callback_register_test_report_id(int aud_id);

void prompt_stop_all_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_prompt_stop_all();
}

void media_PlayAudio_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_locally_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio_locally(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_remotely_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio_remotely(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_standalone_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio_standalone(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_standalone_locally_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio_standalone_locally(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_standalone_remotely_test(char* cmdParam, uint32_t cmdParam_len)
{
    media_PlayAudio_standalone_remotely(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

void media_PlayAudio_continuous_test(char* cmdParam, uint32_t cmdParam_len)
{
    static bool PlayAudio_continuous_test = true;
    if (PlayAudio_continuous_test)
    {
        media_PlayAudio_continuous_start(AUD_ID_BT_CALL_INCOMING_CALL, 0);
    }
    else
    {
        media_PlayAudio_continuous_end(AUD_ID_BT_CALL_INCOMING_CALL, 0);
    }
    PlayAudio_continuous_test = !PlayAudio_continuous_test;
}

static void app_prompt_finish_callback_register_test_report_id(int aud_id)
{
    TRACE(0, "callback_register_test_report %d", aud_id);
}

void app_prompt_finish_callback_register_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_prompt_finish_callback_register(
        (app_prompt_finish_cb)app_prompt_finish_callback_register_test_report_id
        );
}

#ifdef IBRT
void app_ibrt_send_volumeup_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_ibrt_if_a2dp_send_volume_up(BT_DEVICE_ID_1);
}

void app_ibrt_send_volumedown_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_ibrt_if_a2dp_send_volume_down(BT_DEVICE_ID_1);
}

void app_ibrt_send_set_absvolume_test(char* cmdParam, uint32_t cmdParam_len)
{
    static uint8_t abs_vol = 80;
    app_ibrt_if_a2dp_send_set_abs_volume(BT_DEVICE_ID_1, abs_vol++);
}

void app_ibrt_adjust_local_volumeup_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_ibrt_if_set_local_volume_up();
}

void app_ibrt_adjust_local_volumedown_test(char* cmdParam, uint32_t cmdParam_len)
{
    app_ibrt_if_set_local_volume_down();
}
#endif
