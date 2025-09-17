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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "hwtimer_list.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "eaudio_a2dp_timer.h"


#define MIN_CONSUMER_INTERVAL  5000
#define MAX_CONSUMER_INTERVAL  50000

#define pal_logd(fmt,...)   TRACE(3, fmt, ##__VA_ARGS__)
#define pal_logi(fmt,...)   TRACE(3, fmt, ##__VA_ARGS__)

static eaudio_timer_t timer_consumer;

static int32_t first_tick = -1;
static int32_t last_tick = 0;
static int32_t cur_delta_us = 0;
static int32_t last_delta_us = 0;
static int32_t total_delta_us = 0;

typedef struct {
    float Kp;
    float Ki;
    float Kd;
} pid_para_t;

static pid_para_t PID_T;

static void _eaudio_a2dp_timer_consumer_handler(void *param)
{
    eaudio_timer_t *cal_t = (eaudio_timer_t *)param;
    cal_t->timer_run = false;
    //pal_logd("[%s] cal_t->timer_run %d", __func__, cal_t->timer_run);
    if (cal_t->cb) {
        cal_t->cb(cal_t->usrdata);
    }
}

void eaudio_a2dp_timer_consumer_init(eaudio_timer_cb cb, void* usrdata)
{
    eaudio_timer_t* timer_ins = &timer_consumer;
    if (timer_ins->timer_init == true) {
        pal_logi("timer_consumer has been inited");
        return;
    }
    timer_ins->timer_init = true;

    timer_ins->cal_timer_id = hwtimer_alloc(_eaudio_a2dp_timer_consumer_handler, (void *)(timer_ins));
    timer_ins->timer_run = false;
    pal_logi("eaudio_timer_consumer_init");
    timer_ins->cb = cb;
    timer_ins->usrdata = usrdata;

    PID_T.Kp = 0.8;
    PID_T.Ki = 0.1;
    PID_T.Kd = 0.1;

    first_tick = -1;
}

void eaudio_a2dp_timer_consumer_deinit(void)
{
    eaudio_timer_t* timer_ins = &timer_consumer;
    if (timer_ins->timer_run == true) {
        hwtimer_stop(timer_ins->cal_timer_id);
        timer_ins->timer_run  = false;
        pal_logi("[%s] timer_ins->timer_run %d", __func__, timer_ins->timer_run);
    }
    if (timer_ins->cal_timer_id) {
        hwtimer_free(timer_ins->cal_timer_id);
    }
    timer_ins->timer_init = false;
}

void eaudio_a2dp_timer_consumer_start(uint32_t timeout_val)
{
    eaudio_timer_t* timer_ins = &timer_consumer;

    if (timer_ins->timer_run == false) {
        hwtimer_start(timer_ins->cal_timer_id, US_TO_TICKS(timeout_val));
        timer_ins->timer_run  = true;
        pal_logi("[%s]", __func__);
    }
}

void eaudio_a2dp_timer_consumer_stop(void)
{
    eaudio_timer_t* timer_ins = &timer_consumer;

    if (timer_ins->timer_run == true) {
        hwtimer_stop(timer_ins->cal_timer_id);
        timer_ins->timer_run  = false;
        pal_logi("[%s]", __func__);
    }
}

void eaudio_a2dp_timer_consumer_tune(uint32_t dest_val, uint32_t current_tick, void* usrdata)
{
    eaudio_timer_t* timer_ins = &timer_consumer;

    int32_t real_time_us =0;
    timer_ins->timeout_val = dest_val;
    timer_ins->usrdata     = usrdata;
    if(timer_ins->timer_run == true) {
        pal_logi("eaudio timer consumer timer is runing do not start again!!!");
        return;
    }

    real_time_us = dest_val;

    if(first_tick == -1 || current_tick == 0) {
        pal_logd("eaudio timer consumer first");
        first_tick = current_tick;
        last_tick = current_tick;
        cur_delta_us = 0;
        last_delta_us = 0;
        total_delta_us = 0;
        real_time_us = dest_val;
    } else {
        if(current_tick > last_tick) {

            //pal_logd("timer consume tune interval %d", 312*(current_tick-last_tick)+(current_tick-last_tick)/2);
            cur_delta_us = 312*(current_tick-last_tick)+(current_tick-last_tick)/2 - dest_val;
            total_delta_us += cur_delta_us;
            //pal_logd("cur_delta=%d current-last delta=%d",cur_delta_us, (cur_delta_us - last_delta_us));

            real_time_us = dest_val -
                (int32_t)(PID_T.Kp*cur_delta_us + PID_T.Ki*total_delta_us + PID_T.Kd*(cur_delta_us - last_delta_us));
            if(real_time_us < MIN_CONSUMER_INTERVAL) {
                real_time_us = MIN_CONSUMER_INTERVAL;
            }
            if(real_time_us > MAX_CONSUMER_INTERVAL) {
                real_time_us = MAX_CONSUMER_INTERVAL;
            }
            last_delta_us = cur_delta_us;
        }
    }

    last_tick = current_tick;
    if(timer_ins->timer_run == false) {
        //pal_logd("eaudio_timer_consumer_set time %d",real_time_us);
        hwtimer_start(timer_ins->cal_timer_id,US_TO_TICKS(real_time_us));
        timer_ins->timer_run  = true;
    }
}