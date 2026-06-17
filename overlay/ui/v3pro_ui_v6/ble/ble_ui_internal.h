#ifndef __BLE_UI_INTERNAL_H__
#define __BLE_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

void ble_base_ui_init(ui_manager_context_t *thiz);
void ble_custom_ui_init(ui_manager_context_t *thiz);
void ble_base_ui_deinit(ui_manager_context_t *thiz);
void ble_custom_ui_deinit(ui_manager_context_t *thiz);
int ble_base_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y);
int ble_custom_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif // __BLE_UI_INTERNAL_H__
