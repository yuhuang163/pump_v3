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

#if defined(TEST_DMA2D)

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_psc.h"
#include "hal_dma2d.h"
#include "hal_location.h"

#include "display_test.h"

#if 1

#define MAX_BUF_SIZE    (480*480*4)

POSSIBLY_UNUSED
static uint32_t frame_buf[MAX_BUF_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;

static uint32_t lcdc_width = 0;
static uint32_t lcdc_height = 0;
static uint8_t lcdc_bpp = 32;

POSSIBLY_UNUSED
static void test_fillcolor(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint32_t color = 0xffffffff;
    disp_buffer_t buf = {
        .width = lcdc_width,
        .height = lcdc_height,
        .format = FB_FMT_RGB32,
        .data = (uint8_t *)frame_buf,
    };

    if (lcdc_bpp == 16)
        buf.format = FB_FMT_RGB16_565;

    lcdc_test_start();

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        if (lcdc_bpp == 32) {
            color = test_colors_RGBA8888[(frames / fps) % NCOLORS];
        } else {
            color = test_colors_RGB565[(frames / fps) % NCOLORS];
        }

        dma2d_test_fill_color(&buf, NULL, color);

        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dms, dynamic fps %u",
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

    DISP_LOG("DMA2D fill color test, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

/* Picture RGB888 */
#define PIC_WIDTH   368
#define PIC_HEIGHT  448
#define PIC_STRIDE  (PIC_WIDTH*4)
static PSRAM_DATA_LOC ALIGNED(64) uint8_t pic_data_rgb32[] = {
#include "resource/RGB888_unpacked_368x448.txt"
};

POSSIBLY_UNUSED
static void test_blit(uint32_t total_time_ms, uint32_t fps)
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
    src.format = FB_FMT_RGB32;
    src.data = pic_data_rgb32;

    src_area1.x = 0;
    src_area1.y = 0;
    src_area1.w = PIC_WIDTH;
    src_area1.h = PIC_HEIGHT;

    src_area2.x = 0;
    src_area2.y = 0;
    src_area2.w = PIC_WIDTH;
    src_area2.h = PIC_HEIGHT;

    dst.width = lcdc_width;
    dst.height = lcdc_height;
    dst.format = FB_FMT_RGB32;
    dst.data = (uint8_t *)frame_buf;

    dst_area1.x = (lcdc_width-PIC_WIDTH)/2;
    dst_area1.y = 0;
    dst_area1.w = PIC_WIDTH;
    dst_area1.h = PIC_HEIGHT;

    dst_area2.x = (lcdc_width-PIC_WIDTH)/2;
    dst_area2.y = PIC_HEIGHT;
    dst_area2.w = PIC_WIDTH;
    dst_area2.h = lcdc_height-PIC_HEIGHT;

    lcdc_test_start();

    disp_sw_fill_color(&dst, NULL, 0xffffffff);

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        src_area1.y += ystep;
        dst_area1.h -= ystep;
        dma2d_test_blit(&src, &src_area1, &dst, &dst_area1);

        dst_area2.y -= ystep;
        dst_area2.h += ystep;
        dma2d_test_blit(&src, &src_area2, &dst, &dst_area2);

        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();

        if (dst_area1.h <= 2) {
            src_area1.y = 0;
            dst_area1.h = PIC_HEIGHT;
            dst_area2.y = PIC_HEIGHT;
            dst_area2.h = lcdc_height-PIC_HEIGHT;
        }

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dms, dynamic fps %u",
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

    DISP_LOG("DMA2D blit test, total %d frames, %ufps]",
                    frames, (frames * 1000) / (testing_ms));
}

POSSIBLY_UNUSED
static void test_blend(uint32_t total_time_ms, uint32_t fps)
{
    uint32_t testing_ms = 0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    uint32_t frame_interval_ms = 1000 / fps;
    uint8_t opacity = 20;
    int opa_step = 1;
    int ystep = 2;
    disp_buffer_t src, dst;
    disp_area_t src_area1;
    disp_area_t src_area2;
    disp_area_t dst_area1;
    disp_area_t dst_area2;

    src.width = PIC_WIDTH;
    src.height = PIC_HEIGHT;
    src.format = FB_FMT_RGB32;
    src.data = pic_data_rgb32;

    src_area1.x = 0;
    src_area1.y = 0;
    src_area1.w = PIC_WIDTH;
    src_area1.h = PIC_HEIGHT;

    src_area2.x = 0;
    src_area2.y = 0;
    src_area2.w = PIC_WIDTH;
    src_area2.h = PIC_HEIGHT;

    dst.width = lcdc_width;
    dst.height = lcdc_height;
    dst.format = FB_FMT_RGB32;
    dst.data = (uint8_t *)frame_buf;

    dst_area1.x = (lcdc_width-PIC_WIDTH)/2;
    dst_area1.y = 0;
    dst_area1.w = PIC_WIDTH;
    dst_area1.h = PIC_HEIGHT;

    dst_area2.x = (lcdc_width-PIC_WIDTH)/2;
    dst_area2.y = PIC_HEIGHT;
    dst_area2.w = PIC_WIDTH;
    dst_area2.h = lcdc_height-PIC_HEIGHT;

    lcdc_test_start();

    do {
        DISP_LOG("----------------------------------------------------------------------");
        DISP_LOG("#### Frame %u start.", frames);
        frame_start = hal_sys_timer_get();

        disp_sw_fill_color(&dst, NULL, 0xff0000ff);

        src_area1.y += ystep;
        dst_area1.h -= ystep;
        dma2d_test_blend(&src, &src_area1, &dst, &dst_area1, FB_CONST_ALPHA, opacity);

        dst_area2.y -= ystep;
        dst_area2.h += ystep;
        dma2d_test_blend(&src, &src_area2, &dst, &dst_area2, FB_CONST_ALPHA, opacity);

        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();

        if (dst_area1.h <= 2) {
            src_area1.y = 0;
            dst_area1.h = PIC_HEIGHT;
            dst_area2.y = PIC_HEIGHT;
            dst_area2.h = lcdc_height-PIC_HEIGHT;
        }

        opacity += opa_step;
        if (opacity >= 240 || opacity <= 10)
            opa_step = -opa_step;

        frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
        DISP_LOG("#### Frame %u used %dms, dynamic fps %u",
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

    DISP_LOG("DMA2D blend test, total %d frames, %ufps",
                    frames, (frames * 1000) / (testing_ms));
}

void test_dma2d(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    DISP_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf);

    osDelay(500);

    lcdc_width = lcdc_test_get_width();
    lcdc_height = lcdc_test_get_height();
    lcdc_bpp = lcdc_test_get_bpp();

    dma2d_test_init();

    while (1) {
        test_fillcolor(30000, 30);
        test_blit(60000, 30);
        test_blend(120000, 30);
    }
}

#else

#include "stdlib.h"

#define TEST_WIDTH      360
#define TEST_HEIGHT     360

#ifdef TEST_RGB565
#define PIC_WIDTH   360
#define PIC_HEIGHT  360
static PSRAM_NC_DATA_LOC ALIGNED(64) uint8_t pic_data[] = {
#include "resource/RGB565_unpacked_360x360.txt"
};
#else
#define PIC_WIDTH   368
#define PIC_HEIGHT  448
static PSRAM_NC_DATA_LOC ALIGNED(64) uint8_t pic_data[] = {
#include "resource/RGB888_unpacked_368x448.txt"
};
#endif

static uint8_t pic_buf[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64) PSRAM_NC_BSS_LOC;

static uint8_t sram_buf1[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64) SRAM_BSS_LOC;
static uint8_t sram_buf2[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64) SRAM_BSS_LOC;

static uint8_t psram_buf1[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64) PSRAM_NC_BSS_LOC;
static uint8_t psram_buf2[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64) PSRAM_NC_BSS_LOC;

POSSIBLY_UNUSED
static int check_color(uint8_t *buf, uint32_t color)
{
    int err = 0;
#ifdef TEST_RGB565
    uint16_t *p = (uint16_t *)buf;
    color = color & 0xffff;
#else
    uint32_t *p = (uint32_t *)buf;
    color = color & 0xffffff;
#endif
    for (int i = 0; i < TEST_HEIGHT * TEST_WIDTH; i++) {
        if (p[i] != color) {
            DISP_ERR("addr %p = 0x%x, expect 0x%x", &p[i], p[i], color);
            err++;
        }
    }
    return err;
}

POSSIBLY_UNUSED
static void test_fillcolor(int times)
{
    uint32_t color = 0xffffffff;
    disp_buffer_t buf = {
        .width = TEST_WIDTH,
        .height = TEST_HEIGHT,
#ifdef TEST_RGB565
        .format = FB_FMT_RGB16_565,
#else
        .format = FB_FMT_RGB32,
#endif
        .data = NULL,
    };

    for (int i = 0; i < times; i++) {
#ifdef TEST_RGB565
        color = test_colors_RGB565[i % NCOLORS];
#else
        color = test_colors_RGBA8888[i % NCOLORS];
#endif
        /* -> sram */
        buf.data = sram_buf1;
        dma2d_test_fill_color(&buf, NULL, color);
        if (check_color(sram_buf1, color))
            ASSERT(0, "DMA2D fillcolor test failed!");

        /* -> psram */
        buf.data = psram_buf1;
        dma2d_test_fill_color(&buf, NULL, color);
        if (check_color(psram_buf1, color))
            ASSERT(0, "DMA2D fillcolor test failed!");
    }
}

POSSIBLY_UNUSED
static void test_blit(int times)
{
    disp_buffer_t b1, b2;
#ifdef TEST_RGB565
    int bpp = 2;
    b1.format = FB_FMT_RGB16_565;
    b2.format = FB_FMT_RGB16_565;
#else
    int bpp = 4;
    b1.format = FB_FMT_RGB32;
    b2.format = FB_FMT_RGB32;
#endif

    b1.width = PIC_WIDTH;
    b1.height = PIC_HEIGHT;
    b1.data = pic_data;
    b2.width = TEST_WIDTH;
    b2.height = TEST_HEIGHT;
    b2.data = pic_buf;
    disp_sw_blit(&b1, NULL, &b2, NULL);

    b1.width = TEST_WIDTH;
    b1.height = TEST_HEIGHT;

    for (int i = 0; i < times; i++) {
        /* psram -> sram */
        b1.data = pic_buf;
        b2.data = sram_buf1;
        dma2d_test_blit(&b1, NULL, &b2, NULL);

        /* sram -> sram */
        b1.data = sram_buf2;
        dma2d_test_blit(&b2, NULL, &b1, NULL);
        if (memcmp(b1.data, b2.data, TEST_WIDTH*TEST_HEIGHT*bpp))
            ASSERT(0, "DMA2D blit test failed!");

        /* sram -> psram */
        b1.data = psram_buf1;
        dma2d_test_blit(&b2, NULL, &b1, NULL);
        if (memcmp(b1.data, b2.data, TEST_WIDTH*TEST_HEIGHT*bpp))
            ASSERT(0, "DMA2D blit test failed!");

        /* psram -> psram */
        b2.data = psram_buf2;
        dma2d_test_blit(&b1, NULL, &b2, NULL);
        if (memcmp(b1.data, b2.data, TEST_WIDTH*TEST_HEIGHT*bpp))
            ASSERT(0, "DMA2D blit test failed!");
    }
}

static bool is_same_color(uint32_t c1, uint32_t c2)
{
    int r1 = c1 & 0xff;
    int g1 = (c1 >> 8) & 0xff;
    int b1 = (c1 >> 16) & 0xff;
    int r2 = c2 & 0xff;
    int g2 = (c2 >> 8) & 0xff;
    int b2 = (c2 >> 16) & 0xff;
    if (abs(r1-r2) <= 2 && abs(g1-g2) <= 2 && abs(b1-b2) <= 2)
        return true;
    return false;
}

static int compare_blend_buffer(uint8_t *buf, uint8_t *swbuf)
{
    int errs = 0;
#ifdef TEST_RGB565
    uint16_t *p1 = (uint16_t *)buf;
    uint16_t *p2 = (uint16_t *)swbuf;
#else
    uint32_t *p1 = (uint32_t *)buf;
    uint32_t *p2 = (uint32_t *)swbuf;
#endif
    DISP_LOG("Compare DMA2D blend results with CPU...");
    for (int i = 0; i < TEST_WIDTH*TEST_HEIGHT; i++) {
        if (!is_same_color(p1[i], p2[i])) {
            if (errs < 32) {
                DISP_LOG("ERROR: %6d, DMA2D got 0x%x, CPU got 0x%x", i, p1[i], p2[i]);
                osDelay(1);
            }
            errs++;
        }
    }
    if (errs == 0)
        DISP_LOG("DMA2D blending test OKAY!");
    else
        DISP_LOG("DMA2D blending test FAILED, %d errors.", errs);
    TRACE_FLUSH();
    return errs;
}

POSSIBLY_UNUSED
static void test_blend(int times)
{
    uint32_t color;
    disp_buffer_t pic, b1, b2;
#ifdef TEST_RGB565
    pic.format = FB_FMT_RGB16_565;
    b1.format = FB_FMT_RGB16_565;
    b2.format = FB_FMT_RGB16_565;
#else
    pic.format = FB_FMT_RGB32;
    b1.format = FB_FMT_RGB32;
    b2.format = FB_FMT_RGB32;
#endif

    b1.width = PIC_WIDTH;
    b1.height = PIC_HEIGHT;
    b1.data = pic_data;
    pic.width = TEST_WIDTH;
    pic.height = TEST_HEIGHT;
    pic.data = pic_buf;
    disp_sw_blit(&b1, NULL, &pic, NULL);

    b1.width = TEST_WIDTH;
    b1.height = TEST_HEIGHT;
    b2.width = TEST_WIDTH;
    b2.height = TEST_HEIGHT;

    for (int i = 0; i < times; i++) {
        uint8_t opa = (25 - i%25) * 10;
#ifdef TEST_RGB565
        color = test_colors_RGB565[i % NCOLORS];
#else
        color = test_colors_RGBA8888[i % NCOLORS];
#endif

        /* psram -> sram */
        memset(sram_buf1, 0, sizeof(sram_buf1));
        memset(sram_buf2, 0, sizeof(sram_buf2));
        memset(psram_buf1, 0, sizeof(psram_buf1));
        memset(psram_buf2, 0, sizeof(psram_buf2));
        b1.data = psram_buf1;
        dma2d_test_blit(&pic, NULL, &b1, NULL); /* source picture */
        b2.data = sram_buf1;
        dma2d_test_fill_color(&b2, NULL, color); /* background color */
        dma2d_test_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        /* do sw blending, compare with DMA2d */
        b2.data = sram_buf2;
        disp_sw_fill_color(&b2, NULL, color);
        disp_sw_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        compare_blend_buffer(sram_buf1, sram_buf2);

        /* sram -> sram */
        memset(sram_buf1, 0, sizeof(sram_buf1));
        memset(sram_buf2, 0, sizeof(sram_buf2));
        memset(psram_buf1, 0, sizeof(psram_buf1));
        memset(psram_buf2, 0, sizeof(psram_buf2));
        b1.data = sram_buf1;
        dma2d_test_blit(&pic, NULL, &b1, NULL); /* source picture */
        b2.data = sram_buf2;
        dma2d_test_fill_color(&b2, NULL, color); /* background color */
        dma2d_test_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        /* do sw blending, compare with DMA2d */
        b2.data = psram_buf2;
        disp_sw_fill_color(&b2, NULL, color);
        disp_sw_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        compare_blend_buffer(sram_buf2, psram_buf2);

        /* sram -> psram */
        memset(sram_buf1, 0, sizeof(sram_buf1));
        memset(sram_buf2, 0, sizeof(sram_buf2));
        memset(psram_buf1, 0, sizeof(psram_buf1));
        memset(psram_buf2, 0, sizeof(psram_buf2));
        b1.data = sram_buf1;
        dma2d_test_blit(&pic, NULL, &b1, NULL); /* source picture */
        b2.data = psram_buf1;
        dma2d_test_fill_color(&b2, NULL, color); /* background color */
        dma2d_test_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        /* do sw blending, compare with DMA2d */
        b2.data = psram_buf2;
        disp_sw_fill_color(&b2, NULL, color);
        disp_sw_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        compare_blend_buffer(psram_buf1, psram_buf2);

        /* psram -> psram */
        memset(sram_buf1, 0, sizeof(sram_buf1));
        memset(sram_buf2, 0, sizeof(sram_buf2));
        memset(psram_buf1, 0, sizeof(psram_buf1));
        memset(psram_buf2, 0, sizeof(psram_buf2));
        b1.data = psram_buf1;
        dma2d_test_blit(&pic, NULL, &b1, NULL); /* source picture */
        b2.data = psram_buf2;
        dma2d_test_fill_color(&b2, NULL, color); /* background color */
        dma2d_test_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        /* do sw blending, compare with DMA2d */
        b2.data = sram_buf2;
        disp_sw_fill_color(&b2, NULL, color);
        disp_sw_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);
        compare_blend_buffer(psram_buf2, sram_buf2);
    }
}

void test_dma2d(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    DISP_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    DISP_LOG("DMA2D blending test start.");

    dma2d_test_init();

    while (1) {
        test_fillcolor(10);
        test_blit(10);
        test_blend(25);
    }
    while (1);
}

#endif

#endif
