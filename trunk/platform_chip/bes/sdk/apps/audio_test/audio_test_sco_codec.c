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
#include "app_utils.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "speech_memory.h"
#include "app_overlay.h"
#include "bt_sco_codec.h"

/**
 * NOTE: When test this module, comment following PLC code to test. Or will crash
#if defined(ENABLE_LPC_PLC)
            lpc_plc_save(plc_st, dec_pcm_buf);
#else
            PLC_good_frame(&plc_st, dec_pcm_buf, dec_pcm_buf);
#endif
#ifdef SPEECH_RX_PLC_DUMP_DATA
            audio_dump_add_channel_data(0, (short *)dec_pcm_buf, MSBC_PCM_FRAME_SIZE/2);
#endif
 */

// #define TEST_SCO_CODEC_AUDIO_DUMP
// #define TEST_SCO_CODEC_WITH_LOCAL_PCM

#ifdef TEST_SCO_CODEC_WITH_LOCAL_PCM
#include "local_pcm.h"
#endif

static const bt_sco_codec_t *g_bt_sco_codec_ptr = &bt_sco_codec_msbc;
// bt_sco_codec_cvsd
// bt_sco_codec_msbc
// bt_sco_codec_xxxx

static uint8_t g_sco_codec_in_buf[1024];
static uint8_t g_sco_codec_out_buf[1024];
static uint8_t g_sco_codec_temp_buf[1024];
static uint8_t g_sco_codec_heap_buf[1024 * 10];

int32_t audio_test_sco_codec(void)
{
    TRACE(1, "[%s] Start", __func__);

#ifdef TEST_SCO_CODEC_WITH_LOCAL_PCM
    local_pcm_init(NULL, 0);
#endif
#ifdef TEST_SCO_CODEC_AUDIO_DUMP
    audio_dump_init(240, sizeof(short), 2);
#endif

    app_overlay_select(APP_OVERLAY_HFP);
    speech_heap_init(g_sco_codec_heap_buf, sizeof(g_sco_codec_heap_buf));
    g_bt_sco_codec_ptr->init(16000);

    for (uint32_t i=0; i<3000; i++) {
        if (i % 100 == 0) {
            TRACE(2, "[%s] cnt: %d", __func__, i);
        }
#ifdef TEST_SCO_CODEC_WITH_LOCAL_PCM
        local_pcm_get_data((uint8_t *)g_sco_codec_in_buf, 240 * sizeof(short));
#endif
#ifdef TEST_SCO_CODEC_AUDIO_DUMP
        audio_dump_add_channel_data(0, (short *)g_sco_codec_in_buf, 240);
#endif

        g_bt_sco_codec_ptr->encoder(g_sco_codec_temp_buf, 120, g_sco_codec_in_buf, 480);
        g_bt_sco_codec_ptr->decoder(g_sco_codec_temp_buf, 120, g_sco_codec_out_buf, 480);

#ifdef TEST_SCO_CODEC_AUDIO_DUMP
        audio_dump_add_channel_data(1, (short *)g_sco_codec_out_buf, 240);
        audio_dump_run();
#endif
    }

    TRACE(1, "[%s] End", __func__);

    return 0;
}
