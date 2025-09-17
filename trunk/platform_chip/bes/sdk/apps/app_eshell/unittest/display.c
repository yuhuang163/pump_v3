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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && defined(TEST_WITH_LCD)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"

#include "display_test.h"

#include "ut_test.h"

#define frame_buf  frame_test_buf0

static void lcdc_test_display_update(void)
{
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
    lcdc_test_frame_xfer();
    lcdc_test_frame_wait_done();
}

static void test_fill_color(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color = 0xffffffff;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        disp_sw_fill_color(&dst, NULL, color);

        lcdc_test_display_update();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Fill Color, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_rect(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color = 0xff000000;
    uint32_t color_index = 1;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };
    disp_area_t area = {
        .x = 50,
        .y = 50,
        .w = lcdc_test_get_width() - 100,
        .h = lcdc_test_get_height() - 100,
    };
    int step = 2;

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[color_index];
        disp_sw_fill_color(&dst, NULL, 0xffffffff);
        disp_sw_fill_color(&dst, &area, color);

        lcdc_test_display_update();

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

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Draw Rect, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_pic(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };
    uint32_t offset = 0;

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    do {
        frame_start = hal_fast_sys_timer_get();

        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        lcdc_test_display_update();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Draw Picture, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Draw Picture, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_display_control(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    bool off = true;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };
    uint32_t offset = 0;

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    do {
        if (frames % fps == 0) {
            if (!off) {
                lcdc_test_display_off();
                UT_LOG("Display Test [Control: OFF]");
            } else {
                lcdc_test_display_on();
                UT_LOG("Display Test [Control: ON]");
            }
            off = !off;
        }

        frame_start = hal_fast_sys_timer_get();

        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        if (!off)
            lcdc_test_display_update();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Draw Picture, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    lcdc_test_display_on();

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Control, total %d frames, %ufps], total %ums",
                    frames, (frames * 1000) / (testing_ms), testing_ms);
}

static void test_brightness_control(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    bool screen = false;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };
    uint32_t offset = 0;

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    if (brightness % 2 == 0)
        screen = true;

    do {
        frame_start = hal_fast_sys_timer_get();

        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        lcdc_test_display_update();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Draw Picture, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;

        if (screen) {
            brightness += 40;
        } else {
            brightness -= 40;
        }

        if (brightness > 255 || brightness < 0) {
            screen = !screen;
            continue;
        }

        lcdc_test_set_brightness(brightness);
    }while (testing_ms < total_time_ms);

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Brightness, total %d frames, %ufps], brightness %d",
                    frames, (frames * 1000) / (testing_ms), brightness);
}

static void test_reduce_brightness(uint32_t total_time_ms, uint32_t fps, uint32_t brightness)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    disp_buffer_t dst = {
        .width = lcdc_test_get_width(),
        .height = lcdc_test_get_height(),
        .format = (lcdc_test_get_bpp() == 32) ? FB_FMT_RGB32 : FB_FMT_RGB16_565,
        .data = frame_buf,
    };
    uint32_t offset = 0;

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);
    lcdc_test_start();

    lcdc_test_set_brightness(brightness);

    do {
        frame_start = hal_fast_sys_timer_get();

        disp_test_draw_pic(&dst, 48, 0, offset, 0);
        offset += 2;

        lcdc_test_display_update();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("Display Test [Draw Picture, frame %d] used %dus",
                frames, frame_us);

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;

        if (brightness > 50 && brightness <= 255) {
            brightness -= 36;
            lcdc_test_set_brightness(brightness);
            }
    }while (testing_ms < total_time_ms);

    lcdc_test_stop();
    lcdc_test_exit();

    UT_LOG("Display Test [Reduce Brightness, total %d frames, %ufps], brightness %d",
                    frames, (frames * 1000) / (testing_ms), brightness);
}

static void unitest_display_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  utest_display fillcolor [time_ms fps brightness] brightness_value: 0-255\r\n");
    eshell_putstring("  utest_display drawrect [time_ms fps brightness] brightness_value: 0-255\r\n");
    eshell_putstring("  utest_display drawpic [time_ms fps brightness] brightness_value: 0-255\r\n");
    eshell_putstring("  utest_display onoff [time_ms fps brightness] brightness_value: 0-255\r\n");
    eshell_putstring("  utest_display brightness [time_ms fps brightness] brightness_value: 0-255\r\n");
    eshell_putstring("  utest_display lowlight [time_ms fps brightness] brightness_value: 0-255\r\n");
}

static void unitest_display(int argc, char *argv[])
{
    uint32_t time_ms = 10000;
    uint32_t fps = 20;
    uint32_t brightness = 255;
    
    if (argc < 2)
        goto usage;

    if (argc > 2)
        time_ms = atoi(argv[2]);
    if (argc > 3)
        fps = atoi(argv[3]);
    if (argc > 4)
        brightness = atoi(argv[4]);
    
    if (brightness < 0 || brightness > 255)
        goto usage;

    if (strncmp(argv[1], "fillcolor", 9) == 0) {
        test_fill_color(time_ms, fps, brightness);

    } else if (strncmp(argv[1], "drawrect", 8) == 0) {
        test_draw_rect(time_ms, fps, brightness);

    } else if (strncmp(argv[1], "drawpic", 7) == 0) {
        test_draw_pic(time_ms, fps, brightness);

    } else if (strncmp(argv[1], "onoff", 5) == 0) {
        test_display_control(time_ms, fps, brightness);

    } else if (strncmp(argv[1], "brightness", 10) == 0){
        test_brightness_control(time_ms, fps, brightness);
        
    } else if (strncmp(argv[1], "lowlight", 8) == 0){
        test_reduce_brightness(time_ms, fps, brightness);

    } else {
        goto usage;
    }

    return;

usage:
    unitest_display_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_display", "usage: utest_display help",
                   unitest_display);
#endif
