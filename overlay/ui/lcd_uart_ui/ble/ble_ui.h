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
void ui_fill_ble_status(lcd_tlvs_t *tlv);
#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

