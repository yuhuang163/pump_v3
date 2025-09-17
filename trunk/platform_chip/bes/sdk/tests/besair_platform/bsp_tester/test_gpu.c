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

#ifdef TEST_GPU

#include <stdio.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_location.h"

#include "display_test.h"

#define TEST_WITH_LCDC

#define GPU_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)

#define TEST_WIDTH      464
#define TEST_HEIGHT     454

static int test_width = TEST_WIDTH;
static int test_height = TEST_HEIGHT;
static int test_format = VG_LITE_RGBA8888;

static char render_buffer[0x100000] ALIGNED(64) PSRAM_NC_BSS_LOC;
static char frame_buffer[0x100000] ALIGNED(64) PSRAM_NC_BSS_LOC;

static void gpu_test_display_update(void)
{
#ifdef TEST_WITH_LCDC
    gpu_test_blit_to(frame_buffer);
    lcdc_test_frame_xfer();
    lcdc_test_frame_wait_done();
#endif
}

static void gpu_test_frame_xfer(void)
{
#ifdef TEST_WITH_LCDC
    lcdc_test_frame_xfer();
    lcdc_test_frame_wait_done();
#endif
}

static void test_blit(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = render_buffer;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t scale = 20;
    int step = 10;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_set_buf(frame_buffer);
        gpu_test_fill_color(0xffffffff);
        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff0000ff);

        gpu_test_blit(buf, test_width, test_height, test_format,
                      frame_buffer, test_width, test_height, test_format,
                      (float)scale/100.0, rotate);
        gpu_test_frame_xfer();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Blit, frame %d, scale %d%%, rotate %d] used %dus",
                    frames, scale, rotate, frame_us);

        if (frames > 0 && frames % fps == 0) {
            if (scale >= 90 || scale <= 10)
                step = -step;
            scale += step;
        }

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


    GPU_INFO("GPU Test [Blit, total %d frames, %ufps]",
                    frames, (frames * 1000) / testing_ms);
}

static void test_blit_rect(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = render_buffer;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t rect[4] = {200, 200, 50, 50};
    int step = 10;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_set_buf(frame_buffer);
        gpu_test_fill_color(0xffffffff);
        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff0000ff);
        gpu_test_blit_rect(buf, test_width, test_height, test_format,
                      frame_buffer, test_width, test_height, test_format,
                      rect, 1.0f, rotate);
        gpu_test_frame_xfer();

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Blit Rect, frame %d, rect(%d, %d, %d, %d), rotate %d] used %dus",
                    frames, rect[0], rect[1], rect[2], rect[3], rotate, frame_us);

        if (frames > 0 && frames % fps == 0) {
            if (rect[0] >= 220 || rect[0] <= 20)
                step = -step;
            rect[0] -= step;
            rect[1] -= step;
            rect[2] += step * 2;
            rect[3] += step * 2;
        }

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


    GPU_INFO("GPU Test [Blit Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / testing_ms);
}

static int pic_width = 368;
static int pic_height = 448;
static int pic_format = VG_LITE_RGBA8888;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t pic_data[] = {
#include "resource/RGB888_unpacked_368x448.txt"
};
static void test_draw_pattern_rect(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = render_buffer;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t scale = 200;
    int step = 10;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff000000);
        gpu_test_draw_pattern_rect(pic_data, pic_width, pic_height, pic_format,
                              buf, test_width, test_height, test_format,
                              (float)scale/100.0f, rotate);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Pattern Rect, frame %d, scale %d%%, rotate %d] used %dus",
                    frames, scale, rotate, frame_us);

        gpu_test_display_update();

        if (scale >= 400 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 2;
        if (rotate > 360)
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


    GPU_INFO("GPU Test [Blend, total %d frames, %ufps]",
                    frames, (frames * 1000) / testing_ms);
}

static void test_draw_pattern_circle(uint32_t total_time_ms, uint32_t fps)
{
    void *buf = render_buffer;
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t scale = 200;
    int step = 10;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff000000);
        gpu_test_draw_pattern_circle(pic_data, pic_width, pic_height, pic_format,
                            buf, test_width, test_height, test_format,
                            (float)scale/100.0);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Pattern Circle, frame %d, scale %d%%] used %dus",
                    frames, scale, frame_us);

        gpu_test_display_update();

        if (scale >= 400 || scale <= 50)
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


    GPU_INFO("GPU Test [Blend Circle, total %d frames, %ufps]",
                    frames, (frames * 1000) / testing_ms);
}

static void test_draw_tiger(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t step = 10;
    int scale = 100;
    int rotate = 0;

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
        GPU_INFO("GPU Test [Draw Tiger, frame %d, scale %d%%, rotate %d] used %dus",
                    frames, scale, rotate, frame_us);

        gpu_test_display_update();

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


    GPU_INFO("GPU Test [Draw Tiger, total %d frames, %ufps]",
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

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffff0000);
        gpu_test_draw_pacman(index, test_width/2, test_height/2, rotate, 0xff00ffff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Vector Test, frame %d, rotate %d] used %dus",
                    frames, rotate, frame_us);

        gpu_test_display_update();

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

    GPU_INFO("GPU Test [Vector Test, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_fillcolor(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    uint32_t color;

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        gpu_test_fill_color(color);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Fill Color, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);


    GPU_INFO("GPU Test [Fill Color, total %d frames, %ufps]",
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

    do {
        frame_start = hal_fast_sys_timer_get();

        color = test_colors_RGBA8888[frames % NCOLORS];
        gpu_test_clear_rect(x, y, w, h, color);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Clear Rect, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

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

    GPU_INFO("GPU Test [Clear Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

#define LINE_COUNT      8
#define LINE_POINT_NUM  16

POSSIBLY_UNUSED
static vg_lite_point_t line_points[LINE_COUNT][LINE_POINT_NUM] = {
    {
        {0,0}, {28,36}, {55,36}, {89,32}, {123,26}, {144,37}, {172,38}, {198,33},
        {221,33}, {247,32}, {273,22}, {307,37}, {328,23}, {354,34}, {379,28}, {419,32}
    },
    {
        {0,0}, {41,32}, {66,21}, {97,32}, {123,27}, {144,40}, {167,41}, {194,29},
        {221,32}, {249,24}, {290,24}, {326,40}, {354,41}, {375,23}, {410,33}, {438,23}
    },
    {
        {0,0}, {27,36}, {61,32}, {95,26}, {116,37}, {144,38}, {170,33}, {193,33},
        {219,32}, {245,22}, {279,37}, {300,23}, {326,34}, {351,28}, {391,32}, {424,29}
    },
    {
        {0,0}, {25,21}, {56,32}, {82,27}, {103,40}, {126,41}, {153,29}, {180,32},
        {208,24}, {249,24}, {285,40}, {313,41}, {334,23}, {369,33}, {397,23}, {426,28}
    },
    {
        {0,0}, {34,32}, {68,26}, {89,37}, {117,38}, {143,33}, {166,33}, {192,32},
        {218,22}, {252,37}, {273,23}, {299,34}, {324,28}, {364,32}, {397,29}, {419,41}
    },
    {
        {0,0}, {31,32}, {57,27}, {78,40}, {101,41}, {128,29}, {155,32}, {183,24},
        {224,24}, {260,40}, {288,41}, {309,23}, {344,33}, {372,23}, {401,28}, {437,27}
    },
    {
        {0,0}, {34,26}, {55,37}, {83,38}, {109,33}, {132,33}, {158,32}, {184,22},
        {218,37}, {239,23}, {265,34}, {290,28}, {330,32}, {363,29}, {385,41}, {417,25}
    },
    {
        {0,0}, {26,27}, {47,40}, {70,41}, {97,29}, {124,32}, {152,24}, {193,24},
        {229,40}, {257,41}, {278,23}, {313,33}, {341,23}, {370,28}, {406,27}, {442,34}
    },
};

extern int rand(void);
static void test_draw_line(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int offs_x[LINE_COUNT];
    int offs_y = 100;
    int step = 10;

    for (int i = 0; i < LINE_COUNT; i++)
        offs_x[i] = rand() % 100;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);

        for (int i = 0; i < LINE_COUNT; i++) {
            if (i % 2 == 0)
                gpu_test_draw_stroke_lines(line_points[i], LINE_POINT_NUM, offs_x[i], i*20 + offs_y,
                                6, test_colors_RGBA8888[i+2]);
            else
                gpu_test_draw_rect_lines(line_points[i], LINE_POINT_NUM, offs_x[i], i*20 + offs_y,
                                6, test_colors_RGBA8888[i+2]);
        }

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Line, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        if (offs_y >= 400 || offs_y <= 0)
            step = -step;
        offs_y += step;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw Line, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_rect(uint32_t total_time_ms, uint32_t fps)
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

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_rect(x, y, w, h, rotate, 0xff0000ff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Rect, frame %d, rotate %d] used %dus",
                frames, rotate, frame_us);

        gpu_test_display_update();

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

    GPU_INFO("GPU Test [Draw Rect, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_circle(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int cx = test_width/2;
    int cy = test_height/2;
    int r = 200;
    int angle = 32;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_cubic_circle(cx, cy, r, 30, angle, 0xff00ff00);
        gpu_test_draw_arc_circle(cx, cy, r-30, 30, angle, 0xff0000ff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Circle, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        angle += 2;
        if (angle > 360)
            angle %= 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw Circle, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static void test_draw_ring(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int angle = 32;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_cubic_ring(test_width/2-5, test_height/2,
                        180, 40, 30, angle, 0xff0000ff);
        gpu_test_draw_arc_ring(test_width/2-5, test_height/2,
                        120, 40, 30, angle, 0xff00ff00);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Arc, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        angle += 2;
        if (angle > 360)
            angle %= 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < fps_delay_us) {
            hal_sys_timer_delay_us(fps_delay_us-frame_us);
            testing_ms += fps_delay_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw Arc, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static int icon_width = 112;
static int icon_height = 104;
static int icon_format = VG_LITE_RGBA8888;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t icon1_data[] = {
#include "resource/RGB888_unpacked_icon1_112x104.txt"
};
static ALIGNED(64) PSRAM_DATA_LOC uint8_t icon2_data[] = {
#include "resource/RGB888_unpacked_icon2_112x104.txt"
};
static void test_draw_icon(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int step = 1;
    int scale = 130;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xff000000);

        gpu_test_blit_icon(icon1_data, icon_width, icon_height,
                        icon_format, 100, 100, (float)scale/100.0f);
        gpu_test_blit_icon(icon2_data, icon_width, icon_height,
                        icon_format, 260, 100, (float)scale/100.0f);
        gpu_test_draw_icon(icon1_data, icon_width, icon_height,
                        icon_format, 100, 250, (float)scale/100.0f);
        gpu_test_draw_icon(icon2_data, icon_width, icon_height,
                        icon_format, 260, 250, (float)scale/100.0f);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw icon, frame %d, scale %d%%] used %dus",
                frames, scale, frame_us);

        gpu_test_display_update();

        if (scale >= 140 || scale <= 80)
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

    GPU_INFO("GPU Test [Draw icon, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static int index8_width = 160;
static int index8_height = 160;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t index8_data[] = {
#include "resource/Index8_image_data_160x160.txt"
};
static uint32_t index8_colors[] = {
#include "resource/Index8_color_table.txt"
};
POSSIBLY_UNUSED
static void test_draw_index8(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int step = 1;
    int scale = 80;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);

        gpu_test_draw_index8(index8_data, index8_width, index8_height,
                             index8_colors, sizeof(index8_colors)/4, scale/100.0f, rotate);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw index8 image, frame %d, scale %d%%] used %dus",
                frames, scale, frame_us);

        gpu_test_display_update();

        if (scale >= 150 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 2;
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

    GPU_INFO("GPU Test [Draw index8 image, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

#ifndef CHIP_BEST1600
static ALIGNED(64) PSRAM_DATA_LOC uint8_t etc2pkm_data[] = {
#include "resource/etc2pkm_unpacked_400x300.txt"
};

POSSIBLY_UNUSED
static void test_draw_etc2(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t fps_delay_us = 1000000 / fps;
    int step = 1;
    int scale = 80;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffffffff);

        gpu_test_draw_etc2pkm(etc2pkm_data, scale/100.0f, rotate);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw ETC2 PKM, frame %d, scale %d%%] used %dus",
                frames, scale, frame_us);

        gpu_test_display_update();

        if (scale >= 150 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 2;
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

    GPU_INFO("GPU Test [Draw ETC2 PKM, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

#endif /* !CHIP_BEST1600 */

POSSIBLY_UNUSED
static void test_draw_text(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t frame_interval_us = 1000000 / fps;
    uint32_t offset = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

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

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw Text, frame %d ] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        offset += 2;
        if (offset >= 200)
            offset = 0;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < frame_interval_us) {
            hal_sys_timer_delay_us(frame_interval_us-frame_us);
            testing_ms += frame_interval_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw Text, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static int a4_width = 96;
static int a4_height = 100;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t a4_data[] = {
#include "resource/A4_unpacked_96x100.txt"
};
POSSIBLY_UNUSED
static void test_draw_a4(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t frame_interval_us = 1000000 / fps;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xff0000ff);

        gpu_test_draw_a4_img(a4_data, a4_width, a4_height, 200, 120, 0.9f, rotate, 0xff00ff00);
        gpu_test_draw_a4_img(a4_data, a4_width, a4_height, 200, 250, 1.2f, rotate, 0xffcc0000);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw A4, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        rotate += 2;
        if (rotate >= 360)
            rotate %= 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < frame_interval_us) {
            hal_sys_timer_delay_us(frame_interval_us-frame_us);
            testing_ms += frame_interval_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw A4, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

static int a8_width = 96;
static int a8_height = 100;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t a8_data[] = {
#include "resource/A8_unpacked_96x100.txt"
};
POSSIBLY_UNUSED
static void test_draw_a8(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_end, frame_us;
    uint32_t frames = 0;
    uint32_t frame_interval_us = 1000000 / fps;
    int rotate = 0;

    do {
        frame_start = hal_fast_sys_timer_get();

        gpu_test_fill_color(0xffff0000);

        gpu_test_draw_a8_img(a8_data, a8_width, a8_height, 200, 150, 1.2f, rotate, 0xff00cccc);
        gpu_test_draw_a8_img(a8_data, a8_width, a8_height, 200, 270, 0.9f, rotate, 0xff0000ff);

        frame_end = hal_fast_sys_timer_get();
        frame_us = FAST_TICKS_TO_US(frame_end - frame_start);
        GPU_INFO("GPU Test [Draw A8, frame %d] used %dus",
                frames, frame_us);

        gpu_test_display_update();

        rotate += 2;
        if (rotate >= 360)
            rotate %= 360;

        frame_us = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start);
        if (frame_us < frame_interval_us) {
            hal_sys_timer_delay_us(frame_interval_us-frame_us);
            testing_ms += frame_interval_us / 1000;
        } else {
            testing_ms += frame_us / 1000;
        }
        frames++;
    } while (testing_ms < total_time_ms);

    GPU_INFO("GPU Test [Draw A8, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

void test_gpu(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    GPU_INFO("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

#ifdef TEST_WITH_LCDC
    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buffer);
    lcdc_test_start();

    test_width = lcdc_test_get_width();
    test_height = lcdc_test_get_height();

    if (lcdc_test_get_bpp() == 16)
        test_format = VG_LITE_RGB565;
    else
        test_format = VG_LITE_RGBA8888;
#endif

    gpu_test_open(test_width, test_height, test_format, (void *)render_buffer);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);

    while (1) {
        test_fillcolor(20000, 30);
        test_clear_rect(20000, 5);
        test_draw_line(30000, 10);
        test_draw_rect(30000, 30);
        test_draw_circle(30000, 30);
        test_draw_ring(30000, 30);
        test_blit(30000, 20);
        test_blit_rect(30000, 20);
        test_draw_tiger(180000, 50);
        test_draw_pattern_rect(30000, 30);
        test_draw_pattern_circle(30000, 30);
        test_pacman(30000, 30);
        test_draw_icon(30000, 20);
        test_draw_index8(30000, 30);
        test_draw_text(30000, 30);
        test_draw_a4(30000, 30);
        test_draw_a8(30000, 30);
#ifndef CHIP_BEST1600
        test_draw_etc2(30000, 30);
#endif
    }

    gpu_test_close();

#ifdef TEST_WITH_LCDC
    lcdc_test_stop();
    lcdc_test_exit();
#endif
}

#endif
