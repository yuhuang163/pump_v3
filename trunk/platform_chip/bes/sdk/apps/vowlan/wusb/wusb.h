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
#ifndef __WUSB_H__
#define __WUSB_H__

#ifdef __cplusplus
extern "C" {
#endif

int wusb_stream_send(uint8_t *data, uint32_t len, uint64_t play_time);
void wusb_playback_vol_updata(uint32_t level);
void wusb_init(VOW_DEVICE * vodev, SceneParam_t *sceneparam);
void wusb_session_updata(VOW_DEVICE *vodev, SceneParam_t *sceneparam);

#ifdef BLE_WIFI_DUAL_MODE_DONGLE
#include "hal_aud.h"

#define LEA_ENCODE_PCM_FRAME_LEN                (48 * 1 * 2 * 2 * 5)
#define LEA_DECODE_PCM_FRAME_LEN                (48 * 1 * 2 * 2 * 5)

#define BLE_WIFI_DUAL_MODE_SINGAL_PCM_ENCODE    (1 << 0)
#define BLE_WIFI_DUAL_MODE_SINGAL_PCM_DECODE    (1 << 1)

osThreadId get_lea_stream_process_tid(void);

void wusb_lea_stream_process_init(enum AUD_STREAM_T stream);

int wusb_lea_pcmbuff_init(enum AUD_STREAM_T stream);

int wusb_lea_pcmbuff_put(enum AUD_STREAM_T stream, uint8_t *pcm, uint16_t pcm_len);

int wusb_lea_pcmbuff_get(enum AUD_STREAM_T stream, uint8_t *pcm, uint16_t pcm_len);

int wusb_lea_pcmbuff_length(enum AUD_STREAM_T stream);

void wusb_lea_shot_playback_stream_process(void);

void wusb_lea_shot_capture_stream_process(void);

#endif

#ifdef __cplusplus
}
#endif

#endif  /* __WUSB_H__ */