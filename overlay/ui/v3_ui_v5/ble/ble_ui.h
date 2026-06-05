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

#define COMMON_BLE_UI_X                                      (168)
#define COMMON_BLE_UI_Y                                      (22)
#define PUMP_HOME_BLE_UI_X                                   (146)
#define PUMP_HOME_BLE_UI_Y                                   (22)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_ui_init(ui_manager_context_t *thiz);
int ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, ble_manager_context_t *ble, 
    uint16_t x, uint16_t y);
void ble_ui_deinit(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

