#ifndef __ROOT_COMMON_API_H__
#define __ROOT_COMMON_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_common_config.h" // IWYU pragma: keep
#include "root_common.h" // IWYU pragma: keep
#include "root_message.h" // IWYU pragma: keep
#include "root_storage_base.h" // IWYU pragma: keep
#include "thread_api.h" // IWYU pragma: keep
#include "handler_post.h" // IWYU pragma: keep
#include "hang_detect.h" // IWYU pragma: keep
#include "root_algo.h" // IWYU pragma: keep

#include "broadcast_manager_api.h" // IWYU pragma: keep
#include "peripheral_bus_api.h" // IWYU pragma: keep
#include "storage_manager_api.h" // IWYU pragma: keep
#include "power_manager_api.h" // IWYU pragma: keep
#include "time_system_api.h" // IWYU pragma: keep
#include "misc_manager_api.h" // IWYU pragma: keep
#include "windows_manager_api.h" // IWYU pragma: keep
#include "ota_manager_api.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep
#include "key_input_manager_api.h" // IWYU pragma: keep
#include "airplane_mode_api.h" // IWYU pragma: keep
#include "sensor_manager_api.h" // IWYU pragma: keep
#include "tests_manager_api.h" // IWYU pragma: keep
#include "motor_manager_api.h" // IWYU pragma: keep
#include "breast_pump_manager_api.h" // IWYU pragma: keep
#include "algorithm_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep
#include "protocol_manager_api.h" // IWYU pragma: keep

#define ADD_MODULE(name, id, perpare_init_cb, post_init_cb, ptr, list_head, mem_usage) \
    add_new_module(name, id, perpare_init_cb, post_init_cb, ptr, list_head, mem_usage)

/// 定义各个组件id.
typedef enum {
    ID_MODULE_BEGIN = 0,

    ID_MODULE_TIME,
    ID_MODULE_POWER,
    ID_MODULE_DEVICE_BUS,
    ID_MODULE_RES,
    ID_MODULE_WM,
    ID_MODULE_UI,
    ID_MODULE_ALGO_HUB,
    ID_MODULE_STORAGE,
    ID_MODULE_KEY_INPUT,
    ID_MODULE_SENSOR,
    ID_MODULE_BLE,
    ID_MODULE_OTA,

    ID_MODULE_IOT,
    ID_MODULE_MISC,
    ID_MODULE_BREAST_PUMP,
    ID_MODULE_MOTOR,
    ID_MODULE_AUDIO,

    ID_MODULE_AIRPLANE_MODE,
    ID_MODULE_WIFI,
    ID_MODULE_DATA_BURIAL,
    ID_MODULE_VIDEO_CAP,

    ID_MODULE_LED_FESTOON,

    ID_MODULE_WEEKLY_STAT,

    ID_MODULE_TESTS,
    ID_MODULE_PROTOCOL,

    ID_MODULE_MAX
} module_desc_id_e;

/// 描述每个组件对象节点.
typedef struct module_node {
    const char *name;
    module_desc_id_e id;
    int (*perpare_init)(void *ptr);
    int (*post_init)(void *ptr);
    void *module_ptr;
    uint32_t mem_usage;
    list_head_t list;
} module_node_t;

#define CREATE_MSG_QUEUE(msg_id_ptr, queue_num) \
    CREATE_MSG_TYPE_QUEUE(msg_id_ptr, queue_num, sizeof(multi_func_msg_data_t))
/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
/// hook点入口.
void framework_extra_hook(void);
/// 系统重启
void __system_reboot(const char *reason);
/// 注册接口.
int register_interface_for_file_download(void *ptr);
/// 注册模块.
int init_module(void);
/// 注册模块.
int register_module(void);
/// 查找模块.
const void *find_module(module_desc_id_e id);
/// 添加模块.
void *add_new_module(const char *name, module_desc_id_e id, int (*perpare_init_cb)(void *ptr), int (*post_init_cb)(void *ptr), 
                     void *ptr, list_head_t *list_head, uint32_t mem_usage);

/// 运行用户线程调度.
int run_user_thread_dispatch(void);
#ifdef __cplusplus
}
#endif

#endif 

