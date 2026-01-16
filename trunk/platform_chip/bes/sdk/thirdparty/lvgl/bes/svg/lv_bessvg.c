/**
 * @file lv_bessvg.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_BESSVG
#include "src/draw/lv_image_decoder_private.h"
#include "src/core/lv_global.h"

#include "lv_bessvg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hal_timer.h"
#include "hal_cache.h"
#include "svg_render.h"


/*********************
 *      DEFINES
 *********************/
#define DECODER_NAME    "BESSVG"

#define RENDER_MAX_WIDTH    (LV_HOR_RES/2/16*16)
#define RENDER_MAX_HEIGHT   (LV_VER_RES/2)

#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * src, lv_image_header_t * header);
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc);
static void decoder_close(lv_image_decoder_t * dec, lv_image_decoder_dsc_t * dsc);
static lv_draw_buf_t *decode_svg_data(uint8_t *svg_data, size_t svg_data_size);
static int get_svg_info(char *data, uint32_t data_size, int *width, int *height);

static inline lv_draw_buf_t *draw_buf_create(uint32_t w, uint32_t h, lv_color_format_t cf, uint32_t stride)
{
    return lv_draw_buf_create_user(image_cache_draw_buf_handlers, w, h, cf, stride);
}

static inline void draw_buf_destroy(lv_draw_buf_t * buf)
{
    lv_draw_buf_destroy_user(image_cache_draw_buf_handlers, buf);
}

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register the SVG decoder functions in LVGL
 */
void lv_bessvg_init(void)
{
    lv_image_decoder_t * dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);
    dec->name = DECODER_NAME;
}

void lv_bessvg_deinit(void)
{
    lv_image_decoder_t * dec = NULL;
    while((dec = lv_image_decoder_get_next(dec)) != NULL) {
        if(dec->info_cb == decoder_info) {
            lv_image_decoder_delete(dec);
            break;
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info about a SVG image
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       image descriptor containing the source and type of the image and other info.
 * @param header    image information is set in header parameter
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /* Unused */

    lv_image_src_t src_type = dsc->src_type;

    if (src_type == LV_IMAGE_SRC_FILE || src_type == LV_IMAGE_SRC_VARIABLE) {
        uint8_t buf[16];
        int dst_w, dst_h;
        int svg_w, svg_h;

        if (src_type == LV_IMAGE_SRC_FILE) {
            void *data = NULL;
            uint32_t data_size = 512;
            lv_fs_res_t fsres;
            uint32_t rn;

            /* Check '<?xml' in file header */
            lv_fs_read(&dsc->file, buf, sizeof(buf), &rn);
            if (rn != sizeof(buf)) {
                return LV_RESULT_INVALID;
            }
            if (lv_memcmp(buf, "<?xml", 5) != 0 &&
                lv_memcmp(buf, "<?XML", 5) != 0) {
                return LV_RESULT_INVALID;
            }

            data = lv_malloc(data_size);
            if (!data) {
                LV_LOG_WARN("Failed to malloc memory.");
                return LV_RESULT_INVALID;
            }

            fsres = lv_fs_read(&dsc->file, data, data_size, &rn);
            if (fsres != LV_FS_RES_OK || rn == 0) {
                LV_LOG_WARN("Failed to read file: %d", fsres);
                lv_free(data);
                return LV_RESULT_INVALID;
            }

            if (get_svg_info(data, rn, &svg_w, &svg_h) != 0) {
                lv_free(data);
                return LV_RESULT_INVALID;
            }
            lv_free(data);

        } else {
            const lv_image_dsc_t * img_dsc = dsc->src;

            if (img_dsc->data == NULL || img_dsc->data_size <= 0) {
                LV_LOG_WARN("Invalid SVG data.");
                return LV_RESULT_INVALID;
            }

            /* Check '<?xml' in file header */
            if (lv_memcmp(img_dsc->data, "<?xml", 5) != 0 &&
                lv_memcmp(img_dsc->data, "<?XML", 5) != 0) {
                return LV_RESULT_INVALID;
            }

            if (get_svg_info((char *)img_dsc->data, 512, &svg_w, &svg_h) != 0) {
                return LV_RESULT_INVALID;
            }
        }

        /* calculate SVG target render width/height */
        if (svg_w >= svg_h) {
            dst_w = RENDER_MAX_WIDTH;
            dst_h = svg_h * dst_w / svg_w;
        } else {
            dst_h = RENDER_MAX_HEIGHT;
            dst_w = svg_w * dst_h / svg_h;
            dst_w = (dst_w + 15) / 16 * 16;
            if (dst_w > RENDER_MAX_WIDTH)
                dst_w = RENDER_MAX_WIDTH;
        }

        header->cf = LV_COLOR_FORMAT_ARGB8888;
        header->w = (int32_t)dst_w;
        header->h = (int32_t)dst_h;

        return LV_RESULT_OK;
    }

    return LV_RESULT_INVALID;
}

/**
 * Open a SVG image and decode it into dsc.decoded
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    uint8_t * svg_data = NULL;
    size_t svg_data_size = 0;

    if(dsc->src_type == LV_IMAGE_SRC_FILE) {
        const char * fn = dsc->src;
        if(lv_strcmp(lv_fs_get_ext(fn), "svg") != 0) {
            return LV_RESULT_INVALID;
        }

        lv_fs_file_t f;
        lv_fs_res_t fsres;

        fsres = lv_fs_open(&f, fn, LV_FS_MODE_RD);
        if (fsres != LV_FS_RES_OK) {
            LV_LOG_WARN("Failed to open file: %d", fsres);
            return LV_RESULT_INVALID;
        }

        lv_fs_seek(&f, 0, LV_FS_SEEK_END);
        lv_fs_tell(&f, &svg_data_size);
        lv_fs_seek(&f, 0, LV_FS_SEEK_SET);

        if (svg_data_size == 0) {
            LV_LOG_ERROR("Can't get file size.");
            lv_fs_close(&f);
            return LV_RESULT_INVALID;
        }

        svg_data = lv_malloc(svg_data_size);
        if (svg_data) {
            uint32_t rn;
            fsres = lv_fs_read(&f, svg_data, svg_data_size, &rn);
            if (fsres != LV_FS_RES_OK || rn == 0) {
                LV_LOG_WARN("Failed to read file: %d", fsres);
                lv_free(svg_data);
                lv_fs_close(&f);
                return LV_RESULT_INVALID;
            }
        }

        lv_fs_close(&f);
    }
    else if(dsc->src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t * img_dsc = dsc->src;
        svg_data = (uint8_t *)img_dsc->data;
        svg_data_size = img_dsc->data_size;
    }
    else {
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * decoded = decode_svg_data(svg_data, svg_data_size);

    if(dsc->src_type == LV_IMAGE_SRC_FILE) lv_free((void *)svg_data);

    if(!decoded) {
        LV_LOG_WARN("Error decoding SVG");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * adjusted = lv_image_decoder_post_process(dsc, decoded);
    if(adjusted == NULL) {
        draw_buf_destroy(decoded);
        return LV_RESULT_INVALID;
    }

    /*The adjusted draw buffer is newly allocated.*/
    if(adjusted != decoded) {
        draw_buf_destroy(decoded);
        decoded = adjusted;
    }

    dsc->decoded = decoded;

    if(dsc->args.no_cache) return LV_RESULT_OK;

    /*If the image cache is disabled, just return the decoded image*/
    if(!lv_image_cache_is_enabled()) return LV_RESULT_OK;

    /*Add the decoded image to the cache*/
    lv_image_cache_data_t search_key;
    search_key.src_type = dsc->src_type;
    search_key.src = dsc->src;
    search_key.slot.size = decoded->data_size;

    lv_cache_entry_t * entry = lv_image_decoder_add_to_cache(decoder, &search_key, decoded, NULL);

    if(entry == NULL) {
        return LV_RESULT_INVALID;
    }
    dsc->cache_entry = entry;

    return LV_RESULT_OK;    /*If not returned earlier then it failed*/
}

/**
 * Close SVG image and free data
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static void decoder_close(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    if(dsc->args.no_cache || !lv_image_cache_is_enabled())
        draw_buf_destroy((lv_draw_buf_t *)dsc->decoded);
}

static int get_svg_info(char *data, uint32_t data_size, int *width, int *height)
{
    char *p = data;
    char *q;
    int w, h;

    while (p < (data + data_size)) {
        if (*p == '<' && (memcmp(p, "<svg", 4) == 0 || memcmp(p, "<SVG", 4) == 0)) {
            p += 4;

            /* check viewbox first */
            q = strstr(p, "viewBox=");
            if (!q)
                q = strstr(p, "viewbox=");
            if (q) {
                int x, y;
                q += 9;
                sscanf(q, "%d %d %d %d\"", &x, &y, &w, &h);
                LV_LOG_TRACE("SVG viewbox [%d %d %d %d]", x, y, w, h);
                if (w <= 0 || h <= 0)
                    return -1;
                *width = w;
                *height = h;
                return 0;
            }

            /* viewbox not found, search width/height property */
            q = strstr(p, "width=");
            if (!q)
                return -1;
            q += 7;
            sscanf(q, "%d", &w);

            q = strstr(p, "height=");
            if (!q)
                return -1;
            q += 8;
            sscanf(q, "%d", &h);

            LV_LOG_TRACE("SVG width = %d, height = %d", w, h);
            if (w <= 0 || h <= 0)
                return -1;
            *width = w;
            *height = h;
            return 0;
        }
        p++;
    }
    return -1;
}

static lv_draw_buf_t * decode_svg_data(uint8_t *svg_data, size_t svg_data_size)
{
    int dst_w, dst_h;
    int svg_w, svg_h;
    svg_handle svg;
    lv_draw_buf_t *draw_buf = NULL;

    if (svg_data == NULL || svg_data_size <= 0) {
        LV_LOG_ERROR("SVG data is NULL.");
        return NULL;
    }

    LV_LOG_TRACE("SVG data %p, size %d bytes", svg_data, svg_data_size);

    /* Parse SVG data for render */
    if (svg_open(&svg, (const char *)svg_data, (int)svg_data_size) != SVG_OK) {
        LV_LOG_WARN("Failed to parse SVG data.");
        return NULL;
    }

    /* calculate SVG target render width/height */
    svg_w = svg_get_width(svg);
    svg_h = svg_get_height(svg);
    if (svg_w <= 0 || svg_h <= 0) {
        LV_LOG_WARN("Invalid SVG size %dx%d", svg_w, svg_h);
        svg_close(&svg);
        return NULL;
    }
    if (svg_w >= svg_h) {
        dst_w = RENDER_MAX_WIDTH;
        dst_h = svg_h * dst_w / svg_w;
    } else {
        dst_h = RENDER_MAX_HEIGHT;
        dst_w = svg_w * dst_h / svg_h;
        dst_w = (dst_w + 15) / 16 * 16;
        if (dst_w > RENDER_MAX_WIDTH)
            dst_w = RENDER_MAX_WIDTH;
    }

    draw_buf = draw_buf_create(dst_w, dst_h, LV_COLOR_FORMAT_ARGB8888, dst_w * 4);
    if (draw_buf == NULL) {
        LV_LOG_ERROR("Can't malloc draw_buf.");
        return NULL;
    }

    if (draw_buf->data_size < dst_w * dst_h * 4) {
        LV_LOG_ERROR("Small draw_buf %u < %dx%dx4", draw_buf->data_size, dst_w, dst_h);
        draw_buf_destroy(draw_buf);
        return NULL;
    }

    hal_cache_sync_invalidate(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)draw_buf->data),
            LEN_ALIGN_TO_CACHE((uint32_t)draw_buf->data_size, (uint32_t)draw_buf->data));

    if (svg_render(svg, (char *)draw_buf->data, dst_w, dst_h) != SVG_OK) {
        LV_LOG_WARN("Failed to render SVG image.");
    } else {
        LV_LOG_TRACE("SVG rendering done, %dx%d, %p", dst_w, dst_h, draw_buf->data);
    }

    svg_close(&svg);
    return draw_buf;
}

#endif /*LV_USE_BESSVG*/
