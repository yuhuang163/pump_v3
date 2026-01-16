#ifndef __PRODUCT_TESTS_H__
#define __PRODUCT_TESTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

typedef enum {
    AGING_MODE_TYPE_1 = 0x01,
    AGING_MODE_TYPE_2 = 0x02,
} aging_mode_type_e;

typedef enum {
    AGING_TEST_STATUS_NOT_AGE = 0x00,
    AGING_TEST_STATUS_SUCCESS = 0x01,
    AGING_TEST_STATUS_FAILED = 0x02,
} aging_test_status_e;

typedef struct aging_result_info {
    uint32_t total_runtime_time;
    uint16_t aging_status;
    uint16_t aging_cycle;
} aging_result_info_t;

typedef struct aging_target_info {
    uint32_t total_target_time;
    uint16_t aging_type;
} aging_target_info_t;


struct rt_device_node;
typedef struct product_tests_comm_context {
    /// @brief bus设备句柄
    struct rt_device_node *airdev;
    struct rt_device_node * battdev;
    struct rt_device_node *imudev;
    struct rt_device_node *motordev;
    struct rt_device_node *leddev;
#if IS_SUPPORT_DEVICE_UART
    struct rt_device_node *fcuartdev;
#endif
    /// @brief 相关模块上下文句柄
    struct resource_manager_context *res; /// 资源管理器
    struct storage_manager_context *st; /// 存储管理器
    struct misc_manager_context *misc; /// 杂项管理器
    struct power_manager_context *pm; /// 电源管理器
    struct key_input_context *key; /// 按键管理器
    struct breast_pump_manager_context *pump; /// 泵奶管理器
    struct ble_manager_context *ble_ctx; /// 蓝牙处理管理器

} product_tests_comm_context_t;

typedef struct hw_self_tests_context {
    struct rt_device_node *dev;

} hw_self_tests_context_t;

typedef struct factory_tests_context {
    struct rt_device_node *dev;
    struct aging_target_info aging_target_info;
    struct aging_result_info aging_result_info;
} factory_tests_context_t;

#ifdef __cplusplus
}
#endif

#endif 



