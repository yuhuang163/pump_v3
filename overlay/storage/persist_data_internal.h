#ifndef __PERSIST_STORAGE_INTERNAL_H__
#define __PERSIST_STORAGE_INTERNAL_H__

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
void on_post_persist_storage_init(storage_manager_context_t *ptr);
void on_perpare_persist_storage_init(storage_manager_context_t *ptr);
void register_persist_storage_ctx(storage_manager_context_t *ptr);
void on_storage_broadcast_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);

#ifdef __cplusplus
}
#endif

#endif 







