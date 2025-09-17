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
#ifdef DSI_TEST
#include "hal_dsi.h"
#include "hal_lcdc.h"
#endif
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "csi_host.h"
#include "csi_host_app.h"
#include "csi_fb.h"
#include "video_misc.h"
#include "camera_drv.h"

//#define CSI_DEBUG

//#define DSI_IRQ_DEBUG

//#define CSI_USE_GPIO_TRIG

#ifndef CSI_FPS_STAT_ENABLE
#define CSI_FPS_STAT_ENABLE
#endif

#ifndef DSI_FPS_STAT_ENABLE
//#define DSI_FPS_STAT_ENABLE
#endif

#ifdef CSI_FPS_STAT_SLOW
#define FPS_STAT_SLOW_ENABLE 1
#else
#define FPS_STAT_SLOW_ENABLE 0
#endif

#ifndef CSI_DATA_DUMP_SIZE
#define CSI_DATA_DUMP_SIZE 64
#endif

#define CSI_TEST_PATT_CHANGE_MODE 0

enum TEST_SIG_T {
    TEST_SIG_CSI_FRAME_DONE0 = 0x00000001,
    TEST_SIG_CSI_FRAME_DONE1 = 0x00000002,
    TEST_SIG_CSI_START       = 0x00000004,
    TEST_SIG_CSI_RESTART     = 0x00000008,
    TEST_SIG_CSI_STOP        = 0x00000010,
    TEST_SIG_CSI_KEY_ON_OFF  = 0x00000020,
    TEST_SIG_DSI_FRAME_DONE0 = 0x00000040,
    TEST_SIG_DSI_FRAME_DONE1 = 0x00000080,
    TEST_SIG_CSI_FRAME_DUMP  = 0x00000100,
};

#ifdef RTOS
#define CSI_THREAD_STACK_SIZE   0x4000
static osThreadId csi_thread_tid = NULL;
static void csi_thread(void const *argument);
osThreadDef(csi_thread, osPriorityAboveNormal, 1, CSI_THREAD_STACK_SIZE, "CSI_THD");

osMutexId csi_thread_mutex_id = NULL;
osMutexDef(csi_thread_mutex);

#else
static volatile int csi_thread_signal = 0;
static volatile int csi_flag_lock = 0;
void csi_thread(void const *argument);
#endif

static int g_cam_desc_id;
static enum VP_DEVICE_T g_cam_device;

//#define DSI_USE_TIMER_TRIG

#ifdef DSI_USE_TIMER_TRIG
#ifndef RTOS
#error "RTOS should be defined for DSI_USE_TIMER_TRIG"
#endif

static void dsi_timer_callback(void const *param);
osTimerDef(DSI_TIMER, dsi_timer_callback);
osTimerId dsi_timer_id = NULL;
static uint32_t dsi_timer_tick = 0;
static uint32_t dsi_timer_period_ms = 500;

static void dsi_timer_callback(void const *param)
{
    dsi_timer_tick++;

    TRACE(1, "[%s]: tick=%d", __func__, dsi_timer_tick);
    hal_lcdc_start_frame();
}

static void dsi_timer_start(uint32_t period_ms)
{
    TRACE(1, "[%s]: period_ms=%d", __func__, period_ms);

    if (dsi_timer_id == NULL) {
        dsi_timer_id = osTimerCreate(osTimer(DSI_TIMER), osTimerPeriodic, NULL);
    }
    if (dsi_timer_id) {
        dsi_timer_tick = 0;
        dsi_timer_period_ms = period_ms;
        osTimerStart(dsi_timer_id, period_ms);
    }
}

static void dsi_timer_stop(void)
{
    TRACE(1, "[%s]: ", __func__);

    if (dsi_timer_id) {
        osTimerStop(dsi_timer_id);
    }
}

static void dsi_timer_trigger_start(void)
{
    hal_lcdc_select_te_trigger(false);
    dsi_timer_start(dsi_timer_period_ms);
}

static void dsi_timer_trigger_stop(void)
{
    hal_lcdc_select_te_trigger(true);
    dsi_timer_stop();
}

#endif

void csi_lock_thread(void)
{
#ifdef RTOS
    osMutexWait(csi_thread_mutex_id, osWaitForever);
#else
    ASSERT(csi_flag_lock == 0, "[%s]: csi thread has been locked", __func__);
    csi_flag_lock = 1;
#endif
}

void csi_unlock_thread(void)
{
#ifdef RTOS
    osMutexRelease(csi_thread_mutex_id);
#else
    ASSERT(csi_flag_lock == 1, "[%s]: csi thread has been unlocked", __func__);
    csi_flag_lock = 0;
#endif
}

static POSSIBLY_UNUSED uint32_t csi_int_cnt = 0;
static POSSIBLY_UNUSED uint32_t dsi_int_cnt = 0;
static POSSIBLY_UNUSED int csi_host_key_flag = 1;
static POSSIBLY_UNUSED bool csi_host_status = false;

volatile static POSSIBLY_UNUSED bool dsi_data_flush = false;
static POSSIBLY_UNUSED bool dsi_data_update = false;
static POSSIBLY_UNUSED uint32_t dsi_cur_rpos[2] = {0};

#ifdef CSI_NO_TRACE
static bool csi_trace_status = false;
#else
static bool csi_trace_status = true;
#endif

static int csi_poll_reg_on = 0;
static uint32_t csi_poll_reg_addr = 0;
static uint32_t csi_poll_reg_val = 0;

static struct HAL_CSI_CONFIG_T g_csi_cfg;

static CSI_HOST_IRQ_NOTIFY_T csi_irq_notify = NULL;
static CSI_HOST_IRQ_NOTIFY_T dsi_irq_notify = NULL;

#ifdef CSI_USE_GPIO_TRIG
static int gpio_trig_pp_index = 0;
static enum GPIO_TRIG_STATE_T gpio_trig_state;
#endif

static uint32_t g_csi_errcode = 0;

//57: blue, 56: green, 55: red, 54: white
#define PATT_INIT_COLOR 0x56

struct CSI_TEST_PATT_CHECK_INFO_T {
    uint32_t enable   : 1;
    uint32_t mode_chg : 1;
    uint32_t rsv0     : 6;
    uint32_t skip_cnt : 8;
    uint32_t rsv1     : 16;
    uint32_t addr;
    uint32_t len;
    uint32_t err_code;
    uint32_t err_cnt;
    uint32_t chk_cnt;
    uint32_t mode_cnt;
    uint32_t mode_idx;
    uint32_t mode_val[4];
};

static struct CSI_TEST_PATT_CHECK_INFO_T chk_info;

static int csi_host_test_patt_check(uint32_t addr, uint32_t len,  uint32_t errcode);

void csi_host_setup_csi_irq_notify(CSI_HOST_IRQ_NOTIFY_T notify)
{
    csi_irq_notify = notify;
}

void csi_host_setup_dsi_irq_notify(CSI_HOST_IRQ_NOTIFY_T notify)
{
    dsi_irq_notify = notify;
}

static POSSIBLY_UNUSED void csi_signal_set(int signal)
{
#ifdef RTOS
    osSignalSet(csi_thread_tid, signal);
#else
    uint32_t lock = int_lock();
    csi_thread_signal |= signal;
    int_unlock(lock);
#endif
}

static POSSIBLY_UNUSED void csi_signal_clr(void)
{
#ifndef RTOS
    csi_thread_signal = 0;
#endif
}

static POSSIBLY_UNUSED int csi_signal_get(void)
{
#ifdef RTOS
    return 0;
#else
    return csi_thread_signal;
#endif
}

static POSSIBLY_UNUSED void csi_signal_dump(int32_t signal)
{
    if (signal & TEST_SIG_CSI_FRAME_DONE0) {
        TRACE(0, "[TEST_SIG_CSI_FRAME_DONE0]");
    }
    if (signal & TEST_SIG_CSI_FRAME_DONE1) {
        TRACE(0, "[TEST_SIG_CSI_FRAME_DONE1]");
    }
    if (signal & TEST_SIG_CSI_RESTART) {
        TRACE(0, "[TEST_SIG_CSI_RESTART]");
    }
    if (signal & TEST_SIG_CSI_KEY_ON_OFF) {
        TRACE(0, "[TEST_SIG_CSI_KEY_ON_OFF]");
    }
    if (signal & TEST_SIG_DSI_FRAME_DONE0) {
        TRACE(0, "[TEST_SIG_DSI_FRAME_DONE0]");
    }
    if (signal & TEST_SIG_DSI_FRAME_DONE1) {
        TRACE(0, "[TEST_SIG_DSI_FRAME_DONE1]");
    }
}

bool csi_host_status_get(void)
{
    return csi_host_status;
}

void csi_host_status_set(bool status)
{
    csi_host_status = status;
}

#ifdef DSI_TEST
static void dsi_xfer_data_done_cb(uint8_t layer_id, uint8_t channel, uint32_t addr)
{
    dsi_int_cnt++;

#ifdef DSI_IRQ_DEBUG
    TRACE(0, "[%8u][DSI_IRQ]: layer_id=%d, channel=%d, addr=%x", dsi_int_cnt, layer_id, channel, addr);
#endif

    if (dsi_irq_notify) {
        dsi_irq_notify(channel);
    }

#ifdef DSI_FPS_STAT_ENABLE
    if (channel == 0) {
        video_fps_stat_calc(VIDEO_FPS_STAT_ID_DSI, 0);
    }
#endif

    if (csi_fb_flush_status()) {
        dsi_fb_callback(layer_id, channel, addr);
        if (channel == 0) {
            csi_signal_set(TEST_SIG_DSI_FRAME_DONE0);
        } else if (channel == 1) {
            csi_signal_set(TEST_SIG_DSI_FRAME_DONE1);
        }
    }
}
#endif /* DSI_TEST */

static POSSIBLY_UNUSED uint32_t csi_dma_irq_dummy_callback(uint8_t *buf, uint32_t len, uint32_t errcode)
{
    g_csi_errcode = errcode;
    return 0;
}

static uint32_t csi_dma_irq_callback(uint8_t *buf, uint32_t len, uint32_t errcode)
{
    int signal;
    int done;

    csi_int_cnt++;

    if (csi_trace_status) {
        TRACE(0, "[CSI_IRQ][%8u]: buf=0x%x, len=0x%x, errcode=0x%x", csi_int_cnt, (int)buf, len, errcode);
    }

    if (csi_poll_reg_on) {
        csi_poll_reg_val = *(volatile uint32_t *)csi_poll_reg_addr;
        TRACE(0, "REG[%x]=%x", csi_poll_reg_addr, csi_poll_reg_val);
    }

    if (csi_irq_notify) {
        csi_irq_notify(errcode);
    }

#ifdef CSI_FPS_STAT_ENABLE
    video_fps_stat_calc(VIDEO_FPS_STAT_ID_CSI, errcode);
#endif

    done = csi_fb_callback(buf, len, errcode);
    if (done) {
        if (buf == g_csi_cfg.data_ptr) {
            signal = TEST_SIG_CSI_FRAME_DONE0;
        } else {
            signal = TEST_SIG_CSI_FRAME_DONE1;
        }
        //signal |= TEST_SIG_CSI_FRAME_DUMP;
        csi_signal_set(signal);
    }
    return 0;
}

#ifdef CSI_USE_GPIO_TRIG
static void gpio_trigger_csi_callback(int pin, enum GPIO_TRIG_STATE_T state)
{
    struct HAL_CSI_CONFIG_T *cfg = &g_csi_cfg;

    HAL_CSI_STREAM_HANDLER_T handler = cfg->handler;

    gpio_trig_state = state;

    if (state == TRIG_STATE_SYNC_BEGIN) {
        return;
    }
    if (handler) {
        uint8_t *buf;
        uint32_t len;
        int pp_index;
        uint32_t errcode;
        uint32_t lock;

        errcode = g_csi_errcode;

        lock = int_lock();

        pp_index = gpio_trig_pp_index;
        len = cfg->data_size / 2;
        if (pp_index == 0) {
            buf = (uint8_t *)(cfg->data_ptr);
        } else {
            buf = (uint8_t *)(cfg->data_ptr + cfg->data_size / 2);
        }
        if (cfg->pp_mode) {
            gpio_trig_pp_index ^= 1;
        }

        int_unlock(lock);

        if (handler) {
            handler(buf, len, errcode);
        }
    }
}
#endif

static int csi_thread_stream_handler(int stream, int chan)
{
#define CSI_BSIZE 8
    int ret = 0;
    struct CSI_DSI_FB_CNTL_T *csi_fb;
    struct CSI_DSI_FB_CNTL_T *dsi_fb;
    uint32_t lock = 0;
    uint32_t time = hal_sys_timer_get();

    csi_lock_thread();

    if (stream == 0) {
        // DSI STREAM
        csi_fb = csi_fb_get();
        dsi_fb = dsi_fb_get(chan);

        if (!csi_fb->enable) {
            ret = 1;
            goto _end;
        }
        if (!csi_fb->flush) {
            ret = 2;
            goto _end;
        }
#ifdef DSI_TEST
        if (!dsi_fb->enable) {
            ret = 3;
            goto _end;
        }
        if (!dsi_fb->flush) {
            ret = 4;
            goto _end;
        }
#endif
        lock = int_lock();
        csi_fb->lock = 1;
        dsi_fb->lock = 1;
        int_unlock(lock);

#ifdef DSI_TEST
        uint32_t cnt = csi_fb->cnt;

        if (dsi_fb->updata) {
            uint32_t *src     = (uint32_t *)(csi_fb->addr);
            uint32_t wcnt = csi_fb->len / 4;
            uint32_t *dst = (uint32_t *)dsi_fb->addr;
            volatile uint32_t i;
            if (csi_trace_status) {
                TRACE(1, "[DSI_STREAM][UPDATA][%d]: s=%x, d=%x, l=%d",cnt, (int)src, (int)dst, wcnt);
            }
            i = wcnt / CSI_BSIZE;
            while (i > 0) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                dst[4] = src[4];
                dst[5] = src[5];
                dst[6] = src[6];
                dst[7] = src[7];
                dst += CSI_BSIZE;
                src += CSI_BSIZE;
                i--;
            }
            i = wcnt % CSI_BSIZE;
            while (i > 0) {
                *dst++ = *src++;
                i--;
            }
        }
        else if (dsi_fb->upaddr) {
            uint8_t layer = 0;
            uint32_t addr = csi_fb->addr;

            lock = int_lock();
            hal_lcdc_update_addr(layer, 0, addr);
            hal_lcdc_update_addr(layer, 1, addr);
            int_unlock(lock);
            if (csi_trace_status) {
                TRACE(1, "[DSI_STREAM][UPADDR][%d]: layer=%d, chan=%d, addr=%x", cnt, layer, chan, addr);
            }
        }
#endif

        lock = int_lock();
        csi_fb->lock = 0;
        dsi_fb->lock = 0;
        if (csi_fb->flush) {
            csi_fb->flush = 0;
        }
        if (dsi_fb->flush) {
            dsi_fb->flush = 0;
        }
        int_unlock(lock);
    } else {
        // CSI STREAM
#if !defined(DSI_TEST)
        csi_fb = csi_fb_get();
        uint32_t i;
        uint32_t *pd = (uint32_t *)csi_fb->addr;

        // dump some data from CSI fb buffer
        hal_trace_flush_output();
        TRACE(0, "CSI_BUFF_DATA:");
        for (i = 0; i < CSI_DATA_DUMP_SIZE; i += 8) {
            TRACE(0, "%08X: %08X %08X %08X %08X %08X %08X %08X %08X",
            (int)pd, pd[0], pd[1], pd[2], pd[3], pd[4], pd[5], pd[6], pd[7]);
            pd += 8;
        }
        TRACE(0, "END");
        hal_trace_flush_output();

        lock = int_lock();
        if (csi_fb->flush) {
            csi_fb->flush = 0;
        }
        int_unlock(lock);
        if (csi_trace_status) {
            TRACE(0, "[CSI_STREAM]: clear flush status");
        }
#endif
        if (chk_info.enable) {
            csi_fb = csi_fb_get();
            csi_host_test_patt_check(csi_fb->addr, csi_fb->len, csi_fb->err);
        }
    }
_end:
    time = hal_sys_timer_get() - time;
    csi_unlock_thread();
    if (!ret) {
        if (csi_trace_status) {
            TRACE(0, "[STREAM_HDLR][DONE]: cost %d us", TICKS_TO_US(time));
        }
    } else {
//        TRACE(0, "[STREAM_HDLR][ERR]: ret=%d", ret);
    }
    return ret;
}

static void csi_test_patt_init(void)
{
    uint32_t lock;
    struct CSI_TEST_PATT_CHECK_INFO_T *p = &chk_info;

    TRACE(1, "[%s]:", __func__);

    lock = int_lock();

    p->skip_cnt = 2;
    p->addr = 0;
    p->len = 0;
    p->err_code = 0;
    p->err_cnt = 0;
    p->chk_cnt = 0;
    p->mode_cnt = 0;
    p->mode_idx = 0;
    p->mode_val[0] = PATT_INIT_COLOR;
    p->mode_val[1] = 0x55;
    p->mode_val[2] = 0x54;
    p->mode_val[3] = 0x57;
    p->mode_chg = CSI_TEST_PATT_CHANGE_MODE;
    p->enable = 0;

    int_unlock(lock);
}

static void csi_test_patt_enable(void)
{
    struct CSI_TEST_PATT_CHECK_INFO_T *p = &chk_info;

    p->enable = 1;
    TRACE(1, "[%s]: enable=%d", __func__, p->enable);
}

static void csi_test_patt_disable(void)
{
    struct CSI_TEST_PATT_CHECK_INFO_T *p = &chk_info;

    p->enable = 0;
    TRACE(1, "[%s]: enable=%d", __func__, p->enable);
}

static int csi_host_test_patt_check(uint32_t addr, uint32_t len,  uint32_t errcode)
{
    volatile int err = 0;

    uint32_t *pd = (uint32_t *)addr;
    uint32_t num = len / sizeof(uint32_t);
    uint32_t i;
    uint32_t patt = pd[1];
    uint32_t time;

    struct CSI_TEST_PATT_CHECK_INFO_T *p = &chk_info;

    if (p->skip_cnt > 0) {
        TRACE(1, "[%s]: skip cnt %d", __func__, p->skip_cnt);
        p->skip_cnt--;
        return 0;
    }
    if (!p->enable) {
        TRACE(1, "[%s]: not enabled", __func__);
        return 0;
    }
    p->addr = addr;
    p->len  = len;
    p->err_code = errcode;
    p->chk_cnt++;

    // check color value
    time = hal_sys_timer_get();
    patt = pd[0];
    for (i = 1; i < num; i++) {
        if (pd[i] != patt) {
            err++;
            TRACE(1, "[%4u]: err data [%x]=%x, should be %x", err, (int)&pd[i], pd[i], patt);
        }
        if (err >= 10) {
            break;
        }
    }
    time = TICKS_TO_MS(hal_sys_timer_get() - time);
    if (err == 0) {
        TRACE(1, "[PATT_DATA_OKAY]: %d / %d, errcode=0x%x, %d ms",
            p->err_cnt, p->chk_cnt, p->err_code, time);
    } else {
        p->err_cnt++;
        TRACE(1, "[PATT_DATA_ERR ]: %d / %d, addr=0x%x, len=0x%x, errcode=0x%x, %d ms",
            p->err_cnt, p->chk_cnt, addr, len, p->err_code, time);
    }
    //switch new color
    if (p->mode_chg) {
        p->mode_cnt++;
        if (p->mode_cnt >= 200) {
            p->mode_cnt = 0;
            p->skip_cnt = 2;
            p->mode_idx++;
            if (p->mode_idx >= ARRAY_SIZE(p->mode_val)) {
                p->mode_idx = 0;
            }
            camera_device_test_mode(g_cam_desc_id, true, p->mode_val[p->mode_idx], 0, 0);
        }
    }
    return err;
}

int csi_host_cmd_test_patt_enable(bool enable)
{
    TRACE(1, "[%s]: enable=%d", __func__, enable);

    if (enable) {
        csi_test_patt_init();
        camera_device_test_mode(g_cam_desc_id, enable, 0, 0, 0);
        camera_device_test_mode(g_cam_desc_id, enable, PATT_INIT_COLOR, 0, 0);
        csi_test_patt_enable();
    } else {
        csi_test_patt_disable();
    }
    return 0;
}

int csi_host_cmd_set_page(uint32_t page)
{
    camera_device_bus_write(g_cam_desc_id, 0xFE, page);
    TRACE(1, "SET PAGE DONE, I2C WR [0xFE]=%x", page);
    return 0;
}

int csi_host_cmd_access_mem(int wr, uint32_t arg0, uint32_t arg1)
{
    if (wr) {
        uint32_t addr = arg0, val = arg1;
        TRACE(1, "WRITE  MEM[%X] = %X", addr, val);
        *(volatile uint32_t *)addr = val;
    } else {
        uint32_t addr = arg0, n = 1, i, val;
        if (arg1 > 0) {
            n = arg1;
        }
        for (i = 0; i < n; i++) {
            val = *(volatile uint32_t *)addr;
            TRACE(1, "READ MEM[%X] = %X", addr, val);
            addr += 4;
        }
    }
    return 0;
}

int csi_host_cmd_poll_reg(int on, uint32_t addr)
{
    csi_poll_reg_on = on;
    csi_poll_reg_addr = addr;
    csi_poll_reg_val = 0;
    return 0;
}

int csi_host_cmd_dump_data(uint32_t bytes, uint32_t bits)
{
    uint8_t *s = NULL;
    uint32_t i, len = 0;
    uint32_t dsize = 2;

    if (bits > 0) {
        if (bits == 16) {
            dsize = 2;
        } else if (bits == 8) {
            dsize = 1;
        } else {
            dsize = 4;
        }
    }
    if (bytes == 0) {
        TRACE(1, "zero len");
        return 0;
    }
    if (!csi_host_status) {
        TRACE(1, "CSI is stopped");
        return 0;
    }
    hal_csi_stop();
    csi_fb_disable();
    if (bytes < len) {
        len = bytes;
    }
    s = (uint8_t *)csi_fb_get_addr(&len);
    TRACE(1, "CSI DATA DUMP: addr=0x%x, len=0x%x", (int)s, len);
    if (dsize == 2) {
        int16_t *p = (int16_t *)s;
        uint32_t dlen = len / 2;
        for (i = 0; i < dlen; i += 8) {
            TRACE(1, "%04X %04X %04X %04X %04X %04X %04X %04X",
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            p += 8;
            hal_trace_flush_output();
        }
    } else if (dsize == 1) {
        int8_t *p = (int8_t *)s;
        uint32_t dlen = len / 1;
        for (i = 0; i < dlen; i += 8) {
            TRACE(1, "%02X %02X %02X %02X %02X %02X %02X %02X",
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            p += 8;
            hal_trace_flush_output();
        }
    } else {
        int32_t *p = (int32_t *)s;
        uint32_t dlen = len / 4;
        for (i = 0; i < dlen; i += 8) {
            TRACE(1, "%08X %08X %08X %08X %08X %08X %08X %08X",
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            p += 8;
            hal_trace_flush_output();
        }
    }
    TRACE(1, "CSI DUMP DONE");

    csi_fb_enable();
    hal_csi_start();
    return 0;
}

#ifdef RTOS
static void csi_thread(void const *argument)
{
    osEvent evt;

    TRACE(2,"%s %d enter", __func__, __LINE__);

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
#ifdef CSI_DEBUG
        if (csi_trace_status) {
            TRACE(2, "[%s] signals=0x%x cnt=%d", __func__, evt.value.signals, csi_int_cnt);
            csi_signal_dump(evt.value.signals);
        }
#endif
        if (evt.status == osEventSignal) {
            if (evt.value.signals & TEST_SIG_DSI_FRAME_DONE0) {
                csi_thread_stream_handler(0, 0);
            }
            if (evt.value.signals & TEST_SIG_DSI_FRAME_DONE1) {
                csi_thread_stream_handler(0, 1);
            }
            if (evt.value.signals & TEST_SIG_CSI_FRAME_DONE0) {
                csi_thread_stream_handler(1, 0);
            }
            if (evt.value.signals & TEST_SIG_CSI_FRAME_DONE1) {
                csi_thread_stream_handler(1, 1);
            }
            if (evt.value.signals & TEST_SIG_CSI_RESTART) {
                csi_host_app_restart();
            }
            if (evt.value.signals & TEST_SIG_CSI_KEY_ON_OFF) {
                if (csi_host_key_flag) {
                    csi_host_app_on(true);
                } else {
                    csi_host_app_on(false);
                }
            }
            if (evt.value.signals & TEST_SIG_CSI_FRAME_DUMP) {
                csi_host_cmd_dump_data(g_csi_cfg.width * g_csi_cfg.height * 2, 32);
            }
        } else {
            TRACE(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
        }
    }
}

#else // !RTOS
void csi_thread(void const *arguments)
{
    int signal = 0;

    signal = csi_signal_get();
    if (signal == 0) {
        return;
    }
    if (signal != 0) {
        csi_signal_clr();
#ifdef CSI_DEBUG
        TRACE(2, "[%s] signals=0x%x cnt=%d", __func__, signal, csi_int_cnt);
        csi_signal_dump(signal);
#endif
        if (signal & TEST_SIG_DSI_FRAME_DONE0) {
            csi_thread_stream_handler(0, 0);
        }
        if (signal & TEST_SIG_DSI_FRAME_DONE1) {
            csi_thread_stream_handler(0, 1);
        }
        if (signal & TEST_SIG_CSI_FRAME_DONE0) {
            csi_thread_stream_handler(1, 0);
        }
        if (signal & TEST_SIG_CSI_FRAME_DONE1) {
            csi_thread_stream_handler(1, 1);
        }
        if (signal & TEST_SIG_CSI_RESTART) {
        }
        if (signal & TEST_SIG_CSI_KEY_ON_OFF) {
            if (csi_host_key_flag) {
                csi_host_app_on(true);
            } else {
                csi_host_app_on(false);
            }
        }
    }
}
#endif

int csi_key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(0, "[%s]: key_code=%x, key_event=%x", __func__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            csi_host_key_flag ^= 1;
            csi_signal_set(TEST_SIG_CSI_KEY_ON_OFF);
        }
    }
    return 0;
}

int csi_host_open(void)
{
    struct HAL_CSI_CONFIG_T *csi_cfg = &g_csi_cfg;
    TRACE(0, "[%s]", __func__);

#ifdef RTOS
    if (csi_thread_mutex_id == NULL) {
        csi_thread_mutex_id = osMutexCreate(osMutex(csi_thread_mutex));
    }
    if (csi_thread_tid == NULL) {
        csi_thread_tid = osThreadCreate(osThread(csi_thread), NULL);
        if (csi_thread_tid == NULL) {
            TRACE(0, "[%s] ERROR: create csi task fail@@", __func__);
            return -1;
        }
        TRACE(0, "[%s] SUCCESS: create csi task fd:%p", __func__, csi_thread_tid);
    }
#endif
#ifdef DSI_TEST
    hal_lcdc_set_callback(dsi_xfer_data_done_cb);
#endif

    memset((void *)csi_cfg, 0, sizeof(struct HAL_CSI_CONFIG_T));

    uint8_t *buf;
    uint32_t len, pixel_size = 2;

    buf = (uint8_t *)CSI_DATA_BUF_ADDR;
    len = CSI_IMG_PIXEL_WIDTH * CSI_IMG_PIXEL_HEIGHT * pixel_size * 2;
    TRACE(0, "CSI BUFFER: addr=%x, len=%d", (int)buf, len);

    csi_cfg->fmt = CSI_PIX_FMT_YUV422;
    csi_cfg->data_lane_num = CSI_DATA_LANE_NUM_2;
    csi_cfg->width = CSI_IMG_PIXEL_WIDTH;
    csi_cfg->height = CSI_IMG_PIXEL_HEIGHT;
    csi_cfg->pp_mode = 1;
    csi_cfg->data_ptr = buf;
    csi_cfg->data_size = len;
#if defined(CSI_USE_GPIO_TRIG)
    csi_cfg->handler = csi_dma_irq_dummy_callback;
#else
    csi_cfg->handler = csi_dma_irq_callback;
#endif
    hal_csi_open(csi_cfg);

#if defined(CSI_USE_GPIO_TRIG)
    struct GPIO_TRIG_CONFIG_T trig_cfg;

    csi_cfg->handler = csi_dma_irq_callback;
    gpio_trig_pp_index = 0;
    trig_cfg.callback = gpio_trigger_csi_callback;
    gpio_trigger_csi_open(&trig_cfg);
#endif

    return 0;
}

void csi_host_setup_param(int desc_id, int device)
{
    g_cam_desc_id = desc_id;
    g_cam_device = (enum VP_DEVICE_T)device;
}

int csi_host_start(void)
{
    TRACE(0, "[%s]", __func__);
    hal_csi_start();
    return 0;
}

int csi_host_calib_start(void)
{
    hal_csi_start_calib(false, false);
    return 0;
}

int csi_host_stop(void)
{
    TRACE(0, "[%s]", __func__);

    hal_csi_stop();
    return 0;
}

int csi_host_close(void)
{
    TRACE(0, "[%s]", __func__);
    hal_csi_close();
    osDelay(10);

#ifdef CSI_USE_GPIO_TRIG
    gpio_trigger_csi_close();
#endif

    return 0;
}

void csi_host_state_notify(int state)
{
    static uint32_t time = 0;

    TRACE(0, "[%s]: state=%d", __func__, state);
    switch (state) {
    case 0:
        time = hal_sys_timer_get();

        csi_fb_init();
        dsi_fb_init(DSI_FB_CH0);
        dsi_fb_init(DSI_FB_CH1);
#ifdef CSI_FPS_STAT_ENABLE
        video_fps_stat_open(VIDEO_FPS_STAT_ID_CSI);
#endif
#ifdef DSI_FPS_STAT_ENABLE
        video_fps_stat_open(VIDEO_FPS_STAT_ID_DSI);
#endif
        break;
    case 1:
        csi_fb_enable();
#ifdef CSI_USE_GPIO_TRIG
        gpio_trigger_csi_enable(true);
#endif
        dsi_fb_enable(DSI_FB_CH0);
        dsi_fb_enable(DSI_FB_CH1);
#ifdef CSI_FPS_STAT_ENABLE
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_CSI, true);
#endif
#ifdef DSI_FPS_STAT_ENABLE
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_DSI, true);
#endif

#ifdef CAMERA_TEST_MODE_ENABLE
        csi_host_cmd_test_patt_enable(true);
#endif

#ifdef DSI_USE_TIMER_TRIG
        dsi_timer_trigger_start();
#endif

        time = hal_sys_timer_get() - time;
        TRACE(1, "[%s]: state[1] cost time: %d ms", __func__, TICKS_TO_MS(time));
        break;
    case 2:
        time = hal_sys_timer_get();

        dsi_fb_disable(DSI_FB_CH0);
        dsi_fb_disable(DSI_FB_CH1);
        csi_fb_disable();
#ifdef CSI_USE_GPIO_TRIG
        gpio_trigger_csi_enable(false);
#endif
#ifdef CSI_FPS_STAT_ENABLE
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_CSI, false);
        video_fps_stat_close(VIDEO_FPS_STAT_ID_CSI);
#endif
#ifdef DSI_FPS_STAT_ENABLE
        video_fps_stat_enable(VIDEO_FPS_STAT_ID_DSI, false);
        video_fps_stat_close(VIDEO_FPS_STAT_ID_DSI);
#endif

#ifdef CAMERA_TEST_MODE_ENABLE
        csi_host_cmd_test_patt_enable(false);
#endif

#ifdef DSI_USE_TIMER_TRIG
        dsi_timer_trigger_stop();
#endif

        break;
    case 3:
        time = hal_sys_timer_get() - time;
        TRACE(1, "[%s]: state[3] cost time: %d ms", __func__, TICKS_TO_MS(time));
        break;
    default:
        break;
    }
}

int csi_host_cmd_trace_quiet(bool quiet)
{
    TRACE(0, "[%s]: quiet=%d", __func__, quiet);
    if (quiet) {
        csi_trace_status = false;
    } else {
        csi_trace_status = true;
    }
    return 0;
}

void csi_debug_cfg(void)
{
    uint32_t val;
    val = *(volatile uint32_t *)0x400000f4;
    if (val) {
        if (val&0xff) {
            camera_device_bus_write(g_cam_desc_id, 0x29, val&0xff);  // HS-Prepare   ZT
        }
        val = val >> 8;
        if (val&0xff) {
            camera_device_bus_write(g_cam_desc_id, 0x2a, val&0xff);  // hs-zero   ZT
        }
        val = val >> 8;
        if (val&0xff) {
            camera_device_bus_write(g_cam_desc_id, 0x02, val&0xff);  // [6:4] lane0-diff  ZT
        }
        val = val >> 8;
        if (val&0xff) {
            camera_device_bus_write(g_cam_desc_id, 0x03, val&0xff);  // [6:4] lane0-diff  ZT
        }
        val = val >> 8;
        *(volatile uint32_t *)0x400000f4 = 0;
    }
}

