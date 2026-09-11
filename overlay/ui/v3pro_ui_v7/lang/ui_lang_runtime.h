#ifndef __UI_LANG_RUNTIME_H__
#define __UI_LANG_RUNTIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_lang_types.h"
#include "ui_str_id.h"
#include "ui_widget_id.h"
#include "ui_layout_id.h"
#include "ui_page_id.h"

struct _lv_font_t;

extern const char *const g_ui_str_default[UI_STR_ID_COUNT];
extern const ui_widget_style_t g_ui_widget_default[UI_W_COUNT];
extern const int16_t g_ui_layout_default[UI_LAY_COUNT];
extern const struct _lv_font_t *const g_ui_font_default[UI_FONT_ROLE_COUNT];

/* 先查外部语言包，没有再用固件 default。 */
const char *ui_str(ui_str_id_e id);
const ui_widget_style_t *ui_widget(ui_widget_id_e id);
int16_t ui_layout_i16(ui_layout_id_e id);
const struct _lv_font_t *ui_font(ui_font_role_e role);

#ifdef __cplusplus
}
#endif
#endif /* __UI_LANG_RUNTIME_H__ */
