/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_tip_shine_soft.h" // IWYU pragma: keep

#if IS_SUPPORT_LVGL
#include "lvgl.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint32_t ui_tip_shine_smoothstep_u8(uint32_t t)
{
    /* t in 0..255 → t^2*(3-2t) */
    return (t * t * (765U - 2U * t)) / (255U * 255U);
}

static void ui_tip_shine_profile_smoothstep(uint8_t *profile, uint32_t profile_w)
{
    uint32_t fade;
    uint32_t x;

    if (profile == NULL || profile_w == 0U) {
        return;
    }
    fade = (profile_w * 40U) / 100U;
    if (fade < 1U) {
        fade = 1U;
    }
    for (x = 0; x < profile_w; x++) {
        uint32_t edge = 255U;
        if (x < fade) {
            edge = ui_tip_shine_smoothstep_u8((x * 255U) / fade);
        } else if (x + fade >= profile_w) {
            uint32_t d = profile_w - 1U - x;
            edge = ui_tip_shine_smoothstep_u8((d * 255U) / fade);
        }
        profile[x] = (uint8_t)edge;
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
lv_draw_buf_t *ui_tip_shine_buf_create(const lv_draw_buf_t *tip)
{
    lv_draw_buf_t *shine;
    uint32_t w;
    uint32_t h;
    uint32_t y;
    uint32_t x;

    if (tip == NULL || tip->data == NULL) {
        return NULL;
    }
    if (tip->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        tip->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return NULL;
    }

    w = tip->header.w;
    h = tip->header.h;
    if (w == 0U || h == 0U) {
        return NULL;
    }

    shine = lv_draw_buf_create(w, h, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    if (shine == NULL || shine->data == NULL) {
        if (shine != NULL) {
            lv_draw_buf_destroy(shine);
        }
        return NULL;
    }

    for (y = 0; y < h; y++) {
        const lv_color32_t *src =
            (const lv_color32_t *)(tip->data + y * tip->header.stride);
        lv_color32_t *dst = (lv_color32_t *)(shine->data + y * shine->header.stride);
        for (x = 0; x < w; x++) {
            dst[x].red = 0xFF;
            dst[x].green = 0xFF;
            dst[x].blue = 0xFF;
            dst[x].alpha = src[x].alpha;
        }
    }
    lv_draw_buf_flush_cache(shine, NULL);
    return shine;
}

uint8_t *ui_tip_shine_alpha_dup(const lv_draw_buf_t *tip, uint32_t *out_w, uint32_t *out_h)
{
    uint8_t *alpha;
    uint32_t w;
    uint32_t h;
    uint32_t y;
    uint32_t x;

    if (out_w != NULL) {
        *out_w = 0;
    }
    if (out_h != NULL) {
        *out_h = 0;
    }
    if (tip == NULL || tip->data == NULL) {
        return NULL;
    }
    if (tip->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        tip->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return NULL;
    }

    w = tip->header.w;
    h = tip->header.h;
    if (w == 0U || h == 0U) {
        return NULL;
    }

    alpha = (uint8_t *)u_malloz(w * h);
    if (alpha == NULL) {
        return NULL;
    }

    for (y = 0; y < h; y++) {
        const lv_color32_t *src =
            (const lv_color32_t *)(tip->data + y * tip->header.stride);
        uint8_t *dst = alpha + y * w;
        for (x = 0; x < w; x++) {
            dst[x] = src[x].alpha;
        }
    }
    if (out_w != NULL) {
        *out_w = w;
    }
    if (out_h != NULL) {
        *out_h = h;
    }
    return alpha;
}

void ui_tip_shine_profile_build(const lv_draw_buf_t *mask_rgba, uint8_t *profile, uint32_t profile_w)
{
    uint32_t src_w;
    uint32_t src_h;
    uint32_t x0;
    uint32_t mid_y;
    uint32_t x;
    const lv_color32_t *row;

    if (profile == NULL || profile_w == 0U) {
        return;
    }

    if (mask_rgba == NULL || mask_rgba->data == NULL ||
        (mask_rgba->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
         mask_rgba->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) ||
        mask_rgba->header.w == 0U || mask_rgba->header.h == 0U) {
        ui_tip_shine_profile_smoothstep(profile, profile_w);
        /* 与 PNG 路径一致：中心略提亮 */
        for (x = 0; x < profile_w; x++) {
            uint32_t a = profile[x];
            uint32_t lift = (a * a * 20U) / (255U * 255U);
            a = a + lift;
            if (a > 255U) {
                a = 255U;
            }
            profile[x] = (uint8_t)a;
        }
        return;
    }

    src_w = mask_rgba->header.w;
    src_h = mask_rgba->header.h;
    x0 = (src_w > profile_w) ? ((src_w - profile_w) / 2U) : 0U;
    mid_y = src_h / 2U;
    row = (const lv_color32_t *)(mask_rgba->data + mid_y * mask_rgba->header.stride);

    for (x = 0; x < profile_w; x++) {
        uint32_t sx = (x0 + x < src_w) ? (x0 + x) : (src_w - 1U);
        profile[x] = row[sx].alpha;
    }
    /* 中心略提亮：按当前亮度加权抬升（两端几乎不动，峰附近约 +8%） */
    for (x = 0; x < profile_w; x++) {
        uint32_t a = profile[x];
        uint32_t lift = (a * a * 20U) / (255U * 255U); /* 0..20 */
        a = a + lift;
        if (a > 255U) {
            a = 255U;
        }
        profile[x] = (uint8_t)a;
    }
}

static void ui_tip_shine_apply_column(lv_draw_buf_t *shine, const uint8_t *alpha_orig,
    uint32_t tip_w, uint32_t tip_h, uint32_t x, uint32_t edge)
{
    uint32_t y;
    uint32_t stride = shine->header.stride;

    for (y = 0; y < tip_h; y++) {
        lv_color32_t *dst = (lv_color32_t *)(shine->data + y * stride);
        uint32_t cov = alpha_orig[y * tip_w + x];
        uint32_t a;

        if (cov == 0U || edge == 0U) {
            a = 0U;
        } else if (cov >= 128U) {
            a = edge;
        } else {
            a = (cov * edge) / 255U;
        }
        dst[x].alpha = (uint8_t)a;
    }
}

void ui_tip_shine_apply_profile(lv_draw_buf_t *shine, const uint8_t *alpha_orig,
    uint32_t tip_w, uint32_t tip_h, int32_t shine_x, const uint8_t *profile, uint32_t profile_w)
{
    int32_t x;

    if (shine == NULL || shine->data == NULL || alpha_orig == NULL || profile == NULL ||
        tip_w == 0U || tip_h == 0U || profile_w == 0U) {
        return;
    }
    if (shine->header.w < tip_w || shine->header.h < tip_h) {
        return;
    }
    if (shine->header.cf != LV_COLOR_FORMAT_ARGB8888 &&
        shine->header.cf != LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED) {
        return;
    }

    for (x = 0; x < (int32_t)tip_w; x++) {
        uint32_t edge = 0U;
        int32_t mx = shine_x + x;

        if (mx >= 0 && (uint32_t)mx < profile_w) {
            edge = profile[mx];
        }
        ui_tip_shine_apply_column(shine, alpha_orig, tip_w, tip_h, (uint32_t)x, edge);
    }
    lv_draw_buf_flush_cache(shine, NULL);
}

void ui_tip_shine_buf_destroy(lv_draw_buf_t *shine)
{
    if (shine == NULL) {
        return;
    }
    lv_draw_buf_destroy(shine);
}

/** 从 UTF-8 取下一个 Unicode 码点，并推进 *i。无效字节跳过 1 并返回 0。 */
static uint32_t ui_tip_shine_utf8_next(const char *txt, uint32_t *i)
{
    const uint8_t *s;
    uint32_t idx;
    uint32_t result;
    uint8_t c;

    if (txt == NULL) {
        return 0;
    }
    idx = (i == NULL) ? 0U : *i;
    s = (const uint8_t *)txt + idx;
    c = s[0];
    if (c == 0U) {
        return 0;
    }
    if (c < 0x80U) {
        result = c;
        idx += 1U;
    } else if ((c & 0xE0U) == 0xC0U && (s[1] & 0xC0U) == 0x80U) {
        result = ((uint32_t)(c & 0x1FU) << 6) | (uint32_t)(s[1] & 0x3FU);
        idx += 2U;
    } else if ((c & 0xF0U) == 0xE0U && (s[1] & 0xC0U) == 0x80U && (s[2] & 0xC0U) == 0x80U) {
        result = ((uint32_t)(c & 0x0FU) << 12) | ((uint32_t)(s[1] & 0x3FU) << 6) |
            (uint32_t)(s[2] & 0x3FU);
        idx += 3U;
    } else if ((c & 0xF8U) == 0xF0U && (s[1] & 0xC0U) == 0x80U &&
        (s[2] & 0xC0U) == 0x80U && (s[3] & 0xC0U) == 0x80U) {
        result = ((uint32_t)(c & 0x07U) << 18) | ((uint32_t)(s[1] & 0x3FU) << 12) |
            ((uint32_t)(s[2] & 0x3FU) << 6) | (uint32_t)(s[3] & 0x3FU);
        idx += 4U;
    } else {
        result = 0U;
        idx += 1U;
    }
    if (i != NULL) {
        *i = idx;
    }
    return result;
}

lv_draw_buf_t *ui_tip_shine_text_buf_create(const char *text, const lv_font_t *font,
    int32_t letter_space)
{
    lv_point_t size;
    lv_draw_buf_t *out;
    lv_draw_buf_t *glyph_a8;
    int32_t pos_x;
    uint32_t i;

    if (text == NULL || font == NULL) {
        return NULL;
    }

    lv_text_get_size(&size, text, font, letter_space, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    if (size.x <= 0 || size.y <= 0) {
        return NULL;
    }

    out = lv_draw_buf_create((uint32_t)size.x, (uint32_t)size.y, LV_COLOR_FORMAT_ARGB8888,
        LV_STRIDE_AUTO);
    if (out == NULL || out->data == NULL) {
        if (out != NULL) {
            lv_draw_buf_destroy(out);
        }
        return NULL;
    }
    lv_memzero(out->data, out->data_size);

    /* 单字 A8 临时缓冲；19px 字库绰绰有余 */
    glyph_a8 = lv_draw_buf_create(64, 64, LV_COLOR_FORMAT_A8, LV_STRIDE_AUTO);
    if (glyph_a8 == NULL || glyph_a8->data == NULL) {
        if (glyph_a8 != NULL) {
            lv_draw_buf_destroy(glyph_a8);
        }
        lv_draw_buf_destroy(out);
        return NULL;
    }

    pos_x = 0;
    i = 0;
    while (text[i] != '\0') {
        uint32_t letter;
        uint32_t letter_next = 0U;
        uint32_t peek_i;
        lv_font_glyph_dsc_t g;
        int32_t dx;
        int32_t dy;
        int32_t gy;
        int32_t gx;

        letter = ui_tip_shine_utf8_next(text, &i);
        if (letter == 0U) {
            continue;
        }
        peek_i = i;
        if (text[peek_i] != '\0') {
            letter_next = ui_tip_shine_utf8_next(text, &peek_i);
        }
        if (!lv_font_get_glyph_dsc(font, &g, letter, letter_next)) {
            continue;
        }

        if (g.box_w > 0U && g.box_h > 0U) {
            lv_draw_buf_t *shaped = lv_draw_buf_reshape(glyph_a8, LV_COLOR_FORMAT_A8,
                g.box_w, g.box_h, LV_STRIDE_AUTO);
            const lv_draw_buf_t *bm;

            if (shaped == NULL) {
                lv_font_glyph_release_draw_data(&g);
                pos_x += (int32_t)g.adv_w + letter_space;
                continue;
            }
            glyph_a8 = shaped;
            bm = lv_font_get_glyph_bitmap(&g, glyph_a8);
            if (bm != NULL && bm->data != NULL) {
                dx = pos_x + g.ofs_x;
                dy = (int32_t)(font->line_height - font->base_line) - (int32_t)g.box_h - g.ofs_y;
                for (gy = 0; gy < (int32_t)g.box_h; gy++) {
                    int32_t y = dy + gy;
                    const uint8_t *src;
                    lv_color32_t *dst;

                    if (y < 0 || (uint32_t)y >= out->header.h) {
                        continue;
                    }
                    src = bm->data + (uint32_t)gy * bm->header.stride;
                    dst = (lv_color32_t *)(out->data + (uint32_t)y * out->header.stride);
                    for (gx = 0; gx < (int32_t)g.box_w; gx++) {
                        int32_t x = dx + gx;
                        uint8_t a;

                        if (x < 0 || (uint32_t)x >= out->header.w) {
                            continue;
                        }
                        a = src[gx];
                        if (a == 0U) {
                            continue;
                        }
                        if (a >= dst[x].alpha) {
                            dst[x].red = 0xFF;
                            dst[x].green = 0xFF;
                            dst[x].blue = 0xFF;
                            dst[x].alpha = a;
                        }
                    }
                }
            }
            lv_font_glyph_release_draw_data(&g);
        }
        pos_x += (int32_t)g.adv_w + letter_space;
    }

    lv_draw_buf_destroy(glyph_a8);
    lv_draw_buf_flush_cache(out, NULL);
    return out;
}

#endif /* IS_SUPPORT_LVGL */
