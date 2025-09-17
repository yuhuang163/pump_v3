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
#include "video_stream.h"

enum VIDEO_STREAM_CHAN_STATUS_T {
    VIDEO_STREAM_CHAN_STATUS_NULL = 0x00,
    VIDEO_STREAM_CHAN_STATUS_INIT = 0x01,
    VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE = 0x02,
    VIDEO_STREAM_CHAN_STATUS_START_STOP = 0x04,
};

struct VIDEO_STREAM_CHAN_CNTL_T {
    uint32_t status;
    uint32_t seq;
};

struct VIDEO_STREAM_CHAN_DATA_T {
    uint32_t addr;
    uint32_t size;
};

struct VIDEO_STREAM_CHAN_T {
    uint32_t open_num;
    struct VIDEO_STREAM_CHAN_DATA_T data;
    struct VIDEO_STREAM_CHAN_CNTL_T cntl[VIDEO_STREAM_CHAN_ID_QTY];
    VIDEO_STREAM_CHAN_HDLR_T hdlr[VIDEO_STREAM_CHAN_ID_QTY];
};


struct VIDEO_STREAM_CHAN_T g_vs_chan[VP_STREAM_QTY];
#define GET_VS_CHAN(stream) &g_vs_chan[(stream)];

#ifdef RTOS
osMutexId vs_mutex_id = NULL;
osMutexDef(vs_mutex);
#else
static volatile uint32_t vs_flag_lock = 0;
#endif

static void vs_lock_thread(void)
{
#ifdef RTOS
    osMutexWait(vs_mutex_id, osWaitForever);
#else
    ASSERT(vs_flag_lock == 0, "[%s]: thread has been locked", __func__);
    vs_flag_lock = 1;
#endif
}

static void vs_unlock_thread(void)
{
#ifdef RTOS
    osMutexRelease(vs_mutex_id);
#else
    ASSERT(vs_flag_lock == 1, "[%s]: thread not locked", __func__);
    vs_flag_lock = 0;
#endif
}

static void vs_set_flag(enum VP_STREAM_T stream, enum VIDEO_STREAM_CHAN_ID_T id, uint32_t flag)
{
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);
    c->cntl[id].status |= flag;
}

static void vs_clear_flag(enum VP_STREAM_T stream, enum VIDEO_STREAM_CHAN_ID_T id, uint32_t flag)
{
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);
    c->cntl[id].status &= ~flag;
}

void video_stream_chan_init(void)
{
    uint32_t i, j;
    struct VIDEO_STREAM_CHAN_T *c;

    TRACE(1, "[%s]:", __func__);
#ifdef RTOS
    if (vs_mutex_id == NULL) {
        vs_mutex_id = osMutexCreate((osMutex(vs_mutex)));
    }
#endif

    vs_lock_thread();

    for (i = 0; i < VP_STREAM_QTY; i++) {
        c = GET_VS_CHAN(i);
        c->data.addr = 0;
        c->data.size = 0;
        c->open_num = 0;
        for (j = 0; j < VIDEO_STREAM_CHAN_ID_QTY; j++) {
            if (c->cntl[j].status == VIDEO_STREAM_CHAN_STATUS_NULL) {
                c->cntl[j].status = VIDEO_STREAM_CHAN_STATUS_INIT;
                c->cntl[j].seq = 0;
                c->hdlr[j] = NULL;
            }
        }
    }

    vs_unlock_thread();
}

int video_stream_chan_open(enum VP_STREAM_T stream,
    enum VIDEO_STREAM_CHAN_ID_T id, VIDEO_STREAM_CHAN_HDLR_T handler)
{
    int ret = 0;
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);

    TRACE(1, "[%s]:stream=%d, id=%d, handler=%x", __func__, stream, id, (int)handler);

    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream %d", __func__, stream);
    ASSERT(id < VIDEO_STREAM_CHAN_ID_QTY, "[%s]: Bad id %d", __func__, id);

    vs_lock_thread();

    if (c->cntl[id].status != VIDEO_STREAM_CHAN_STATUS_INIT) {
        TRACE(1, "[%s]: Bad status %x", __func__, c->cntl[id].status);
        ret = -1;
        goto _exit;
    }

    c->hdlr[id] = handler;
    vs_set_flag(stream, id, VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE);
    c->open_num++;
_exit:
    vs_unlock_thread();
    return ret;
}

int video_stream_chan_close(enum VP_STREAM_T stream, enum VIDEO_STREAM_CHAN_ID_T id)
{
    int ret = 0;
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);

    TRACE(1, "[%s]:stream=%d, id=%d", __func__, stream, id);
    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream %d", __func__, stream);
    ASSERT(id < VIDEO_STREAM_CHAN_ID_QTY, "[%s]: Bad id %d", __func__, id);

    vs_lock_thread();

    if (c->cntl[id].status != (VIDEO_STREAM_CHAN_STATUS_INIT | VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE)) {
        TRACE(1, "[%s]: Bad status %x", __func__, c->cntl[id].status);
        ret = -1;
        goto _exit;
    }

    c->hdlr[id] = NULL;
    vs_clear_flag(stream, id, VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE);
    if (c->open_num > 0) {
        c->open_num--;
    }
_exit:
    vs_unlock_thread();
    return ret;
}

int video_stream_chan_start(enum VP_STREAM_T stream, enum VIDEO_STREAM_CHAN_ID_T id)
{
    int ret = 0;
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);

    TRACE(1, "[%s]:stream=%d, id=%d", __func__, stream, id);
    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream %d", __func__, stream);
    ASSERT(id < VIDEO_STREAM_CHAN_ID_QTY, "[%s]: Bad id %d", __func__, id);

    vs_lock_thread();

    if (c->cntl[id].status != (VIDEO_STREAM_CHAN_STATUS_INIT | VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE)) {
        TRACE(1, "[%s]: Bad status %x", __func__, c->cntl[id].status);
        ret = -1;
        goto _exit;
    }
    c->cntl[id].seq = 0;
    vs_set_flag(stream, id, VIDEO_STREAM_CHAN_STATUS_START_STOP);

_exit:
    vs_unlock_thread();
    return ret;
}

int video_stream_chan_stop(enum VP_STREAM_T stream, enum VIDEO_STREAM_CHAN_ID_T id)
{
    int ret = 0;
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);
    uint32_t msk;

    TRACE(1, "[%s]:stream=%d, id=%d", __func__, stream, id);
    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream %d", __func__, stream);
    ASSERT(id < VIDEO_STREAM_CHAN_ID_QTY, "[%s]: Bad id %d", __func__, id);

    vs_lock_thread();

    msk = VIDEO_STREAM_CHAN_STATUS_INIT
        | VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE
        | VIDEO_STREAM_CHAN_STATUS_START_STOP;
    if (c->cntl[id].status != msk) {
        TRACE(1, "[%s]: Bad status %x", __func__, c->cntl[id].status);
        ret = -1;
        goto _exit;
    }

    vs_clear_flag(stream, id, VIDEO_STREAM_CHAN_STATUS_START_STOP);

_exit:
    vs_unlock_thread();
    return ret;
}

int video_stream_chan_data_handler(enum VP_STREAM_T stream, uint8_t *buf, uint32_t len)
{
    int ret = 0;
    struct VIDEO_STREAM_CHAN_T *c = GET_VS_CHAN(stream);
    uint32_t i, status;
    uint32_t msk;
    uint32_t lock;

    if (c->open_num == 0) {
        goto _exit;
    }

    msk = VIDEO_STREAM_CHAN_STATUS_INIT
        | VIDEO_STREAM_CHAN_STATUS_OPEN_CLOSE
        | VIDEO_STREAM_CHAN_STATUS_START_STOP;

    c->data.addr = (uint32_t)buf;
    c->data.size = (uint32_t)len;
    for (i = 0; i < VIDEO_STREAM_CHAN_ID_QTY; i++) {
        status = c->cntl[i].status;
        if (status != msk) {
            continue;
        }
        lock = int_lock();
        VIDEO_STREAM_CHAN_HDLR_T hdlr = c->hdlr[i];
        c->cntl[i].seq += 1;
        int_unlock(lock);
        if (hdlr) {
            hdlr(c->data.addr, c->data.size, c->cntl[i].seq);
        }
    }

_exit:
    return ret;
}

