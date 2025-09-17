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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_i2c.h"
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "camera_drv.h"

#ifdef CAMERA_IMAGE_DUMP_NUM
static int cam_data_dump_num = CAMERA_IMAGE_DUMP_NUM;
#else
static int cam_data_dump_num = 1;
#endif
static int cam_data_dump_cnt = 0;

#define SEED_INIT_VALUE 0x12345678
static uint32_t g_seed_value = SEED_INIT_VALUE;

#define IMG_CHECK_BUFF_MAX_SIZE (480*(640/10)*2)
#define TGT_IMG_FRM_SEQ         (5)
#define TGT_IMG_CHK_NUM         (10)

#ifdef CAMERA_TEST_DATA_CHECK
struct CAMERA_IMG_CHECK_CNTL_T {
    char check_start;
    char check_finish;
    char check_enable;
    char oneshot;
    uint32_t check_errcnt;
    uint32_t img_width;
    uint32_t img_height;
    uint32_t img_pixel_size;
    uint32_t cur_frm_seq;
    uint32_t chk_img_frm_seq;
    uint32_t tgt_img_frm_seq;
    uint32_t tgt_img_chk_num;
    uint32_t tgt_img_chk_cnt;
    uint32_t tgt_img_chk_len;
    uint32_t tgt_img_buf_size;
    uint32_t cb_seq;
    CAMERA_DATA_CHECK_CALLBACK_T callback;
    uint8_t tgt_img_buf[IMG_CHECK_BUFF_MAX_SIZE];
};

static struct CAMERA_IMG_CHECK_CNTL_T camera_img_check_cntl;
#endif /* CAMERA_TEST_DATA_CHECK */

POSSIBLY_UNUSED static uint32_t seed_value_gen(void)
{
    g_seed_value++;

    return g_seed_value;
}

void camera_data_dump_init(void)
{
    TRACE(1, "[%s]:", __func__);

    cam_data_dump_cnt = 0;

    g_seed_value = SEED_INIT_VALUE;
}

void camera_data_dump_trap(void)
{
    if (cam_data_dump_cnt == cam_data_dump_num) {
        TRACE(1, "CPU_ENTER_WHILE1");
        while (1);
    }
}

#ifdef CAMERA_TEST_DATA_CHECK
void camera_test_data_check_init(uint32_t width, uint32_t height, uint32_t pixel_size,
    CAMERA_DATA_CHECK_CALLBACK_T callback, bool oneshot)
{
    struct CAMERA_IMG_CHECK_CNTL_T *c = &camera_img_check_cntl;

    c->img_width = width;
    c->img_height = height;
    c->img_pixel_size = pixel_size;

    c->cur_frm_seq     = 0;
    c->chk_img_frm_seq = TGT_IMG_FRM_SEQ + 1;

    c->tgt_img_frm_seq = TGT_IMG_FRM_SEQ;
    c->tgt_img_chk_num = TGT_IMG_CHK_NUM;
    c->tgt_img_chk_cnt = 0;
    c->tgt_img_chk_len = 0;
    c->tgt_img_buf_size = IMG_CHECK_BUFF_MAX_SIZE;

    c->cb_seq = 0;
    c->callback = callback;
    c->oneshot = oneshot == true ? 1 : 0;


    TRACE(1, "[%s]: -------- DUMP img check cntl info --------", __func__);
    TRACE(1, "img_width      : %d", c->img_width);
    TRACE(1, "img_height     : %d", c->img_height);
    TRACE(1, "img_pixel_size : %d", c->img_pixel_size);

    TRACE(1, "cur_frm_seq    : %d", c->cur_frm_seq);
    TRACE(1, "chk_img_frm_seq: %d", c->chk_img_frm_seq);

    TRACE(1, "tgt_img_frm_seq: %d", c->tgt_img_frm_seq);
    TRACE(1, "tgt_img_chk_num: %d", c->tgt_img_chk_num);
    TRACE(1, "tgt_img_chk_cnt: %d", c->tgt_img_chk_cnt);
    TRACE(1, "tgt_img_buf_size : %d", c->tgt_img_buf_size);
    TRACE(1, "dump end");
}

void camera_test_data_check_enable(bool enable)
{
    struct CAMERA_IMG_CHECK_CNTL_T *c = &camera_img_check_cntl;

    c->cur_frm_seq = 0;
    c->check_start = 0;
    c->check_finish = 0;
    c->check_errcnt = 0;
    c->check_enable = enable == true ? 1 : 0;

    TRACE(1, "[%s]: check_enable=%d", __func__, c->check_enable);
}

int camera_test_data_check_run(uint8_t *buf, uint32_t len)
{
#define PIX_BSIZE 8
    int chk_err = 0;
    struct CAMERA_IMG_CHECK_CNTL_T *c = &camera_img_check_cntl;

    if (!c->check_enable) {
        return 0;
    }

    c->cur_frm_seq++;

    if (c->cur_frm_seq == c->tgt_img_frm_seq) {
        if (c->img_pixel_size == 2) {
            int16_t *dst = (int16_t *)c->tgt_img_buf;
            int16_t *src = (int16_t *)buf;
            uint32_t pix_cnt;

            if (len > c->tgt_img_buf_size) {
                pix_cnt = c->tgt_img_buf_size / 2;
            } else {
                pix_cnt = len / 2;
            }
            pix_cnt = (pix_cnt / PIX_BSIZE) * PIX_BSIZE;
            c->tgt_img_chk_len = pix_cnt * 2;
            while (pix_cnt > 0) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                dst[4] = src[4];
                dst[5] = src[5];
                dst[6] = src[6];
                dst[7] = src[7];

                dst += PIX_BSIZE;
                src += PIX_BSIZE;
                pix_cnt -= PIX_BSIZE;
            }
        }
        else if (c->img_pixel_size == 4) {
            int32_t *dst = (int32_t *)c->tgt_img_buf;
            int32_t *src = (int32_t *)buf;
            uint32_t pix_cnt;

            if (len > c->tgt_img_buf_size) {
                pix_cnt = c->tgt_img_buf_size / 4;
            } else {
                pix_cnt = len / 4;
            }
            pix_cnt = (pix_cnt / PIX_BSIZE) * PIX_BSIZE;
            c->tgt_img_chk_len = pix_cnt * 4;
            while (pix_cnt > 0) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                dst[4] = src[4];
                dst[5] = src[5];
                dst[6] = src[6];
                dst[7] = src[7];

                dst += PIX_BSIZE;
                src += PIX_BSIZE;
                pix_cnt -= PIX_BSIZE;
            }
        }
    }
    if (c->cur_frm_seq == c->chk_img_frm_seq) {
        c->tgt_img_chk_cnt = 0;
        c->check_finish = 0;
        c->check_errcnt = 0;
        c->check_start = 1;
    }
    if (c->check_start) {
        uint32_t i;

        if (c->img_pixel_size == 2) {
            int16_t *dst = (int16_t *)c->tgt_img_buf;
            int16_t *src = (int16_t *)buf;
            uint32_t pix_cnt;
            uint64_t dv = 0, sv = 0;

            pix_cnt = c->tgt_img_chk_len / 2;
            for (i = 0; i < pix_cnt; i += PIX_BSIZE) {
                dv = sv = 0;

                dv += dst[0];
                dv += dst[1];
                dv += dst[2];
                dv += dst[3];
                dv += dst[4];
                dv += dst[5];
                dv += dst[6];
                dv += dst[7];

                sv += src[0];
                sv += src[1];
                sv += src[2];
                sv += src[3];
                sv += src[4];
                sv += src[5];
                sv += src[6];
                sv += src[7];

                if (sv != dv) {
                    chk_err = 1;
                    break;
                }
                src += PIX_BSIZE;
                dst += PIX_BSIZE;
            }
            if (chk_err) {
                hal_trace_flush_output();

                TRACE(1, "[%s]: CHECK_ERR16, seq=%d, pos=%d, src=%x, dst=%x", __func__,
                    c->cur_frm_seq, i, (int)buf, (int)c->tgt_img_buf);
                TRACE(1, "SRC[%8X]: %d %d %d %d %d %d %d %d",(int)src,
                    src[0],src[1],src[2],src[3],src[4],src[5],src[6],src[7]);
                TRACE(1, "DST[%8X]: %d %d %d %d %d %d %d %d",(int)dst,
                    dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7]);

                hal_trace_flush_output();
            }
        }
        else if (c->img_pixel_size == 4) {
            int32_t *dst = (int32_t *)c->tgt_img_buf;
            int32_t *src = (int32_t *)buf;
            uint32_t pix_cnt;
            uint64_t dv = 0, sv = 0;

            pix_cnt = c->tgt_img_chk_len / 4;
            for (i = 0; i < pix_cnt; i += PIX_BSIZE) {
                dv = sv = 0;

                dv += dst[0];
                dv += dst[1];
                dv += dst[2];
                dv += dst[3];
                dv += dst[4];
                dv += dst[5];
                dv += dst[6];
                dv += dst[7];

                sv += src[0];
                sv += src[1];
                sv += src[2];
                sv += src[3];
                sv += src[4];
                sv += src[5];
                sv += src[6];
                sv += src[7];

                if (sv != dv) {
                    chk_err = 1;
                    break;
                }
                src += PIX_BSIZE;
                dst += PIX_BSIZE;
            }
            if (chk_err) {
                hal_trace_flush_output();

                TRACE(1, "[%s]: CHECK_ERR32, seq=%d, pos=%d, src=%x, dst=%x", __func__,
                    c->cur_frm_seq, i, (int)buf, (int)c->tgt_img_buf);
                TRACE(1, "SRC[%8X]: %08X %08X %08X %08X %08X %08X %08X %08X",(int)src,
                    src[0],src[1],src[2],src[3],src[4],src[5],src[6],src[7]);
                TRACE(1, "DST[%8X]: %08X %08X %08X %08X %08X %08X %08X %08X",(int)dst,
                    dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7]);

                hal_trace_flush_output();
            }
        }
        if (chk_err) {
            c->check_errcnt++;
        }
        c->tgt_img_chk_cnt++;
        if (chk_err || (c->tgt_img_chk_cnt >= c->tgt_img_chk_num)) {
            c->check_start = 0;
            c->check_finish = 1;
        }
    }
    if (c->check_finish) {
        int err_cnt = c->check_errcnt;
        int chk_cnt = c->tgt_img_chk_num;
        int cb_seq;

        c->check_finish = 0;
        if (!c->oneshot) {
            c->cur_frm_seq = 0;
            c->check_start = 0;
            c->check_errcnt = 0;
        }
        c->cb_seq++;
        cb_seq = c->cb_seq;
        if (c->callback) {
            c->callback(err_cnt, chk_cnt, cb_seq);
        }
    }
    return chk_err;
}

#endif /* CAMERA_TEST_DATA_CHECK */

void camera_data_dump(uint8_t *buf, uint32_t len, uint32_t pix_bits)
{
    uint32_t i, dump_cnt = 0;

    TRACE(1, "CAM_DATA_DUMP: buf=%x, len=%x, bits=%d", (int)buf, len, pix_bits);
    hal_trace_flush_output();

    if (pix_bits <= 8) {
        uint8_t *p = (uint8_t *)buf;
        uint32_t n = len / 2;

        for (i = 0; i < n; i += 8) {
            TRACE(1, "[%8X]: %02X %02X %02X %02X %02X %02X %02X %02X", (int)p,
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    else if (pix_bits <= 16) {
        uint16_t *p = (uint16_t *)buf;
        uint32_t n = len / 2;

        for (i = 0; i < n; i += 8) {
            TRACE(1, "[%8X]: %04X %04X %04X %04X %04X %04X %04X %04X", (int)p,
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    else if (pix_bits <= 32) {
        uint32_t *p = (uint32_t *)buf;
        uint32_t n = len / 4;

        for (i = 0; i < n; i += 8) {
            TRACE(1, "[%8X]: %08X %08X %08X %08X %08X %08X %08X %08X", (int)p,
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    TRACE(1, "DUMP_END");
    hal_trace_flush_output();
}

void camera_data_dump2(uint8_t *buf, uint32_t len, uint32_t pix_bits)
{
#define ATTR2 0x30000
    uint32_t i, dump_cnt = 0;

    cam_data_dump_cnt++;

    TRACE(ATTR2, "[%d]CAM_DATA_DUMP2: buf=%x, len=%x, bits=%d",
        cam_data_dump_cnt, (int)buf, len, pix_bits);

    hal_trace_flush_output();

    TRACE(ATTR2, "cam_data[]={");
    if (pix_bits <= 8) {
        uint8_t *p = (uint8_t *)buf;
        uint32_t n = len / 2;

        for (i = 0; i < n; i += 8) {
            TRACE(ATTR2, "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,",
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    else if (pix_bits <= 16) {
        uint16_t *p = (uint16_t *)buf;
        uint32_t n = len / 2;

        for (i = 0; i < n; i += 8) {
            TRACE(ATTR2, "0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X,",
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    else if (pix_bits <= 32) {
        uint32_t *p = (uint32_t *)buf;
        uint32_t n = len / 4;

        for (i = 0; i < n; i += 8) {
            TRACE(ATTR2, "0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X,",
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            p += 8;

            dump_cnt++;
            if ((dump_cnt % 8) == 0) {
                hal_trace_flush_output();
            }
        }
    }
    TRACE(ATTR2, "};");
    TRACE(ATTR2, "DUMP2_END");
    hal_trace_flush_output();
}

int camera_data_memcpy_dma(uint8_t *dst, uint8_t *src, uint32_t size)
{
    int ret = 0;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_RET_T dma_ret;
    uint32_t remains = 0;

    memset(&dma_cfg,0,sizeof(dma_cfg));

    dma_cfg.dst_periph = HAL_GPDMA_MEM;
    dma_cfg.ch = hal_dma_get_chan(dma_cfg.dst_periph, HAL_DMA_LOW_PRIO);
    ASSERT(dma_cfg.ch != HAL_DMA_CHAN_NONE, "[%s]: Faild to get dma chan", __func__);
    dma_cfg.dst_bsize  = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg.src_periph = HAL_GPDMA_MEM;
    dma_cfg.src_bsize  = HAL_DMA_BSIZE_4;
    dma_cfg.src_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg.type       = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg.try_burst  = false;
    dma_cfg.handler    = NULL;
    dma_cfg.dst = (uint32_t)dst;
    dma_cfg.src = (uint32_t)src;

    remains = size / 4;
    while (remains > 0) {
        if (remains > HAL_DMA_MAX_DESC_XFER_SIZE) {
            dma_cfg.src_tsize = HAL_DMA_MAX_DESC_XFER_SIZE;
        } else {
            dma_cfg.src_tsize = remains;
        }
        dma_ret = hal_dma_start(&dma_cfg);
        if (dma_ret != HAL_DMA_OK) {
            TRACE(1, "[%s]: Failed to start dma: %d, ch=%x", __func__, dma_ret, dma_cfg.ch);
            ret = 21;
            break;
        }
        while (hal_dma_chan_busy(dma_cfg.ch));

        dma_cfg.dst += dma_cfg.src_tsize * 4;
        dma_cfg.src += dma_cfg.src_tsize * 4;
        remains -= dma_cfg.src_tsize;
    }
    hal_dma_stop(dma_cfg.ch);
    hal_dma_free_chan(dma_cfg.ch);
    return ret;
}

int camera_data_memcpy(uint8_t *dst, uint8_t *src, uint32_t size)
{
#define BS           (8)
    volatile uint32_t *pd = (volatile uint32_t *)dst;
    volatile uint32_t *ps = (volatile uint32_t *)src;
    uint32_t blk = size / 4 / BS;
    uint32_t lst = (size / 4) % BS;
    uint32_t i;

    if (size == 0) {
        return 0;
    }
    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = ps[0];
            pd[1] = ps[1];
            pd[2] = ps[2];
            pd[3] = ps[3];
            pd[4] = ps[4];
            pd[5] = ps[5];
            pd[6] = ps[6];
            pd[7] = ps[7];
            pd += BS;
            ps += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = ps[i];
        }
    }
    return 0;
}

int camera_data_raw10_to_yuv422(uint32_t dst, uint32_t src, uint32_t size)
{
#define BS           (8)
#define BM           (0x00FF00FF)
#define BF           (0x80008000)
#define RAW10_YUV(v) ((((v) >> 2) & BM) | BF)
//#define RAW10_YUV(v) (v)

    volatile uint32_t *pd = (volatile uint32_t *)dst;
    volatile uint32_t *ps = (volatile uint32_t *)src;
    uint32_t blk = size / 4 / BS;
    uint32_t lst = (size / 4) % BS;
    uint32_t i;

    if (size == 0) {
        return 0;
    }

    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = RAW10_YUV(ps[0]);
            pd[1] = RAW10_YUV(ps[1]);
            pd[2] = RAW10_YUV(ps[2]);
            pd[3] = RAW10_YUV(ps[3]);
            pd[4] = RAW10_YUV(ps[4]);
            pd[5] = RAW10_YUV(ps[5]);
            pd[6] = RAW10_YUV(ps[6]);
            pd[7] = RAW10_YUV(ps[7]);
            pd += BS;
            ps += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = RAW10_YUV(ps[i]);
        }
    }

    return 0;
}

int camera_data_raw8_to_yuv422(uint32_t dst, uint32_t src, uint32_t size)
{
#define BS              (8)
#define RAW8_YUV422(v)  (((uint16_t)(v)) | 0x8000)

    uint16_t *pd = (uint16_t *)dst;
    uint8_t *ps  = (uint8_t *)src;
    uint32_t blk = size / BS;
    uint32_t lst = size % BS;
    uint32_t i;

    if (size == 0) {
        return 0;
    }

    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = RAW8_YUV422(ps[0]);
            pd[1] = RAW8_YUV422(ps[1]);
            pd[2] = RAW8_YUV422(ps[2]);
            pd[3] = RAW8_YUV422(ps[3]);
            pd[4] = RAW8_YUV422(ps[4]);
            pd[5] = RAW8_YUV422(ps[5]);
            pd[6] = RAW8_YUV422(ps[6]);
            pd[7] = RAW8_YUV422(ps[7]);
            pd += BS;
            ps += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = RAW8_YUV422(ps[i]);
        }
    }

    return 0;
}

int camera_data_raw10_to_yuv420(uint32_t dst, uint32_t src, uint32_t size)
{
#define BS  (8)
#define RAW10_TO_Y(v)   (((v) >> 2) & 0xff)

    uint8_t *pd = (uint8_t *)dst;
    uint16_t *ps = (uint16_t *)src;
    uint32_t pix_n = size / 2;
    uint32_t blk = pix_n / BS;
    uint32_t lst = pix_n % BS;
    uint32_t i;

    if (pix_n == 0) {
        return 0;
    }
    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = RAW10_TO_Y(ps[0]);
            pd[1] = RAW10_TO_Y(ps[1]);
            pd[2] = RAW10_TO_Y(ps[2]);
            pd[3] = RAW10_TO_Y(ps[3]);
            pd[4] = RAW10_TO_Y(ps[4]);
            pd[5] = RAW10_TO_Y(ps[5]);
            pd[6] = RAW10_TO_Y(ps[6]);
            pd[7] = RAW10_TO_Y(ps[7]);
            pd += BS;
            ps += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = RAW10_TO_Y(ps[i]);
        }
    }
    pd = (uint8_t*)(dst + pix_n);
    memset(pd, 0x80, (pix_n & ~0x3) >> 1);

    return 0;
}

int camera_data_clean(uint8_t *src, uint32_t size)
{
    uint32_t *pd = (uint32_t *)src;
    uint32_t i;
    uint32_t blk = (size >> 2) / BS;
    uint32_t lst = (size >> 2) % BS;
    uint32_t seed;

    if (size == 0) {
        return 0;
    }

    seed = seed_value_gen();
    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            pd[0] = seed;
            pd[1] = seed;
            pd[2] = seed;
            pd[3] = seed;
            pd[4] = seed;
            pd[5] = seed;
            pd[6] = seed;
            pd[7] = seed;
            pd += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            pd[i] = seed;
        }
    }

    return 0;
}
