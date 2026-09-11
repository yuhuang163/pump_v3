#include "root_common_base.h"
#include "ui_lang_apply.h"

#if IS_SUPPORT_LVGL

#include "lvgl.h"

static lv_color_t ui_style_color(const ui_widget_style_t *style)
{
    uint16_t rgb = style->color_rgb;
    uint8_t r = (uint8_t)((rgb >> 11) << 3);
    uint8_t g = (uint8_t)(((rgb >> 5) & 0x3F) << 2);
    uint8_t b = (uint8_t)((rgb & 0x1F) << 3);

    return lv_color_make(r, g, b);
}

void ui_widget_apply_pos(lv_obj_t *obj, ui_widget_id_e id)
{
    const ui_widget_style_t *style = ui_widget(id);

    if (obj == NULL) {
        return;
    }
    lv_obj_align(obj, (lv_align_t)style->obj_align, style->x, style->y);
}

void ui_label_apply(lv_obj_t *label, ui_widget_id_e style_id, ui_str_id_e str_id)
{
    const ui_widget_style_t *style = ui_widget(style_id);

    if (label == NULL) {
        return;
    }
    lv_label_set_text(label, ui_str(str_id));
    if (style->font_role < UI_FONT_ROLE_COUNT) {
        lv_obj_set_style_text_font(label, ui_font((ui_font_role_e)style->font_role), 0);
    }
    if ((style->flags & UI_STYLE_F_COLOR_INHERIT) == 0) {
        lv_obj_set_style_text_color(label, ui_style_color(style), 0);
    }
    if ((style->flags & UI_STYLE_F_OPA_INHERIT) == 0) {
        lv_obj_set_style_opa(label, style->opa, 0);
    }
    lv_obj_set_style_text_align(label, (lv_text_align_t)style->text_align, 0);
    lv_obj_set_style_text_line_space(label, style->line_space, 0);
    if (style->max_w != 0) {
        lv_obj_set_width(label, style->max_w);
    }
    ui_widget_apply_pos(label, style_id);
}

void ui_icon_apply(lv_obj_t *icon, ui_widget_id_e style_id)
{
    const ui_widget_style_t *style = ui_widget(style_id);

    if (icon == NULL) {
        return;
    }
    if ((style->flags & UI_STYLE_F_OPA_INHERIT) == 0) {
        lv_obj_set_style_opa(icon, style->opa, 0);
    }
    ui_widget_apply_pos(icon, style_id);
}

#endif /* IS_SUPPORT_LVGL */
