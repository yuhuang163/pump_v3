#ifndef __UI_MANAGER_API_H__
#define __UI_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h" // IWYU pragma: keep
#include "root_message.h" // IWYU pragma: keep

#ifndef LVGL_TIME_TICK_PERIOD
#define LVGL_TIME_TICK_PERIOD (10)
#endif
#define LVGL_HDR_TASK_PERIOD (16)

#if !CUST_LVGL_SUPPORT
typedef uint16_t ui_color_t;
typedef void * ui_obj_t;
typedef struct {

    uint32_t cf : 5;          /*Color format: See `lv_img_color_format_t`*/
    uint32_t always_zero : 3; /*It the upper bits of the first byte. Always zero to look like a
                                 non-printable character*/

    uint32_t reserved : 2; /*Reserved to be used later*/

    uint32_t w : 11; /*Width of the image map*/
    uint32_t h : 11; /*Height of the image map*/
} ui_img_header_t;

typedef struct {
    ui_img_header_t header; /**< A header describing the basics of the image*/
    uint32_t data_size;     /**< Size of the image in bytes*/
    const uint8_t * data;   /**< Pointer to the data of the image*/
} ui_img_dsc_t;
#endif

/// 获取图片资源回调.
typedef void *(*ui_get_img_res_loop_fptr)(void *user_data, int args1, int args2);
typedef struct {
    ui_get_img_res_loop_fptr get_img_res_cb;
    const char *(*get_dest_text_show)(int args1, int args2);
    void (*show_ui_text)(int *argv, int argc, const char *text);
    void (*set_bg_img_color)(int args, ui_color_t color);
    void *(*draw_qrcode_view)(void *user_data, int qr_size, int x, int y, const char *text);
    int (*animation_progress_hook)(void *user_data, int index, int total);
    uint16_t ani_loop_play;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    void *bg_img_obj;
#if IS_SUPPORT_OVERLAY_IMG
    uint8_t overlay_img_num;
    void *overlay_img_bg_img_obj[OVERLAY_IMG_NUM_MAX];
    ui_obj_t *overlay_img_layer[OVERLAY_IMG_NUM_MAX];
#endif
} ui_process_cb_parms_t;

typedef enum {
    ANI_TIMER_DRAW_BEFORE = 0, /// 背景合并之前
    ANI_TIMER_DRAW_AFTER,      /// 背景合并之后
    ANI_TIMER_DRAW_FINISH,     /// 退出.
} ui_ani_progress_state_e;

/// 动画绘制进度回调.
typedef void (*progress_cb_fptr)(void *user_data, int index, ui_ani_progress_state_e state);

/// 动画绘制信息.
typedef struct ui_play_info {
    const char *tag;
    void *user_data;
    const uint16_t *res_array;
    uint16_t res_count;
    int fps;
    ui_process_cb_parms_t *parms;
    progress_cb_fptr progress_cb;
} ui_play_info_t;

/// 图片数据包装.
typedef struct {
    int exit_flag;
    int res_index;
    int lang_index;
    int mode_index;
    uint16_t x;
    uint16_t y;
    ui_img_dsc_t img_dsc;
} ui_img_dsc_wrapper_t;

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
    uint16_t *res_array;
    uint16_t res_count;
    uint16_t played_count;
    ui_img_dsc_wrapper_t img_wrap;
    #if IS_SUPPORT_OVERLAY_IMG
    ui_img_dsc_wrapper_t img_wrap_overlay[OVERLAY_IMG_NUM_MAX];
    #endif
    ui_timer_t *update_timer;
    /// public:
    void (*progress_cb)(void *user_data, int index, ui_ani_progress_state_e state);
    void (*reset_play_count_cb)(struct ui_animation_play *thiz);
} ui_animation_play_t;

struct rt_device_node;
typedef struct ui_manager_context {
    /// private:
    struct __ui_manager_private {
        thread_context_t ui_hdr_thd; /// UI显示调度线程
        os_timer_id lv_tick_timer; /// LVGL定时器
        os_mutex_id ui_draw_lock; /// 显示刷新锁
        broadcast_obj_t m_bcd; /// 广播监听对象
        struct rt_device_node *panel_dev; /// 显示设备
        struct rt_device_node *led_dev; /// LED设备
        bool is_playing_ui_animation; /// 是否正在播放UI动画
        uint8_t ui_backlight_val; /// 背光亮度
        bool lv_tick_timer_running; /// LVGL定时器是否正在运行
        ui_animation_play_t play_info; /// 当前播放的动画信息
    } priv;

    /// public:
    void (*animation_play_status_update)(struct ui_manager_context *thiz, bool flag);
    void (*play_animation_lock)(const char *tag);
    void (*play_animation_unlock)(const char *tag);
    /// 外部hook点.
    struct __ui_manager_hook {
        int (*on_post_init_route)(struct ui_manager_context *thiz);
        int (*on_perpare_init_route)(struct ui_manager_context *thiz);
        void (*lvgl_display_handler)(struct ui_manager_context *thiz);
        void (*ui_animation_play_init)(struct ui_manager_context *thiz, ui_play_info_t play_info);
        void (*on_ui_broadcast_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);
        void (*set_ui_backlight)(struct ui_manager_context *thiz, uint16_t val);
        bool (*is_ui_backlight_on)(struct ui_manager_context *thiz);
    } *hook;
} ui_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

