#ifndef __WINDOWS_MANAGER_H__
#define __WINDOWS_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep
#include "windows_manager_api.h" // IWYU pragma: keep
#include "windows_define.h" // IWYU pragma: keep


#define MUTEX_LOCK_WM(tag, __lock) \
do { \
    MUTEX_LOCK(__lock); \
    if (tag) LOG_WM("display_stack_lock=>Take:%s", (char *)tag); \
} while (0)

#define MUTEX_UNLOCK_WM(tag, __lock) \
do { \
    if (tag) LOG_WM("display_stack_lock=>Give:%s", (char *)tag); \
    MUTEX_UNLOCK(__lock); \
} while (0)

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

typedef enum {
    ACTION_SET_BL = 1,
    ACTION_STOP_LV_TICK_TIMER,
    ACTION_BOOT_COMPLETED,
} window_action_e;

typedef struct {
    uint8_t window_id;
    list_head_t list;
} window_history_node_t;

typedef enum {
    ID_SCR_STATE_ON_CREATE = 100,
    ID_SCR_STATE_ON_RESUME = 101,
    ID_SCR_STATE_ON_PAUSE = 102,
    ID_SCR_STATE_ON_DESTORY = 103,
    ID_SCR_STATE_MAX
} window_lift_cycle_e;

typedef enum {
    ID_WINDOW_DISPLAY_OK = 0,
    ID_WINDOW_DISPLAY_TRY_HISTORY = -1,
    ID_WINDOW_DISPLAY_IGNORE = -2,
} window_enter_result_e;

/**
 * 叠图送给LVGL显示的步骤
 */
typedef enum {
    OVERLAY_IMG_DIS_STEP_FULL_BG,
    OVERLAY_IMG_DIS_STEP_SYNC_FB,
    OVERLAY_IMG_DIS_STEP_LVGL_SHOW,
    OVERLAY_IMG_DIS_STEP_MAX
} overlay_img_dis_step_t;

/**
 * UI生命周期监听
 */
struct window_context;

typedef int (*window_life_cycle_enter_cb)(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *old_scr);

typedef int (*window_life_cycle_exit_cb)(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *new_scr);

typedef int (*window_async_evt_handler_cb)(struct window_event_data *evt_data);

typedef struct {
    /// 传入 get_res_buf使用，一般为 window_context_t
    void *user_data;
    /// 需要获取的图片资源ID号
    uint16_t res_id; 
    /// 存放图片数据 draw_buffer
    uint8_t *dest;
    /// 移动的指针，用来提高刷新率，加载不同的图片时移动 dest_addr 位置即可，不需要memcpy
    uint8_t **dest_addr;
    /// 移动的指针 dest_addr 原始地址，当在内存列表里面找不到图片时，让 dest_addr 回到原位再从Flash读取数据
    uint8_t **dest_addr_base;
} get_res_buf_args_t;

typedef struct window_context {
    /// 当前窗口ID
    uint8_t new_window_id;
    /// 是否压入历史栈
    uint8_t not_push_history;
    /// 用于记录具体的window类型和参数.
    uint8_t is_popup_window;
    /// 窗口上下文指针.
    void *user_data;
    window_manager_context_t *wm;
    /// 窗口自定义优先级,会覆盖掉系统默认优先级.

    /// 窗口事件异步处理回调.
    window_async_evt_handler_cb async_evt_handler_cb;
    /// entry_type,switch or only preload
    window_entry_type_e entry_type;

    window_priority_e z_order;
    // 窗口生命周期监听回调.
    window_life_cycle_enter_cb evt_cb_entry;
    window_life_cycle_exit_cb evt_cb_exit;
#if CUST_LVGL_SUPPORT
    /// 动画绘制上下文参数.
    lv_animation_play_st *ani;
    /// 动画绘制合并数组
    ui_draw_property_t draw[MAX_UI_DRAW_PROP];
    /// 合并数组长度.
    uint16_t draw_count;
    /// 获取资源缓冲区
    res_obj_desc_t *(*get_res_buf)(get_res_buf_args_t * p_args);
    /// 叠图图片数据初始化接口
    void (*overlay_img_wrap_init_cb)(void *user_data);
    /// 叠图接口，如果某个页面需要叠图，在页面实现 overlay_img_display_cb 即可
    void (*overlay_img_display_cb)(void *user_data, overlay_img_dis_step_t step);
#endif

#if IS_SUPPORT_OVERLAY_IMG
    /// 动画绘制合并数组
    ui_draw_property_t overlay_img_draw[OVERLAY_IMG_NUM_MAX][MAX_UI_DRAW_PROP];
    /// 合并数组长度
    uint16_t overlay_img_draw_count[OVERLAY_IMG_NUM_MAX];
    /// 是否叠图显示
    uint8_t is_display_overlay_img_mode;
#endif

#if USING_DENTAL_GRADUAL_CHANGE
    void (*dental_mold_to_frame_buffer)(uint8_t *fb, uint8_t *src, ui_draw_property_t *d);
#endif    
} window_context_t;

/**
 * 描述窗口切换的生命周期.
 */
typedef struct {
    void (*on_create)(void *ptr);   /// 创建
    void (*on_resume)(void *ptr);   /// 恢复
    void (*on_pause)(void *ptr);    /// 暂停
    void (*on_destory)(void);       /// 销毁
} window_life_cycle_t;

#define DECLARE_WM_HANDLE_OBJ(x) extern const window_life_cycle_t x
#define REFS_T(x) ((const window_life_cycle_t *)(&x))

typedef window_enter_result_e (*enter_new_window_fptr)(window_manager_context_t *context, uint8_t window_id);

/**
 * 描述窗口处理结构.
 */
typedef struct {
    window_id_e window_id;                         /// 窗口id
    window_priority_e z_order;                     /// 窗口Z轴层级优先级.
    const enter_new_window_fptr new_window_cb;     /// 窗口处理业务逻辑.
    const window_life_cycle_t *scr_obj;            /// 窗口UI处理.
} window_context_handle_t;

/**
 * 窗口记录.
 */
typedef struct {
    window_context_handle_t *window_hdr;
    window_context_t *window_context;
    window_priority_e z_order;
} window_context_record_t;

/**
 * 窗口页对象.
 */
typedef struct ui_page_obj
{
    const char *name;
    void (*create)  (void *argv);
    void (*destory) (void *argv);
    void (*draw)    (struct ui_page_obj *return_menu, void *argv);
    // ui_widget_typedef_t *p_widget_list;
    int16_t detal_x;
    int16_t detal_y;
    uint8_t current_max_widget;
    uint8_t is_notif:1;
    uint8_t if_stay_flag:1;
    uint8_t dynamicFlag:1;
#if FEATURE_PSRAM
    uint16_t psram_has_up_cache : 1;
    uint16_t psram_has_down_cache : 1;
    uint16_t psram_has_left_cache : 1;
    uint16_t psram_has_right_cache : 1;
    uint16_t psram_has_center_cache : 1;
	uint16_t psram_last_cache : 1;
#endif
} ui_page_obj_t;

typedef struct ui_page_handle
{
    void (*draw) (ui_page_obj_t *return_menu, void *argv);
    void (*timer_handler)(void *argv);
} ui_page_handle_t;

#define DECLARE_UI_PAGE_OBJ(name) ui_page_handle_t PAGE_CONTEXT_NAME(name)

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


