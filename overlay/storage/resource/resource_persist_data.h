#ifndef __RESOURCE_PERSIST_DATA_H__
#define __RESOURCE_PERSIST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int write_img_res_ota_info_to_flash(store_func_args_t args);
int read_img_res_ota_info_from_flash(store_func_args_t args);
int write_img_res_ota_info_to_ram(store_func_args_t args);
int read_img_res_ota_info_from_ram(store_func_args_t args);
void reset_img_res_ota_info(storage_manager_context_t *ctx, img_res_ota_info_t *img_res_ota_info);

#ifdef __cplusplus
}
#endif

#endif 







