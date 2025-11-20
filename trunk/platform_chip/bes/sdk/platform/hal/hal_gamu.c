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

/**
 * DOC: GAMU is a graphical memory address map unit:

• Reduce memory usage to the circal displayer
• Physically store only visible pixel
• Up to 4 virtual buffers
• Each virtual buffer have 2048 bytes per line and 512 lines
• Each virtual buffer can be physically mapped to any system memory
*/

#include "plat_addr_map.h"

#ifdef GAMU_BASE

#include "string.h"
#include "reg_gamu.h"
#include "hal_gamu.h"
#include "hal_trace.h"
#include "hal_cmu.h"

//#define GAMU_DEBUG
#ifdef  GAMU_DEBUG
#define GAMU_INFO(...)      TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define GAMU_INFO(...)
#endif

#define GAMU_ERR(...)       TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#ifdef GAMU_VMEM_NC_BASE
#define GAMU_BUF_BASE       GAMU_VMEM_NC_BASE
#else
#define GAMU_BUF_BASE       GAMU_VMEM_BASE
#endif

#define GAMU_BUF_CNT        4

#define GAMU_BUF_WIDTH      512
#define GAMU_BUF_HEIGHT     512

static struct GAMU_REG_T *gamu = (struct GAMU_REG_T *)GAMU_BASE;

struct GAMU_CONFIG_T {
    bool inited;
    enum GAMU_FORMAT_E format;

    uint8_t blk_bytes;   /* bytes in one buffer block */
    uint8_t blk_pixels;  /* pixels in one buffer block */

    uint16_t active_lines;       /* active lines */
    uint16_t active_blks;        /* acitve virtual blocks */
    uint16_t active_vblks;       /* acitve visible blocks */

    struct {
        uint32_t size;
        uint32_t phys;
        uint32_t virt;
    } buffer[GAMU_BUF_CNT];
};

static struct GAMU_CONFIG_T gamu_cfg;

static uint32_t gamu_fake_wbuf[16];

int hal_gamu_open(enum GAMU_FORMAT_E fmt)
{
    if (gamu_cfg.inited)
        return 0;

    if (fmt != GAMU_RGB32 && fmt != GAMU_RGB16 && fmt != GAMU_RGB24) {
        GAMU_ERR("%s: invalid format %d", __func__, fmt);
        return -1;
    }

    memset(&gamu_cfg, 0, sizeof(gamu_cfg));

    if (fmt == GAMU_RGB16) {
        gamu_cfg.blk_bytes = 16;
        gamu_cfg.blk_pixels = 8;
    } else if (fmt == GAMU_RGB24) {
        gamu_cfg.blk_bytes = 12;
        gamu_cfg.blk_pixels = 4;
    } else {
        gamu_cfg.blk_bytes = 16;
        gamu_cfg.blk_pixels = 4;
    }

    hal_cmu_gamu_clock_enable();

    gamu->REG_810 = GAMU_RGB_INPUT_FORMAT(fmt) | GAMU_VVM_EN | GAMU_CROSS_1K_EN;
    gamu->REG_828 = GAMU_RGB_INPUT_FORMAT_WR(fmt) | GAMU_VVM_EN_WR;

    gamu->REG_814 = ADDR_CPU_TO_DEV((uint32_t)gamu_fake_wbuf);
    gamu->REG_830 = ADDR_CPU_TO_DEV((uint32_t)gamu_fake_wbuf);

    gamu_cfg.format = fmt;
    gamu_cfg.inited = true;
    GAMU_INFO("GAMU enabled, buffer max size [%dx%dx%d].", GAMU_BUF_WIDTH, GAMU_BUF_HEIGHT,
              (fmt == GAMU_RGB16) ? 16 : (fmt == GAMU_RGB24 ? 24 : 32));
    return 0;
}

void hal_gamu_close(void)
{
    if (!gamu_cfg.inited)
        return;

    memset(&gamu_cfg, 0, sizeof(gamu_cfg));

    gamu->REG_810 = 0;
    gamu->REG_828 = 0;

    hal_cmu_gamu_clock_disable();

    GAMU_INFO("GAMU disabled.");
}

void hal_gamu_sleep(void)
{
    hal_cmu_gamu_sleep();
}

void hal_gamu_wakeup(void)
{
    hal_cmu_gamu_wakeup();
}

int hal_gamu_config_shape(struct GAMU_LINE_CONFIG_T *cfg, int count)
{
    uint16_t blk_offset = 0;  /* offset of the first visible block in this line */
    uint8_t line_blks = 0;

    if (!cfg)
        return -1;

    if (!gamu_cfg.inited) {
        GAMU_ERR("%s: GAMU is not initialized!", __func__);
        return -1;
    }

    if (count < 4 || count > GAMU_BUF_HEIGHT) {
        GAMU_ERR("%s: line count need be 4 ~ %d", __func__, GAMU_BUF_HEIGHT);
        return -1;
    }

    line_blks = GAMU_BUF_WIDTH / gamu_cfg.blk_pixels;

    gamu_cfg.active_lines = count;
    gamu_cfg.active_blks = line_blks * count;
    gamu_cfg.active_vblks = 0;

    for (int i = 0; i < count; i++) {
        uint8_t blk_start = 0;
        uint8_t blk_end = 0;

        if (cfg[i].pixel_end > cfg[i].pixel_start) {
            blk_start = cfg[i].pixel_start / gamu_cfg.blk_pixels;
            blk_end = cfg[i].pixel_end / gamu_cfg.blk_pixels;

            if (blk_start >= line_blks) {
                blk_start = 0;
                blk_end = 0;
            }

            if (blk_end >= line_blks)
                blk_end = line_blks - 1;
        }

        blk_offset = gamu_cfg.active_vblks - blk_start;
        gamu->REG_LUT[i] = GAMU_LUT_BLOCK_START(blk_start) | GAMU_LUT_BLOCK_END(blk_end) |
                           GAMU_LUT_BLOCK_OFFSET(blk_offset) | GAMU_LUT_LINE_ACTIVE;
        GAMU_INFO("#### GAMU LUT %3d: blk_start %d, blk_end %d, blk_offset %u",
                    i, blk_start, blk_end, blk_offset);
        gamu_cfg.active_vblks += blk_end - blk_start + 1;
    }
    return 0;
}

int hal_gamu_config_buf(uint8_t id, uint32_t paddr, uint32_t psize)
{
    uint32_t need_size;

    if (!gamu_cfg.inited) {
        GAMU_ERR("%s: GAMU is not initialized!", __func__);
        return -1;
    }

    if (gamu_cfg.active_lines == 0) {
        GAMU_ERR("%s: GAMU shape is not configured!", __func__);
        return -1;
    }

    if (id >= GAMU_BUF_CNT || !paddr) {
        GAMU_ERR("%s: invalid buffer id %d or address 0x%08x.", __func__, id, paddr);
        return -1;
    }

    need_size = gamu_cfg.active_vblks * gamu_cfg.blk_bytes;
    if (psize < need_size) {
        GAMU_ERR("%s: physical buffer size is not enough, need %d bytes.",
                __func__, need_size);
        return -1;
    }

    gamu->REG_PADDR[id] = ADDR_CPU_TO_DEV(paddr);
    gamu->REG_PADDR_WR[id] = ADDR_CPU_TO_DEV(paddr);

    gamu_cfg.buffer[id].size = psize;
    gamu_cfg.buffer[id].phys = paddr;
    if (gamu_cfg.format == GAMU_RGB16)
        gamu_cfg.buffer[id].virt = GAMU_BUF_BASE | (id << 19);
    else
        gamu_cfg.buffer[id].virt = GAMU_BUF_BASE | (id << 20);

    GAMU_INFO("GAMU buffer %d enabled, phys 0x%08x, virt 0x%08x", id,
                gamu_cfg.buffer[id].phys, gamu_cfg.buffer[id].virt);
    return 0;
}

uint16_t hal_gamu_buf_width(void)
{
    return GAMU_BUF_WIDTH;
}

uint16_t hal_gamu_buf_height(void)
{
    return gamu_cfg.active_lines;
}

uint32_t hal_gamu_buf_vaddr(uint8_t id, uint16_t x, uint16_t y)
{
    uint32_t offs;

    if (id >= GAMU_BUF_CNT)
        return 0;
    if (y >= gamu_cfg.active_lines)
        return 0;
    if (!gamu_cfg.buffer[id].virt)
        return 0;

    if (gamu_cfg.format == GAMU_RGB16)
        offs =  (y * hal_gamu_buf_width() + x) * 2;
    else if (gamu_cfg.format == GAMU_RGB24)
        offs =  (y * hal_gamu_buf_width() + x) * 3;
    else
        offs =  (y * hal_gamu_buf_width() + x) * 4;

    return (gamu_cfg.buffer[id].virt + offs);
}

void hal_gamu_set_fake_rdata(uint32_t val)
{
    gamu->REG_82C = val;
}

#endif
