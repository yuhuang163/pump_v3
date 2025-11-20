#ifndef __HW_SELF_TESTS_HANDLER_H__
#define __HW_SELF_TESTS_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

typedef enum {
    SH_TST_READ_SW_VER_EVENT    = TST_SELF_HARDWARE_EVENT_BEGIN,
    SH_TST_READ_STORAGE_EVENT,
    SH_TST_STORAGE_CRC_CHECK_EVENT,
} self_hardware_tst_event_type_e;

void hw_self_tests_thread_handler(struct tests_manager_context *ctx, void *msg);
int on_hw_self_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
/// 初始化工厂测试模块，获取工厂测试单独需要相关设备句柄.
void on_perpare_hw_self_tests_init(tests_manager_context_t *ctx);
/// 初始化工厂测试模块，获取设备存储的产测信息等操作.
void on_post_hw_self_tests_init(tests_manager_context_t *ctx);

/// 监听事件上报事
int dispatch_listen_hw_self_event_report(tests_manager_context_t *ctx, uint16_t event_id, void *data, uint16_t data_len);

#ifdef __cplusplus
}
#endif

#endif 