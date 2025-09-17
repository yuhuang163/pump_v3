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
#if defined(VIDEO_PLAYER) && defined(CHIP_ROLE_DSP)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_i2c.h"
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "csi_host_app.h"
#include "video_player.h"
#include "video_stream.h"
#include "video_stream_rpc.h"
#ifdef VIDEO_RECORD_ENABLE
#include "video_record.h"
#endif
#include "video_misc.h"
#include "camera_drv.h"
#ifdef CAMERA_USE_SOFT_ISP
#include "soft_isp.h"
#endif
#include "dsp_vplayer_app.h"
#include "fs_retarget.h"
#include "plat_addr_map.h"
#if defined(CHIP_ROLE_DSP) && !defined(RTOS)
#error "RTOS should be defined for CHIP_ROLE_DSP !"
#endif
#ifdef QRCODE_TEST
#include "dsp_qrcode_process.h"
#include "qrcode_gui.h"
#endif


#ifndef DSP_VPLAYER_APP_DEBUG
#define DSP_VPLAYER_APP_DEBUG 0
#endif

#ifdef H264_TEST
int x264_buffer_to_h264_test(uint32_t id,uint32_t stream, uint8_t *dst_buf,uint32_t dst_buf_size,uint32_t dst_buf_seq);
#endif

#define IMG_WIDTH         CAMEAR_IMAGE_WIDTH
#define IMG_HEIGHT        CAMERA_IMAGE_HEIGHT
#define IMG_PIXEL_SIZE    (2)
#define ALIGN_PAD_SIZE    (0x800)
#define DSP_CAP_IMG_SIZE  (IMG_WIDTH*IMG_HEIGHT*IMG_PIXEL_SIZE*2)
#define DSP_CAP_BUF_SIZE  (DSP_CAP_IMG_SIZE + ALIGN_PAD_SIZE)
#define DSP_PLAY_IMG_SIZE (IMG_WIDTH*IMG_HEIGHT*IMG_PIXEL_SIZE*1)
#define DSP_PLAY_BUF_SIZE (DSP_PLAY_IMG_SIZE + ALIGN_PAD_SIZE)

#define DSP_RSP_MSG_NUM   (4)

enum {
    DSP_VP_STATUS_NULL,
    DSP_VP_STATUS_OPEN_CLOSE = (1<<0),
    DSP_VP_STATUS_STREAM_OPEN_CLOSE = (1<<1),
    DSP_VP_STATUS_STREAM_START_STOP = (1<<2),
};

enum DSP_SIG_T {
    DSP_SIG_SEND_ACK_MSG = 0x01,
    DSP_SIG_PROCESS_DATA = 0x02,
    DSP_SIG_SOFT_ISP_OPEN = 0x04,
    DSP_SIG_SOFT_ISP_CLOSE = 0x08,
};

struct dsp_vp_stream_cfg_t {
    uint8_t status;
    uint32_t mem_size;
    uint32_t mem_addr;
    struct VP_STREAM_CONFIG_T cfg;
    struct VP_DEVICE_CONFIG_T dev_cfg;
    int dev_desc_id;
};

static SYNC_FLAGS_LOC enum VPLAYER_APP_MODE_T vp_app_mode = VPLAYER_APP_MODE_NONE;

static SYNC_FLAGS_LOC struct dsp_vp_stream_cfg_t dsp_vp_stream_cfg[VP_STREAM_ID_QTY][VP_STREAM_QTY];

static SYNC_FLAGS_LOC struct RPC_MSG_QUEUE_T g_msg_queue;

static SYNC_FLAGS_LOC struct RPC_RSP_MSG_T g_rsp_msg[DSP_RSP_MSG_NUM];

#ifndef DSP_VPLAYER_APP_USE_HEAP
static ALIGNED4 uint8_t g_dsp_playback_buf[DSP_PLAY_BUF_SIZE];
static ALIGNED4 uint8_t g_dsp_capture_buf[DSP_CAP_BUF_SIZE];
#endif

#ifdef RTOS
#define DSP_VP_THD_STK_SIZE (0x1f000)
static osThreadId dsp_vp_tid = NULL;
static void dsp_vplayer_thread(void const *arguments);
osThreadDef(dsp_vplayer_thread, osPriorityHigh, 1, DSP_VP_THD_STK_SIZE, "dsp_vp_thd");
#endif

static struct dsp_vp_stream_cfg_t *dsp_get_vp_stream_role(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    ASSERT(id < VP_STREAM_ID_QTY, "[%s]: Bad id=%d", __func__, id);
    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream=%d", __func__, stream);

    return &dsp_vp_stream_cfg[id][stream];
}

int dsp_vplayer_open(void)
{
    uint32_t id, stream;
    struct dsp_vp_stream_cfg_t *role = NULL;

    TRACE(1, "[%s]", __func__);

    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        for (stream = 0; stream < VP_STREAM_QTY; stream++) {
            role = dsp_get_vp_stream_role(id, stream);
            if (role->status == DSP_VP_STATUS_NULL) {
                role->status = DSP_VP_STATUS_OPEN_CLOSE;
            }
        }
    }
    return 0;
}

int dsp_vplayer_close(void)
{
    uint32_t id, stream;
    struct dsp_vp_stream_cfg_t *role = NULL;

    TRACE(1, "[%s]", __func__);

    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        for (stream = 0; stream < VP_STREAM_QTY; stream++) {
            role = dsp_get_vp_stream_role(id, stream);
            role->status = DSP_VP_STATUS_NULL;
        }
    }
    return 0;
}

int dsp_vplayer_stream_open(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream,
    struct VP_STREAM_CONFIG_T *cfg)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);
    struct VP_DEVICE_CONFIG_T *dev_cfg;
    enum VP_DEVICE_T device;
    int ret = 0;

    TRACE(1, "[%s]: id=%d, stream=%d, cfg=%x", __func__, id, stream, (int)cfg);

    vplayer_stream_dump_cfg(cfg);

    if (role->status != DSP_VP_STATUS_OPEN_CLOSE) {
        TRACE(1, "[%s]: Bad status=%x", __func__, role->status);
        return -1;
    }
    role->cfg = *cfg;
    device = cfg->device;
    dev_cfg = &role->dev_cfg;
    if (stream == VP_STREAM_CAPTURE) {
        ret = camera_device_get_config(device, dev_cfg);
        ASSERT(ret == 0, "[%s]: get device config failed, ret=%d", __func__, ret);
        vplayer_device_dump_cfg(dev_cfg);
#ifdef CAMERA_USE_SOFT_ISP
        if (dev_cfg->op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
            //FIXME: dev_desc_id
            role->dev_desc_id = 0;
            soft_isp_setup_device(role->dev_desc_id, device, dev_cfg);
            osSignalSet(dsp_vp_tid, DSP_SIG_SOFT_ISP_OPEN);
        }
#endif
    } else {
        //TODO: get display device config ?
    }
    role->status |= DSP_VP_STATUS_STREAM_OPEN_CLOSE;

    vplayer_stream_dump_cfg(&role->cfg);
    return 0;
}

int dsp_vplayer_stream_set_cfg(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream,
    struct VP_STREAM_CONFIG_T *cfg)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);

    TRACE(1, "[%s]: id=%d, stream=%d, cfg=%x", __func__, id, stream, (int)cfg);
    if (cfg) {
        role->cfg = *cfg;
    }
    return 0;
}

int dsp_vplayer_stream_get_cfg(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream,
    struct VP_STREAM_CONFIG_T *cfg)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);

    TRACE(1, "[%s]: id=%d, stream=%d, cfg=%x", __func__, id, stream, (int)cfg);
    if (cfg) {
        *cfg = role->cfg;
    }
    return 0;
}

int dsp_vplayer_stream_start(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);

    TRACE(1, "[%s]: id=%d, stream=%d", __func__, id, stream);
    if (role->status != (DSP_VP_STATUS_OPEN_CLOSE | DSP_VP_STATUS_STREAM_OPEN_CLOSE)) {
        TRACE(1, "[%s]: Bad status=%x", __func__, role->status);
        return -1;
    }
    role->status |= DSP_VP_STATUS_STREAM_START_STOP;
    return 0;
}

int dsp_vplayer_stream_stop(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);

    TRACE(1, "[%s]: id=%d, stream=%d", __func__, id, stream);
    if (role->status != (DSP_VP_STATUS_OPEN_CLOSE
                        | DSP_VP_STATUS_STREAM_OPEN_CLOSE
                        | DSP_VP_STATUS_STREAM_START_STOP)) {
        TRACE(1, "[%s]: Bad status=%x", __func__, role->status);
        return -1;
    }
    role->status &= ~DSP_VP_STATUS_STREAM_START_STOP;
    return 0;
}

int dsp_vplayer_stream_close(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    struct dsp_vp_stream_cfg_t *role = dsp_get_vp_stream_role(id, stream);

    TRACE(1, "[%s]: id=%d, stream=%d", __func__, id, stream);
    if (role->status != (DSP_VP_STATUS_OPEN_CLOSE | DSP_VP_STATUS_STREAM_OPEN_CLOSE)) {
        TRACE(1, "[%s]: Bad status=%x", __func__, role->status);
        return -1;
    }
#ifdef CAMERA_USE_SOFT_ISP
    if (role->dev_cfg.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
        osSignalSet(dsp_vp_tid, DSP_SIG_SOFT_ISP_CLOSE);
    }
#endif
    role->status &= ~DSP_VP_STATUS_STREAM_OPEN_CLOSE;
    return 0;
}

void dsp_send_reply_msg(uint16_t cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
    int ret;
    struct RPC_RSP_MSG_T msg;

    memset((void *)&msg, 0, sizeof(msg));
    msg.hdr.id = RPC_MSG_HDR_ID;
    msg.hdr.reply = 1;
    msg.cmd = cmd;
    msg.param[0] = p0;
    msg.param[1] = p1;
    msg.param[2] = p2;
    msg.param[3] = p3;
    msg.param[4] = 0;
    ret = vplayer_rpc_msg_enqueue(&g_msg_queue, &msg);
    ASSERT(ret == 0, "[%s]: msg enqueue failed, ret=%d", __func__, ret);

#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 1)
    TRACE(1, "[%s]: cmd=%d, param=[%x/%x/%x/%x]", __func__, cmd, p0, p1, p2, p3);
#endif

#ifdef RTOS
    osSignalSet(dsp_vp_tid, DSP_SIG_SEND_ACK_MSG);
#endif
}

void dsp_data_come_notify(uint16_t cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    int ret;
    struct RPC_RSP_MSG_T msg;

    memset((void *)&msg, 0, sizeof(msg));
    msg.hdr.id = RPC_MSG_HDR_ID;
    msg.hdr.reply = 1;
    msg.cmd = cmd;
    msg.param[0] = p0;
    msg.param[1] = p1;
    msg.param[2] = p2;
    msg.param[3] = p3;
    msg.param[4] = p4;
    ret = vplayer_rpc_msg_enqueue(&g_msg_queue, &msg);
    ASSERT(ret == 0, "[%s]: msg enqueue failed, ret=%d", __func__, ret);

#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 1)
    TRACE(1, "[%s]: cmd=%d, param=[%x/%x/%x/%x/%x]", __func__, cmd, p0, p1, p2, p3, p4);
#endif

#ifdef RTOS
    osSignalSet(dsp_vp_tid, DSP_SIG_PROCESS_DATA);
#endif
}

int dsp_vplayer_process_data(struct RPC_RSP_MSG_T *msg)
{
    uint8_t *buf;
    uint32_t len;
    uint8_t *dst_buf;
    uint32_t dst_buf_size;
    uint32_t dst_buf_seq;
    uint8_t *src_buf;
    uint32_t src_buf_size;
    uint32_t id, stream, seq;
    struct dsp_vp_stream_cfg_t *role;

    POSSIBLY_UNUSED uint32_t time[2]={0,0};

    id     = msg->param[0];
    stream = msg->param[1];
    buf    = (uint8_t *)msg->param[2];
    len    = msg->param[3];
    seq    = msg->param[4];

    src_buf      = buf;
    src_buf_size = len;
    dst_buf_seq  = seq;

    role  = dsp_get_vp_stream_role(id, stream);

    if (vp_app_mode == VPLAYER_APP_MODE_CSI_DSI_LOOP) {
        struct dsp_vp_stream_cfg_t *play_role, *cap_role;
        uint8_t *cap_buf_ptr = NULL;
        uint32_t cap_buf_size = 0;
        uint32_t play_buf_size = 0;
        bool pp_buf_mode = false;

        play_role = dsp_get_vp_stream_role(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
        cap_role  = dsp_get_vp_stream_role(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
        cap_buf_ptr   = cap_role->cfg.data_ptr;
        cap_buf_size  = cap_role->cfg.data_size;
        play_buf_size = play_role->mem_size;

        if (play_buf_size >= cap_buf_size) {
            pp_buf_mode = true;
        } else {
            ASSERT(play_buf_size >= cap_buf_size / 2,
            "[%s]: Bad play_buf_size=%d, cap_buf_size/2=%d", __func__, play_buf_size, cap_buf_size);
        }
        if (pp_buf_mode) {
            if (src_buf == cap_buf_ptr) {
                dst_buf = play_role->cfg.data_ptr + cap_buf_size / 2;
            } else {
                dst_buf = play_role->cfg.data_ptr;
            }
            dst_buf_size = cap_buf_size / 2;
        } else {
            dst_buf = play_role->cfg.data_ptr;
            dst_buf_size = cap_buf_size / 2;
        }
    } else {
        dst_buf = buf;
        dst_buf_size = len;
    }

    uint32_t src_c_buf = PSRAMUHS_NC_TO_C((uint32_t)(src_buf));
    uint32_t dst_c_buf = PSRAMUHS_NC_TO_C((uint32_t)(dst_buf));

#ifdef CAMERA_USE_SOFT_ISP
    time[0] = hal_fast_sys_timer_get();
    if (role->dev_cfg.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
        soft_isp_aec_process_data(src_c_buf, src_buf_size);
    }
#ifdef QRCODE_TEST
    qrcode_get_img(src_c_buf, 640, 480, 16, 2);
#endif
    time[0] = hal_fast_sys_timer_get() - time[0];
#endif

    time[1] = hal_fast_sys_timer_get();

    ASSERT((src_c_buf % 64) == 0, "src_c_buf memory no 64_align");
    ASSERT((src_buf_size % 64) == 0, "src_buf_size no 64_align");
    L1C_CleanInvalidateCacheRange(0,(size_t)src_c_buf,(size_t)src_c_buf + src_buf_size);
#ifdef CAMERA_DATA_RAW10_TO_YUV
    if (role->dev_cfg.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
#ifdef QRCODE_TEST
        qr_gui_show_data_refresh(dst_c_buf, src_c_buf, src_buf_size);
#else
        camera_data_raw10_to_yuv422(dst_c_buf, src_c_buf, src_buf_size);
#endif
    } else
#endif
    {
        camera_data_memcpy((uint8_t *)dst_c_buf, (uint8_t *)src_c_buf, src_buf_size);
    }

#ifdef H264_TEST

#ifdef CSI_ENCODER_320_240
        x264_buffer_to_h264_test(id, stream,(uint8_t *)dst_c_buf, dst_buf_size, dst_buf_seq);
#endif

#ifdef CSI_ENCODER_640_480
    if ((dst_buf_seq % 2) ==0) {
        x264_buffer_to_h264_test(id, stream,(uint8_t *)dst_c_buf, dst_buf_size, dst_buf_seq);
    }
#endif
#endif

    time[1] = hal_fast_sys_timer_get() - time[1];

#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 0)
    TRACE(1, "[DSP_PROC][%d]:mips=[%d/%d],dst=[%x/%d],src=[%x/%d]", dst_buf_seq,
        FAST_TICKS_TO_US(time[0]), FAST_TICKS_TO_US(time[1]),
        (int)dst_buf, dst_buf_size,
        (int)src_buf, src_buf_size);
#endif

    // send data done msg
#ifdef H264_STERAM_A7_TO_M33
    return 0;
#else
    ASSERT((dst_c_buf % 64) == 0, "dst_buf memory no 64_align");
    ASSERT((dst_buf_size % 64) == 0, "dst_buf_size no 64_align");
    L1C_CleanInvalidateCacheRange(1,(size_t)dst_c_buf,(size_t)dst_c_buf + dst_buf_size);
    vplayer_rpc_stream_data_done(id, stream, (size_t)dst_buf, dst_buf_size, dst_buf_seq);
    return 0;
#endif
}

#ifdef RTOS
static void dsp_vplayer_thread(void const *arguments)
{
    osEvent evt;
    uint32_t signals = 0;
    struct RPC_RSP_MSG_T msg;
    int err;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        if (evt.status == osEventSignal) {
            if (signals & DSP_SIG_SEND_ACK_MSG) {
                err = vplayer_rpc_msg_dequeue(&g_msg_queue, &msg);
                if (!err) {
                    vplayer_fine_reply_msg_send(&msg);
                } else {
                    TRACE(1, "WARNING: dequeue err1: %d", err);
                }
            }
            if (signals & DSP_SIG_PROCESS_DATA) {
                int not_empty;
                do {
                    err = vplayer_rpc_msg_dequeue(&g_msg_queue, &msg);
                    if (!err) {
                        dsp_vplayer_process_data(&msg);
                    } else {
                        TRACE(1, "WARNING: dequeue err2: %d", err);
                    }
                    not_empty = vplayer_rpc_msg_queue_is_not_empty(&g_msg_queue);
                } while (not_empty);
            }
#ifdef CAMERA_USE_SOFT_ISP
            if (signals & DSP_SIG_SOFT_ISP_OPEN) {
                soft_isp_aec_open();
            }
            if (signals & DSP_SIG_SOFT_ISP_CLOSE) {
                soft_isp_aec_close();
            }
#endif
        }
    }
}
#endif /* RTOS */

static void dsp_vplayer_app_msg_rx_handler(char *data, uint32_t len)
{
    struct RPC_REQ_MSG_T *msg = (struct RPC_REQ_MSG_T *)data;
    uint16_t cmd;
    uint32_t *argv;
    int ret;

#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 1)
    TRACE(1, "[%s]: data=%x, len=%d", __func__, (int)data, len);
#endif
    ASSERT(msg != NULL, "[%s]: null msg ptr", __func__);
    ASSERT(len >= sizeof(struct RPC_REQ_MSG_T), "[%s]: Bad len=%d", __func__, len);
    cmd = msg->cmd;
    argv = msg->param;

#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 0)
    TRACE(1, "[DSP_APP_RX]:[%d],param=[%x/%x/%x/%x]", cmd, argv[0], argv[1], argv[2], argv[3]);
#endif
    switch (cmd) {
    case RPC_MSG_VP_OPEN:
        dsp_vplayer_open();
        dsp_send_reply_msg(cmd, 0, 0, 0, 0);
        break;
    case RPC_MSG_VP_CLOSE:
        dsp_vplayer_close();
        dsp_send_reply_msg(cmd, 0, 0, 0, 0);
        break;
    case RPC_MSG_VP_STREAM_OPEN:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;
        struct VP_STREAM_CONFIG_T *cfg;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        cfg    = (struct VP_STREAM_CONFIG_T *)argv[2];
        ret = dsp_vplayer_stream_open(id, stream, cfg);
        dsp_send_reply_msg(cmd, id, stream, (uint32_t)cfg, ret);
    }
        break;
    case RPC_MSG_VP_STREAM_START:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        ret = dsp_vplayer_stream_start(id, stream);
        dsp_send_reply_msg(cmd, id, stream, ret, 0);
    }
        break;
    case RPC_MSG_VP_STREAM_STOP:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        ret = dsp_vplayer_stream_stop(id, stream);
        dsp_send_reply_msg(cmd, id, stream, ret, 0);
    }
        break;
    case RPC_MSG_VP_STREAM_CLOSE:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        ret = dsp_vplayer_stream_close(id, stream);
        dsp_send_reply_msg(cmd, id, stream, ret, 0);
    }
        break;
    case RPC_MSG_VP_STREAM_SET_CFG:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;
        struct VP_STREAM_CONFIG_T *cfg;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        cfg    = (struct VP_STREAM_CONFIG_T *)argv[2];
        ret = dsp_vplayer_stream_set_cfg(id, stream, cfg);
        dsp_send_reply_msg(cmd, id, stream, (uint32_t)cfg, ret);
    }
        break;
    case RPC_MSG_VP_STREAM_GET_CFG:
    {
        enum VP_STREAM_ID_T id;
        enum VP_STREAM_T stream;
        struct VP_STREAM_CONFIG_T *cfg;

        id     = (enum VP_STREAM_ID_T)argv[0];
        stream = (enum VP_STREAM_T)argv[1];
        cfg    = (struct VP_STREAM_CONFIG_T *)argv[2];
        ret = dsp_vplayer_stream_get_cfg(id, stream, cfg);
        dsp_send_reply_msg(cmd, id, stream, (uint32_t)cfg, ret);
    }
        break;
    case RPC_MSG_VP_STREAM_DATA_COME:
    {
        uint32_t id, stream, len, seq;
        uint8_t *ptr;

        id     = argv[0];
        stream = argv[1];
        ptr    = (uint8_t *)argv[2];
        len    = argv[3];
        seq    = argv[4];
#if defined(DSP_VPLAYER_APP_DEBUG) && (DSP_VPLAYER_APP_DEBUG > 1)
        TRACE(0, "[%d][%d] data come: ptr=%x, len=%d, seq=%d", id, stream, (int)ptr, len, seq);
#endif
        dsp_data_come_notify(cmd, id, stream, (uint32_t)ptr, len, seq);
    }
        break;
    case RPC_MSG_VP_STREAM_MALLOC_BUF:
    {
        uint32_t id, stream, len;
        uint8_t *ptr;
        struct dsp_vp_stream_cfg_t *role;

        id     = argv[0];
        stream = argv[1];
        len    = argv[2];

#ifdef DSP_VPLAYER_APP_USE_HEAP
        ptr = media_heap_malloc(len);
#else
        uint32_t addr;
        if (stream == VP_STREAM_PLAYBACK) {
            addr = (uint32_t)g_dsp_playback_buf;
        } else {
            addr = (uint32_t)g_dsp_capture_buf;
        }
        addr = (addr + 0x200) & (~(0x200-1));
        ptr  = (uint8_t *)REMAP_NC_ADDR(addr);
#endif
        role = dsp_get_vp_stream_role(id, stream);
        role->mem_size = len;
        role->mem_addr = (uint32_t)ptr;
        TRACE(1, "malloc buff: ptr=%x, len=%d", (int)ptr, len);
        dsp_send_reply_msg(cmd, id, stream, (uint32_t)REMAP_DMA_ADDR(ptr), len);
    }
        break;
    case RPC_MSG_VP_STREAM_FREE_BUF:
    {
        uint32_t id, stream, len;
        uint32_t addr;
        struct dsp_vp_stream_cfg_t *role;

        id     = argv[0];
        stream = argv[1];
        addr   = REMAP_C_ADDR(argv[2]);
        len    = argv[3];
        role = dsp_get_vp_stream_role(id, stream);
        ASSERT(role->mem_addr == addr, "[%s]: Bad addr=%x, mem_addr=%x", __func__, addr, role->mem_addr);
        role->mem_size = 0;
        role->mem_addr = 0;
        TRACE(1, "free buff: ptr=%x, len=%d", addr, len);
#ifdef DSP_VPLAYER_APP_USE_HEAP
        media_heap_free((uint8_t*)addr);
#endif
        dsp_send_reply_msg(cmd, id, stream, 0, 0);
    }
        break;
    case RPC_MSG_VP_QRCODE_CMD:
    {
#ifdef QRCODE_TEST
        dsp_qrcode_rpc_rx_handler((void *)msg, len);
#endif
    }
        break;

    default:
        break;
    }
}

void dsp_vplayer_app_open(enum VPLAYER_APP_MODE_T mode)
{
    TRACE(1, "[%s]: ", __func__);

    vp_app_mode = mode;

#ifdef DSP_VPLAYER_APP_USE_HEAP
    media_heap_init();
#endif
    vplayer_rpc_msg_queue_init(&g_msg_queue, g_rsp_msg, ARRAY_SIZE(g_rsp_msg));
    dsp_vplayer_open();
    vplayer_rpc_msg_setup_rx_handler(RPC_MSG_RX_HDLR_ID_0, dsp_vplayer_app_msg_rx_handler);

#ifdef RTOS
    if (dsp_vp_tid == NULL) {
        dsp_vp_tid = osThreadCreate(osThread(dsp_vplayer_thread), NULL);
        osSignalSet(dsp_vp_tid, 0x00);
    }
    TRACE(1, "dsp_vp_tid=%x", (int)dsp_vp_tid);
#ifdef QRCODE_TEST
    dsp_qrcode_process_init();
#endif
#endif
}

void dsp_vplayer_app_close(enum VPLAYER_APP_MODE_T mode)
{
    TRACE(1, "[%s]:", __func__);
    vplayer_rpc_msg_setup_rx_handler(RPC_MSG_RX_HDLR_ID_0, NULL);
}

void dsp_vplayer_app_trigger(bool on)
{
}

#endif /* defined(VIDEO_PLAYER) && defined(CHIP_ROLE_DSP) */
