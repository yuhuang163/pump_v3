#ifndef __TESTS_MANAGER_API_H__
#define __TESTS_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

struct rt_device_node;
typedef struct product_tests_comm_context {
    /// @brief bus设备句柄
    struct rt_device_node *airdev;
    struct rt_device_node * battdev;
    struct rt_device_node *imudev;
    struct rt_device_node *motordev;
    struct rt_device_node *leddev;
    struct rt_device_node *fcuartdev;
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

} factory_tests_context_t;

/// 测试类型
typedef enum {
    TST_FACTORY_TYPE = 0,
    TST_SELF_HARDWARE_TYPE,
    TST_STRESS_TYPE,
    TST_COMMON_TYPE,
} product_tst_report_type_e;

/// 测试类型
typedef enum {
    TST_POLL_EVENT                = 0,
    TST_FACTORY_EVENT_BEGIN       = 1,
    TST_SELF_HARDWARE_EVENT_BEGIN = 101,
    TST_STRESS_EVENT_BEGIN        = 201,
    TST_COMM_EVENT_BEGIN          = 301,
} product_tst_event_type_e;

typedef struct {
    uint16_t event_type;     /// 事件类型
    void *extra_data;       /// 额外数据指针
    uint16_t data_length;    /// 额外数据长度
} product_test_entry_extra_t;

typedef struct {
    struct tests_manager_context *ctx;
    int arg1;
    int arg2;
    void *val;
} tests_func_args_t;

typedef int (*get_hw_self_tests_process_func)(tests_func_args_t args);
typedef int (*set_hw_self_tests_process_func)(tests_func_args_t args);

typedef int (*get_factory_tests_process_func)(tests_func_args_t args);
typedef int (*set_factory_tests_process_func)(tests_func_args_t args);

typedef int (*get_common_process_func)(tests_func_args_t args);
typedef int (*set_common_process_func)(tests_func_args_t args);

typedef struct {
    uint16_t event_id;                            /// event id
    get_hw_self_tests_process_func event_process; /// 处理接口.
} get_hw_self_tests_process_handle_t;

typedef struct {
    uint16_t event_id;                            /// event id
    set_hw_self_tests_process_func event_process; /// 处理接口.
} set_hw_self_tests_process_handle_t;

typedef struct {
    uint16_t event_id;                    /// event id
    get_factory_tests_process_func event_process; /// 处理接口.
} get_factory_tests_process_handle_t;

typedef struct {
    uint16_t event_id;                    /// event id
    set_factory_tests_process_func event_process; /// 处理接口.
} set_factory_tests_process_handle_t;

typedef struct {
    uint16_t event_id;                    /// event id
    get_common_process_func event_process; /// 处理接口.
} get_common_tests_process_handle_t;

typedef struct {
    uint16_t event_id;                    /// event id
    set_common_process_func event_process; /// 处理接口.
} set_common_tests_process_handle_t;

/// 窗口注册事件回调函数
typedef void (*on_product_test_evt_cb_fptr)(void *user_data, uint16_t event_id, void *data, int data_len);

typedef struct tests_manager_context {
    /// @brief private members
    struct __tests_private {
        thread_context_t tests_thd;
        os_timer_id tests_timer;
        os_mutex_id m_lock;
        os_mutex_id handle_lock;
        struct broadcast_obj *bcd;
        list_head_t product_test_list;
        uint8_t enabled;
        uint8_t current_mode;
        struct product_tests_comm_context comm_ctx;
        struct factory_tests_context factory_ctx;
        struct hw_self_tests_context hw_self_ctx;

        /// 产品测试外部接口列表.
        const get_hw_self_tests_process_handle_t *get_hw_self_tests_process_list;
        const set_hw_self_tests_process_handle_t *set_hw_self_tests_process_list;
        const get_factory_tests_process_handle_t *get_factory_tests_process_list;
        const set_factory_tests_process_handle_t *set_factory_tests_process_list;
        const get_common_tests_process_handle_t *get_commom_tests_process_list;
        const set_common_tests_process_handle_t *set_commom_tests_process_list;

        /// 产品测试外部接口列表长度.
        uint16_t get_hw_self_tests_process_list_len;
        uint16_t set_hw_self_tests_process_list_len;
        uint16_t get_factory_tests_process_list_len;
        uint16_t set_factory_tests_process_list_len;
        uint16_t get_common_tests_process_list_len;
        uint16_t set_common_tests_process_list_len;
    } priv;

    /// @brief external interfaces
    uint8_t (*tests_manager_is_enable)(void);
    uint8_t (*tests_manager_set_enable)(uint8_t enable);
    uint8_t (*tests_manager_get_current_mode)(void);
    uint8_t (*tests_manager_set_current_mode)(uint8_t mode);
    void (*add_product_tests_evt_lisenser)(void *user_data, product_tst_report_type_e report_type,
           on_product_test_evt_cb_fptr lisenser);
    void (*remove_product_tests_evt_lisenser)(on_product_test_evt_cb_fptr lisenser);

    int (*get_hw_self_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);
    int (*set_hw_self_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);
    int (*get_factory_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);
    int (*set_factory_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);
    int (*get_common_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);
    int (*set_common_tests_event_process)(uint16_t event_id, int arg1, int arg2,  void *val_in);

    /// @brief external hook points
    struct product_tst_manager_hook {
        int (*on_perpare_init_route)(struct tests_manager_context *ctx);
        int (*on_post_init_route)(struct tests_manager_context *ctx);
        /// 广播路由
        int (*on_product_tests_br_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        /// 线程路由
        int (*product_tests_thread_route)(struct tests_manager_context *ctx, void *msg);
    } *hook;
} tests_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


