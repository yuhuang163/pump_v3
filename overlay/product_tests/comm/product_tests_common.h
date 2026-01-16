#ifndef __PRODUCT_TESTS_COMMOM_H__
#define __PRODUCT_TESTS_COMMOM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

void on_perpare_common_test_init(tests_manager_context_t *ctx);
void on_post_common_test_init(tests_manager_context_t *ctx);
void common_tests_thread_handler(struct tests_manager_context *ctx, void *msg);
int on_common_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);

/// 监听事件上报事
int dispatch_listen_common_event_report(tests_manager_context_t *ctx, uint16_t event_id, void *data, uint16_t data_len);

#ifdef __cplusplus
}
#endif

#endif 