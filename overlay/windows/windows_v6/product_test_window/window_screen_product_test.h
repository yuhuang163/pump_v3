#ifndef __WINDOW_SCREEN_PRODUCT_TEST_H__
#define __WINDOW_SCREEN_PRODUCT_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep

/// 产品测试动画频率
#if IS_SUPPORT_LVGL
#define PRODUCT_TEST_ANIMATION_FREQ                 (25)
#else
#define PRODUCT_TEST_ANIMATION_FREQ                 (5)
#endif

/// 产品测试UI动作枚举
typedef enum {
    PRODUCT_TEST_HOME = 100,
    PRODUCT_TEST_LCD_COLOR,
    PRODUCT_TEST_AGING_TEST,
} product_test_ui_action_e;

/// 产品测试UI类型枚举
typedef enum {
    PRODUCT_TEST_UI_TYPE_NONE = 0,
    PRODUCT_TEST_UI_TYPE_HOME,
    PRODUCT_TEST_UI_TYPE_LCD_COLOR,
    PRODUCT_TEST_UI_TYPE_AGING,
    PRODUCT_TEST_UI_TYPE_POWER_OFF_HOLD, /// 长按 power/start 关机确认
    PRODUCT_TEST_UI_TYPE_MAX
} product_test_ui_type_e;

/// 产品测试模式枚举
typedef enum {
    PRODUCT_TEST_MODE_NONE = 0,
    PRODUCT_TEST_HOME_MODE,
    PRODUCT_TEST_LCD_COLOR_MODE,
    PRODUCT_TEST_AGING_TEST_MODE,
    PRODUCT_TEST_MODE_MAX
} product_test_mode_e;

/// 产品测试UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;  
    key_input_context_t *key;
    battery_manager_context_t *battery;
    tests_manager_context_t *tests;
    ble_manager_context_t *ble;
    airplane_mode_context_t *airplane;
    resource_manager_context_t *res;
    led_manager_context_t *led;
    storage_manager_context_t *st;
    breast_pump_manager_context_t *pump;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. product_test_ui_type_e
    uint16_t power_off_hold_restore_ui_type; /// 关机确认页退出后恢复的测试 UI
    uint32_t lcd_color; /// 当前LCD颜色.
    product_test_mode_e tests_mode; /// 产品测试模式.
    uint16_t popup_ui_type; /// 当前弹窗类型. product_test_popup_ui_type_e
    uint8_t power_off_hold_holding; /// 关机确认页是否处于按住倒计时
} ui_proc_product_test_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void stop_product_test_ui(ui_proc_product_test_t *proc);
void show_product_home_test_ui(ui_proc_product_test_t *proc);
void product_home_test_ui_hdr_remove(void);
void product_home_test_keyevent_cb(ui_proc_product_test_t *proc, uint16_t keycode, uint16_t keytype);
void show_lcd_color_test_ui(ui_proc_product_test_t *proc);
void lcd_color_test_ui_hdr_remove(void);
void lcd_color_test_keyevent_cb(ui_proc_product_test_t *proc, uint16_t keycode, uint16_t keytype);
void show_aging_test_ui(ui_proc_product_test_t *proc);
void aging_test_keyevent_cb(ui_proc_product_test_t *proc, uint16_t keycode, uint16_t keytype);

/// 长按 power/start 关机确认页
void show_product_test_power_off_hold_ui(ui_proc_product_test_t *proc);
void product_test_power_off_hold_ui_exit(ui_proc_product_test_t *proc);
void product_test_power_off_hold_dismiss(ui_proc_product_test_t *proc);
void product_test_power_off_hold_key_long_2s_handler(ui_proc_product_test_t *proc);
void product_test_power_off_hold_key_up_handler(ui_proc_product_test_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


