#ifndef __UI_LANG_APPLY_H__
#define __UI_LANG_APPLY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_lang_runtime.h"

#if IS_SUPPORT_LVGL
#include "lvgl.h"

/* 把 ui_widget_id_e 样式套到 LVGL 对象上；文案仍用 ui_str_id_e。 */
void ui_widget_apply_pos(lv_obj_t *obj, ui_widget_id_e id);
void ui_label_apply(lv_obj_t *label, ui_widget_id_e style_id, ui_str_id_e str_id);
void ui_icon_apply(lv_obj_t *icon, ui_widget_id_e style_id);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __UI_LANG_APPLY_H__ */
