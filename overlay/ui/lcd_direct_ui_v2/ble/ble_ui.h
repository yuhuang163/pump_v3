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

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_ui_update(ui_manager_context_t *thiz, lv_obj_t *root, uint8_t is_connected);
void ble_ui_deinit(ui_manager_context_t *thiz);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

