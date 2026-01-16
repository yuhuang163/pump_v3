/**
 * @file lv_besetc2.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_BESETC2
#include "src/draw/lv_image_decoder_private.h"
#include "src/core/lv_global.h"

#include "lv_besetc2.h"
#include <stdlib.h>
#include <string.h>
#include "hal_timer.h"
#include "hal_cache.h"


/*********************
 *      DEFINES
 *********************/
#define DECODER_NAME    "BESETC2"

#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static const uint8_t magic[] = {'P', 'K', 'M', ' '};

static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * src, lv_image_header_t * header);
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc);
static void decoder_close(lv_image_decoder_t * dec, lv_image_decoder_dsc_t * dsc);
static lv_draw_buf_t * decode_etc2_data(uint8_t *etc2_data, size_t etc2_data_size);

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
 * Register the ETC2 decoder functions in LVGL
 */
void lv_besetc2_init(void)
{
    lv_image_decoder_t * dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);
    dec->name = DECODER_NAME;
}

void lv_besetc2_deinit(void)
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
 * Get info about a ETC2 image
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       image descriptor containing the source and type of the image and other info.
 * @param header    image information is set in header parameter
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /* Unused */

    lv_image_src_t src_type = dsc->src_type;          /* Get the source type */

    if(src_type == LV_IMAGE_SRC_FILE || src_type == LV_IMAGE_SRC_VARIABLE) {
        uint16_t w, h;
        uint8_t buf[16];

        /* If it's a ETC2 file... */
        if(src_type == LV_IMAGE_SRC_FILE) {
            /* Read the width and height from the file. They have a constant location:
            * [8..9]:   encoded width, align to 4 pixels
            * [10..11]: encoded height, align to 4 pixels
            * [12..13]: orginal width
            * [14..15]: orginal height
            */
            uint32_t rn;
            lv_fs_read(&dsc->file, buf, sizeof(buf), &rn);

            if(rn != sizeof(buf)) return LV_RESULT_INVALID;

            if(lv_memcmp(buf, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;

            w = buf[8] << 8 | buf[9];
            h = buf[10] << 8 | buf[11];
        }
        /*If it's a ETC2 file in a  C array...*/
        else {
            const lv_image_dsc_t * img_dsc = dsc->src;
            const uint32_t data_size = img_dsc->data_size;
            const uint8_t *p = img_dsc->data;

            if(data_size < sizeof(magic)) return LV_RESULT_INVALID;
            if(lv_memcmp(p, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;

            w = p[8] << 8 | p[9];
            h = p[10] << 8 | p[11];
        }

        /*Save the data in the header*/
        header->cf = LV_COLOR_FORMAT_ARGB8888;
        /*The width and height are stored in Big endian format so convert them to little endian*/
        header->w = (int32_t)w;
        header->h = (int32_t)h;

        return LV_RESULT_OK;
    }

    return LV_RESULT_INVALID;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a ETC2 image and decode it into dsc.decoded
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    uint8_t * etc2_data = NULL;
    size_t etc2_data_size = 0;

    if(dsc->src_type == LV_IMAGE_SRC_FILE) {
        const char * fn = dsc->src;
        if(lv_strcmp(lv_fs_get_ext(fn), "pkm") != 0) {
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
        lv_fs_tell(&f, &etc2_data_size);
        lv_fs_seek(&f, 0, LV_FS_SEEK_SET);

        if (etc2_data_size == 0) {
            LV_LOG_ERROR("Can't get file size.");
            lv_fs_close(&f);
            return LV_RESULT_INVALID;
        }

        etc2_data = lv_malloc(etc2_data_size);
        if (etc2_data) {
            uint32_t rn;
            fsres = lv_fs_read(&f, etc2_data, etc2_data_size, &rn);
            if (fsres != LV_FS_RES_OK || rn == 0) {
                LV_LOG_WARN("Failed to read file: %d", fsres);
                lv_free(etc2_data);
                lv_fs_close(&f);
                return LV_RESULT_INVALID;
            }
        }

        lv_fs_close(&f);
    }
    else if(dsc->src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t * img_dsc = dsc->src;
        etc2_data = (uint8_t *)img_dsc->data;
        etc2_data_size = img_dsc->data_size;
    }
    else {
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * decoded = decode_etc2_data(etc2_data, etc2_data_size);

    if(dsc->src_type == LV_IMAGE_SRC_FILE) lv_free((void *)etc2_data);

    if(!decoded) {
        LV_LOG_WARN("Error decoding ETC2");
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
 * Close ETC2 image and free data
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

static lv_draw_buf_t * decode_etc2_data(uint8_t *etc2_data, size_t etc2_data_size)
{
    int w = 0, h = 0;
    uint8_t *p = etc2_data;
    lv_draw_buf_t *draw_buf = NULL;

    if (etc2_data == NULL || etc2_data_size <= 0) {
        LV_LOG_ERROR("etc2 data is NULL.");
        return NULL;
    }

    LV_LOG_TRACE("ETC2 data %p, size %d bytes", etc2_data, etc2_data_size);

    if(memcmp(etc2_data, magic, sizeof(magic)) != 0) {
        LV_LOG_ERROR("Invalid etc2pkm file.");
        return LV_RESULT_INVALID;
    }

    /* Get image size */
    w = p[8] << 8 | p[9];
    h = p[10] << 8 | p[11];
    if (w <= 0 || h <= 0) {
        LV_LOG_ERROR("Invalid etc2 size, %dx%d", w, h);
        return NULL;
    }

    draw_buf = draw_buf_create(w, h, LV_COLOR_FORMAT_ARGB8888, w);
    if (draw_buf == NULL) {
        LV_LOG_ERROR("Can't malloc draw_buf.");
        return NULL;
    }

    if (draw_buf->data_size < w * h) {
        LV_LOG_ERROR("Small draw_buf %u < %dx%d", draw_buf->data_size, w, h);
        draw_buf_destroy(draw_buf);
        return NULL;
    }

    lv_memcpy(draw_buf->data, &etc2_data[16], etc2_data_size - 16);

    hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)draw_buf->data),
            LEN_ALIGN_TO_CACHE((etc2_data_size - 16), (uint32_t)draw_buf->data));

    LV_LOG_TRACE("ETC2 decode done, %dx%d", w, h);
    return draw_buf;
}

#endif /*LV_USE_BESETC2*/
