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

/// 产品测试动画展示时间
#define PRODUCT_TESTS_ANIMATION_DISPLAY_TIME        (2000) /// 2s

/// 产品测试UI动作枚举
typedef enum {
    PRODUCT_TEST_START = 100,
    PRODUCT_TEST_E10   = 101,
    PRODUCT_TEST_E11   = 102,
} product_test_ui_action_e;

/// 产品测试UI类型枚举
typedef enum {
    PRODUCT_TEST_UI_TYPE_NONE = 0,
    PRODUCT_TEST_UI_TYPE_HOME,
    PRODUCT_TEST_UI_TYPE_MAX
} product_test_ui_type_e;

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
    storage_manager_context_t *st;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. product_test_ui_type_e
} ui_proc_product_test_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_product_tests_ui(ui_proc_product_test_t *proc);
void stop_product_tests_ui_animation(ui_proc_product_test_t *proc);
void product_tests_start_key_event_handler(ui_proc_product_test_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


