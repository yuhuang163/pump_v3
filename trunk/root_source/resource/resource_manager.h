#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "resource_img_define.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep
#include "resource_id_define.h" // IWYU pragma: keep
#include "resource_info.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int img_res_init(resource_manager_context_t *thiz);
int install_img_res(resource_manager_context_t *thiz, res_theme_type_e theme_type);
int uninstall_img_res(resource_manager_context_t *thiz, res_theme_type_e theme_type);
int res_data_load_with_theme(resource_manager_context_t *thiz, res_theme_type_e which, uint16_t img_id, res_img_info_t *out);
#ifdef __cplusplus
}
#endif

#endif 


