#include "ui_lang_runtime.h"
#include "ui_lang_pack.h"

#include <stddef.h>
#include <stdint.h>

static const ui_widget_style_t s_widget_fallback = {
    .kind = UI_WIDGET_KIND_TEXT,
    .obj_align = 2,
    .opa = 255,
    .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    .font_role = 0xFF,
};

const char *ui_str(ui_str_id_e id)
{
    const char *pack_str = ui_lang_pack_str(id);

    if (pack_str != NULL) {
        return pack_str;
    }
    if ((uint32_t)id >= (uint32_t)UI_STR_ID_COUNT) {
        return "";
    }
    if (g_ui_str_default[id] == NULL) {
        return "";
    }
    return g_ui_str_default[id];
}

const ui_widget_style_t *ui_widget(ui_widget_id_e id)
{
    const ui_widget_style_t *pack_widget = ui_lang_pack_widget(id);

    if (pack_widget != NULL) {
        return pack_widget;
    }
    if ((uint32_t)id >= (uint32_t)UI_W_COUNT) {
        return &s_widget_fallback;
    }
    return &g_ui_widget_default[id];
}

int16_t ui_layout_i16(ui_layout_id_e id)
{
    int16_t value = 0;

    if (ui_lang_pack_layout(id, &value) == 0) {
        return value;
    }
    if ((uint32_t)id >= (uint32_t)UI_LAY_COUNT) {
        return 0;
    }
    return g_ui_layout_default[id];
}

const struct _lv_font_t *ui_font(ui_font_role_e role)
{
    const struct _lv_font_t *pack_font;

    if ((uint32_t)role >= (uint32_t)UI_FONT_ROLE_COUNT) {
        role = UI_FONT_ROLE_HINT;
    }
    pack_font = ui_lang_pack_font(role);
    if (pack_font != NULL) {
        return pack_font;
    }
    return g_ui_font_default[role];
}
