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

#if defined(TEST_LCDC) && defined(PSRAM_ENABLE)

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

#include "display_test.h"

enum {
    TEST_WITH_ONE_BUFFER = 0,  /* CPU draw with one buffer and one layer */
    TEST_WITH_DOUBLE_BUFFER,   /* CPU draw with double buffer and one layer */
    TEST_WITH_GPU_DRAW,        /* GPU draw with double buffer and one layer, 60fps test */
    TEST_WITH_TWO_LAYERS,      /* LCDC two layers alpha test */
};
static int test_mode = TEST_WITH_GPU_DRAW;

#define LCDC_SIZE   (480*480*4)

#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)

/* LCDC layer buffer */
static uint8_t frame_buf0[LCDC_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
static uint8_t frame_buf1[LCDC_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;

static uint32_t lcdc_width = 0;
static uint32_t lcdc_height = 0;
static uint8_t  lcdc_format = FB_FMT_RGB32;

POSSIBLY_UNUSED
static void test_fill_color_with_one_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        disp_sw_fill_color(&dst, NULL, color);

        LCDC_INFO("#### Frame %u [Fill Color] drawing used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        lcdc_test_frame_xfer();

        lcdc_test_frame_wait_done();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_draw_rect_with_one_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xff000000;
    uint32_t color_index = 1;
    int step = 2;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };
    disp_area_t area = {
        .x = 50,
        .y = 50,
        .w = lcdc_width - 100,
        .h = lcdc_height - 100,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[color_index];
        disp_sw_fill_color(&dst, NULL, 0xffffffff);
        disp_sw_fill_color(&dst, &area, color);

        LCDC_INFO("#### Frame %u [Draw Rect] drawing used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        lcdc_test_frame_xfer();

        area.x += step;
        area.y += step;
        area.w -= step*2;
        area.h -= step*2;

        if (area.x > 200 || area.x < 50) {
            step = -step;

            color_index++;
            if (color_index >= NCOLORS)
                color_index = 1;
        }

        lcdc_test_frame_wait_done();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_draw_pic_with_one_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t offset = 0;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        if ((testing_ms/10000) % 2)
            lcdc_test_set_brightness(140);
        else
            lcdc_test_set_brightness(240);

        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        LCDC_INFO("#### Frame %u [Draw Picture] drawing used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        lcdc_test_frame_xfer();

        lcdc_test_frame_wait_done();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Draw Picture, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_fill_color_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [Fill Color] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];

        dst.data = buf;
        disp_sw_fill_color(&dst, NULL, color);

        LCDC_INFO("#### Frame %u [Fill Color] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);


        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_draw_rect_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xff000000;
    uint32_t color_index = 1;
    int step = 2;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };
    disp_area_t area = {
        .x = 50,
        .y = 50,
        .w = lcdc_width - 100,
        .h = lcdc_height - 100,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [Draw Rect] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[color_index];

        dst.data = buf;
        disp_sw_fill_color(&dst, NULL, 0xffffffff);
        disp_sw_fill_color(&dst, &area, color);

        LCDC_INFO("#### Frame %u [Draw Rect] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        area.x += step;
        area.y += step;
        area.w -= step*2;
        area.h -= step*2;

        if (area.x > 200 || area.x < 50) {
            step = -step;

            color_index++;
            if (color_index >= NCOLORS)
                color_index = 1;
        }

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);
        lcdc_test_buffer_set(0, buf);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_draw_pic_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t offset = 0;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf,
    };

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        if ((testing_ms/10000) % 2)
            lcdc_test_set_brightness(140);
        else
            lcdc_test_set_brightness(240);

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [Draw Picture] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        dst.data = buf;
        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        LCDC_INFO("#### Frame %u [Draw Picture] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);
        lcdc_test_buffer_set(0, buf);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Draw Picture, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_gpu_fill_color_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [Fill Color] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];

        gpu_test_set_buf(buf);
        gpu_test_fill_color(color);

        LCDC_INFO("#### Frame %u [Fill Color] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);


        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_gpu_draw_rect_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;
    int step = 10;
    int w = 200;
    int h = 200;
    int x = (lcdc_width - w) / 2;
    int y = (lcdc_height - h) / 2;
    uint32_t rotate = 0;

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();
    lcdc_test_set_brightness(240);

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [GPU Draw Rect] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];

        gpu_test_set_buf(buf);
        gpu_test_fill_color(color);
        gpu_test_draw_rect(x, y, w, h, rotate, 0xff00cccc);

        LCDC_INFO("#### Frame %u [GPU Draw Rect] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        if (w > 400 || w < 100)
            step = -step;
        w += step;
        h += step;
        x = (lcdc_width - w) / 2;
        y = (lcdc_height - h) / 2;

        rotate += 1;
        if (rotate > 360)
            rotate = rotate % 360;

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);
        lcdc_test_buffer_set(0, buf);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [GPU Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_gpu_draw_tiger_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    float scale = 2.0;
    float step = 0.1;
    uint32_t rotate = 0;

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        if ((testing_ms/10000) % 2)
            lcdc_test_set_brightness(140);
        else
            lcdc_test_set_brightness(240);

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [GPU Draw Tiger] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff0000ff);
        gpu_test_draw_tiger(lcdc_width/2, lcdc_height/2, scale, rotate);

        LCDC_INFO("#### Frame %u [GPU Draw Tiger] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));

        if (scale >= 3.0 || scale <= 1.0)
            step = -step;
        scale += step;
        rotate += 6;
        if (rotate > 360)
            rotate = rotate % 360;

        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);
        lcdc_test_buffer_set(0, buf);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [GPU Draw Tiger, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_gpu_draw_text_with_double_buffer(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = frame_buf0;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t draw_start;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t offset = 0;

    lcdc_test_buffer_set(0, buf);
    lcdc_test_start();

    do {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        if ((testing_ms/10000) % 2)
            lcdc_test_set_brightness(140);
        else
            lcdc_test_set_brightness(240);

        lcdc_test_frame_xfer();

        if (frames % 2 == 0)
            buf = frame_buf1;
        else
            buf = frame_buf0;

        LCDC_INFO("#### Frame %u [GPU Draw Path] drawing start.", frames+1);
        draw_start = hal_sys_timer_get();

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);

        for (int i = 0; i < 7; i++) {
            int y = i*100 - offset;
            int w = lcdc_test_get_width();
            int h = 95;
            gpu_test_clear_rect(0, y, w, h, (i%2) ? 0xffaaaaaa : 0xffcccccc);
            gpu_test_draw_text_xian(100, y+80, 0.9f, 0, 0xff000000);
            gpu_test_draw_text_xian(200, y+80, 0.9f, 0, 0xff000000);
            gpu_test_draw_text_xian(300, y+80, 0.9f, 0, 0xff00ffff);
            gpu_test_draw_text_xian(400, y+80, 0.9f, 0, 0xff00ffff);
        }

        offset += 2;
        if (offset >= 200)
            offset = 0;

        LCDC_INFO("#### Frame %u [GPU Draw Path] drawing used %ums",
                    frames+1, TICKS_TO_MS(hal_sys_timer_get() - draw_start));
        lcdc_test_frame_wait_done();

        LCDC_INFO("#### Frame %u ready for xfer, buffer %p", frames+1, buf);
        lcdc_test_buffer_set(0, buf);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        LCDC_INFO("#### Frame %u used %dms, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms)*1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();

    LCDC_INFO("LCDC Test [GPU Draw Path, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_with_two_layers(void)
{
    void *buf0 = lcdc_test_buffer_get(0);
    void *buf1 = lcdc_test_buffer_get(1);;
    uint32_t times = 0;
    disp_buffer_t dst = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = lcdc_format,
        .data = buf0,
    };
    disp_area_t area = {
        .x = 50,
        .y = 50,
        .w = 150,
        .h = 150,
    };

    lcdc_test_start();

    while (1) {
        LCDC_INFO("----------------------------------------------------------------------");
        LCDC_INFO("#### Two layers test %u start.", times++);

        LCDC_INFO("#### Display layer[0].");

        lcdc_test_layer_enable(0);
        lcdc_test_layer_disable(1);

        dst.data = buf0;
        disp_sw_fill_color(&dst, NULL, 0xff0000ff);
        area.x = 60;
        area.y = 150;
        disp_sw_fill_color(&dst, &area, 0xffffffff);

        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();

        osDelay(1000);

        LCDC_INFO("#### Display layer[1].");

        lcdc_test_layer_enable(1);
        lcdc_test_layer_disable(0);

        dst.data = buf1;
        disp_sw_fill_color(&dst, NULL, 0xff00ff00);
        area.x = 250;
        area.y = 150;
        disp_sw_fill_color(&dst, &area, 0xff000000);

        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();

        osDelay(1000);

        for (int i = 0x10; i < 0xf0; i += 0x20) {
            LCDC_INFO("#### Display two layers with tranparent [%x]", i);
            lcdc_test_layer_disable(0);
            lcdc_test_layer_disable(1);
            lcdc_test_const_transp_set(0, i);
            lcdc_test_const_transp_set(1, 0x0);
            lcdc_test_layer_enable(0);
            lcdc_test_layer_enable(1);

            lcdc_test_frame_xfer();
            lcdc_test_frame_wait_done();
            osDelay(1000);
        }
    }

    lcdc_test_stop();
}


void test_lcdc(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    LCDC_INFO("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf0);

    if (test_mode == TEST_WITH_TWO_LAYERS)
        lcdc_test_layer_init(1, frame_buf1);

    lcdc_width = lcdc_test_get_width();
    lcdc_height = lcdc_test_get_height();
    lcdc_format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565;

    if (lcdc_format == FB_FMT_RGB16_565)
        gpu_test_open(lcdc_width, lcdc_height, VG_LITE_RGB565, NULL);
    else
        gpu_test_open(lcdc_width, lcdc_height, 0, NULL);

    while (1) {
        switch (test_mode) {
            case TEST_WITH_GPU_DRAW:
                test_gpu_fill_color_with_double_buffer(30000, 60);
                test_gpu_draw_rect_with_double_buffer(30000, 60);
                test_gpu_draw_tiger_with_double_buffer(120000, 60);
                test_gpu_draw_text_with_double_buffer(120000, 60);
                break;
            case TEST_WITH_DOUBLE_BUFFER:
                test_fill_color_with_double_buffer(30000, 60);
                test_draw_rect_with_double_buffer(30000, 60);
                test_draw_pic_with_double_buffer(60000, 60);
                break;

            case TEST_WITH_TWO_LAYERS:
                test_with_two_layers();
                break;

            case TEST_WITH_ONE_BUFFER:
            default:
                test_fill_color_with_one_buffer(30000, 60);
                test_draw_rect_with_one_buffer(30000, 60);
                test_draw_pic_with_one_buffer(60000, 60);
                break;
        }
    }
    gpu_test_close();
    while (1);
}

#endif
