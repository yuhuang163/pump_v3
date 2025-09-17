/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "rmt_trace_client.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

#ifndef RMT_TRC_BUF_SIZE
#define RMT_TRC_BUF_SIZE                    (1024 * 4)
#endif
#ifndef RMT_TRC_ENTRY_CNT
#define RMT_TRC_ENTRY_CNT                   100
#endif
#ifndef RMT_TRC_INTVL_MS
#ifdef SIMU
#define RMT_TRC_INTVL_MS                    2
#else
#define RMT_TRC_INTVL_MS                    100
#endif
#endif

#define RMT_CRASH_WAIT_TIMEOUT_MS           200

#define VAR_MAX_VALUE(a)                    ((1 << (sizeof(a) * 8)) - 1)

STATIC_ASSERT(RMT_TRC_BUF_SIZE < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->buf_size), "RMT_TRC_BUF_SIZE too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_cnt), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_rpos), "RMT_TRC_ENTRY_CNT too large");
STATIC_ASSERT(RMT_TRC_ENTRY_CNT < VAR_MAX_VALUE(((struct RMT_TRC_CTRL_T *)0)->entry_wpos), "RMT_TRC_ENTRY_CNT too large");

#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
#ifndef RMT_TRC_AON_BUF_SIZE
#define RMT_TRC_AON_BUF_SIZE                1024
#endif
#define RMT_TRC_CTRL_LOC
#define RMT_TRC_MSG_LOC                     AON_MBX_BSS_LOC

AON_MBX_BUF_LOC
static uint8_t aon_trc_buf[RMT_TRC_AON_BUF_SIZE];
AON_MBX_BSS_LOC
static uint16_t aon_trc_entry_list[2];
AON_MBX_BSS_LOC
static struct RMT_TRC_CTRL_T aon_trc_ctrl;
#else
#define RMT_TRC_CTRL_LOC                  SYNC_FLAGS_LOC
#define RMT_TRC_MSG_LOC                   SYNC_FLAGS_LOC
#endif

RMT_TRC_CTRL_LOC
static uint8_t trc_buf[RMT_TRC_BUF_SIZE];
RMT_TRC_CTRL_LOC
static uint16_t trc_entry_list[RMT_TRC_ENTRY_CNT];
RMT_TRC_CTRL_LOC
static struct RMT_TRC_CTRL_T trc_ctrl;
RMT_TRC_MSG_LOC
static struct RMT_TRC_MSG_T trc_msg;
RMT_TRC_MSG_LOC
static struct RMT_TRC_MSG_T flush_msg;

static bool in_crash;
static bool in_server_dump;
static bool trc_msg_sending;

static char trc_tag[16];
static uint8_t trc_tag_len;

static RMT_TRC_SEND_CB send_cb;
static RMT_TRC_TX_IRQ_RUN_CB tx_irq_run_cb;

#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
static int rmt_trace_fill_aon_buffer(void)
{
    uint32_t aon_entry_rpos, aon_entry_wpos;
    uint32_t entry_rpos, entry_wpos, next_rpos;
    uint32_t trace_start, trace_end;
    uint32_t len, aon_len;

    aon_trc_ctrl.discard_cnt_w = trc_ctrl.discard_cnt_w;

    aon_entry_rpos = aon_trc_ctrl.entry_rpos;
    aon_entry_wpos = aon_trc_ctrl.entry_wpos;
    if (aon_entry_rpos != aon_entry_wpos) {
        // Previous trace not processed yet. Just continue
        return 0;
    }

    entry_rpos = trc_ctrl.entry_rpos;
    entry_wpos = trc_ctrl.entry_wpos;
    if (entry_rpos >= ARRAY_SIZE(trc_entry_list) || entry_wpos >= ARRAY_SIZE(trc_entry_list)) {
        return 1;
    }

    trace_start = trc_entry_list[entry_rpos];
    if (trace_start > sizeof(trc_buf)) {
        return 2;
    }

    aon_len = 0;

    while (entry_rpos != entry_wpos) {
        next_rpos = entry_rpos + 1;
        if (next_rpos >= ARRAY_SIZE(trc_entry_list)) {
            next_rpos = 0;
        }
        trace_end = trc_entry_list[next_rpos];
        if (trace_end > sizeof(trc_buf)) {
            return 3;
        }
        if (trace_end <= trace_start || trace_start >= sizeof(trc_buf)) {
            // Trace starts from buffer head
            // Client trace will never wrap buffer so that one invokation of TRACE_OUTPUT can process one trace.
            trace_start = 0;
        }
        len = trace_end - trace_start;
        if (len) {
            if (len > sizeof(aon_trc_buf)) {
                len = sizeof(aon_trc_buf);
            }
            if (len + aon_len > sizeof(aon_trc_buf)) {
                break;
            }
            memcpy(&aon_trc_buf[aon_len], &trc_buf[trace_start], len);
            aon_len += len;
        }
        trace_start = trace_end;
        entry_rpos = next_rpos;
    }

    trc_ctrl.entry_rpos = entry_rpos;
    aon_trc_entry_list[aon_entry_wpos] = 0;
    aon_entry_wpos++;
    if (aon_entry_wpos >= ARRAY_SIZE(aon_trc_entry_list)) {
        aon_entry_wpos = 0;
    }
    aon_trc_entry_list[aon_entry_wpos] = aon_len;
    aon_trc_ctrl.entry_wpos = aon_entry_wpos;
    return 0;
}
#endif

static void rmt_wait_tx_msg_done(void)
{
    uint32_t time;

    time = hal_sys_timer_get();

    if (in_server_dump) {
        while (trc_ctrl.entry_rpos != trc_ctrl.entry_wpos &&
                (hal_sys_timer_get() - time < MS_TO_TICKS(RMT_CRASH_WAIT_TIMEOUT_MS))) {
#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
            rmt_trace_fill_aon_buffer();
#endif
        }
#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
        if (trc_ctrl.entry_rpos == trc_ctrl.entry_wpos) {
            aon_trc_ctrl.state = trc_ctrl.state;
        }
#endif
    } else {
        while (trc_msg_sending &&
                (hal_sys_timer_get() - time < MS_TO_TICKS(RMT_CRASH_WAIT_TIMEOUT_MS))) {
            tx_irq_run_cb();
        }
    }
}

static int send_trace_msg_raw(void)
{
    int ret;

    if (trc_msg_sending) {
        return 1;
    }

#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
    ret = rmt_trace_fill_aon_buffer();
    if (ret) {
        return ret;
    }
#endif

    ret = send_cb(&trc_msg, sizeof(trc_msg), NULL);
    if (ret == 0) {
        trc_msg_sending = true;
    }

    return 0;
}

static void rmt_send_trace_msg(void)
{
    uint32_t lock;
    int ret;

    if (in_server_dump) {
        rmt_wait_tx_msg_done();
        return;
    }

    lock = int_lock();

    ret = send_trace_msg_raw();
    if (ret) {
        goto _exit;
    }

    if (in_crash) {
        rmt_wait_tx_msg_done();
    }

_exit:
    int_unlock(lock);
}

static void rmt_trace_flush_handler(void)
{
    uint32_t lock;
    uint32_t entry_rpos, entry_wpos;
    uint32_t discard_cnt_r, discard_cnt_w;

    if (in_server_dump || in_crash) {
        return;
    }

    lock = int_lock();

    entry_rpos = trc_ctrl.entry_rpos;
    entry_wpos = trc_ctrl.entry_wpos;
    discard_cnt_r = trc_ctrl.discard_cnt_r;
    discard_cnt_w = trc_ctrl.discard_cnt_w;
    if (entry_rpos == entry_wpos && discard_cnt_r == discard_cnt_w) {
        goto _exit;
    }

    rmt_wait_tx_msg_done();

    rmt_send_trace_msg();

    rmt_wait_tx_msg_done();

    send_cb(&flush_msg, sizeof(flush_msg), NULL);

    rmt_wait_tx_msg_done();

_exit:
    int_unlock(lock);
}

#if (RMT_TRC_INTVL_MS > 0)
static HWTIMER_ID msg_timer;

static void msg_timer_handler(void *param)
{
    uint32_t lock;
    uint32_t entry_rpos, entry_wpos;
    uint32_t discard_cnt_r, discard_cnt_w;

    lock = int_lock();

    entry_rpos = trc_ctrl.entry_rpos;
    entry_wpos = trc_ctrl.entry_wpos;
    discard_cnt_r = trc_ctrl.discard_cnt_r;
    discard_cnt_w = trc_ctrl.discard_cnt_w;
    if (entry_rpos == entry_wpos && discard_cnt_r == discard_cnt_w) {
        goto _exit;
    }

    rmt_send_trace_msg();

_exit:
    int_unlock(lock);
}
#endif

static void rmt_trace_notify_handler(enum HAL_TRACE_STATE_T state)
{
    in_crash = true;

    trc_msg.id = RMT_TRC_MSG_ID_CRASH;

    if (state == HAL_TRACE_STATE_CRASH_ASSERT_START) {
        trc_ctrl.state = RMT_TRC_STATE_CRASH_ASSERT_START;
    } else if (state == HAL_TRACE_STATE_CRASH_FAULT_START) {
        trc_ctrl.state = RMT_TRC_STATE_CRASH_FAULT_START;
    } else {
        trc_ctrl.state = RMT_TRC_STATE_CRASH_END;
    }

#if (RMT_TRC_INTVL_MS > 0)
    hwtimer_stop(msg_timer);
#endif
    rmt_wait_tx_msg_done();

    rmt_send_trace_msg();
}

static int rmt_global_tag_handler(char *buf, unsigned int buf_len)
{
#ifdef USE_TRACE_ID
    if (buf_len == 0) {
        uint32_t tag = hal_trace_get_tag(buf);
        uint32_t cpu_id;

#ifdef RMT_TRC_CPU_ID
        cpu_id = RMT_TRC_CPU_ID;
#else
        cpu_id = get_cpu_id();
#endif
        hal_trace_set_tag(buf, SET_BITFIELD(tag, TAG_VAL_CPUID, cpu_id));
        return 0;
    }
#endif

    unsigned int len;

    if (buf_len && trc_tag_len) {
        len = (buf_len <= trc_tag_len) ? buf_len : trc_tag_len;
        memcpy(buf, trc_tag, len);
        return len;
    }

    return 0;
}

static void rmt_trace_output_handler(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t entry_rpos, entry_wpos;
    uint32_t entry_avail = 0;
    uint32_t buf_rpos, buf_wpos;
    uint32_t buf_avail = 0;
    uint32_t len;

    if (buf_len == 0) {
        return;
    }

    entry_rpos = trc_ctrl.entry_rpos;
    entry_wpos = trc_ctrl.entry_wpos;

    if (entry_wpos >= entry_rpos) {
        entry_avail = ARRAY_SIZE(trc_entry_list) - (entry_wpos - entry_rpos) - 1;
    } else {
        entry_avail = (entry_rpos - entry_wpos) - 1;
    }

    if (entry_avail < 1) {
        trc_ctrl.discard_cnt_w++;
        goto _tell_server;
    }

    buf_rpos = trc_entry_list[entry_rpos];
    buf_wpos = trc_entry_list[entry_wpos];
    if (buf_wpos >= buf_rpos) {
        // Not to wrap buffer -- simplify the atomic trace operation on server
        buf_avail = sizeof(trc_buf) - buf_wpos;
        if (buf_avail < buf_len && buf_rpos) {
            buf_wpos = 0;
            buf_avail = buf_rpos;
        }
    } else {
        buf_avail = (buf_rpos - buf_wpos);
    }

    if (buf_avail < buf_len) {
        trc_ctrl.discard_cnt_w++;
        goto _tell_server;
    }

    len = buf_len;
    memcpy(trc_buf + buf_wpos, buf, len);
    buf_wpos += buf_len;

    entry_wpos++;
    if (entry_wpos >= ARRAY_SIZE(trc_entry_list)) {
        entry_wpos -= ARRAY_SIZE(trc_entry_list);
    }

    trc_entry_list[entry_wpos] = buf_wpos;
    trc_ctrl.entry_wpos = entry_wpos;

_tell_server:
    // Send message to SERVER
    if (in_crash) {
        if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
            rmt_send_trace_msg();
        }
        return;
    }

#if (RMT_TRC_INTVL_MS > 0)
    if ((entry_avail < ARRAY_SIZE(trc_entry_list) / 2) || (buf_avail < sizeof(trc_buf) / 2)) {
        hwtimer_stop(msg_timer);
        rmt_send_trace_msg();
        return;
    }

    if (!hwtimer_active(msg_timer)) {
        hwtimer_start(msg_timer, MS_TO_TICKS(RMT_TRC_INTVL_MS));
    }
#else
    rmt_send_trace_msg();
#endif
}

int rmt_trace_client_open(const struct RMT_TRC_CLIENT_CFG_T *cfg)
{
    if (!cfg) {
        return 1;
    }
    if (!cfg->tag) {
        return 2;
    }
    if (!cfg->send_cb) {
        return 3;
    }
    if (!cfg->tx_irq_run_cb) {
        return 4;
    }

    int i = 0;
    while (i < sizeof(trc_tag) && cfg->tag[i]) {
        trc_tag[i] = cfg->tag[i];
        i++;
    }
    trc_tag_len = i;
    send_cb = cfg->send_cb;
    tx_irq_run_cb = cfg->tx_irq_run_cb;

    trc_msg.msg_magic = RMT_TRC_MSG_MAGIC_WORD;
    trc_msg.id = RMT_TRC_MSG_ID_TRACE;
#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
    trc_msg.param = &aon_trc_ctrl;
#else
    trc_msg.param = ADDR_CPU_TO_DEV(&trc_ctrl);
#endif

    flush_msg.msg_magic = RMT_TRC_MSG_MAGIC_WORD;
    flush_msg.id = RMT_TRC_MSG_ID_FLUSH;
    flush_msg.param = NULL;

    trc_ctrl.ctrl_magic = RMT_TRC_CTRL_MAGIC_WORD;
    trc_ctrl.buf_start = ADDR_CPU_TO_DEV(&trc_buf[0]);
    trc_ctrl.entry_start = ADDR_CPU_TO_DEV(&trc_entry_list[0]);
    trc_ctrl.buf_size = sizeof(trc_buf);
    trc_ctrl.entry_cnt = ARRAY_SIZE(trc_entry_list);
    trc_ctrl.entry_rpos = 0;
    trc_ctrl.entry_wpos = 0;
    trc_ctrl.discard_cnt_r = 0;
    trc_ctrl.discard_cnt_w = 0;
    trc_ctrl.server_discard_cnt = 0;
    trc_ctrl.state = RMT_TRC_STATE_TRACE;

    trc_entry_list[0] = 0;
    trc_entry_list[1] = 0;

#if defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0)
    aon_trc_ctrl.ctrl_magic = RMT_TRC_CTRL_MAGIC_WORD;
    aon_trc_ctrl.buf_start = &aon_trc_buf[0];
    aon_trc_ctrl.entry_start =&aon_trc_entry_list[0];
    aon_trc_ctrl.buf_size = sizeof(aon_trc_buf);
    aon_trc_ctrl.entry_cnt = ARRAY_SIZE(aon_trc_entry_list);
    aon_trc_ctrl.entry_rpos = 0;
    aon_trc_ctrl.entry_wpos = 0;
    aon_trc_ctrl.discard_cnt_r = 0;
    aon_trc_ctrl.discard_cnt_w = 0;
    aon_trc_ctrl.server_discard_cnt = 0;
    aon_trc_ctrl.state = RMT_TRC_STATE_TRACE;

    aon_trc_entry_list[0] = 0;
    aon_trc_entry_list[0] = 0;
#endif

    in_crash = false;
    in_server_dump = false;
    trc_msg_sending = false;

#if (RMT_TRC_INTVL_MS > 0)
    msg_timer = hwtimer_alloc(msg_timer_handler, NULL);
    ASSERT(msg_timer, "%s: Failed to alloc msg_timer", __func__);
#endif

    hal_trace_global_tag_register(rmt_global_tag_handler);
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_0, rmt_trace_notify_handler, rmt_trace_output_handler);
    hal_trace_app_flush_register(rmt_trace_flush_handler);

    return 0;
}

unsigned int rmt_trace_client_rx_handler(const void *data, unsigned int len)
{
    const struct RMT_TRC_MSG_T *msg;

#ifdef RMT_TRC_IN_MSG_CHAN
    if (!rmt_trace_check_trace(data, len)) {
        return 0;
    }
#endif

    ASSERT(data, "%s: data ptr null", __func__);
    ASSERT(len == sizeof(*msg), "%s: Bad msg len %u (expecting %u)", __func__, len, sizeof(*msg));

    msg = (const struct RMT_TRC_MSG_T *)data;
    ASSERT((enum RMT_TRC_MSG_ID_T)msg->id == RMT_TRC_MSG_ID_SERVER_DUMP, "%s: Bad msg id %d (expecting %d)", __func__, msg->id, RMT_TRC_MSG_ID_SERVER_DUMP);

    in_server_dump = true;

    ASSERT(false, "%s: trace server dump", __func__);

    return len;
}

void rmt_trace_client_tx_handler(const void *data, unsigned int len)
{
    uint32_t lock;
    uint32_t entry_rpos, entry_wpos;

#if 0
    if (!rmt_trace_check_trace(data, len)) {
        return;
    }
#endif
    if (data != &trc_msg) {
        return;
    }

    lock = int_lock();

    trc_msg_sending = false;

    entry_rpos = trc_ctrl.entry_rpos;
    entry_wpos = trc_ctrl.entry_wpos;

    if (entry_rpos != entry_wpos) {
        send_trace_msg_raw();
    }

    int_unlock(lock);
}

int rmt_trace_client_tx_handler_ext(const void *data, unsigned int len)
{
    if (rmt_trace_check_trace(data, len)) {
        rmt_trace_client_tx_handler(data, len);
        return len;
    }

    return 0;
}
