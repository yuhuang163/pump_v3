#ifndef __LED_MANAGER_API_H__
#define __LED_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
 #include "root_common_base.h" // IWYU pragma: keep
 #include "broadcast_manager_api.h" // IWYU pragma: keep
 #include "root_message.h" // IWYU pragma: keep

typedef enum {
    LED_BROADCAST_NIGHT_LIGHT_ONOFF = 100, /// 夜灯开关.
    LED_BROADCAST_NIGHT_LIGHT_LEVEL,      /// 夜灯亮度.
    LED_BROADCAST_NIGHT_LIGHT_ADAPTIVE,   /// 夜灯自适应开关/调光.
    LED_BROADCAST_ID_MAX,
} led_broadcast_id_e;

#define ADD_LED_STATE_CB(ctx, user_data, cb) \
do { \
    if (ctx) { \
        ctx->add_led_state_cb(user_data, cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

#define REMOVE_LED_STATE_CB(ctx, cb) \
do { \
    if (ctx) { \
        ctx->remove_led_state_cb(cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

/// LED 状态快照（注册回调分发载荷）
typedef struct {
    uint8_t evt_id;                       /// led_broadcast_id_e
    uint8_t night_light_switch;           /// 夜灯开关
    uint8_t night_light_gear;             /// 夜灯档位
    uint8_t global_adaptive_dimming_switch;  /// 自适应全局开关
    uint8_t screen_key_level;                /// 屏幕/按键背光亮度
} led_state_info_t;

typedef void (*on_led_state_cb_ptr)(void *user_data, led_state_info_t *info);

typedef enum {
    LED_OP_CODE_SET_UI_KEY_BACKLIGHT = 100, /// 设置UI按键背光.
    LED_OP_CODE_SET_LED_BLINK, /// 设置LED闪烁.
    LED_OP_CODE_SET_LED_LEVEL, /// 设置LED亮度.
    LED_OP_CODE_SET_LED_ONOFF, /// 设置LED开关.
    LED_OP_CODE_TOGGLE_NIGHT_LIGHT, /// 切换夜灯开关.

} led_op_code_e;

/// 描述电池管理器上下文对象
struct rt_device_node;
struct broadcast_obj;
struct power_manager_context;
typedef struct led_manager_context {
/// public:
    void (*set_led_blink)(uint8_t led_index, uint16_t period, uint16_t blink_times, uint8_t finish_status);
    void (*set_led_level)(uint8_t led_index, uint8_t level);
    void (*set_led_onoff)(uint8_t led_index, uint8_t onoff);
    void (*add_led_state_cb)(void *user_data, on_led_state_cb_ptr cb);
    void (*remove_led_state_cb)(on_led_state_cb_ptr cb);
    void (*notify_led_state_change)(led_state_info_t *info);
    /// 外部hook点.
    struct __led_manager_hook {
        int (*on_post_init_route)(struct led_manager_context *ctx);
        int (*on_broadcast_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);
#if IS_SUPPORT_NIGHT_LIGHT
        int (*get_night_light_onoff)(struct led_manager_context *ctx);
        int (*toggle_night_light)(struct led_manager_context *ctx);
        void (*set_night_light_level)(struct led_manager_context *ctx, uint8_t level);
        int (*get_night_light_level)(struct led_manager_context *ctx);
        void (*set_night_light_gear)(struct led_manager_context *ctx, uint8_t gear);
        int (*get_night_light_gear)(struct led_manager_context *ctx);
        void (*set_night_light_switch)(struct led_manager_context *ctx, uint8_t onoff);
        int (*get_night_light_switch)(struct led_manager_context *ctx);
        void (*set_night_light_adaptive_dimming_switch)(struct led_manager_context *ctx, uint8_t onoff);
        int (*get_night_light_adaptive_dimming_switch)(struct led_manager_context *ctx);
        /// 电源键开启夜灯接口
        int (*sync_night_light_power_key)(struct led_manager_context *ctx);
#endif
        void (*set_key_backlight)(struct led_manager_context *ctx, uint8_t level);
        void (*set_key_backlight_level)(struct led_manager_context *ctx, uint8_t level);
        int (*get_key_backlight_level)(struct led_manager_context *ctx);
        void (*key_light_adaptive_level_update)(struct led_manager_context *ctx, uint8_t level);
        void (*set_key_backlight_adaptive_dimming_switch)(struct led_manager_context *ctx, uint8_t onoff);
        void (*set_global_adaptive_dimming_switch)(struct led_manager_context *ctx, uint8_t onoff);
        int (*get_global_adaptive_dimming_switch)(struct led_manager_context *ctx);
        void (*set_adaptive_dimming_fct_suppress)(struct led_manager_context *ctx, uint8_t onoff);/// 工厂模式下,屏/键/夜灯子 ALS 开关抑制
        int (*get_ui_backlight_adaptive_dimming_switch)(struct led_manager_context *ctx);
        int (*get_key_backlight_adaptive_dimming_switch)(struct led_manager_context *ctx);
    } *hook;
/// private:
    struct __led_private {
        struct rt_device_node  *led_dev;
        broadcast_obj_t m_bcd;
        struct power_manager_context *power;
        list_head_t client_list;
        os_mutex_id m_lock;
        #if IS_SUPPORT_NIGHT_LIGHT
        uint8_t night_light_onoff;  /// 夜灯亮度开关
        uint8_t night_light_switch; /// 夜灯功能开关，用于区分夜灯功能是否开启
        uint8_t night_light_level;  /// 夜灯当前 PWM 亮度（pwm_table[] 索引；手动/ALS 共用）
        uint8_t night_light_gear;   /// 夜灯档位，后续评估是否可用night_light_level替代
        uint8_t night_light_adaptive_dimming_switch;  /// 夜灯自适应调节开关，用于屏蔽夜灯 ALS（屏蔽 ALS 时，不更新背光档位）（RAM）    
        #endif
        uint8_t key_light_level_max;
        uint8_t key_light_level_rate;
        uint8_t key_light_adaptive_level;

        uint8_t global_adaptive_dimming_switch;         /// 自适应全局开关（RAM，随 global）
        uint8_t ui_backlight_adaptive_dimming_switch;   /// UI 背光 ALS 开关（RAM，随 global）
        uint8_t key_backlight_adaptive_dimming_switch;  /// 按键背光 ALS 开关（RAM，随 global）
    } priv;
} led_manager_context_t;

#if IS_SUPPORT_NIGHT_LIGHT
/// App 手动关闭夜灯（关 ALS 子开关并 turn_off）
void night_light_on_app_manual_off(led_manager_context_t *led);
#endif

#ifdef __cplusplus
}
#endif

#endif


