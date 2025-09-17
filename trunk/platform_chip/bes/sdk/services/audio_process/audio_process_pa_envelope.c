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
#include "audio_process_pa_envelope.h"
#include "hal_trace.h"

static audio_process_pa_envelope_if_t pa_envelope_process_if = {0};
static uint8_t audio_process_pa_envelope_enabled = 0;

void audio_process_pa_envelope_register_if(audio_process_pa_envelope_if_t *ift)
{
    if (ift)
    {
        pa_envelope_process_if.open = ift->open;
        pa_envelope_process_if.start = ift->start,
        pa_envelope_process_if.set_signal = ift->set_signal;
        pa_envelope_process_if.run = ift->run;
        pa_envelope_process_if.stop = ift->stop;
        pa_envelope_process_if.close = ift->close;
        pa_envelope_process_if.set_cfg = ift->set_cfg;
        pa_envelope_process_if.get_cfg = ift->get_cfg;
    }
}

void audio_process_pa_envelope_unregister_if(void)
{
    memset(&pa_envelope_process_if, 0, sizeof(audio_process_pa_envelope_if_t));
}

int audio_process_pa_envelope_if_open(int sample_rate, int channels, int bits, uint32_t half_dma_size, uint32_t envelope_sample_num)
{
    if (pa_envelope_process_if.open) {
        return pa_envelope_process_if.open(sample_rate, channels, bits, half_dma_size, envelope_sample_num);
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_if_start(void)
{
    if (pa_envelope_process_if.start) {
        return pa_envelope_process_if.start();
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_if_close(void)
{
    if (pa_envelope_process_if.close) {
        return pa_envelope_process_if.close();
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_if_run(const uint8_t *input, uint32_t len)
{
    if (pa_envelope_process_if.run) {
        return pa_envelope_process_if.run(input, len);
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_if_set_signal(int delay_time)
{
    if (pa_envelope_process_if.set_signal) {
        return pa_envelope_process_if.set_signal(delay_time);
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_if_stop(void)
{
    if (pa_envelope_process_if.stop) {
        return pa_envelope_process_if.stop();
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_set_cfg(audio_process_pa_envelope_cfg_t *cfg)
{
    if (pa_envelope_process_if.set_cfg) {
        return pa_envelope_process_if.set_cfg(cfg);
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

int audio_process_pa_envelope_get_cfg(audio_process_pa_envelope_cfg_t *cfg)
{
    if (pa_envelope_process_if.get_cfg) {
        return pa_envelope_process_if.get_cfg(cfg);
    }

    TRACE(0, "%s not support", __func__);
    return -1;
}

void audio_process_pa_envelope_enable(void)
{
    audio_process_pa_envelope_enabled = 1;
}

void audio_process_pa_envelope_disable(void)
{
    audio_process_pa_envelope_enabled = 0;
}

uint8_t audio_process_pa_envelope_if_is_enable(void)
{
    return audio_process_pa_envelope_enabled;
}