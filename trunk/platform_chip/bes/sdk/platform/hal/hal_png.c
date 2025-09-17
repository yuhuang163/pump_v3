/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
/*
 * png-decoder
 * features:
 *
 *    ______________           _______________________
 *   |              |         |                       |
 *   |png file data |-------->| png_buffer            |
 *   |______________|         |                       |
 *                            |        decoder        |
 *                            |                       |
 *                   -------->| start                 |---->IDAT_ST_irq
 *                            |                       |
 *                   -------->| dma trigger lines     |
 *                   -------->| reset if     pix dma  |---->png_done_irq
 *                   -------->| clk                   |     ______________
 *                            |_______________________|----|              |
 *                            |_______________________|    | pixes buffers|
 *                                                         |______________|
 *
 * usecase:
 * 1. start up
 *    hclk/pixclk enable/reset,
 *    pixs output buffer start address
 *    set png file data buffer address
 *    enable IDAT_ST/png_done irq
 *    start decoding
 * 2. shutdown/stop
 * 3. enter/exit low power mode
 *
 */
#include "cmsis.h"
#include "plat_addr_map.h"

#ifdef PNG_BASE
#include "string.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_png.h"
#include "hal_png.h"

//#define PNG_DEBUG
#ifdef  PNG_DEBUG
#define PNG_INFO(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define PNG_INFO(...)
#endif

#define MAX_WIDTH   0x1ff
#define MAX_HEIGHT  0x1ff

static struct PNG_DEC_REG_T * const png = (struct PNG_DEC_REG_T *)PNG_BASE;

static bool png_working = false;
static struct HAL_PNG_CONFIG_T *png_cfg = NULL;

static void hal_png_irq_handler(void)
{
    uint32_t v = png->REG_18;
    if (v & PNG_DEC_IRQ_TRIG_LINE) {
        PNG_INFO("PNG TRIG LINE!");
    }
    if (v & PNG_DEC_IRQ_DECODE_DONE) {
        PNG_INFO("PNG DECOE DONE!");
        if (png_cfg) {
            if (png_cfg->done_cb)
                png_cfg->done_cb();
            png_cfg->result = HAL_PNG_OK;
        }
        png_working = false;
    }
    if (v & PNG_DEC_IRQ_DMA_WR_ERR) {
        PNG_INFO("PNG DMA WR ERROR");
        if (png_cfg)
            png_cfg->result = HAL_PNG_DMA_WR_ERR;
        png_working = false;
    }
    if (v & PNG_DEC_IRQ_COMP_MODE_ERR) {
        PNG_INFO("PNG COMP MODE ERROR!");
        if (png_cfg)
            png_cfg->result = HAL_PNG_COMP_MODE_ERR;
        png_working = false;
    }
    if (v & PNG_DEC_IRQ_DECODE_ERR) {
        PNG_INFO("PNG DECODE ERROR!");
        if (png_cfg)
            png_cfg->result = HAL_PNG_DECODE_ERR;
        png_working = false;
    }
}

static void hal_png_irq_enable(void)
{
    NVIC_SetVector(PNG_IRQn, (uint32_t)hal_png_irq_handler);
    NVIC_SetPriority(PNG_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(PNG_IRQn);
    NVIC_EnableIRQ(PNG_IRQn);

    png->REG_14 = PNG_DEC_IRQ_DECODE_DONE | PNG_DEC_IRQ_DMA_WR_ERR |
                  PNG_DEC_IRQ_COMP_MODE_ERR | PNG_DEC_IRQ_DECODE_ERR;
}

static void hal_png_irq_disable(void)
{
    png->REG_14 = 0;

    NVIC_DisableIRQ(PNG_IRQn);
}

void hal_png_open(void)
{
    hal_cmu_png_clock_enable(200);
}

void hal_png_close(void)
{
    hal_cmu_png_clock_disable();
}

#define PNG_GET32(p, n)  ((p[n] << 24) | (p[n + 1] << 16) | (p[n + 2] << 8) | p[n + 3])

static int hal_png_parse(struct HAL_PNG_CONFIG_T *cfg)
{
    const uint8_t png_hdr[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    uint8_t *data = cfg->png_data;
    uint8_t *p;

    if (memcmp(data, png_hdr, sizeof(png_hdr)) != 0)
        return HAL_PNG_ERR;

    p = data + 8;
    while (p - data < cfg->png_size) {
        uint32_t chunk_len = PNG_GET32(p, 0);
        if (p[4] == 'I' && p[5] == 'H' && p[6] == 'D' && p[7] == 'R') {
            /* IHDR, parse image info */
            cfg->img_width = PNG_GET32(p, 8);
            cfg->img_height = PNG_GET32(p, 12);
            cfg->img_color_depth = p[16];
            cfg->img_color_type = p[17];
            cfg->img_comp_mode = p[18];
        } else if (p[4] == 'I' && p[5] == 'D' && p[6] == 'A' && p[7] == 'T') {
            /* IDAT, get zlib compress level */
            if (p[8] == 0x78 && p[9] == 0x01) {
                if (p[10] & 0x4)
                    cfg->img_comp_level = 1;
                else
                    cfg->img_comp_level = 0;
            } else if (p[8] == 0x78 && p[9] == 0x5E) {
                cfg->img_comp_level = 1;
            } else if (p[8] == 0x78 && p[9] == 0x9C) {
                cfg->img_comp_level = 2;
            } else if (p[8] == 0x78 && p[9] == 0xDA) {
                cfg->img_comp_level = 3;
            } else {
                cfg->img_comp_level = 0xff;
            }
            break;
        }
        p += 12 + chunk_len;
    }
    return HAL_PNG_OK;
}

int hal_png_start(struct HAL_PNG_CONFIG_T *cfg)
{
    int ret;
    uint32_t val;

    if (png_working)
        return HAL_PNG_ERR;

    if (!cfg)
        return HAL_PNG_ERR;
    if (!cfg->png_data || !cfg->pix_buf || cfg->png_size == 0)
        return HAL_PNG_ERR;

    ret = hal_png_parse(cfg);
    if (ret != HAL_PNG_OK)
        goto failed;

    if (cfg->img_comp_level > 0) {
        PNG_INFO("Not support dynamic huffman compressed stream.");
        ret = HAL_PNG_NOT_SUPPORT;
        goto failed;
    }
    if (cfg->img_width == 0 || cfg->img_width > MAX_WIDTH) {
        PNG_INFO("Invalid image width: %u, must in 1 ~ %u", cfg->img_width, MAX_WIDTH);
        ret = HAL_PNG_ERR;
        goto failed;
    }
    if (cfg->img_height == 0 || cfg->img_height > MAX_HEIGHT) {
        PNG_INFO("Invalid image height: %u, must in 1 ~ %u", cfg->img_height, MAX_HEIGHT);
        ret = HAL_PNG_ERR;
        goto failed;
    }
    if (cfg->img_color_type != HAL_PNG_COLOR_RGBA8888 &&
        cfg->img_color_type != HAL_PNG_COLOR_RGB888) {
        PNG_INFO("Invalid color type: %u, only support RGB888(2) & RGBA8888(6)",
                    cfg->img_color_type);
        ret = HAL_PNG_NOT_SUPPORT;
        goto failed;
    }

    png_working = true;
    png_cfg = cfg;

    /* Reset to clear status */
    hal_cmu_reset_set(HAL_CMU_MOD_H_DIS_PNG);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_DIS_PNG);

    /* Source data */
    png->REG_28 = cfg->png_size;
    png->REG_04 = ADDR_CPU_TO_DEV((uint32_t)cfg->png_data);

    /* Start to decode */
    png->REG_00 = PNG_DEC_BURST_0TO3(0) | PNG_DEC_START;

    /* Set image width/height manually */
    png->REG_2C = PNG_DEC_PIX_SIZE_SET | PNG_DEC_PIX_WIDTH(cfg->img_width) |
                  PNG_DEC_PIX_HEIGHT(cfg->img_height);

#if 0
    uint32_t start = hal_sys_timer_get();
    while (TICKS_TO_US(hal_sys_timer_get() - start) < 200) {
        if (png->REG_18 & PNG_DEC_IRQ_IHDR_READ_DONE)
            break;
    }
    if (!(png->REG_18 & PNG_DEC_IRQ_IHDR_READ_DONE)) {
        cfg->result = HAL_PNG_ERR;
        goto failed;
    }

    /* Get image info from REG_20 */
    val = png->REG_20;
    cfg->img_width = (val & PNG_DEC_IMG_WIDTH_MASK) >> PNG_DEC_IMG_WIDTH_SHIFT;
    cfg->img_height = (val & PNG_DEC_IMG_HEIGHT_MASK) >> PNG_DEC_IMG_HEIGHT_SHIFT;
    cfg->img_color_type = (val & PNG_DEC_IMG_COLOR_TYPE_MASK) >> PNG_DEC_IMG_COLOR_TYPE_SHIFT;
    cfg->img_color_depth = (val & PNG_DEC_IMG_COLOR_DEPTH_MASK) >> PNG_DEC_IMG_COLOR_DEPTH_SHIFT;
    cfg->img_comp_mode = (val & PNG_DEC_IMG_COMP_MODE_MASK) >> PNG_DEC_IMG_COMP_MODE_SHIFT;

    png->REG_2C = 0; /* Decoder use image width/height in REG_20 */
#endif

    png->REG_10 = PNG_DEC_AWQOS(4) | PNG_DEC_ARQOS(4) | PNG_DEC_MAX_REQ_NUM(4) |
                  PNG_DEC_RST_N_PWR;
    if (cfg->global_alpha > 0)
        png->REG_10 |= PNG_DEC_GLOBAL_ALPHA_EN | PNG_DEC_GLOBAL_ALPHA(cfg->global_alpha);

    png->REG_24 = PNG_DEC_TRIG_LINES(cfg->img_height);
    png->REG_0C = ADDR_CPU_TO_DEV((uint32_t)cfg->pix_buf);

    /* Enable IRQ */
    hal_png_irq_enable();

    /* Select pixel format and start decode */
    switch (cfg->pix_fmt) {
        case HAL_PNG_PIX_RGB565:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 2) | PNG_DEC_DMA_PIX_FMT(0) |
                  PNG_DEC_DMA_PIX_RGB565;
            break;
        case HAL_PNG_PIX_RGBA565:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 3) | PNG_DEC_DMA_PIX_FMT(1) |
                  PNG_DEC_DMA_PIX_RGBA565;
            break;
        case HAL_PNG_PIX_ARGB565:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 3) | PNG_DEC_DMA_PIX_FMT(1) |
                  PNG_DEC_DMA_PIX_ARGB565;
            break;
        case HAL_PNG_PIX_RGB888:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 3) | PNG_DEC_DMA_PIX_FMT(1);
            break;
        case HAL_PNG_PIX_ARGB8888:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 4) | PNG_DEC_DMA_PIX_FMT(2);
            break;
        case HAL_PNG_PIX_RGBA8888:
        default:
            val = PNG_DEC_DMA_IMG_PITCH(cfg->img_width * 4) | PNG_DEC_DMA_PIX_FMT(3);
            break;
    }
    png->REG_08 = val | PNG_DEC_DMA_BURST_LEN(4) | PNG_DEC_DMA_ENA;
    return HAL_PNG_OK;

failed:
    png_working = false;
    cfg->result = ret;
    return ret;
}

void hal_png_stop(void)
{
    hal_png_irq_disable();

    png->REG_08 = 0;
    png->REG_00 = 0;

    png_cfg = NULL;
    png_working = false;
}

bool hal_png_busy(void)
{
    return png_working;
}

#endif /* PNG_BASE */
