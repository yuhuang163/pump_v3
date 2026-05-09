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
    AGING_RUN_STATUS_IDLE = 0x00,
    AGING_RUN_STATUS_WORK = 0x01,
    AGING_RUN_STATUS_END = 0x02,
} aging_run_status_e;

typedef enum {
    AGING_TEST_STATUS_NOT_AGE = 0x00,
    AGING_TEST_STATUS_SUCCESS = 0x01,
    AGING_TEST_STATUS_FAILED = 0x02,
} aging_test_status_e;

typedef struct aging_result_info {
    uint32_t start_time;//老化测试开始时间，单位毫秒
    uint32_t time_offset;//老化计时对时时间，单位毫秒
    uint32_t current_cycle_work_time;//老化测试当前循环工作时间，单位毫秒
    uint16_t aging_status;//老化测试状态
    uint16_t aging_run_status;//老化测试运行状态，0：休息，1：工作 2:结束
    uint16_t aging_cycle;//老化测试循环次数
} aging_result_info_t;

typedef struct aging_target_info {
    uint32_t total_target_time;//老化测试目标时间，单位秒
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



