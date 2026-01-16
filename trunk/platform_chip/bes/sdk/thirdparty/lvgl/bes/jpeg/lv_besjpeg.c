/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_BESJPEG
#include "src/draw/lv_image_decoder_private.h"
#include "src/core/lv_global.h"
#include "lv_besjpeg.h"
#include "jpeg_drv.h"
#include "gpu_port.h"
#if LV_USE_BESBAJ
#include "bajdec.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define ALIGN16(n)      ((n+15)/16*16)
#define DECODER_NAME    "BESJPEG"
#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int besjpeg_get_info(uint8_t *jpeg_data, uint32_t jpeg_size, int *width, int *height);
static lv_draw_buf_t *besjpeg_decode_file(uint8_t *jpeg_data, uint32_t jpeg_size);

#if LV_USE_BESBAJ
static lv_draw_buf_t *baj_decode_file(uint8_t *data, uint32_t data_size);
#endif

static lv_result_t decoder_info(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc, lv_image_header_t *header);
static lv_result_t decoder_open(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc);
static void decoder_close(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc);

static inline lv_draw_buf_t *draw_buf_create(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride)
{
    return lv_draw_buf_create_user(image_cache_draw_buf_handlers, w, h, cf, stride);
}

static inline void draw_buf_destroy(lv_draw_buf_t * buf)
{
    lv_draw_buf_destroy_user(image_cache_draw_buf_handlers, buf);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register the JPEG decoder functions in LVGL
 */
void lv_besjpeg_init(void)
{
    lv_image_decoder_t *dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);
    dec->name = DECODER_NAME;
}

void lv_besjpeg_deinit(void)
{
    lv_image_decoder_t *dec = NULL;
    while ((dec = lv_image_decoder_get_next(dec)) != NULL) {
        if (dec->info_cb == decoder_info) {
            lv_image_decoder_delete(dec);
            break;
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info of a JPEG image
 * @param dst image descriptor
 * @param header store the info here
 * @return LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc, lv_image_header_t *header)
{
    LV_UNUSED(decoder);

    lv_result_t res = LV_RESULT_INVALID;
    const void * src = dsc->src;
    lv_image_src_t src_type = lv_image_src_get_type(src);

    uint8_t *tmpbuf = NULL;
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 0;
    int w = 0, h = 0;

    if (src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t *image = src;
        jpeg_data = (uint8_t *)image->data;
        jpeg_size = image->data_size;

    } else if (src_type == LV_IMAGE_SRC_FILE) {
        if (lv_strcmp(lv_fs_get_ext(src), "jpg") &&
           lv_strcmp(lv_fs_get_ext(src), "JPG") &&
#if LV_USE_BESBAJ
           lv_strcmp(lv_fs_get_ext(src), "baj") &&
           lv_strcmp(lv_fs_get_ext(src), "BAJ") &&
#endif
           lv_strcmp(lv_fs_get_ext(src), "jpeg") &&
           lv_strcmp(lv_fs_get_ext(src), "JPEG")) {
            return LV_RESULT_INVALID;
        }
        uint32_t filesz = 0;

        lv_fs_seek(&dsc->file, 0, LV_FS_SEEK_END);
        lv_fs_tell(&dsc->file, &filesz);
        lv_fs_seek(&dsc->file, 0, LV_FS_SEEK_SET);
        if (filesz == 0) {
            LV_LOG_WARN("Invalid file.");
            return LV_RESULT_INVALID;
        }

        tmpbuf = lv_malloc(filesz);
        if (tmpbuf) {
            lv_fs_res_t fsres;
            uint32_t rn;
            fsres = lv_fs_read(&dsc->file, tmpbuf, filesz, &rn);
            if (fsres != LV_FS_RES_OK || rn == 0) {
                LV_LOG_WARN("Failed to read file: %d", fsres);
                lv_free(tmpbuf);
                return LV_RESULT_INVALID;
            }

            jpeg_data = tmpbuf;
            jpeg_size = rn;
        }
    }

#if LV_USE_BESBAJ
    if (jpeg_data && jpeg_size > 0 &&
        baj_get_info(jpeg_data, jpeg_size, &w, &h, NULL, NULL) == BAJ_OK) {
        if (w > 0 && h > 0) {
            header->cf = LV_COLOR_FORMAT_ARGB8888;
            header->w = w;
            header->h = h;
            header->stride = header->w * 4;
            res = LV_RESULT_OK;
        }
        LV_LOG_TRACE("baj info: %dx%d", w, h);
    } else
#endif
    if (jpeg_data && jpeg_size > 0 &&
        besjpeg_get_info(jpeg_data, jpeg_size, &w, &h) == 0) {
        if (w > 0 && h > 0) {
            header->cf = LV_COLOR_FORMAT_YUY2;
            header->w = w;
            header->h = h;
            header->stride = header->w * 2;
            res = LV_RESULT_OK;
        }
        LV_LOG_TRACE("jpeg info: %dx%d", w, h);
    }

    if (tmpbuf) {
        lv_free(tmpbuf);
    }
    return res;
}

/**
 * Open a JPEG image and return the decided image
 * @param decoder pointer to the decoder
 * @param dsc     pointer to the decoder descriptor
 * @return LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);

    const void * src = dsc->src;
    lv_image_src_t src_type = lv_image_src_get_type(src);

    uint8_t *tmpbuf = NULL;
    uint8_t *jpeg_data = NULL;
    uint32_t jpeg_size = 0;

    if (dsc->src_type == LV_IMAGE_SRC_VARIABLE) {
        lv_image_dsc_t * image = (lv_image_dsc_t *)src;
        jpeg_data = (uint8_t *)image->data;
        jpeg_size = image->data_size;

    } else if (src_type == LV_IMAGE_SRC_FILE) {
        if (lv_strcmp(lv_fs_get_ext(src), "jpg") &&
           lv_strcmp(lv_fs_get_ext(src), "JPG") &&
#if LV_USE_BESBAJ
           lv_strcmp(lv_fs_get_ext(src), "baj") &&
           lv_strcmp(lv_fs_get_ext(src), "BAJ") &&
#endif
           lv_strcmp(lv_fs_get_ext(src), "jpeg") &&
           lv_strcmp(lv_fs_get_ext(src), "JPEG")) {
            return LV_RESULT_INVALID;
        }

        lv_fs_file_t f;
        lv_fs_res_t fsres;

        fsres = lv_fs_open(&f, src, LV_FS_MODE_RD);
        if (fsres != LV_FS_RES_OK) {
            LV_LOG_WARN("Failed to open file: %d", fsres);
            return LV_RESULT_INVALID;
        }

        lv_fs_seek(&f, 0, LV_FS_SEEK_END);
        lv_fs_tell(&f, &jpeg_size);
        lv_fs_seek(&f, 0, LV_FS_SEEK_SET);

        if (jpeg_size == 0) {
            LV_LOG_ERROR("Can't get file size.");
            lv_fs_close(&f);
            return LV_RESULT_INVALID;
        }

        tmpbuf = lv_malloc(jpeg_size);
        if (tmpbuf) {
            uint32_t rn;
            fsres = lv_fs_read(&f, tmpbuf, jpeg_size, &rn);
            if (fsres != LV_FS_RES_OK || rn == 0) {
                LV_LOG_WARN("Failed to read file: %d", fsres);
                lv_free(tmpbuf);
                lv_fs_close(&f);
                return LV_RESULT_INVALID;
            }

            jpeg_data = tmpbuf;
            jpeg_size = rn;
        }

        lv_fs_close(&f);

    } else {
        return LV_RESULT_INVALID;
    }

    if (!jpeg_data || jpeg_size == 0) {
        if (tmpbuf) {
            lv_free(tmpbuf);
        }
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t *decoded = NULL;

#if LV_USE_BESBAJ
    /* Alpha-JPEG file */
    if (baj_get_info(jpeg_data, jpeg_size, NULL, NULL, NULL, NULL) == BAJ_OK) {
        decoded = baj_decode_file(jpeg_data, jpeg_size);
    }
#endif
    /* Regular JPEG file */
    if (decoded == NULL) {
        decoded = besjpeg_decode_file(jpeg_data, jpeg_size);
    }

    if (tmpbuf) {
        lv_free(tmpbuf);
    }

    if (decoded == NULL) {
        LV_LOG_ERROR("failed to open jpeg file.");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * adjusted = lv_image_decoder_post_process(dsc, decoded);
    if (adjusted == NULL) {
        draw_buf_destroy(decoded);
        return LV_RESULT_INVALID;
    }

    /* The adjusted draw buffer is newly allocated. */
    if (adjusted != decoded) {
        draw_buf_destroy(decoded);
        decoded = adjusted;
    }
    dsc->decoded = decoded;

    if (dsc->args.no_cache) {
        return LV_RESULT_OK;
    }

    /* If the image cache is disabled, just return the decoded image */
    if (!lv_image_cache_is_enabled()) {
        return LV_RESULT_OK;
    }

    /* Add the decoded image to the cache */
    lv_image_cache_data_t search_key;
    search_key.src_type = dsc->src_type;
    search_key.src = dsc->src;
    search_key.slot.size = decoded->data_size;

    lv_cache_entry_t *entry = lv_image_decoder_add_to_cache(decoder, &search_key, decoded, NULL);

    if(entry == NULL) {
        return LV_RESULT_INVALID;
    }
    dsc->cache_entry = entry;

    return LV_RESULT_OK;
}

/**
 * Free the allocated resources
 */
static void decoder_close(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    if (dsc->args.no_cache || !lv_image_cache_is_enabled()) {
        if (dsc->decoded) {
            draw_buf_destroy((lv_draw_buf_t *)dsc->decoded);
        }
    }
}

#define GETC(p)                 ((*p++) & 0xff)

static int besjpeg_get_info(uint8_t *jpeg_data, uint32_t jpeg_size, int *width, int *height)
{
    uint8_t v;
    uint8_t *p = jpeg_data;

    /* Is a jpeg file? */
    if (p[0] != 0xff || p[1] != 0xd8) {
        return -1;
    }

    while (p < (jpeg_data + jpeg_size)) {
        int m, len;

        if (!(p[0] == 0xff && p[1] > 0 && p[1] < 0xff)) {
            p++;
            continue;
        }

        m = p[1];
        if (m >= 0xc0 && m <= 0xc3) { /* Start of frame */
            int bpp, color_num;

            p += 4;
            bpp = GETC(p);
            if (bpp != 8) {
                return -1;
            }

            v = GETC(p);
            *height = (v << 8) | GETC(p);
            v = GETC(p);
            *width = (v << 8) | GETC(p);

            color_num = GETC(p);
            if (color_num != 3) {
                return -1;
            }
            for (int i = 0; i < color_num; i++) {
                p += 3;
            }

        } else if ((m >= 0xc5 && m <= 0xc7) || (m >= 0xc9 && m <= 0xcb) || (m >= 0xcd && m <= 0xcf)) { /* other SOFs */
            len = (p[2] << 8 | p[3]);
            p += len + 2;;

        } else if (m == 0xc4) { /* H table */
            len = (p[2] << 8 | p[3]);
            p += len + 2;

        } else if (m == 0xc8) {
            return -1;

        } else if (m == 0xcc) {
            return -1;

        } else if (m >= 0xd0 && m <= 0xd7) {
            p += 2;

        } else if (m == 0xd8) { /* SOI */
            p += 2;

        } else if (m == 0xd9) { /* End of image */
            break;

        } else if (m == 0xda) { /* Start of Scans */
            len = (p[2] << 8 | p[3]);
            p += len + 2;
            while (p < (jpeg_data + jpeg_size)) {
                if (p[0] == 0xff) {
                    if (p[1] != 0xff && p[1] >= 0xc0 && (p[1] & 0xf8) != 0xd0) {
                        break;
                    } else {
                        p += 2;
                    }
                } else {
                    p++;
                }
            }

        } else if (m == 0xdb) { /* Q table */
            len = (p[2] << 8 | p[3]);
            p += len + 2;

        } else if (m == 0xdd) { /* Restart interval */
            len = (p[2] << 8 | p[3]);
            p += len + 2;

        } else if (m >= 0xe0 && m <= 0xef) { /* APPs */
            len = (p[2] << 8 | p[3]);
            p += len + 2;;

        } else if (m == 0xfe) { /* Comments */
            len = (p[2] << 8 | p[3]);
            p += len + 2;

        } else {
            p += 2;
        }
    }
    return 0;
}

static lv_draw_buf_t *besjpeg_decode_file(uint8_t *jpeg_data, uint32_t jpeg_size)
{
    struct jpeg_handle_t handle;
    uint8_t *tmp_buf = NULL;
    uint8_t *ecs_buf = NULL;
    uint32_t ecs_bufsz = 0;
    int w = 0, h = 0;
    int aligned_w = 0, aligned_h = 0;
    uint32_t pixsz = 0;
    lv_draw_buf_t *draw_buf = NULL;

    if (jpeg_data == NULL || jpeg_size == 0) {
        LV_LOG_ERROR("jpeg data is NULL.");
        return NULL;
    }

    if (besjpeg_get_info(jpeg_data, jpeg_size, &w, &h) != 0) {
        LV_LOG_ERROR("Invalid jpeg data.");
        return NULL;
    }
    if (w <= 0 || h <= 0) {
        LV_LOG_ERROR("Invalid jpeg size, %dx%d", w, h);
        return NULL;
    }
    aligned_w = ALIGN16(w);
    aligned_h = ALIGN16(h);
    LV_LOG_TRACE("jpeg image size %dx%d, aligned to %dx%d", w, h, aligned_w, aligned_h);
    pixsz = aligned_w * aligned_h * 2;

    tmp_buf = lv_malloc(jpeg_size + 64);
    if (tmp_buf == NULL) {
        LV_LOG_ERROR("Can't malloc tmp_buf.");
        return NULL;
    }

    ecs_buf = (uint8_t *)(((uint32_t)tmp_buf + 63) & 0xffffffc0);
    ecs_bufsz = (jpeg_size + 63) & 0xffffffc0;

    draw_buf = draw_buf_create(aligned_w, aligned_h, LV_COLOR_FORMAT_YUY2, aligned_w * 2);
    if (draw_buf == NULL) {
        LV_LOG_ERROR("Can't malloc draw_buf.");
        lv_free(tmp_buf);
        return NULL;
    }

    if (draw_buf->data_size < pixsz) {
        LV_LOG_ERROR("Small draw_buf %u < %dx%dx2", draw_buf->data_size, aligned_w, aligned_h);
        lv_free(tmp_buf);
        draw_buf_destroy(draw_buf);
        return NULL;
    }

    handle.jpeg_data = jpeg_data;
    handle.jpeg_size = jpeg_size;
    handle.ecs_buf = ecs_buf;
    handle.ecs_bufsz = ecs_bufsz;
    handle.pix_data = (uint8_t *)gpu_hal_buf_addr((uint32_t)draw_buf->data);
    handle.pix_size = pixsz;
    handle.pix_fmt = PIX_YUV422_YUYV;
    LV_LOG_TRACE("jpeg data [%p, %u bytes], ecs %p, pixbuf [%p, %u bytes]",
            jpeg_data, jpeg_size, ecs_buf, draw_buf->data, pixsz);

    jpeg_init();

    int ret = jpeg_decode(&handle);
    if (ret != JPEG_OK) {
        LV_LOG_ERROR("Failed to decode jpeg. ret = %d", ret);
        draw_buf_destroy(draw_buf);
        draw_buf = NULL;
    }

    jpeg_exit();

    /* set the image width/height/stride of draw buffer */
    if (draw_buf) {
        draw_buf->header.w = handle.jpeg_width;
        draw_buf->header.h = handle.jpeg_height;
        draw_buf->header.stride = handle.width * 2;
    }

    lv_free(tmp_buf);
    return draw_buf;
}

#if LV_USE_BESBAJ
static lv_draw_buf_t *baj_decode_file(uint8_t *data, uint32_t data_size)
{
    lv_draw_buf_t *draw_buf = NULL;
    int aligned_w, aligned_h;
    int org_w, org_h;
    uint8_t *tmpbuf, *aligned_buf;
    uint32_t tmpbuf_sz;

    if (baj_get_info(data, data_size, &org_w, &org_h, &aligned_w, &aligned_h) != BAJ_OK) {
        LV_LOG_ERROR("Invalid BAJ data.");
        return NULL;
    }

    tmpbuf_sz = aligned_w * aligned_h * 7; /* Alpha-JPEG decoding need a big buffer */
    tmpbuf = lv_malloc(tmpbuf_sz + 63);
    if (!tmpbuf) {
        LV_LOG_ERROR("Failed to malloc memory");
        return NULL;
    }
    aligned_buf = (uint8_t *)((uint32_t)(tmpbuf + 63) & 0xffffffc0);

    /* Prepare draw buffer */
    draw_buf = draw_buf_create(org_w, org_h, LV_COLOR_FORMAT_ARGB8888, org_w * 4);
    if (draw_buf == NULL) {
        LV_LOG_ERROR("Can't malloc draw_buf.");
        goto exit;
    }

    int ret = baj_decode(data, data_size, draw_buf->data, aligned_buf, tmpbuf_sz);
    if (ret != 0) {
        LV_LOG_ERROR("Failed to decode BAJ file. %d", ret);
        draw_buf_destroy(draw_buf);
        draw_buf = NULL;
        goto exit;
    }

    LV_LOG_TRACE("Decoded Alpha-JPEG to RGBA %p, %dx%d", draw_buf->data, org_w, org_h);

exit:
    if (tmpbuf)
        lv_free(tmpbuf);
    return draw_buf;
}

#endif

#endif /*LV_USE_BESJPEG*/
