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
#if defined(VIDEO_PLAYER) && !defined(CHIP_ROLE_DSP)
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
#include "hal_sysfreq.h"
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
#include "display_drv.h"
#ifdef CAMERA_USE_SOFT_ISP
#include "soft_isp.h"
#endif
#include "ap_vplayer_app.h"
#ifdef QRCODE_TEST
#include "ap_qrcode_app.h"
#endif

//#define CAMERA_MIPS_CALC

#define LOOP_STREAM_CHAN_ID VIDEO_STREAM_CHAN_ID_0

#define CAMERA_DATA_FRAME_NUM     2
#define DISPLAY_DATA_FRAME_NUM    2

#ifdef CSI_SLT_TEST
#define CAMERA_DATA_CHECK_MODE_ONESHOT false
#else
#define CAMERA_DATA_CHECK_MODE_ONESHOT true
#endif

// PLAYBACK STREAM CONFIG
#ifdef DISPLAY_IMAGE_WIDTH
#define PLAYBACK_WIDTH  (DISPLAY_IMAGE_WIDTH)
#else
#define PLAYBACK_WIDTH  (800)
#endif
#ifdef DISPLAY_IMAGE_HEIGHT
#define PLAYBACK_HEIGHT (DISPLAY_IMAGE_HEIGHT)
#else
#define PLAYBACK_HEIGHT (600)
#endif

#define PLAYBACK_CHAN_NUM VP_CHANNEL_NUM_2
#define PLAYBACK_CHAN_MAP VP_CHANNEL_MAP_CH0 | VP_CHANNEL_MAP_CH1
#define PLAYBACK_FMT      VP_PIXEL_FMT_YUV422

#ifdef DSI_SENS_A064
#define PLAYBACK_DEVICE   VP_DEVICE_A064
#else
#error "Invalid playback device !!!"
#endif

#define SYSFREQ_USER_VIDEO_PLAYER HAL_SYSFREQ_USER_APP_12

static bool ap_vplayer_status = false;

static uint32_t cap_stream_en = 0;
static uint32_t play_stream_en = 0;
static enum VPLAYER_APP_MODE_T vp_app_mode = VPLAYER_APP_MODE_NONE;
static POSSIBLY_UNUSED uint32_t g_cost_time = 0;
static POSSIBLY_UNUSED uint32_t g_cost_id = 0;
static AP_VPLAYER_STREAM_READY_NOTIFY_T stream_notify_func = NULL;

#define TIME_CALC_START(id) { \
        g_cost_id = (id); \
        g_cost_time = hal_sys_timer_get(); \
    }

#define TIME_CALC_END(id) { \
        if (g_cost_id == (id)) { \
            g_cost_time = hal_sys_timer_get() - g_cost_time; \
            TRACE(1, "[%s]: cost %d ms", __func__, TICKS_TO_MS(g_cost_time)); \
        } \
    }


static POSSIBLY_UNUSED uint8_t *cam_image_buff1 = (uint8_t *)(DSI_DATA_BUF_ADDR);
static POSSIBLY_UNUSED uint8_t *cam_image_buff2 = (uint8_t *)(DSI_DATA_BUF_ADDR + DSI_DATA_BUF_SIZE / 2);
static POSSIBLY_UNUSED uint8_t *cur_image_buff = NULL;
static POSSIBLY_UNUSED uint32_t cur_image_size = 0;
static POSSIBLY_UNUSED uint32_t image_buff_idx = 0;

#define AP_RSP_MSG_NUM 5

typedef struct {
    uint32_t data_addr;
    uint32_t data_size;
    uint32_t data_seq;
    uint32_t frame_cnt;
    uint32_t lost_cnt;
} mipi_data_msg_t;

static SYNC_FLAGS_LOC mipi_data_msg_t cam_frame_msg;
static SYNC_FLAGS_LOC mipi_data_msg_t disp_frame_msg;
static SYNC_FLAGS_LOC struct VP_STREAM_CONFIG_T play_stream_cfg;
static SYNC_FLAGS_LOC struct VP_STREAM_CONFIG_T cap_stream_cfg;

static SYNC_FLAGS_LOC int g_cam_desc_id = 0;
static SYNC_FLAGS_LOC enum VP_DEVICE_T g_cam_device = VP_DEVICE_NULL;
static SYNC_FLAGS_LOC struct VP_DEVICE_CONFIG_T g_cam_config;

#ifndef CHIP_ROLE_AP
#define AP_DISP_THREAD
#endif

#ifdef CHIP_ROLE_AP

enum AP_SIG_T {
    AP_SIG_APP_INIT = 0x01,
    AP_SIG_APP_ON   = 0x02,
    AP_SIG_APP_OFF  = 0x04,
    AP_SIG_APP_I2C_RD  = 0x08,
    AP_SIG_APP_I2C_WR  = 0x10,
};

static SYNC_FLAGS_LOC struct RPC_MSG_QUEUE_T g_msg_queue;
static SYNC_FLAGS_LOC struct RPC_RSP_MSG_T g_rsp_msg[AP_RSP_MSG_NUM];

#define AP_VP_THD_STK_SIZE (4*1024)
#ifdef RTOS
static osThreadId ap_vp_tid = NULL;
static void ap_vplayer_thread(void const *arguments);
osThreadDef(ap_vplayer_thread, osPriorityHigh, 1, AP_VP_THD_STK_SIZE, "ap_vp_thd");
#endif
#endif

#if defined(RTOS) && defined(AP_DISP_THREAD)
#define CAMERA_FRAME_MSG_COME  (0x01)
#define DISP_STACK_SIZE        (1024*2)
static osThreadId disp_thread_tid = NULL;
static void display_thread(void const *arguments);
osThreadDef(display_thread, osPriorityHigh, 1, DISP_STACK_SIZE, "disp_thd");
#endif

uint32_t hal_csi_get_err_status(void);

static uint32_t pixel_fmt_to_size(enum VP_PIXEL_FMT_T fmt)
{
    uint32_t psize = 2;

    if (fmt == VP_PIXEL_FMT_RGB888) {
        psize = 4;
    } else if (fmt == VP_PIXEL_FMT_RGBA888) {
        psize = 4;
    } else if (fmt == VP_PIXEL_FMT_RGB565) {
        psize = 2;
    } else {
        psize = 2;
    }
    return psize;
}

static uint32_t calc_image_size(uint32_t width, uint32_t height, uint32_t pixel_size, uint32_t num)
{
    uint32_t bytes;

    bytes = width * height * pixel_size * num;
    return bytes;
}

static int mipi_data_msg_pack(mipi_data_msg_t *msg, uint32_t addr, uint32_t len, uint32_t seq)
{
    uint32_t lock;

    lock = int_lock();
    if (msg->frame_cnt == 0) {
        msg->data_addr = addr;
        msg->data_size = len;
        msg->data_seq = seq;
        msg->frame_cnt++;
    } else {
        msg->lost_cnt++;
    }
    int_unlock(lock);

    return msg->lost_cnt;
}

static void mipi_data_msg_reset(mipi_data_msg_t *msg)
{
	uint32_t lock;

    lock = int_lock();
    msg->lost_cnt = 0;
    msg->frame_cnt = 0;
    int_unlock(lock);
}

#if defined(CHIP_ROLE_AP)
static void camera_data_display_proc(mipi_data_msg_t *msg)
{
    uint32_t addr;
    uint32_t len;
    uint32_t seq;

    addr = msg->data_addr;
    len = msg->data_size;
    seq = msg->data_seq;
    if (msg->lost_cnt > 0) {
        TRACE(1, "[DISP_PROCESS]: WARN1: lost msg, cnt=%d", msg->lost_cnt);
    }
    mipi_data_msg_pack(&disp_frame_msg, addr, len, seq);
    mipi_data_msg_reset(msg);
}

#else /* !defined(CHIP_ROLE_AP) */
static void camera_data_display_proc(mipi_data_msg_t *msg)
{
    uint32_t addr, len;
    uint32_t src_c_buf, dst_c_buf, src_buf_size;
    uint32_t seq;

    addr = msg->data_addr;
    len = msg->data_size;
    seq = msg->data_seq;
    if (msg->lost_cnt > 0) {
        TRACE(1, "[DISP_PROCESS]: WARN1: lost msg, cnt=%d", msg->lost_cnt);
    }

#ifdef CAMERA_DATA_RAW10_TO_YUV
    if (image_buff_idx == 0) {
        cur_image_buff = cam_image_buff1;
    } else {
        cur_image_buff = cam_image_buff2;
    }
    cur_image_size = len;
    image_buff_idx = 1 - image_buff_idx;

#ifdef CAMERA_MIPS_CALC
    TIME_CALC_START(0);
#endif

    src_c_buf = REMAP_C_ADDR((uint32_t)(addr));
    dst_c_buf = REMAP_C_ADDR((uint32_t)(cur_image_buff));
    src_buf_size = len;

#ifdef CAMERA_USE_SOFT_ISP
    if (g_cam_config.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
        soft_isp_aec_process_data(src_c_buf, src_buf_size);
    }
#endif

    if (g_cam_config.op_flag & CAMERA_OP_FLAG_RAW10_TO_YUV) {
        camera_data_raw10_to_yuv422(dst_c_buf, src_c_buf, src_buf_size);
    } else {
        cur_image_buff = (uint8_t *)addr;
        cur_image_size = len;
    }

#ifdef CAMERA_MIPS_CALC
    TIME_CALC_END(0);
#endif

#else /* !CAMERA_DATA_RAW10_TO_YUV */
    cur_image_buff = (uint8_t *)addr;
    cur_image_size = len;
#endif

    mipi_data_msg_pack(&disp_frame_msg, (uint32_t)cur_image_buff, cur_image_size, seq);

    // check msg lost count
#ifdef CAMERA_MIPS_CALC
    if (msg->lost_cnt > 0) {
        TRACE(1, "[DISP_PROCESS]: WARN2: lost msg, cnt=%d", msg->lost_cnt);
    }
#endif
    mipi_data_msg_reset(msg);
}

#endif /* CHIP_ROLE_AP */

#if defined(RTOS) && defined(AP_DISP_THREAD)
static void display_thread(void const *arguments)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        if (evt.status == osEventSignal) {
            if (signals & CAMERA_FRAME_MSG_COME) {
                camera_data_display_proc(&cam_frame_msg);
            }
        }
    }
}
#endif

#ifdef CHIP_ROLE_AP
static void ap_data_notify(uint32_t id, uint32_t stream, uint32_t addr, uint32_t len, uint32_t seq)
{
#if defined(AP_VPLAYER_APP_DEBUG) && (AP_VPLAYER_APP_DEBUG > 0)
    TRACE(1, "[AP_TX_DATA][seq-%d]:%d/%d/%x/%d", seq, id, stream, addr, len);
#endif

    vplayer_rpc_stream_data_notify(id, stream, addr, len, seq);
}
#endif

static void loop_stream_cap_data_handler(uint32_t addr, uint32_t len, uint32_t seq)
{
    POSSIBLY_UNUSED int ret = 0;

#ifdef CHIP_ROLE_AP
    ap_data_notify(VP_STREAM_ID_0, VP_STREAM_CAPTURE, addr, len ,seq);

#else /* !defined(CHIP_ROLE_AP) */
    ret = mipi_data_msg_pack(&cam_frame_msg, addr, len, seq);
    if (ret) {
        TRACE(1, "[%s]: WARNING: pack msg failed, ret=%d", __func__, ret);
    }
#if defined(RTOS)
    osSignalSet(disp_thread_tid, CAMERA_FRAME_MSG_COME);
#else
    camera_data_display_proc(&cam_frame_msg);
#endif
#endif /* CHIP_ROLE_AP */
}

static void loop_stream_play_data_handler(uint32_t addr, uint32_t len, uint32_t seq)
{
    uint32_t lock;
    uint8_t *buf = NULL;
    bool update = false;
    mipi_data_msg_t *msg = &disp_frame_msg;

    if (msg->frame_cnt > 0) {
        lock = int_lock();

        buf = (uint8_t *)msg->data_addr;
        update = true;
        mipi_data_msg_reset(msg);

        int_unlock(lock);
    }
    if (update) {
        vplayer_stream_update_dma_addr(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, buf, false);
    }
}

static POSSIBLY_UNUSED uint32_t capture_stream_data_handler(uint8_t *buf, uint32_t len)
{
    static uint32_t old_time = 0;

    uint32_t time = hal_sys_timer_get();

#ifdef CAMERA_IMAGE_DUMP
    camera_data_dump2(buf, len, 32);
    camera_data_dump_trap();
#endif

#ifdef CAMERA_TEST_DATA_CHECK
    camera_test_data_check_run(buf, len);
#endif

#ifdef CAMERA_DATA_DUMP
     camera_data_dump(buf, CAMERA_DATA_DUMP_NUM, 16);
#endif

    video_stream_chan_data_handler(VP_STREAM_CAPTURE, buf, len);

    if (old_time == 0) {
        old_time = time;
    }
//    TRACE(0, "[CAMERA_STREAM ][%d]: buf=%x, len=%x", TICKS_TO_MS(time - old_time), (int)buf, len);
    old_time = time;

    return 0;
}

#ifdef CAMERA_TEST_DATA_CHECK
#define CAMERA_TEST_MODE_SWITCH
#ifdef CAMERA_TEST_MODE_SWITCH
static void camera_test_mode_switch(void)
{
    static uint8_t cam_test_mode = 0x1;

    int desc_id = g_cam_desc_id;
    int device = g_cam_device;

    if (device == VP_DEVICE_GC2145) {
        cam_test_mode++;
        if (cam_test_mode > 0xF) {
            cam_test_mode = 1;
        }
        camera_device_test_mode(desc_id, true, cam_test_mode, 0, 0);
    }
}
#endif

static void camera_data_check_callback(int err_cnt, int chk_cnt, int seq)
{
    if (err_cnt > 0) {
        TRACE(1, "[CAMREA_TEST_DATA_CHECK_ERROR][%d][%d/%d]", seq, err_cnt, chk_cnt);
#ifdef CSI_SLT_TEST
        camera_test_data_check_enable(false);
        hal_cmu_simu_fail();
        TRACE(1, "CMU SIMU FAIL FLAG");
        ASSERT(false, "[%s]: CPU ASSERTED BY CAMERA DATA CHECK ERROR", __func__);
#endif
    } else {
        TRACE(1, "[CAMREA_TEST_DATA_CHECK_SUCESS][%d]", seq);
        hal_cmu_simu_pass();
#ifdef CAMERA_TEST_MODE_SWITCH
        camera_test_mode_switch();
#endif
    }
}
#endif

static uint32_t playback_stream_data_handler(uint8_t *buf, uint32_t len)
{
    static uint32_t old_time = 0;

    uint32_t time = hal_sys_timer_get();;

    video_stream_chan_data_handler(VP_STREAM_PLAYBACK, buf, len);

    if (old_time == 0) {
        old_time = time;
    }
//    TRACE(0, "[DISPLAY_STREAM][%d]: buf=%x, len=%x", TICKS_TO_MS(time - old_time), (int)buf, len);
    old_time = time;
    return 0;
}

int vplayer_stream_buff_malloc(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint8_t **ptr, uint32_t len)
{
    uint8_t *buf = NULL;

#if defined(AP_VPLAYER_APP_USE_HEAP)
    buf = media_heap_malloc(len);
#else
    if (stream == VP_STREAM_CAPTURE) {
        buf = (uint8_t *)CSI_DATA_BUF_ADDR;
    } else {
        buf = (uint8_t *)DSI_DATA_BUF_ADDR;
    }
#endif
    if (ptr) {
        *ptr = buf;
    }
    TRACE(1, "[%s]: id=%d, stream=%d, buf=%x, len=%d", __func__, id, stream, (int)buf, len);
    return 0;
}

void vplayer_stream_buff_free(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint8_t *buf)
{
    TRACE(1, "[%s]: id=%d, stream=%d, buf=%x", __func__, id, stream, (int)buf);

#if defined(AP_VPLAYER_APP_USE_HEAP)
    media_heap_free(buf);
#endif
}

int ap_vplayer_app_init(enum VPLAYER_APP_MODE_T mode)
{
    int ret = 0;

    TRACE(1, "[%s]: mode=%d", __func__, mode);

    vp_app_mode = mode;

#if defined(AP_VPLAYER_APP_USE_HEAP)
    media_heap_init();
#endif

    vplayer_open();

    if (vp_app_mode == VPLAYER_APP_MODE_CSI_DSI_LOOP
        || vp_app_mode == VPLAYER_APP_MODE_CSI) {

        ret = vplayer_stream_probe_device(VP_STREAM_CAPTURE, &g_cam_device);
        if (ret) {
            TRACE(1, "[%s]: WARNING: probe device failed, ret=%d", __func__, ret);
            return -1;
        }
        TRACE(1, "g_cam_device=%d", g_cam_device);

        ret = vplayer_stream_get_device_cfg(VP_STREAM_CAPTURE, g_cam_device, &g_cam_config);
        if (ret) {
            TRACE(1, "[%s]: WARNING: get device cfg failed, ret=%d", __func__, ret);
            return -2;
        }

        vplayer_device_dump_cfg("CAMERA", &g_cam_config);

#if defined(CSI_SENS_CALIB)
        if (g_cam_config.op_flag & CAMERA_OP_FLAG_CALIB_PHY) {
            uint8_t *img_buf;
            uint32_t img_size, pix_size;
            struct VP_DEVICE_CONFIG_T *cfg = &g_cam_config;

            pix_size = pixel_fmt_to_size(cfg->fmt);
            img_size = calc_image_size(cfg->width, cfg->height, pix_size, CAMERA_DATA_FRAME_NUM);

            vplayer_stream_buff_malloc(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &img_buf, img_size);
            TRACE(1, "pix_size=%d, img_size=0x%x, img_buf=%x", pix_size, img_size, (int)img_buf);

            vplayer_stream_calib_start(VP_STREAM_CAPTURE, g_cam_device,
                cfg->width, cfg->height, cfg->fmt, img_buf, img_size);

            osDelay(100);
            vplayer_stream_buff_free(VP_STREAM_ID_0, VP_STREAM_CAPTURE, img_buf);
        }
#endif
    }

    video_stream_chan_init();

#ifdef VIDEO_RECORD_ENABLE
    video_record_init();
#endif

#if defined(RTOS) && defined(AP_DISP_THREAD)
    if (disp_thread_tid == NULL) {
        disp_thread_tid = osThreadCreate(osThread(display_thread), NULL);
        osSignalSet(disp_thread_tid, 0x0);
    }
#endif
    cur_image_size = 0;
    image_buff_idx = 0;
    return 0;
}

void ap_vplayer_app_request_clock(void)
{
#ifdef NOAPP
    hal_sysfreq_req(SYSFREQ_USER_VIDEO_PLAYER, HAL_CMU_FREQ_390M);
#endif
}

void ap_vplayer_app_release_clock(void)
{
#ifdef NOAPP
    hal_sysfreq_req(SYSFREQ_USER_VIDEO_PLAYER, HAL_CMU_FREQ_32K);
#endif
}

void ap_vplayer_app_setup_config(enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    uint8_t *buf;
    uint32_t len, psize;
    POSSIBLY_UNUSED int ret = 0;

    memset((void *)cfg, 0, sizeof(struct VP_STREAM_CONFIG_T));

    if (stream == VP_STREAM_PLAYBACK) {
        cfg->width       = PLAYBACK_WIDTH;
        cfg->height      = PLAYBACK_HEIGHT;
        cfg->fmt         = PLAYBACK_FMT;
        cfg->channel_num = PLAYBACK_CHAN_NUM;
        cfg->device      = PLAYBACK_DEVICE;
        cfg->layer_id    = VP_LAYER_ID_0;
        cfg->handler     = playback_stream_data_handler;

        if (vp_app_mode == VPLAYER_APP_MODE_CSI_DSI_LOOP) {
            struct VP_DEVICE_CONFIG_T *cam_cfg = &g_cam_config;

            psize = pixel_fmt_to_size(cam_cfg->fmt);
            len = calc_image_size(cam_cfg->width, cam_cfg->height, psize, CAMERA_DATA_FRAME_NUM);
        } else {
            psize = pixel_fmt_to_size(cfg->fmt);
            len = calc_image_size(cfg->width, cfg->height, psize, DISPLAY_DATA_FRAME_NUM);
        }
#ifdef CHIP_ROLE_AP
        ret = vplayer_rpc_stream_get_buff(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, &buf, len);
#else
        ret = vplayer_stream_buff_malloc(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, &buf, len);
#endif
        ASSERT(ret==0, "[%s]: get playback buff failed, ret=%d", __func__, ret);

        cfg->data_ptr  = buf;
        cfg->data_size = len;

        TRACE(1, "PLAY STREAM: w=%d, h=%d, fmt=%d, dev=%d, chnum=%d, buf=%x, len=%d",
            cfg->width, cfg->height, cfg->fmt, cfg->device, cfg->channel_num,
            (int)cfg->data_ptr, len);

        display_load_boot_image_data(buf, len / 2);
        display_load_boot_image_data(buf + len / 2, len / 2);
        vplayer_stream_dump_cfg("PLAYBACK", cfg);
    } else {
        struct VP_DEVICE_CONFIG_T *cam_cfg = &g_cam_config;

        ASSERT(g_cam_device != VP_DEVICE_NULL, "[%s]: null camera device!", __func__);
        ASSERT(cam_cfg->channel_num == 2, "[%s]: Bad channel_num=%d", __func__, cam_cfg->channel_num);
        cfg->width       = cam_cfg->width;
        cfg->height      = cam_cfg->height;
        cfg->fmt         = cam_cfg->fmt;
        cfg->channel_num = cam_cfg->channel_num;
        cfg->device      = g_cam_device;
        cfg->handler     = capture_stream_data_handler;

        psize = pixel_fmt_to_size(cfg->fmt);
        len = calc_image_size(cfg->width, cfg->height, psize, CAMERA_DATA_FRAME_NUM);
#ifdef CHIP_ROLE_AP
        ret = vplayer_rpc_stream_get_buff(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &buf, len);
#else
        ret = vplayer_stream_buff_malloc(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &buf, len);
#endif
        ASSERT(ret==0, "[%s]: get capture buff failed, ret=%d", __func__, ret);

        cfg->data_ptr  = buf;
        cfg->data_size = len;

        TRACE(1, "CAP STREAM: w=%d, h=%d, fmt=%d, dev=%d, chnum=%d, buf=%x, len=%d",
            cfg->width,  cfg->height, cfg->fmt, cfg->device, cfg->channel_num,
            (int)cfg->data_ptr, len);
        vplayer_stream_dump_cfg("CAPTURE", cfg);
    }
}

void ap_vplayer_app_on(bool on)
{
    int ret = 0;

    if (ap_vplayer_status == on) {
        TRACE(1, "[%s]: ap_vplayer_status=%d", __func__, ap_vplayer_status);
        return;
    }
    TRACE(1, "[%s]: on=%d", __func__, on);

    if (vp_app_mode == VPLAYER_APP_MODE_CSI_DSI_LOOP) {
        play_stream_en = true;
        cap_stream_en = true;
        TRACE(1, "CSI DSI LOOP");
    } else if (vp_app_mode == VPLAYER_APP_MODE_CSI) {
        cap_stream_en = true;
        play_stream_en = false;
        TRACE(1, "CSI only");
    } else if (vp_app_mode == VPLAYER_APP_MODE_DSI) {
        cap_stream_en = false;
        play_stream_en = true;
        TRACE(1, "DSI only");
    }
    if (on) {
        ap_vplayer_app_request_clock();
        if (cap_stream_en) {
            ap_vplayer_app_setup_config(VP_STREAM_CAPTURE, &cap_stream_cfg);
            if (vp_app_mode == VPLAYER_APP_MODE_CSI_DSI_LOOP) {
                display_device_setup_input_image_size(cap_stream_cfg.width, cap_stream_cfg.height);
            }
        }
        if (play_stream_en) {
            ap_vplayer_app_setup_config(VP_STREAM_PLAYBACK, &play_stream_cfg);
        }
        /* open playback & capture stream */
        if (play_stream_en) {
#ifdef CHIP_ROLE_AP
            ret = vplayer_rpc_stream_open(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, &play_stream_cfg);
            ASSERT(ret==0, "[%s]: open rpc playback stream failed, ret=%d", __func__, ret);
#endif
            ret = vplayer_stream_open(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, &play_stream_cfg);
            ASSERT(ret==0, "[%s]: open playback stream failed, ret=%d", __func__, ret);

            video_stream_chan_open(VP_STREAM_PLAYBACK, LOOP_STREAM_CHAN_ID, loop_stream_play_data_handler);
        }
        if (cap_stream_en) {
#ifdef CHIP_ROLE_AP
            ret = vplayer_rpc_stream_open(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &cap_stream_cfg);
            ASSERT(ret==0, "[%s]: open rpc capture stream failed, ret=%d", __func__, ret);
#endif
            ret = vplayer_stream_open(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &cap_stream_cfg);
            ASSERT(ret==0, "[%s]: stream open failed, ret=%d", __func__, ret);

            g_cam_desc_id = vplayer_stream_get_device_desc_id(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            TRACE(1, "[%s]: g_cam_desc_id=%d", __func__, g_cam_desc_id);

            video_stream_chan_open(VP_STREAM_CAPTURE, LOOP_STREAM_CHAN_ID, loop_stream_cap_data_handler);

#if defined(CAMERA_USE_SOFT_ISP) && !defined(CHIP_ROLE_AP)
            if (g_cam_config.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
                soft_isp_setup_device(g_cam_desc_id, g_cam_device, &g_cam_config);
                soft_isp_aec_open();
            }
#endif

#ifdef VIDEO_RECORD_ENABLE
            video_record_open(&cap_stream_cfg);
#endif

#ifdef CAMERA_TEST_DATA_CHECK
#ifdef CSI_SLT_TEST
            hal_cmu_simu_init();
#endif
            uint32_t pixel_size = pixel_fmt_to_size(cap_stream_cfg.fmt);
            camera_test_data_check_init(cap_stream_cfg.width, cap_stream_cfg.height, pixel_size,
                camera_data_check_callback, CAMERA_DATA_CHECK_MODE_ONESHOT);
            camera_test_data_check_enable(true);
#endif

        }

        /* start playback & capture stream */
        if (play_stream_en) {
#ifdef CHIP_ROLE_AP
            ret = vplayer_rpc_stream_start(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
            ASSERT(ret==0, "[%s]: rpc stream start failed, ret=%d", __func__, ret);
#endif
            ret = vplayer_stream_start(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
            ASSERT(ret==0, "[%s]: stream start failed, ret=%d", __func__, ret);

            video_stream_chan_start(VP_STREAM_PLAYBACK, LOOP_STREAM_CHAN_ID);
        }
        if (cap_stream_en) {
#ifdef CHIP_ROLE_AP
            ret = vplayer_rpc_stream_start(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            ASSERT(ret==0, "[%s]: rpc stream start failed, ret=%d", __func__, ret);
#endif
            ret = vplayer_stream_start(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            ASSERT(ret==0, "[%s]: stream start failed, ret=%d", __func__, ret);

            video_stream_chan_start(VP_STREAM_CAPTURE, LOOP_STREAM_CHAN_ID);
#ifdef VIDEO_RECORD_ENABLE
            video_record_start();
#endif
        }
    } else {
        /* stop playback & capture stream */
        if (cap_stream_en) {
            vplayer_stream_stop(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            video_stream_chan_stop(VP_STREAM_CAPTURE, LOOP_STREAM_CHAN_ID);

#if defined(CAMERA_USE_SOFT_ISP) && !defined(CHIP_ROLE_AP)
            if (g_cam_config.op_flag & CAMERA_OP_FLAG_SOFT_ISP) {
                soft_isp_aec_close();
            }
#endif

#ifdef VIDEO_RECORD_ENABLE
            video_record_stop();
#endif

#ifdef CAMERA_TEST_DATA_CHECK
            camera_test_data_check_enable(false);
#endif

#ifdef CHIP_ROLE_AP
            vplayer_rpc_stream_stop(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
#endif

        }
        if (play_stream_en) {
            vplayer_stream_stop(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
            video_stream_chan_stop(VP_STREAM_PLAYBACK, LOOP_STREAM_CHAN_ID);
#ifdef CHIP_ROLE_AP
            vplayer_rpc_stream_stop(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
#endif
        }

        /* close playback & capture stream */
        if (cap_stream_en) {
            vplayer_stream_close(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            video_stream_chan_close(VP_STREAM_CAPTURE, LOOP_STREAM_CHAN_ID);

#ifdef VIDEO_RECORD_ENABLE
            video_record_close();
#endif

#ifdef CHIP_ROLE_AP
            vplayer_rpc_stream_close(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
            vplayer_rpc_stream_free_buff(VP_STREAM_ID_0, VP_STREAM_CAPTURE,
                cap_stream_cfg.data_ptr, cap_stream_cfg.data_size);
#else
            vplayer_stream_buff_free(VP_STREAM_ID_0, VP_STREAM_CAPTURE, cap_stream_cfg.data_ptr);
            cap_stream_cfg.data_ptr = NULL;
#endif
        }
        if (play_stream_en) {
            vplayer_stream_close(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
            video_stream_chan_close(VP_STREAM_PLAYBACK, LOOP_STREAM_CHAN_ID);
#ifdef CHIP_ROLE_AP
            vplayer_rpc_stream_close(VP_STREAM_ID_0, VP_STREAM_PLAYBACK);
            vplayer_rpc_stream_free_buff(VP_STREAM_ID_0, VP_STREAM_PLAYBACK,
                play_stream_cfg.data_ptr, play_stream_cfg.data_size);
#else

            vplayer_stream_buff_free(VP_STREAM_ID_0, VP_STREAM_PLAYBACK, play_stream_cfg.data_ptr);
            play_stream_cfg.data_ptr = NULL;
#endif
        }
    }
    ap_vplayer_status = on;
}

#ifdef CHIP_ROLE_AP
static int camera_i2c_write(uint32_t *param, uint32_t num)
{
    uint32_t addr, val;
    uint32_t i;

    ASSERT(num <= RPC_RSP_MSG_PARAM_NUM, "[%s]: Bad num=%d", __func__, num);
    for (i = 0;i < num; i++) {
        addr = param[i] >> 16;
        val  = param[i] & 0xFFFF;
        if (addr != 0) {
            camera_device_bus_write(g_cam_desc_id, addr, val);
        }
    }
    return num;
}

static int camera_i2c_read(uint32_t *param, uint32_t num)
{
    uint32_t addr, val[RPC_RSP_MSG_PARAM_NUM] = {0};
    uint32_t i;

    ASSERT(num <= RPC_RSP_MSG_PARAM_NUM, "[%s]: Bad num=%d", __func__, num);
    for (i = 0;i < num; i++) {
        addr = param[i] >> 16;
        if (addr != 0) {
            camera_device_bus_read(g_cam_desc_id, addr, &val[i]);
            param[i] |= (val[i] & 0xFFFF);
        }
    }
    return num;
}

static void camera_i2c_write_notify(uint16_t cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    int ret;
    struct RPC_RSP_MSG_T msg;

    memset((void *)&msg, 0, sizeof(msg));
    msg.hdr.id = RPC_MSG_HDR_ID;
    msg.hdr.trans_cnt = RPC_RSP_MSG_PARAM_NUM;
    msg.cmd = cmd;
    msg.param[0] = p0;
    msg.param[1] = p1;
    msg.param[2] = p2;
    msg.param[3] = p3;
    msg.param[4] = p4;
    ret = vplayer_rpc_msg_enqueue(&g_msg_queue, &msg);
    ASSERT(ret == 0, "[%s]: msg enqueue failed, ret=%d", __func__, ret);

#if defined(AP_VPLAYER_APP_DEBUG) && (AP_VPLAYER_APP_DEBUG > 1)
    TRACE(1, "[%s]: cmd=%d, param=[%x/%x/%x/%x/%x]", __func__, cmd, p0, p1, p2, p3, p4);
#endif

#ifdef RTOS
    osSignalSet(ap_vp_tid, AP_SIG_APP_I2C_WR);
#endif
}

static void camera_i2c_read_notify(uint16_t cmd, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    int ret;
    struct RPC_RSP_MSG_T msg;

    memset((void *)&msg, 0, sizeof(msg));
    msg.hdr.id = RPC_MSG_HDR_ID;
    msg.hdr.trans_cnt = RPC_RSP_MSG_PARAM_NUM;
    msg.cmd = cmd;
    msg.param[0] = p0;
    msg.param[1] = p1;
    msg.param[2] = p2;
    msg.param[3] = p3;
    msg.param[4] = p4;
    ret = vplayer_rpc_msg_enqueue(&g_msg_queue, &msg);
    ASSERT(ret == 0, "[%s]: msg enqueue failed, ret=%d", __func__, ret);

#if defined(AP_VPLAYER_APP_DEBUG) && (AP_VPLAYER_APP_DEBUG > 1)
    TRACE(1, "[%s]: cmd=%d, param=[%x/%x/%x/%x/%x]", __func__, cmd, p0, p1, p2, p3, p4);
#endif

#ifdef RTOS
    osSignalSet(ap_vp_tid, AP_SIG_APP_I2C_RD);
#endif
}

#ifdef RTOS
static void ap_vplayer_thread(void const *arguments)
{
    osEvent evt;
    uint32_t signals = 0;
    struct RPC_RSP_MSG_T msg;
    int err;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        TRACE(1, "[%s]: hello signals=%x", __func__, signals);
        if (evt.status == osEventSignal) {
            if (signals & AP_SIG_APP_INIT) {
                ap_vplayer_app_init(vp_app_mode);
            }
            if (signals & AP_SIG_APP_ON) {
                ap_vplayer_app_on(true);
            }
            if (signals & AP_SIG_APP_OFF) {
                ap_vplayer_app_on(false);
            }
            if (signals & (AP_SIG_APP_I2C_WR | AP_SIG_APP_I2C_RD)) {
                int not_empty;
                do {
                    err = vplayer_rpc_msg_dequeue(&g_msg_queue, &msg);
                    if (!err) {
                        if (msg.cmd == RPC_MSG_VP_CAMERA_I2C_WR) {
                            camera_i2c_write(&msg.param[0], msg.hdr.trans_cnt);
                        } else if (msg.cmd == RPC_MSG_VP_CAMERA_I2C_RD) {
                            camera_i2c_read(&msg.param[0], msg.hdr.trans_cnt);
                            vplayer_fine_reply_msg_send(&msg);
                        } else {
                            TRACE(1, "WARNING: Bad cmd=%d", msg.cmd);
                        }
                    } else {
                        TRACE(1, "WARNING: dequeue err2: %d", err);
                    }
                    not_empty = vplayer_rpc_msg_queue_is_not_empty(&g_msg_queue);
                } while (not_empty);
            }
        }
    }
}
#endif

static void ap_vplayer_app_msg_rx_handler(char *data, uint32_t len)
{
    struct RPC_REQ_MSG_T *msg = (struct RPC_REQ_MSG_T *)data;
    int ret = 0;

    switch (msg->cmd) {
    case RPC_MSG_VP_STREAM_DATA_DONE:
    {
        POSSIBLY_UNUSED uint32_t addr, dlen, seq;
        POSSIBLY_UNUSED uint32_t id, stream;

        id       = msg->param[0];
        stream   = msg->param[1];
        addr     = msg->param[2];
        dlen     = msg->param[3];
        seq      = msg->param[4];

#if defined(AP_VPLAYER_APP_DEBUG) && (AP_VPLAYER_APP_DEBUG > 0)
        TRACE(1, "[AP_RX_DATA][%d]:%d/%d/%x/%d", seq, id, stream, addr, dlen);
#endif
        ret = mipi_data_msg_pack(&cam_frame_msg, addr, dlen, seq);
        if (ret) {
            TRACE(1, "[AP_RX_DATA]: WARNING, lost 1 frame");
        }
        camera_data_display_proc(&cam_frame_msg);
    }
        break;
    case RPC_MSG_VP_STREAM_ENCODER_DATA_DONE:
    {
        POSSIBLY_UNUSED uint32_t encoder_addr, encoder_dlen, encoder_seq;
        POSSIBLY_UNUSED uint32_t encoder_id, encoder_stream;

        encoder_id       = msg->param[0];
        encoder_stream   = msg->param[1];
        encoder_addr     = msg->param[2];
        encoder_dlen     = msg->param[3];
        encoder_seq      = msg->param[4];

        if(stream_notify_func) {
            stream_notify_func(encoder_id, encoder_stream, encoder_addr, encoder_dlen, encoder_seq);
        }
    }
        break;
    case RPC_MSG_VP_CAMERA_I2C_WR:
    {
        camera_i2c_write_notify(msg->cmd, msg->param[0], msg->param[1],
            msg->param[2], msg->param[3], msg->param[4]);
    }
        break;
    case RPC_MSG_VP_CAMERA_I2C_RD:
    {
        camera_i2c_read_notify(msg->cmd, msg->param[0], msg->param[1],
            msg->param[2], msg->param[3], msg->param[4]);
    }
        break;
    case RPC_MSG_VP_QRCODE_CMD:
    {
#ifdef QRCODE_TEST
        ap_qrcode_rpc_rx_handler((void *)msg, len);
#endif
    }
        break;
    default:
        TRACE(1, "[%s]: WARNING: ignore cmd: %d", __func__, msg->cmd);
        break;
    }
}

void ap_vplayer_app_open(enum VPLAYER_APP_MODE_T mode)
{
    TRACE(1, "[%s]: ", __func__);

    vp_app_mode = mode;

    vplayer_rpc_msg_queue_init(&g_msg_queue, g_rsp_msg, ARRAY_SIZE(g_rsp_msg));
#ifdef RTOS
    if (ap_vp_tid == NULL) {
        ap_vp_tid = osThreadCreate(osThread(ap_vplayer_thread), NULL);
        osSignalSet(ap_vp_tid, AP_SIG_APP_INIT);
    }
    TRACE(1, "ap_vp_tid=%x", (int)ap_vp_tid);
#endif

    vplayer_rpc_msg_setup_rx_handler(RPC_MSG_RX_HDLR_ID_0, ap_vplayer_app_msg_rx_handler);
}

void ap_vplayer_app_close(enum VPLAYER_APP_MODE_T mode)
{
    TRACE(1, "[%s]: ", __func__);
}

void ap_vplayer_app_trigger(bool on)
{
    if (on) {
        osSignalSet(ap_vp_tid, AP_SIG_APP_ON);
    } else {
        osSignalSet(ap_vp_tid, AP_SIG_APP_OFF);
    }
}

#endif /* CHIP_ROLE_AP */

void ap_vplayer_app_register_stream_ready(AP_VPLAYER_STREAM_READY_NOTIFY_T fn)
{
    stream_notify_func = fn;
}

#endif /* defined(VIDEO_PLAYER) && !defined(CHIP_ROLE_DSP) */
