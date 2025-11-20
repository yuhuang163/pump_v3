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
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rmt_trace_server.h"
#include "string.h"

#ifndef RMT_TRC_SERVER_DUMP_CB_CNT
#define RMT_TRC_SERVER_DUMP_CB_CNT          4
#endif

#define RMT_SERVER_DUMP_WAIT_TIMEOUT_MS     200

static RMT_TRC_SERVER_DUMP_CB svr_dump_cb[RMT_TRC_SERVER_DUMP_CB_CNT];

static void trace_msg_handler(struct RMT_TRC_SERVER_CFG_T *cfg, struct RMT_TRC_CTRL_T *ctrl)
{
    uint32_t entry_rpos, entry_wpos, next_rpos;
    uint32_t trace_start, trace_end;
    uint16_t discard_cnt_w;
    uint16_t discard_cnt;
    uint16_t server_discard_cnt;
    uint32_t len;
    int proc_len;
    int ret;
    int done;

    //TRACE(0, "Rx %s trace", cfg->name);

    if (cfg->first_msg) {
        cfg->first_msg = false;
        cfg->buf_start = ctrl->buf_start;
        cfg->entry_start = ctrl->entry_start;
        cfg->buf_total_size = ctrl->buf_size;
        cfg->entry_total_cnt = ctrl->entry_cnt;
        cfg->ctrl = ctrl;
    }

_loop:
    done = false;

    discard_cnt_w = ctrl->discard_cnt_w;
    discard_cnt = discard_cnt_w - ctrl->discard_cnt_r;

    if (discard_cnt) {
        ret = TRACE(0, "*** WARNING: %s TRACE LOST %u", cfg->name, discard_cnt);
        if (ret > 0) {
            ctrl->discard_cnt_r = discard_cnt_w;
        }
    }

    server_discard_cnt = ctrl->server_discard_cnt;
    if (server_discard_cnt) {
        ret = TRACE(0, "*** WARNING: %s TRACE SERVER LOST %u", cfg->name, server_discard_cnt);
        if (ret > 0) {
            server_discard_cnt = 0;
        }
    }

    entry_rpos = ctrl->entry_rpos;
    entry_wpos = ctrl->entry_wpos;
    if (entry_rpos >= cfg->entry_total_cnt || entry_wpos >= cfg->entry_total_cnt) {
        TRACE(0, "*** WARNING: %s Bad entry rpos=%u wpos=%u (should < %u)", cfg->name, entry_rpos, entry_wpos, cfg->entry_total_cnt);
        goto _exit;
    }

    trace_start = cfg->entry_start[entry_rpos];
    if (trace_start > cfg->buf_total_size) {
        TRACE(0, "*** WARNING: %s Bad trace_start=%u (should <= %u) entry_rpos=%u", cfg->name, trace_start, cfg->buf_total_size, entry_rpos);
        goto _exit;
    }

    while (entry_rpos != entry_wpos) {
        next_rpos = entry_rpos + 1;
        if (next_rpos >= cfg->entry_total_cnt) {
            next_rpos = 0;
        }
        trace_end = cfg->entry_start[next_rpos];
        if (trace_end > cfg->buf_total_size) {
            TRACE(0, "*** WARNING: %s Bad trace_end=%u (should <= %u) next_rpos=%u", cfg->name, trace_end, cfg->buf_total_size, next_rpos);
            goto _exit;
        }
        if (trace_end <= trace_start || trace_start >= cfg->buf_total_size) {
            // Trace starts from buffer head
            // Client trace will never wrap buffer so that one invokation of TRACE_OUTPUT can process one trace.
            trace_start = 0;
        }
        len = trace_end - trace_start;
        if (len) {
            proc_len = TRACE_OUTPUT(cfg->buf_start + trace_start, len);
            if (proc_len < len) {
                server_discard_cnt++;
            }
        }
        trace_start = trace_end;
        entry_rpos = next_rpos;
    }

    done = true;

_exit:
    ctrl->entry_rpos = entry_rpos;
    ctrl->server_discard_cnt = server_discard_cnt;

    if (done) {
        // Normal exit: Check if there is any new trace
        if (entry_wpos != ctrl->entry_wpos) {
            goto _loop;
        }
    }
}

unsigned int rmt_trace_server_msg_handler(struct RMT_TRC_SERVER_CFG_T *cfg, const void *data, unsigned int len)
{
    const struct RMT_TRC_MSG_T *trc_msg;
    struct RMT_TRC_CTRL_T *ctrl;
    enum RMT_TRC_MSG_ID_T id;

    ASSERT(cfg, "%s: cfg ptr null", __func__);

#ifdef RMT_TRC_IN_MSG_CHAN
    if (cfg->in_msg_chan && !rmt_trace_check_trace(data, len)) {
        return 0;
    }
#endif

    ASSERT(data, "%s:%s: data ptr null", __func__, cfg->name);
    ASSERT(len == sizeof(*trc_msg), "%s:%s: Bad msg len %u (expecting %u)", __func__, cfg->name, len, sizeof(*trc_msg));

    trc_msg = (const struct RMT_TRC_MSG_T *)data;
    id = (enum RMT_TRC_MSG_ID_T)trc_msg->id;
    switch (id) {
    case RMT_TRC_MSG_ID_FLUSH:
        TRACE_FLUSH();
        //TRACE(0, "%s:%s: Got FLUSH msg", __func__, cfg->name);
        break;
    case RMT_TRC_MSG_ID_CRASH:
        TRACE_FLUSH();
        // Trace msg will be changed to crash msg after crash occurs
        // FALL THROUGH
    case RMT_TRC_MSG_ID_TRACE:
        ctrl = (struct RMT_TRC_CTRL_T *)trc_msg->param;
#ifndef SYSTEMVIEW_TO_UART
        trace_msg_handler(cfg, ctrl);
#endif
        break;
    default:
        ASSERT(false, "%s:%s: Bad trc msg id: %d", __func__, cfg->name, id)
    }

    return len;
}

void rmt_trace_server_dump(struct RMT_TRC_SERVER_CFG_T *cfg)
{
#if (defined(AON_MAILBOX_SIZE) && (AON_MAILBOX_SIZE > 0))
    AON_MBX_BSS_LOC static
#endif
    struct RMT_TRC_MSG_T server_dump_msg;

    memset(&server_dump_msg, 0, sizeof(server_dump_msg));
    server_dump_msg.msg_magic = RMT_TRC_MSG_MAGIC_WORD;
    server_dump_msg.id = RMT_TRC_MSG_ID_SERVER_DUMP;
    server_dump_msg.param = NULL;

    TRACE_IMM(0, " ");
    TRACE_IMM(0, "--------------------------------");
    TRACE_IMM(0, "Dump log for %s:", cfg->name);
    TRACE_IMM(0, " ");
    //TRACE_IMM(0, "(send_cb=%p ctrl=%p)", cfg->send_cb, cfg->ctrl);

    if (cfg->send_cb && cfg->ctrl) {
        cfg->send_cb(&server_dump_msg, sizeof(server_dump_msg), NULL);
        // Wait for client wakeup
        hal_sys_timer_delay(MS_TO_TICKS(10));
        // Check ram validity
        if (cfg->ctrl->ctrl_magic == RMT_TRC_CTRL_MAGIC_WORD) {
            uint32_t time;
            struct RMT_TRC_CTRL_T *ctrl;

            time = hal_sys_timer_get();
            ctrl = cfg->ctrl;
            while (!((enum RMT_TRC_STATE_T)ctrl->state == RMT_TRC_STATE_CRASH_END &&
                    ctrl->entry_rpos == ctrl->entry_wpos) &&
                    (hal_sys_timer_get() - time < MS_TO_TICKS(RMT_SERVER_DUMP_WAIT_TIMEOUT_MS))) {
                trace_msg_handler(cfg, ctrl);
            }
        }
    }

    TRACE_IMM(0, " ");
    TRACE_IMM(0, "--------------------------------");
    TRACE_IMM(0, " ");
}

static void rmt_trace_server_dump_callback(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(svr_dump_cb); i++) {
        if (svr_dump_cb[i]) {
            svr_dump_cb[i]();
        }
    }
}

int rmt_trace_server_dump_register(RMT_TRC_SERVER_DUMP_CB cb)
{
    int i;

    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_RMT_TRC, rmt_trace_server_dump_callback);

    for (i = 0; i < ARRAY_SIZE(svr_dump_cb); i++) {
        if (svr_dump_cb[i] == NULL) {
            svr_dump_cb[i] = cb;
            return 0;
        }
    }

    return 1;
}

int rmt_trace_server_dump_deregister(RMT_TRC_SERVER_DUMP_CB cb)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(svr_dump_cb); i++) {
        if (svr_dump_cb[i] == cb) {
            svr_dump_cb[i] = NULL;
            return 0;
        }
    }

    return 1;
}

void rmt_trace_server_show_ctrl(const struct RMT_TRC_SERVER_CFG_T *cfg)
{
    const struct RMT_TRC_CTRL_T *ctrl = cfg->ctrl;

    if (!ctrl) {
        TRACE(0, "%s not got trace yet", cfg->name);
        return;
    }

    TRACE(0, "%s entry=%u/%u discard=%u/%u svr_discard=%u",
        cfg->name, ctrl->entry_rpos, ctrl->entry_wpos,
        ctrl->discard_cnt_r, ctrl->discard_cnt_w, ctrl->server_discard_cnt);
}
