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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"

#include "vow_dev.h"
#include "vow_session.h"
#include "wsink.h"

/* Slave reception RF data , Lacol player.*/

static int wsink_vsess_i2s_id = -1;
VOW_AUDIO_CFG local_out_audio_cfg;

void wsink_init(VOW_DEVICE *vodev, SceneParam_t *sceneparam)
{
    static bool isinit = false;

    if (!vodev) {
        TRACE(0,"wsink_init fail, vodev error");
        return;
    }

    if(isinit) {
        return;
    } else {
        isinit =true;
    }

    if ((sceneparam->output_device & VOW_OUTPUT_IIS) || (sceneparam->output_device & VOW_OUTPUT_DAC)) {
        local_out_audio_cfg.sample_rate = sceneparam->sample_freq;
        local_out_audio_cfg.sample_depth = sceneparam->sample_depth;
        local_out_audio_cfg.mute_state = 0;
        local_out_audio_cfg.delay = 0;
        local_out_audio_cfg.channel_num = sceneparam->play_ch_num;
        local_out_audio_cfg.vol = 15;
        local_out_audio_cfg.burst_size = 1;

        if (sceneparam->output_device & VOW_OUTPUT_IIS) {
            wsink_vsess_i2s_id = vow_session_alloc(vodev, &local_out_audio_cfg, VOW_SOURCE_WIFI, VOW_DEST_IIS);
            if (wsink_vsess_i2s_id < 0) {
                TRACE(0,"%s vow_session_alloc fail line=%d wsink_vsess_i2s_id =%d", __func__, __LINE__, wsink_vsess_i2s_id);
                return;
            }
        }

        /* TODO : VOW_OUTPUT_DAC */
    }
}

void wsink_playback_vol_changed(uint32_t level)
{
    TRACE(0, "%s %d", __func__, level);
    SceneParam_t * sceneparam = vow_get_scene_param();
    VOW_SESSION *session = NULL;
    uint8_t i;

    if (local_out_audio_cfg.vol != level) {
        if (wsink_vsess_i2s_id < 0) {
            TRACE(0,"%s wsink_vsess_i2s_id = %d is error!!.", __func__, wsink_vsess_i2s_id);
            return;
        } else {
//            session = vow_get_session(wsink_vsess_i2s_id);
//            if (session == NULL) {
//                TRACE(0,"%s, session[%d] is NULL", __func__, i);
//                return;
//            }
//
//            session->audio_cfg.vol = level;

            uint8_t cmd_data[VOW_FRAME_MAX_CMD_LEN] = {0};

            cmd_data[VOW_CMD_ACK_TYPE_SEQ] = VOW_ACK_TYPE_SYNC_CFG;

            cmd_data[VOW_CMD_ACK_SYNC_CFG_TYPE_SEQ] &= ~(1 << 7);   /* DIR : S to M. */
            cmd_data[VOW_CMD_ACK_SYNC_CFG_TYPE_SEQ] |= (1 << 2);    /* TYPE : set vol CMD */

            cmd_data[VOW_CMD_ACK_SYNC_CFG_CONT2_SEQ] = level;
            for (i = 0; i < sceneparam->session_num ; i++) {
                session = vow_get_session(i);
                if (session == NULL) {
                    TRACE(0,"%s, session[%d] is NULL", __func__, i);
                    break;
                }
                session->audio_cfg.vol = level;
                TRACE(0,"SYNC_CFG CMD set vol [%d]", level);
            }

            vow_cmd_input(VOW_CMD_TYPE_ACK, cmd_data, VOW_FRAME_ACK_CMD_LEN);
        }

        local_out_audio_cfg.vol = level;
    }
}

