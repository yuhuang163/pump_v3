/**
 * @file lv_bespng.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_BESPNG
#include "src/draw/lv_image_decoder_private.h"
#include "src/core/lv_global.h"

#include "lv_bespng.h"
#include "lodepng.h"
#include <stdlib.h>
#include "hal_png.h"
#include "hal_timer.h"
#include "hal_cache.h"


/*********************
 *      DEFINES
 *********************/

#if defined(PNG_BASE) || defined(CHIP_HAS_PNG)
#define PNG_HW_DECODE
#endif

#define TIMEOUT_MS      100
#define DECODER_NAME    "BESPNG"

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
static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size);

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
 * Register the PNG decoder functions in LVGL
 */
void lv_bespng_init(void)
{
    lv_image_decoder_t * dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);
    dec->name = DECODER_NAME;
}

void lv_bespng_deinit(void)
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
 * Get info about a PNG image
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       image descriptor containing the source and type of the image and other info.
 * @param header    image information is set in header parameter
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /*Unused*/

    lv_image_src_t src_type = dsc->src_type;          /*Get the source type*/

    if(src_type == LV_IMAGE_SRC_FILE || src_type == LV_IMAGE_SRC_VARIABLE) {
        uint32_t * size;
        static const uint8_t magic[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        uint8_t buf[24];

        /*If it's a PNG file...*/
        if(src_type == LV_IMAGE_SRC_FILE) {
            /* Read the width and height from the file. They have a constant location:
            * [16..19]: width
            * [20..23]: height
            */
            uint32_t rn;
            lv_fs_read(&dsc->file, buf, sizeof(buf), &rn);

            if(rn != sizeof(buf)) return LV_RESULT_INVALID;

            if(lv_memcmp(buf, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;

            size = (uint32_t *)&buf[16];
        }
        /*If it's a PNG file in a  C array...*/
        else {
            const lv_image_dsc_t * img_dsc = dsc->src;
            const uint32_t data_size = img_dsc->data_size;
            size = ((uint32_t *)img_dsc->data) + 4;

            if(data_size < sizeof(magic)) return LV_RESULT_INVALID;
            if(lv_memcmp(img_dsc->data, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;
        }

        /*Save the data in the header*/
        header->cf = LV_COLOR_FORMAT_ARGB8888;
        /*The width and height are stored in Big endian format so convert them to little endian*/
        header->w = (int32_t)((size[0] & 0xff000000) >> 24) + ((size[0] & 0x00ff0000) >> 8);
        header->h = (int32_t)((size[1] & 0xff000000) >> 24) + ((size[1] & 0x00ff0000) >> 8);

        return LV_RESULT_OK;
    }

    return LV_RESULT_INVALID;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a PNG image and decode it into dsc.decoded
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    const uint8_t * png_data = NULL;
    size_t png_data_size = 0;
    if(dsc->src_type == LV_IMAGE_SRC_FILE) {
        const char * fn = dsc->src;
        if(lv_strcmp(lv_fs_get_ext(fn), "png") == 0) {              /*Check the extension*/
            unsigned error;
            error = lodepng_load_file((void *)&png_data, &png_data_size, fn);  /*Load the file*/
            if(error) {
                if(png_data != NULL) {
                    lv_free((void *)png_data);
                }
                LV_LOG_WARN("error %u: %s\n", error, lodepng_error_text(error));
                return LV_RESULT_INVALID;
            }
        }
    }
    else if(dsc->src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t * img_dsc = dsc->src;
        png_data = img_dsc->data;
        png_data_size = img_dsc->data_size;
    }
    else {
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * decoded = decode_png_data(png_data, png_data_size);

    if(dsc->src_type == LV_IMAGE_SRC_FILE) lv_free((void *)png_data);

    if(!decoded) {
        LV_LOG_WARN("Error decoding PNG");
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
 * Close PNG image and free data
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

/************************** PNG Hardware decode *******************************/
#ifdef PNG_HW_DECODE
static int bespng_get_info(uint8_t *png_data, uint32_t png_size, int *width, int *height)
{
    int comp_level = 0;
    const uint8_t png_hdr[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    uint8_t *data = png_data;
    uint8_t *p;

    for (int i = 0; i < sizeof(png_hdr); i++) {
        if (data[i] != png_hdr[i])
            return -1;
    }

#define PNG_GET32(p, n)  ((p[n] << 24) | (p[n + 1] << 16) | (p[n + 2] << 8) | p[n + 3])

    p = data + 8;
    while (p - data < png_size) {
        uint32_t chunk_len = PNG_GET32(p, 0);
        if (p[4] == 'I' && p[5] == 'H' && p[6] == 'D' && p[7] == 'R') {
            /* IHDR, parse image info */
            *width = PNG_GET32(p, 8);
            *height = PNG_GET32(p, 12);
        } else if (p[4] == 'I' && p[5] == 'D' && p[6] == 'A' && p[7] == 'T') {
            /* IDAT, get zlib compress level */
            if (p[8] == 0x78 && p[9] == 0x01) {
                if (p[10] & 0x4)
                    comp_level = 1;
                else
                    comp_level = 0;
            } else if (p[8] == 0x78 && p[9] == 0x5E) {
                comp_level = 1;
            } else if (p[8] == 0x78 && p[9] == 0x9C) {
                comp_level = 2;
            } else if (p[8] == 0x78 && p[9] == 0xDA) {
                comp_level = 3;
            } else {
                comp_level = 0xff;
            }
            break;
        }
        p += 12 + chunk_len;
    }
    LV_LOG_TRACE("PNG image %dx%d, compress level %d", *width, *height, comp_level);
    return (comp_level > 0) ? -1 : 0;
}

static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size)
{
    struct HAL_PNG_CONFIG_T cfg;
    int timeout = 0;
    int w = 0, h = 0;
    lv_draw_buf_t *draw_buf = NULL;
    int i;

    if (png_data == NULL || png_data_size <= 0) {
        LV_LOG_ERROR("png data is NULL.");
        return NULL;
    }

    LV_LOG_TRACE("PNG data %p, size %d bytes", png_data, png_data_size);

    if (bespng_get_info((uint8_t *)png_data, 1024, &w, &h) != 0) {
        LV_LOG_ERROR("Invalid png data.");
        return NULL;
    }
    if (w <= 0 || h <= 0) {
        LV_LOG_ERROR("Invalid png size, %dx%d", w, h);
        return NULL;
    }

    draw_buf = draw_buf_create(w, h, LV_COLOR_FORMAT_ARGB8888, w * 4);
    if (draw_buf == NULL) {
        LV_LOG_ERROR("Can't malloc draw_buf.");
        return NULL;
    }

    if (draw_buf->data_size < w * h * 4) {
        LV_LOG_ERROR("Small draw_buf %u < %dx%dx4", draw_buf->data_size, w, h);
        draw_buf_destroy(draw_buf);
        return NULL;
    }

    hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)png_data),
            LEN_ALIGN_TO_CACHE(png_data_size, (uint32_t)png_data));
    hal_cache_sync_invalidate(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)draw_buf->data),
            LEN_ALIGN_TO_CACHE((uint32_t)draw_buf->data_size, (uint32_t)draw_buf->data));

    hal_png_open();

    lv_memset(&cfg, 0, sizeof(cfg));
    cfg.png_data = (uint8_t *)png_data;
    cfg.png_size = png_data_size;
    cfg.pix_buf = draw_buf->data;
    cfg.pix_fmt = HAL_PNG_PIX_RGBA8888;

    if (hal_png_start(&cfg) != HAL_PNG_OK) {
        LV_LOG_ERROR("Decode start failed. %d", cfg.result);
        goto error;
    }

    for (i = 0; i < TIMEOUT_MS; i++) {
        if (!hal_png_busy())
            break;
        lv_delay_ms(1);
    }
    if (i >= TIMEOUT_MS)
        timeout = 1;

    if (timeout) {
        LV_LOG_ERROR("PNG decode timeout!");
        goto error;
    } else if (cfg.result != HAL_PNG_OK) {
        LV_LOG_ERROR("PNG decode failed, result = %d.", cfg.result);
        goto error;
    }

    LV_LOG_TRACE("PNG decode done, %dx%d", cfg.img_width, cfg.img_height);
    return draw_buf;

error:
    hal_png_stop();
    hal_png_close();
    draw_buf_destroy(draw_buf);
    return NULL;
}
#else
/************************** PNG Software decode *******************************/
static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size)
{
    unsigned png_width;             /*Not used, just required by the decoder*/
    unsigned png_height;            /*Not used, just required by the decoder*/
    lv_draw_buf_t * decoded = NULL;

    /*Decode the image in ARGB8888 */
    unsigned error = lodepng_decode32((unsigned char **)&decoded, &png_width, &png_height, png_data, png_data_size);
    if(error) {
        if(decoded != NULL)  draw_buf_destroy(decoded);
        return NULL;
    }
    hal_cache_sync(HAL_CACHE_ID_D_CACHE, ADDR_ALIGN_TO_CACHE((uint32_t)decoded->data),
            LEN_ALIGN_TO_CACHE((uint32_t)decoded->data_size, (uint32_t)decoded->data));
    return decoded;
}
#endif /* !PNG_HW_DECODE */

#endif /*LV_USE_BESPNG*/
