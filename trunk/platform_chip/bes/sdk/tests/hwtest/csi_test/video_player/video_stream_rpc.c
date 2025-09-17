/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#if defined(CHIP_ROLE_AP) || defined(CHIP_ROLE_DSP)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "video_stream_rpc.h"
#include "a7_cmd.h"
//#define VP_STREAM_RPC_DEBUG

static SYNC_FLAGS_LOC struct RPC_REQ_MSG_T vp_req_msg;
static SYNC_FLAGS_LOC struct RPC_RSP_MSG_T vp_rsp_msg;
static SYNC_FLAGS_LOC uint32_t vp_rsp_msg_cnt = 0;
static SYNC_FLAGS_LOC VP_RPC_MSG_DATA_HDLR_T rpc_msg_rx_handler[RPC_MSG_RX_HDLR_ID_NUM] = {0};

static int rpc_msg_send(struct RPC_REQ_MSG_T *msg)
{
    int err = 0;
    A7_CMD_T vp_tx_msg;

    struct RPC_REQ_MSG_T *p = (struct RPC_REQ_MSG_T *)vp_tx_msg.str;
    vp_tx_msg.type = A7_CMD_TYPE_CSI_TEST;
    vp_tx_msg.p1 = 0;
    vp_tx_msg.p2 = 0;
    *p = *msg;

#ifdef CHIP_ROLE_AP
    mcu_cmd_send(&vp_tx_msg);
#else
    a7_cmd_send2(&vp_tx_msg);
#endif

    return err;
}

#define CASE_ENUM(e) case e: return "["#e"]"
static const char *rpc_cmd_to_str(enum RPC_MSG_CMD_T cmd)
{
    switch (cmd) {
    CASE_ENUM(RPC_MSG_VP_OPEN);
    CASE_ENUM(RPC_MSG_VP_CLOSE);
    CASE_ENUM(RPC_MSG_VP_STREAM_OPEN);
    CASE_ENUM(RPC_MSG_VP_STREAM_START);
    CASE_ENUM(RPC_MSG_VP_STREAM_STOP);
    CASE_ENUM(RPC_MSG_VP_STREAM_CLOSE);
    CASE_ENUM(RPC_MSG_VP_STREAM_SET_CFG);
    CASE_ENUM(RPC_MSG_VP_STREAM_GET_CFG);
    CASE_ENUM(RPC_MSG_VP_STREAM_DATA_COME);
    CASE_ENUM(RPC_MSG_VP_STREAM_DATA_DONE);
    CASE_ENUM(RPC_MSG_VP_STREAM_MALLOC_BUF);
    CASE_ENUM(RPC_MSG_VP_STREAM_FREE_BUF);
    CASE_ENUM(RPC_MSG_VP_CAMERA_I2C_WR);
    CASE_ENUM(RPC_MSG_VP_CAMERA_I2C_RD);
    CASE_ENUM(RPC_MSG_VP_STREAM_ENCODER_DATA_DONE);
    CASE_ENUM(RPC_MSG_VP_QRCODE_CMD);
    }
    return "[]";
}

#ifdef CHIP_ROLE_AP
void ap_recv_rpc_msg_callback(char *data, uint32_t len)
{
    uint32_t i;
    A7_CMD_T *rx_msg = (A7_CMD_T *)data;
    struct RPC_MSG_HDR_T *hdr;

    ASSERT(data != NULL, "[%s]: null data ptr", __func__);
    ASSERT(len >= sizeof(A7_CMD_T), "[%s]: Bad len=%d", __func__, len);

    hdr = (struct RPC_MSG_HDR_T *)rx_msg->str;
    if (hdr->id != RPC_MSG_HDR_ID) {
        TRACE(1, "[%s]: Bad msg hdr id=%d", __func__, hdr->id);
        return;
    }
    if (hdr->reply == 0) {
        struct RPC_REQ_MSG_T *msg = (struct RPC_REQ_MSG_T *)rx_msg->str;
#ifdef VP_STREAM_RPC_DEBUG
        TRACE(1, "[AP_RPC_RX]:%s,param=[%x/%x/%x/%x]",
            rpc_cmd_to_str(msg->cmd), msg->param[0], msg->param[1], msg->param[2], msg->param[3]);
#endif
        for (i = 0; i < RPC_MSG_RX_HDLR_ID_NUM; i++) {
            if (rpc_msg_rx_handler[i]) {
                rpc_msg_rx_handler[i]((char *)msg, sizeof(struct RPC_REQ_MSG_T));
            }
        }
    } else {
        struct RPC_RSP_MSG_T *msg = (struct RPC_RSP_MSG_T *)rx_msg->str;
        vp_rsp_msg = *msg;
        vp_rsp_msg_cnt++;
#ifdef VP_STREAM_RPC_DEBUG
        TRACE(1, "[AP_RPC_RX2]:%s, param=[%x/%x/%x/%x]",
            rpc_cmd_to_str(msg->cmd), msg->param[0], msg->param[1], msg->param[2], msg->param[3]);
#endif
    }
}
#else
void dsp_recv_rpc_msg_callback(char *data, uint32_t len)
{
    uint32_t i;
    A7_CMD_T *rx_msg = (A7_CMD_T *)data;
    struct RPC_MSG_HDR_T *hdr;

    ASSERT(data != NULL, "[%s]: null data ptr", __func__);
    ASSERT(len >= sizeof(A7_CMD_T), "[%s]: Bad len=%d", __func__, len);
//    TRACE(1, "[%s]: data=%x, len=%d", __func__, (int)data, len);

    hdr = (struct RPC_MSG_HDR_T *)(rx_msg->str);
    if (hdr->id != RPC_MSG_HDR_ID) {
        TRACE(1, "[%s]: Bad msg hdr id=%d", __func__, hdr->id);
        return;
    }
    if (hdr->reply == 0) {
        struct RPC_REQ_MSG_T *msg = (struct RPC_REQ_MSG_T *)rx_msg->str;
#ifdef VP_STREAM_RPC_DEBUG
        TRACE(1, "[DSP_RPC_RX]:%s,param=[%x/%x/%x/%x]",
            rpc_cmd_to_str(msg->cmd), msg->param[0], msg->param[1], msg->param[2], msg->param[3]);
#endif

        for (i = 0; i < RPC_MSG_RX_HDLR_ID_NUM; i++) {
            if (rpc_msg_rx_handler[i]) {
                rpc_msg_rx_handler[i]((char *)msg, sizeof(struct RPC_REQ_MSG_T));
            }
        }
    } else {
        struct RPC_RSP_MSG_T *msg = (struct RPC_RSP_MSG_T *)rx_msg->str;
        vp_rsp_msg = *msg;
#ifdef VP_STREAM_RPC_DEBUG
        TRACE(1, "[DSP_RPC_RX2]:%s, param=[%x/%x/%x/%x]",
            rpc_cmd_to_str(msg->cmd), msg->param[0], msg->param[1], msg->param[2], msg->param[3]);
#endif
    }
}
#endif

int vplayer_rpc_msg_rx_handler(char *data, uint32_t size)
{
#ifdef CHIP_ROLE_AP
    ap_recv_rpc_msg_callback(data, size);
#else
    dsp_recv_rpc_msg_callback(data, size);
#endif
    return 0;
}

int vplayer_rpc_msg_tx_handler(char *data, uint32_t size)
{
    return 0;
}

static void rsp_msg_clear(struct RPC_RSP_MSG_T *msg)
{
    msg->hdr.id = 0;
    msg->hdr.reply = 0;
    msg->hdr.wait_sync_flag = 0;
    msg->hdr.sync_flag = 0;
    msg->cmd = 0;
}

static int wait_msg_ack(struct RPC_REQ_MSG_T *msg)
{
    int err = 0;
    volatile int timeout = 100;

    do {
        osDelay(20);
        if (vp_rsp_msg_cnt > 0) {
            break;
        }
        timeout--;
        if (timeout == 0) {
            err = 1;
            break;
        }
    } while (1);
    return err;
}

int vplayer_common_msg_send(enum RPC_MSG_CMD_T cmd,
    uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4,
    uint16_t reply, uint16_t wait)
{
    int err = 0;
    struct RPC_REQ_MSG_T *msg = &vp_req_msg;

#ifdef VP_STREAM_RPC_DEBUG
    TRACE(1, "[%s]:%s,param=[%x/%x/%x/%x]", __func__, rpc_cmd_to_str(cmd), p0, p1, p2, p3);
#endif
    // private message
    msg->hdr.id = RPC_MSG_HDR_ID;
    msg->hdr.reply = reply;
    msg->hdr.sync_flag = 0;
    msg->hdr.wait_sync_flag = wait;
    msg->hdr.trans_cnt = RPC_RSP_MSG_PARAM_NUM;
    msg->cmd = cmd;
    msg->param[0] = p0;
    msg->param[1] = p1;
    msg->param[2] = p2;
    msg->param[3] = p3;
    msg->param[4] = p4;

    if (wait) {
        vp_rsp_msg_cnt = 0;
        rsp_msg_clear(&vp_rsp_msg);
    }
    rpc_msg_send(msg);
    if (wait) {
        err = wait_msg_ack(msg);
        if (err) {
            TRACE(1, "[%s]: wait msg ack timeout, err=%d", __func__, err);
        }
    }
    return err;
}

int vplayer_request_msg_send_no_ack(enum RPC_MSG_CMD_T cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    return vplayer_common_msg_send(cmd, p0, p1, p2, p3, p4, 0, 0);
}

int vplayer_request_msg_send(enum RPC_MSG_CMD_T cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
    return vplayer_common_msg_send(cmd, p0, p1, p2, p3, 0, 0, 1);
}

int vplayer_fine_request_msg_send(struct RPC_REQ_MSG_T *msg)
{
    return vplayer_common_msg_send(msg->cmd, msg->param[0], msg->param[1], msg->param[2], msg->param[3], msg->param[4], 0, 1);
}

int vplayer_fine_request_msg_send_no_ack(struct RPC_REQ_MSG_T *msg)
{
    return vplayer_common_msg_send(msg->cmd, msg->param[0], msg->param[1], msg->param[2], msg->param[3], msg->param[4], 0, 0);
}

int vplayer_reply_msg_send(enum RPC_MSG_CMD_T cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
    return vplayer_common_msg_send(cmd, p0, p1, p2, p3, 0, 1, 0);
}

int vplayer_fine_reply_msg_send(struct RPC_RSP_MSG_T *msg)
{
    return vplayer_common_msg_send(msg->cmd, msg->param[0], msg->param[1], msg->param[2], msg->param[3], msg->param[4], 1, 0);
}

void vplayer_rpc_msg_setup_rx_handler(enum RPC_MSG_RX_HDLR_ID_T id, VP_RPC_MSG_DATA_HDLR_T handler)
{
    ASSERT(id < RPC_MSG_RX_HDLR_ID_NUM, "[%s]: Bad id=%d", __func__, id);
    rpc_msg_rx_handler[id] = handler;
}

int vplayer_rpc_open(void)
{
    return vplayer_request_msg_send(RPC_MSG_VP_OPEN, 0, 0, 0, 0);
}

int vplayer_rpc_close(void)
{
    return vplayer_request_msg_send(RPC_MSG_VP_CLOSE, 0, 0, 0, 0);
}

int vplayer_rpc_stream_open(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_OPEN, id, stream, (uint32_t)cfg, 0);
}

int vplayer_rpc_stream_start(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_START, id, stream, 0, 0);
}

int vplayer_rpc_stream_stop(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_STOP, id, stream, 0, 0);
}

int vplayer_rpc_stream_close(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_CLOSE, id, stream, 0, 0);
}

int vplayer_rpc_stream_data_notify(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint32_t addr, uint32_t len, uint32_t seq)
{
    return vplayer_request_msg_send_no_ack(RPC_MSG_VP_STREAM_DATA_COME, id, stream, addr, len, seq);
}

int vplayer_rpc_stream_data_done(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint32_t addr, uint32_t len, uint32_t seq)
{
    return vplayer_request_msg_send_no_ack(RPC_MSG_VP_STREAM_DATA_DONE, id, stream, addr, len, seq);
}

int vplayer_rpc_stream_get_cfg(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_GET_CFG, id, stream, (uint32_t)cfg, 0);
}

int vplayer_rpc_stream_set_cfg(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_SET_CFG, id, stream, (uint32_t)cfg, 0);
}
int x264_rpc_stream_data_done(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint32_t addr, uint32_t len, uint32_t seq)
{
    return vplayer_request_msg_send_no_ack(RPC_MSG_VP_STREAM_ENCODER_DATA_DONE, id, stream, addr, len, seq);
}

int vplayer_rpc_stream_get_buff(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint8_t **ptr, uint32_t len)
{
    int ret = 0;
    ret = vplayer_request_msg_send(RPC_MSG_VP_STREAM_MALLOC_BUF, id, stream, len, 0);
    if (!ret) {
        struct RPC_RSP_MSG_T *msg = &vp_rsp_msg;
        enum VP_STREAM_ID_T rsp_id;
        enum VP_STREAM_T rsp_stream;
        uint8_t *rsp_buf;
        uint32_t rsp_len;

        rsp_id     = (enum VP_STREAM_ID_T)(msg->param[0]);
        rsp_stream = (enum VP_STREAM_T)(msg->param[1]);
        rsp_buf    = (uint8_t *)(msg->param[2]);
        rsp_len    = (uint32_t)(msg->param[3]);

        if (msg->cmd != RPC_MSG_VP_STREAM_MALLOC_BUF) {
            ret |= 0x01;
        }
        if (rsp_id != id) {
            ret |= 0x02;
        }
        if (rsp_stream != stream) {
            ret |= 0x04;
        }
        if (rsp_len != len) {
            ret |= 0x08;
        }
        if (rsp_buf == NULL) {
            ret |= 0x10;
        }
        if (!ret) {
            if (ptr) {
                *ptr = rsp_buf;
            }
        }
    } else {
        ret = 0x20;
    }
    return ret;
}

int vplayer_rpc_stream_free_buff(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint8_t *ptr, uint32_t len)
{
    return vplayer_request_msg_send(RPC_MSG_VP_STREAM_FREE_BUF, id, stream, (uint32_t)ptr, len);
}

int vplayer_rpc_msg_queue_init(struct RPC_MSG_QUEUE_T *q, struct RPC_RSP_MSG_T *msg, uint32_t num)
{
    uint32_t lock;

    lock = int_lock();
    q->msg = msg;
    q->rpos = 0;
    q->wpos = 0;
    q->size = num;
    int_unlock(lock);

    return 0;
}

int vplayer_rpc_msg_enqueue(struct RPC_MSG_QUEUE_T *q, struct RPC_RSP_MSG_T *m)
{
    uint32_t lock;
    int err = 0;

    if (q->wpos >= q->size) {
        err = 1; // queue is full
    } else {
        lock = int_lock();
        if (m) {
            q->msg[q->wpos] = *m;
        }
        q->wpos++;
        int_unlock(lock);
    }
    if (err) {
        TRACE(1, "[%s]: WARNING: err=%d", __func__, err);
    }
    return err;
}

int vplayer_rpc_msg_queue_is_not_empty(struct RPC_MSG_QUEUE_T *q)
{
    return q->wpos;
}

int vplayer_rpc_msg_dequeue(struct RPC_MSG_QUEUE_T *q, struct RPC_RSP_MSG_T *m)
{
    uint32_t lock;
    int err = 0;

    if (q->wpos == 0) {
        if (q->rpos == 0) {
            err = 1;// queue is empty
        } else {
            q->rpos = 0;// reste to zero
            q->wpos = 0;
            err = 2; //queue rpos error, reset
        }
    } else {
        // rpos == wpos && wpos != 0
        if (q->rpos == q->wpos) {
            q->rpos = 0;
            q->wpos = 0;
            err = 3;// queue is empty
        } else if (q->rpos > q->wpos) {
            q->rpos = 0;
            q->wpos = 0;
            err = 4;// queue wpos error, reset;
        } else {
            //rpos < wpos && wpos != 0
            lock = int_lock();
            if (m) {
                *m = q->msg[q->rpos];
            }
            q->rpos++;
            if (q->rpos == q->wpos) {
                q->rpos = 0;
                q->wpos = 0;
            }
            int_unlock(lock);
        }
    }
    if (err) {
        TRACE(1, "[%s]: WARNING: err=%d", __func__, err);
    }
    return err;
}

#endif
