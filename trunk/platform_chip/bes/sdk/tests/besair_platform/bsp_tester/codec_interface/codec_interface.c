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
#include "plat_addr_map.h"
#include "codec_interface.h"
#include "analog.h"
#include "cmsis.h"
#include "codec_int.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "pmu.h"
#include "string.h"
#include "hal_timer.h"
#include "tgt_hardware.h"
#include "hw_codec_iir_process.h"

#ifdef RTOS
#include "cmsis_os.h"
#else
#include "hal_sleep.h"
#endif

#define CODEC_TRACE_DEBUG()   // TRACE(2,"%s:%d\n", __func__, __LINE__)


/* internal use */
enum CODEC_BOOL_T{
    CODEC_FALSE = 0,
    CODEC_TRUE = 1
};

enum CODEC_RESULT_T{
    CODEC_RES_SUCCESS = 0,
    CODEC_RES_FAILD = 1
};


static uint32_t POSSIBLY_UNUSED _get_fast_time_stamp(void)
{
#ifdef TIMER1_BASE
    return hal_fast_sys_timer_get();
#else
    return hal_sys_timer_get();
#endif
}


static uint32_t POSSIBLY_UNUSED _fast_time_to_us(uint32_t ticks)
{
#ifdef TIMER1_BASE
    return FAST_TICKS_TO_US(ticks);
#else
    return TICKS_TO_US(ticks);
#endif
}


// Codec logical link will open.
// Note: Dma logical link not open.
uint32_t codec_stream_open(enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg)
{
    CODEC_TRACE_DEBUG();
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    TRACE(2, "[%s], stream = %d", __func__, stream);

    if (stream == AUD_STREAM_PLAYBACK) {
        // playback open device and stream
        CODEC_TRACE_DEBUG();
        codec_int_open();
        codec_int_stream_open(stream);
    } else {
        // capture open device and stream
        CODEC_TRACE_DEBUG();
        codec_int_open();
        codec_int_stream_open(stream);
    }

    CODEC_TRACE_DEBUG();
    ret = CODEC_RES_SUCCESS;
    if (ret == CODEC_RES_SUCCESS) {
        codec_stream_setup(stream, cfg);
    }

    return ret;
}


//volume, path, sample rate, channel num ...
uint32_t codec_stream_setup(enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;
    struct HAL_CODEC_CONFIG_T codec_cfg;

    TRACE(2,"[%s] stream = %d", __func__, stream);
    CODEC_TRACE_DEBUG();

    memset(&codec_cfg, 0, sizeof(codec_cfg));
    codec_cfg.bits = cfg->bits;
    codec_cfg.sample_rate = cfg->sample_rate;
    codec_cfg.channel_num = cfg->channel_num;
    codec_cfg.channel_map = cfg->channel_map;
    codec_cfg.use_dma = CODEC_TRUE;
    codec_cfg.vol = cfg->vol;
    codec_cfg.io_path = cfg->io_path;

    TRACE(2, "[%s] current vol:%d", __func__, cfg->vol);
    codec_int_stream_setup(stream, &codec_cfg);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


uint32_t codec_stream_mute(enum AUD_STREAM_T stream, bool mute)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    CODEC_TRACE_DEBUG();
    codec_int_stream_mute(stream, mute);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


uint32_t codec_stream_set_chan_vol(enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    CODEC_TRACE_DEBUG();
    codec_int_stream_set_chan_vol(stream, ch_map, vol);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


uint32_t codec_stream_restore_chan_vol(enum AUD_STREAM_T stream)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    CODEC_TRACE_DEBUG();
    codec_int_stream_restore_chan_vol(stream);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


uint32_t codec_stream_start(enum AUD_STREAM_T stream)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    TRACE(2,"[%s] stream = %d", __func__, stream);

    if (stream == AUD_STREAM_PLAYBACK) {
    }

    CODEC_TRACE_DEBUG();
    codec_int_stream_start(stream);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;

    return ret;
}


uint32_t codec_stream_stop( enum AUD_STREAM_T stream)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;
    TRACE(2,"[%s] stream = %d", __func__, stream);

    codec_int_stream_stop(stream);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


uint32_t codec_stream_pause(enum AUD_STREAM_T stream)
{
    return codec_stream_stop(stream);
}


uint32_t codec_stream_restart(enum AUD_STREAM_T stream)
{
    return codec_stream_start(stream);
}


uint32_t codec_stream_close(enum AUD_STREAM_T stream)
{
    enum CODEC_RESULT_T ret = CODEC_RES_FAILD;

    TRACE(3,"[%s] stream = %d", __func__, stream);

    codec_int_stream_close(stream);
    codec_int_close(CODEC_CLOSE_NORMAL);
    CODEC_TRACE_DEBUG();

    ret = CODEC_RES_SUCCESS;
    return ret;
}


void codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    codec_direct_tune_resample_rate(stream, ratio);
}


void codec_direct_tune_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream < AUD_STREAM_NUM, "[%s] Bad stream=%d", __func__, stream);

    hal_codec_tune_resample_rate(stream, ratio);
}


void codec_tune_both_resample_rate(float ratio)
{
    hal_codec_tune_both_resample_rate(ratio);
}


void codec_tune_pll(float ratio)
{
    analog_aud_pll_tune(ratio);
}


void codec_tune_xtal(float ratio)
{
    analog_aud_xtal_tune(ratio);
}


void codec_tune(enum AUD_STREAM_T stream, float ratio)
{
    codec_direct_tune(stream, ratio);
}


void codec_direct_tune(enum AUD_STREAM_T stream, float ratio)
{
    ASSERT(stream <= AUD_STREAM_NUM, "[%s] Bad stream=%d", __func__, stream);

#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        if (stream < AUD_STREAM_NUM) {
            hal_codec_tune_resample_rate(stream, ratio);
        } else {
            hal_codec_tune_both_resample_rate(ratio);
        }
    } else
#endif
    {
        analog_aud_pll_tune(ratio);
    }
}


void codec_set_perf_test_power(int type)
{
    hal_codec_dac_gain_m60db_check((enum HAL_CODEC_PERF_TEST_POWER_T)type);
}


void codec_set_noise_reduction(bool enable)
{
    hal_codec_set_noise_reduction(enable);
}


void codec_bt_trigger_config(bool en, CODEC_BT_TRIGGER_CALLBACK callback)
{
    if (en) {
        hal_codec_set_bt_trigger_callback(callback);
        hal_codec_bt_trigger_start();
    } else {
        hal_codec_bt_trigger_stop();
    }
}


void codec_set_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src)
{
#ifndef CHIP_BEST1000
    if (stream == AUD_STREAM_PLAYBACK) {
        hal_codec_set_dac_bt_sync_source(src);
    } else {
        hal_codec_set_adc_bt_sync_source(src);
    }
#endif
}


void codec_sync_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_disable();
        }
    }
#endif
}


void codec_sync_resample_rate_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_resample_rate_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_resample_rate_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_resample_rate_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_resample_rate_disable();
        }
    }
#endif
}


void codec_sync_gain_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable)
{
#ifndef CHIP_BEST1000
    if (stream == AUD_STREAM_PLAYBACK) {
        if (enable) {
            hal_codec_sync_dac_gain_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_dac_gain_disable();
        }
    } else {
        if (enable) {
            hal_codec_sync_adc_gain_enable((enum HAL_CODEC_SYNC_TYPE_T)type);
        } else {
            hal_codec_sync_adc_gain_disable();
        }
    }
#endif
}


void codec_set_device_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src)
{
    codec_set_bt_sync_source(stream, src);
}


void codec_sync_device_config(enum AUD_STREAM_T stream, enum CODEC_SYNC_TYPE_T type, bool enable)
{
    codec_sync_config(stream, type, enable);
}


void codec_swap_output(bool swap)
{
    hal_codec_swap_output(swap);
}


void codec_min_phase_mode_enable(enum AUD_STREAM_T stream)
{
    hal_codec_min_phase_mode_enable(stream);
}


void codec_min_phase_mode_disable(enum AUD_STREAM_T stream)
{
    hal_codec_min_phase_mode_disable(stream);
}

/* hw dac iir eq demo */
#ifdef HW_DAC_IIR_EQ
#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif
extern const IIR_CFG_T * const audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM];

void eq_start_demo(enum AUD_SAMPRATE_T sample_rate)
{
    HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;
    enum AUD_SAMPRATE_T sample_rate_hw_dac_iir = sample_rate;
    const IIR_CFG_T *iir_cfg = audio_eq_hw_dac_iir_cfg_list[0];

    hw_codec_iir_open(sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC, CODEC_OUTPUT_DEV);

    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, iir_cfg);
    ASSERT(hw_iir_cfg_dac != NULL, "[%s] %d codec IIR parameter error!", __func__, (uint32_t)hw_iir_cfg_dac);

    hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
}
#endif
/* end hw dac iir eq demo */

// demo code
#if HIFI4_ADDA_LOOP_TEST

#define ADDA_SAMPLE_RATE                16000
#define ADDA_SAMPLE_SIZE                2

#define ADDA_PLAYBACK_CHAN              2
#define ADDA_CAPTURE_CHAN               7

static enum AUD_BITS_T sample_size_to_enum(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        return AUD_BITS_24;
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __FUNCTION__, size);
    }

    return 0;
}

static enum AUD_CHANNEL_NUM_T chan_num_to_enum(uint32_t num)
{
    if (num == 8) {
        return AUD_CHANNEL_NUM_8;
    } else if (num == 7) {
        return AUD_CHANNEL_NUM_7;
    } else if (num == 6) {
        return AUD_CHANNEL_NUM_6;
    } else if (num == 5) {
        return AUD_CHANNEL_NUM_5;
    } else if (num == 4) {
        return AUD_CHANNEL_NUM_4;
    } else if (num == 3) {
        return AUD_CHANNEL_NUM_3;
    } else if (num == 2) {
        return AUD_CHANNEL_NUM_2;
    } else if (num == 1) {
        return AUD_CHANNEL_NUM_1;
    } else {
        ASSERT(false, "%s: Invalid channel num: %u", __FUNCTION__, num);
    }

    return 0;
}

static void POSSIBLY_UNUSED sys_domain_dma_to_codec(bool enable)
{
    uint32_t reg_addr  = 0x4008004c;
    uint32_t value = 0;

    value = DIGITAL_REG(reg_addr);
    if (enable) {
        value |= (1 << 9);
    } else {
        value |= (1 << 8);
    }
    DIGITAL_REG(reg_addr) = value;

    TRACE(0, "%s: codec reg[%x] = %x", __func__, reg_addr, value);
}

static void POSSIBLY_UNUSED sys_domain_clk_to_codec(bool enable)
{
    uint32_t value    = 0;
    uint32_t reg_addr = 0x4008001c;

    value = DIGITAL_REG(reg_addr);
    if (enable) {
        value |= (1 << 23);
    } else {
        value &= ~(1 << 23);
    }
    DIGITAL_REG(reg_addr) = value;
    TRACE(0, "%s:write codec reg[%x] = %x", __func__, reg_addr, value);
}

void adda_loop_start_demo(void)
{
    int ret = 0;
    struct HAL_CODEC_CONFIG_T codec_cfg;
    memset(&codec_cfg, 0, sizeof(codec_cfg));

    codec_cfg.bits = sample_size_to_enum(ADDA_SAMPLE_SIZE);
    codec_cfg.sample_rate = ADDA_SAMPLE_RATE;
    codec_cfg.channel_num = chan_num_to_enum(ADDA_PLAYBACK_CHAN);
    codec_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    codec_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    codec_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
    ret = codec_stream_open(AUD_STREAM_PLAYBACK, &codec_cfg);
    ASSERT(ret == 0, "codec_stream_open playback failed: %d", ret);

    memset(&codec_cfg, 0, sizeof(codec_cfg));
    codec_cfg.bits = sample_size_to_enum(ADDA_SAMPLE_SIZE);
    codec_cfg.sample_rate = ADDA_SAMPLE_RATE;
    codec_cfg.channel_num = chan_num_to_enum(ADDA_CAPTURE_CHAN);
    codec_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1
                            | AUD_CHANNEL_MAP_CH2 | AUD_CHANNEL_MAP_CH3
                            | AUD_CHANNEL_MAP_CH4 | AUD_CHANNEL_MAP_CH5
                        | AUD_CHANNEL_MAP_ECMIC_CH0;// | AUD_CHANNEL_MAP_ECMIC_CH1;
    // codec_cfg.channel_map = AUD_CHANNEL_MAP_DIGMIC_CH2 | AUD_CHANNEL_MAP_DIGMIC_CH3;
    codec_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    codec_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    ret = codec_stream_open(AUD_STREAM_CAPTURE, &codec_cfg);
    ASSERT(ret == 0, "codec_stream_open catpure failed: %d", ret);

#ifdef HW_DAC_IIR_EQ
    eq_start_demo(codec_cfg.sample_rate);
#endif

    ret = codec_stream_start(AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "codec_stream_start playback failed: %d", ret);

    ret = codec_stream_start(AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "codec_stream_start catpure failed: %d", ret);

    // select sys domain clk for the codec clk source.
    sys_domain_clk_to_codec(false);

    // sys domain uses dma and codec to transmitd data.
    sys_domain_dma_to_codec(true);
}
#endif