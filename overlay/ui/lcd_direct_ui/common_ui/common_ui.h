#ifndef __COMMON_UI_H__
#define __COMMON_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void common_ui_img_resource_init(ui_manager_context_t *thiz, uint16_t id, res_img_info_t *info);
void common_ui_img_resource_deinit(ui_manager_context_t *thiz, res_img_info_t *info);

#ifdef __cplusplus
}
#endif
#endif// __UI_DISPLAY_H__

