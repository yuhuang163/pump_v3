#ifndef __WINDOWS_MANAGER_H__
#define __WINDOWS_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "windows_manager_api.h" // IWYU pragma: keep
#include "ui_manager_api.h" // IWYU pragma: keep

/// 调试LV FPS.
#define DEBUG_LV_FPS (1)

/// 锁定窗口切换锁.
#define MUTEX_LOCK_WM(tag, __lock) \
do { \
    MUTEX_LOCK(__lock); \
    if (tag) LOG_WM("display_stack_lock=>Take:%s", (char *)tag); \
} while (0)

/// 解锁窗口切换锁.
#define MUTEX_UNLOCK_WM(tag, __lock) \
do { \
    if (tag) LOG_WM("display_stack_lock=>Give:%s", (char *)tag); \
    MUTEX_UNLOCK(__lock); \
} while (0)

/// 打印窗口历史记录.
#define DUMP_HISTORY(tag, head) \
do { \
    int j = 0; \
    LIST_FOR_EACH((pos), (head)) { \
        window_history_node_t *__node = CONTAINER_OF(pos, window_history_node_t, list); \
        LOG_WM("%d: %s: (%s)", j, tag, to_window_name(__node->window_id)); \
        j++; \
    } \
    if (j == 0) { \
        LOG_WM("%s:list is null", tag); \
    } \
} while (0)

// 窗口引用记录宏定义.
#define INIT_WINDOW_CONTEXT() \
    static void *__s_window_refs = NULL

/// 获取窗口上下文.
#define GET_WINDOW_CONTEXT() (__s_window_refs)

/// 窗口上下文保护宏定义.
#define WINDOW_CONTEXT_GUARD_VOID(proc, expected_window_id) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_VOID(__wm); \
        if (GET_WINDOW_CONTEXT() == NULL) { \
            LOG_WM("window context not present, ignore."); \
            return; \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        uint8_t overlay_window_id = __wm->get_overlay_window_id(__wm); \
        if (current_window_id != expected_window_id && \
            overlay_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d/%d.", \
                 current_window_id, overlay_window_id); \
            return; \
        } \
    } while (0)

#define WINDOW_CONTEXT_GUARD_INT(proc, expected_window_id, ret) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_INT(__wm); \
        if (GET_WINDOW_CONTEXT() == NULL) { \
            LOG_WM("window context not present, ignore."); \
            return (ret); \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        uint8_t overlay_window_id = __wm->get_overlay_window_id(__wm); \
        if (current_window_id != expected_window_id && \
            overlay_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d/%d.", \
                 current_window_id, overlay_window_id); \
            return (ret); \
        } \
    } while (0)

/// 窗口上下文保护宏定义.
#define WINDOW_CURRENT_ID_GUARD_VOID(proc, expected_window_id) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_VOID(__wm); \
        if (GET_WINDOW_CONTEXT() == NULL) { \
            LOG_WM("window context not present, ignore."); \
            return; \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        if (current_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d.", \
                 current_window_id); \
            return; \
        } \
    } while (0)

#define WINDOW_CURRENT_ID_GUARD_INT(proc, expected_window_id, ret) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_INT(__wm); \
        if (GET_WINDOW_CONTEXT() == NULL) { \
            LOG_WM("window context not present, ignore."); \
            return (ret); \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        if (current_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d.", \
                 current_window_id); \
            return (ret); \
        } \
    } while (0)

/// 窗口ID保护宏定义.
#define WINDOW_ID_GUARD_VOID(proc, expected_window_id) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_VOID(__wm); \
        if (proc->margic == UI_WINDOW_MARGIC_CODE_INVALID) { \
            LOG_WM("margic invalid, ignore."); \
            return; \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        uint8_t overlay_window_id = __wm->get_overlay_window_id(__wm); \
        if (current_window_id != expected_window_id && \
            overlay_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d/%d.", \
                 current_window_id, overlay_window_id); \
            return; \
        } \
    } while (0)

#define WINDOW_ID_GUARD_INT(proc, expected_window_id, ret) \
    do { \
        window_manager_context_t *__wm = (proc) ? (proc)->wm : NULL; \
        EXCEPTION_CHECK_INT(__wm); \
        if (proc->margic == UI_WINDOW_MARGIC_CODE_INVALID) { \
            LOG_WM("margic invalid, ignore."); \
            return (ret); \
        } \
        uint8_t current_window_id = __wm->get_current_window_id(); \
        uint8_t overlay_window_id = __wm->get_overlay_window_id(__wm); \
        if (current_window_id != expected_window_id && \
            overlay_window_id != expected_window_id) { \
            LOG_WM("window id not match, ignore. %d/%d.", \
                 current_window_id, overlay_window_id); \
            return (ret); \
        } \
    } while (0) \

/// 设置窗口上下文.
#define SET_WINDOW_CONTEXT(ctx) \
    do { \
        __s_window_refs = ctx; \
        LOG_WM("set_window_context:%p", __s_window_refs); \
    } while (0)

/// 判断窗口上下文是否存在.
#define IS_WINDOW_CONTEXT_PRESENT(closure) \
    do { \
        if (__s_window_refs != NULL) { \
            { closure } \
        } else { \
            LOG_UI("Oops! NOT WINDOW_PRESENT"); \
        } \
    } while (0)

/// 窗口动作.
typedef enum {
    ACTION_SET_BL = 1,
    ACTION_STOP_LV_TICK_TIMER,
    ACTION_BOOT_COMPLETED,
} window_action_e;

/// 窗口历史记录.
typedef struct {
    uint8_t window_id;
    list_head_t list;
} window_history_node_t;

/// 窗口生命周期记录code.
typedef enum {
    ID_SCR_STATE_ON_CREATE = 100,
    ID_SCR_STATE_ON_RESUME = 101,
    ID_SCR_STATE_ON_PAUSE = 102,
    ID_SCR_STATE_ON_DESTORY = 103,
    ID_SCR_STATE_MAX
} window_lift_cycle_e;

/// 窗口显示结果.
typedef enum {
    ID_WINDOW_DISPLAY_OK = 0,
    ID_WINDOW_DISPLAY_TRY_HISTORY = -1,
    ID_WINDOW_DISPLAY_IGNORE = -2,
} window_enter_result_e;

/// 飞行模式进入/退出.
typedef enum {
    SETTINGS_AIRPLANE_MODE_ENTER = 0,
    SETTINGS_AIRPLANE_MODE_EXIT,
} settings_airplane_mode_e;

/// 窗口生命周期记录code.
#define UI_WINDOW_MARGIC_CODE (0x44556677)
#define UI_WINDOW_MARGIC_CODE_INVALID (0x00000000)

/// UI生命周期监听
struct window_context;
typedef int (*window_life_cycle_enter_cb)(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *old_scr);

/// 窗口生命周期退出回调.
typedef int (*window_life_cycle_exit_cb)(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *new_scr);

/// 窗口事件异步处理回调.
typedef int (*window_async_evt_handler_cb)(struct window_event_data *evt_data);

/// 窗口上下文.
typedef struct window_context {
    /// 当前窗口ID
    uint8_t new_window_id;
    /// 是否压入历史栈
    uint8_t not_push_history;
    /// 用于记录具体的window类型和参数.
    uint8_t is_popup_window;
    /// 窗口上下文指针
    void *user_data;
    /// 窗口事件异步处理回调.
    window_async_evt_handler_cb async_evt_handler_cb;
    /// 窗口进入类型
    window_entry_type_e entry_type;
    /// 窗口优先级.
    window_priority_e z_order;
    /// 窗口生命周期监听回调.
    window_life_cycle_enter_cb evt_cb_entry;
    window_life_cycle_exit_cb evt_cb_exit;  
} window_context_t;

/// 描述窗口切换的生命周期.
typedef struct {
    void (*on_create)(void *ptr);   /// 创建
    void (*on_resume)(void *ptr);   /// 恢复
    void (*on_pause)(void *ptr);    /// 暂停
    void (*on_destory)(void);       /// 销毁
} window_life_cycle_t;

#define DECLARE_WM_HANDLE_OBJ(x) extern const window_life_cycle_t x
#define REFS_T(x) ((const window_life_cycle_t *)(&x))

typedef window_enter_result_e (*enter_new_window_fptr)(window_manager_context_t *context, uint8_t window_id);

/// 描述窗口处理结构.
typedef struct {
    window_id_e window_id;                         /// 窗口id
    window_priority_e z_order;                     /// 窗口Z轴层级优先级.
    const enter_new_window_fptr new_window_cb;     /// 窗口处理业务逻辑.
    const window_life_cycle_t *scr_obj;            /// 窗口UI处理.
} window_context_handle_t;

/// 窗口记录.
typedef struct {
    window_id_e next_window_id;                /// 即将进入的新窗口id
    window_context_handle_t *window_hdr;       /// 当前窗口
    window_context_t *window_context;          /// 当前窗口上下文
    window_priority_e z_order;                 /// 当前窗口层级优先级
    window_context_handle_t *base_window_hdr;  /// 被覆盖的 base window
    window_context_t *base_window_context;     /// 被覆盖的 base window 上下文
    window_priority_e base_z_order;            /// 被覆盖的 base window 层级优先级
} window_context_record_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void register_window_state_module(window_manager_context_t *wm);
#ifdef __cplusplus
}
#endif

#endif 


