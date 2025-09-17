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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "video_player.h"
#include "video_stream.h"
#include "video_misc.h"
#include "video_record.h"
#include "cqueue.h"

#define VIDEO_STREAM_CHAN_REC VIDEO_STREAM_CHAN_ID_1

#define VIDEO_RECORD_MSG_LEN_SIZE        2
#define VIDEO_RECORD_QUEUE_BUFF_SIZE     0x100
#define VIDEO_RECORD_SIGNAL_DATA_ENQUEUE 0x01

#define VIDEO_RECORD_STACK_SIZE (0x1000)

#define VIDEO_REC_DBG() {TRACE(1, "[%s]", __func__);}

#ifdef RTOS
static osThreadId rec_thread_tid = NULL;
static void video_record_thread(void const *arguments);
osThreadDef(video_record_thread, osPriorityHigh, 1, VIDEO_RECORD_STACK_SIZE, "video_record");
#else
static volatile uint32_t vp_flag_lock = 0;
static volatile uint32_t vp_flag_signal = 0;
static volatile uint32_t vp_flag_open = 0;
#endif

static bool video_record_inited = false;
static CQueue g_vrqueue;
static uint8_t video_record_queue_buff[VIDEO_RECORD_QUEUE_BUFF_SIZE];
static uint32_t video_enqueue_seq = 0;
static uint32_t video_dequeue_seq = 0;
static bool video_queue_debug = false;

static void video_record_queue_init(CQueue *q, uint8_t *buf, uint32_t len)
{
    uint32_t lock = int_lock();

    memset(buf, 0, len);
    InitCQueue(q, len, (CQItemType *)buf);
    int_unlock(lock);

    TRACE(1, "[%s]: buf=%x, len=%d", __func__, (int)buf, len);
}

static void video_record_queue_dump(CQueue *q)
{
    TRACE(1, "dump_cqueue: read=%d, write=%d, len=%d, size=%d",
        q->read, q->write, q->len, q->size);
    TRACE(1, "queue_seq: enqueue_seq=%d, dequeue_seq=%d",
        video_enqueue_seq, video_dequeue_seq);
}

static int video_record_enqueue(CQueue *q, const void *data, uint32_t len)
{
    int ret = 0, avail = 0;
    uint32_t lock;
    uint16_t dlen = (uint16_t)len;

    video_enqueue_seq++;

    if (len > 0) {
        lock = int_lock();
        avail = AvailableOfCQueue(q);
        if (dlen <= avail) {
            ret = EnCQueue(q, (CQItemType *)data, dlen);
        } else {
            ret = -1;
        }
        int_unlock(lock);
    }
    if (ret) {
        TRACE(1, "[%s]: WARNING: Enqueue err=%d, avail=%d, len=%d, seq=%d",
            __func__, ret, avail, len, video_enqueue_seq);
    }
    if (video_queue_debug) {
        video_record_queue_dump(q);
    }
    return ret;
}

static POSSIBLY_UNUSED int video_record_dequeue(CQueue *q, uint8_t *buf, uint32_t len)
{
    int ret = 0;
    uint32_t lock;

    video_dequeue_seq++;

    if (len > 0) {
        lock = int_lock();
        ret = DeCQueue(q, (CQItemType *)buf, len);
        int_unlock(lock);
    }
    if (ret) {
        TRACE(1, "[%s]: WARNING: Dequeue err=%d, buf=%x, len=%d", __func__, ret, (int)buf, len);
    }
    if (video_queue_debug) {
        video_record_queue_dump(q);
    }
    return ret;
}

static void camera_stream_data_record_handler(uint32_t addr, uint32_t len, uint32_t seq)
{
    video_record_msg_t msg;

    msg.addr = addr;
    msg.len = len;
    msg.seq = seq;

    video_record_ping(&msg);
    TRACE(1, "[CSD_RECORD]: addr=%x, len=%x, seq=%d", msg.addr, msg.len, msg.seq);
}

int video_record_init(void)
{
    VIDEO_REC_DBG();

    if (video_record_inited) {
        return 0;
    }

#ifdef RTOS
    if (rec_thread_tid == NULL) {
        rec_thread_tid = osThreadCreate(osThread(video_record_thread), NULL);
        osSignalSet(rec_thread_tid, 0x0);
    }
#endif

    video_record_queue_init(&g_vrqueue, video_record_queue_buff, VIDEO_RECORD_QUEUE_BUFF_SIZE);
    video_record_inited = true;
    return 0;
}

int video_record_stream_handler(video_record_msg_t *msg)
{
    TRACE(1, "[%s]: addr=%x, len=%d, seq=%d", __func__, msg->addr, msg->len, msg->seq);

#if 0
    if (msg->addr == 0 || msg->len == 0) {
        video_queue_debug = true;
        video_record_queue_dump(&g_vrqueue);
    }
#endif

    //TODO: H264 encoder run here

    return 0;
}

#ifdef RTOS
static void video_record_thread(void const *arguments)
{
    osEvent evt;
    uint32_t signals;
    video_record_msg_t msg;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        if (evt.status == osEventSignal) {
            if (signals & VIDEO_RECORD_SIGNAL_DATA_ENQUEUE) {
                video_record_dequeue(&g_vrqueue, (uint8_t *)&msg, sizeof(video_record_msg_t));
                video_record_stream_handler(&msg);
            }
        }
    }
}

#else
void video_record_thread(void const *arguments)
{
    //TODO: process stream data
}

#endif

int video_record_open(struct VP_STREAM_CONFIG_T *cfg)
{
    VIDEO_REC_DBG();
    ASSERT(video_record_inited != false, "[%s]: not inited", __func__);
    video_stream_chan_open(VP_STREAM_CAPTURE,
        VIDEO_STREAM_CHAN_REC, camera_stream_data_record_handler);

    return 0;
}

int video_record_start(void)
{
    VIDEO_REC_DBG();
    ASSERT(video_record_inited != false, "[%s]: not inited", __func__);
    video_stream_chan_start(VP_STREAM_CAPTURE, VIDEO_STREAM_CHAN_REC);
    return 0;
}

int video_record_stop(void)
{
    VIDEO_REC_DBG();
    ASSERT(video_record_inited != false, "[%s]: not inited", __func__);
    video_stream_chan_stop(VP_STREAM_CAPTURE, VIDEO_STREAM_CHAN_REC);
    return 0;
}

int video_record_close(void)
{
    VIDEO_REC_DBG();
    ASSERT(video_record_inited != false, "[%s]: not inited", __func__);
    video_stream_chan_close(VP_STREAM_CAPTURE, VIDEO_STREAM_CHAN_REC);
    return 0;
}

int video_record_ping(video_record_msg_t *msg)
{
    ASSERT(video_record_inited != false, "[%s]: not inited", __func__);
    video_record_enqueue(&g_vrqueue, (const void *)msg, sizeof(video_record_msg_t));
#ifdef RTOS
    osSignalSet(rec_thread_tid, VIDEO_RECORD_SIGNAL_DATA_ENQUEUE);
#endif
    return 0;
}

