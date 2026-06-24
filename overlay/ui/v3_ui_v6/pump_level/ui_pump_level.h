#ifndef __UI_PUMP_LEVEL_H__
#define __UI_PUMP_LEVEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

void ui_pump_both_level_base_prepare(ui_manager_context_t *thiz);
void ui_pump_both_level_base_remove(ui_manager_context_t *thiz);
int ui_pump_both_level_base_process(ui_manager_context_t *thiz, uint8_t *level,
    int8_t level_change, uint16_t popup_ui_type);
void ui_pump_both_level_custom_prepare(ui_manager_context_t *thiz);
void ui_pump_both_level_custom_remove(ui_manager_context_t *thiz);
int ui_pump_both_level_custom_process(ui_manager_context_t *thiz, uint8_t *level,
    int8_t level_change, uint16_t popup_ui_type);

void ui_pump_level_base_prepare(ui_manager_context_t *thiz);
void ui_pump_level_base_remove(ui_manager_context_t *thiz);
int ui_pump_level_base_process(ui_manager_context_t *thiz, uint8_t level,
    int8_t level_change, bool is_left, uint8_t popup_ui_type);
void ui_pump_level_custom_prepare(ui_manager_context_t *thiz);
void ui_pump_level_custom_remove(ui_manager_context_t *thiz);
int ui_pump_level_custom_process(ui_manager_context_t *thiz, uint8_t level,
    int8_t level_change, bool is_left, uint8_t popup_ui_type);

#ifdef __cplusplus
}
#endif

#endif // __UI_PUMP_LEVEL_H__
