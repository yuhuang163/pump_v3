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
int user_database_put(storage_manager_context_t *ctx, database_key_t key, const uint8_t *buf, int buf_len);
int user_database_get(storage_manager_context_t *ctx, database_key_t key, uint8_t *buf, int buf_len);
int user_database_remove(storage_manager_context_t *ctx, database_key_t key);

#ifdef __cplusplus
}
#endif

#endif 







