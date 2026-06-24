#ifndef __UI_POPUP_H__
#define __UI_POPUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

void ui_popup_base_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, popup_ui_id_e type);
void ui_popup_base_remove(ui_manager_context_t *thiz);
void ui_popup_base_process(ui_manager_context_t *thiz, popup_ui_id_e type);
void ui_popup_base_update(ui_manager_context_t *thiz,
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index);

void ui_popup_custom_prepare(ui_manager_context_t *thiz, lv_obj_t *parent, popup_ui_id_e type);
void ui_popup_custom_remove(ui_manager_context_t *thiz);
void ui_popup_custom_process(ui_manager_context_t *thiz, popup_ui_id_e type);
void ui_popup_custom_update(ui_manager_context_t *thiz,
    popup_ui_id_e type, lv_obj_t *parent, uint8_t *ui_index);

#ifdef __cplusplus
}
#endif

#endif // __UI_POPUP_H__
