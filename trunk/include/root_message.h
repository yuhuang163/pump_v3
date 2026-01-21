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
    MSG_TYPE_WIN_DISPATCH,
    MSG_TYPE_WIN_PRELOAD,
    MSG_TYPE_WIN_EVENT,
    MSG_TYPE_WIN_FLUSH,
    MSG_TYPE_INPUT_KEY_EVENT,
    MSG_TYPE_HANDLER_POST_EVENT,
    MSG_TYPE_BROADCAST_EVENT,
    MSG_TYPE_MAX
} aync_msg_type_e;

/// 弹出框ID枚举.
typedef enum {
    ID_POPUP_WINDOW_BT_CONNECT,
    ID_POPUP_WINDOW_BT_DISCONNECT,
    ID_POPUP_WINDOW_AIRPLANE_MODE,
    ID_POPUP_WINDOW_MAX
} puoup_window_id_e;

/// 窗口屏幕进入参数描述,记录进入新窗口屏幕需要的参数.
typedef struct {
    uint8_t window_id;
    uint8_t pm_wakeup_reason;
    uint8_t set_bst_mode_flag;
    uint8_t goto_breast_pump;
    uint8_t setting_sub_menu_pos;
    uint8_t gretting_which_time;
    uint8_t show_breast_pump_ui;
    void *user_data;
    /// 弹出框入参数参数描述.
    puoup_window_id_e popup_bg_res_index;
    uint8_t popup_bg_which;
    uint32_t popup_exit_time;
    int (*popup_exit_cb)(void *user_data, int args1, int args2);
    uint8_t is_background;
    /// 进入ota的异常类型.
    uint32_t ota_enter_exception_type;
} window_entry_extra_t;

/// 牙菌斑窗口带入参数描述
typedef struct {
    uint8_t window_id;
    uint8_t dental_plaque_level;
    uint16_t popup_exit_timeout; /// 弹出框窗口自动退出时间.
} window_entry_plaque_extra_t;

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

/// 窗口进入类型
typedef enum {
    UI_MODE_ENTRY_WIZARD = 101,            /// 向导进入吸奶界面展示风险提醒
    UI_MODE_ENTRY_APP_SWITCH_MODE,   /// APP切换模式唤醒进入模式界面
} ui_mode_entry_e;

/// 窗口进入参数描述
typedef struct {
    uint8_t window_id;
    uint8_t pm_wakeup_reason;
    ui_mode_entry_e mode_entry_type;
} ui_mode_entry_extra_t;

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
    uint8_t window_id;
    int factory_test_id;    /// 工厂测试ID
    uint32_t color_test_id;      /// 颜色测试ID
    int aging_test_id;      /// 老化测试ID
    int enter_from_source;  /// 进入源来自哪里:开机自动0,按键进入1
} factory_test_entry_extra_t;

typedef struct {
    ota_event_type_e event;
    bool result;         ///事件结果
    int state;
    uint32_t sub_code;
} ota_event_type_t;

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
        window_entry_plaque_extra_t plaque_extra;
        window_event_data_t window_evt_data;
        ui_mode_entry_extra_t ui_mode_entry_extra;
        sensor_event_type_t sensor_evt_type;
        sensor_wrapper_data_t sensor_data;
        ble_event_type_t ble_evt_type;
        factory_test_entry_extra_t factory_extra;
        hdr_post_args_t hdr_post;
        ota_event_type_t ota_evt_type;
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


