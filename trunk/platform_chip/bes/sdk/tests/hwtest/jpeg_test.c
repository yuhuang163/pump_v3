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

#if defined(JPEG_TEST)

#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_cache.h"
#include "string.h"
#include "stdint.h"
#include "hal_jpeg.h"
#include "hal_location.h"
#include "jpegutils.h"

/* Test config */
//#define TEST_ENC
//#define TEST_DUMP
//#define TEST_CHECK_PIX
//#define TEST_YUV444
//#define TEST_YUV420

#define TEST_WIDTH      128
#define TEST_HEIGHT     128
#define PIX_BUF_SIZE    (TEST_WIDTH * TEST_HEIGHT * 2)
#define ECS_BUF_SIZE    (PIX_BUF_SIZE / 4)
#define JFIF_BUF_SIZE   (ECS_BUF_SIZE + 1024)

static ALIGNED(16) uint8_t pix_buf[PIX_BUF_SIZE];
static ALIGNED(16) uint8_t ecs_buf[ECS_BUF_SIZE];

POSSIBLY_UNUSED static uint8_t jfif_buf[JFIF_BUF_SIZE];

POSSIBLY_UNUSED static int hbase_buf[64];
POSSIBLY_UNUSED static int hsymb_buf[336];
POSSIBLY_UNUSED static int hmin_buf[64];
POSSIBLY_UNUSED static int qtable_buf[256];

#if (TEST_WIDTH == 480 && TEST_HEIGHT == 480)
POSSIBLY_UNUSED
static uint8_t yuv422_jpeg[] = {
#include "jpegdec/jpeg_yuv422_480x480.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv444_jpeg[] = {
#include "jpegdec/jpeg_yuv444_480x480.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv420_jpeg[] = {
#include "jpegdec/jpeg_yuv420_480x480.txt"
};
#else /* default test size is 128x128 */
POSSIBLY_UNUSED
static uint8_t yuv422_jpeg[] = {
#include "jpegdec/jpeg_yuv422_128x128.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv444_jpeg[] = {
#include "jpegdec/jpeg_yuv444_128x128.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv420_jpeg[] = {
#include "jpegdec/jpeg_yuv420_128x128.txt"
};
#endif

POSSIBLY_UNUSED
static uint8_t yuv422_pix[] = {
#include "jpegdec/yuv422/pix.txt"
};
POSSIBLY_UNUSED
static int yuv422_qetable[] = {
#include "jpegdec/yuv422/qetable.txt"
};
POSSIBLY_UNUSED
static int yuv422_hetable[] = {
#include "jpegdec/yuv422/hetable.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv444_pix[] = {
#include "jpegdec/yuv444/pix.txt"
};
POSSIBLY_UNUSED
static uint8_t yuv420_pix[] = {
#include "jpegdec/yuv420/pix.txt"
};

POSSIBLY_UNUSED
static const char yuvstr[3][8] = {
    "YUV422", "YUV420", "YUV444",
};

POSSIBLY_UNUSED
static struct jpeg_info_t jpeg_info;

POSSIBLY_UNUSED
static int jpeg_extract(struct HAL_JPEG_CONFIG_T *cfg, uint8_t *jpeg_data, int jpeg_sz)
{
    uint32_t start = hal_fast_sys_timer_get();
    int *q;

    if (!cfg || !jpeg_data || jpeg_sz <= 0)
        return -1;

    memset(&jpeg_info, 0, sizeof(jpeg_info));
    jpeg_info.ecs_data = cfg->ecs;

    if (jpeg_load_jfif_file(&jpeg_info, jpeg_data, jpeg_sz)) {
        TRACE(0, "Failed to load jfif file data.");
        return -1;
    }

    cfg->width = jpeg_info.width;
    cfg->height = jpeg_info.height;
    if (jpeg_info.format == JPEG_YUV420) {
        cfg->format = HAL_JPEG_YUV420;
        cfg->pix_size = cfg->width * cfg->height * 3 / 2;
    } else if (jpeg_info.format == JPEG_YUV444) {
        cfg->format = HAL_JPEG_YUV444;
        cfg->pix_size = cfg->width * cfg->height * 3;
    } else {
        cfg->format = HAL_JPEG_YUV422;
        cfg->pix_size = cfg->width * cfg->height * 2;
    }
    cfg->ecs_size = jpeg_info.ecs_size;

    jpeg_convert_htable_to_hdtables(jpeg_info.htables, cfg->hbase, cfg->hmin, cfg->hsymb);
    cfg->hbase_size = 64 * 4;
    cfg->hmin_size = 16 * 4;
    cfg->hsymb_size = 336 * 4;

    q = (int *)cfg->qtable;
    for (int i = 0; i < 128; i++) {
        q[i] = jpeg_info.qtables[i];
    }
    cfg->qtable_size = 128 * 4;
    TRACE(0, "JPEG extract [%dx%d, %s] used %uus.",
            cfg->width, cfg->height, yuvstr[cfg->format],
            FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));
    return 0;
}

POSSIBLY_UNUSED
static int jpeg_merge(struct HAL_JPEG_CONFIG_T *cfg, uint8_t *jpeg_data, int *jpeg_sz)
{
    int ret;
    uint32_t start = hal_fast_sys_timer_get();

    if (!cfg || !jpeg_data || !jpeg_sz)
        return -1;

    memset(&jpeg_info, 0, sizeof(jpeg_info));

    jpeg_info.width = cfg->width;
    jpeg_info.height = cfg->height;
    if (cfg->format == HAL_JPEG_YUV420)
        jpeg_info.format = JPEG_YUV420;
    else if (cfg->format == HAL_JPEG_YUV444)
        jpeg_info.format = JPEG_YUV420;
    else
        jpeg_info.format = JPEG_YUV422;

    jpeg_info.ecs_data = (uint8_t *)cfg->ecs;
    jpeg_info.ecs_size = cfg->ecs_size;

    jpeg_convert_qetable_to_qtable(cfg->qtable, jpeg_info.qtables);
    jpeg_convert_hetable_to_htable(cfg->hetable, jpeg_info.htables);

    ret = jpeg_save_jfif_file(&jpeg_info, jpeg_data, jpeg_sz);
    TRACE(0, "JPEG merge [%dx%d, %s] used %uus.",
            cfg->width, cfg->height, yuvstr[cfg->format],
            FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));
    return ret;
}

POSSIBLY_UNUSED
static void yuv422_packed_to_planar(uint8_t *srcbuf, uint8_t *dstbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;
    int cnt = w * h;

    /* Copy Y data */
    p = srcbuf + 1;
    for (int i = 0; i < cnt; i++) {
        *q++ = *p;
        p += 2;
    }

    /* Copy U data */
    p = srcbuf + 2;
    for (int i = 0; i < cnt/2; i++) {
        *q++ = *p;
        p += 4;
    }

    /* Copy V data */
    p = srcbuf;
    for (int i = 0; i < cnt/2; i++) {
        *q++ = *p;
        p += 4;
    }
}

POSSIBLY_UNUSED
static void yuv422_packed_to_planar_dump(uint8_t *srcbuf, int w, int h)
{
    uint8_t dstbuf[16];
    uint8_t *p = srcbuf;
    uint8_t *q = dstbuf;
    int cnt = w * h;

    /* Dump Y data */
    p = srcbuf + 1;
    for (int i = 0; i < cnt; i++) {
        *q++ = *p;
        p += 2;
        if (q - dstbuf >= 16) {
            DUMP8("0x%02x, ", dstbuf, 16);
            hal_sys_timer_delay_us(1000);
            q = dstbuf;
        }
    }

    /* Dump U data */
    p = srcbuf + 2;
    q = dstbuf;
    for (int i = 0; i < cnt/2; i++) {
        *q++ = *p;
        p += 4;
        if (q - dstbuf >= 16) {
            DUMP8("0x%02x, ", dstbuf, 16);
            hal_sys_timer_delay_us(1000);
            q = dstbuf;
        }
    }

    /* Dump V data */
    p = srcbuf;
    q = dstbuf;
    for (int i = 0; i < cnt/2; i++) {
        *q++ = *p;
        p += 4;
        if (q - dstbuf >= 16) {
            DUMP8("0x%02x, ", dstbuf, 16);
            hal_sys_timer_delay_us(1000);
            q = dstbuf;
        }
    }
}

POSSIBLY_UNUSED
static void yuv422_packed_dump(uint8_t *srcbuf, int w, int h)
{
    uint8_t *p = srcbuf;
    for (int i = 0; i < w * h * 2; i += 16) {
        DUMP8("0x%02x, ", p, 16);
        p += 16;
        hal_sys_timer_delay(MS_TO_TICKS(2));
    }
}

POSSIBLY_UNUSED
static void jpeg_dec_test(void)
{
    uint32_t start;
    struct HAL_JPEG_CONFIG_T cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_JPEG_DEC_MODE;
    cfg.ecs = ecs_buf;
    cfg.pix = pix_buf;
    cfg.qtable = qtable_buf;
    cfg.hbase = hbase_buf;
    cfg.hmin = hmin_buf;
    cfg.hsymb = hsymb_buf;
    cfg.swap_uv = false;
    cfg.swap_yu = false;

    for (int i = 0; i < 30; i++) {
        uint32_t jpeg_sz;
        uint8_t *jpeg_data;
        POSSIBLY_UNUSED uint8_t *pix_data;
        switch (i % 3) {
#ifdef TEST_YUV444
            case 2:
                jpeg_sz = sizeof(yuv444_jpeg);
                jpeg_data = yuv444_jpeg;
                pix_data = yuv444_pix;
                break;
#endif
#ifdef TEST_YUV420
            case 1:
                jpeg_sz = sizeof(yuv420_jpeg);
                jpeg_data = yuv420_jpeg;
                pix_data = yuv420_pix;
                break;
#endif
            default:
                jpeg_sz = sizeof(yuv422_jpeg);
                jpeg_data = yuv422_jpeg;
                pix_data = yuv422_pix;
                break;
        }

        memset(hmin_buf, 0, sizeof(hmin_buf));
        memset(hbase_buf, 0, sizeof(hbase_buf));
        memset(hsymb_buf, 0, sizeof(hsymb_buf));
        memset(qtable_buf, 0, sizeof(qtable_buf));
        memset(ecs_buf, 0, sizeof(ecs_buf));
        memset(pix_buf, 0, sizeof(pix_buf));

        if (jpeg_extract(&cfg, jpeg_data, jpeg_sz)) {
            TRACE(0, "Failed to extract JPEG file.");
            while (1);
        }

        hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

        start = hal_fast_sys_timer_get();

        if (hal_jpeg_start(&cfg)) {
            TRACE(0, "Failed to start jpeg codec!");
            while (1);
        }

        for (int j = 0; j < 10000; j++) {
            hal_sys_timer_delay_us(10);
            if (!hal_jpeg_busy())
                break;
        }
        hal_jpeg_stop();

        TRACE(0, "JPEG decode [%dx%d, %s] used %uus.",
                cfg.width, cfg.height, yuvstr[cfg.format],
                FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

#ifdef TEST_CHECK_PIX
        int err = 0;
        for (int j = 0; j < cfg.pix_size; j++) {
            if (pix_buf[j] != pix_data[j]) {
                TRACE(0, "ERROR   pix[%d] = %02x, expect %02x",
                        j, pix_buf[j], pix_data[j]);
                err++;
            }
        }

        if (err > 0) {
            ASSERT(0, "Test %02d failed, %d errors.", i, err);
        } else {
            TRACE(0, "Test %02d pixel data check OK!", i);
            TRACE(0, "Dump 32 bytes:");
            DUMP8("%02x ", pix_buf, 32);
        }
        TRACE_IMM(0, "\n");
#endif
#ifdef TEST_DUMP
        /* Convert YUV422 Packed (VYUY) to YUV422 Planar data,
         * then dump all data to check picture in YUV tool. */
        yuv422_packed_to_planar_dump(pix_buf, cfg.width, cfg.height);
        //yuv422_packed_dump(pix_buf, cfg.width, cfg.height);
        while (1);
#endif
    }
}

POSSIBLY_UNUSED
static void jpeg_enc_test(void)
{
    uint32_t start;
    struct HAL_JPEG_CONFIG_T cfg;

    /* prepare pixel data */
    memcpy(pix_buf, yuv422_pix, PIX_BUF_SIZE);

    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_JPEG_ENC_MODE;
    cfg.width = TEST_WIDTH;
    cfg.height = TEST_HEIGHT;
    cfg.format = HAL_JPEG_YUV422;
    cfg.ecs = ecs_buf;
    cfg.pix = pix_buf;
    cfg.pix_size = PIX_BUF_SIZE;
    cfg.qtable = yuv422_qetable;
    cfg.qtable_size = sizeof(yuv422_qetable);
    cfg.hetable = yuv422_hetable;
    cfg.hetable_size = sizeof(yuv422_hetable);
    cfg.swap_uv = false;
    cfg.swap_yu = false;

    for (int i = 0; i < 10; i++) {
        cfg.ecs_size = 0;
        memset(ecs_buf, 0, sizeof(ecs_buf));

        hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

        start = hal_fast_sys_timer_get();

        if (hal_jpeg_start(&cfg)) {
            TRACE(0, "Failed to start jpeg codec!");
            while (1);
        }

        for (int j = 0; j < 10000; j++) {
            hal_sys_timer_delay_us(10);
            if (!hal_jpeg_busy())
                break;
        }
        hal_jpeg_stop();

        TRACE(0, "JPEG encode [%dx%d, %s] used %uus.",
                cfg.width, cfg.height, yuvstr[cfg.format],
                FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

        TRACE(0, "ECS data total %u bytes:", cfg.ecs_size);
        TRACE(0, "Dump 64 bytes:");
        DUMP8("%02x ", ecs_buf, 32);
        DUMP8("%02x ", ecs_buf+32, 32);

#ifdef TEST_DUMP
        /* Create jfif file and dump it for check */
        int jfif_sz = 0;
        if (jpeg_merge(&cfg, jfif_buf, &jfif_sz))
            ASSERT(0, "Failed to merge data to jfif file.");

        for (int i = 0; i < jfif_sz; i += 16) {
            DUMP8(",%02x", jfif_buf + i, 16);
            hal_sys_timer_delay(MS_TO_TICKS(1));
        }
        while (1);
#endif
    }
}

void jpeg_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    hal_jpeg_open();
    hal_dma_open();

#ifdef TEST_ENC
    TRACE(0, "jpeg encode test");
    jpeg_enc_test();
#else
    TRACE(0, "jpeg decode test");
    jpeg_dec_test();
#endif

    hal_jpeg_close();
    TRACE_IMM(0, "All is done!");
    while (1);
}

#endif /* JPEG_TEST */
