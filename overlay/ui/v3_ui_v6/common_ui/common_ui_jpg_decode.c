#include "common_ui_jpg_decode.h"
#include "lvgl.h" // IWYU pragma: keep
#include "jpeg_drv.h"
#include "gpu_port.h"

/*
* DEFINES
*****************************************************************************************
*/
#define ALIGN16(n) ((n + 15U) / 16U * 16U)
#define GETC(p) ((*p++) & 0xff)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t common_ui_jpg_clip_i32(int32_t x)
{
    if (x < 0) {
        return 0;
    }
    if (x > 255) {
        return 255;
    }
    return (uint8_t)x;
}

static void common_ui_jpg_yuv_to_rgb(uint8_t y, uint8_t u, uint8_t v,
                                     uint8_t *r, uint8_t *g, uint8_t *b)
{
    const int32_t c = (int32_t)y - 16;
    const int32_t d = (int32_t)u - 128;
    const int32_t e = (int32_t)v - 128;
    const int32_t cc = c < 0 ? 0 : c;

    const int32_t rr = (298 * cc + 409 * e + 128) >> 8;
    const int32_t gg = (298 * cc - 100 * d - 208 * e + 128) >> 8;
    const int32_t bb = (298 * cc + 516 * d + 128) >> 8;

    *r = common_ui_jpg_clip_i32(rr);
    *g = common_ui_jpg_clip_i32(gg);
    *b = common_ui_jpg_clip_i32(bb);
}

static int common_ui_jpg_get_info(const uint8_t *jpg_data, size_t jpg_size, uint16_t *width, uint16_t *height)
{
    uint8_t v;
    uint8_t *p = (uint8_t *)jpg_data;
    uint8_t *end = p + jpg_size;

    if (jpg_data == NULL || jpg_size < 4U || width == NULL || height == NULL) {
        return -1;
    }

    if (p[0] != 0xff || p[1] != 0xd8) {
        return -2;
    }

    /* Skip SOI marker (0xFFD8) first; SOI has no length field. */
    p += 2;

    while (p < end) {
        int m;
        int len;

        if (!(p[0] == 0xff && p[1] > 0 && p[1] < 0xff)) {
            p++;
            continue;
        }

        m = p[1];
        /* Markers without length field. */
        if (m == 0xd8 || m == 0xd9 || (m >= 0xd0 && m <= 0xd7) || m == 0x01) {
            p += 2;
            continue;
        }

        if (m >= 0xc0 && m <= 0xc3) { /* Start of frame */
            int bpp;
            int color_num;

            p += 4;
            if (p >= end) {
                return -3;
            }

            bpp = GETC(p);
            if (bpp != 8) {
                return -4;
            }

            if ((p + 4) > end) {
                return -5;
            }

            v = GETC(p);
            *height = (uint16_t)((v << 8) | GETC(p));
            v = GETC(p);
            *width = (uint16_t)((v << 8) | GETC(p));

            if (p >= end) {
                return -6;
            }

            color_num = GETC(p);
            if (color_num != 3) {
                return -7;
            }
            return 0;
        }

        if ((p + 4) > end) {
            return -8;
        }

        len = (p[2] << 8) | p[3];
        if (len < 2) {
            return -9;
        }
        p += len + 2;
    }

    return -10;
}

static int common_ui_jpg_bes_decode_to_yuy2(const uint8_t *jpg_data, size_t jpg_size, lv_draw_buf_t **yuy2_buf_out,
                                             uint32_t *w_out, uint32_t *h_out)
{
    uint16_t w = 0;
    uint16_t h = 0;
    uint32_t aligned_w;
    uint32_t aligned_h;
    uint32_t pix_size;
    struct jpeg_handle_t handle;
    uint8_t *tmp_buf = NULL;
    uint8_t *ecs_buf;
    uint32_t ecs_bufsz;
    lv_draw_buf_t *yuy2_buf = NULL;
    int ret;

    if (jpg_data == NULL || jpg_size == 0U || yuy2_buf_out == NULL || w_out == NULL || h_out == NULL) {
        return -1;
    }

    *yuy2_buf_out = NULL;
    *w_out = 0;
    *h_out = 0;

    ret = common_ui_jpg_get_info(jpg_data, jpg_size, &w, &h);
    if (ret != 0) {
        return ret;
    }
    if (w == 0U || h == 0U) {
        return -11;
    }

    aligned_w = ALIGN16((uint32_t)w);
    aligned_h = ALIGN16((uint32_t)h);
    pix_size = aligned_w * aligned_h * 2U;

    tmp_buf = lv_malloc((uint32_t)jpg_size + 64U);
    if (tmp_buf == NULL) {
        return -3;
    }
    ecs_buf = (uint8_t *)(((uintptr_t)tmp_buf + 63U) & ~(uintptr_t)63U);
    ecs_bufsz = ((uint32_t)jpg_size + 63U) & ~(uint32_t)63U;

    yuy2_buf = lv_draw_buf_create(aligned_w, aligned_h, LV_COLOR_FORMAT_YUY2, aligned_w * 2U);
    if (yuy2_buf == NULL) {
        lv_free(tmp_buf);
        return -4;
    }
    if (yuy2_buf->data_size < pix_size) {
        lv_draw_buf_destroy(yuy2_buf);
        lv_free(tmp_buf);
        return -5;
    }

    lv_memzero(&handle, sizeof(handle));
    handle.jpeg_data = (uint8_t *)jpg_data;
    handle.jpeg_size = (uint32_t)jpg_size;
    handle.ecs_buf = ecs_buf;
    handle.ecs_bufsz = ecs_bufsz;
    handle.pix_data = (uint8_t *)gpu_hal_buf_addr((uint32_t)(uintptr_t)yuy2_buf->data);
    handle.pix_size = pix_size;
    handle.pix_fmt = PIX_YUV422_YUYV;

    jpeg_init();
    ret = jpeg_decode(&handle);
    jpeg_exit();

    lv_free(tmp_buf);

    if (ret != JPEG_OK) {
        lv_draw_buf_destroy(yuy2_buf);
        return -6;
    }

    yuy2_buf->header.w = handle.jpeg_width;
    yuy2_buf->header.h = handle.jpeg_height;
    yuy2_buf->header.stride = (uint32_t)handle.width * 2U;

    *yuy2_buf_out = yuy2_buf;
    *w_out = (uint32_t)handle.jpeg_width;
    *h_out = (uint32_t)handle.jpeg_height;

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int common_ui_jpg_decode(const uint8_t *jpg_data, size_t jpg_size, common_ui_jpg_decode_fmt_e fmt,
                         common_ui_jpg_decode_result_t *out)
{
    if (out == NULL || jpg_data == NULL || jpg_size == 0U) {
        return -1;
    }

    out->draw_buf = NULL;
    out->width = 0;
    out->height = 0;

    lv_draw_buf_t *yuy2_buf = NULL;
    uint32_t width = 0;
    uint32_t height = 0;

    int yuy2_ret = common_ui_jpg_bes_decode_to_yuy2(jpg_data, jpg_size, &yuy2_buf, &width, &height);
    if (yuy2_ret != 0 || yuy2_buf == NULL) {
        return yuy2_ret != 0 ? yuy2_ret : -2;
    }

    lv_draw_buf_t *rgb_buf = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
    if (rgb_buf == NULL) {
        lv_draw_buf_destroy(yuy2_buf);
        return -5;
    }

    const uint32_t src_stride = yuy2_buf->header.stride;
    const uint32_t dst_stride = rgb_buf->header.stride;
    const uint8_t *src_base = yuy2_buf->data;

    for (uint32_t y = 0; y < height; y++) {
        const uint8_t *src_row = src_base + y * src_stride;
        uint8_t *dst_row = rgb_buf->data + y * dst_stride;

        for (uint32_t x = 0; x < width; x += 2U) {
            const uint8_t y0 = src_row[x * 2U + 0U];
            const uint8_t u = src_row[x * 2U + 1U];
            const uint8_t y1 = src_row[x * 2U + 2U];
            const uint8_t v = src_row[x * 2U + 3U];

            uint8_t r0, g0, b0;
            uint8_t r1, g1, b1;
            common_ui_jpg_yuv_to_rgb(y0, u, v, &r0, &g0, &b0);
            common_ui_jpg_yuv_to_rgb(y1, u, v, &r1, &g1, &b1);

            dst_row[x * 3U + 0U] = r0;
            dst_row[x * 3U + 1U] = g0;
            dst_row[x * 3U + 2U] = b0;

            if ((x + 1U) < width) {
                dst_row[(x + 1U) * 3U + 0U] = r1;
                dst_row[(x + 1U) * 3U + 1U] = g1;
                dst_row[(x + 1U) * 3U + 2U] = b1;
            }
        }

        if (dst_stride > width * 3U) {
            lv_memset(dst_row + width * 3U, 0, dst_stride - width * 3U);
        }
    }

    lv_draw_buf_destroy(yuy2_buf);

    if (fmt == COMMON_UI_JPG_DECODE_RGB888) {
        out->draw_buf = rgb_buf;
        out->width = width;
        out->height = height;
        return 0;
    }

    if (fmt == COMMON_UI_JPG_DECODE_RGBA8888) {
        lv_draw_buf_t *rgba_buf =
            lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
        if (rgba_buf == NULL) {
            lv_draw_buf_destroy(rgb_buf);
            return -7;
        }

        const uint32_t src_stride = rgb_buf->header.stride;
        const uint32_t rgba_stride = rgba_buf->header.stride;
        const uint32_t px_per_line = width;

        for (uint32_t y = 0; y < height; y++) {
            const uint8_t *src_row = rgb_buf->data + y * src_stride;
            lv_color32_t *dst_row =
                (lv_color32_t *)((uint8_t *)rgba_buf->data + y * rgba_stride);

            for (uint32_t x = 0; x < px_per_line; x++) {
                const uint8_t r = src_row[x * 3U + 0U];
                const uint8_t g = src_row[x * 3U + 1U];
                const uint8_t b = src_row[x * 3U + 2U];

                dst_row[x].red = r;
                dst_row[x].green = g;
                dst_row[x].blue = b;
                dst_row[x].alpha = 0xFF;
            }

            if (rgba_stride > px_per_line * sizeof(lv_color32_t)) {
                lv_memset((uint8_t *)dst_row + px_per_line * sizeof(lv_color32_t), 0,
                          rgba_stride - px_per_line * sizeof(lv_color32_t));
            }
        }

        lv_draw_buf_destroy(rgb_buf);

        out->draw_buf = rgba_buf;
        out->width = width;
        out->height = height;
        return 0;
    }

    lv_draw_buf_destroy(rgb_buf);
    return -8;
}

void common_ui_jpg_decode_release(lv_draw_buf_t *draw_buf)
{
    if (draw_buf != NULL) {
        lv_draw_buf_destroy(draw_buf);
    }
}

