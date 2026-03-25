/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#include <string.h>
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cache.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "lvgl.h"
#include "lvgl_private.h"
#include "vg_lite.h"

#include "disp_conf.h"

#define FLUSH_TIME_OUT      60 /* ms */
#define USE_THREE_BUFFER    0 /// 使用三个缓冲区，0：使用两个缓冲区，1：使用三个缓冲区

#ifdef RTOS
static osSemaphoreId_t flush_sem = NULL;
#endif

static void disp_release_cb(lv_event_t *event);
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p);
static void disp_flush_wait_cb(lv_display_t *disp);
static void lcdc_flush_done(int error);

static lv_display_t *disp_drv = NULL;

#ifdef CONFIG_LCDC_COMPRESS
#if (CONFIG_LCDC_COMPRESS == 2)
static PSRAM_BSS_LOC uint8_t lvgl_buffer0[LCDC_SIZE*2/5] ALIGNED(64);
static PSRAM_BSS_LOC uint8_t lvgl_buffer1[LCDC_SIZE*2/5] ALIGNED(64);
#else
static PSRAM_BSS_LOC uint8_t lvgl_buffer0[LCDC_SIZE/2] ALIGNED(64);
static PSRAM_BSS_LOC uint8_t lvgl_buffer1[LCDC_SIZE/2] ALIGNED(64);
#endif
#define DISP_RENDER_MODE    LV_DISPLAY_RENDER_MODE_FULL
#else
#if USE_THREE_BUFFER
static uint8_t lvgl_render_buffer[LCDC_SIZE] ALIGNED(64);
#endif
static PSRAM_BSS_LOC uint8_t lvgl_buffer0[LCDC_SIZE] ALIGNED(64);
static PSRAM_BSS_LOC uint8_t lvgl_buffer1[LCDC_SIZE] ALIGNED(64);
#define DISP_RENDER_MODE    LV_DISPLAY_RENDER_MODE_DIRECT
#endif

static const void *lcdc_addr(void *buf)
{
    uint32_t addr = (uint32_t)buf;
#if defined(PSRAM_BASE) && defined(PSRAM_SIZE) && defined(PSRAM_NC_BASE)
    if (addr >= PSRAM_BASE && addr < (PSRAM_BASE+PSRAM_SIZE))
        return (void *)(addr - PSRAM_BASE + PSRAM_NC_BASE);
#endif
#ifdef ADDR_CPU_TO_DEV
    return (void *)ADDR_CPU_TO_DEV(addr);
#else
    return (void *)addr;
#endif
}

#if LV_USE_DRAW_VG_LITE && LV_VG_LITE_USE_GPU_INIT
void gpu_init(void)
{
    vg_lite_init(LCDC_WIDTH, LCDC_HEIGHT);
}
#endif

extern int disp_lcdc_init(void *buf, void (*flush_cb)(int));
extern int disp_lcdc_exit(void);
extern void disp_lcdc_send_frame(const void *buf);
extern void lv_besjpeg_init(void);
extern void disp_set_buffer_address(void* buffer1, void* buffer2);

int lv_hal_disp_init(void)
{
#if LV_USE_DRAW_BES_VGLITE
    vg_lite_init(LCDC_WIDTH, LCDC_HEIGHT);
#endif

    /* Initialize framebuffers */
    memset(lvgl_buffer0, 0, sizeof(lvgl_buffer0));
    memset(lvgl_buffer1, 0, sizeof(lvgl_buffer1));

    disp_drv = lv_display_create(LCDC_WIDTH, LCDC_HEIGHT);
    lv_display_set_driver_data(disp_drv, NULL);
    lv_display_add_event_cb(disp_drv, disp_release_cb, LV_EVENT_DELETE, disp_drv);
    lv_display_set_flush_cb(disp_drv, disp_flush_cb);
    lv_display_set_flush_wait_cb(disp_drv, disp_flush_wait_cb);
#if USE_THREE_BUFFER
    lv_display_set_buffers(disp_drv, lvgl_render_buffer, NULL, LCDC_SIZE, DISP_RENDER_MODE);
    disp_set_buffer_address(lvgl_buffer0, lvgl_buffer1);
#else
    lv_display_set_buffers(disp_drv, lvgl_buffer0, lvgl_buffer1, LCDC_SIZE, DISP_RENDER_MODE);
#endif
#ifdef RTOS
    flush_sem = osSemaphoreNew(1, 0, NULL);
#endif

    /* Initialize LCDC/DSI/QSPI and LCD panel */
    if(disp_lcdc_init((void *)lcdc_addr(lvgl_buffer0), lcdc_flush_done) != 0)
        ASSERT(0, "Failed to initialize LCDC.");
    return 0;
}

void lv_hal_disp_exit(void)
{
    disp_lcdc_exit();

#if LV_USE_DRAW_BES_VGLITE
    vg_lite_close();
#endif
}

extern int buffer_copy(int w, int h, void *src, void *dst);
extern void* disp_get_buffer_address(void);
static void disp_release_cb(lv_event_t *event)
{
    lv_hal_disp_exit();
}

static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p)
{
    if (!lv_display_flush_is_last(disp)) {
        lv_display_flush_ready(disp);
        return;
    }

    hal_cache_sync_invalidate_all(HAL_CACHE_ID_D_CACHE);
#if USE_THREE_BUFFER
    void* disp_buffer_address = NULL;
    while(!(disp_buffer_address = disp_get_buffer_address()));
    uint8_t status = buffer_copy(LCDC_WIDTH, LCDC_HEIGHT, color_p, disp_buffer_address);
    if(!status)
    {
       disp_lcdc_send_frame(disp_buffer_address);
    } else {
        LV_LOG_ERROR("buffer copy fauiler");
    }
#else
    disp_lcdc_send_frame(lcdc_addr(color_p));
#endif

#ifdef RTOS
    /* reset semaphore */
    if (flush_sem && osSemaphoreGetCount(flush_sem) == 1)
        osSemaphoreAcquire(flush_sem, -1);
#endif
}

static void disp_flush_wait_cb(lv_display_t *disp)
{
    if (!disp->flushing)
        return;
#ifdef RTOS
    if (flush_sem) {
        osSemaphoreAcquire(flush_sem, FLUSH_TIME_OUT);
    } else {
        uint32_t start = hal_sys_timer_get();
        while (disp->flushing &&
            TICKS_TO_MS(hal_sys_timer_get() - start) < FLUSH_TIME_OUT) {
            osDelay(1);
        }
    }
#else
    uint32_t start = hal_sys_timer_get();
    while (disp->flushing &&
           TICKS_TO_MS(hal_sys_timer_get() - start) < FLUSH_TIME_OUT) {
        hal_sys_timer_delay(MS_TO_TICKS(1));
    }
#endif
}

static void lcdc_flush_done(int error)
{
#ifdef RTOS
    if (flush_sem)
        osSemaphoreRelease(flush_sem);
#endif
    lv_display_flush_ready(disp_drv);
}
