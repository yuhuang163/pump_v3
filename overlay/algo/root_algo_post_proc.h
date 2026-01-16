#ifndef __ROOT_ALGO_POST_PROC_H__
#define __ROOT_ALGO_POST_PROC_H__

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
void algo_fusion_data_cb(algo_manager_context_t *ctx, uint8_t *data, int data_len);
#ifdef __cplusplus
}
#endif

#endif 







