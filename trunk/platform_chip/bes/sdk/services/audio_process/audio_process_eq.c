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
#include "hal_trace.h"
#include "tgt_hardware.h"
#include "string.h"
#include "audio_process_eq.h"
#include "hw_codec_iir_process.h"
#include "hw_iir_process.h"
#include "hal_codec.h"

// Enable this to test AUDIO_CUSTOM_EQ with audiotools,
// eq config from audiotools will be merge with default eq config.
// #define TEST_AUDIO_CUSTOM_EQ

#if defined(AUDIO_CUSTOM_EQ)
#include "heap_api.h"
#include "ae_math.h"
#endif

#if defined(USB_EQ_TUNING)
#if !defined(__HW_DAC_IIR_EQ_PROCESS__) && !defined(__SW_IIR_EQ_PROCESS__)
#error "Either HW_DAC_IIR_EQ_PROCESS or SW_IIR_EQ_PROCESS should be defined when enabling USB_EQ_TUNING"
#endif
#endif

#if defined(AUDIO_EQ_TUNING)
#include "hal_cmd.h"

#if defined(__SW_IIR_EQ_PROCESS__)
#define AUDIO_EQ_SW_IIR_UPDATE_CFG
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
#define AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
#define AUDIO_EQ_HW_IIR_UPDATE_CFG
#endif

#ifdef __HW_FIR_EQ_PROCESS__
#define AUDIO_EQ_HW_FIR_UPDATE_CFG
#endif
#endif

#if defined(__SW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[];
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
extern const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[];
#if defined(__HW_FIR_EQ_PROCESS_2CH__)
extern const FIR_CFG_T * const audio_hw_hpfir_cfg_list[];
extern const FIR_CFG_T * const audio_hw_lpfir_cfg_list[];
#endif
#if defined(AUDIO_ADAPTIVE_FIR_EQ)
extern FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg;
#endif
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const audio_eq_hw_dac_iir_cfg_list[];
#if defined(AUDIO_ADAPTIVE_IIR_EQ)
extern IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg;
#endif
#if defined(AUDIO_EQ_MC)
extern IIR_CFG_T g_audio_eq_mc_cfg_running;
#endif
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const audio_eq_hw_iir_cfg_list[];
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) || defined(AUDIO_EQ_HW_FIR_UPDATE_CFG)|| defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)|| defined(AUDIO_EQ_HW_IIR_UPDATE_CFG)
#define AUDIO_UPDATE_CFG
#endif

#ifdef __AUDIO_ADJ_EQ__
#define AUDIO_ADJ_EQ_NEEDED_SIZE (1024*10)
extern void get_anc_assist_cfg(int *frame_size, int *sample_rate);
#include "adj_eq.h"
#else
#define AUDIO_ADJ_EQ_NEEDED_SIZE (0)
#endif

#ifdef AUDIO_HEARING_COMPSATN
extern float get_hear_compen_val_fir(int *test_buf, float *gain_buf, int num);
extern float get_hear_compen_val_multi_level(int *test_buf, float *gain_buf, int num);
extern short get_hear_level(void);
#include "hear_fir2.h"
#include "speech_memory.h"
#define HEAR_COMP_BUF_SIZE    1024*17
#ifdef HEARING_USE_STATIC_RAM
extern char HEAR_DET_STREAM_BUF[HEAR_COMP_BUF_SIZE];
#else
uint8_t *hear_comp_buf = NULL;
#endif

//int band_std[6] = {14,10,10,8,9,18};
#if HWFIR_MOD==HEARING_MOD_VAL
float fir_filter[384] = {0.0F};
#else
float hear_iir_coef[30] = {0.0F};
#endif
#define HWFIR_MOD   0
#define SWIIR_MOD   1
#define HWIIR_MOD   2
#endif

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif

typedef signed int          pcm_24bits_t;
typedef signed short int    pcm_16bits_t;

typedef struct{
    enum AUD_BITS_T         sample_bits;
    enum AUD_SAMPRATE_T     sample_rate;
    enum AUD_CHANNEL_NUM_T  sw_ch_num;
    enum AUD_CHANNEL_NUM_T  hw_ch_num;

#if defined(__SW_IIR_EQ_PROCESS__)
    bool sw_iir_enable;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    bool hw_dac_iir_enable;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
    bool hw_iir_enable;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
    bool hw_fir_enable;
#endif

#ifdef AUDIO_UPDATE_CFG
    bool update_cfg;
#endif

#ifdef USB_EQ_TUNING
	bool eq_updated_cfg;
#endif

#ifdef AUDIO_HEARING_COMPSATN
    //bool hear_updated_cfg;
#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
        IIR_CFG_T hear_iir_cfg;
        IIR_CFG_T hear_iir_cfg_update;
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
        FIR_CFG_T hear_hw_fir_cfg;
#endif
#endif
#endif

#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    IIR_CFG_T sw_iir_cfg;
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    IIR_CFG_T hw_dac_iir_cfg;
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    IIR_CFG_T hw_iir_cfg;
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    FIR_CFG_T hw_fir_cfg;
#endif

#ifdef AUDIO_CUSTOM_EQ
    uint32_t nfft;
    float *work_buffer;
#endif
} AUDIO_PROCESS_EQ_T;

static AUDIO_PROCESS_EQ_T audio_eq = {
#if defined(__SW_IIR_EQ_PROCESS__)
    .sw_iir_enable =  false,
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    .hw_dac_iir_enable =  false,
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
    .hw_iir_enable =  false,
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
    .hw_fir_enable =  false,
#endif

#ifdef AUDIO_HEARING_COMPSATN
#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
    .hear_iir_cfg = {.num = 0},
    .hear_iir_cfg_update = {.num = 0},
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    .hear_hw_fir_cfg = {.len = 0},
#endif
#endif
#endif

#ifdef AUDIO_UPDATE_CFG
    .update_cfg = false,
#endif

#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    .sw_iir_cfg = {.num = 0},
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    .hw_dac_iir_cfg = {.num = 0},
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    .hw_iir_cfg = {.num = 0},
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    .hw_fir_cfg = {.len = 0},
#endif
};

static uint8_t g_eq_index[AUDIO_EQ_TYPE_QTY] = {0, 0, 0, 0};

uint32_t audio_process_eq_get_index(AUDIO_EQ_TYPE_T type)
{
    return g_eq_index[type];
}

int32_t audio_process_eq_set_index(AUDIO_EQ_TYPE_T type, uint32_t index)
{
    ASSERT(type < AUDIO_EQ_TYPE_QTY, "[%s] type is invalid: %d", __func__, type);

#ifdef __SW_IIR_EQ_PROCESS__
    if (index >= EQ_SW_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_FIR_EQ_PROCESS__
    if (index >= EQ_HW_FIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
    if (index >= EQ_HW_DAC_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_IIR_EQ_PROCESS__
    if (index >= EQ_HW_IIR_LIST_NUM)
        return -1;
#endif

    g_eq_index[type] = index;

    return index;
}

void audio_process_iir_eq_mix(IIR_CFG_T *iir_cfg_mix)
{
#if defined(AUDIO_ADAPTIVE_IIR_EQ)
    iir_cfg_mix->gain0 += audio_eq_hw_dac_iir_adaptive_eq_cfg.gain0;
    iir_cfg_mix->gain1 += audio_eq_hw_dac_iir_adaptive_eq_cfg.gain1;
    ASSERT(iir_cfg_mix->num + audio_eq_hw_dac_iir_adaptive_eq_cfg.num <= IIR_PARAM_NUM, "[%s] %d exceeds maximum EQ band number!",
                                                        __func__, iir_cfg_mix->num + audio_eq_hw_dac_iir_adaptive_eq_cfg.num);
    for(int i=0; i<audio_eq_hw_dac_iir_adaptive_eq_cfg.num; i++){
        iir_cfg_mix->param[i+iir_cfg_mix->num] = audio_eq_hw_dac_iir_adaptive_eq_cfg.param[i];
    }
    iir_cfg_mix->num += audio_eq_hw_dac_iir_adaptive_eq_cfg.num;
#endif

#if defined(AUDIO_EQ_MC)
    iir_cfg_mix->gain0 += g_audio_eq_mc_cfg_running.gain0;
    iir_cfg_mix->gain1 += g_audio_eq_mc_cfg_running.gain1;
    ASSERT(iir_cfg_mix->num + g_audio_eq_mc_cfg_running.num <= IIR_PARAM_NUM, "[%s] %d exceeds maximum EQ band number!",
                                                        __func__, iir_cfg_mix->num + g_audio_eq_mc_cfg_running.num);
    for(int i=0; i<g_audio_eq_mc_cfg_running.num; i++){
        iir_cfg_mix->param[i+iir_cfg_mix->num] = g_audio_eq_mc_cfg_running.param[i];
    }
    iir_cfg_mix->num += g_audio_eq_mc_cfg_running.num;
#endif
}

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const FIR_CFG_T *fir_cfg_2,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type)
#else
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type)
#endif
{
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(iir_cfg)
            {
                audio_eq.sw_iir_enable = false;
#ifdef USB_EQ_TUNING
                if (audio_eq.eq_updated_cfg) {
                    iir_set_cfg(&audio_eq.sw_iir_cfg);
                } else
#endif
                {
#if defined(AUDIO_HEARING_COMPSATN) && SWIIR_MOD==HEARING_MOD_VAL
                    iir_set_cfg(&audio_eq.hear_iir_cfg_update);
#elif defined(AUDIO_ADAPTIVE_EQ) || defined(AUDIO_EQ_MC)
                    IIR_CFG_T iir_cfg_mix;
                    memcpy(&iir_cfg_mix, iir_cfg, sizeof(IIR_CFG_T));
                    audio_process_iir_eq_mix(&iir_cfg_mix);
                    iir_set_cfg(&iir_cfg_mix);
#else
                    iir_set_cfg(iir_cfg);
#endif
                }
                audio_eq.sw_iir_enable = true;
            }
            else
            {
                audio_eq.sw_iir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(fir_cfg)
            {
                audio_eq.hw_fir_enable = false;
#if defined(AUDIO_HEARING_COMPSATN) && HWFIR_MOD==HEARING_MOD_VAL
                fir_set_cfg(&audio_eq.hear_hw_fir_cfg);
#elif __HW_FIR_EQ_PROCESS_2CH__
                fir_set_cfg_ch(fir_cfg,AUD_CHANNEL_NUM_1);
                fir_set_cfg_ch(fir_cfg_2,AUD_CHANNEL_NUM_2);
#else

                fir_set_cfg(fir_cfg);
#endif
                audio_eq.hw_fir_enable = true;
            }
            else
            {
                audio_eq.hw_fir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(iir_cfg)
            {
                HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac=NULL;
                enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
                sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_eq.sample_rate,1);
                TRACE(3,"audio_eq.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_eq.sample_rate, sample_rate_hw_dac_iir);
#else
                sample_rate_hw_dac_iir=audio_eq.sample_rate;
#endif
                audio_eq.hw_dac_iir_enable = false;
#ifdef USB_EQ_TUNING
                if (audio_eq.eq_updated_cfg) {
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &audio_eq.hw_dac_iir_cfg);
                } else
#endif
                {
#if defined(AUDIO_HEARING_COMPSATN) && HWIIR_MOD==HEARING_MOD_VAL
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_eq.hear_iir_cfg_update);
#else
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,iir_cfg);
#endif
#if defined(AUDIO_ADAPTIVE_EQ) || defined(AUDIO_EQ_MC)
                    IIR_CFG_T iir_cfg_mix;
                    memcpy(&iir_cfg_mix, iir_cfg, sizeof(IIR_CFG_T));
                    audio_process_iir_eq_mix(&iir_cfg_mix);
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &iir_cfg_mix);
#endif
                }
                ASSERT(hw_iir_cfg_dac != NULL, "[%s] codec IIR parameter error!", __func__);
                hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
                audio_eq.hw_dac_iir_enable = true;
            }
            else
            {
                audio_eq.hw_dac_iir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(iir_cfg)
            {
                HW_IIR_CFG_T *hw_iir_cfg=NULL;
                audio_eq.hw_iir_enable = false;
                hw_iir_cfg = hw_iir_get_cfg(audio_eq.sample_rate,iir_cfg);
                ASSERT(hw_iir_cfg != NULL,"[%s] 0x%x codec IIR parameter error!", __func__, (unsigned int)hw_iir_cfg);
                hw_iir_set_cfg(hw_iir_cfg);
                audio_eq.hw_iir_enable = true;
            }
            else
            {
                audio_eq.hw_iir_enable = false;
            }

        }
        break;
#endif

        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif

    return 0;
}

#ifdef AUDIO_EQ_MC
IIR_CFG_T g_audio_eq_mc_cfg_running;
uint32_t bt_audio_set_eq_mc_cfg(IIR_CFG_T *iir_cfg)
{
    TRACE(0, "[%s]", __func__);
    memcpy(&g_audio_eq_mc_cfg_running, iir_cfg, sizeof(IIR_CFG_T));
#if BLE_AUDIO_ENABLED
    if (amgr_is_bt_a2dp_on() || is_le_media_mode()) {
#else
    if (amgr_is_bt_a2dp_on()) {
#endif
#ifdef __SW_IIR_EQ_PROCESS__
        audio_eq_set_cfg(NULL, audio_eq_sw_iir_cfg_list[audio_process_eq_get_index(AUDIO_EQ_TYPE_SW_IIR)], AUDIO_EQ_TYPE_SW_IIR);
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
        audio_eq_set_cfg(NULL, audio_eq_hw_dac_iir_cfg_list[audio_process_eq_get_index(AUDIO_EQ_TYPE_HW_DAC_IIR)], AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
    } else {
        TRACE(0, "[%s] bt_a2dp_is_not_run !", __func__);
    }

    return 0;
}
#endif

#ifdef AUDIO_ADAPTIVE_IIR_EQ
uint32_t bt_audio_set_adaptive_dac_iir_eq(IIR_CFG_T *iir_cfg)
{
        memcpy(&audio_eq_hw_dac_iir_adaptive_eq_cfg, iir_cfg, sizeof(IIR_CFG_T));
#if defined(__SW_IIR_EQ_PROCESS__)
        audio_eq_set_cfg(NULL, audio_eq_sw_iir_cfg_list[audio_process_eq_get_index(AUDIO_EQ_TYPE_SW_IIR)], AUDIO_EQ_TYPE_SW_IIR);
#endif
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        audio_eq_set_cfg(NULL, audio_eq_hw_dac_iir_cfg_list[audio_process_eq_get_index(AUDIO_EQ_TYPE_HW_DAC_IIR)], AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif

    return 0;
}
#endif

#ifdef AUDIO_ADAPTIVE_FIR_EQ
uint32_t bt_audio_set_adaptive_fir_eq(FIR_CFG_T *fir_cfg)
{
#if defined(__HW_FIR_EQ_PROCESS__)
    audio_eq_set_cfg(fir_cfg, NULL, AUDIO_EQ_TYPE_HW_FIR);
#endif
    return 0;
}
#endif

uint32_t bt_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type, uint8_t index)
{
    const FIR_CFG_T *fir_cfg=NULL;
    const IIR_CFG_T *iir_cfg=NULL;

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
    const FIR_CFG_T *fir_cfg_2 = NULL;
#endif

    TRACE(0, "[EQ] set type=%d,index=%d",  audio_eq_type,index);

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(index >= EQ_SW_IIR_LIST_NUM)
            {
                TRACE(0, "[EQ] SET index %u > EQ_SW_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_sw_iir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(index >= EQ_HW_FIR_LIST_NUM)
            {
                TRACE(0, "[EQ] SET index %u > EQ_HW_FIR_LIST_NUM", index);
                return 1;
            }

        #if defined(__HW_FIR_EQ_PROCESS_2CH__)
            fir_cfg=audio_hw_hpfir_cfg_list[index];
            fir_cfg_2=audio_hw_lpfir_cfg_list[index];
        #else
            fir_cfg=audio_eq_hw_fir_cfg_list[index];
        #endif
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(index >= EQ_HW_DAC_IIR_LIST_NUM)
            {
                TRACE(0, "[EQ] SET index %u > EQ_HW_DAC_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_hw_dac_iir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(index >= EQ_HW_IIR_LIST_NUM)
            {
                TRACE(0, "[EQ] SET index %u > EQ_HW_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_hw_iir_cfg_list[index];
        }
        break;
#endif
        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif

#ifdef AUDIO_SECTION_ENABLE
    const IIR_CFG_T *iir_cfg_from_audio_section = (const IIR_CFG_T *)load_audio_cfg_from_audio_section(AUDIO_PROCESS_TYPE_IIR_EQ);
    if (iir_cfg_from_audio_section)
    {
        iir_cfg = iir_cfg_from_audio_section;
    }
#endif

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
    return audio_eq_set_cfg(fir_cfg,fir_cfg_2,iir_cfg,audio_eq_type);
#else
    return audio_eq_set_cfg(fir_cfg,iir_cfg,audio_eq_type);
#endif
}

int audio_process_eq_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits,enum AUD_CHANNEL_NUM_T sw_ch_num, enum AUD_CHANNEL_NUM_T hw_ch_num,int32_t frame_size, void *eq_buf, uint32_t len)
{
    TRACE(5,"[%s] sample_rate = %d, sample_bits = %d, sw_ch_num = %d, hw_ch_num = %d", __func__, sample_rate, sample_bits, sw_ch_num, hw_ch_num);

    audio_eq.sample_rate = sample_rate;
    audio_eq.sample_bits = sample_bits;
    audio_eq.sw_ch_num = sw_ch_num;
    audio_eq.hw_ch_num = hw_ch_num;

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
    void *fir_eq_buf = eq_buf;
    uint32_t fir_len = len/2;
    void *iir_eq_buf = (uint8_t *)eq_buf+fir_len;
    uint32_t iir_len = len/2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)
    void *fir_eq_buf = eq_buf;
    uint32_t fir_len = len;
#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
    void *iir_eq_buf = eq_buf;
    uint32_t iir_len = len;
#endif

#ifdef __SW_IIR_EQ_PROCESS__
    iir_open(sample_rate, sample_bits,sw_ch_num);
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    audio_eq_set_cfg(NULL, &audio_eq.sw_iir_cfg, AUDIO_EQ_TYPE_SW_IIR);
#endif
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
    sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_eq.sample_rate,1);
   TRACE(3,"audio_eq.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_eq.sample_rate, sample_rate_hw_dac_iir);
#else
    sample_rate_hw_dac_iir = audio_eq.sample_rate;
#endif

    hw_codec_iir_open(sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC, CODEC_OUTPUT_DEV);
#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    audio_eq_set_cfg(NULL, &audio_eq.hw_dac_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    hw_iir_open(sample_rate, sample_bits, sw_ch_num, iir_eq_buf, iir_len);
#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    audio_eq_set_cfg(NULL, &audio_eq.hw_iir_cfg, AUDIO_EQ_TYPE_HW_IIR);
#endif
#endif

#ifdef __HW_FIR_EQ_PROCESS__
#if defined(CHIP_BEST1000) && defined(FIR_HIGHSPEED)
    hal_cmu_fir_high_speed_enable(HAL_CMU_FIR_USER_EQ);
#endif
    fir_open(sample_rate, sample_bits, sw_ch_num, fir_eq_buf, fir_len);
#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    audio_eq_set_cfg(&audio_eq.hw_fir_cfg, NULL, AUDIO_EQ_TYPE_HW_FIR);
#endif
#endif

#ifdef AUDIO_CUSTOM_EQ
    audio_eq.nfft = 1024;
    syspool_get_buff((uint8_t **)&audio_eq.work_buffer, iir_cfg_find_peak_needed_size(IIR_PARAM_NUM, audio_eq.nfft));
#endif

#if defined(__AUDIO_ADJ_EQ__)
    int anc_assist_frame, anc_assist_sample_rate;
    get_anc_assist_cfg(&anc_assist_frame, &anc_assist_sample_rate);
    adj_eq_init(anc_assist_frame/4, frame_size, anc_assist_sample_rate/4, sample_rate);
#endif

    return 0;
}

int audio_peocess_eq_set_eq(void)
{
// disable audio eq config on a2dp start for audio tuning tools
#ifndef AUDIO_EQ_TUNING
#ifdef __SW_IIR_EQ_PROCESS__
    bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR, audio_process_eq_get_index(AUDIO_EQ_TYPE_SW_IIR));
#endif
#ifdef __HW_FIR_EQ_PROCESS__
    bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR, audio_process_eq_get_index(AUDIO_EQ_TYPE_HW_FIR));
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
    bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR, audio_process_eq_get_index(AUDIO_EQ_TYPE_HW_DAC_IIR));
#endif
#ifdef __HW_IIR_EQ_PROCESS__
    bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR, audio_process_eq_get_index(AUDIO_EQ_TYPE_HW_IIR));
#endif
#endif  // #ifndef AUDIO_EQ_TUNING

    return 0;
}

int audio_process_eq_close(void)
{
#if defined(__AUDIO_ADJ_EQ__)
     adj_eq_deinit();
#endif

#ifdef __SW_IIR_EQ_PROCESS__
    audio_eq.sw_iir_enable = false;
    iir_close();
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    audio_eq.hw_dac_iir_enable = false;
    hw_codec_iir_close(HW_CODEC_IIR_DAC);
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    audio_eq.hw_iir_enable = false;
    hw_iir_close();
#endif

#ifdef __HW_FIR_EQ_PROCESS__
    audio_eq.hw_fir_enable = false;
    fir_close();
#if defined(CHIP_BEST1000) && defined(FIR_HIGHSPEED)
    hal_cmu_fir_high_speed_disable(HAL_CMU_FIR_USER_EQ);
#endif
#endif

    return 0;
}

#ifdef AUDIO_PROCESS_IN_CP
SRAM_TEXT_LOC
#endif
int audio_process_eq_run(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = 0;

    if(audio_eq.sample_bits == AUD_BITS_16)
    {
        pcm_len = len / sizeof(pcm_16bits_t);
    }
    else if(audio_eq.sample_bits == AUD_BITS_24)
    {
        pcm_len = len / sizeof(pcm_24bits_t);
    }
    else
    {
        ASSERT(0, "[%s] bits(%d) is invalid", __func__, audio_eq.sample_bits);
    }


#ifdef __SW_IIR_EQ_PROCESS__
    if(audio_eq.sw_iir_enable)
    {
        iir_run(buf, pcm_len);
    }
#endif

#ifdef __HW_FIR_EQ_PROCESS__
    if(audio_eq.hw_fir_enable)
    {
        fir_run(buf, pcm_len);
    }
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    if(audio_eq.hw_iir_enable)
    {
        hw_iir_run(buf, pcm_len);
    }
#endif

    return 0;
}

#if defined(AUDIO_CUSTOM_EQ)
/*
 * Current merge strategy:
 * Normally master gain should be the negative of max peak of frequency response, but it is not always true.
 * Sometime we want to reduce the overall output power, sometimes we can accept some distortion.
 * We define **HEADROOM** to be the extra gain applied to master gain, that is
 *
 * headroom = -master_gain - design_peak_dB
 *
 * When we merge design eq with custom eq, we keep the same headroom as design eq.
 */
void audio_eq_merge_custom_eq(IIR_CFG_T *cfg, IIR_CFG_T *custom_cfg, const IIR_CFG_T *design_cfg)
{
    if (custom_cfg->num + design_cfg->num > IIR_PARAM_NUM) {
        TRACE(0, "[%s] custom eq num(%d) exceed max avaliable num(%d)",
            __FUNCTION__, custom_cfg->num, IIR_PARAM_NUM - design_cfg->num);
        return;
    }

    memcpy(cfg, design_cfg, sizeof(IIR_CFG_T));

    if (custom_cfg->num == 0) {
        return;
    }

    for (uint8_t i = 0, j = design_cfg->num; i < custom_cfg->num; i++, j++) {
        memcpy(&cfg->param[j], &custom_cfg->param[i], sizeof(IIR_PARAM_T));
    }

    cfg->num += custom_cfg->num;

    enum AUD_SAMPRATE_T fs = audio_eq.sample_rate;
    int32_t nfft = audio_eq.nfft;

    float design_peak_idx = 0;
    float design_peak = iir_cfg_find_peak(audio_eq.sample_rate, design_cfg, nfft, &design_peak_idx, audio_eq.work_buffer);

    float design_headroom = -design_cfg->gain0 - LIN2DB(design_peak);

    TRACE(3, "design idx = %d(e-3), design peak = %d(e-3), f0 = %d(e-3)\n", (int)(design_peak_idx*1000), (int)(design_peak*1000), (int)(design_peak_idx / nfft * fs * 1000));

    float peak_idx = 0;
    float peak = iir_cfg_find_peak(audio_eq.sample_rate, cfg, nfft, &peak_idx, audio_eq.work_buffer);

    TRACE(3, "idx = %d(e-3), peak = %d(e-3), f0 = %d(e-3)\n", (int)(peak_idx*1000), (int)(peak*1000), (int)(peak_idx / nfft * fs * 1000));

    // update master gain
    float peak_dB = LIN2DB(peak);
    cfg->gain0 = -peak_dB - design_headroom;
    cfg->gain1 = -peak_dB - design_headroom;
}
#endif

#if defined(AUDIO_EQ_TUNING)
#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) && !defined(USB_EQ_TUNING)
#ifndef USB_AUDIO_APP
int audio_eq_sw_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

    memcpy(&audio_eq.sw_iir_cfg, buf, sizeof(IIR_CFG_T));
    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_eq.sw_iir_cfg.num,
                                                (int32_t)(audio_eq.sw_iir_cfg.gain0*10),
                                                (int32_t)(audio_eq.sw_iir_cfg.gain1*10));
    for (uint8_t i = 0; i<audio_eq.sw_iir_cfg.num; i++){
        TRACE(5,"band num:%d type %d gain:%d fc:%d q:%d", i,
			                           (int32_t)(audio_eq.sw_iir_cfg.param[i].type),
                                                (int32_t)(audio_eq.sw_iir_cfg.param[i].gain*10),
                                                (int32_t)(audio_eq.sw_iir_cfg.param[i].fc*10),
                                                (int32_t)(audio_eq.sw_iir_cfg.param[i].Q*10));
    }

#ifdef __SW_IIR_EQ_PROCESS__
    {
        iir_set_cfg(&audio_eq.sw_iir_cfg);
        audio_eq.sw_iir_enable = true;
    }
#endif

    return 0;
}
#else
int audio_eq_sw_iir_callback(uint8_t *buf, uint32_t  len)
{
    // IIR_CFG_T *rx_iir_cfg = NULL;

    // rx_iir_cfg = (IIR_CFG_T *)buf;

    // TRACE(3,"[%s] left gain = %f, right gain = %f", __func__, rx_iir_cfg->gain0, rx_iir_cfg->gain1);

    // for(int i=0; i<rx_iir_cfg->num; i++)
    // {
    //     TRACE(5,"[%s] iir[%d] gain = %f, f = %f, Q = %f", __func__, i, rx_iir_cfg->param[i].gain, rx_iir_cfg->param[i].fc, rx_iir_cfg->param[i].Q);
    // }

    // audio_eq_set_cfg(NULL,(const IIR_CFG_T *)rx_iir_cfg,AUDIO_EQ_TYPE_SW_IIR);

    iir_update_cfg_tbl(buf, len);

    return 0;
}
#endif
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
int audio_eq_hw_fir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(FIR_CFG_T));

    if (len != sizeof(FIR_CFG_T))
    {
        return 1;
    }

    FIR_CFG_T *rx_fir_cfg = NULL;

    rx_fir_cfg = (FIR_CFG_T *)buf;

    TRACE(3,"[%s] left gain = %d, right gain = %d", __func__, rx_fir_cfg->gain0, rx_fir_cfg->gain1);

    TRACE(6,"[%s] len = %d, coef: %d, %d......%d, %d", __func__, rx_fir_cfg->len, rx_fir_cfg->coef[0], rx_fir_cfg->coef[1], rx_fir_cfg->coef[rx_fir_cfg->len-2], rx_fir_cfg->coef[rx_fir_cfg->len-1]);

    rx_fir_cfg->gain0 = 6;
    rx_fir_cfg->gain1 = 6;

    if(rx_fir_cfg)
    {
        memcpy(&audio_eq.fir_cfg, rx_fir_cfg, sizeof(audio_eq.fir_cfg));
        audio_eq.fir_enable = true;
        fir_set_cfg(&audio_eq.fir_cfg);
    }
    else
    {
        audio_eq.fir_enable = false;
    }

    return 0;
}
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG) && !defined(USB_EQ_TUNING)
int audio_eq_hw_dac_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

#if defined(TEST_AUDIO_CUSTOM_EQ)
    audio_eq_merge_custom_eq(&audio_eq.hw_dac_iir_cfg, (IIR_CFG_T *)buf, audio_eq_hw_dac_iir_cfg_list[0]);
#else
    memcpy(&audio_eq.hw_dac_iir_cfg, buf, sizeof(IIR_CFG_T));
#endif

    TRACE(0, "num: %d, gain0: %d, gain1: %d",
                                                (int32_t)audio_eq.hw_dac_iir_cfg.num,
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.gain0*10),
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.gain1*10));
    for (uint8_t i = 0; i<audio_eq.hw_dac_iir_cfg.num; i++){
        TRACE(0, "[%d] type: %d, gain: %d, fc: %d, q: %d", i,
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.param[i].type),
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.param[i].gain*10),
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.param[i].fc*10),
                                                (int32_t)(audio_eq.hw_dac_iir_cfg.param[i].Q*10));
    }

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    {
        HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;
        enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
        sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_eq.sample_rate,1);
        TRACE(3,"audio_eq.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_eq.sample_rate, sample_rate_hw_dac_iir);
#else
        sample_rate_hw_dac_iir = audio_eq.sample_rate;
#endif
        hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_eq.hw_dac_iir_cfg);
        ASSERT(hw_iir_cfg_dac != NULL, "[%s] %d codec IIR parameter error!", __func__, (uint32_t)hw_iir_cfg_dac);

        // hal_codec_iir_dump(hw_iir_cfg_dac);

        hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
        audio_eq.hw_dac_iir_enable = true;

    }
#endif

    return 0;
}
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
int audio_eq_hw_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

    memcpy(&audio_eq.hw_iir_cfg, buf, sizeof(IIR_CFG_T));
    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_eq.hw_iir_cfg.num,
                                                (int32_t)(audio_eq.hw_iir_cfg.gain0*10),
                                                (int32_t)(audio_eq.hw_iir_cfg.gain1*10));

    for (uint8_t i = 0; i<audio_eq.hw_iir_cfg.num; i++)
    {
        TRACE(5,"band num:%d type %d gain:%d fc:%d q:%d", i,
			                                    (int32_t)(audio_eq.hw_iir_cfg.param[i].type),
                                                (int32_t)(audio_eq.hw_iir_cfg.param[i].gain*10),
                                                (int32_t)(audio_eq.hw_iir_cfg.param[i].fc*10),
                                                (int32_t)(audio_eq.hw_iir_cfg.param[i].Q*10));
    }

#ifdef __HW_IIR_EQ_PROCESS__
    {
        HW_IIR_CFG_T *hw_iir_cfg=NULL;
#ifdef __AUDIO_RESAMPLE__
        enum AUD_SAMPRATE_T sample_rate_hw_iir=AUD_SAMPRATE_50781;
#else
        enum AUD_SAMPRATE_T sample_rate_hw_iir=audio_eq.sample_rate;
#endif
        hw_iir_cfg = hw_iir_get_cfg(sample_rate_hw_iir,&audio_eq.hw_iir_cfg);
        ASSERT(hw_iir_cfg != NULL, "[%s] %p codec IIR parameter error!", __func__, hw_iir_cfg);
        hw_iir_set_cfg(hw_iir_cfg);
        audio_eq.hw_iir_enable = true;
    }
#endif

    return 0;
}
#endif
#endif  // #if defined(AUDIO_EQ_TUNING)

#ifdef USB_EQ_TUNING
int audio_eq_usb_iir_callback(uint8_t *buf, uint32_t  len)
{
	IIR_CFG_T* cur_cfg;

	TRACE(2,"usb_iir_cb: len=[%d - %d]", len, sizeof(IIR_CFG_T));

	if (len != sizeof(IIR_CFG_T)) {
        return 1;
    }

	cur_cfg = (IIR_CFG_T*)buf;
	TRACE(2,"-> sample_rate[%d], num[%d]", /*cur_cfg->samplerate,*/ audio_eq.sample_rate, cur_cfg->num);

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_eq.sw_iir_cfg.gain0 = cur_cfg->gain0;
	audio_eq.sw_iir_cfg.gain1 = cur_cfg->gain1;
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_eq.hw_dac_iir_cfg.gain0 = 0;
	audio_eq.hw_dac_iir_cfg.gain1 = 0;
#else
    audio_eq.hw_dac_iir_cfg.gain0 = cur_cfg->gain0;
	audio_eq.hw_dac_iir_cfg.gain1 = cur_cfg->gain1;
#endif
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
	if (cur_cfg->num > AUD_DAC_IIR_NUM_EQ) {
		audio_eq.hw_dac_iir_cfg.num = AUD_DAC_IIR_NUM_EQ;
	} else {
		audio_eq.hw_dac_iir_cfg.num = cur_cfg->num;
	}
    TRACE(1,"-> hw_dac_iir_num[%d]", audio_eq.hw_dac_iir_cfg.num);
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_eq.sw_iir_cfg.num = cur_cfg->num - audio_eq.hw_dac_iir_cfg.num;
    TRACE(1,"-> sw_iir_num[%d]", audio_eq.sw_iir_cfg.num);
#endif
	//TRACE(2,"-> iir_num[%d - %d]", audio_eq.hw_dac_iir_cfg.num, audio_eq.sw_iir_cfg.num);

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
	if (audio_eq.hw_dac_iir_cfg.num) {
		memcpy((void*)(&audio_eq.hw_dac_iir_cfg.param[0]),
					(void*)(&cur_cfg->param[0]),
					audio_eq.hw_dac_iir_cfg.num*sizeof(IIR_PARAM_T));
	}
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	if (audio_eq.sw_iir_cfg.num) {

		memcpy((void*)(&audio_eq.sw_iir_cfg.param[0]),
					(void*)(&cur_cfg->param[audio_eq.hw_dac_iir_cfg.num]),
					audio_eq.sw_iir_cfg.num * sizeof(IIR_PARAM_T));

	} else {

		// set a default filter
		audio_eq.sw_iir_cfg.num = 1;

		audio_eq.sw_iir_cfg.param[0].fc = 1000.0;
		audio_eq.sw_iir_cfg.param[0].gain = 0.0;
		audio_eq.sw_iir_cfg.param[0].type = IIR_TYPE_PEAK;
		audio_eq.sw_iir_cfg.param[0].Q = 1.0;

	}
#endif

	if (audio_eq.sample_rate) {
		audio_eq.update_cfg = true;
	}

	audio_eq.eq_updated_cfg = true;

	return 0;
}

void audio_eq_usb_eq_update (void)
{
	if (audio_eq.update_cfg) {

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
		HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;

		if (audio_eq.hw_dac_iir_cfg.num) {
                    enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
                    sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_eq.sample_rate,1);
                    TRACE(3,"audio_eq.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_eq.sample_rate, sample_rate_hw_dac_iir);
#else
                    sample_rate_hw_dac_iir = audio_eq.sample_rate;
#endif
			hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &audio_eq.hw_dac_iir_cfg);

			hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
			audio_eq.hw_dac_iir_enable = true;

		} else {

			audio_eq.hw_dac_iir_enable = false;
		}
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
		iir_set_cfg(&audio_eq.sw_iir_cfg);
		audio_eq.sw_iir_enable = true;
#endif

		audio_eq.update_cfg = false;

/*
		TRACE(4,"USB EQ Update: en[%d-%d], num[%d-%d]",
				audio_eq.hw_dac_iir_enable, audio_eq.sw_iir_enable,
				audio_eq.hw_dac_iir_cfg.num, audio_eq.sw_iir_cfg.num);
*/
	}

}
#endif	// USB_EQ_TUNING

#ifdef AUDIO_HEARING_COMPSATN
uint8_t hear_cfg_update_flg = 1;
void audio_get_eq_para(int *para_buf, uint8_t cfg_flag)
{
    if(1 == cfg_flag)
        hear_cfg_update_flg = 1;
    else
        hear_cfg_update_flg = 0;

    int *band_compen = (int *)para_buf;
    int iir_num = 6;
    float gain_buf[6] = {0.0F};
#if HWFIR_MOD==HEARING_MOD_VAL
    get_hear_compen_val_fir(band_compen,gain_buf,iir_num);
#else
    get_hear_compen_val_multi_level(band_compen,gain_buf,iir_num);
    iir_num -= 1;
#endif
    for(int i=0;i<iir_num;i++)
        TRACE(1,"gain_buf[%d]=%de-2",i,(int)(gain_buf[i]*100));

#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
    float fc_buf[6] = {600.0, 1200.0, 2400.0, 4800.0, 8000.0, 8000.0};
#if defined(__SW_IIR_EQ_PROCESS__)
    const IIR_CFG_T *iir_default_cfg= audio_eq_sw_iir_cfg_list[0];
#else
    const IIR_CFG_T *iir_default_cfg= audio_eq_hw_dac_iir_cfg_list[0];
#endif
    int default_iir_num = iir_default_cfg->num;
    float default_iir_gain0 = iir_default_cfg->gain0;
    float default_iir_gain1 = iir_default_cfg->gain1;
    float gain_tmp0 = default_iir_gain0;
    float gain_tmp1 = default_iir_gain1;
    TRACE(3,"default IIR band num:%d gain0:%d, gain1:%d",
                                    default_iir_num,
                                    (int32_t)default_iir_gain0,
                                    (int32_t)default_iir_gain1);
    audio_eq.hear_iir_cfg.num = default_iir_num + iir_num;
    audio_eq.hear_iir_cfg.gain0 = gain_tmp0;
    audio_eq.hear_iir_cfg.gain1 = gain_tmp1;

    int band_idx = 0 ;
    int iir_coef_idx = 0;
    for(int i=default_iir_num;i<audio_eq.hear_iir_cfg.num;i++)
    {
        if(default_iir_num==i)
            audio_eq.hear_iir_cfg.param[i].type = IIR_TYPE_LOW_SHELF;
        else if((default_iir_num+iir_num-1)==i)
            audio_eq.hear_iir_cfg.param[i].type = IIR_TYPE_HIGH_SHELF;
        else
            audio_eq.hear_iir_cfg.param[i].type = IIR_TYPE_PEAK;

        if(default_iir_num==i)
            audio_eq.hear_iir_cfg.param[i].Q = 0.707;
        else if((default_iir_num+iir_num-1)==i)
            audio_eq.hear_iir_cfg.param[i].Q = 0.707;
        else
            audio_eq.hear_iir_cfg.param[i].Q = 1.2;

        audio_eq.hear_iir_cfg.param[i].gain = gain_buf[band_idx];
        audio_eq.hear_iir_cfg.param[i].fc = fc_buf[band_idx];

        get_iir_coef(&audio_eq.hear_iir_cfg, i, 44100, &hear_iir_coef[iir_coef_idx]);
        iir_coef_idx += 6;
        band_idx++;
    }

    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                            (int32_t)audio_eq.hear_iir_cfg.num,
                                            (int32_t)(audio_eq.hear_iir_cfg.gain0),
                                            (int32_t)(audio_eq.hear_iir_cfg.gain1));
    for (uint8_t i = default_iir_num; i<audio_eq.hear_iir_cfg.num; i++)
    {
        TRACE(5,"band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                            (int32_t)(audio_eq.hear_iir_cfg.param[i].type),
                                            (int32_t)(audio_eq.hear_iir_cfg.param[i].gain*1000),
                                            (int32_t)(audio_eq.hear_iir_cfg.param[i].fc),
                                            (int32_t)(audio_eq.hear_iir_cfg.param[i].Q*1000));
    }

    for(int i=0;i<6;i++)
        TRACE(1,"hear_iir_coef[%d]=%de-3",i,(int)(hear_iir_coef[i]*1000));

    TRACE(1,"hear_level=%d",get_hear_level());

    if(1 == hear_cfg_update_flg)
    {
        memcpy(&audio_eq.hear_iir_cfg_update, &audio_eq.hear_iir_cfg, sizeof(IIR_CFG_T));

        TRACE(3,"update band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_eq.hear_iir_cfg_update.num,
                                                (int32_t)(audio_eq.hear_iir_cfg_update.gain0),
                                                (int32_t)(audio_eq.hear_iir_cfg_update.gain1));
        for (uint8_t i = default_iir_num; i<audio_eq.hear_iir_cfg_update.num; i++)
        {
            TRACE(5,"update band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                                (int32_t)(audio_eq.hear_iir_cfg_update.param[i].type),
                                                (int32_t)(audio_eq.hear_iir_cfg_update.param[i].gain*1000),
                                                (int32_t)(audio_eq.hear_iir_cfg_update.param[i].fc),
                                                (int32_t)(audio_eq.hear_iir_cfg_update.param[i].Q*1000));
        }
    }

#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    float fir_scale_db = 0.0;
    //For large gain in low freq
    float gain_tmp = -22;
    TRACE(2,"[%s],gain_tmp0=%d",__func__,(int)(gain_tmp));
    for(int i=0;i<iir_num;i++)
        TRACE(2,"gain_buf[%d]=%de-2",i,(int)(gain_buf[i]*100));

    //Get buffer from system pool
#ifdef HEARING_USE_STATIC_RAM
    speech_heap_init(HEAR_DET_STREAM_BUF, HEAR_COMP_BUF_SIZE);
#else
    if(NULL==hear_comp_buf)
        syspool_get_buff(&hear_comp_buf, HEAR_COMP_BUF_SIZE);
    speech_heap_init(hear_comp_buf, HEAR_COMP_BUF_SIZE);
#endif

    HearFir2State* hear_st = HearFir2_create(44100);
    fir_scale_db = HearFir2_process(hear_st, fir_filter,gain_buf);
    TRACE(1,"fir_scale = %de-2",(int)(fir_scale_db*100));
    HearFir2_destroy(hear_st);
    //Free buffer
    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(3,"HEAP INFO: total=%d, used=%d, max_used=%d", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    if(1 == hear_cfg_update_flg)
    {
        float fir_gain_tmp = gain_tmp + fir_scale_db;
        if(0 < fir_gain_tmp)
            fir_gain_tmp = 0;
        audio_eq.hear_hw_fir_cfg.gain  = fir_gain_tmp;
        audio_eq.hear_hw_fir_cfg.len   = 384;

        for(int i=0;i<384;i++)
        {
            audio_eq.hear_hw_fir_cfg.coef[i] = (int)(fir_filter[i]*(1<<23));
        }
    }
    float fir_scale_val = hear_val2db(fir_scale_db);
    TRACE(1,"fir_scale_val = %de-2",(int)(fir_scale_val*100));
    for(int i=0;i<384;i++)
    {
        fir_filter[i] = fir_filter[i] * fir_scale_val;
    }

#endif
#endif
}
#endif

int audio_process_eq_init(void)
{
#if defined(AUDIO_EQ_TUNING)
#if defined(USB_EQ_TUNING)
#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) || defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
    hal_cmd_register("iir_eq", audio_eq_usb_iir_callback);
#endif

#else   // #if defined(USB_EQ_TUNING)
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    hal_cmd_register("sw_iir_eq", audio_eq_sw_iir_callback);
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    hal_cmd_register("dac_iir_eq", audio_eq_hw_dac_iir_callback);
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    hal_cmd_register("hw_iir_eq", audio_eq_hw_iir_callback);
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    hal_cmd_register("fir_eq", audio_eq_hw_fir_callback);
#endif
#endif  // #if defined(USB_EQ_TUNING)
#endif  // #if defined(AUDIO_EQ_TUNING)

#ifdef AUDIO_HEARING_COMPSATN
#if SWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__)
    memcpy(&audio_eq.hear_iir_cfg, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
    memcpy(&audio_eq.hear_iir_cfg_update, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    memcpy(&audio_eq.hear_hw_fir_cfg, audio_eq_hw_fir_cfg_list[0], sizeof(FIR_CFG_T));
#endif
#elif HWIIR_MOD==HEARING_MOD_VAL
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    memcpy(&audio_eq.hear_iir_cfg, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
    memcpy(&audio_eq.hear_iir_cfg_update, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif
#endif
#endif

    // load default cfg
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    memcpy(&audio_eq.sw_iir_cfg, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    memcpy(&audio_eq.hw_dac_iir_cfg, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    memcpy(&audio_eq.hw_iir_cfg, audio_eq_hw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    memcpy(&audio_eq.hw_fir_cfg, audio_eq_hw_fir_cfg_list[0], sizeof(FIR_CFG_T));
#endif

    return 0;
}