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

#if defined(DMA2D_TEST)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_cmu.h"
#include "hal_psc.h"
#include "hal_dma2d.h"

#include "display_test.h"

#define TEST_WIDTH      160
#define TEST_HEIGHT     160

#define PIC_WIDTH   112
#define PIC_HEIGHT  104
#define PIC_FORMAT  FB_FMT_RGB32
POSSIBLY_UNUSED
static ALIGNED(64) uint8_t pic_data[] = {
#include "display_resource/RGB888_unpacked_icon1_112x104.txt"
};

static uint8_t sram_buf1[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64);
static uint8_t sram_buf2[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64);
static uint8_t sram_buf3[TEST_WIDTH*TEST_HEIGHT*4] ALIGNED(64);

POSSIBLY_UNUSED
static int check_color(uint8_t *buf, uint32_t color)
{
    int err = 0;
    uint32_t *p = (uint32_t *)buf;
    color = color & 0xffffff;
    for (int i = 0; i < TEST_HEIGHT * TEST_WIDTH; i++) {
        if (p[i] != color) {
            if (err < 16)
                DISP_ERR("addr %p = 0x%x, expect 0x%x", &p[i], p[i], color);
            err++;
        }
    }
    return err;
}

POSSIBLY_UNUSED
static void test_fillcolor(int times)
{
    uint32_t color;
    disp_buffer_t buf = {
        .width = TEST_WIDTH,
        .height = TEST_HEIGHT,
        .format = FB_FMT_RGB32,
        .data = sram_buf1,
    };

    for (int i = 0; i < times; i++) {
        color = test_colors_RGBA8888[i % NCOLORS];

        dma2d_test_fill_color(&buf, NULL, color);
        if (check_color(sram_buf1, color))
            ASSERT(0, "DMA2D fillcolor test failed!");
    }
}

POSSIBLY_UNUSED
static void test_blit(int times)
{
    disp_buffer_t pic, b1, b2;
    int bpp = 4;

    b1.format = FB_FMT_RGB32;
    b1.width = TEST_WIDTH;
    b1.height = TEST_HEIGHT;
    memcpy(&b2, &b1, sizeof(b1));

    /* Prepare source buffer */
    pic.width = PIC_WIDTH;
    pic.height = PIC_HEIGHT;
    pic.format = PIC_FORMAT;
    pic.data = pic_data;
    b1.data = sram_buf1;
    disp_sw_blit(&pic, NULL, &b1, NULL);

    for (int i = 0; i < times; i++) {
        memset(sram_buf2, 0, sizeof(sram_buf2));

        /* Do buffer blit */
        b1.data = sram_buf1;
        b2.data = sram_buf2;
        dma2d_test_blit(&b1, NULL, &b2, NULL);

        /* Check data in buffers */
        if (memcmp(b1.data, b2.data, TEST_WIDTH*TEST_HEIGHT*bpp))
            ASSERT(0, "DMA2D blit test failed!");
    }
}

POSSIBLY_UNUSED
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

POSSIBLY_UNUSED
static int compare_blend_buffer(uint8_t *buf, uint8_t *swbuf)
{
    int errs = 0;
    uint32_t *p1 = (uint32_t *)buf;
    uint32_t *p2 = (uint32_t *)swbuf;

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

    b1.format = FB_FMT_RGB32;
    b1.width = TEST_WIDTH;
    b1.height = TEST_HEIGHT;
    memcpy(&b2, &b1, sizeof(b1));

    /* prepare source buffer */
    pic.width = PIC_WIDTH;
    pic.height = PIC_HEIGHT;
    pic.format = PIC_FORMAT;
    pic.data = pic_data;
    b1.data = sram_buf1;
    disp_sw_blit(&pic, NULL, &b1, NULL);

    for (int i = 0; i < times; i++) {
        uint8_t opa = (25 - i%25) * 10;
        color = test_colors_RGBA8888[i % NCOLORS];

        /* DMA2D blending */
        memset(sram_buf2, 0, sizeof(sram_buf2));
        b2.data = sram_buf2;
        disp_sw_fill_color(&b2, NULL, color); /* background color */
        dma2d_test_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);

        /* CPU blending */
        memset(sram_buf3, 0, sizeof(sram_buf3));
        b2.data = sram_buf3;
        disp_sw_fill_color(&b2, NULL, color); /* background color */
        disp_sw_blend(&b1, NULL, &b2, NULL, FB_CONST_ALPHA, opa);

        /* Compare DMA2D/CPU blending buffers */
        if (compare_blend_buffer(sram_buf2, sram_buf3))
            ASSERT(0, "DMA2D blending test failed.");
    }
}

void dma2d_test(void)
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
}

#endif /* DMA2D_TEST */
