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
#ifdef TEST_FB

#include <stdio.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_cache.h"
#include "hal_location.h"

#include "display_test.h"

static uint8_t *fb_mem = NULL;
static int fb_width = 464;
static int fb_height = 454;
static uint8_t fb_format = FB_FMT_RGB32;

static void test_fill_color(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;
    disp_buffer_t dst;

    dst.width = fb_width;
    dst.height = fb_height;
    dst.format = fb_format;
    dst.data = fb_mem;

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        disp_sw_fill_color(&dst, NULL, color);

        DISP_LOG("#### Frame %u fill color used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        fb_test_update();

        fb_test_wait_sync();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dus, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms) * 1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    DISP_LOG("FB Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_rect(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xff000000;
    uint32_t color_index = 1;
    int step = 2;
    disp_buffer_t dst;
    disp_area_t area;

    dst.width = fb_width;
    dst.height = fb_height;
    dst.format = fb_format;
    dst.data = fb_mem;

    area.x = 50;
    area.y = 50;
    area.w = fb_width-100;
    area.h = fb_height-100;

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        color = test_colors_RGBA8888[color_index];

        disp_sw_fill_color(&dst, NULL, 0xffffffff);
        disp_sw_fill_color(&dst, &area, color);

        DISP_LOG("#### Frame %u draw rect used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        fb_test_update();

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

        fb_test_wait_sync();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dus, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms) * 1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    DISP_LOG("FB Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

#if 1
/* Picture RGB888 */
#define PIC_WIDTH   368
#define PIC_HEIGHT  448
#define PIC_FORMAT  FB_FMT_RGB32
static const uint8_t pic_data[] = {
#include "resource/RGB888_unpacked_368x448.txt"
};
#else
/* Picture RGB565 */
#define PIC_WIDTH   360
#define PIC_HEIGHT  360
#define PIC_FORMAT  FB_FMT_RGB16_565
static const uint16_t pic_data[] = {
#include "resource/RGB565_unpacked_360x360.txt"
};
#endif

static void test_blit_pic(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    int ystep = 2;
    disp_buffer_t src, dst;
    disp_area_t src_area1;
    disp_area_t src_area2;
    disp_area_t dst_area1;
    disp_area_t dst_area2;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = PIC_FORMAT;
    src.data = (uint8_t *)pic_data;

    src_area1.x = 0;
    src_area1.y = 0;
    src_area1.w = PIC_WIDTH;
    src_area1.h = PIC_HEIGHT;

    src_area2.x = 0;
    src_area2.y = 0;
    src_area2.w = PIC_WIDTH;
    src_area2.h = PIC_HEIGHT;

    dst.width = fb_width;
    dst.height = fb_height;
    dst.format = fb_format;
    dst.data = fb_mem;

    dst_area1.x = (fb_width-PIC_WIDTH)/2;
    dst_area1.y = 0;
    dst_area1.w = PIC_WIDTH;
    dst_area1.h = PIC_HEIGHT;

    dst_area2.x = (fb_width-PIC_WIDTH)/2;
    dst_area2.y = PIC_HEIGHT;
    dst_area2.w = PIC_WIDTH;
    dst_area2.h = fb_height-PIC_HEIGHT;

    disp_sw_fill_color(&dst, NULL, 0xffffffff);

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);

        frame_start = hal_sys_timer_get();

        src_area1.y += ystep;
        dst_area1.h -= ystep;
        disp_sw_blit(&src, &src_area1, &dst, &dst_area1);

        dst_area2.y -= ystep;
        dst_area2.h += ystep;
        disp_sw_blit(&src, &src_area2, &dst, &dst_area2);

        DISP_LOG("#### Frame %u blit picture used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        fb_test_update();

        if (dst_area1.h <= 2) {
            src_area1.y = 0;
            dst_area1.h = PIC_HEIGHT;
            dst_area2.y = PIC_HEIGHT;
            dst_area2.h = fb_height-PIC_HEIGHT;
        }

        fb_test_wait_sync();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dus, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms) * 1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    DISP_LOG("FB Test [Blit Picture, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_blend_pic(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint8_t opa = 20;
    int opa_step = 1;
    int ystep = 2;
    disp_buffer_t src, dst;
    disp_area_t src_area1;
    disp_area_t src_area2;
    disp_area_t dst_area1;
    disp_area_t dst_area2;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = PIC_FORMAT;
    src.data = (uint8_t *)pic_data;

    src_area1.x = 0;
    src_area1.y = 0;
    src_area1.w = PIC_WIDTH;
    src_area1.h = PIC_HEIGHT;

    src_area2.x = 0;
    src_area2.y = 0;
    src_area2.w = PIC_WIDTH;
    src_area2.h = PIC_HEIGHT;

    dst.width = fb_width;
    dst.height = fb_height;
    dst.format = fb_format;
    dst.data = fb_mem;

    dst_area1.x = (fb_width-PIC_WIDTH)/2;
    dst_area1.y = 0;
    dst_area1.w = PIC_WIDTH;
    dst_area1.h = PIC_HEIGHT;

    dst_area2.x = (fb_width-PIC_WIDTH)/2;
    dst_area2.y = PIC_HEIGHT;
    dst_area2.w = PIC_WIDTH;
    dst_area2.h = fb_height-PIC_HEIGHT;

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);

        frame_start = hal_sys_timer_get();

        disp_sw_fill_color(&dst, NULL, 0xff0000ff);

        src_area1.y += ystep;
        dst_area1.h -= ystep;
        disp_sw_blend(&src, &src_area1, &dst, &dst_area1, FB_CONST_ALPHA, opa);

        dst_area2.y -= ystep;
        dst_area2.h += ystep;
        disp_sw_blend(&src, &src_area2, &dst, &dst_area2, FB_CONST_ALPHA, opa);

        DISP_LOG("#### Frame %u blend picture used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        fb_test_update();

        if (dst_area1.h <= 2) {
            src_area1.y = 0;
            dst_area1.h = PIC_HEIGHT;
            dst_area2.y = PIC_HEIGHT;
            dst_area2.h = fb_height-PIC_HEIGHT;
        }

        opa += opa_step;
        if (opa >= 240 || opa <= 10)
            opa_step = -opa_step;

        fb_test_wait_sync();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dus, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms) * 1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    DISP_LOG("FB Test [Blend Picture, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_display_control(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    bool off = true;
    int ystep = 2;
    disp_buffer_t src, dst;
    disp_area_t src_area1;
    disp_area_t src_area2;
    disp_area_t dst_area1;
    disp_area_t dst_area2;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = PIC_FORMAT;
    src.data = (uint8_t *)pic_data;

    src_area1.x = 0;
    src_area1.y = 0;
    src_area1.w = PIC_WIDTH;
    src_area1.h = PIC_HEIGHT;

    src_area2.x = 0;
    src_area2.y = 0;
    src_area2.w = PIC_WIDTH;
    src_area2.h = PIC_HEIGHT;

    dst.width = fb_width;
    dst.height = fb_height;
    dst.format = fb_format;
    dst.data = fb_mem;

    dst_area1.x = (fb_width-PIC_WIDTH)/2;
    dst_area1.y = 0;
    dst_area1.w = PIC_WIDTH;
    dst_area1.h = PIC_HEIGHT;

    dst_area2.x = (fb_width-PIC_WIDTH)/2;
    dst_area2.y = PIC_HEIGHT;
    dst_area2.w = PIC_WIDTH;
    dst_area2.h = fb_height-PIC_HEIGHT;

    disp_sw_fill_color(&dst, NULL, 0xffffffff);

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);

        if (frames % fps == 0) {
            if (!off) {
                fb_test_display_off();
                DISP_LOG("Display Test [Control: OFF]");
            } else {
                fb_test_display_on();
                DISP_LOG("Display Test [Control: ON]");
            }
            off = !off;
        }

        frame_start = hal_sys_timer_get();

        src_area1.y += ystep;
        dst_area1.h -= ystep;
        disp_sw_blit(&src, &src_area1, &dst, &dst_area1);

        dst_area2.y -= ystep;
        dst_area2.h += ystep;
        disp_sw_blit(&src, &src_area2, &dst, &dst_area2);

        DISP_LOG("#### Frame %u blit picture used %ums",
                    frames, TICKS_TO_MS(hal_sys_timer_get() - frame_start));

        if (!off)
            fb_test_update();

        if (dst_area1.h <= 2) {
            src_area1.y = 0;
            dst_area1.h = PIC_HEIGHT;
            dst_area2.y = PIC_HEIGHT;
            dst_area2.h = fb_height-PIC_HEIGHT;
        }

        if (!off)
            fb_test_wait_sync();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dus, dynamic fps %u",
                frames, frame_ms, 1000/frame_ms);

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        if (frame_ms < frame_interval_ms) {
            hal_sys_timer_delay_us((frame_interval_ms-frame_ms) * 1000);
            testing_ms += frame_interval_ms;
        } else {
            testing_ms += frame_ms;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    DISP_LOG("FB Test [Control, total %ums]", testing_ms);

    fb_test_display_on();
}

void test_fb(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    DISP_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    fb_test_init();

    osDelay(500);

    fb_width = fb_test_get_width();
    fb_height = fb_test_get_height();
    fb_format = (fb_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565;
    fb_mem = fb_test_get_fbmem();

    while (1) {
        test_fill_color(20000, 30);
        test_draw_rect(20000, 60);
        test_blit_pic(60000, 40);
        test_blend_pic(60000, 40);
        test_display_control(30000, 40);
    }

    fb_test_exit();
}

#endif
