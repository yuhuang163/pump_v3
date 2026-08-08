#include "ui_img_png_decode.h"
#include "ui_img_resource.h" // IWYU pragma: keep
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
static void ui_img_png_swap_rb(uint8_t *data, uint32_t px_cnt)
{
    lv_color32_t *px = (lv_color32_t *)data;
    for (uint32_t i = 0; i < px_cnt; i++) {
        uint8_t b = px[i].blue;
        px[i].blue = px[i].red;
        px[i].red = b;
    }
}

typedef struct {
    lv_draw_buf_t *draw_buf;
    uint32_t width;
    uint32_t height;
} ui_img_png_decode_result_t;

static void ui_img_png_decode_release(lv_draw_buf_t *draw_buf)
{
    if (draw_buf != NULL) {
        lv_draw_buf_destroy(draw_buf);
    }
}

static int ui_img_png_decode(const uint8_t *png_data, size_t png_size, ui_img_png_decode_fmt_e fmt,
                                ui_img_png_decode_result_t *out)
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

    ui_img_png_swap_rb(rgba_src->data, (uint32_t)w * (uint32_t)h);

    if (fmt == UI_IMG_PNG_DECODE_RGBA8888) {
        out->draw_buf = rgba_src;
        out->width = (uint32_t)w;
        out->height = (uint32_t)h;
        return 0;
    }

    if (fmt == UI_IMG_PNG_DECODE_RGB888) {
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
                /* LV_COLOR_FORMAT_RGB888 在本平台按 BGR 字节序（VGLITE BGR888） */
                dst_row[x * 3 + 0] = src_row[x].blue;
                dst_row[x * 3 + 1] = src_row[x].green;
                dst_row[x * 3 + 2] = src_row[x].red;
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

    if (fmt == UI_IMG_PNG_DECODE_NATIVE) {
        lv_draw_buf_t *native = lv_draw_buf_create((uint32_t)w, (uint32_t)h, LV_COLOR_FORMAT_NATIVE,
            LV_STRIDE_AUTO);
        if (native == NULL) {
            lv_draw_buf_destroy(rgba_src);
            return -3;
        }

        const uint32_t src_stride = rgba_src->header.stride;
        const uint32_t dst_stride = native->header.stride;
        const lv_color32_t *src_base = (const lv_color32_t *)rgba_src->data;
        const uint32_t px_size = lv_color_format_get_size(LV_COLOR_FORMAT_NATIVE);

        for (uint32_t y = 0; y < (uint32_t)h; y++) {
            const lv_color32_t *src_row = (const lv_color32_t *)((const uint8_t *)src_base + y * src_stride);
            uint8_t *dst_row = native->data + y * dst_stride;
            for (uint32_t x = 0; x < (uint32_t)w; x++) {
#if LV_COLOR_DEPTH == 16
                lv_color_t c = lv_color_make(src_row[x].red, src_row[x].green, src_row[x].blue);
                ((uint16_t *)dst_row)[x] = lv_color_to_u16(c);
#elif LV_COLOR_DEPTH == 24
                /* NATIVE=RGB888，本平台 VGLITE 按 BGR888 取色，须 B,G,R 落盘 */
                dst_row[x * 3U + 0U] = src_row[x].blue;
                dst_row[x * 3U + 1U] = src_row[x].green;
                dst_row[x * 3U + 2U] = src_row[x].red;
#elif LV_COLOR_DEPTH == 32
                lv_color_t c = lv_color_make(src_row[x].red, src_row[x].green, src_row[x].blue);
                ((uint32_t *)dst_row)[x] = lv_color_to_u32(c);
#else
#error "Unsupported LV_COLOR_DEPTH for UI_IMG_PNG_DECODE_NATIVE"
#endif
            }
            if (dst_stride > (uint32_t)w * px_size) {
                lv_memset(dst_row + (uint32_t)w * px_size, 0, dst_stride - (uint32_t)w * px_size);
            }
        }

        lv_draw_buf_destroy(rgba_src);
        out->draw_buf = native;
        out->width = (uint32_t)w;
        out->height = (uint32_t)h;
        return 0;
    }

    lv_draw_buf_destroy(rgba_src);
    return -4;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_img_png_release_decoded_buf(lv_obj_t *obj)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    const void *src = lv_image_get_src(obj);
    if (src != NULL && lv_image_src_get_type(src) == LV_IMAGE_SRC_VARIABLE) {
        lv_image_set_src(obj, NULL);
        ui_img_png_decode_release((lv_draw_buf_t *)src);
    }
}

lv_draw_buf_t *ui_img_png_decode_draw_buf(const uint8_t *png_data, size_t png_size,
                                          ui_img_png_decode_fmt_e fmt)
{
    ui_img_png_decode_result_t dec;
    if (ui_img_png_decode(png_data, png_size, fmt, &dec) != 0) {
        return NULL;
    }
    return dec.draw_buf;
}

lv_draw_buf_t *ui_img_png_theme_decode_draw_buf(ui_manager_context_t *thiz, uint16_t resource_id,
                                                res_theme_type_e theme_type, ui_img_png_decode_fmt_e fmt)
{
    if (thiz == NULL) {
        return NULL;
    }

    res_img_info_t img_info = {0};
    ui_theme_img_resource_init(thiz, resource_id, theme_type, &img_info);
    if (img_info.data == NULL) {
        return NULL;
    }

    lv_draw_buf_t *draw_buf = ui_img_png_decode_draw_buf(img_info.data, img_info.size, fmt);
    ui_img_resource_deinit(thiz, &img_info);
    return draw_buf;
}

int ui_img_png_set_src(lv_obj_t *obj, const uint8_t *png_data, size_t png_size,
                                ui_img_png_decode_fmt_e fmt)
{
    if (obj == NULL || !lv_obj_is_valid(obj) || png_data == NULL || png_size == 0U) {
        return -1;
    }

    ui_img_png_release_decoded_buf(obj);

    ui_img_png_decode_result_t dec;
    int ret = ui_img_png_decode(png_data, png_size, fmt, &dec);
    if (ret == 0) {
        lv_image_set_src(obj, dec.draw_buf);
        return 0;
    }

    lv_fs_path_ex_t img_path;
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER, png_data, png_size, "png");
    lv_image_set_src(obj, &img_path);
    return ret;
}

int ui_img_png_theme_set_src(ui_manager_context_t *thiz, lv_obj_t *obj, uint16_t resource_id,
                                      res_theme_type_e theme_type, ui_img_png_decode_fmt_e fmt)
{
    if (thiz == NULL || obj == NULL) {
        return -1;
    }

    res_img_info_t img_info = {0};
    ui_theme_img_resource_init(thiz, resource_id, theme_type, &img_info);
    if (img_info.data == NULL) {
        return -2;
    }

    int ret = ui_img_png_set_src(obj, img_info.data, img_info.size, fmt);
    ui_img_resource_deinit(thiz, &img_info);
    return ret;
}
