#ifndef __BLE_CUSTOM_UI_INTERNAL_H__
#define __BLE_CUSTOM_UI_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep
#include "ble_ui.h" // IWYU pragma: keep

typedef struct {
    lv_obj_t *ble_img;
    uint32_t fps_start_time;
    uint8_t is_show_ble;
    uint8_t is_connected;
    uint8_t force_update;
    uint8_t rsv[14];
} ble_custom_ui_info_t;

void ble_custom_ui_init(ui_manager_context_t *thiz);
int ble_custom_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble,
    uint16_t x, uint16_t y, const ble_ui_blink_sync_t *sync);
void ble_custom_ui_deinit(ui_manager_context_t *thiz);

#ifdef __cplusplus
}
#endif

#endif // __BLE_CUSTOM_UI_INTERNAL_H__
