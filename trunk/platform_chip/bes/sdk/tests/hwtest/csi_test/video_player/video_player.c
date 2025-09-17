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
#include "hal_sleep.h"
#include "csi_dsi_conf.h"
#include "video_player.h"
#include "video_misc.h"
#include "camera_drv.h"
#include "camera_sens_sc132gs.h"
#include "display_drv.h"
#include "lcdc_driver.h"
#if defined(SC132GS_STREAM_CHECK_ERR)
#include "mipiphy.h"
#endif

#define VPLAYER_DBG_EN

#ifdef VPLAYER_DBG_EN
#define VPLAYER_DEBUG() TRACE(0, "[%s]: ", __func__);
#else
#define VPLAYER_DEBUG(...) do{ } while(0)
#endif

enum VP_RESULT_T {
    VP_RES_SUCCESS = 0,
    VP_RES_FAILED = 1,
};

enum VP_PP_T {
    PP_PING  = 0,
    PP_PANG  = 1,
};

enum VP_STATUS_T {
    VP_STATUS_NULL = 0x00,
    VP_STATUS_OPEN_CLOSE = 0x01,
    VP_STATUS_STREAM_OPEN_CLOSE = 0x02,
    VP_STATUS_STREAM_START_STOP = 0x04,
    VP_STATUS_STREAM_PAUSE_RESTART = 0x08,
    VP_STATUS_MASK = 0x0F,
};

enum VP_STREAM_CALIB_STATE_T {
    VP_STREAM_CALIB_CLOSE = 0,
    VP_STREAM_CALIB_OPEN = 1,
    VP_STREAM_CALIB_DONE = 2,
};

struct vp_stream_cntl_t {
    uint8_t status;
    uint8_t pp_index;
    uint32_t pp_cnt;
};

struct VP_DSI_CONFIG_T {
    enum VP_PIXEL_FMT_T fmt;
    uint32_t width;
    uint32_t height;
    uint8_t layer_id;
    uint8_t channel_num;
    uint8_t *data_ptr;
    uint32_t data_size;
};

struct vp_stream_cfg_t {
    int dev_desc_id;
    struct vp_stream_cntl_t cntl;
    struct VP_DEVICE_CONFIG_T dev_cfg;
    struct VP_STREAM_CONFIG_T cfg;
    struct HAL_CSI_CONFIG_T csi_cfg;
    struct VP_DSI_CONFIG_T vp_dsi_cfg;
    uint8_t *dma_buf_ptr;
    uint32_t dma_buf_size;
    VP_STREAM_HANDLER_T handler;
};

static struct vp_stream_cfg_t vp_stream_cfg[VP_STREAM_ID_QTY][VP_STREAM_QTY];

static volatile int stream_calib_status = 0;
static bool stream_calib_finish = false;
static bool vp_open_status = false;

#if defined(SC132GS_STREAM_CHECK_ERR)
static uint32_t stream_error_count;
#endif

#define VP_CPU_WAKE_USER HAL_CPU_WAKE_LOCK_USER_16
#define VP_STACK_SIZE (0x4000)

#ifdef RTOS
osMutexId vp_mutex_id = NULL;
osMutexDef(vp_mutex);

static osThreadId vp_thread_tid = NULL;
static void vplayer_thread(void const *arguments);
osThreadDef(vplayer_thread, osPriorityHigh, 1, VP_STACK_SIZE, "video_player");
static int vp_default_priority = 0;
#else
static volatile uint32_t vp_flag_lock = 0;
static volatile uint32_t vp_flag_signal = 0;
static volatile uint32_t vp_flag_open = 0;
#endif

uint32_t hal_csi_get_err_status(void);

static uint32_t vp_csi_err_code = 0;

static VP_DMA_IRQ_NOTIFY_T vp_dma_irq_notify[VP_STREAM_QTY] = {NULL};

static uint32_t vp_sig_lost_cnt[VP_STREAM_ID_QTY][VP_STREAM_QTY];

static void vp_lock_thread(void)
{
#ifdef RTOS
    osMutexWait(vp_mutex_id, osWaitForever);
#else
    ASSERT(vp_flag_lock == 0, "[%s]: thread has been locked", __func__);
    vp_flag_lock = 1;
#endif
}

static void vp_unlock_thread(void)
{
#ifdef RTOS
    osMutexRelease(vp_mutex_id);
#else
    ASSERT(vp_flag_lock == 1, "[%s]: thread not locked", __func__);
    vp_flag_lock = 0;
#endif
}

static inline struct vp_stream_cfg_t *vplayer_get_stream_role(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    ASSERT(id < VP_STREAM_ID_QTY, "[%s]: Bad id %d", __func__, id);
    ASSERT(stream < VP_STREAM_QTY, "[%s]: Bad stream %d", __func__, stream);

    return &vp_stream_cfg[id][stream];
}

static void vplayer_set_status(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, enum VP_STATUS_T status)
{
    struct vp_stream_cfg_t *role;

    role = vplayer_get_stream_role(id, stream);

    role->cntl.status |= status;
}

static void vplayer_clear_status(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, enum VP_STATUS_T status)
{
    struct vp_stream_cfg_t *role;

    role = vplayer_get_stream_role(id, stream);

    role->cntl.status &= ~status;
}

#ifdef RTOS
static int vplayer_get_priority(void)
{
    return (int)osThreadGetPriority(vp_thread_tid);
}
#endif

int vplayer_open(void)
{
    int ret = 0;
    struct vp_stream_cfg_t *role = NULL;

    VPLAYER_DEBUG();

    if (vp_open_status) {
        return 0;
    }

#ifdef RTOS
    if (vp_mutex_id == NULL) {
        vp_mutex_id = osMutexCreate((osMutex(vp_mutex)));
    }
#endif
    vp_lock_thread();

    uint8_t stream, id;
    for (stream = VP_STREAM_PLAYBACK; stream < VP_STREAM_QTY; stream++) {
        for (id = 0; id < VP_STREAM_ID_QTY; id++) {
            role = vplayer_get_stream_role(id, stream);
            if (role->cntl.status == VP_STATUS_NULL) {
                role->dma_buf_ptr = NULL;
                role->dma_buf_size = 0;
                role->cntl.status = VP_STATUS_OPEN_CLOSE;
            } else {
                ASSERT(false, "[%s]: ERROR: id=%d, stream=%d", __func__, id, stream);
            }
        }
    }

#ifdef RTOS
    if (vp_thread_tid == NULL) {
        vp_thread_tid = osThreadCreate(osThread(vplayer_thread), NULL);
        vp_default_priority = vplayer_get_priority();
        osSignalSet(vp_thread_tid, 0x0);
    }
#endif

#ifndef __ARM_ARCH_ISA_ARM
#if defined(CHIP_HAS_A7_DSP) && !defined(__A7_DSP_ENABLE__) && defined(CSI_TEST)
    //TODO: remove these dangrous code
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();
#endif
#endif

    camera_device_open();

    vp_open_status = true;
    vp_unlock_thread();
    return ret;
}

int vplayer_device_setup(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    VPLAYER_DEBUG();

    role = vplayer_get_stream_role(id, stream);

    role->dev_cfg.width = cfg->width;
    role->dev_cfg.height = cfg->height;
    role->dev_cfg.channel_num = cfg->channel_num;
    role->dev_cfg.fmt = cfg->fmt;

    if (stream == VP_STREAM_PLAYBACK) {
        display_device_setup(cfg->device, &role->dev_cfg);
        display_device_power_on();
    } else {
        int desc_id = 0;
        enum VP_DEVICE_T cam_device;

        cam_device = cfg->device;
        //FIXME: rm NULL
        desc_id = camera_device_stream_open(cam_device, NULL, &role->dev_cfg);
        ASSERT(desc_id >= 0, "[%s]: camera device stream open failed, %d", __func__, desc_id);
        role->dev_desc_id = desc_id;
        TRACE(1, "[%s]: desc_id=%d, cam_device=%d", __func__, desc_id, cam_device);
    }

    return ret;
}

int vplayer_stream_get_device_desc_id(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    struct vp_stream_cfg_t *role;

    VPLAYER_DEBUG();

    role = vplayer_get_stream_role(id, stream);
    return role->dev_desc_id;
}

void vplayer_stream_update_dma_addr(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream,
    uint8_t *addr, bool lock_thread)
{
    uint32_t i;
    struct vp_stream_cfg_t *role = NULL;

    role = vplayer_get_stream_role(id, stream);

    ASSERT(addr != NULL, "[%s]: null ptr", __func__);
    if (lock_thread) {
        vp_lock_thread();
    }
    if (stream == VP_STREAM_PLAYBACK) {
        if ((uint32_t)addr != (uint32_t)(role->dma_buf_ptr)) {
            role->dma_buf_ptr = (uint8_t *)addr;
        }
        for (i = 0; i < VP_CHANNEL_MAP_QTY; i++) {
            lcdc_hw_update_dma_addr(role->vp_dsi_cfg.layer_id, i, (uint32_t)addr);
        }
    }
    if (lock_thread) {
        vp_unlock_thread();
    }
}

enum VP_CHANNEL_MAP_T vplayer_stream_get_channel_by_addr(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, uint8_t *buf)
{
    enum VP_CHANNEL_MAP_T channel;
    struct vp_stream_cfg_t *role = NULL;

    ASSERT(stream == VP_STREAM_PLAYBACK, "[%s]: Bad stream %d", __func__, stream);

    role = vplayer_get_stream_role(id, stream);
    if (buf == role->dma_buf_ptr) {
        channel = VP_CHANNEL_MAP_CH0;
    } else {
        channel = VP_CHANNEL_MAP_CH1;
    }
    return channel;
}

static void vplayer_thread_stream_handler(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    uint32_t lock;
    struct vp_stream_cfg_t *role = NULL;
    uint8_t *buf = NULL;
    uint32_t len;
    uint32_t pp_cnt;
    uint32_t pp_index;
    bool skip_handler;

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    if ((role->cntl.status & VP_STATUS_STREAM_START_STOP) && (role->handler)) {
        lock = int_lock();
        pp_cnt = role->cntl.pp_cnt;
        role->cntl.pp_cnt = 0;
        int_unlock(lock);

        if (pp_cnt > 0) {
            vp_sig_lost_cnt[id][stream] = pp_cnt - 1;
        }
        if (vp_sig_lost_cnt[id][stream]) {
            TRACE(1, "[vp_thread]: WARNING: id=%d, stream=%d lost %u signals", id, stream, vp_sig_lost_cnt[id][stream]);
        }

        lock = int_lock();

        len = role->dma_buf_size / 2;
        pp_index = role->cntl.pp_index;
        if (pp_index == PP_PANG) {
            buf = role->dma_buf_ptr;
        } else {
            buf = role->dma_buf_ptr + len;
        }
        pp_index ^= 1;
        role->cntl.pp_index = pp_index;

        int_unlock(lock);

        skip_handler = false;

        if (!skip_handler) {
            role->handler(buf, len);
        }

        if (role->cntl.pp_cnt) {
            TRACE(1, "[vp_thread]: WARNING: id=%d, stream=%d hdlr run tool long (pp_cnt=%d)",
                id, stream, role->cntl.pp_cnt);
        }
    }

    vp_unlock_thread();
}

#ifdef RTOS
static void vplayer_thread(void const *arguments)
{
    osEvent evt;
    uint32_t signals = 0;
    enum VP_STREAM_T stream;
    enum VP_STREAM_ID_T id;
    uint32_t i;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        if (evt.status == osEventSignal) {
            for (i = 0; i < VP_STREAM_ID_QTY * VP_STREAM_QTY; i++) {
                if (signals & (1 << i)) {
                    id = (enum VP_STREAM_ID_T)(i >> 1);
                    stream = (enum VP_STREAM_T)(i & 1);
                    vplayer_thread_stream_handler(id, stream);
                }
            }
        }
    }
}

#else /* !RTOS */
static void vp_set_flag(volatile uint32_t *flag, int signal)
{
    uint32_t lock;

    lock = int_lock();
    *flag |= signal;
    int_unlock(lock);
}

static void vp_clear_flag(volatile uint32_t *flag, uint32_t signal)
{
    uint32_t lock;

    lock = int_lock();
    *flag &= ~signal;
    int_unlock(lock);
}

static int vp_flag_active(volatile uint32_t *flag, uint32_t bits)
{
    return !!(*flag & bits);
}

void vplayer_thread(void const *arguments)
{
    uint32_t lock;
    uint32_t i;
    enum VP_STREAM_ID_T id;
    enum VP_STREAM_T stream;

    if (vp_flag_open == 0) {
        return;
    }
    for (i = 0; i < VP_STREAM_ID_QTY * VP_STREAM_QTY; i++) {
        if (!vp_flag_active(&vp_flag_signal, 1 << i)) {
            continue;
        }
        vp_clear_flag(&vp_flag_signal, 1 << i);
        id = (enum VP_STREAM_ID_T)(i >> 1);
        stream = (enum VP_STREAM_T)(i & 1);
        vplayer_thread_stream_handler(id, stream);
    }

    lock = int_lock();
    if (vp_flag_signal == 0) {
        hal_cpu_wake_unlock(VP_CPU_WAKE_USER);
    }
    int_unlock(lock);
}
#endif /* RTOS */

static POSSIBLY_UNUSED uint32_t vp_csi_dma_irq_dummy_handler(uint8_t *buf, uint32_t len, uint32_t errcode)
{
    if (stream_calib_status == VP_STREAM_CALIB_OPEN) {
        stream_calib_status = VP_STREAM_CALIB_DONE;
        TRACE(1, "[%s]: CALIB DONE: errcode=0x%x", __func__, errcode);
    }
    vp_csi_err_code = errcode;
    return 0;
}

static void gpio_trigger_csi_callback(int pin, enum GPIO_TRIG_STATE_T state)
{
    struct vp_stream_cfg_t *role;
    uint8_t id, stream = VP_STREAM_CAPTURE;
    uint8_t mask;
    uint32_t len;
    uint8_t *buf;
    HAL_CSI_STREAM_HANDLER_T handler;

    if (stream_calib_status != VP_STREAM_CALIB_CLOSE) {
        return;
    }

    if (state == TRIG_STATE_SYNC_BEGIN) {
        return;
    }
    mask = VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE | VP_STATUS_STREAM_START_STOP;
    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        role = vplayer_get_stream_role((enum VP_STREAM_ID_T)id, (enum VP_STREAM_T)stream);
        if (role->cntl.status != mask) {
            continue;
        }
        handler = role->csi_cfg.handler;
        if (handler) {
            buf = (uint8_t *)(role->csi_cfg.data_ptr);
            len = role->csi_cfg.data_size / 2;
            handler(buf, len, vp_csi_err_code);
        }
    }
}

#if defined(SC132GS_STREAM_CHECK_ERR)
static void vplayer_stream_check_error(void)
{
    uint16_t vh_lprx = 0;
    uint32_t errstatus = hal_csi_get_err_status();

    if(((errstatus) & 0x2A00) != 0) {
        if((stream_error_count % 2) == 0) {
            vh_lprx = 0x2;
        } else {
            vh_lprx = 0x6;
        }
        TRACE(0, "[%s]: err status err=0x%x", __func__, errstatus);
        csiphy_set_vh_lprx(vh_lprx);
        hal_cmu_reset_pulse(HAL_CMU_MOD_Q_CSI_LANE);
        hal_cmu_reset_pulse(HAL_CMU_MOD_Q_CSI_PIX);
        stream_error_count = stream_error_count + 1;
    }
}
#endif

static uint32_t vp_csi_dma_irq_handler(uint8_t *buf, uint32_t len, uint32_t errcode)
{
    int signal;
    uint8_t id;
    enum VP_STREAM_T stream = VP_STREAM_CAPTURE;
    struct vp_stream_cfg_t *role = NULL;
    bool tell = false;
    uint8_t channel = 0;

    if (stream_calib_status != VP_STREAM_CALIB_CLOSE) {
        return 0;
    }
//    TRACE(0, "[VP_CSI_IRQ]: buf=0x%x, len=0x%x, errcode=0x%x", (int)buf, len, errcode);

#if defined(SC132GS_STREAM_CHECK_ERR)
    vplayer_stream_check_error();
#endif

#if defined(CSI_FPS_STAT_ENABLE)
    video_fps_stat_calc(VIDEO_FPS_STAT_ID_CSI, errcode);
#endif

    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        role = vplayer_get_stream_role((enum VP_STREAM_ID_T)id, stream);
        // check the buffer address to determine if tell the player thread or not
        uint8_t *start_addr = role->dma_buf_ptr;
        uint8_t *end_addr  = role->dma_buf_ptr + role->dma_buf_size;
        if ((buf >= start_addr) && (buf < end_addr)) {
            if (buf == start_addr) {
                channel = 0;
            } else {
                channel = 1;
            }
            role->cntl.pp_cnt++;
            signal = 0x01 << (id * 2 + stream);
            tell = true;
            break;
        }
    }
    if (tell) {
        VP_DMA_IRQ_NOTIFY_T notify = vp_dma_irq_notify[stream];
        if (notify) {
            notify((enum VP_STREAM_ID_T)id, stream, channel, errcode);
        }
#if defined(RTOS)
        osSignalSet(vp_thread_tid, signal);
#else
        vp_set_flag(&vp_flag_signal, signal);
        hal_cpu_wake_lock(VP_CPU_WAKE_USER);
#endif
    }
    return 0;
}

POSSIBLY_UNUSED static void vp_dsi_dma_irq_handler(uint8_t layer_id, uint8_t channel, uint32_t addr)
{
    int signal;
    uint8_t id;
    enum VP_STREAM_T stream = VP_STREAM_PLAYBACK;
    struct vp_stream_cfg_t *role = NULL;
    bool tell = false;

#if defined(DSI_FPS_STAT_ENABLE)
    if (channel == 0) {
        video_fps_stat_calc(VIDEO_FPS_STAT_ID_DSI, channel);
    }
#endif

    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        role = vplayer_get_stream_role((enum VP_STREAM_ID_T)id, stream);
        // check the layer id to determine if tell the player thread or not
        if (role->vp_dsi_cfg.layer_id == layer_id) {
            role->cntl.pp_cnt++;
            signal = 0x01 << (id * 2 + stream);
            tell = true;
            break;
        }
    }
    if (tell) {
        VP_DMA_IRQ_NOTIFY_T notify = vp_dma_irq_notify[stream];
        if (notify) {
            notify((enum VP_STREAM_ID_T)id, stream, channel, 0);
        }
#if defined(RTOS)
        osSignalSet(vp_thread_tid, signal);
#else
        vp_set_flag(&vp_flag_signal, signal);
        hal_cpu_wake_lock(VP_CPU_WAKE_USER);
#endif
    }
}

int vplayer_stream_setup(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);
    if (stream == VP_STREAM_CAPTURE) {
        struct HAL_CSI_CONFIG_T *csi_cfg = &role->csi_cfg;

        memset((void *)csi_cfg, 0, sizeof(struct HAL_CSI_CONFIG_T));

        TRACE(1, "width=%d, height=%d, data_lane=%d", cfg->width, cfg->height, cfg->channel_num);
        csi_cfg->fmt = (enum CSI_PIX_FMT_T)cfg->fmt;
        csi_cfg->width = cfg->width;
        csi_cfg->height = cfg->height;
        csi_cfg->pp_mode = 1;
        csi_cfg->data_lane_num = CSI_DATA_LANE_NUM_2;
        csi_cfg->data_ptr = cfg->data_ptr;
        csi_cfg->data_size = cfg->data_size;
#if defined(CSI_USE_GPIO_TRIG)
        csi_cfg->handler = vp_csi_dma_irq_dummy_handler;
#else
        csi_cfg->handler = vp_csi_dma_irq_handler;
#endif
        hal_csi_open(csi_cfg);

#if defined(CSI_USE_GPIO_TRIG)
        csi_cfg->handler = vp_csi_dma_irq_handler;

        struct GPIO_TRIG_CONFIG_T trig_cfg;
        trig_cfg.callback = gpio_trigger_csi_callback;
        gpio_trigger_csi_open(&trig_cfg);
#endif

#if defined(CSI_FPS_STAT_ENABLE)
        video_fps_stat_open(VIDEO_FPS_STAT_ID_CSI);
#endif
    } else {
        struct VP_DSI_CONFIG_T *dsi_cfg = &role->vp_dsi_cfg;

        memset((void *)dsi_cfg, 0, sizeof(struct VP_DSI_CONFIG_T));
        dsi_cfg->fmt = cfg->fmt;
        dsi_cfg->width = cfg->width;
        dsi_cfg->height = cfg->height;
        dsi_cfg->layer_id = cfg->layer_id;
        dsi_cfg->channel_num = cfg->channel_num;
        dsi_cfg->data_ptr = cfg->data_ptr;
        dsi_cfg->data_size = cfg->data_size;

#if defined(DSI_FPS_STAT_ENABLE)
        video_fps_stat_open(VIDEO_FPS_STAT_ID_DSI);
#endif

        lcdc_hw_setup_callback(vp_dsi_dma_irq_handler);
    }

    return ret;
}

int vplayer_stream_open(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T *cfg)
{
    int ret = VP_RES_SUCCESS;
    struct vp_stream_cfg_t *role;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);
    ASSERT(cfg->data_ptr != NULL, "[%s]: null ptr", __func__);
    ASSERT(cfg->data_size != 0, "[%s]: zero data size", __func__);
    ASSERT(cfg->height != 0, "[%s]: zero height", __func__);
    ASSERT(cfg->width != 0, "[%s]: zero width", __func__);

    vp_lock_thread();

    if (role->cntl.status != VP_STATUS_OPEN_CLOSE) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }

    if (stream == VP_STREAM_CAPTURE) {
        ASSERT(cfg->device == VP_DEVICE_GC2145
            || cfg->device == VP_DEVICE_SC132GS,
            "[%s]: Invalid capture device %d", __func__, cfg->device);
        ASSERT(cfg->channel_num == VP_CHANNEL_NUM_2,
            "[%s]: Bad capture channel number %d", __func__, cfg->channel_num);
    } else {
        ASSERT(cfg->device == VP_DEVICE_A064
            , "[%s]: Invalid playback device %d", __func__, cfg->device);

        ASSERT(cfg->channel_num == VP_CHANNEL_NUM_2,
            "[%s]: Bad playback channel number %d", __func__, cfg->channel_num);
    }

    role->dma_buf_ptr = cfg->data_ptr;
    role->dma_buf_size = cfg->data_size;
    role->cfg = *cfg;
    role->handler = cfg->handler;
    vplayer_device_setup(id, stream, &role->cfg);
    vplayer_stream_setup(id, stream, &role->cfg);
    vplayer_set_status(id, stream, VP_STATUS_STREAM_OPEN_CLOSE);

#if !defined(RTOS)
    vp_clear_flag(&vp_flag_signal, 1 << (id * 2 + stream));
    vp_set_flag(&vp_flag_open, 1 << (id * 2 + stream));
#endif

_exit:
    vp_unlock_thread();
    return ret;
}

int vplayer_stream_get_cfg(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream, struct VP_STREAM_CONFIG_T **cfg)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    VPLAYER_DEBUG();

    role = vplayer_get_stream_role(id, stream);
    if (role->cntl.status & VP_STATUS_STREAM_OPEN_CLOSE) {
        if (cfg) {
            *cfg = &role->cfg;
            ret = VP_RES_SUCCESS;
        } else {
            ret = VP_RES_FAILED;
        }
    }
    return ret;
}

int vplayer_stream_start(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    if (role->cntl.status != (VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE)) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }
    role->cntl.pp_index = 0;
    role->cntl.pp_cnt = 0;

    if (stream == VP_STREAM_CAPTURE) {
        hal_csi_start();
        camera_device_stream_start(role->dev_desc_id);
#if defined(CSI_USE_GPIO_TRIG)
        gpio_trigger_csi_enable(true);
#endif
#if defined(CSI_FPS_STAT_ENABLE)
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_CSI, true);
#endif
    } else {
        display_device_open();
        display_device_start();
#if defined(DSI_FPS_STAT_ENABLE)
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_DSI, true);
#endif
    }
    vplayer_set_status(id, stream, VP_STATUS_STREAM_START_STOP);

_exit:
    vp_unlock_thread();
    return ret;
}

int vplayer_stream_stop(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    if (role->cntl.status != (VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE
        | VP_STATUS_STREAM_START_STOP)) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }
    if (stream == VP_STREAM_CAPTURE) {
        camera_device_stream_stop(role->dev_desc_id);
        hal_csi_stop();
#if defined(CSI_USE_GPIO_TRIG)
        gpio_trigger_csi_enable(false);
#endif
#if defined(CSI_FPS_STAT_ENABLE)
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_CSI, false);
#endif
    } else {
        display_device_stop();
        display_device_reset();
#if defined(DSI_FPS_STAT_ENABLE)
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_DSI, false);
#endif
    }
    vplayer_clear_status(id, stream, VP_STATUS_STREAM_START_STOP);

_exit:
    vp_unlock_thread();
    return ret;
}

int vplayer_stream_close(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    if (role->cntl.status != (VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE)) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }
    if (stream == VP_STREAM_CAPTURE) {
        camera_device_stream_close(role->dev_desc_id);
        hal_csi_close();
#if defined(CSI_USE_GPIO_TRIG)
        gpio_trigger_csi_close();
#endif
#if defined(CSI_FPS_STAT_ENABLE)
        video_fps_stat_close(VIDEO_FPS_STAT_ID_CSI);
#endif
    } else {
        display_device_close();
        display_device_power_off();
#if defined(DSI_FPS_STAT_ENABLE)
        video_fps_stat_close(VIDEO_FPS_STAT_ID_DSI);
#endif
    }
    vplayer_clear_status(id, stream, VP_STATUS_STREAM_OPEN_CLOSE);

#if !defined(RTOS)
    vp_clear_flag(&vp_flag_open, 1 << (id * 2 + stream));
    if (vp_flag_open == 0) {
        hal_cpu_wake_unlock(VP_CPU_WAKE_USER);
    }
#endif

_exit:
    vp_unlock_thread();
    return ret;
}

int vplayer_stream_pause(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;
    uint8_t mask;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    mask = VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE | VP_STATUS_STREAM_START_STOP;

    if (role->cntl.status != mask) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }
    if (stream == VP_STREAM_CAPTURE) {
        camera_device_stream_stop(role->dev_desc_id);
    } else {
        display_device_stop();
    }
    vplayer_set_status(id, stream, VP_STATUS_STREAM_PAUSE_RESTART);

_exit:
    vp_unlock_thread();
    return ret;
}

int vplayer_stream_resume(enum VP_STREAM_ID_T id, enum VP_STREAM_T stream)
{
    int ret = 0;
    struct vp_stream_cfg_t *role;
    uint8_t mask;

    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    role = vplayer_get_stream_role(id, stream);

    vp_lock_thread();

    mask = VP_STATUS_OPEN_CLOSE | VP_STATUS_STREAM_OPEN_CLOSE | VP_STATUS_STREAM_START_STOP
        | VP_STATUS_STREAM_PAUSE_RESTART;

    if (role->cntl.status != mask) {
        TRACE(1, "[%s]: ERROR: status=%x", __func__, role->cntl.status);
        ret = VP_RES_FAILED;
        goto _exit;
    }
    if (stream == VP_STREAM_CAPTURE) {
        camera_device_stream_start(role->dev_desc_id);
    } else {
        display_device_start();
    }
    vplayer_clear_status(id, stream, VP_STATUS_STREAM_PAUSE_RESTART);

_exit:
    vp_unlock_thread();
    return ret;
    TRACE(0, "[%s]: id=%d, stream=%d", __func__, id, stream);

    return ret;
}

void vplayer_setup_irq_notify(enum VP_STREAM_T stream, VP_DMA_IRQ_NOTIFY_T notify)
{
    vp_dma_irq_notify[stream] = notify;
}

int vplayer_close(void)
{
    int ret = VP_RES_SUCCESS;
    struct vp_stream_cfg_t *role;
    uint8_t id, stream;

    VPLAYER_DEBUG();

    if (!vp_open_status) {
        return 0;
    }
    for (id = 0; id < VP_STREAM_ID_QTY; id++) {
        for (stream = 0; stream < VP_STREAM_QTY; stream++) {
            role = vplayer_get_stream_role((enum VP_STREAM_ID_T)id, (enum VP_STREAM_T)stream);
            role->cntl.status = VP_STATUS_NULL;
        }
    }
    vp_open_status = false;

    camera_device_close();

    return ret;
}

#define CALIB_CHAN_NUM     (VP_CHANNEL_NUM_2)
#define CALIB_CHAN_MAP     (VP_CHANNEL_MAP_CH0 | VP_CHANNEL_MAP_CH1)
#define CALIB_BUF_MIN_SIZE(w, h) ((w) * (h) * 2 * 2)

static uint32_t vp_capture_stream_calib_handler(uint8_t *buf, uint32_t len)
{
    TRACE(0, "[%s]: buf=%x, len=%d", __func__, (int)buf, len);
    return 0;
}

void vplayer_stream_calib_init(enum VP_STREAM_T stream, enum VP_DEVICE_T device,
    uint32_t width, uint32_t height, uint8_t fmt)
{
    struct HAL_CSI_CALIB_CONFIG_T calib_cfg;
    uint32_t dsp_pll_clk = hal_cmu_get_dsp_pll_source_clock();

    TRACE(1, "[%s]: device=%d, stream=%d", __func__, device, stream);

    hal_csi_get_calib_value(&calib_cfg);

    if (0) {
    }
#ifdef CSI_SENS_SC132GS
    else if (device == VP_DEVICE_SC132GS) {
        uint32_t mipi_clock;

        calib_cfg.clock_range   = 0x0; //0:1GHz, 1:500Mhz, 2:300MHz, 3:200MHz
        calib_cfg.clock_delay   = 0x0;
        calib_cfg.data_delay[0] = 0x0;
        calib_cfg.data_delay[0] = 0x0;
        calib_cfg.prpr_time     = 0x12;

        mipi_clock = sc132gs_sensor_calc_mipi_clock(width, height, 10, 60);
        if (mipi_clock <= 150000000) {
            calib_cfg.pix_clk_mhz = 50;
            calib_cfg.r_hdly[0]   = 0x040;
        } else {
            calib_cfg.pix_clk_mhz = 100;
            calib_cfg.r_hdly[0]   = 0x00FF;
        }
        TRACE(1, "update SC132GS calib param done");
    }
#endif
#ifdef CSI_SENS_GC2145
    else if (device == VP_DEVICE_GC2145) {
        calib_cfg.pix_clk_mhz = 50;
        calib_cfg.r_hdly[0]   = 0x030F;
        calib_cfg.prpr_time   = 0x15;
        TRACE(1, "update GC2145 calib param done");
    }
#endif
    hal_csi_set_calib_value(&calib_cfg);
    TRACE(1, "DSP_PLL_CLK=%d MHz, PIX_CLK_MHz=%d", dsp_pll_clk, calib_cfg.pix_clk_mhz);
    TRACE(1, "[%s]: update calib param for device %d", __func__, device);
}

int vplayer_stream_probe_device(enum VP_STREAM_T stream, enum VP_DEVICE_T *device)
{
    enum VP_DEVICE_T dev = VP_DEVICE_NULL;
    struct VP_DEVICE_BUS_T dev_bus;
    bool success;

    if (stream == VP_STREAM_CAPTURE) {
        success = camera_device_probe(&dev, &dev_bus);
        if (!success) {
            TRACE(1, "[%s]: probe camera device failed", __func__);
            return -1;
        }
        if (device) {
            *device = dev;
        }
    } else {
        //TODO: probe playback device
    }
    return 0;
}

int vplayer_stream_get_device_cfg(enum VP_STREAM_T stream, enum VP_DEVICE_T device,
    struct VP_DEVICE_CONFIG_T *cfg)
{
    int ret = 0;

    if (stream == VP_STREAM_CAPTURE) {
        ret = camera_device_get_config(device, cfg);
        if (ret) {
            TRACE(1, "[%s]: get camera device cfg failed, ret=%d", __func__, ret);
        }
    } else {
    }
    return ret;
}

int vplayer_stream_calib_start(enum VP_STREAM_T stream, enum VP_DEVICE_T device,
    uint32_t width, uint32_t height, uint8_t fmt, uint8_t *buf, uint32_t len)
{
    int ret = 0;
    bool need_open = false;

#ifdef CSI_PHY_NO_CALIB
    TRACE(1, "[%s]: skip calib", __func__);
    return 0;
#endif

    TRACE(1, "[%s]: stream=%d, device=%d, width=%d, height=%d", __func__, stream, device, width, height);

    if (stream_calib_finish) {
        TRACE(1, "has finished");
        return 0;
    }

    ASSERT(device == VP_DEVICE_GC2145
        || device == VP_DEVICE_SC132GS,
        "[%s]: Bad device %d", __func__, device);

    stream_calib_status = VP_STREAM_CALIB_OPEN;

    if (!vp_open_status) {
        need_open = true;
    }
    if (need_open) {
        vplayer_open();
    }

    vplayer_stream_calib_init(stream, device, width, height, fmt);

    struct VP_STREAM_CONFIG_T stream_cfg;
    volatile int timeout = 200;

    memset((void *)&stream_cfg, 0, sizeof(stream_cfg));
    ASSERT(len >= CALIB_BUF_MIN_SIZE(width, height), "[%s]: Bad len %d", __func__, len);
    stream_cfg.width       = width;
    stream_cfg.height      = height;
    stream_cfg.fmt         = fmt;
    stream_cfg.channel_num = CALIB_CHAN_NUM;
    stream_cfg.device      = device;
    stream_cfg.handler     = vp_capture_stream_calib_handler;
    stream_cfg.data_ptr    = buf;
    stream_cfg.data_size   = len;

    TRACE(1, "[%s]: DEVICE(%d) CALIB START", __func__, device);
    ret = vplayer_stream_open(VP_STREAM_ID_0, VP_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret==0, "[%s]: stream open failed, ret=%d", __func__, ret);
    ret = vplayer_stream_start(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
    ASSERT(ret==0, "[%s]: stream start failed, ret=%d", __func__, ret);

    hal_csi_start_calib(false, false);

    while (1) {
        osDelay(20);
        timeout--;
        if (stream_calib_status == VP_STREAM_CALIB_DONE) {
            break;
        }
        if (timeout == 0) {
            TRACE(1, "calib timeout");
            ret = 1;
            break;
        }
    }
    if (ret == 0) {
        TRACE(1, "[%s]: CALIB SUCESS", __func__);
    } else {
        TRACE(1, "[%s]: CALIB ERROR", __func__);
    }
    ret = vplayer_stream_stop(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
    ASSERT(ret==0, "[%s]: stream stop failed, ret=%d", __func__, ret);
    ret = vplayer_stream_close(VP_STREAM_ID_0, VP_STREAM_CAPTURE);
    ASSERT(ret==0, "[%s]: stream close failed, ret=%d", __func__, ret);

    if (need_open) {
        vplayer_close();
    }
    stream_calib_finish = true;
    stream_calib_status = VP_STREAM_CALIB_CLOSE;
    return ret;
}

void vplayer_stream_dump_cfg(const char *title, struct VP_STREAM_CONFIG_T *cfg)
{
    TRACE(1, "=============%s: STREAM_CFG[%x] DUMP=============", title, (int)cfg);
    TRACE(1, "width       =%d", cfg->width);
    TRACE(1, "height      =%d", cfg->height);
    TRACE(1, "fmt         =%d", cfg->fmt);
    TRACE(1, "channel_num =%d", cfg->channel_num);
    TRACE(1, "layer_id    =%d", cfg->layer_id);
    TRACE(1, "device      =%d", cfg->device);
    TRACE(1, "data_ptr    =%x", (int)cfg->data_ptr);
    TRACE(1, "data_size   =%x", cfg->data_size);
    TRACE(1, "DUMP_END=================================================");
}

void vplayer_device_dump_cfg(const char *title, struct VP_DEVICE_CONFIG_T *cfg)
{
    TRACE(1, "=============%s: DEVICE_CFG[%x] DUMP=============", title, (int)cfg);
    TRACE(1, "width       =%d", cfg->width);
    TRACE(1, "height      =%d", cfg->height);
    TRACE(1, "fmt         =%d", cfg->fmt);
    TRACE(1, "channel_num =%d", cfg->channel_num);
    TRACE(1, "op_flag     =0x%x", cfg->op_flag);
    TRACE(1, "DUMP_END=================================================");
}

