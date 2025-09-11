#ifndef __WINDOW_MANAGER_API_H__
#define __WINDOW_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "root_message.h"

#if CUST_LVGL_SUPPORT
#include "lv_conf.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#endif /// CUST_LVGL_SUPPORT

/// 语言类型，不能随意修改排序，会影响打包脚本
typedef enum {
    SYS_LANGUANGE_CHINESE = 0,
    SYS_LANGUANGE_ENGLISH,
    SYS_LANGUANGE_JAPANESE,
    SYS_LANGUANGE_KOREAN,
    SYS_LANGUANGE_MAX,
} sys_language_e;

/*
 * 窗口ID定义.
 */
typedef enum {
    ID_WINDOW_INVALID = 0,
    ID_WINDOW_BEGIN = ID_WINDOW_INVALID,
    ID_WINDOW_DUMMY,               /// suspend
    ID_WINDOW_LOGO,                /// 开机LOGO
    ID_WINDOW_GUIDE,               /// 引导界面
    ID_WINDOW_CHARGE,              /// 充电UI
    ID_WINDOW_BATTERY,             /// 低电量.
    ID_WINDOW_OTA_UPGRADE,         /// OTA
    ID_WINDOW_POPUP_WINDOW,        /// 通用弹出框
    ID_WINDOW_BREATH_PUMP,         /// 泵奶主流程
    ID_WINDOW_FACTORY_MODE,        /// 工厂模式
    ID_WINDOW_SETTINGS,            /// 系统设置
    ID_WINDOW_AIRPLANE_MODE,       /// 进入飞行模式后和退出飞行模式前处理飞行模式事件的中间窗口
    /// add new screen id above this.
    ID_WINDOW_MAX
} window_id_e;

/// 窗口优先级枚举.
typedef enum {
    WINDOW_PRIO_LOW = 0,
    WINDOW_PRIO_LEVEL1 = 1,
    WINDOW_PRIO_LEVEL2 = 2,
    WINDOW_PRIO_LEVEL3 = 3,
    WINDOW_PRIO_LEVEL4 = 4,
    WINDOW_PRIO_LEVEL5 = 5,
    WINDOW_PRIO_LEVEL6 = 6,
    WINDOW_PRIO_LEVEL7 = 7,
    WINDOW_PRIO_LEVEL8 = 8,
    WINDOW_PRIO_LEVEL9 = 9,
    WINDOW_PRIO_HIGH = 10,
    WINDOW_PRIO_TOP = 99,

    WINDOW_PRIO_BATTERY = WINDOW_PRIO_LEVEL1,
    WINDOW_PRIO_BREATH_PUMP_HEAD = WINDOW_PRIO_LEVEL2,
    WINDOW_PRIO_CHARGE = WINDOW_PRIO_LEVEL3,
    WINDOW_PRIO_POP_WINDOW = WINDOW_PRIO_LEVEL7,
    WINDOW_PRIO_FTM = WINDOW_PRIO_LEVEL8,
    WINDOW_PRIO_OTA = WINDOW_PRIO_LEVEL9,
    WINDOW_PRIO_BACKUP = WINDOW_PRIO_LEVEL9,
    WINDOW_PRIO_AP_MODE = WINDOW_PRIO_HIGH,
    WINDOW_PRIO_LOGO = WINDOW_PRIO_TOP,
} window_priority_e;

/// 显示管理.
struct window_context;
struct resource_manager_context;
struct rt_device_node;

typedef struct window_manager_context {
/// private:
    struct __window_manager_private {
        struct resource_manager_context *res; /// 资源管理器
    #if CUST_LVGL_SUPPORT
        thread_context_t lv_hdr_thd; /// lvgl 显示调度线程
        os_timer_id lv_tick_timer; /// lvgl tick定时器
        uint8_t lv_tick_status; /// lvgl tick状态
    #else
        thread_context_t window_flush_thd; /// 窗口刷新调度线程
    #endif
        thread_context_t window_dispatch_thd; /// 窗口调度线程
        struct rt_device_node *panel_dev; /// 显示设备
        multi_func_msg_data_t new_msg; /// 新消息
        uint16_t go_back_history_flag; /// 回退历史标志
        list_head_t history_list; /// 历史列表
        os_mutex_id history_lock; /// 历史列表锁
        os_mutex_id window_switch_lock; /// 窗口切换锁
        os_mutex_id ui_draw_lock; /// 显示刷新锁
        broadcast_obj_t *bcd; /// 广播对象
        sys_language_e language_index; /// 语言索引
        bool system_is_ready; /// 系统启动完成标志
        void *window_private[ID_WINDOW_MAX]; /// 记录scr上下文指针.
        int display_ready; /// 显示设备状态
        uint8_t wake_up_without_operate; /// 飞行模式累计次数
        uint8_t is_restting_device; /// 手柄是否正在重置
        int window_switch_forbid;  /// 禁止窗口切换 特殊场景例如系统重置操作时
        int m_backlight_val; /// 当前背光值
        bool is_playing_realtime_ani; /// 播放动画期间
        uint8_t *frame_buffer; /// 帧缓冲区
        uint8_t *draw_buffer; /// 绘制缓冲区
    } priv;

// public:
    void (*set_backlight)(uint16_t val);
    void (*set_is_restting_device)(uint8_t val);
    int (*set_window_switch_forbid)(int forbid);
    bool (*system_is_ready)(void);
    void (*play_boot_animation)(void);
    int (*post_msg_to_window)(const char *tag, multi_func_msg_data_t *msg);
    bool (*wait_window_ready)(int wait_dummy);
    bool (*is_current_window)(uint8_t window_id);
    bool (*is_on_window_dispatch)(void);
    const char *(*to_window_name)(uint8_t window_id);
    uint8_t (*get_current_window_id)(void);
    int (*dispatch_new_window_args)(struct window_manager_context *wm, multi_func_msg_data_t *msg);
} window_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

