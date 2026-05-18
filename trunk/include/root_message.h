#ifndef __ROOT_MESSAGE_H__
#define __ROOT_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "handler_post.h" // IWYU pragma: keep
#include "broadcast_manager_api.h" // IWYU pragma: keep
#include "project_config.h" // IWYU pragma: keep
#include "tests_manager_api.h" // IWYU pragma: keep

/// 描述异步消息类型
typedef enum {
    MSG_TYPE_DEFAULT = 0,
    MSG_TYPE_WIN_DISPATCH, /// 窗口调度.
    MSG_TYPE_WIN_PRELOAD, /// 窗口预加载.
    MSG_TYPE_WIN_EVENT, /// 窗口事件.
    MSG_TYPE_WIN_FLUSH, /// 窗口刷新.
    MSG_TYPE_INPUT_KEY_EVENT, /// 按键事件.
    MSG_TYPE_HANDLER_POST_EVENT, /// 处理事件.
    MSG_TYPE_BROADCAST_EVENT, /// 广播事件.
    MSG_TYPE_MAX
} aync_msg_type_e;

/// 弹出框ID枚举.
typedef enum {
    ID_POPUP_UI_NONE = 0,
    ID_POPUP_UI_OP_SUCCESS = 100,
    ID_POPUP_UI_OP_SUCCESS_PRESET,
    ID_POPUP_UI_OP_FAILED ,
    ID_POPUP_UI_LEVEL_ADJUST_WARNING , /// 档位范围调整警告
    ID_POPUP_UI_ONLY_AVAILABLE_IN_PUMPING,  /// 仅在泵奶中可以用
    ID_POPUP_UI_AIRPLANE_MODE_PROMPT,  /// 飞行模式提示弹窗
    ID_POPUP_UI_PUMP_FINISH_TIMEOUT, ///
    ID_POPUP_UI_PUMP_FINISH_MANUAL,
    ID_POPUP_UI_PUMP_PAUSE_MANUAL,
    ID_POPUP_UI_PUMP_PAUSE_TIMEOUT,
    ID_POPUP_UI_BT_CONNECT,
    ID_POPUP_UI_BT_DISCONNECT,
    ID_POPUP_UI_BT_CONNECTING,
    ID_POPUP_UI_DEVICE_BIND,
    ID_POPUP_UI_DEVICE_UNBIND,
    ID_POPUP_UI_COUNT_DOWN,
    ID_POPUP_UI_LOW_POWER,
    ID_POPUP_UI_WILL_POWER_OFF,
} popup_ui_id_e;

/// 窗口屏幕进入参数描述,记录进入新窗口屏幕需要的参数.
typedef struct {
    uint8_t window_id;   /// 窗口ID.
    bool is_bg_dispatch; /// 是否后台调度. 配合窗口优先级使用
    bool forbid_short_press_exit_popup; /// 禁止短按退出弹窗. 默认允许短按退出弹窗.
    int arg1;            /// 参数1.
    int arg2;            /// 参数2.
    void *user_data;     /// 上下文指针.
    uint32_t popup_exit_time; /// 弹出框退出时间.
    int popup_priority;   /// 弹出框优先级.
    int (*popup_exit_cb)(void *user_data, int args1, int args2); /// 弹出框退出回调.
} window_entry_extra_t;

/// 描述窗口事件结构.
typedef struct window_event_data {
    uint8_t window_id;
    uint32_t args1;
    uint32_t args2;
    void *user_data;
    void *wm;   /// window_manager
    void *window_ctx;
} window_event_data_t;

typedef enum {
    ID_ENTRY_TYPE_SWITCH = 0,
    ID_ENTRY_TYPE_PRELOAD = 1000,
    ID_ENTRY_TYPE_MAX
} window_entry_type_e;

/// sensor事件类型
typedef struct {
    uint8_t report_type;
} sensor_event_type_t;

/// 传感器掩码类型
typedef enum {
    SENSOR_MASK_IMU = 1 << 0,
    SENSOR_MASK_FSENSOR = 1 << 1,
    SENSOR_MASK_AIR_PRESSURE = 1 << 2,
} sensor_wrapper_data_mask_e;

/// sensor数据集
typedef struct {
    #if IS_SUPPORT_DEVICE_IMU
    uint16_t imu_serial_num;
    imu_wrapper_data_t imu_data;
    #endif
    #if IS_SUPPORT_DEVICE_FSENSOR
    uint16_t f_sensor_serial_num;
    f_sensor_data_t force_raw_data;
    #endif
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    uint16_t air_pressure_serial_num;
    air_pressue_status_t air_pressure_data[AIR_PRESSURE_NUM];
    #endif
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
    uint16_t light_sensor_serial_num;
    light_sensor_data_t light_sensor_data;
    #endif
    uint32_t sensor_mask; /// 传感器掩码，用于记录传感器数据是否有效
} sensor_wrapper_data_t;

/// ble事件类型
typedef struct {
    uint8_t msg_id;
    uint16_t data_len;
    void *ble_data;
} ble_event_type_t;

/// 测试指令类型
typedef enum {
    FAC_TEST_NONE,
    FAC_TEST_SHOW_QRCODE,
    FAC_TEST_SHOW_VERSION_INFO,
    FAC_TEST_SHOW_HW_STATUS,
    FAC_TEST_SHOW_BUILDTIME,
    FAC_TEST_DISPLAY_COLOR,
    FAC_TEST_AGING_WAIT,
    FAC_TEST_AGING_START,
    FAC_TEST_AGING_FINISH,
    FAC_TEST_AGING_EXIT,
    FAC_TEST_IF_QUALIFIED,
    FAC_TEST_WORKING_CURRENT,
    FAC_TEST_LED_ADJUST,
    FAC_TEST_MAX
} factory_test_type_e;

/// 颜色测试类型
typedef enum {
    FAC_TEST_DISPLAY_COLOR_RED = 0,
    FAC_TEST_DISPLAY_COLOR_GREEN,
    FAC_TEST_DISPLAY_COLOR_BLUE,
    FAC_TEST_DISPLAY_COLOR_YELLOW,
    FAC_TEST_DISPLAY_COLOR_WHITE,
    FAC_TEST_DISPLAY_COLOR_BLACK,
    FAC_TEST_DISPLAY_COLOR_MAX
} factory_display_color_test_e;

/// 老化测试类型
typedef enum {
    FAC_AGING_TEST_TYPE_NONE = 0,
    FAC_AGING_TEST_TYPE_1,     /// 启动吸奶马达开始振动，工作两分钟停止两分钟，循环此过程15次后停止.
    FAC_AGING_TEST_TYPE_2,     /// 工作两分钟 停两分钟  一直循环
    FAC_AGING_TEST_TYPE_3,     /// 两分钟一个周期 一直工作  clean模式
    FAC_AGING_TEST_TYPE_4,     /// 两分钟一个周期 一直工作  soft模式
    FAC_AGING_OVER,            /// 老化完毕
    FAC_AGING_TEST_TYPE_MAX
} factory_aging_test_e;

/// 工厂测试入口参数描述
typedef struct {
    uint8_t window_id;            /// 窗口ID.
    int factory_test_id;          /// 工厂测试ID
    uint32_t color_test_id;       /// 颜色测试ID
    int aging_test_id;            /// 老化测试ID
    int enter_from_source;        /// 进入源来自哪里
} factory_test_entry_extra_t;

typedef struct {
    ota_event_type_e event;
    int object;
    int args1;
    int args2;
} ota_event_info_t;

/// 描述按键事件结构.
typedef struct input_key_event_data {
    uint16_t keycode;
    uint16_t keytype;
} input_key_event_data_t;

/// 消息实体封装
#define MULTI_FUNC_MSG_DATA_SIZE (48)
typedef struct multi_func_msg_data {
    uint32_t msg_type;
    union {
        uint8_t  parms[MULTI_FUNC_MSG_DATA_SIZE];
        uint16_t parms_16[MULTI_FUNC_MSG_DATA_SIZE / 2];
        uint32_t parms_32[MULTI_FUNC_MSG_DATA_SIZE / 4];
        window_entry_extra_t extra;
        window_event_data_t window_evt_data;
        sensor_event_type_t sensor_evt_type;
        sensor_wrapper_data_t sensor_data;
        ble_event_type_t ble_evt_type;
        factory_test_entry_extra_t factory_extra;
        hdr_post_args_t hdr_post;
        ota_event_info_t ota_info;
        input_key_event_data_t key_event_data;
        bcr_pkg_struct_t bcr_pkg;
        product_test_entry_extra_t product_test_extra;
    } union_body;
} multi_func_msg_data_t;

/// 消息通道ID枚举
typedef enum {
    DEVICE_BLE_FAC_MSG_CHANNEL = 0, /// 蓝牙工厂通道
    DEVICE_BLE_MASTER_MSG_CHANNEL,  /// 蓝牙主通道
    DEVICE_BLE_REMOTE_MSG_CHANNEL,  /// 蓝牙远程通道
    DEVICE_UART_MSG_CHANNEL,        /// 串口通道
    DEVICE_MSG_CHANNEL_ID_MAX
} device_message_channel_id_e;

#ifdef __cplusplus
}
#endif

#endif 


