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

#if defined(TEST_BLEND) && defined(PSRAM_ENABLE)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_sleep.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_lcdc.h"
#include "hal_psc.h"
#include "hal_dsi.h"

#include "test_colors.h"
#include "lcdc_test.h"
#include "blend_utils.h"
#include "beco.h"

#define LCDC_SIZE   (480*480*4)

#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)

static uint8_t frame_buf[LCDC_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
static uint8_t test_buf[LCDC_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;

/* Picture RGB888 */
#define PIC_WIDTH   368
#define PIC_HEIGHT  448
#define PIC_STRIDE  (PIC_WIDTH*4)
static uint8_t test_pic[] = {
#include "resource/RGB888_unpacked_368x448.txt"
};

static int blend_test_draw(blend_mode_t blend)
{
    blend_buffer_t src, dst;
    blend_rect_t rect;
    blend_transform_t transform;

    blend_transform_init(&transform);

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = BLEND_COLOR_RGBA8888;
    src.data = test_pic;

    rect.x = 100;
    rect.y = 100;
    rect.w = 200;
    rect.h = 200;

    dst.width = 300;
    dst.height = 300;
    dst.format = BLEND_COLOR_RGBA8888;
    dst.data = test_buf;

    blend_transform_translate(&transform, 50, 50);

    blend_clear_beco(&dst, 0xff0000ff);
    blend_blit_beco(&dst, &src, &rect, &transform, blend, 125);

    src.width = 300;
    src.height = 300;
    src.format = BLEND_COLOR_RGBA8888;
    src.data = test_buf;

    dst.width = lcdc_test_get_width();
    dst.height = lcdc_test_get_height();;
    dst.format = BLEND_COLOR_RGBA8888;
    dst.data = frame_buf;

    blend_transform_translate(&transform, 70, 70);

    blend_clear_beco(&dst, 0xff000000);
    blend_blit_beco(&dst, &src, NULL, &transform, BLEND_NONE, 0);
    return 0;
}

static int blend_test_rotate(int rotate)
{
    blend_buffer_t src, dst;
    blend_rect_t rect;
    blend_transform_t transform;
    int target_x, target_y;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = BLEND_COLOR_RGBA8888;
    src.data = test_pic;

    rect.x = 50;
    rect.y = 100;
    rect.w = 300;
    rect.h = 300;

    dst.width = lcdc_test_get_width();
    dst.height = lcdc_test_get_height();;
    dst.format = BLEND_COLOR_RGBA8888;
    dst.data = frame_buf;

    target_x = (dst.width - rect.w) / 2;
    target_y = (dst.height - rect.h) / 2;

    blend_clear_beco(&dst, 0xff000000);

    blend_transform_init(&transform);
    blend_transform_translate(&transform, target_x, target_y);
    blend_transform_rotate(&transform, rotate);

    blend_blit_beco(&dst, &src, &rect, &transform, BLEND_NONE, 0);
    return 0;
}

static int blend_test_scale(float scale)
{
    blend_buffer_t src, dst;
    blend_rect_t rect;
    blend_transform_t transform;
    int target_x, target_y;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = BLEND_COLOR_RGBA8888;
    src.data = test_pic;

    rect.x = 100;
    rect.y = 100;
    rect.w = 200;
    rect.h = 200;

    dst.width = lcdc_test_get_width();
    dst.height = lcdc_test_get_height();;
    dst.format = BLEND_COLOR_RGBA8888;
    dst.data = frame_buf;

    target_x = (int)(dst.width - rect.w * scale) / 2;
    target_y = (int)(dst.height - rect.h * scale) / 2;

    blend_clear_beco(&dst, 0xff000000);

    blend_transform_init(&transform);
    blend_transform_translate(&transform, target_x, target_y);
    blend_transform_scale(&transform, scale, scale);

    blend_blit_beco(&dst, &src, &rect, &transform, BLEND_NONE, 0);
    return 0;
}


void test_blend(void)
{
    uint32_t start;

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    LCDC_INFO("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();
#if ENABLE_BECO
    BECO_INIT();
#endif

    while (1) {
        /* blending test */
        for (int m = 0; m < 10 ; m++) {
            LCDC_INFO("----------------------------------------------------------------------");
            start = hal_sys_timer_get();

            blend_test_draw(m%10);

            LCDC_INFO("#### Frame %u, blend mode %u, drawing used %ums",
                        m, m%10, TICKS_TO_MS(hal_sys_timer_get() - start));

            lcdc_test_frame_xfer();
            lcdc_test_frame_wait_done();

            osDelay(1000);
        }

        /* rotation test */
        for (int r = 0; r < 360; r++) {
            LCDC_INFO("----------------------------------------------------------------------");
            start = hal_sys_timer_get();

            blend_test_rotate(r);

            LCDC_INFO("#### Frame %u, rotate %u, drawing used %ums",
                        r, r, TICKS_TO_MS(hal_sys_timer_get() - start));

            lcdc_test_frame_xfer();
            lcdc_test_frame_wait_done();
        }

        /* scale test */
        int scale = 100; /* 0.5 ~ 2.0 */
        int step = 10;
        for (int i = 0; i < 360; i++) {
            LCDC_INFO("----------------------------------------------------------------------");
            start = hal_sys_timer_get();

            blend_test_scale(scale/100.0f);

            LCDC_INFO("#### Frame %u, scale %u%%, drawing used %ums",
                        i, scale, TICKS_TO_MS(hal_sys_timer_get() - start));

            lcdc_test_frame_xfer();
            lcdc_test_frame_wait_done();

            scale += step;
            if (scale > 200 || scale < 50)
                step = -step;
        }
    }

    lcdc_test_stop();
#if ENABLE_BECO
    BECO_EXIT(0);
#endif
    while (1);
}

#endif
