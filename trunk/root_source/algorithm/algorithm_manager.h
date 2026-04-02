#ifndef __ALGORITHM_MANAGER_H__
#define __ALGORITHM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager_api.h" // IWYU pragma: keep


/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
typedef struct {
    void *user_data;
    on_algo_result_cb_ptr cb;
    list_head_t list;
} algo_evt_client_node_t;

void algo_manager_dispatch_to_clients(struct algo_manager_context *ctx);

#ifdef __cplusplus
}
#endif

#endif 







