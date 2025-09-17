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
#ifndef __EAUDIO_A2DP_TIMER_H__
#define __EAUDIO_A2DP_TIMER_H__

#include "plat_types.h"
#include "hwtimer_list.h"

typedef void (*eaudio_timer_cb)(void *param);

typedef struct {
    bool                        timer_init;
    bool                        timer_run;
    uint32_t                    timeout_val;
    HWTIMER_ID                  cal_timer_id;
    eaudio_timer_cb             cb;
    void*                       usrdata;
} eaudio_timer_t;

#if defined(__cplusplus)
extern "C" {
#endif

void eaudio_a2dp_timer_consumer_init(eaudio_timer_cb cb, void* usrdata);

void eaudio_a2dp_timer_consumer_deinit(void);

void eaudio_a2dp_timer_consumer_start(uint32_t timeout_val);

void eaudio_a2dp_timer_consumer_stop(void);

void eaudio_a2dp_timer_consumer_tune(uint32_t dest_val, uint32_t current_tick, void* usrdata);

#if defined(__cplusplus)
}
#endif
#endif // end of __EAUDIO_A2DP_TIMER_H__