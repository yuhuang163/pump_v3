#ifndef __ROOT_ALGO_ACQ_PROC_H__
#define __ROOT_ALGO_ACQ_PROC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void algo_sampling_start(algo_manager_context_t *ctx);
void algo_sampling_stop(algo_manager_context_t *ctx);
bool algo_sampling_is_sampling(algo_manager_context_t *ctx);
void algo_sampling_data_cb(algo_manager_context_t *ctx, uint8_t *data, int data_len);
bool algo_change_sampling_interval(algo_manager_context_t *ctx, uint16_t interval);
#ifdef __cplusplus
}
#endif

#endif 







