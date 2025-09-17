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
#ifndef __AUDIO_PROCESS_EQ_H__
#define __AUDIO_PROCESS_EQ_H__

#include "stdint.h"
#include "iir_process.h"
#include "fir_process.h"
#ifdef AUDIO_PROCESS_IN_CP
#include "hal_location.h"
#endif

typedef enum {
    AUDIO_EQ_TYPE_SW_IIR = 0,
    AUDIO_EQ_TYPE_HW_FIR,
    AUDIO_EQ_TYPE_HW_DAC_IIR,
    AUDIO_EQ_TYPE_HW_IIR,
    AUDIO_EQ_TYPE_QTY
} AUDIO_EQ_TYPE_T;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t audio_process_eq_get_index(AUDIO_EQ_TYPE_T type);
int32_t audio_process_eq_set_index(AUDIO_EQ_TYPE_T type, uint32_t index);

uint32_t audio_process_eq_need_audio_buf_size(void);

int audio_process_eq_init(void);
int audio_process_eq_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits,enum AUD_CHANNEL_NUM_T sw_ch_num, enum AUD_CHANNEL_NUM_T hw_ch_num,int32_t frame_size, void *eq_buf, uint32_t len);
#ifdef AUDIO_PROCESS_IN_CP
SRAM_TEXT_LOC
#endif
int audio_process_eq_run(uint8_t *buf, uint32_t len);
int audio_process_eq_close(void);

int audio_peocess_eq_set_eq(void);

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const FIR_CFG_T *fir_cfg_2,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type);
#else
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type);
#endif

#ifdef USB_EQ_TUNING
void audio_eq_usb_eq_update (void);
#endif

#ifdef __cplusplus
}
#endif

#endif
