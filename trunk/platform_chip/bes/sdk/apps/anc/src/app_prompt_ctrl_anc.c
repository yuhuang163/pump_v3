/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "hal_trace.h"
#include "app_anc.h"
#include "app_anc_utils.h"
#include "bluetooth_bt_api.h"
#include "app_prompt_ctrl_anc.h"
#include "app_bt_audio.h"
#include "app_tws_ibrt.h"
#include "audio_player_adapter.h"

#define PROMOT_ID_ANC_MASK                          (0xFFFF0FFF)

#define PROMOT_ID_SET_ANC_MODE(mode)                (((uint32_t)mode & 0x07) << 0)
#define PROMOT_ID_GET_ANC_MODE(id)                  ((id >> 0) & 0x07)

#define PROMOT_ID_SET_TRANPARENCY_VOLUME_L(vol)     (((uint32_t)vol & 0x03) << 3)
#define PROMOT_ID_GET_TRANPARENCY_VOLUME_L(id)      ((id >> 3) & 0x03)

#define PROMOT_ID_SET_TRANPARENCY_VOLUME_R(vol)     (((uint32_t)vol & 0x03) << 5)
#define PROMOT_ID_GET_TRANPARENCY_VOLUME_R(id)      ((id >> 5) & 0x03)

#define PROMOT_ID_ANC_CMD_FLAG                      (0x01 << 7)

#define PROMOT_ID_SET_CHANGE_MODE(en)               (((uint32_t)en & 0x01) << 29)
#define PROMOT_ID_GET_CHANGE_MODE(id)               ((id >> 29) & 0x01)

static app_anc_mode_t g_anc_mode    = APP_ANC_MODE_QTY;
static uint16_t g_anc_volume        = ANC_VOLUME_QTY;

#if defined(ANC_PROMPT_CTRL_MUSIC)
#include "audio_process_vol.h"
#define ANC_PROMPT_CTRL_MUSIC_PRE_MS    (300)
#define ANC_PROMPT_CTRL_MUSIC_POST_MS   (300)
#define ANC_PROMPT_CTRL_MUSIC_GAIN      (0.8)
#define ANC_PROMPT_PRE_SKIP_CNT_THD     (ANC_PROMPT_CTRL_MUSIC_PRE_MS / 16)  // 16ms
static uint32_t g_anc_prompt_pre_skip_cnt = 0;
static uint32_t g_anc_prompt_pre_skip_en = false;

uint8_t is_a2dp_mode(void);
#endif

static float g_anc_volume_table[ANC_VOLUME_QTY] = {
    0.25f,
    0.50f,
    1.00f
};

bool is_prompt_ctrl_anc(uint32_t aud_id)
{
    return (aud_id & PROMOT_ID_ANC_CMD_FLAG);
}

app_anc_mode_t app_prompt_ctrl_anc_get_mode(uint32_t aud_id)
{
    return (app_anc_mode_t)PROMOT_ID_GET_ANC_MODE(aud_id);
}

static void _prompt_ctrl_anc_reset(void)
{
    g_anc_mode      = APP_ANC_MODE_QTY;
    g_anc_volume    = ANC_VOLUME_QTY;
}

uint32_t app_prompt_ctrl_anc(app_anc_mode_t mode)
{
    uint32_t aud_id = PROMOT_ID_ANC_CMD_FLAG;

    ASSERT(mode <= APP_ANC_MODE1, "[%s] mode(%d) > APP_ANC_MODE1", __func__, mode);

    aud_id |= PROMOT_ID_SET_ANC_MODE(mode);
    aud_id |= PROMOT_ID_SET_CHANGE_MODE(true);

    TRACE(3, "[%s] aud_id: 0x%x, mode: %d", __func__, aud_id, mode);

    audio_player_play_prompt((AUD_ID_ENUM)aud_id, 0);

    return aud_id;
}

uint32_t app_prompt_ctrl_anc_volume(uint16_t need_update_mode, uint16_t vol_l, uint16_t vol_r)
{
    uint32_t aud_id = PROMOT_ID_ANC_CMD_FLAG;

    aud_id |= PROMOT_ID_SET_CHANGE_MODE(need_update_mode);
    aud_id |= PROMOT_ID_SET_ANC_MODE(APP_ANC_MODE1);
    aud_id |= PROMOT_ID_SET_TRANPARENCY_VOLUME_L(vol_l);
    aud_id |= PROMOT_ID_SET_TRANPARENCY_VOLUME_R(vol_r);

    TRACE(5, "[%s] aud_id: 0x%x, need: %d, vol: %d, %d", __func__, aud_id, need_update_mode, vol_l, vol_r);

    audio_player_play_prompt((AUD_ID_ENUM)aud_id, 0);

    return aud_id;
}

uint32_t app_prompt_ctrl_anc_init(uint32_t aud_id)
{
#if defined(ANC_PROMPT_CTRL_MUSIC)
    g_anc_prompt_pre_skip_cnt = 0;
    g_anc_prompt_pre_skip_en = false;
#endif

    if (is_prompt_ctrl_anc(aud_id) == false) {
        return PROMPT_ID_FROM_ID_VALUE(aud_id);
    }

    _prompt_ctrl_anc_reset();

    aud_id &= PROMOT_ID_ANC_MASK;
    g_anc_mode = (app_anc_mode_t)PROMOT_ID_GET_ANC_MODE(aud_id);

    if (g_anc_mode == APP_ANC_MODE_OFF) {
        aud_id = AUD_ID_ANC_MODE0;
        TRACE(3, "[%s] aud_id: 0x%x, mode: %d", __func__, aud_id, g_anc_mode);
    } else if (g_anc_mode == APP_ANC_MODE1) {
        // FIXME: Check hw side
        if (app_tws_ibrt_get_local_tws_role() == BTIF_BCR_MASTER) {
            g_anc_volume = PROMOT_ID_GET_TRANPARENCY_VOLUME_L(aud_id);
        } else {
            g_anc_volume = PROMOT_ID_GET_TRANPARENCY_VOLUME_R(aud_id);
        }
        if (PROMOT_ID_GET_CHANGE_MODE(aud_id)) {
            aud_id = AUD_ID_ANC_MODE1;
        } else {
            aud_id = AUD_ID_ANC_MUTE;
            g_anc_mode = APP_ANC_MODE_QTY;
        }
        TRACE(4, "[%s] aud_id: 0x%x, mode: %d, volume: %d", __func__, aud_id, g_anc_mode, g_anc_volume);
    } else {
        // NOTE: Don't need to change anc mode
        aud_id = AUD_ID_ANC_MUTE;
        TRACE(3, "[%s] Don't change mode. aud_id: 0x%x, mode: %d", __func__, aud_id, g_anc_mode);
    }

    return aud_id;
}

int32_t app_prompt_ctrl_anc_process(void)
{
    if (g_anc_mode < APP_ANC_MODE_QTY) {
        TRACE(2, "[%s] Mode: %d", __func__, g_anc_mode);
#if defined(ANC_PROMPT_CTRL_MUSIC)
        if (is_a2dp_mode()) {
            TRACE(1, "[%s] Start ctrl music gain", __func__);
            g_anc_prompt_pre_skip_cnt = ANC_PROMPT_PRE_SKIP_CNT_THD;
            g_anc_prompt_pre_skip_en = true;
        } else {
            g_anc_prompt_pre_skip_cnt = 0;
            g_anc_prompt_pre_skip_en = false;
        }
#endif
        app_anc_switch_locally(g_anc_mode);
        g_anc_mode = APP_ANC_MODE_QTY;
    }

    if (g_anc_volume < ANC_VOLUME_QTY) {
        TRACE(2, "[%s] g_anc_volume: %d", __func__, g_anc_volume);
        app_anc_thread_set_gain(ANC_FEEDFORWARD, g_anc_volume_table[g_anc_volume], g_anc_volume_table[g_anc_volume]);
        g_anc_volume = ANC_VOLUME_QTY;
    }

    return 0;
}

#if defined(ANC_PROMPT_CTRL_MUSIC)
int32_t app_prompt_ctrl_music_process(void)
{
    ASSERT(g_anc_prompt_pre_skip_cnt <= ANC_PROMPT_PRE_SKIP_CNT_THD, "[%s] Invalid: %d", __func__, g_anc_prompt_pre_skip_cnt);
    if (g_anc_prompt_pre_skip_cnt > 0) {
        if (g_anc_prompt_pre_skip_cnt == ANC_PROMPT_PRE_SKIP_CNT_THD) {
            audio_process_vol_start_impl(ANC_PROMPT_CTRL_MUSIC_GAIN, (uint32_t)(ANC_PROMPT_CTRL_MUSIC_PRE_MS / (1.0 - ANC_PROMPT_CTRL_MUSIC_GAIN)));
        } else {
            TRACE(2, "[%s] ANC switch skip cnt: %d", __func__, g_anc_prompt_pre_skip_cnt);
        }
        g_anc_prompt_pre_skip_cnt--;
        return 1;
    } else {
        return 0;
    }
}

int32_t app_prompt_ctrl_music_check_stop(void)
{
    if (g_anc_prompt_pre_skip_en) {
        g_anc_prompt_pre_skip_en = false;
        audio_process_vol_start_impl(1.0, (uint32_t)(ANC_PROMPT_CTRL_MUSIC_POST_MS / (1.0 - ANC_PROMPT_CTRL_MUSIC_GAIN)));
    }

    return 0;
}
#endif