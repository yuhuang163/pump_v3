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

#ifndef __BTH_WALKIE_TALKIE_STREAM_H__
#define __BTH_WALKIE_TALKIE_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WT_AUDIO_ENCODER_INIT_BUFFER)(void);
typedef void (*WT_AUDIO_ENCODER_INIT_FUNC)(void);
typedef void (*WT_AUDIO_ENCODER_DEINIT_FUNC)(void* p_stream_env);
typedef bool (*WT_AUDIO_ENCODER_ENCODE_FRAME_FUNC)(uint8_t* in_buf, uint8_t* out_buff, uint32_t length);
typedef void (*WT_AUDIO_ENCODER_DEINIT_BUFFER)(void* p_stream_env);

typedef struct
{
    WT_AUDIO_ENCODER_INIT_BUFFER   encoder_init_buf_func;
    WT_AUDIO_ENCODER_INIT_FUNC     encoder_init_func;
    WT_AUDIO_ENCODER_DEINIT_FUNC   encoder_deinit_func;
    WT_AUDIO_ENCODER_ENCODE_FRAME_FUNC encoder_encode_frame_func;
    WT_AUDIO_ENCODER_DEINIT_BUFFER   encoder_deinit_buf_func;
} WT_AUDIO_ENCODER_FUNC_LIST_T;

int wt_audio_capture_stream_start(void);
int wt_audio_capture_stream_stop(void);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_WALKIE_TALKIE_STREAM_H__