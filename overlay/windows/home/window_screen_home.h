#ifndef __WINDOW_SCREEN_HOME_H__
#define __WINDOW_SCREEN_HOME_H__

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

#if IS_SUPPORT_LVGL
#define HOME_ANIMATION_FREQ                 (25)
#else
#define HOME_ANIMATION_FREQ                 (5)
#endif

/// 主界面动画展示时间
#define HOME_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 主界面绑定二维码展示时长
#define HOME_BIND_QRCODE_DISPLAY_TIMEOUT    (3000) /// 3s

/// 主界面UI动作枚举
typedef enum {
    HOME_HDR_ACTION_PUMP_MODE_SELECT,
    HOME_HDR_ACTION_MAX
} home_ui_hdr_action_e;

/// 主界面UI类型枚举
typedef enum {
    HOME_UI_TYPE_NONE = 0,
    HOME_UI_TYPE_HOME,
    HOME_UI_TYPE_HOME_IDLE,
    HOME_UI_TYPE_BIND_QRCODE,
    HOME_UI_TYPE_MAX
} home_ui_type_e;

typedef enum {
    HOME_POPUP_UI_NONE = 0,
    HOME_POPUP_UI_OPERATION_SUCCESS = 1, /// 操作成功提示
    HOME_POPUP_UI_OPERATION_FAILED = 2, /// 操作失败提示
} home_popup_ui_type_e;

/// 主界面UI结构体
typedef struct {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    ble_manager_context_t *ble;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    led_manager_context_t *led;
    breast_pump_manager_context_t *pump;
    broadcast_obj_t receiver;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
    uint16_t popup_ui_type; /// 当前弹窗类型. home_popup_ui_type_e
    uint32_t popup_ui_start_time; /// 当前弹窗开始时间.
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
void stop_home_common_ui(ui_proc_home_t *proc);
void home_start_key_event_handler(ui_proc_home_t *proc);

/// 绑定二维码展示
void start_home_bind_ui_animation(ui_proc_home_t *proc);
void home_bind_ui_hdr_remove(void);

/// 弹窗类型更新
void home_popup_ui_type_update(ui_proc_home_t *proc, home_popup_ui_type_e type);
#ifdef __cplusplus
}
#endif

#endif 


