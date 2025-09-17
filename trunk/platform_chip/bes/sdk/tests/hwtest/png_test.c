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

#if defined(PNG_TEST)

#include "stdint.h"
#include "string.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_png.h"

//#define TEST_DUMP

#define MAX_W  500
#define MAX_H  500

POSSIBLY_UNUSED
static const char *COLORTYPE_STR[] = {
    "GRAY", "", "RGB888", "INDEX", "GRAY+ALPHA", "", "RGBA8888",
};

POSSIBLY_UNUSED
static const char *PIXFMT_STR[] = {
    "RGBA8888", "ARGB8888", "RGB888", "RGB565", "RGBA565", "ARGB565",
};

static PSRAM_NC_BSS_LOC uint8_t ALIGNED(64) pix_buf[MAX_W*MAX_H*4];
static PSRAM_DATA_LOC uint8_t ALIGNED(64) png_data[] = {
#include "pngdec/png_500x500_24bpp.txt"
};

void png_test(void)
{
    uint32_t start, end;
    struct HAL_PNG_CONFIG_T cfg;

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    hal_png_open();

    for (int i = 0; i < 10; i++) {
        TRACE(0, "########### PNG DECODE TEST %02d", i);

        memset(&cfg, 0, sizeof(cfg));
        cfg.png_data = png_data;
        cfg.png_size = sizeof(png_data);
        cfg.pix_buf = pix_buf;

        if (i % 3 == 0)
            cfg.pix_fmt = HAL_PNG_PIX_RGBA8888;
        else if (i % 3 == 1)
            cfg.pix_fmt = HAL_PNG_PIX_RGB888;
        else
            cfg.pix_fmt = HAL_PNG_PIX_RGB565;

        TRACE(0, "Decode Config:");
        TRACE(0, "  PNG file data: %p", cfg.png_data);
        TRACE(0, "  PNG file size: %u", cfg.png_size);
        TRACE(0, "  Output buffer: %p", cfg.pix_buf);
        TRACE(0, "  Output format: %s (%d)", PIXFMT_STR[cfg.pix_fmt], cfg.pix_fmt);

        start = hal_fast_sys_timer_get();
        if (hal_png_start(&cfg) != HAL_PNG_OK) {
            TRACE(0, "Decode start failed. %d", cfg.result);
            continue;
        }

        while (hal_png_busy()) {
            hal_sys_timer_delay(US_TO_TICKS(10));
            if (FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - start) > 200)
                break;
        }

        end = hal_fast_sys_timer_get();

        TRACE(0, "Image Info:");
        TRACE(0, "  Width:          %u", cfg.img_width);
        TRACE(0, "  Height:         %u", cfg.img_height);
        TRACE(0, "  Color type:     %s (%d)",
                    (cfg.img_color_type <= 6) ? COLORTYPE_STR[cfg.img_color_type] : "NA",
                    cfg.img_color_type);
        TRACE(0, "  Color depth:    0x%x", cfg.img_color_depth);
        TRACE(0, "  Compress mode:  0x%x", cfg.img_comp_mode);
        TRACE(0, "  Compress level: 0x%x", cfg.img_comp_level);

        hal_png_stop();

        if (hal_png_busy()) {
            ASSERT(0, "Decode timeout! used %uus", FAST_TICKS_TO_US(end - start));
        } else if (cfg.result != HAL_PNG_OK) {
            ASSERT(0, "Decode failed, result = %d.", cfg.result);
        } else {
            TRACE(0, "Decode done! used %uus.", FAST_TICKS_TO_US(end - start));
        }

#ifdef TEST_DUMP
        if (cfg.result == HAL_PNG_OK) {
            for (int i = 0; i < cfg.img_width*cfg.img_height*4; i += 32) {
                DUMP8("%02x ", (void *)pix_buf+i, 32);
                hal_sys_timer_delay_us(10000);
            }
        }
        while (1);
#endif
    }

    hal_png_close();
    while (1);
}

#endif /* PNG_TEST */
