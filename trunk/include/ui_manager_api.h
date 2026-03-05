#ifndef __UI_MANAGER_API_H__
#define __UI_MANAGER_API_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h" // IWYU pragma: keep
#include "root_message.h" // IWYU pragma: keep
#if IS_SUPPORT_LVGL
#include "lvgl.h" // IWYU pragma: keep
#include "lv_hal_disp.h" // IWYU pragma: keep
#include "lv_hal_indev.h" // IWYU pragma: keep
#endif

#ifndef LVGL_TIME_TICK_PERIOD
#define LVGL_TIME_TICK_PERIOD (10)
#endif
#define LVGL_HDR_TASK_PERIOD (16)

typedef enum {
    ANI_TIMER_DRAW_BEFORE = 0, /// 背景合并之前
    ANI_TIMER_DRAW_AFTER,      /// 背景合并之后
    ANI_TIMER_DRAW_FINISH,     /// 退出.
    ANI_POPUP_DISPLAY_FINISH,  /// 弹窗UI显示完成
} ui_ani_progress_state_e;

/// 动画绘制进度回调.
typedef void (*progress_cb_fptr)(void *user_data, int index, ui_ani_progress_state_e state);
/// 动画绘制准备回调.
typedef void (*display_prepare_cb_fptr)(void *user_data);
/// 动画绘制移除回调.
typedef void (*display_remove_cb_fptr)(void *user_data);
/// 动画绘制信息.
typedef struct ui_play_info {
    const char *tag;
    void *user_data;
    const uint16_t *res_array;
    uint16_t res_count;
    int fps;
    progress_cb_fptr progress_cb;
    display_prepare_cb_fptr display_prepare_cb;
    display_remove_cb_fptr display_remove_cb;
} ui_play_info_t;

/// 动画绘制定时器.
struct _ui_timer_t;
typedef void (*ui_timer_cb_t)(struct _ui_timer_t *);
typedef struct _ui_timer_t {
    uint32_t period; /**< How often the timer should run*/
    uint32_t last_run; /**< Last time the timer ran*/
    ui_timer_cb_t timer_cb; /**< Timer function*/
    void * user_data; /**< Custom user data*/
    int32_t repeat_count; /**< 1: One time;  -1 : infinity;  n>0: residual times*/
    uint32_t paused : 1;
} ui_timer_t;

/// 动画绘制对象.
typedef struct ui_animation_play {
    /// private:
    char *tag;
    void *user_data;
    uint16_t played_count;
    ui_timer_t *update_timer;
    /// public:
    progress_cb_fptr progress_cb;
    display_prepare_cb_fptr display_prepare_cb;
    display_remove_cb_fptr display_remove_cb;
    void (*reset_play_count_cb)(struct ui_animation_play *thiz);
} ui_animation_play_t;

struct rt_device_node;
struct resource_manager_context;
struct power_manager_context;
typedef struct ui_manager_context {
    /// private:
    struct __ui_manager_private {
        thread_context_t ui_hdr_thd; /// UI显示调度线程
    #if IS_SUPPORT_LVGL
            lv_timer_t *lv_timer; /// LVGL定时器
    #else
            os_timer_id lv_timer; /// LVGL定时器
    #endif
        os_mutex_id ui_draw_lock; /// 显示刷新锁
        broadcast_obj_t m_bcd; /// 广播监听对象
        struct rt_device_node *panel_dev;   /// 显示设备
        struct rt_device_node *led_dev;     /// LED设备
        bool is_playing_ui_animation;       /// 是否正在播放UI动画
        bool is_playing_ui_overlay;         /// 是否正在播放UI overlay（弹窗等叠加层）
        uint8_t ui_backlight_val;           /// 背光亮度
        bool ui_backlight_onoff;            /// 背光开关
        uint8_t ui_backlight_adaptive_val;  /// LCD背光自适应亮度
        bool lv_tick_timer_running;  /// LVGL定时器是否正在运行
    #if IS_SUPPORT_LVGL
        bool lvgl_initialized;      /// LVGL是否已初始化完成
        uint16_t fps_count;         /// FPS计数器
        uint32_t fps_start_time;    /// FPS开始时间戳
    #endif
    #if IS_SUPPORT_MODULE_RES
        struct resource_manager_context *res; /// 资源管理上下文
    #endif
        ui_animation_play_t play_info; /// 当前播放的动画信息
        ui_animation_play_t overlay_play_info; /// overlay 动画信息（不会移除 base）
    } priv;

    /// public:
    void (*play_animation_lock)(const char *tag);
    void (*play_animation_unlock)(const char *tag);

#if IS_SUPPORT_LVGL
    bool (*is_lvgl_initialized)(struct ui_manager_context *thiz);
#endif
    /// 外部hook点.
    struct __ui_manager_hook {
        int (*on_post_init_route)(struct ui_manager_context *thiz);
        int (*on_perpare_init_route)(struct ui_manager_context *thiz);
        void (*on_ui_broadcast_receiver_route)(broadcast_mask_id_t id, void *context, 
            int arg1, int arg2, broadcast_priv_data_t *pdata);
        void (*ui_thread_init_route)(struct ui_manager_context *thiz);
        void (*lvgl_display_handler)(struct ui_manager_context *thiz);
        void (*ui_animation_play_init)(struct ui_manager_context *thiz, ui_play_info_t play_info);
        void (*ui_animation_play_deinit)(struct ui_manager_context *thiz);
        void (*ui_animation_overlay_init)(struct ui_manager_context *thiz, ui_play_info_t play_info);
        void (*ui_animation_overlay_deinit)(struct ui_manager_context *thiz);
        void (*set_ui_backlight_onoff)(struct ui_manager_context *thiz, bool onoff);
        void (*set_ui_backlight_level)(struct ui_manager_context *thiz, uint8_t level);
        void (*set_ui_backlight_adaptive_level)(struct ui_manager_context *thiz, uint8_t level);
    } *hook;
} ui_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

