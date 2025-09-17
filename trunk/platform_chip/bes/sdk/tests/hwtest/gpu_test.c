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

#ifdef GPU_TEST

#include <stdio.h>
#include <string.h>
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_lcdc.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "hwtest.h"
#include "vg_lite.h"
#include "gpu_test.h"
#include "lcdc_test.h"
#include "test_colors.h"

#define GPU_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)

#define LOOP_RUN(func)  \
    for (int i = 0; i < 5; i++) { \
        func; \
    }

#ifndef TEST_WIDTH
#define TEST_WIDTH      320
#endif
#ifndef TEST_HEIGHT
#define TEST_HEIGHT     320
#endif
#define TEST_MAX_SIZE       (TEST_WIDTH*TEST_HEIGHT*4)

static int test_width = TEST_WIDTH;
static int test_height = TEST_HEIGHT;
static int test_format = VG_LITE_RGBA8888;
static int test_stride = TEST_WIDTH*4;

POSSIBLY_UNUSED
static ALIGNED(64) uint8_t sram_buf1[TEST_MAX_SIZE];
#ifdef TEST_DOUBLE_SRAM_BUFFER
POSSIBLY_UNUSED
static ALIGNED(64) uint8_t sram_buf2[TEST_MAX_SIZE];
#endif
POSSIBLY_UNUSED
static ALIGNED(64) PSRAM_NC_BSS_LOC uint8_t psram_buf1[TEST_MAX_SIZE];
POSSIBLY_UNUSED
static ALIGNED(64) PSRAM_NC_BSS_LOC uint8_t psram_buf2[TEST_MAX_SIZE];

static int pic_width = 112;
static int pic_height = 104;
static int pic_format = VG_LITE_RGBA8888;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t pic_data[] = {
#include "display_resource/RGB888_unpacked_icon1_112x104.txt"
};

static void lcdc_init(void)
{
#ifdef TEST_WITH_LCDC
    lcdc_test_init();
    lcdc_test_layer_init(0, sram_buf1);
    lcdc_test_start();

    test_width = lcdc_test_get_width();
    test_height = lcdc_test_get_height();

    if (lcdc_test_get_bpp() == 16) {
        test_format = VG_LITE_RGB565;
        test_stride = test_width * 2;
    } else {
        test_format = VG_LITE_RGBA8888;
        test_stride = test_width * 4;
    }

    if (test_stride*test_height > TEST_MAX_SIZE)
        ASSERT(0, "Use TEST_WIDTH=%d and TEST_HEIGHT=%d to define bigger size",
                test_width, test_height);
#endif
}

static void lcdc_send_frame(void *buf)
{
#ifdef TEST_WITH_LCDC
    lcdc_test_buffer_set(0, buf);
    lcdc_test_frame_xfer();
    lcdc_test_frame_wait_done();
#endif
}

POSSIBLY_UNUSED
static void lcdc_send_compress_frame(void *buf, int mode)
{
#ifdef TEST_WITH_LCDC
    lcdc_test_compress_frame_xfer(buf, mode);
    lcdc_test_frame_wait_done();
#endif
}

static void gpu_init(void)
{
    gpu_test_open(test_width, test_height, test_format, (void *)sram_buf1);
    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);
}

/* clear with color */
POSSIBLY_UNUSED
static void test_fillcolor(int frames)
{
    uint8_t *buf;
    uint32_t color;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        color = test_colors_RGBA8888[(i/10) % NCOLORS];
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(color);

        lcdc_send_frame(buf);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    };
}

/* blit, no blending */
POSSIBLY_UNUSED
static void test_blit(int frames)
{
    int x, y, offset;

    GPU_INFO("###### %s", __func__);
    gpu_test_set_blend(VG_LITE_BLEND_NONE);
    gpu_test_set_filter(VG_LITE_FILTER_POINT);

    x = (test_width - pic_width*2)/2;
    y = (test_height - pic_height*2)/2;

    if (test_format == VG_LITE_RGB565)
        offset = (y + pic_height/2) * test_stride + x * 2;
    else
        offset = (y + pic_height/2) * test_stride + x * 4;

    for (int i = 0; i < frames; i++) {
        gpu_test_set_buf(sram_buf1);
        gpu_test_fill_color(0xffffffff);

        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x, y, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x+pic_width+5, y, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x, y+pic_height+10, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x+pic_width+5, y+pic_height+10, 1.0f);

#ifdef TEST_DOUBLE_SRAM_BUFFER
        /* sram -> sram */
        gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf2, test_width, test_height, test_format,
                      1.0f, 0);
#endif
        /* sram -> psram */
        gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      psram_buf1, test_width, test_height, test_format,
                      1.0f, 0);
        /* psram -> psram */
        gpu_test_blit(psram_buf1, test_width, test_height, test_format,
                      psram_buf2, test_width, test_height, test_format,
                      1.0f, 0);
        /* psram -> sram */
        gpu_test_blit(psram_buf2, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      1.0f, 0);

        hal_cache_sync_invalidate_all(HAL_CACHE_ID_D_CACHE);

        GPU_INFO("Dump %p:", sram_buf1+offset);
        DUMP8("%02x ", sram_buf1+offset, 48);

        GPU_INFO("Dump %p:", psram_buf1+offset);
        DUMP8("%02x ", psram_buf1+offset, 48);

        GPU_INFO("Dump %p:", psram_buf2+offset);
        DUMP8("%02x ", psram_buf2+offset, 48);

        lcdc_send_frame(sram_buf1);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    };

    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);
}

/* blending, src_over */
POSSIBLY_UNUSED
static void test_blend(int frames)
{
    int x, y, offset;

    GPU_INFO("###### %s", __func__);
    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);

    x = (test_width - pic_width*2)/2;
    y = (test_height - pic_height*2)/2;

    if (test_format == VG_LITE_RGB565)
        offset = (y + pic_height/2) * test_stride + x * 2;
    else
        offset = (y + pic_height/2) * test_stride + x * 4;

    for (int i = 0; i < frames; i++) {
        if (i % 3 == 2)
            gpu_test_set_filter(VG_LITE_FILTER_POINT);
        else if (i % 3 == 1)
            gpu_test_set_filter(VG_LITE_FILTER_BI_LINEAR);
        else
            gpu_test_set_filter(VG_LITE_FILTER_LINEAR);

        gpu_test_set_buf(sram_buf1);
        gpu_test_fill_color(0xffffffff);

        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x, y, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x+pic_width+5, y, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x, y+pic_height+10, 1.0f);
        gpu_test_blit_icon(pic_data, pic_width, pic_height, pic_format,
                           x+pic_width+5, y+pic_height+10, 1.0f);

#ifdef TEST_DOUBLE_SRAM_BUFFER
        /* sram -> sram */
        gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf2, test_width, test_height, test_format,
                      1.0f, 0);
#endif
        /* sram -> psram */
        gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      psram_buf1, test_width, test_height, test_format,
                      1.0f, 0);
        /* psram -> psram */
        gpu_test_blit(psram_buf1, test_width, test_height, test_format,
                      psram_buf2, test_width, test_height, test_format,
                      1.0f, 0);
        /* psram -> sram */
        gpu_test_blit(psram_buf2, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      1.0f, 0);

        hal_cache_sync_invalidate_all(HAL_CACHE_ID_D_CACHE);

        GPU_INFO("Dump %p:", sram_buf1+offset);
        DUMP8("%02x ", sram_buf1+offset, 48);

        GPU_INFO("Dump %p:", psram_buf1+offset);
        DUMP8("%02x ", psram_buf1+offset, 48);

        GPU_INFO("Dump %p:", psram_buf2+offset);
        DUMP8("%02x ", psram_buf2+offset, 48);

        lcdc_send_frame(sram_buf1);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    };

    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);
}

/* vector drawing, operation MOVE & LINE */
POSSIBLY_UNUSED
static void test_draw_rect(int frames)
{
    uint8_t *buf;
    int step = 10;
    int w = 200;
    int h = 200;
    int x = (test_width - w) / 2;
    int y = (test_height - h) / 2;
    uint32_t rotate = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_rect(x, y, w, h, rotate, 0xff0000ff);

        lcdc_send_frame(buf);

        if (w > 400 || w < 100)
            step = -step;
        w += step;
        h += step;
        x = (test_width - w) / 2;
        y = (test_height - h) / 2;

        rotate += 6;
        if (rotate >= 360)
            rotate = rotate % 360;
    }
}

/* vector drawing, operation QUAD & CUBI & SCWARC */
POSSIBLY_UNUSED
static void test_draw_ring(int frames)
{
    uint8_t *buf;
    int cx = test_width/2;
    int cy = test_height/2;
    int r = test_width/2-20;
    int angle = 32;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_cubic_ring(cx, cy, r, 36, 30, angle, 0xff0000ff);
        gpu_test_draw_arc_ring(cx, cy, r-40, 36, 30, angle, 0xff00ff00);

        lcdc_send_frame(buf);

        angle += 2;
        if (angle > 360)
            angle %= 360;
    }
}

/* vector drawing, operation QUAD & CUBI & SCWARC */
POSSIBLY_UNUSED
static void test_draw_circle(int frames)
{
    uint8_t *buf;
    int cx = test_width/2;
    int cy = test_height/2;
    int r = test_width/2-10;
    int angle = 32;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);
        gpu_test_draw_cubic_circle(cx, cy, r, 30, angle, 0xff00ff00);
        gpu_test_draw_arc_circle(cx, cy, r-30, 30, angle, 0xff0000ff);

        lcdc_send_frame(buf);

        angle += 2;
        if (angle > 360)
            angle %= 360;
    }
}

/* vector drawing, tiger image */
POSSIBLY_UNUSED
static void test_draw_tiger(int frames)
{
    uint8_t *buf;
    uint32_t step = 10;
    int scale = 100;
    int rotate = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff0000ff);
        gpu_test_draw_tiger(test_width/2, test_height/2, (float)scale/100.0, rotate);

        lcdc_send_frame(buf);

        if (scale >= 400 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 6;
        rotate = rotate % 360;
    }
}

/* vector drawing, chinese text */
POSSIBLY_UNUSED
static void test_draw_text(int frames)
{
    uint8_t *buf;
    uint32_t offset = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);

        for (int j = 0; j < 7; j++) {
            int y = j*100 - offset;
            int w = lcdc_test_get_width();
            int h = 95;
            gpu_test_clear_rect(0, y, w, h, (j%2) ? 0xffaaaaaa : 0xffcccccc);
            gpu_test_draw_text_xian(100, y+80, 0.9f, 0, 0xff000000);
            gpu_test_draw_text_xian(200, y+80, 0.9f, 0, 0xff000000);
            gpu_test_draw_text_xian(300, y+80, 0.9f, 0, 0xff00ffff);
            gpu_test_draw_text_xian(400, y+80, 0.9f, 0, 0xff00ffff);
        }

        lcdc_send_frame(buf);

        offset += 2;
        if (offset >= 200)
            offset = 0;
    }
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

/* vector drawing, stroke path */
POSSIBLY_UNUSED
static void test_draw_line(int frames)
{
    uint8_t *buf;
    int offs_x[LINE_COUNT];
    int offs_y = 100;
    int step = 10;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < LINE_COUNT; i++)
        offs_x[i] = rand() % 100;

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);

        for (int j = 0; j < LINE_COUNT; j++) {
            if (j % 2 == 0)
                gpu_test_draw_stroke_lines(line_points[j], LINE_POINT_NUM, offs_x[j], j*20 + offs_y,
                                6, test_colors_RGBA8888[j+2]);
            else
                gpu_test_draw_rect_lines(line_points[j], LINE_POINT_NUM, offs_x[j], j*20 + offs_y,
                                6, test_colors_RGBA8888[j+2]);
        }

        lcdc_send_frame(buf);

        if (offs_y >= 400 || offs_y <= 0)
            step = -step;
        offs_y += step;
    }
}

/* draw pattern with rect or circle path */
POSSIBLY_UNUSED
static void test_draw_pattern(int frames)
{
    uint8_t *buf;
    uint32_t scale = 100;
    int step = 10;
    int rotate = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff000000);
#if 1
        gpu_test_draw_pattern_rect(pic_data, pic_width, pic_height, pic_format,
                                   buf, test_width, test_height, test_format,
                                   (float)scale/100.0f, rotate);
#else
        gpu_test_draw_pattern_circle(pic_data, pic_width, pic_height, pic_format,
                                   buf, test_width, test_height, test_format,
                                   (float)scale/100.0f);
#endif

        lcdc_send_frame(buf);

        if (scale >= 150 || scale <= 20)
            step = -step;
        scale += step;

        rotate += 2;
        if (rotate > 360)
            rotate = rotate % 360;
    }
}

static int a4_width = 96;
static int a4_height = 100;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t a4_data[] = {
#include "display_resource/A4_unpacked_96x100.txt"
};
/* draw A4 image with blit */
POSSIBLY_UNUSED
static void test_draw_a4(int frames)
{
    uint8_t *buf;
    int rotate = 0;
    int x = (test_width - a4_width)/2;
    int y = (test_height - a4_height)/2;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xff0000ff);

        gpu_test_draw_a4_img(a4_data, a4_width, a4_height, x, y-50, 0.9f, rotate, 0xff00ff00);
        gpu_test_draw_a4_img(a4_data, a4_width, a4_height, x, y+50, 1.2f, rotate, 0xffcc0000);

        lcdc_send_frame(buf);

        rotate += 2;
        if (rotate >= 360)
            rotate %= 360;
    }
}

static int a8_width = 96;
static int a8_height = 100;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t a8_data[] = {
#include "display_resource/A8_unpacked_96x100.txt"
};
/* draw A8 image with blit */
POSSIBLY_UNUSED
static void test_draw_a8(int frames)
{
    uint8_t *buf;
    int rotate = 0;
    int x = (test_width - a8_width)/2;
    int y = (test_height - a8_height)/2;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffff0000);

        gpu_test_draw_a8_img(a8_data, a8_width, a8_height, x, y-50, 1.2f, rotate, 0xff00cccc);
        gpu_test_draw_a8_img(a8_data, a8_width, a8_height, y, y+50, 0.9f, rotate, 0xff0000ff);

        lcdc_send_frame(buf);

        rotate += 2;
        if (rotate >= 360)
            rotate %= 360;
    }
}

static int index8_width = 160;
static int index8_height = 160;
static ALIGNED(64) PSRAM_DATA_LOC uint8_t index8_data[] = {
#include "display_resource/Index8_image_data_160x160.txt"
};
static uint32_t index8_colors[] = {
#include "display_resource/Index8_color_table.txt"
};
/* draw index8 image with blit */
POSSIBLY_UNUSED
static void test_draw_index8(int frames)
{
    uint8_t *buf;
    int step = 1;
    int scale = 80;
    int rotate = 0;
    int color_count = sizeof(index8_colors) / sizeof(uint32_t);

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);

        gpu_test_draw_index8(index8_data, index8_width, index8_height,
                             index8_colors, color_count, scale/100.0f, rotate);

        lcdc_send_frame(buf);

        if (scale >= 200 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 2;
        if (rotate >= 360)
            rotate = rotate % 360;
    }
}

static ALIGNED(64) PSRAM_DATA_LOC uint8_t etc2pkm_data[] = {
#include "display_resource/etc2pkm_unpacked_160x160.txt"
};
/* draw etc2 image with blit */
POSSIBLY_UNUSED
static void test_draw_etc2(int frames)
{
    uint8_t *buf;
    int step = 1;
    int scale = 80;
    int rotate = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; i < frames; i++) {
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;

        gpu_test_set_buf(buf);
        gpu_test_fill_color(0xffffffff);

        gpu_test_draw_etc2pkm(etc2pkm_data, scale/100.0f, rotate);

        lcdc_send_frame(buf);

        if (scale >= 200 || scale <= 50)
            step = -step;
        scale += step;

        rotate += 2;
        if (rotate >= 360)
            rotate = rotate % 360;
    }
}

/* decnano compress */
POSSIBLY_UNUSED
static void test_decnano(void)
{
    uint8_t *buf;
    uint32_t color;
    uint8_t comp_modes[] = { VG_LITE_DEC_NON_SAMPLE, VG_LITE_DEC_HSAMPLE, VG_LITE_DEC_HV_SAMPLE };
    uint8_t mode;
    float scale = 1.0f;
    uint32_t rotate = 0;

    GPU_INFO("###### %s", __func__);

    for (int i = 0; ; i++) {
        color = test_colors_RGBA8888[i % NCOLORS];
        mode = comp_modes[(i / NCOLORS) % 3];
        rotate = (i * 10) % 360;
        scale = 1.0f + (i % 6) * 0.1f;
        if (i % 2 == 0)
            buf = sram_buf1;
        else
            buf = psram_buf1;
        gpu_test_set_buf(buf);
        gpu_test_set_compress_mode(mode);
        gpu_test_fill_color(color);
        gpu_test_draw_tiger(test_width/2, test_height/2, scale, rotate);

        GPU_INFO("Dump compressed buffer %p:", buf);
        DUMP8("%02x ", buf, 32);
        DUMP8("%02x ", buf+32, 32);

        gpu_test_blit_decomp(psram_buf2);
        GPU_INFO("Dump uncompressed buffer %p:", psram_buf2);
        DUMP8("%02x ", psram_buf2, 32);
        DUMP8("%02x ", psram_buf2+32, 32);

        lcdc_send_compress_frame(buf, mode);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(800));
#endif
    };
}

POSSIBLY_UNUSED
static void test_perf(void)
{
    float scale = 1.0f;
    int rotate = 0;

    GPU_INFO("GPU performance test, %dx%d, format %d",
            test_width, test_height, test_format);

    GPU_INFO("###### fill color");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_fill_color(0xff0a59f7));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_fill_color(0xff0a59f7));

    GPU_INFO("###### blit");
    gpu_test_set_blend(VG_LITE_BLEND_NONE);
    gpu_test_set_filter(VG_LITE_FILTER_POINT);

    for (int i = 0; i < 6; i++) {
        if (i / 3 == 0)
            rotate = 0;
        else
            rotate = 90;
        if (i % 3 == 1)
            scale = 0.7f;
        else if (i % 3 == 2)
            scale = 2.7f;
        else
            scale = 1.0f;
        /* sram -> sram */
#ifdef TEST_DOUBLE_SRAM_BUFFER
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf2, test_width, test_height, test_format,
                      scale, rotate));
#else
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      scale, rotate));
#endif
        /* sram -> psram */
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      psram_buf1, test_width, test_height, test_format,
                      scale, rotate));
        /* psram -> psram */
        LOOP_RUN(gpu_test_blit(psram_buf1, test_width, test_height, test_format,
                      psram_buf2, test_width, test_height, test_format,
                      scale, rotate));
        /* psram -> sram */
        LOOP_RUN(gpu_test_blit(psram_buf2, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      scale, rotate));
    }

    GPU_INFO("###### blending");

    /* prepare buffers with different color */
    gpu_test_set_buf(sram_buf1);
    gpu_test_fill_color(0xc00a59f7);
#ifdef TEST_DOUBLE_SRAM_BUFFER
    gpu_test_set_buf(sram_buf2);
    gpu_test_fill_color(0x77ff0000);
#endif
    gpu_test_set_buf(psram_buf1);
    gpu_test_fill_color(0x7700ff00);
    gpu_test_set_buf(psram_buf2);
    gpu_test_fill_color(0x770000ff);

    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);

    for (int i = 0; i < 6; i++) {
        if (i / 3 == 0)
            rotate = 0;
        else
            rotate = 90;
        if (i % 3 == 1)
            scale = 0.7f;
        else if (i % 3 == 2)
            scale = 2.7f;
        else
            scale = 1.0f;
        /* sram -> sram */
#ifdef TEST_DOUBLE_SRAM_BUFFER
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf2, test_width, test_height, test_format,
                      scale, rotate));
#else
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      scale, rotate));
#endif
        /* sram -> psram */
        LOOP_RUN(gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                      psram_buf1, test_width, test_height, test_format,
                      scale, rotate));
        /* psram -> psram */
        LOOP_RUN(gpu_test_blit(psram_buf1, test_width, test_height, test_format,
                      psram_buf2, test_width, test_height, test_format,
                      scale, rotate));
        /* psram -> sram */
        LOOP_RUN(gpu_test_blit(psram_buf2, test_width, test_height, test_format,
                      sram_buf1, test_width, test_height, test_format,
                      scale, rotate));
    }

    GPU_INFO("###### draw tiger");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 0.7f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 2.7f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 90));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 0.7f, 90));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 2.7f, 90));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 0.7f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 2.7f, 0));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 90));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 0.7f, 90));
    LOOP_RUN(gpu_test_draw_tiger(test_width/2, test_height/2, 2.7f, 90));

    GPU_INFO("###### draw line");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_draw_rect_lines(line_points[0], LINE_POINT_NUM, 10, 100, 6, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_stroke_lines(line_points[0], LINE_POINT_NUM, 10, 100, 6, 0xff0a59f7));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_draw_rect_lines(line_points[0], LINE_POINT_NUM, 10, 100, 6, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_stroke_lines(line_points[0], LINE_POINT_NUM, 10, 100, 6, 0xff0a59f7));

    GPU_INFO("###### draw rect");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_draw_rect(50, 50, 200, 200, 0, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_rect(50, 50, 200, 200, 45, 0xff0a59f7));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_draw_rect(50, 50, 200, 200, 0, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_rect(50, 50, 200, 200, 45, 0xff0a59f7));

    GPU_INFO("###### draw circle");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_draw_cubic_circle(test_width/2, test_height/2, 100, 30, 390, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_arc_circle(test_width/2, test_height/2, 100, 30, 390, 0xff0a59f7));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_draw_cubic_circle(test_width/2, test_height/2, 100, 30, 390, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_arc_circle(test_width/2, test_height/2, 100, 30, 390, 0xff0a59f7));

    GPU_INFO("###### draw ring");
    gpu_test_set_buf(sram_buf1);
    LOOP_RUN(gpu_test_draw_cubic_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_arc_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7));
    gpu_test_set_buf(psram_buf1);
    LOOP_RUN(gpu_test_draw_cubic_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7));
    LOOP_RUN(gpu_test_draw_arc_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7));
}

POSSIBLY_UNUSED
static void test_basic(void)
{
    float scale = 1.0f;
    int rotate = 0;

    GPU_INFO("GPU basic test, %dx%d, format %d",
            test_width, test_height, test_format);

    GPU_INFO("###### fillcolor & blit/blend");

    /* prepare buffers with different color */
    gpu_test_set_buf(sram_buf1);
    gpu_test_fill_color(0xc00a59f7);
    gpu_test_set_buf(psram_buf1);
    gpu_test_fill_color(0x7700ff00);
    gpu_test_set_buf(psram_buf2);
    gpu_test_fill_color(0x770000ff);

    gpu_test_set_blend(VG_LITE_BLEND_SRC_OVER);
    gpu_test_set_filter(VG_LITE_FILTER_LINEAR);

    /* sram -> psram */
    gpu_test_blit(sram_buf1, test_width, test_height, test_format,
                  psram_buf1, test_width, test_height, test_format,
                  scale, rotate);
    /* psram -> psram */
    gpu_test_blit(psram_buf1, test_width, test_height, test_format,
                  psram_buf2, test_width, test_height, test_format,
                  scale, rotate);
    /* psram -> sram */
    gpu_test_blit(psram_buf2, test_width, test_height, test_format,
                  sram_buf1, test_width, test_height, test_format,
                  scale, rotate);

    GPU_INFO("###### draw tiger");
    gpu_test_set_buf(sram_buf1);
    gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 0);
    gpu_test_set_buf(psram_buf1);
    gpu_test_draw_tiger(test_width/2, test_height/2, 1.0f, 0);

    GPU_INFO("###### draw rect");
    gpu_test_set_buf(sram_buf1);
    gpu_test_draw_rect(50, 50, 200, 200, 0, 0xff0a59f7);
    gpu_test_set_buf(psram_buf1);
    gpu_test_draw_rect(50, 50, 200, 200, 0, 0xff0a59f7);

    GPU_INFO("###### draw ring");
    gpu_test_set_buf(sram_buf1);
    gpu_test_draw_cubic_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7);
    gpu_test_draw_arc_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7);
    gpu_test_set_buf(psram_buf1);
    gpu_test_draw_cubic_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7);
    gpu_test_draw_arc_ring(test_width/2, test_height/2, 100, 40, 30, 390, 0xff0a59f7);
}

void gpu_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    GPU_INFO("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    GPU_INFO("GPU test.");

    lcdc_init();
    gpu_init();

#ifdef DECNANO_TEST
    test_decnano();
    while (1);
#endif

#ifdef BASIC_TEST
    test_basic();
    while (1);
#endif

#ifdef PERF_TEST
    test_perf();
    while (1);
#endif

    while (1) {
        test_fillcolor(100);
        test_blit(100);
        test_blend(100);
        test_draw_rect(120);
        test_draw_circle(360);
        test_draw_ring(360);
        test_draw_tiger(300);
        test_draw_text(200);
        test_draw_line(200);
        test_draw_pattern(100);
        test_draw_a4(100);
        test_draw_a8(100);
        test_draw_index8(100);
        test_draw_etc2(100);
    }
}

#endif /* GPU_TEST */
