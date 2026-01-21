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
    ID_WINDOW_HOME,                /// 主界面
    ID_WINDOW_CHARGE,              /// 充电UI
    ID_WINDOW_BATTERY,             /// 低电量.
    ID_WINDOW_OTA_UPGRADE,         /// OTA
    ID_WINDOW_POPUP_WINDOW,        /// 通用弹出框
    ID_WINDOW_PUMP,                /// 泵奶主流程
    ID_WINDOW_FACTORY_MODE,        /// 工厂模式
    ID_WINDOW_SETTINGS,            /// 系统设置
    ID_WINDOW_AIRPLANE_MODE,       /// 飞行模式
    ID_WINDOW_PRODUCT_TEST_MODE,   /// 测试模式
    ID_WINDOW_FACTORY_RESET,       /// 工厂重置
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
    WINDOW_PRIO_LEVEL10 = 10,
    WINDOW_PRIO_LEVEL11 = 11,
    WINDOW_PRIO_LEVEL12 = 12,
    WINDOW_PRIO_HIGH = 11,

    WINDOW_PRIO_TOP = 99,

    WINDOW_PRIO_BATTERY = WINDOW_PRIO_LEVEL1,
    WINDOW_PRIO_BREAST_PUMP_HEAD = WINDOW_PRIO_LEVEL2,
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
        struct storage_manager_context *st; /// 存储管理器
        thread_context_t window_dispatch_thd; /// 窗口调度线程
        multi_func_msg_data_t new_msg; /// 新消息
        uint16_t go_back_history_flag; /// 回退历史标志
        list_head_t history_list; /// 历史列表
        os_mutex_id history_lock; /// 历史列表锁
        os_mutex_id window_switch_lock; /// 窗口切换锁
        broadcast_obj_t *bcd; /// 广播对象
        sys_language_e language_index; /// 语言索引
        bool system_is_ready; /// 系统启动完成标志
        void *window_private[ID_WINDOW_MAX]; /// 记录scr上下文指针.
        int display_ready; /// 显示设备状态
        uint8_t is_restting_device; /// 手柄是否正在重置
        int window_switch_forbid;  /// 禁止窗口切换 特殊场景例如系统重置操作时
    } priv;

    /// public:
    int (*dispatch_new_window)(struct window_context *ctx);
    /// 系统是否启动完成
    bool (*system_is_ready)(struct window_manager_context *wm);
    /// 系统启动完成
    void (*window_boot_completed)(struct window_manager_context *wm);
    /// 窗口调度开始
    void (*window_scheduling_start)(void);
    /// 发送消息到窗口
    int (*post_msg_to_window)(const char *tag, multi_func_msg_data_t *msg);
    /// 等待窗口准备就绪
    bool (*wait_window_ready)(int wait_dummy);
    /// 是否当前窗口
    bool (*is_current_window)(struct window_manager_context *wm, uint8_t window_id);
    bool (*is_the_same_window)(struct window_manager_context *wm, uint8_t new_window_id);
    bool (*is_valid_window_id)(uint8_t window_id);
    /// 窗口引导是否已部署
    bool (*is_window_guidance_deployed)(void); 
    /// 设置窗口引导已部署标志
    void (*set_window_guidance_deployed_flag)(uint8_t flag);
    /// 窗口调度相关.
    bool (*is_on_window_dispatch)(struct window_manager_context *wm);
    /// 窗口上下文相关.
    void (*set_window_private_context)(struct window_manager_context *wm, uint8_t window_id, void *context);
    void *(*get_window_private_context)(struct window_manager_context *wm, uint8_t window_id);
    /// 窗口入口参数相关.
    const void *(*get_window_entry_extra)(struct window_manager_context *wm);
    /// 系统是否正在重置设备标志
    void (*set_system_is_restting_device)(struct window_manager_context *wm, bool val); 
    bool (*get_system_is_restting_device)(struct window_manager_context *wm);
    /// 禁止窗口切换
    int (*set_window_switch_forbid)(struct window_manager_context *wm, int forbid);
    /// 窗口名称相关.
    const char *(*to_window_name)(uint8_t window_id);
    /// 获取当前窗口ID
    uint8_t (*get_current_window_id)(void);
    /// 调度新窗口参数
    int (*dispatch_new_window_args)(struct window_manager_context *wm, multi_func_msg_data_t *msg);
    /// 回退历史相关.
    int (*go_back_to_history)(void *wm, window_entry_extra_t *__extra);
    /// 清除回退历史标志
    void (*clear_window_go_back_history_flag)(struct window_manager_context *wm);
} window_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

