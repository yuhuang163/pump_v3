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
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "csi_dsi_conf.h"
#include "csi_fb.h"

#ifdef PSRAMUHS_ENABLE
#define DSI_FB_UPDATE_DMA_DATA 0
#define DSI_FB_UPDATE_DMA_ADDR 1
#else
#define DSI_FB_UPDATE_DMA_DATA 0
#define DSI_FB_UPDATE_DMA_ADDR 0
#endif

//#define DSI_FB_DEBUG

#ifndef CSI_FB_DEBUG
#define CSI_FB_DEBUG
#endif

static struct CSI_DSI_FB_CNTL_T csi_fb_cntl;
static struct CSI_DSI_FB_CNTL_T dsi_fb_cntl[2];

void dsi_fb_init(enum DSI_FB_CH_T chan)
{
    struct CSI_DSI_FB_CNTL_T *c;
    uint32_t lock;

    ASSERT(chan < 2, "[%s]: Bad chan %d", __func__, chan);

    lock = int_lock();
    c = &dsi_fb_cntl[chan];
    c->enable = 0;
    c->flush  = 0;
    c->lock   = 0;
    c->updata = DSI_FB_UPDATE_DMA_DATA;
    c->upaddr = DSI_FB_UPDATE_DMA_ADDR;
    c->addr   = 0;
    c->len   = 0;
    c->err   = 0;
    c->cnt   = 0;
    int_unlock(lock);

    TRACE(1, "[%s]: fb[%d], updata=%d, upaddr=%d", __func__, chan, c->updata, c->upaddr);
}

void dsi_fb_enable(enum DSI_FB_CH_T chan)
{
    ASSERT(chan < 2, "[%s]: Bad chan %d", __func__, chan);
    dsi_fb_cntl[chan].enable = 1;
}

void dsi_fb_disable(enum DSI_FB_CH_T chan)
{
    ASSERT(chan < 2, "[%s]: Bad chan %d", __func__, chan);
    dsi_fb_cntl[chan].enable = 0;
}

int dsi_fb_callback(uint8_t layer, uint8_t chan, uint32_t addr)
{
    struct CSI_DSI_FB_CNTL_T *c;
    int done = 0;
    uint32_t lock = 0;

    ASSERT(chan < 2, "[%s]: Bad chan %d", __func__, chan);
    c = &dsi_fb_cntl[chan];
    if (!c->enable) {
#ifdef DSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: DSI FB not enable", __func__);
#endif
        return 0;
    }
    if (c->flush) {
#ifdef DSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: DSI FB not flush", __func__);
#endif
        return 0;
    }
    if (c->lock) {
#ifdef DSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: DSI FB lock", __func__);
#endif
        return 0;
    }
    lock = int_lock();
    c->addr = addr;
    c->len  = 0x100000;
    c->err  = 0;
    c->cnt++;
    c->flush = 1;
    done = 1;
    int_unlock(lock);
    return done;
}

struct CSI_DSI_FB_CNTL_T *dsi_fb_get(enum DSI_FB_CH_T chan)
{
    ASSERT(chan < 2, "[%s]: Bad chan %d", __func__, chan);
    return &dsi_fb_cntl[chan];
}

void csi_fb_init(void)
{
    uint32_t lock = int_lock();
    csi_fb_cntl.enable = 0;
    csi_fb_cntl.flush  = 0;
    csi_fb_cntl.lock   = 0;
    csi_fb_cntl.addr   = 0;
    csi_fb_cntl.len   = 0;
    csi_fb_cntl.err   = 0;
    csi_fb_cntl.cnt   = 0;
    int_unlock(lock);
}

void csi_fb_enable(void)
{
    csi_fb_cntl.enable = 1;
}

void csi_fb_disable(void)
{
    csi_fb_cntl.enable = 0;
}

int csi_fb_callback(uint8_t *buf, uint32_t len, uint32_t errcode)
{
    int done = 0;
    uint32_t lock = 0;

    if (!csi_fb_cntl.enable) {
#ifdef CSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: CSI FB not enable", __func__);
#endif
        return 0;
    }
    if (csi_fb_cntl.flush) {
#ifdef CSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: CSI FB not flush", __func__);
#endif
        return 0;
    }
    if (csi_fb_cntl.lock) {
#ifdef CSI_FB_DEBUG
        TRACE(0, "[%s]: WARNING: CSI FB lock", __func__);
#endif
        return 0;
    }
    lock = int_lock();
    csi_fb_cntl.addr = (uint32_t)buf;
    csi_fb_cntl.len  = len;
    csi_fb_cntl.err  = errcode;
    csi_fb_cntl.cnt++;
    csi_fb_cntl.flush = 1;
    done = 1;
    int_unlock(lock);
    return done;
}

struct CSI_DSI_FB_CNTL_T *csi_fb_get(void)
{
    return &csi_fb_cntl;
}

uint32_t csi_fb_get_addr(uint32_t *plen)
{
    uint32_t addr = 0;

    if (plen) {
        *plen = csi_fb_cntl.len;
    }
    addr = csi_fb_cntl.addr;
    return addr;
}

int csi_fb_flush_status(void)
{
    return csi_fb_cntl.flush;
}

