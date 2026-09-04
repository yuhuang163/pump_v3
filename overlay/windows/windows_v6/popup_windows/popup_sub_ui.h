#ifndef __POPUP_SUB_UI_H__
#define __POPUP_SUB_UI_H__

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
#include "window_screen_popup.h" // IWYU pragma: keep

/// top overlay 弹窗优先级（数值越大越高）
typedef enum {
    POPUP_OVERLAY_PRIO_NONE = 0,
    POPUP_OVERLAY_PRIO_BLE = 10,            /// 蓝牙连接/断开/绑定
    POPUP_OVERLAY_PRIO_LOW_POWER = 30,      /// 低电 / 即将关机
    POPUP_OVERLAY_PRIO_TRAVEL_LOCK = 40,    /// 旅行锁倒计时（长按中，不被其它 overlay 打断）
} popup_overlay_prio_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_low_power_ui_animation(ui_proc_popup_t *proc);

void start_travel_lock_countdown_ui_animation(ui_proc_popup_t *proc);

void popup_window_exit_handler_cb(ui_proc_popup_t *proc);

void start_ble_status_ui_animation(ui_proc_popup_t *proc, uint16_t popup_ui_type);

int popup_overlay_priority_get(uint16_t popup_type);
bool popup_overlay_can_preempt(uint16_t current_type, uint16_t new_type);
#ifdef __cplusplus
}
#endif

#endif 


