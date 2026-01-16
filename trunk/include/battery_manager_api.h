#ifndef __BATTERY_MANAGER_API_H__
#define __BATTERY_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/// 定义空电量百分比
#ifndef EMPTY_BAT_PERCENTAGE
#define EMPTY_BAT_PERCENTAGE              5
#endif

/// 定义低电量百分比
#ifndef LOW_BAT_PERCENTAGE
#define LOW_BAT_PERCENTAGE                20
#endif

/// 描述充电相关
typedef void (*on_charge_state_change_fptr)(void *wm, charge_state_e new_state);
typedef void (*on_battery_state_change_fptr)(void *wm, int is_low_battery);

/// 描述电池管理器上下文对象
struct rt_device_node;
struct broadcast_obj;
typedef struct battery_manager_context {
/// public:
    uint8_t (*is_charge_plug_in)(void);
    uint8_t (*get_charge_status)(void);
    void (*set_charge_ui_show_flag)(int flag);
    uint8_t (*show_charge_ui_if_need)(void *wm, int is_fg);
    void (*set_state_change_listener)(on_charge_state_change_fptr cb);

    uint8_t (*is_low_battery)(void);
    uint8_t (*is_empty_battery)(void);
    uint8_t (*get_battery_percent)(void);
    void (*set_battery_ui_show_flag)(int flag);
    uint8_t (*show_battery_ui_if_need)(void *wm, int is_fg);
    void (*set_state_battery_listener)(on_battery_state_change_fptr cb);
/// private:
    struct __battery_private {
        uint8_t is_plug_state_changed;
        uint8_t charge_ui_has_show;
        uint8_t battery_percent;
        uint8_t low_battery;
        uint8_t battery_ui_has_show;   /// 0 未显示；1 正在显示；2 本次唤醒提醒过；
        on_battery_state_change_fptr m_battery_listener_cb;
        on_charge_state_change_fptr m_charge_listener_cb;
        struct rt_device_node *battery;
        struct rt_device_node  *led_dev;
        struct misc_manager_context *misc;
    } priv;
} battery_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


