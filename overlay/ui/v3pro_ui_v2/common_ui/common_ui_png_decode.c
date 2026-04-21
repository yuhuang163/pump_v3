/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "common_ui_png_decode.h"
#include "lvgl.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
unsigned lodepng_decode32(unsigned char **out, unsigned *w, unsigned *h, const unsigned char *in, size_t insize);
static void common_ui_png_swap_rb(uint8_t *data, uint32_t px_cnt)
{
    lv_color32_t *px = (lv_color32_t *)data;
    for (uint32_t i = 0; i < px_cnt; i++) {
        uint8_t b = px[i].blue;
        px[i].blue = px[i].red;
        px[i].red = b;
    }
}

int common_ui_png_decode(const uint8_t *png_data, size_t png_size, common_ui_png_decode_fmt_e fmt,
                         common_ui_png_decode_result_t *out)
{
    if (out == NULL || png_data == NULL || png_size == 0) {
        return -1;
    }

    out->draw_buf = NULL;
    out->width = 0;
    out->height = 0;

    lv_draw_buf_t *rgba_src = NULL;
    unsigned w = 0;
    unsigned h = 0;
    unsigned err = lodepng_decode32((unsigned char **)(void *)&rgba_src, &w, &h, png_data, png_size);
    if (err != 0 || rgba_src == NULL) {
        if (rgba_src != NULL) {
            lv_draw_buf_destroy(rgba_src);
        }
        return err != 0 ? (int)err : -2;
    }

    common_ui_png_swap_rb(rgba_src->data, (uint32_t)w * (uint32_t)h);

    if (fmt == COMMON_UI_PNG_DECODE_RGBA8888) {
        out->draw_buf = rgba_src;
        out->width = (uint32_t)w;
        out->height = (uint32_t)h;
        return 0;
    }

    if (fmt == COMMON_UI_PNG_DECODE_RGB888) {
        lv_draw_buf_t *rgb = lv_draw_buf_create((uint32_t)w, (uint32_t)h, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
        if (rgb == NULL) {
            lv_draw_buf_destroy(rgba_src);
            return -3;
        }

        const uint32_t src_stride = rgba_src->header.stride;
        const uint32_t dst_stride = rgb->header.stride;
        const lv_color32_t *src_base = (const lv_color32_t *)rgba_src->data;

        for (uint32_t y = 0; y < (uint32_t)h; y++) {
            const lv_color32_t *src_row = (const lv_color32_t *)((const uint8_t *)src_base + y * src_stride);
            uint8_t *dst_row = rgb->data + y * dst_stride;
            for (uint32_t x = 0; x < (uint32_t)w; x++) {
                dst_row[x * 3 + 0] = src_row[x].red;
                dst_row[x * 3 + 1] = src_row[x].green;
                dst_row[x * 3 + 2] = src_row[x].blue;
            }
            if (dst_stride > (uint32_t)w * 3U) {
                lv_memset(dst_row + (uint32_t)w * 3U, 0, dst_stride - (uint32_t)w * 3U);
            }
        }

        lv_draw_buf_destroy(rgba_src);
        out->draw_buf = rgb;
        out->width = (uint32_t)w;
        out->height = (uint32_t)h;
        return 0;
    }

    lv_draw_buf_destroy(rgba_src);
    return -4;
}

void common_ui_png_decode_release(lv_draw_buf_t *draw_buf)
{
    if (draw_buf != NULL) {
        lv_draw_buf_destroy(draw_buf);
    }
}
