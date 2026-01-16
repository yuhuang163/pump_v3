#ifndef __STORAGE_MANAGER_INTERNAL_H__
#define __STORAGE_MANAGER_INTERNAL_H__

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
int user_database_put(storage_manager_context_t *ctx, int database_id, const uint8_t *buf, int buf_len);
int user_database_get(storage_manager_context_t *ctx, int database_id, uint8_t *buf, int buf_len);
int user_database_remove(storage_manager_context_t *ctx, int database_id);
int factory_flash_put(storage_manager_context_t *ctx, const uint8_t *buf, int buf_len);
int factory_flash_get(storage_manager_context_t *ctx, uint8_t *buf, int buf_len);

#ifdef __cplusplus
}
#endif

#endif 







