#ifndef __BLE_UI_H__
#define __BLE_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep

#define BASE_BLE_UI_X                                      (0)
#define BASE_BLE_UI_Y                                      (16)
#define BASE_PUMP_HOME_BLE_UI_X                            (0)
#define BASE_PUMP_HOME_BLE_UI_Y                            (16)

#define CUSTOM_BLE_UI_X                                    (0)
#define CUSTOM_BLE_UI_Y                                    (16)
#define CUSTOM_PUMP_HOME_BLE_UI_X                          (0)
#define CUSTOM_PUMP_HOME_BLE_UI_Y                          (16)

#define BLE_UI_BLINK_PERIOD                                  (500) /// 1Hz 闪烁半周期(ms).
#define BLE_UI_CONNECT_PROMPT_TIMEOUT_MS                     (30000) /// 未连接提示闪烁时长(ms).
#define BLE_UI_BLINK_VISIBLE(blink_time) \
    ((((get_timestamp() - (blink_time)) / BLE_UI_BLINK_PERIOD) % 2U) == 0U)

typedef struct {
    uint8_t sync_enable; /// 与外部状态闪烁同频同步.
    uint32_t blink_time; /// 外部状态闪烁起始时间.
} ble_ui_blink_sync_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_ui_init(ui_manager_context_t *thiz);
int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble,
    uint16_t x, uint16_t y, const ble_ui_blink_sync_t *sync);
void ble_ui_deinit(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

