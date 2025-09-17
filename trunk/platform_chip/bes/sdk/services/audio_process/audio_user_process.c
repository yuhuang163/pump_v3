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
#include "plat_types.h"
#include <string.h>
#include "audio_user_process.h"
#include "hal_trace.h"

static audio_user_process_t audio_user_process = {0};

void audio_user_process_register(audio_user_process_t *h)
{
    if (h) {
        audio_user_process.close = h->close;
        audio_user_process.open  = h->open;
        audio_user_process.run   = h->run;
    }
}

void audio_user_process_unregister(void)
{
    memset(&audio_user_process, 0, sizeof(audio_user_process_t));
}

int audio_user_process_open(int sample_rate, int in_channels, int out_channels, int in_bits, int out_bits, uint32_t timeout)
{
    if (audio_user_process.open) {
        return audio_user_process.open(sample_rate, in_channels, out_channels, in_bits, out_bits, timeout);
    }

    return -1;
}

int audio_user_process_close(void)
{
    if (audio_user_process.close) {
        return audio_user_process.close();
    }

    return -1;
}

int audio_user_process_run(uint8_t *data, uint32_t len)
{
    if (audio_user_process.run) {
        return audio_user_process.run(data, len);
    }

    return -1;
}

bool audio_user_process_is_enable(void)
{
    if (audio_user_process.run) {
        return true;
    } else {
        return false;
    }
}

static audio_user_sw_gain_process_t audio_user_sw_gain_process = {0};

void audio_user_sw_gain_process_register(audio_user_sw_gain_process_t *h)
{
    if (h) {
        audio_user_sw_gain_process.open     = h->open;
        audio_user_sw_gain_process.process  = h->process;
        audio_user_sw_gain_process.close    = h->close;
        audio_user_sw_gain_process.fade_in  = h->fade_in;
        audio_user_sw_gain_process.fade_out = h->fade_out;
    }
}
void audio_user_sw_gain_process_unregister(void)
{
    memset(&audio_user_sw_gain_process, 0, sizeof(audio_user_sw_gain_process_t));
}

int audio_user_sw_gain_process_open(uint8_t channels, uint8_t bits, uint32_t sample_rate, float f0)
{
    if (audio_user_sw_gain_process.open) {
        audio_user_sw_gain_process.open(channels, bits, sample_rate, f0);
    }
    return 0;
}

int audio_user_sw_gain_process_close(void)
{
    if (audio_user_sw_gain_process.close) {
        audio_user_sw_gain_process.close();
    }
    return 0;
}

int audio_user_sw_gain_process_run(uint8_t *buf, uint32_t len)
{
    if (audio_user_sw_gain_process.process) {
        audio_user_sw_gain_process.process(buf, len);
    }
    return 0;
}

int audio_user_sw_gain_process_fade_in(bool skip_zero_data)
{
    if (audio_user_sw_gain_process.fade_in) {
        return audio_user_sw_gain_process.fade_in(skip_zero_data);
    }
    return 0;
}

int audio_user_sw_gain_process_fade_out(void)
{
    if (audio_user_sw_gain_process.fade_out) {
        return audio_user_sw_gain_process.fade_out();
    }
    return 0;
}

bool audio_user_sw_gain_process_is_enable(void)
{
    if (audio_user_sw_gain_process.process) {
        return true;
    } else {
        return false;
    }
}

static audio_user_dump_t audio_user_dump = {0};

void audio_user_dump_register(audio_user_dump_t *h)
{
    if (h) {
        audio_user_dump.dump_raw_data_config = h->dump_raw_data_config;
        audio_user_dump.dump_raw_data        = h->dump_raw_data;
        audio_user_dump.dump_before_process  = h->dump_before_process;
        audio_user_dump.dump_after_process   = h->dump_after_process;
    }
}

void audio_user_dump_unreister(void)
{
    memset(&audio_user_dump, 0, sizeof(audio_user_dump_t));
}

int audio_user_dump_raw_data_config(uint32_t sample_rate, uint8_t channels, uint8_t bits)
{
    if (audio_user_dump.dump_raw_data_config) {
        return audio_user_dump.dump_raw_data_config(sample_rate, channels, bits);
    }
    return -1;
}

int audio_user_dump_raw_data(uint8_t *data, uint32_t length)
{
    if (audio_user_dump.dump_raw_data) {
        return audio_user_dump.dump_raw_data(data, length);
    }
    return -1;
}

int audio_user_dump_before_process(uint8_t *data, uint32_t length)
{
    if (audio_user_dump.dump_before_process) {
        return audio_user_dump.dump_before_process(data, length);
    }
    return -1;
}

int audio_user_dump_after_process(uint8_t *data, uint32_t length)
{
    if (audio_user_dump.dump_after_process) {
        return audio_user_dump.dump_after_process(data, length);
    }
    return -1;
}

static audio_user_focus_ctrl_t audio_user_focus_ctrl = {0};

void audio_user_focus_ctrl_register(audio_user_focus_ctrl_t *h)
{
    if (h) {
        audio_user_focus_ctrl.acquire = h->acquire;
        audio_user_focus_ctrl.release = h->release;
    }
}

void audio_user_focus_ctrl_unregister(void)
{
    memset(&audio_user_focus_ctrl, 0, sizeof(audio_user_focus_ctrl_t));
}

int audio_user_focus_ctrl_acquire(audio_user_focus_ctrl_type_t type)
{
    if (audio_user_focus_ctrl.acquire) {
        return audio_user_focus_ctrl.acquire(type);
    }
    return -1;
}

int audio_user_focus_ctrl_release(audio_user_focus_ctrl_type_t type)
{
    if (audio_user_focus_ctrl.release) {
        return audio_user_focus_ctrl.release(type);
    }
    return -1;
}