#include "ui_img_baj_decode.h"
#include "ui_img_resource.h" // IWYU pragma: keep
#include "bajdec.h" // IWYU pragma: keep
#include "lvgl.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int ui_img_baj_decode_to_rgba(const uint8_t *baj_data, size_t baj_size, lv_draw_buf_t **rgba_out,
                                        uint32_t *w_out, uint32_t *h_out)
{
    int org_w = 0;
    int org_h = 0;
    int aligned_w = 0;
    int aligned_h = 0;
    uint8_t *tmp_buf = NULL;
    uint8_t *aligned_buf;
    uint32_t tmp_buf_sz;
    lv_draw_buf_t *rgba_buf = NULL;
    int ret;

    if (baj_data == NULL || baj_size == 0U || rgba_out == NULL || w_out == NULL || h_out == NULL) {
        return -1;
    }

    *rgba_out = NULL;
    *w_out = 0;
    *h_out = 0;

    ret = baj_get_info((uint8_t *)baj_data, (uint32_t)baj_size, &org_w, &org_h, &aligned_w, &aligned_h);
    if (ret != BAJ_OK) {
        return ret;
    }
    if (org_w <= 0 || org_h <= 0 || aligned_w <= 0 || aligned_h <= 0) {
        return -2;
    }

    tmp_buf_sz = (uint32_t)aligned_w * (uint32_t)aligned_h * 7U;
    tmp_buf = lv_malloc(tmp_buf_sz + 64U);
    if (tmp_buf == NULL) {
        return -3;
    }
    aligned_buf = (uint8_t *)(((uintptr_t)tmp_buf + 63U) & ~(uintptr_t)63U);

    rgba_buf = lv_draw_buf_create((uint32_t)org_w, (uint32_t)org_h, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    if (rgba_buf == NULL) {
        lv_free(tmp_buf);
        return -4;
    }

    ret = baj_decode((uint8_t *)baj_data, (uint32_t)baj_size, rgba_buf->data, aligned_buf, tmp_buf_sz);
    lv_free(tmp_buf);

    if (ret != BAJ_OK) {
        lv_draw_buf_destroy(rgba_buf);
        return ret;
    }

    *rgba_out = rgba_buf;
    *w_out = (uint32_t)org_w;
    *h_out = (uint32_t)org_h;
    return 0;
}

typedef struct {
    lv_draw_buf_t *draw_buf;
    uint32_t width;
    uint32_t height;
} ui_img_baj_decode_result_t;

static void ui_img_baj_decode_release(lv_draw_buf_t *draw_buf)
{
    if (draw_buf != NULL) {
        lv_draw_buf_destroy(draw_buf);
    }
}

static int ui_img_baj_decode(const uint8_t *baj_data, size_t baj_size, ui_img_baj_decode_fmt_e fmt,
                                ui_img_baj_decode_result_t *out)
{
    if (out == NULL || baj_data == NULL || baj_size == 0U) {
        return -1;
    }

    out->draw_buf = NULL;
    out->width = 0;
    out->height = 0;

    lv_draw_buf_t *rgba_src = NULL;
    uint32_t w = 0;
    uint32_t h = 0;

    int ret = ui_img_baj_decode_to_rgba(baj_data, baj_size, &rgba_src, &w, &h);
    if (ret != 0 || rgba_src == NULL) {
        return ret != 0 ? ret : -2;
    }

    if (fmt == UI_IMG_BAJ_DECODE_RGBA8888) {
        out->draw_buf = rgba_src;
        out->width = w;
        out->height = h;
        return 0;
    }

    if (fmt == UI_IMG_BAJ_DECODE_RGB888) {
        lv_draw_buf_t *rgb = lv_draw_buf_create(w, h, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
        if (rgb == NULL) {
            lv_draw_buf_destroy(rgba_src);
            return -3;
        }

        const uint32_t src_stride = rgba_src->header.stride;
        const uint32_t dst_stride = rgb->header.stride;
        const lv_color32_t *src_base = (const lv_color32_t *)rgba_src->data;

        for (uint32_t y = 0; y < h; y++) {
            const lv_color32_t *src_row = (const lv_color32_t *)((const uint8_t *)src_base + y * src_stride);
            uint8_t *dst_row = rgb->data + y * dst_stride;
            for (uint32_t x = 0; x < w; x++) {
                dst_row[x * 3U + 0U] = src_row[x].red;
                dst_row[x * 3U + 1U] = src_row[x].green;
                dst_row[x * 3U + 2U] = src_row[x].blue;
            }
            if (dst_stride > w * 3U) {
                lv_memset(dst_row + w * 3U, 0, dst_stride - w * 3U);
            }
        }

        lv_draw_buf_destroy(rgba_src);
        out->draw_buf = rgb;
        out->width = w;
        out->height = h;
        return 0;
    }

    lv_draw_buf_destroy(rgba_src);
    return -4;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_img_baj_release_decoded_buf(lv_obj_t *obj)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return;
    }
    const void *src = lv_image_get_src(obj);
    if (src != NULL && lv_image_src_get_type(src) == LV_IMAGE_SRC_VARIABLE) {
        lv_image_set_src(obj, NULL);
        ui_img_baj_decode_release((lv_draw_buf_t *)src);
    }
}

int ui_img_baj_set_src(lv_obj_t *obj, const uint8_t *baj_data, size_t baj_size,
                                ui_img_baj_decode_fmt_e fmt)
{
    if (obj == NULL || !lv_obj_is_valid(obj) || baj_data == NULL || baj_size == 0U) {
        return -1;
    }

    ui_img_baj_release_decoded_buf(obj);

    ui_img_baj_decode_result_t dec;
    int ret = ui_img_baj_decode(baj_data, baj_size, fmt, &dec);
    if (ret == 0) {
        lv_image_set_src(obj, dec.draw_buf);
        return 0;
    }

    lv_fs_path_ex_t img_path;
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER, baj_data, baj_size, "baj");
    lv_image_set_src(obj, &img_path);
    return ret;
}

int ui_img_baj_theme_set_src(ui_manager_context_t *thiz, lv_obj_t *obj, uint16_t resource_id,
                                      res_theme_type_e theme_type, ui_img_baj_decode_fmt_e fmt)
{
    if (thiz == NULL || obj == NULL) {
        return -1;
    }

    res_img_info_t img_info = {0};
    ui_theme_img_resource_init(thiz, resource_id, theme_type, &img_info);
    if (img_info.data == NULL) {
        return -2;
    }

    int ret = ui_img_baj_set_src(obj, img_info.data, img_info.size, fmt);
    ui_img_resource_deinit(thiz, &img_info);
    return ret;
}

lv_draw_buf_t *ui_img_baj_decode_draw_buf(const uint8_t *baj_data, size_t baj_size,
    ui_img_baj_decode_fmt_e fmt)
{
    ui_img_baj_decode_result_t dec;
    int ret;

    if (baj_data == NULL || baj_size == 0U) {
        return NULL;
    }

    ret = ui_img_baj_decode(baj_data, baj_size, fmt, &dec);
    if (ret != 0) {
        return NULL;
    }
    return dec.draw_buf;
}

lv_draw_buf_t *ui_img_baj_theme_decode_draw_buf(ui_manager_context_t *thiz, uint16_t resource_id,
    res_theme_type_e theme_type, ui_img_baj_decode_fmt_e fmt)
{
    res_img_info_t img_info = {0};
    lv_draw_buf_t *draw_buf;

    if (thiz == NULL) {
        return NULL;
    }

    ui_theme_img_resource_init(thiz, resource_id, theme_type, &img_info);
    if (img_info.data == NULL) {
        return NULL;
    }

    draw_buf = ui_img_baj_decode_draw_buf(img_info.data, img_info.size, fmt);
    ui_img_resource_deinit(thiz, &img_info);
    return draw_buf;
}
