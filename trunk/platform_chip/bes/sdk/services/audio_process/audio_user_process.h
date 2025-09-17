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
#ifndef __AUDIO_USER_PROCESS_H__
#define __AUDIO_USER_PROCESS_H__

#include "stdbool.h"

typedef enum
{
    AUDIO_USER_FOCUS_TYPE_IDLE,
    ADUIO_USER_FOCUS_TYPE_A2DP,
    AUDIO_USER_FOCUS_TYPE_SCO,
    ADUIO_USER_FOCUS_TYPE_BIS,
    AUDIO_USER_FOCUS_TYPE_ALSA,
    AUDIO_USER_FOCUS_TYPE_NUM,
} audio_user_focus_ctrl_type_t;

#define USER_PROCESS_48k_SAMPLE_NUM (256 * 2)

typedef int (*audio_user_process_open_t)(int sample_rate, int in_channels, int out_channels, int in_bits, int out_bits, uint32_t timeout);
typedef int (*audio_user_process_close_t)(void);
typedef int (*audio_user_process_run_t)(uint8_t *data, uint32_t in_out_length);

typedef struct
{
    audio_user_process_open_t open;
    audio_user_process_close_t close;
    audio_user_process_run_t run;
} audio_user_process_t;

typedef int (*audio_user_sw_gain_porcess_open_t)(uint8_t channels, uint8_t bits, uint32_t sample_rate, float f0);
typedef int (*audio_user_sw_gain_process_close_t)(void);
typedef int (*audio_user_sw_gain_process_run_t)(uint8_t *buf, uint32_t len);
typedef int (*audio_user_sw_gain_process_fade_in_t)(bool skip_zero_data);
typedef int (*audio_user_sw_gain_process_fade_out_t)(void);

typedef struct
{
    audio_user_sw_gain_porcess_open_t open;
    audio_user_sw_gain_process_close_t close;
    audio_user_sw_gain_process_run_t process;
    audio_user_sw_gain_process_fade_in_t fade_in;
    audio_user_sw_gain_process_fade_out_t fade_out;
} audio_user_sw_gain_process_t;

typedef int (*audio_user_dump_raw_data_config_t)(uint32_t sample_rate, uint8_t channels, uint8_t bits);
typedef int (*audio_user_dump_raw_data_t)(uint8_t *data, uint32_t length);
typedef int (*audio_user_dump_before_process_t)(uint8_t *data, uint32_t length);
typedef int (*audio_user_dump_after_process_t)(uint8_t *data, uint32_t length);

typedef struct
{
    audio_user_dump_raw_data_config_t dump_raw_data_config;
    audio_user_dump_raw_data_t dump_raw_data;
    audio_user_dump_before_process_t dump_before_process;
    audio_user_dump_after_process_t dump_after_process;
} audio_user_dump_t;

typedef int (*audio_user_focus_ctrl_acquire_t)(audio_user_focus_ctrl_type_t type);
typedef int (*audio_user_focus_ctrl_release_t)(audio_user_focus_ctrl_type_t type);
typedef struct {
    audio_user_focus_ctrl_acquire_t acquire;
    audio_user_focus_ctrl_release_t release;
} audio_user_focus_ctrl_t;

#ifdef __cplusplus
extern "C"
{
#endif

    void audio_user_process_register(audio_user_process_t *h);
    void audio_user_process_unregister(void);
    int audio_user_process_open(int sample_rate, int in_channels, int out_channels, int in_bits, int out_bits, uint32_t timeout);
    int audio_user_process_close(void);
    int audio_user_process_run(uint8_t *data, uint32_t len);
    bool audio_user_process_is_enable(void);

    void audio_user_sw_gain_process_register(audio_user_sw_gain_process_t *ift);
    void audio_user_sw_gain_process_unregister(void);
    int audio_user_sw_gain_process_open(uint8_t channels, uint8_t bits, uint32_t sample_rate, float f0);
    int audio_user_sw_gain_process_close(void);
    int audio_user_sw_gain_process_run(uint8_t *buf, uint32_t len);
    int audio_user_sw_gain_process_fade_in(bool skip_zero_data);
    int audio_user_sw_gain_process_fade_out(void);
    bool audio_user_sw_gain_process_is_enable(void);

    void audio_user_dump_register(audio_user_dump_t *h);
    void audio_user_dump_unreister(void);
    int audio_user_dump_raw_data_config(uint32_t sample_rate, uint8_t channels, uint8_t bits);
    int audio_user_dump_raw_data(uint8_t *data, uint32_t length);
    int audio_user_dump_before_process(uint8_t *data, uint32_t length);
    int audio_user_dump_after_process(uint8_t *data, uint32_t length);

    void audio_user_focus_ctrl_register(audio_user_focus_ctrl_t *h);
    void audio_user_focus_ctrl_unregister(void);
    int audio_user_focus_ctrl_acquire(audio_user_focus_ctrl_type_t user);
    int audio_user_focus_ctrl_release(audio_user_focus_ctrl_type_t user);

#ifdef __cplusplus
}
#endif

#endif
