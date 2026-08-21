/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_tip_shine_soft.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "lvgl.h" // IWYU pragma: keep

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_shine_soft_release_bufs(ui_shine_info_t *info)
{
    if (info == NULL) {
        return;
    }
    if (info->shine != NULL && lv_obj_is_valid(info->shine)) {
        lv_image_set_src(info->shine, NULL);
    }
    if (info->shine_buf != NULL) {
        ui_tip_shine_buf_destroy(info->shine_buf);
        info->shine_buf = NULL;
    }
    if (info->alpha != NULL) {
        u_free(info->alpha);
        info->alpha = NULL;
    }
    if (info->profile != NULL) {
        u_free(info->profile);
        info->profile = NULL;
    }
    info->alpha_w = 0;
    info->alpha_h = 0;
}

static void ui_shine_profile_ensure(ui_shine_info_t *info, const ui_shine_style_t *style)
{
    if (info->profile != NULL || style == NULL || style->mask_w == 0U) {
        return;
    }
    info->profile = (uint8_t *)u_malloz(style->mask_w);
    if (info->profile != NULL) {
        ui_tip_shine_profile_build(get_predecoded_text_shine_mask(),
            info->profile, style->mask_w);
    }
}

static void ui_shine_sync(ui_shine_info_t *info, const ui_shine_style_t *style,
    lv_obj_t *content)
{
    lv_coord_t content_x;
    lv_coord_t content_y;
    int32_t shine_x;

    if (info == NULL || style == NULL || content == NULL || !lv_obj_is_valid(content) ||
        info->shine == NULL || !lv_obj_is_valid(info->shine) ||
        info->mask == NULL || !lv_obj_is_valid(info->mask)) {
        return;
    }

    content_x = lv_obj_get_x(content);
    content_y = lv_obj_get_y(content);
    shine_x = content_x - info->mask_x;
    /* 整幅 tip 亮层：在 mask 内按相对 x 定位，软边烘焙与位移一致 */
    lv_obj_set_pos(info->shine, shine_x, content_y - style->mask_y);

    if (info->shine_buf != NULL && info->alpha != NULL && info->profile != NULL) {
        ui_tip_shine_apply_profile(info->shine_buf, info->alpha,
            info->alpha_w, info->alpha_h, shine_x, info->profile, style->mask_w);
        lv_obj_invalidate(info->shine);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_shine_style_init(ui_shine_style_t *style)
{
    EXCEPTION_CHECK_VOID(style);
    style->mask_w = UI_SHINE_MASK_W_DEFAULT;
    style->mask_y = 0;
    style->screen_w = UI_SHINE_SCREEN_W_DEFAULT;
    style->period_ms = UI_SHINE_PERIOD_MS_DEFAULT;
    style->shine_opa = LV_OPA_80; /* == UI_SHINE_IMG_OPA_DEFAULT (204)，非字面量 80 */
}

void ui_shine_reset(ui_shine_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    memset(info, 0, sizeof(*info));
}

void ui_shine_prepare_from_tip(ui_shine_info_t *info, const ui_shine_style_t *style,
    const lv_draw_buf_t *tip)
{
    EXCEPTION_CHECK_VOID(info && style);
    if (tip == NULL) {
        return;
    }
    if (info->alpha == NULL) {
        info->alpha = ui_tip_shine_alpha_dup(tip, &info->alpha_w, &info->alpha_h);
    }
    if (info->shine_buf == NULL) {
        info->shine_buf = ui_tip_shine_buf_create(tip);
    }
    ui_shine_profile_ensure(info, style);
}

void ui_shine_prepare_from_text(ui_shine_info_t *info, const ui_shine_style_t *style,
    const char *text, const lv_font_t *font, int32_t letter_space, int32_t line_space)
{
    lv_draw_buf_t *text_buf;

    EXCEPTION_CHECK_VOID(info && style && text && font);
    if (info->alpha != NULL && info->shine_buf != NULL) {
        ui_shine_profile_ensure(info, style);
        return;
    }

    text_buf = ui_tip_shine_text_buf_create(text, font, letter_space, line_space);
    if (text_buf == NULL) {
        return;
    }
    if (info->alpha == NULL) {
        info->alpha = ui_tip_shine_alpha_dup(text_buf, &info->alpha_w, &info->alpha_h);
    }
    if (info->shine_buf == NULL) {
        info->shine_buf = ui_tip_shine_buf_create(text_buf);
    }
    ui_tip_shine_buf_destroy(text_buf);
    ui_shine_profile_ensure(info, style);
}

void ui_shine_update(ui_shine_info_t *info, lv_obj_t *parent, const ui_shine_style_t *style,
    lv_obj_t *content, uint8_t force_update)
{
    lv_coord_t mask_h;

    EXCEPTION_CHECK_VOID(info && parent && style);

    if (info->mask != NULL && lv_obj_is_valid(info->mask) &&
        info->shine != NULL && lv_obj_is_valid(info->shine) &&
        force_update == 0) {
        /* 位移只在 anim 里 sync，避免每帧重复烘焙 */
        return;
    }

    mask_h = (info->alpha_h > 0U) ? (lv_coord_t)info->alpha_h : 40;

    if (info->mask == NULL || !lv_obj_is_valid(info->mask)) {
        info->mask = lv_obj_create(parent);
        EXCEPTION_CHECK_VOID(info->mask);
        lv_obj_remove_style_all(info->mask);
        lv_obj_set_size(info->mask, style->mask_w, mask_h);
        lv_obj_set_style_bg_opa(info->mask, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info->mask, 0, 0);
        lv_obj_set_style_pad_all(info->mask, 0, 0);
        lv_obj_set_style_blend_mode(info->mask, LV_BLEND_MODE_ADDITIVE, 0);
        lv_obj_clear_flag(info->mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(info->mask, LV_OBJ_FLAG_CLICKABLE);
        info->mask_x = (int16_t)(-(int16_t)style->mask_w);
        info->mask_tick_ms = get_timestamp();
    } else {
        lv_obj_set_size(info->mask, style->mask_w, mask_h);
    }

    if (info->shine == NULL || !lv_obj_is_valid(info->shine)) {
        info->shine = lv_image_create(info->mask);
        EXCEPTION_CHECK_VOID(info->shine);
        lv_obj_clear_flag(info->shine, LV_OBJ_FLAG_CLICKABLE);
        if (info->shine_buf != NULL) {
            lv_image_set_src(info->shine, info->shine_buf);
        }
        lv_obj_set_style_image_opa(info->shine, style->shine_opa, 0);
        lv_obj_set_style_blend_mode(info->shine, LV_BLEND_MODE_NORMAL, 0);
    }

    lv_obj_set_style_align(info->mask, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_set_pos(info->mask, info->mask_x, style->mask_y);
    lv_obj_move_foreground(info->mask);
    if (content != NULL) {
        ui_shine_sync(info, style, content);
    }
}

void ui_shine_anim_update(ui_shine_info_t *info, const ui_shine_style_t *style,
    lv_obj_t *content, uint8_t paused)
{
    uint32_t elapsed;
    int32_t x;
    int32_t x_min;
    int32_t x_max;

    EXCEPTION_CHECK_VOID(info && style);
    if (paused != 0) {
        return;
    }
    if (info->mask == NULL || !lv_obj_is_valid(info->mask)) {
        return;
    }
    if (lv_obj_has_flag(info->mask, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    x_min = -(int32_t)style->mask_w;
    x_max = style->screen_w;
    elapsed = get_timestamp() - info->mask_tick_ms;
    if (elapsed >= style->period_ms) {
        info->mask_tick_ms = get_timestamp();
        elapsed = 0;
    }
    if (style->period_ms == 0U) {
        return;
    }
    x = x_min + (int32_t)((x_max - x_min) * elapsed) / style->period_ms;
    if (x != info->mask_x) {
        info->mask_x = (int16_t)x;
        lv_obj_set_pos(info->mask, info->mask_x, style->mask_y);
        lv_obj_move_foreground(info->mask);
        if (content != NULL) {
            ui_shine_sync(info, style, content);
        }
    }
}

void ui_shine_hide(ui_shine_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->mask != NULL && lv_obj_is_valid(info->mask)) {
        lv_obj_add_flag(info->mask, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_shine_show(ui_shine_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->mask != NULL && lv_obj_is_valid(info->mask)) {
        lv_obj_clear_flag(info->mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(info->mask);
    }
}

void ui_shine_remove(ui_shine_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    ui_shine_soft_release_bufs(info);
    /* 先删子对象再删 mask，避免 parent delete 后野指针 */
    if (info->shine != NULL && lv_obj_is_valid(info->shine)) {
        lv_obj_del(info->shine);
    }
    info->shine = NULL;
    if (info->mask != NULL && lv_obj_is_valid(info->mask)) {
        lv_obj_del(info->mask);
    }
    info->mask = NULL;
    info->mask_x = 0;
    info->mask_tick_ms = 0;
}
