#ifndef __TESTS_MANAGER_API_H__
#define __TESTS_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "product_tests.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"

/// 测试类型
typedef enum {
    TST_NONE_TYPE = 0x0000,           /// 无模式
    TST_FACTORY_TYPE = 0x0001,        /// 产测模式
    TST_AGING_TEST_TYPE = 0x0002,     /// 老化测试模式1
    TST_SUCTION_TEST_TYPE = 0x0004,   /// 吸力测试模式  
    TST_SELF_HARDWARE_TYPE = 0x0005, /// 自检模式
    TST_STRESS_TYPE = 0x0006,        /// 压力测试模式
    TST_COMMON_TYPE = 0x0007,        /// 通用测试模式
    TST_LCD_COLOR_TYPE = 0x0008,  /// LCD颜色测试模式
    TST_LCD_COLOR_SET_TYPE = 0x0009, /// 设置LCD颜色
} product_tst_report_type_e;

/// 测试类型
typedef enum {
    TST_POLL_EVENT                = 0,
    TST_FACTORY_EVENT_BEGIN       = 1,
    TST_SELF_HARDWARE_EVENT_BEGIN = 101,
    TST_STRESS_EVENT_BEGIN        = 201,
    TST_COMM_EVENT_BEGIN          = 301,
} product_tst_event_type_e;

typedef enum {
    FAC_TST_READ_SW_VER_EVENT    = TST_FACTORY_EVENT_BEGIN,
    FAC_TST_READ_DEV_MAC_EVENT,
    FAC_TST_MOTOR_CTR_EVENT,
    FAC_TST_AGING_TEST_ENTER_EVENT,
    FAC_TST_AGING_TEST_EXIT_EVENT,
    FAC_TST_AGING_TEST_GET_CURRENT_AGE_TIME_EVENT,
    FAC_TST_SUCTION_TEST_EVENT,
    FAC_TST_ENTER_EVENT,
    FAC_TST_EXIT_EVENT,
} factory_tst_event_type_e;

typedef enum {
    COMM_TST_READ_SW_VER_EVENT    = TST_COMM_EVENT_BEGIN,
    COMM_TST_READ_DEV_MAC_EVENT,
    COMM_TST_MOTOR_CTR_EVENT,
} comm_tst_event_type_e;

typedef enum {
    SH_TST_READ_SW_VER_EVENT    = TST_SELF_HARDWARE_EVENT_BEGIN,
    SH_TST_READ_STORAGE_EVENT,
    SH_TST_STORAGE_CRC_CHECK_EVENT,
} self_hardware_tst_event_type_e;

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
        uint8_t lcd_set_color; /// 产测设置LCD颜色: 红0x01/绿0x02/蓝0x03/黑0x04/白0x05/灰度测试0x06/灰色0x07，0表示未设置
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


