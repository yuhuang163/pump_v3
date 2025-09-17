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
#include <stdint.h>
#include <string.h>

typedef struct
{
    uint32_t freq;
    uint32_t *ratio_set[2];
    uint32_t ratio_cnt;
    uint32_t up_delay_time;
    uint32_t down_delay_time;
    uint8_t bypass;
} audio_process_pa_envelope_cfg_t;

typedef int (*audio_pa_envelope_pwm_ctr_t)(void *duty_ratio, uint32_t num, uint32_t freq);
typedef int (*audio_process_pa_envelope_open_t)(int sample_rate, int channels, int bits, uint32_t half_dma_size, uint32_t envelope_sample_num);
typedef int (*audio_process_pa_envelope_start_t)(void);
typedef int (*audio_process_pa_envelope_close_t)(void);
typedef int (*audio_process_pa_envelope_run_t)(const uint8_t *input, uint32_t len);
typedef int (*audio_process_pa_envelope_signal_t)(int delay_time);
typedef int (*audio_process_pa_envelope_stop_t)(void);
typedef int (*audio_process_pa_envelope_set_cfg_t)(audio_process_pa_envelope_cfg_t *cfg);
typedef int (*audio_process_pa_envelope_get_cfg_t)(audio_process_pa_envelope_cfg_t *cfg);

typedef struct
{
    audio_process_pa_envelope_open_t open;
    audio_process_pa_envelope_start_t start;
    audio_process_pa_envelope_signal_t set_signal;
    audio_process_pa_envelope_run_t run;
    audio_process_pa_envelope_stop_t stop;
    audio_process_pa_envelope_close_t close;
    audio_process_pa_envelope_set_cfg_t set_cfg;
    audio_process_pa_envelope_get_cfg_t get_cfg;
} audio_process_pa_envelope_if_t;

#ifdef __cplusplus
extern "C" {
#endif

void audio_process_pa_envelope_register_if(audio_process_pa_envelope_if_t *ift);

void audio_process_pa_envelope_unregister_if(void);

int audio_process_pa_envelope_if_open(int sample_rate, int channels, int bits, uint32_t half_dma_size, uint32_t envelope_sample_num);

int audio_process_pa_envelope_if_start(void);

int audio_process_pa_envelope_if_close(void);

int audio_process_pa_envelope_if_run(const uint8_t *input, uint32_t len);

int audio_process_pa_envelope_if_set_signal(int delay_time);

int audio_process_pa_envelope_if_stop(void);

uint8_t audio_process_pa_envelope_if_is_enable(void);

void audio_process_pa_envelope_enable(void);

void audio_process_pa_envelope_disable(void);

int audio_process_pa_envelope_set_cfg(audio_process_pa_envelope_cfg_t *cfg);

int audio_process_pa_envelope_get_cfg(audio_process_pa_envelope_cfg_t *cfg);

#ifdef __cplusplus
}
#endif