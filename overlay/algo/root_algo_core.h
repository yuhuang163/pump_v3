#ifndef __ROOT_ALGO_CORE_H__
#define __ROOT_ALGO_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "root_algo_post_proc.h" // IWYU pragma: keep
#include "root_algo_acq_proc.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void root_algo_init(algo_manager_context_t *ctx);
void root_algo_deinit(algo_manager_context_t *ctx);
void root_algo_reset(algo_manager_context_t *ctx);
int root_algo_version_get(void);
void root_algo_sensor_cb_register(algo_manager_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif 







