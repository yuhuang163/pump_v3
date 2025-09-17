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
/*******************************************************************************
** namer DAC EQ filter
** description: IIR filter
** version: V1.0
** author: xuml
** modify: 2020.10.5.
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "plat_addr_map.h"
#include "reg_codec_best1600.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "hal_timer.h"
#include "hw_codec_iir_process.h"
#include "hwtimer_list.h"



/**********************************macro**************************************/

/*
iir coefficients(b/a Q27)
a1
a2
b1
b2
b0
*/
struct _iir_coefs
{
    int32_t a1 ;
    int32_t a2 ;

    int32_t b1 ;
    int32_t b2 ;
    int32_t b0 ;
};


static struct CODEC_T * const codec = (struct CODEC_T *)CODEC_BASE;

typedef struct _anc_rir_coefs {
    int32_t coef_b[3];
    int32_t coef_a[3];
} anc_iir_coefs;


#define FIXED_GAIN_RAMP_Q (1 << 27)

#define DEQ_GAIN_RAMP_THRESHLD (40)

#define DEQ_GAIN_RAMP_ZERO (20)

#define DEQ_GAIN_RAMP_BURST_THRESHLD (DEQ_GAIN_RAMP_THRESHLD*10000)

#define IIR_DAC_COUNTER_L (20)
#define IIR_DAC_COUNTER_R (20)

#define IIR_DAC_COUNTER_MAX (40)

/*******************************data struction***********************************/

#define ANC_BASE                            ((uint32_t)CODEC_BASE)

#define IIR_COEF_BASE  (ANC_BASE + 0x2800)

//IIR registers
volatile static struct _iir_coefs *deq_dac_iir_coefs0_l = (volatile struct _iir_coefs *)(IIR_COEF_BASE);
volatile static struct _iir_coefs *deq_dac_iir_coefs0_r = (volatile struct _iir_coefs *)(IIR_COEF_BASE + IIR_DAC_COUNTER_L*20);

volatile static struct _iir_coefs *deq_dac_gain_iir_coefs0_l = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x0320);
volatile static struct _iir_coefs *deq_dac_gain_iir_coefs0_r = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x0334);

volatile static struct _iir_coefs *deq_dac_iir_coefs1_l = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x400);
volatile static struct _iir_coefs *deq_dac_iir_coefs1_r = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x400 + IIR_DAC_COUNTER_L*20);

volatile static struct _iir_coefs *deq_dac_gain_iir_coefs1_l = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x720);
volatile static struct _iir_coefs *deq_dac_gain_iir_coefs1_r = (volatile struct _iir_coefs *)(IIR_COEF_BASE + 0x734);

static int32_t dac_eq_counter_l=IIR_DAC_COUNTER_L;
static int32_t dac_eq_counter_r=IIR_DAC_COUNTER_R;

volatile static int daq_eq_output_ch_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;

#define DEQ_GAIN_RAMP_DELAY   (1)

#if DEQ_GAIN_RAMP_DELAY == 1

/*
Type='Low Passing';
Freq=240;
Gain=0;
Q=0.2;
[b,a]=biquad1(Freq,Gain,Q,Type,SampleRate);
*/
static const anc_iir_coefs   iir_coef_gain_ramp_deq=
{
    .coef_b={30703,        61406,        30703},
    .coef_a={134217728,   -248768019,    114673103},
};
#define DEQ_SET_GAIN_TIME  ((MS_TO_TICKS(50)))

#elif DEQ_GAIN_RAMP_DELAY==2
/*

Type='Low Passing';
Freq=120;
Gain=0;
Q=0.2;
[b,a]=biquad1(Freq,Gain,Q,Type,SampleRate);
*/
static const anc_iir_coefs   iir_coef_gain_ramp_deq=
{
    .coef_b={7966,        15932,         7966},
    .coef_a={134217728,   -258260876,    124075013},
};

#define DEQ_SET_GAIN_TIME  ((MS_TO_TICKS(100)))
#elif DEQ_GAIN_RAMP_DELAY==3

/*
Type='Low Passing';
Freq=60;
Gain=0;
Q=0.2;
[b,a]=biquad1(Freq,Gain,Q,Type,SampleRate);
*/
static const anc_iir_coefs   iir_coef_gain_ramp_deq=
{
    .coef_b={2030,         4060,         2030},
    .coef_a={134217728,   -263258168,    129048560},
};
#define DEQ_SET_GAIN_TIME  ((MS_TO_TICKS(200)))
#elif DEQ_GAIN_RAMP_DELAY==4


/*
Type='Low Passing';
Freq=30;
Gain=0;
Q=0.2;
[b,a]=biquad1(Freq,Gain,Q,Type,SampleRate);
*/

static const anc_iir_coefs   iir_coef_gain_ramp_deq=
{
    .coef_b={            512,         1025,          512},
    .coef_a={134217728,   -265823675,    131607997},
};
#define DEQ_SET_GAIN_TIME  ((MS_TO_TICKS(400)))

#endif


#define TRACE_COUNTER (10000)

volatile static int dac_open_flag=0;

static HW_CODEC_IIR_FILTERS_T dac_filtes_l_old;
static HW_CODEC_IIR_FILTERS_T dac_filtes_r_old;

volatile static int8_t dac_iir_reset_flag;

volatile static int8_t iir_coef_using;

volatile static enum AUD_SAMPRATE_T sample_rate_dac;

static int32_t trace_counter=0;

volatile static int32_t deq_iir_gaina_ch0;
volatile static int32_t deq_iir_gainb_ch0;

volatile static int32_t deq_iir_gaina_ch1;
volatile static int32_t deq_iir_gainb_ch1;

HW_CODEC_IIR_CFG_T dac_iir_cfg_new;
HW_CODEC_IIR_CFG_T dac_iir_cfg;

volatile static int max_deq_gain_l=0;
volatile static int max_deq_gain_r=0;

static HWTIMER_ID deq_switching_timer=NULL;
static bool deq_using_flag=0;
static bool deq_reserve_flag=0;
static int deq_switching_coef_delay;

volatile static int codec_dac_flag=0;

#define FIXED_COEF_Q (1<<27)
#define GAIN_Q (9)

static int iir_eq_filter_para_copy(HW_CODEC_IIR_FILTERS_T *filtes_old,const HW_CODEC_IIR_FILTERS_T *filtes_new,int iir_dac_counter)
{
    filtes_old->iir_bypass_flag=filtes_new->iir_bypass_flag;
    filtes_old->iir_counter=filtes_new->iir_counter;

    int i;

    if( filtes_new->iir_counter>iir_dac_counter) {
        TRACE(1,"%s: warning filtes_new->iir_counter:%d",__func__,filtes_new->iir_counter);
        filtes_old->iir_counter = iir_dac_counter;
    } else if ( filtes_new->iir_counter <= 0) {
        TRACE(1,"%s: warning filtes_new->iir_counter:%d",__func__,filtes_new->iir_counter);
        filtes_old->iir_counter = 0;
        filtes_old->iir_bypass_flag = 1;
    }

    for (i = 0; i < filtes_old->iir_counter; i++) {
        filtes_old->iir_coef[i] = filtes_new->iir_coef[i];
    }

    for( ; i < iir_dac_counter; i++) {
        filtes_old->iir_coef[i].coef_a[0] = FIXED_COEF_Q;
        filtes_old->iir_coef[i].coef_a[1] = 0;
        filtes_old->iir_coef[i].coef_a[2] = 0;
        filtes_old->iir_coef[i].coef_b[0] = FIXED_COEF_Q;
        filtes_old->iir_coef[i].coef_b[1] = 0;
        filtes_old->iir_coef[i].coef_b[2] = 0;
    }

    filtes_old->iir_counter = iir_dac_counter;

    return 0;
}

static int iir_eq_filter_coef_copy(volatile struct _iir_coefs *anc_iir_coefs,HW_CODEC_IIR_FILTERS_T *filtes_old)
{
    int i;
    // TRACE(1,"%s:anc_iir_coefs:0x%x",__func__,(int)anc_iir_coefs);
    for (i = 0; i < filtes_old->iir_counter; i++) {
        anc_iir_coefs[i].a1 = -filtes_old->iir_coef[i].coef_a[1];
        anc_iir_coefs[i].a2 = -filtes_old->iir_coef[i].coef_a[2];
        anc_iir_coefs[i].b0 = filtes_old->iir_coef[i].coef_b[0];
        anc_iir_coefs[i].b1 = filtes_old->iir_coef[i].coef_b[1];
        anc_iir_coefs[i].b2 = filtes_old->iir_coef[i].coef_b[2];
        // TRACE(1,"%s:anc_iir_coefs[i].a1:0x%x",__func__,anc_iir_coefs[i].a1);
        // TRACE(1,"%s:filtes_old->iir_coef[i].coef_a[1]:0x%x\n",__func__,-filtes_old->iir_coef[i].coef_a[1]);
    }
    return 0;
}
static int deq_gaina_cfg_gain(void)
{
    uint32_t lock;

    TRACE(1,"%s:deq_iir_gaina_ch0:%d,deq_iir_gaina_ch1:%d",__func__,deq_iir_gaina_ch0,deq_iir_gaina_ch1);

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0) {
        lock = int_lock();
        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH0;
        codec->REG_2E0 = SET_BITFIELD(codec->REG_2E0, CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH0, deq_iir_gaina_ch0);
        codec->REG_2E8 = SET_BITFIELD(codec->REG_2E8, CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH0, DEQ_GAIN_RAMP_ZERO);
        hal_sys_timer_delay_us(2);
        codec->REG_25C |= CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH0;
        int_unlock(lock);
    }

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1) {
        lock = int_lock();
        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH1;
        codec->REG_2E4 = SET_BITFIELD(codec->REG_2E4, CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH1, deq_iir_gaina_ch1);
        codec->REG_2EC = SET_BITFIELD(codec->REG_2EC, CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH1, DEQ_GAIN_RAMP_ZERO);
        hal_sys_timer_delay_us(2);
        codec->REG_25C |= CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH1;
        int_unlock(lock);
    }

    return 0;
}


static int deq_gainb_cfg_gain(void)
{
    uint32_t lock;

    TRACE(1,"%s:deq_iir_gainb_ch0:%d,deq_iir_gainb_ch1:%d",__func__,deq_iir_gainb_ch0,deq_iir_gainb_ch1);

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0) {
        lock = int_lock();
        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH0;
        codec->REG_2E0 = SET_BITFIELD(codec->REG_2E0, CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH0, DEQ_GAIN_RAMP_ZERO);
        codec->REG_2E8 = SET_BITFIELD(codec->REG_2E8, CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH0, deq_iir_gainb_ch0);
        hal_sys_timer_delay_us(2);
        codec->REG_25C |= CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH0;
        int_unlock(lock);
    }

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1) {
        lock = int_lock();
        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH1;
        codec->REG_2E4 = SET_BITFIELD(codec->REG_2E4, CODEC_CODEC_DEQ_IIR_GAINA_EXT_CH1, DEQ_GAIN_RAMP_ZERO);
        codec->REG_2EC = SET_BITFIELD(codec->REG_2EC, CODEC_CODEC_DEQ_IIR_GAINB_EXT_CH1, deq_iir_gainb_ch1);
        hal_sys_timer_delay_us(2);
        codec->REG_25C |= CODEC_CODEC_DEQ_IIR_GAIN_EXT_UPDATE_CH1;
        int_unlock(lock);
    }

    return 0;
}

static int hw_codec_iir_set_dac_cfg_internal(HW_CODEC_IIR_CFG_T *cfg)
{
    HW_CODEC_IIR_ERROR err=HW_CODEC_IIR_NO_ERR;

    const HW_CODEC_IIR_FILTERS_T *dac_filtes_l = NULL;
    const HW_CODEC_IIR_FILTERS_T *dac_filtes_r = NULL;


    TRACE(1,"%s:",__func__);

    if (dac_open_flag == 0) {
        TRACE(1,"%s: dac iir not opened", __func__);
        return HW_CODEC_IIR_TYPE_ERR;
    }

    dac_filtes_l = &(cfg->iir_filtes_l);
    dac_filtes_r = &(cfg->iir_filtes_r);

#if 0
    TRACE(1,"eq counter %d", dac_filtes_l->iir_counter);

    for(int j = 0; j <dac_filtes_l->iir_counter ; j++)
    {
       TRACE(1,"iir coef dac eq l 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
        dac_filtes_l->iir_coef[j].coef_b[0], \
        dac_filtes_l->iir_coef[j].coef_b[1], \
        dac_filtes_l->iir_coef[j].coef_b[2], \
        dac_filtes_l->iir_coef[j].coef_a[0], \
        dac_filtes_l->iir_coef[j].coef_a[1], \
        dac_filtes_l->iir_coef[j].coef_a[2]);
    }
    for(int j = 0; j <dac_filtes_l->iir_counter ; j++)
    {
       TRACE(1,"iir coef dac eq r 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",\
        dac_filtes_r->iir_coef[j].coef_b[0], \
        dac_filtes_r->iir_coef[j].coef_b[1], \
        dac_filtes_r->iir_coef[j].coef_b[2], \
        dac_filtes_r->iir_coef[j].coef_a[0], \
        dac_filtes_r->iir_coef[j].coef_a[1], \
        dac_filtes_r->iir_coef[j].coef_a[2]);
    }

#endif

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0) {
        iir_eq_filter_para_copy(&dac_filtes_l_old, dac_filtes_l, dac_eq_counter_l);
        max_deq_gain_l = (1 << GAIN_Q);
    }

    if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1) {
        iir_eq_filter_para_copy(&dac_filtes_r_old, dac_filtes_r, dac_eq_counter_r);
        max_deq_gain_r = (1 << GAIN_Q);
    }

    if (iir_coef_using == 0) {
        trace_counter = 0;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0) {
            codec->REG_258 = (codec->REG_258 & ~CODEC_CODEC_DEQ_IIR_CH0_BYPASS) | dac_filtes_l_old.iir_bypass_flag;
            codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH0, dac_filtes_l_old.iir_counter);

            iir_eq_filter_coef_copy(deq_dac_iir_coefs1_l,&dac_filtes_l_old);
        }
        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1) {
            codec->REG_258 = (codec->REG_258 & ~CODEC_CODEC_DEQ_IIR_CH1_BYPASS) | dac_filtes_r_old.iir_bypass_flag;
            codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH1, dac_filtes_r_old.iir_counter);

            iir_eq_filter_coef_copy(deq_dac_iir_coefs1_r,&dac_filtes_r_old);
        }

        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0) {
            deq_iir_gaina_ch0 = 0;
            deq_iir_gainb_ch0 = (int32_t)(((float)max_deq_gain_l/512.0f)*FIXED_GAIN_RAMP_Q);
        }

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1) {
            deq_iir_gaina_ch1 = 0;
            deq_iir_gainb_ch1 = (int32_t)(((float)max_deq_gain_r/512.0f)*FIXED_GAIN_RAMP_Q);
        }

        deq_gainb_cfg_gain();

        iir_coef_using = 1;
    }
    else
    {
        trace_counter=0;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0)
        {
            codec->REG_258 = (codec->REG_258 & ~CODEC_CODEC_DEQ_IIR_CH0_BYPASS) | dac_filtes_l_old.iir_bypass_flag;
            codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH0, dac_filtes_l_old.iir_counter);

            iir_eq_filter_coef_copy(deq_dac_iir_coefs0_l, &dac_filtes_l_old);
        }
        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1)
        {
            codec->REG_258 = (codec->REG_258 & ~CODEC_CODEC_DEQ_IIR_CH1_BYPASS) | dac_filtes_r_old.iir_bypass_flag;
            codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH1, dac_filtes_r_old.iir_counter);

            iir_eq_filter_coef_copy(deq_dac_iir_coefs0_r,&dac_filtes_r_old);
        }

        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH0)
        {
            deq_iir_gaina_ch0 = (int32_t)(((float)max_deq_gain_l/512.0f)*FIXED_GAIN_RAMP_Q);
            deq_iir_gainb_ch0 = 0;
        }

        if (daq_eq_output_ch_map & AUD_CHANNEL_MAP_CH1)
        {
            deq_iir_gaina_ch1 = (int32_t)(((float)max_deq_gain_r/512.0f)*FIXED_GAIN_RAMP_Q);
            deq_iir_gainb_ch1 = 0;
        }

        deq_gaina_cfg_gain();

        iir_coef_using = 0;
    }

    return err;
}
static void deq_switching_timer_handler(void *param)
{
    TRACE(1,"%s", __func__);

    uint32_t lock;
    lock = int_lock();

    bool deq_reserve_old_flag = deq_reserve_flag;

    if(deq_reserve_flag) {
        deq_using_flag = 1;
        deq_reserve_flag = 0;
        dac_iir_cfg = dac_iir_cfg_new;
    } else {
        deq_using_flag = 0;
    }
    int_unlock(lock);

    if(deq_reserve_old_flag) {
        hw_codec_iir_set_dac_cfg_internal(&dac_iir_cfg);
        hwtimer_stop(deq_switching_timer);
        hwtimer_start(deq_switching_timer, deq_switching_coef_delay);
    }
    return;
}

int hw_codec_iir_set_cfg(HW_CODEC_IIR_CFG_T *cfg,enum AUD_SAMPRATE_T sample_rate,HW_CODEC_IIR_TYPE_T hw_iir_type)
{
    HW_CODEC_IIR_ERROR err = HW_CODEC_IIR_NO_ERR;
    uint32_t lock;

    TRACE(1,"%s", __func__);

    if (cfg == NULL) {
        TRACE(1,"%s: cfg is null", __func__);
        return HW_CODEC_IIR_OTHER_ERR;
    }

    if (hw_iir_type != HW_CODEC_IIR_DAC ) {
        TRACE(1,"%s: can't support hw_iir_type=%d.", __func__, hw_iir_type);
        return HW_CODEC_IIR_TYPE_ERR;
    }

    if (hw_iir_type==HW_CODEC_IIR_DAC && dac_open_flag==0) {
        TRACE(1,"%s: hw_iir_type=%d not opened", __func__, hw_iir_type);
        return HW_CODEC_IIR_TYPE_ERR;
    }

    lock = int_lock();
    bool deq_using_old_flag = deq_using_flag;

    dac_iir_cfg_new = *cfg;

    if (deq_using_flag == 0) {
        deq_using_flag = 1;
        deq_reserve_flag = 0;
        dac_iir_cfg = dac_iir_cfg_new;
    } else {
        deq_reserve_flag = 1;
    }
    int_unlock(lock);

    if (deq_using_old_flag == 0) {
        hw_codec_iir_set_dac_cfg_internal(&dac_iir_cfg);
        hwtimer_stop(deq_switching_timer);
        hwtimer_start(deq_switching_timer, deq_switching_coef_delay);
    }
    return err;
}


static void hw_iir_ctrl_reg_init(void)
{
    //disable dac iir;
    codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_ENABLE;

    codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;
    codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;

    codec->REG_318 = SET_BITFIELD(codec->REG_318, CODEC_CODEC_DEQ_IIR_GAIN_EXT_TH, DEQ_GAIN_RAMP_THRESHLD);

    codec->REG_0DC &= ~CODEC_CODEC_DAC_L_EQ_ENABLE;
    codec->REG_0DC &= ~CODEC_CODEC_DAC_R_EQ_ENABLE;
}


static void hw_iir_ctrl_reg_open( HW_CODEC_IIR_TYPE_T hw_iir_type)
{
    TRACE(1,"%s", __func__);

    if ( hw_iir_type == HW_CODEC_IIR_DAC) {

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_ENABLE;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_AUTO_STOP;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_COEF_SWAP;

        deq_dac_gain_iir_coefs0_l[0].a1 = -iir_coef_gain_ramp_deq.coef_a[1];
        deq_dac_gain_iir_coefs0_l[0].a2 = -iir_coef_gain_ramp_deq.coef_a[2];
        deq_dac_gain_iir_coefs0_l[0].b0 = iir_coef_gain_ramp_deq.coef_b[0];
        deq_dac_gain_iir_coefs0_l[0].b1 = iir_coef_gain_ramp_deq.coef_b[1];
        deq_dac_gain_iir_coefs0_l[0].b2 = iir_coef_gain_ramp_deq.coef_b[2];

        deq_dac_gain_iir_coefs1_l[0].a1 = -iir_coef_gain_ramp_deq.coef_a[1];
        deq_dac_gain_iir_coefs1_l[0].a2 = -iir_coef_gain_ramp_deq.coef_a[2];
        deq_dac_gain_iir_coefs1_l[0].b0 = iir_coef_gain_ramp_deq.coef_b[0];
        deq_dac_gain_iir_coefs1_l[0].b1 = iir_coef_gain_ramp_deq.coef_b[1];
        deq_dac_gain_iir_coefs1_l[0].b2 = iir_coef_gain_ramp_deq.coef_b[2];

        deq_dac_gain_iir_coefs0_r[0].a1 = -iir_coef_gain_ramp_deq.coef_a[1];
        deq_dac_gain_iir_coefs0_r[0].a2 = -iir_coef_gain_ramp_deq.coef_a[2];
        deq_dac_gain_iir_coefs0_r[0].b0 = iir_coef_gain_ramp_deq.coef_b[0];
        deq_dac_gain_iir_coefs0_r[0].b1 = iir_coef_gain_ramp_deq.coef_b[1];
        deq_dac_gain_iir_coefs0_r[0].b2 = iir_coef_gain_ramp_deq.coef_b[2];

        deq_dac_gain_iir_coefs1_r[0].a1 = -iir_coef_gain_ramp_deq.coef_a[1];
        deq_dac_gain_iir_coefs1_r[0].a2 = -iir_coef_gain_ramp_deq.coef_a[2];
        deq_dac_gain_iir_coefs1_r[0].b0 = iir_coef_gain_ramp_deq.coef_b[0];
        deq_dac_gain_iir_coefs1_r[0].b1 = iir_coef_gain_ramp_deq.coef_b[1];
        deq_dac_gain_iir_coefs1_r[0].b2 = iir_coef_gain_ramp_deq.coef_b[2];

        for (int i = 0; i < IIR_DAC_COUNTER_L; i++) {
            deq_dac_iir_coefs0_l[i].a1 = 0;
            deq_dac_iir_coefs0_l[i].a2 = 0;
            deq_dac_iir_coefs0_l[i].b0 = 0;
            deq_dac_iir_coefs0_l[i].b1 = 0;
            deq_dac_iir_coefs0_l[i].b2 = 0;

            deq_dac_iir_coefs1_l[i].a1 = 0;
            deq_dac_iir_coefs1_l[i].a2 = 0;
            deq_dac_iir_coefs1_l[i].b0 = 0;
            deq_dac_iir_coefs1_l[i].b1 = 0;
            deq_dac_iir_coefs1_l[i].b2 = 0;
        }

        for (int i = 0; i < IIR_DAC_COUNTER_R; i++){
            deq_dac_iir_coefs0_r[i].a1 = 0;
            deq_dac_iir_coefs0_r[i].a2 = 0;
            deq_dac_iir_coefs0_r[i].b0 = 0;
            deq_dac_iir_coefs0_r[i].b1 = 0;
            deq_dac_iir_coefs0_r[i].b2 = 0;

            deq_dac_iir_coefs1_r[i].a1 = 0;
            deq_dac_iir_coefs1_r[i].a2 = 0;
            deq_dac_iir_coefs1_r[i].b0 = 0;
            deq_dac_iir_coefs1_r[i].b1 = 0;
            deq_dac_iir_coefs1_r[i].b2 = 0;
        }

        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_CH0_BYPASS;
        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_CH1_BYPASS;

        codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH0, dac_eq_counter_l);
        codec->REG_258 = SET_BITFIELD(codec->REG_258, CODEC_CODEC_DEQ_IIR_COUNT_CH1, dac_eq_counter_r);

        codec->REG_0DC |= CODEC_CODEC_DAC_L_EQ_ENABLE;
        codec->REG_0DC |= CODEC_CODEC_DAC_R_EQ_ENABLE;

        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_SEL_CH0;
        codec->REG_25C &= ~CODEC_CODEC_DEQ_IIR_GAIN_EXT_SEL_CH1;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_GAINCAL_EXT_CH0_BYPASS;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_GAINCAL_EXT_CH1_BYPASS;

        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_GAINUSE_EXT_CH0_BYPASS;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_GAINUSE_EXT_CH1_BYPASS;

        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_AUTO_STOP;

        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;
        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;
        codec->REG_258 |= CODEC_CODEC_DEQ_IIR_ENABLE;

        iir_coef_using = 0;
        deq_iir_gaina_ch0 = DEQ_GAIN_RAMP_ZERO;
        deq_iir_gainb_ch0 = DEQ_GAIN_RAMP_ZERO;
        max_deq_gain_l = 0;

        deq_iir_gaina_ch1 = DEQ_GAIN_RAMP_ZERO;
        deq_iir_gainb_ch1 = DEQ_GAIN_RAMP_ZERO;
        max_deq_gain_r = 0;

        deq_using_flag = 0;
        deq_reserve_flag = 0;
        codec_dac_flag = 0;
        deq_switching_coef_delay = DEQ_SET_GAIN_TIME;

        deq_gaina_cfg_gain();
    }
}

static void hw_iir_ctrl_reg_close(HW_CODEC_IIR_TYPE_T hw_iir_type)
{
    if (hw_iir_type == HW_CODEC_IIR_DAC) {
        //disable dac iir;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRA_ENABLE;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_IIRB_ENABLE;
        codec->REG_258 &= ~CODEC_CODEC_DEQ_IIR_ENABLE;
        codec->REG_0DC &= ~CODEC_CODEC_DAC_L_EQ_ENABLE;
        codec->REG_0DC &= ~CODEC_CODEC_DAC_R_EQ_ENABLE;
    }
}

int hw_codec_iir_open(enum AUD_SAMPRATE_T sample_rate,  HW_CODEC_IIR_TYPE_T hw_iir_type, int32_t ch_map)
{
    HW_CODEC_IIR_ERROR err = HW_CODEC_IIR_NO_ERR;

    TRACE(1,"%s:sample_rate:%d", __func__,sample_rate);

    if (ch_map & AUD_CHANNEL_MAP_CH0) {
        daq_eq_output_ch_map |= AUD_CHANNEL_MAP_CH0;
    } else {
        daq_eq_output_ch_map &= ~AUD_CHANNEL_MAP_CH0;
        dac_eq_counter_l=0;
    }

    if (ch_map & AUD_CHANNEL_MAP_CH1) {
        daq_eq_output_ch_map |= AUD_CHANNEL_MAP_CH0;
    } else {
        daq_eq_output_ch_map &= ~AUD_CHANNEL_MAP_CH1;
        dac_eq_counter_r=0;
    }

    if (daq_eq_output_ch_map == AUD_CHANNEL_MAP_CH0) {
        dac_eq_counter_l = IIR_DAC_COUNTER_MAX;
    }


    TRACE(1,"%s:dac_eq_counter_l:%d,dac_eq_counter_r:%d", __func__,dac_eq_counter_l, dac_eq_counter_r);

    if (dac_open_flag == 0) {
        if (sample_rate <= AUD_SAMPRATE_50781) {
            if ((dac_eq_counter_l + dac_eq_counter_r) > 34) {
                hal_codec_iir_eq_enable(24000000);
            } else {
                hal_codec_iir_eq_enable(24000000/2);
            }
        } else if (sample_rate <= AUD_SAMPRATE_101562) {
            if ((dac_eq_counter_l + dac_eq_counter_r) > 34) {
                hal_codec_iir_eq_enable(24000000*2);
            } else {
                hal_codec_iir_eq_enable(24000000);
            }
        } else if (sample_rate <= AUD_SAMPRATE_203125) {
            if ((dac_eq_counter_l + dac_eq_counter_r) > 34) {
                hal_codec_iir_eq_enable(24000000*4);
            } else {
                hal_codec_iir_eq_enable(24000000*2);
            }
        } else {
            if ((dac_eq_counter_l + dac_eq_counter_r) > 34) {
                hal_codec_iir_eq_enable(24000000*8);
            } else {
                hal_codec_iir_eq_enable(24000000*4);
            }
        }

        hw_iir_ctrl_reg_init();

        iir_coef_using = 0;
        sample_rate_dac = AUD_SAMPRATE_NULL;
    }
// this not need.
    if(deq_switching_timer == NULL) {
        deq_switching_timer = hwtimer_alloc(deq_switching_timer_handler, NULL);
        ASSERT(deq_switching_timer, "Failed to alloc deq_switching_timer");
    }

    if(hw_iir_type==HW_CODEC_IIR_DAC) {
        dac_iir_reset_flag = 0;
        dac_open_flag = 1;
        sample_rate_dac = sample_rate;

        //clear iir_counter;
        dac_filtes_l_old.iir_counter = 0;
        dac_filtes_l_old.iir_bypass_flag = 1;

        dac_filtes_r_old.iir_counter = 0;
        dac_filtes_r_old.iir_bypass_flag = 1;
    }

    hw_iir_ctrl_reg_open(hw_iir_type);

    return err;
}


int hw_codec_iir_close( HW_CODEC_IIR_TYPE_T hw_iir_type)
{
    HW_CODEC_IIR_ERROR err = HW_CODEC_IIR_NO_ERR;

    TRACE(1,"%s,%d", __func__,hw_iir_type);

    if(hw_iir_type == HW_CODEC_IIR_DAC) {
        dac_open_flag = 0;
        sample_rate_dac = AUD_SAMPRATE_NULL;
    }

    hw_iir_ctrl_reg_close(hw_iir_type);

    if(dac_open_flag == 0) {
        hwtimer_stop(deq_switching_timer);
        hal_codec_iir_eq_disable();
    }

    return err;
}


int hw_codec_iir_set_coefs(HW_CODEC_IIR_CFG_F *cfg, HW_CODEC_IIR_TYPE_T hw_iir_type)
{
    HW_CODEC_IIR_ERROR err=HW_CODEC_IIR_NO_ERR;

    HW_CODEC_IIR_CFG_T *hw_iir_cfg = hw_codec_iir_convert_cfg(cfg);

    err = hw_codec_iir_set_cfg(hw_iir_cfg, AUD_SAMPRATE_NULL, hw_iir_type);

    return err;
}

void hw_codec_iir_enable(HW_CODEC_IIR_TYPE_T hw_iir_type, int flag)
{
    if (hw_iir_type == HW_CODEC_IIR_DAC) {
        if (flag == 0) {
            //disable dac iir;
            codec->REG_0DC &= ~CODEC_CODEC_DAC_L_EQ_ENABLE;
            codec->REG_0DC &= ~CODEC_CODEC_DAC_R_EQ_ENABLE;
        } else {
            //enable dac iir;
            codec->REG_0DC |= CODEC_CODEC_DAC_L_EQ_ENABLE;
            codec->REG_0DC |= CODEC_CODEC_DAC_R_EQ_ENABLE;
        }
    }
}
/*
void hw_codec_iir_run(void)
{
    codec_dac_flag=1;
}*/
