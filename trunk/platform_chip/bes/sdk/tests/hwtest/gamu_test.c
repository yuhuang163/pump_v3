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

#if defined(GAMU_TEST)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
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
#include "hal_gamu.h"
#include "display_test.h"

#define TEST_WIDTH          320
#define TEST_HEIGHT         320

#if (TEST_HEIGHT > 512 || TEST_WIDTH > 512)
#error "GAMU buffer max size is 512x512"
#endif

#define GAMU_SHAPE_CIRCLE
//#define GAMU_SHAPE_TRIANGLE
//#define GAMU_SHAPE_RECTANGLE

#ifdef TEST_RGB565
#define GAMU_FMT            GAMU_RGB16
#define GAMU_BLK_BYTES      16
#define GAMU_PIX_BYTES      2
#define GAMU_BLK_PIXELS     8
#else
#define GAMU_FMT            GAMU_RGB32
#define GAMU_BLK_BYTES      16
#define GAMU_PIX_BYTES      4
#define GAMU_BLK_PIXELS     4
#endif
#define GAMU_FAKE_DATA      0

#ifdef GAMU_SHAPE_TRIANGLE
#define PBUF_SIZE           (TEST_WIDTH * TEST_HEIGHT * GAMU_PIX_BYTES * 55 / 100)
#elif defined(GAMU_SHAPE_CIRCLE)
#define PBUF_SIZE           (TEST_WIDTH * TEST_HEIGHT * GAMU_PIX_BYTES * 81 / 100)
#else
#define PBUF_SIZE           (TEST_WIDTH * TEST_HEIGHT * GAMU_PIX_BYTES)
#endif

static ALIGNED(64) uint8_t pbuf0[PBUF_SIZE];
static ALIGNED(64) uint8_t pbuf1[PBUF_SIZE];

static struct GAMU_LINE_CONFIG_T lines_cfg[TEST_HEIGHT];
POSSIBLY_UNUSED static int visible_pixels = 0;

POSSIBLY_UNUSED
static void gamu_init_buf_shape(void)
{
#ifdef GAMU_SHAPE_CIRCLE
    /* Circle Shape */
    uint16_t center_x = TEST_WIDTH / 2;
    uint16_t center_y = TEST_HEIGHT / 2;
    uint16_t radius = (TEST_WIDTH > TEST_HEIGHT) ? center_y : center_x;
    for (int y = 0; y < TEST_HEIGHT; y++) {
        int d = (int)sqrtf(radius * radius - (y - center_y) * (y - center_y));
        lines_cfg[y].pixel_start = center_x - d;
        lines_cfg[y].pixel_end = center_x + d - 1;

        if (lines_cfg[y].pixel_end >= lines_cfg[y].pixel_start)
            visible_pixels += (lines_cfg[y].pixel_end - lines_cfg[y].pixel_start) + 1;
    }
#elif defined(GAMU_SHAPE_TRIANGLE)
    /* Triangle Shape:
     * *
     * **
     * ***
     * ...
     */
    for (int y = 0; y < TEST_HEIGHT; y++) {
        lines_cfg[y].pixel_start = 0;
        lines_cfg[y].pixel_end = y + 1;
        //lines_cfg[y].pixel_start = TEST_WIDTH - 1 - y;
        //lines_cfg[y].pixel_end = TEST_WIDTH - 1;

        if (lines_cfg[y].pixel_end >= lines_cfg[y].pixel_start)
            visible_pixels += (lines_cfg[y].pixel_end - lines_cfg[y].pixel_start) + 1;
    }
#else
    /* Rectangle Shape */
    for (int y = 0; y < TEST_HEIGHT; y++) {
        lines_cfg[y].pixel_start = 0;
        lines_cfg[y].pixel_end = TEST_WIDTH - 1;

        if (lines_cfg[y].pixel_end >= lines_cfg[y].pixel_start)
            visible_pixels += (lines_cfg[y].pixel_end - lines_cfg[y].pixel_start) + 1;
    }
#endif
}

POSSIBLY_UNUSED
static uint32_t gamu_get_buf_paddr(uint32_t pbase, uint16_t x, uint16_t y)
{
    uint16_t blk_offs = 0;
    uint8_t blk_start;
    uint8_t blk_end;
    uint8_t blk_xoffs;

    if (y >= TEST_HEIGHT || x >= TEST_WIDTH)
        return 0;

    blk_start = lines_cfg[y].pixel_start / GAMU_BLK_PIXELS;
    blk_end = lines_cfg[y].pixel_end / GAMU_BLK_PIXELS;
    blk_xoffs = x / GAMU_BLK_PIXELS;

    if (blk_end < blk_start) {
        blk_start = 0;
        blk_end = 0;
    }

    if (blk_xoffs < blk_start || blk_xoffs > blk_end)
        return 0;

    for (int i = 0; i < y; i++) {
        if (lines_cfg[i].pixel_end > lines_cfg[i].pixel_start)
            blk_offs += (lines_cfg[i].pixel_end / GAMU_BLK_PIXELS - lines_cfg[i].pixel_start / GAMU_BLK_PIXELS) + 1;
        else
            blk_offs++;
    }

    blk_offs += blk_xoffs - blk_start;
    return (pbase + blk_offs * GAMU_BLK_BYTES + (x % GAMU_BLK_PIXELS) * GAMU_PIX_BYTES);
}

POSSIBLY_UNUSED
static void gamu_dump_vbuf(void *vbuf, int x, int y)
{
    int w = hal_gamu_buf_width();
    int offs = (y * w + x) * GAMU_PIX_BYTES;
    vbuf += offs;
    TRACE(0, "Dump vbuf %p, x %d, y %d:", vbuf, x, y);
#ifdef TEST_RGB565
    DUMP16("%04x ", vbuf, 24);
    DUMP16("%04x ", vbuf + (24 * 2), 24);
#else
    DUMP32("%08x ", vbuf, 16);
    DUMP32("%08x ", vbuf + (16 * 4), 16);
#endif
}

static void gamu_init(void)
{
    hal_gamu_open(GAMU_FMT);

    gamu_init_buf_shape();

    if (hal_gamu_config_shape(lines_cfg, TEST_HEIGHT))
        ASSERT(0, "Failed to config GAMU shape.");
    if (hal_gamu_config_buf(0, (uint32_t)pbuf0, PBUF_SIZE))
        ASSERT(0, "Failed to config GAMU buffer0.");
    if (hal_gamu_config_buf(1, (uint32_t)pbuf1, PBUF_SIZE))
        ASSERT(0, "Failed to config GAMU buffer1.");

    hal_gamu_set_fake_rdata(GAMU_FAKE_DATA);

#if defined(CHIP_BEST1503) || defined(CHIP_BEST1502X)
    /* Configure LUT twice to workaround a hardware issue */
    hal_gamu_config_shape(lines_cfg, TEST_HEIGHT);
#endif
}

#ifdef TEST_WITH_LCDC
static void lcdc_init(int vbuf_width)
{
    lcdc_test_init();
#ifdef TEST_RGB565
    lcdc_test_pitch_set(0, vbuf_width * 2);
#else
    lcdc_test_pitch_set(0, vbuf_width * 4);
#endif
    lcdc_test_layer_init(0, pbuf0);
    lcdc_test_start();
}

static void lcdc_send_frame(void *buf)
{
    lcdc_test_buffer_set(0, buf);
    lcdc_test_frame_xfer();
    lcdc_test_frame_wait_done();
}
#endif

void gamu_test(void)
{
    int vbuf_w;
    uint8_t *vbuf0;
    uint8_t *vbuf1;

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    TRACE(0, "gamu test");

    gamu_init();

    vbuf_w = hal_gamu_buf_width();

    vbuf0 = (void *)hal_gamu_buf_vaddr(0, 0, 0);
    vbuf1 = (void *)hal_gamu_buf_vaddr(1, 0, 0);

#ifdef TEST_WITH_LCDC
    disp_buffer_t dispbuf;

    dma2d_test_init();
    lcdc_init(vbuf_w);

    dispbuf.width = vbuf_w;
    dispbuf.height = TEST_HEIGHT;
#ifdef TEST_RGB565
    dispbuf.format = FB_FMT_RGB16_565;
#else
    dispbuf.format = FB_FMT_RGB32;
#endif

    for (int i = 0; ; i++) {
#ifdef TEST_RGB565
        uint16_t color = test_colors_RGB565[(i/10) % NCOLORS];
#else
        uint32_t color = test_colors_RGBA8888[(i/10) % NCOLORS];
#endif
        if (i % 2 == 1)
            dispbuf.data = vbuf1;
        else
            dispbuf.data = vbuf0;

        TRACE(0, "############## TEST %d, vbuf %p", i, dispbuf.data);

        TRACE(0, "fill color 0x%08x to buffer %p", color, dispbuf.data);
        dma2d_test_fill_color(&dispbuf, NULL, color);

        lcdc_send_frame(dispbuf.data);
#ifndef SIMU
        hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    }

#else /* TEST_WITH_CPU */
    for (int i = 0; ; i++) {
        int errors = 0;
        int pixels = 0;
        uint8_t *vbuf, *pbuf;

        if (i % 2 == 1) {
            vbuf = vbuf1;
            pbuf = pbuf1;
        } else {
            vbuf = vbuf0;
            pbuf = pbuf0;
        }
        TRACE(0, "############## TEST %d, vbuf %p, pbuf %p", i, vbuf, pbuf);

        TRACE(0, "Clear GAMU buffer, addr %p", vbuf);
        memset(vbuf, 0, vbuf_w * TEST_HEIGHT * GAMU_PIX_BYTES);

        TRACE(0, "Check GAMU buffer...");
        for (int y = 0; y < TEST_HEIGHT; y++) {
#ifdef TEST_RGB565
            uint16_t *row = (uint16_t *)vbuf + (y * vbuf_w);
#else
            uint32_t *row = (uint32_t *)vbuf + (y * vbuf_w);
#endif
            for (int x = 0; x < TEST_WIDTH; x++) {
#ifdef TEST_RGB565
                uint16_t *paddr = (uint16_t *)gamu_get_buf_paddr((uint32_t)pbuf, x, y);
#else
                uint32_t *paddr = (uint32_t *)gamu_get_buf_paddr((uint32_t)pbuf, x, y);
#endif
                if (!paddr && row[x] != GAMU_FAKE_DATA) {
                    TRACE(0, "ERROR: vaddr %p = %08x, expect %d",
                            &row[x], row[x], GAMU_FAKE_DATA);
                    errors++;
                } else if (paddr && (row[x] != *paddr || row[x] != 0)) {
                    TRACE(0, "ERROR: vaddr %p = %08x, paddr %p = %08x, expect 0",
                            &row[x], row[x], paddr, *paddr);
                    errors++;
                }
                if (paddr)
                    pixels++;
            }
        }

        if (errors > 0)
            ASSERT(0, "Check failed, %d errors!", errors);
        if (pixels < visible_pixels)
            ASSERT(0, "Check %d pixels, at least %d pixels", pixels, visible_pixels);

        TRACE(0, "Check OK!");
        gamu_dump_vbuf(vbuf, 0, TEST_HEIGHT/2);

        TRACE(0, "Write data to GAMU buffer, addr %p.", vbuf);
        for (int y = 0; y < TEST_HEIGHT; y++) {
#ifdef TEST_RGB565
            uint16_t *row = (uint16_t *)vbuf + (y * vbuf_w);
            for (int x = 0; x < TEST_WIDTH; x++)
                row[x] = ((y << 8) & 0xff) | (x & 0xff);
#else
            uint32_t *row = (uint32_t *)vbuf + (y * vbuf_w);
            for (int x = 0; x < TEST_WIDTH; x++)
                row[x] = 0xff000000 | y << 12 | x;
#endif
        }

        errors = 0;
        pixels = 0;
        TRACE(0, "Check GAMU buffer...");

        for (int y = 0; y < TEST_HEIGHT; y++) {
#ifdef TEST_RGB565
            uint16_t *row = (uint16_t *)vbuf + (y * vbuf_w);
#else
            uint32_t *row = (uint32_t *)vbuf + (y * vbuf_w);
#endif
            for (int x = 0; x < TEST_WIDTH; x++) {
#ifdef TEST_RGB565
                uint16_t val = ((y << 8) & 0xff) | (x & 0xff);
                uint16_t *paddr = (uint16_t *)gamu_get_buf_paddr((uint32_t)pbuf, x, y);
#else
                uint32_t val = 0xff000000 | y << 12 | x;
                uint32_t *paddr = (uint32_t *)gamu_get_buf_paddr((uint32_t)pbuf, x, y);
#endif
                if (!paddr && row[x] != GAMU_FAKE_DATA) {
                    TRACE(0, "ERROR: vaddr %p = %08x, expect %d",
                            &row[x], row[x], GAMU_FAKE_DATA);
                    errors++;
                } else if (paddr && (row[x] != *paddr || row[x] != val)) {
                    TRACE(0, "ERROR: [%d,%d], vaddr %p = %08x, paddr %p = %08x, expect %08x",
                            x, y, &row[x], row[x], paddr, *paddr, val);
                    errors++;
                }
                if (paddr)
                    pixels++;
            }
        }

        if (errors > 0)
            ASSERT(0, "Check failed, %d errors!", errors);
        if (pixels < visible_pixels)
            ASSERT(0, "Check %d pixels, at least %d pixels", pixels, visible_pixels);

        TRACE(0, "Check OK!");
        gamu_dump_vbuf(vbuf, 0, TEST_HEIGHT/2);
    }
#endif
    while (1);
}

#endif /* GAMU_TEST */
