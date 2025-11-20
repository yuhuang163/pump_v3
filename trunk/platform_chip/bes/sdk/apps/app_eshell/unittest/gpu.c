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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && defined(TEST_WITH_GPU)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "hal_iomux.h"

#include "display_test.h"

#include "ut_test.h"


static int test_width = 464;
static int test_height = 454;
static int test_bpp = 32;

static int test_with_lcdc = true;

static void test_start(void)
{
    void *buf = frame_test_nc_buf0;

    if (!test_with_lcdc)
        return;

    lcdc_test_init();
    lcdc_test_layer_init(0, buf);
    lcdc_test_start();

    test_width = lcdc_test_get_width();
    test_height = lcdc_test_get_height();
    test_bpp = lcdc_test_get_bpp();

    if (test_bpp == 16)
        gpu_test_open(test_width, test_height, VG_LITE_BGR565, buf);
    else
        gpu_test_open(test_width, test_height, 0, buf);
}

static void test_stop(void)
{
    gpu_test_close();

    if (test_with_lcdc) {
        lcdc_test_stop();
        lcdc_test_exit();
    }
}

static void test_display_update(void)
{
    if (test_with_lcdc) {
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
    }
}

static void test_tiger(uint32_t total_time_ms, uint32_t fps, int scale, uint32_t rotate)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t step = 10;

    test_start();

    if (scale < 100)
        scale = 100;
    if (scale > 300)
        scale = 300;

    rotate = rotate % 360;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xff0000ff);
        gpu_test_draw_tiger(test_width/2, test_height/2, (float)scale/100.0, rotate);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Draw Tiger, frame %d, scale %d%%, rotate %d] used %dus",
                    frames, scale, rotate, frame_us);

        test_display_update();

        if (scale >= 400 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 6;
        rotate = rotate % 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }

        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Draw Tiger, total %d frames, %ufps]",
                    frames, (frames * 1000) / testing_ms);
}

static void test_pacman(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t index = 0;
    uint32_t rotate = 0;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xff000000);
        gpu_test_draw_pacman(index, test_width/2, test_height/2, rotate, 0xff00ffff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Vector Test, frame %d, rotate %d] used %dus",
                    frames, rotate, frame_us);

        test_display_update();

        index += 2;
        if (index >= 100) {
            index = 0;
            rotate += 90;
            rotate = rotate % 360;
        }

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }

        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Vector Test, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_fillcolor(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        gpu_test_fill_color(color);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Fill Color, frame %d] used %dus",
                frames, frame_us);

        test_display_update();

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();


    UT_LOG("GPU Test [Fill Color, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_clear_rect(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t w = test_width;
    uint32_t h = test_height;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[frames % NCOLORS];
        gpu_test_clear_rect(x, y, w, h, color);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Clear Rect, frame %d] used %dus",
                frames, frame_us);

        test_display_update();

        x += 20;
        y += 20;
        w -= 40;
        h -= 40;
        if (x > 200) {
            x = 0;
            y = 0;
            w = test_width;
            h = test_height;
        }

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while(testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Clear Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_drawline(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[(frames % (NCOLORS-1))+1];
        switch (frames % 5) {
            case 1:
                gpu_test_draw_rect_line(10, 220, 440, 220, 10, color);
                break;
            case 2:
                gpu_test_draw_rect_line(230, 10, 220, 440, 10, color);
                break;
            case 3:
                gpu_test_draw_rect_line(85, 80, 385, 380, 10, color);
                break;
            case 4:
                gpu_test_draw_rect_line(380, 75, 70, 385, 10, color);
                break;
            case 0:
            default:
                gpu_test_fill_color(test_colors_RGBA8888[0]);
                break;
        }

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Draw Line, frame %d] used %dus",
                frames, frame_us);

        test_display_update();

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Draw Line, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_drawrect(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int step = 10;
    int w = 200;
    int h = 200;
    int x = (test_width - w) / 2;
    int y = (test_height - h) / 2;
    uint32_t rotate = 0;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_rect(x, y, w, h, rotate, 0xff0000ff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Draw Rect, frame %d, rotate %d] used %dus",
                frames, rotate, frame_us);

        test_display_update();

        if (w > 400 || w < 100)
            step = -step;
        w += step;
        h += step;
        x = (test_width - w) / 2;
        y = (test_height - h) / 2;

        rotate += 6;
        if (rotate >= 360)
            rotate = rotate % 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_circle(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int step = 10;
    int scale = 200;

    test_start();

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_arc_circle(test_width/2, test_height/2, scale*50/100, 0, 360, 0xff00ff00);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        UT_LOG("GPU Test [Draw Circle, frame %d, scale %d%%] used %dus",
                frames, scale, frame_us);

        test_display_update();

        if (scale >= 400 || scale <= 100)
            step = -step;
        scale += step;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    test_stop();

    UT_LOG("GPU Test [Draw Circle, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void unitest_gpu_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  utest_gpu tiger [time_ms fps scale rotate]\r\n");
    eshell_putstring("          time_ms:         test drawing time in mili-seconds, default is 1000ms.\r\n");
    eshell_putstring("          fps:             drawing framerate (1~60), default is 20fps.\r\n");
    eshell_putstring("          scale:           drawing init scale, default is 1.\r\n");
    eshell_putstring("          rotate:          drawing init rotate degree, default is 0.\r\n");
    eshell_putstring("  utest_gpu drawline [time_ms fps]\r\n");
    eshell_putstring("  utest_gpu drawrect [time_ms fps]\r\n");
    eshell_putstring("  utest_gpu drawcircle [time_ms fps]\r\n");
    eshell_putstring("  utest_gpu pacman [time_ms fps]\r\n");
    eshell_putstring("  utest_gpu fillcolor [time_ms fps]\r\n");
    eshell_putstring("  utest_gpu clearrect [time_ms fps]\r\n");
}

static void unitest_gpu_blit_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  utest_gpu blit src_addr src_w src_h src_format dst_addr dst_w dst_h dst_format [blend_mode scale rotation]\r\n");
    eshell_putstring("           src_addr:       source address.\r\n");
    eshell_putstring("           src_w:          source width: 16 ~ 512, aligned to 16.\r\n");
    eshell_putstring("           src_h:          source height: 16 ~ 512.\r\n");
    eshell_putstring("           src_format      source color format: rgba8888, rgb565, a4, a8.\r\n");
    eshell_putstring("           dst_addr:       destination address.\r\n");
    eshell_putstring("           dst_w:          destination width: 16 ~ 512, aligned to 16.\r\n");
    eshell_putstring("           dst_h:          destination height: 16 ~ 512.\r\n");
    eshell_putstring("           dst_format:     destination color format: rgba8888, rgb565.\r\n");
    eshell_putstring("           blend_mode:     blending mode: none, srcover, mutilply, screen.\r\n");
    eshell_putstring("           scale:          scale percent: 10 ~ 9000 (%%)\r\n");
    eshell_putstring("           rotation:       rotation angle: 0 ~ 360.\r\n");
}

static void unitest_gpu_blit(int argc, char *argv[])
{
    uint32_t src_addr;
    int src_w, src_h;
    int src_format = VG_LITE_RGBA8888;
    uint32_t dst_addr;
    int dst_w, dst_h;
    int dst_format = VG_LITE_RGBA8888;
    int blend = VG_LITE_BLEND_NONE;
    int scale = 100;
    int rotate = 0;

    if (argc < 10)
        goto usage;

    src_addr = strtoul(argv[2], NULL, 16);
    src_w = strtol(argv[3], NULL, 10);
    src_h = strtol(argv[4], NULL, 10);

    if (strncmp(argv[5], "rgba8888", 8) == 0)
        src_format = VG_LITE_RGBA8888;
    else if (strncmp(argv[5], "rgb565", 6) == 0)
        src_format = VG_LITE_RGB565;
    else if (strncmp(argv[5], "a4", 2) == 0)
        src_format = VG_LITE_A4;
    else if (strncmp(argv[5], "a8", 2) == 0)
        src_format = VG_LITE_A8;
    else
        goto usage;

    dst_addr = strtoul(argv[6], NULL, 16);
    dst_w = strtol(argv[7], NULL, 10);
    dst_h = strtol(argv[8], NULL, 10);

    if (strncmp(argv[9], "rgba8888", 8) == 0)
        dst_format = VG_LITE_RGBA8888;
    else if (strncmp(argv[9], "rgb565", 6) == 0)
        dst_format = VG_LITE_RGB565;
    else
        goto usage;

    if (argc > 10) {
        if (strncmp(argv[10], "none", 4) == 0)
            blend = VG_LITE_BLEND_NONE;
        else if (strncmp(argv[10], "srcover", 7) == 0)
            blend = VG_LITE_BLEND_SRC_OVER;
        else if (strncmp(argv[10], "multiply", 8) == 0)
            blend = VG_LITE_BLEND_MULTIPLY;
        else if (strncmp(argv[10], "screen", 6) == 0)
            blend = VG_LITE_BLEND_SCREEN;
        else
            goto usage;
    }

    if (argc > 11) {
        scale = strtol(argv[11], NULL, 10);
        if (scale < 10)
            scale = 10;
        if (scale > 9000)
            scale = 9000;
    }

    if (argc > 12)
        rotate = strtol(argv[12], NULL, 10) % 360;

    if (src_w > GPU_TEST_MAX_WIDTH || src_w < 16 || (src_w % 16) > 0 ||
        dst_w > GPU_TEST_MAX_WIDTH || dst_w < 16 || (dst_w % 16) > 0) {
        eshell_putstring("blit width must be in 16 ~ %u, aligned to 16.\r\n", GPU_TEST_MAX_WIDTH);
        return;
    }

    if (src_h > GPU_TEST_MAX_HEIGHT || src_h < 16 ||
        dst_h > GPU_TEST_MAX_HEIGHT || dst_h < 16) {
        eshell_putstring("blit height must be in 16 ~ %u.\r\n", GPU_TEST_MAX_HEIGHT);
        return;
    }

    UT_LOG("GPU Test [blit 0x%08x (%dx%d, %s) -> 0x%08x (%dx%d, %s)]",
                        src_addr, src_w, src_h, argv[5],
                        dst_addr, dst_w, dst_h, argv[9]);

    gpu_test_open(GPU_TEST_MAX_WIDTH, GPU_TEST_MAX_HEIGHT, 0, NULL);

    gpu_test_set_blend(blend);
    gpu_test_blit((void *)src_addr, src_w, src_h, src_format,
                  (void *)dst_addr, dst_w, dst_h, dst_format,
                  (float)scale / 100.0, rotate);

    gpu_test_close();
    return;

usage:
    unitest_gpu_blit_usage();
}

static void unitest_gpu(int argc, char *argv[])
{
    uint32_t time_ms = 1000;
    uint32_t fps = 20;
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "blit", 4) == 0) {
        unitest_gpu_blit(argc, argv);
        return;
    }

    if (argc > 2)
        time_ms = atoi(argv[2]);
    if (argc > 3)
        fps = atoi(argv[3]);

    if (strncmp(argv[1], "tiger", 5) == 0) {
        uint32_t scale = 1;
        uint32_t rotate = 0;

        if (argc > 4)
            scale = atoi(argv[4]);
        if (argc > 5)
            rotate = atoi(argv[5]);

        test_tiger(time_ms, fps, scale, rotate);

    } else if (strncmp(argv[1], "fillcolor", 9) == 0) {
        test_fillcolor(time_ms, fps);

    } else if (strncmp(argv[1], "clearrect", 9) == 0) {
        test_clear_rect(time_ms, fps);

    } else if (strncmp(argv[1], "drawline", 8) == 0) {
        test_drawline(time_ms, fps);

    } else if (strncmp(argv[1], "drawrect", 8) == 0) {
        test_drawrect(time_ms, fps);

    } else if (strncmp(argv[1], "drawcircle", 10) == 0) {
        test_draw_circle(time_ms, fps);

    } else if (strncmp(argv[1], "pacman", 6) == 0) {
        test_pacman(time_ms, fps);

    } else {
        goto usage;
    }

    return;

usage:
    unitest_gpu_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_gpu", "usage: utest_gpu help",
                   unitest_gpu);
#endif
