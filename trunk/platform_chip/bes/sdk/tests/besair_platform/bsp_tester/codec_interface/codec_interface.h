/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __CODEC_INTERFACE_H__
#define __CODEC_INTERFACE_H__

#include "plat_types.h"
#include "stdbool.h"
#include "hal_aud.h"
#include "hal_codec.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef uint32_t (*AF_STREAM_HANDLER_T)(uint8_t *buf, uint32_t len);
// typedef void (*AF_IRQ_NOTIFICATION_T)(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
typedef void (*CODEC_PLAYBACK_POST_HANDLER_T)(uint8_t *buf, uint32_t len, void *cfg);

enum CODEC_SYNC_TYPE_T {
    CODEC_SYNC_TYPE_NONE,
    CODEC_SYNC_TYPE_GPIO,
    CODEC_SYNC_TYPE_BT,
    CODEC_SYNC_TYPE_WIFI,
};

//Should define return status
uint32_t codec_stream_open(enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg);
uint32_t codec_stream_setup(enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg);
uint32_t codec_stream_mute(enum AUD_STREAM_T stream, bool mute);
uint32_t codec_stream_set_chan_vol(enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);
uint32_t codec_stream_restore_chan_vol(enum AUD_STREAM_T stream);
uint32_t codec_stream_start(enum AUD_STREAM_T stream);
uint32_t codec_stream_stop(enum AUD_STREAM_T stream);
uint32_t codec_stream_pause(enum AUD_STREAM_T stream);
uint32_t codec_stream_restart(enum AUD_STREAM_T stream);
uint32_t codec_stream_close(enum AUD_STREAM_T stream);


void codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio);
void codec_direct_tune_resample_rate(enum AUD_STREAM_T stream, float ratio);
// void codec_tune_both_resample_rate(float ratio);
// void codec_tune_pll(float ratio);
// void codec_tune_xtal(float ratio);
// void codec_tune(enum AUD_STREAM_T stream, float ratio);
void codec_direct_tune(enum AUD_STREAM_T stream, float ratio);

// void codec_set_perf_test_power(int type);
// void codec_set_noise_reduction(bool enable);
// void codec_swap_output(bool swap);
// void codec_set_playback_post_handler(CODEC_PLAYBACK_POST_HANDLER_T hdlr);
// void codec_min_phase_mode_enable(enum AUD_STREAM_T stream);
// void codec_min_phase_mode_disable(enum AUD_STREAM_T stream);


// void codec_set_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src);
// void codec_sync_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable);
// void codec_sync_resample_rate_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable);
// void codec_sync_gain_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable);

// void codec_set_device_bt_sync_source( enum AUD_STREAM_T stream, uint32_t src);
// void codec_sync_device_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable);

// typedef void (*_ANC_HANDLER)(enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T rate, enum AUD_SAMPRATE_T *new_play, enum AUD_SAMPRATE_T *new_cap);
// int _anc_open(enum ANC_TYPE_T type, enum AUD_SAMPRATE_T play_rate, enum AUD_SAMPRATE_T capture_rate, _ANC_HANDLER handler);
// int _anc_close(enum ANC_TYPE_T type);

// int codec_vad_open_simplified(bool init, struct AUD_VAD_SIMP_CFG_T *cfg);
// int _vad_open(const struct AUD_VAD_CONFIG_T *cfg);
// int _vad_close(void);
// int _vad_start(void);
// int _vad_stop(void);
// uint32_t _vad_get_data(uint8_t *buf, uint32_t len);
// void _vad_get_data_info(struct CODEC_VAD_BUF_INFO_T * vad_buf_info);

typedef void (*CODEC_BT_TRIGGER_CALLBACK)(void);
void codec_bt_trigger_config(bool en, CODEC_BT_TRIGGER_CALLBACK callback);

#ifdef HIFI4_ADDA_LOOP_TEST
#define DIGITAL_REG(a)                        (*(volatile uint32_t *)(a))
void adda_loop_start_demo(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CODEC_INTERFACE_H__ */
