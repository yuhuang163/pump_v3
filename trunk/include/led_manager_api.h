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
    uint8_t night_light_onoff;            /// 夜灯开关
    uint8_t night_light_level;            /// 夜灯当前亮度
    uint8_t night_light_adaptive_switch;  /// 夜灯自适应开关
    uint8_t night_light_adaptive_dimming; /// 夜灯自适应调光
} led_state_info_t;

typedef void (*on_led_state_cb_ptr)(void *user_data, led_state_info_t *info);

typedef enum {
    LED_OP_CODE_SET_UI_KEY_BACKLIGHT = 100, /// 设置UI按键背光.
    LED_OP_CODE_SET_LED_BLINK, /// 设置LED闪烁.
    LED_OP_CODE_SET_LED_LEVEL, /// 设置LED亮度.
    LED_OP_CODE_SET_LED_ONOFF, /// 设置LED开关.
    LED_OP_CODE_TOGGLE_NIGHT_LIGHT, /// 切换夜灯开关.
    LED_OP_CODE_SET_NIGHT_LIGHT_LEVEL, /// 设置夜灯亮度.
    LED_OP_CODE_SET_NIGHT_LIGHT_ADAPTIVE_SWITCH,  /// 设置夜灯自适应开关.
    LED_OP_CODE_SET_NIGHT_LIGHT_ADAPTIVE_DIMMING, /// 设置夜灯自适应调光.
    LED_OP_CODE_GET_NIGHT_LIGHT_ADAPTIVE_SWITCH,  /// 获取夜灯自适应开关.
    LED_OP_CODE_GET_NIGHT_LIGHT_ADAPTIVE_DIMMING, /// 获取夜灯自适应调光.
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
        int (*toggle_night_light)(struct led_manager_context *ctx);
        void (*set_night_light_level)(struct led_manager_context *ctx, uint8_t level); /// 设置夜灯亮度会自动关闭自适应调光.
        void (*night_light_adaptive_level_update)(struct led_manager_context *ctx, uint8_t level);
        void (*set_night_light_adaptive_switch)(struct led_manager_context *ctx, uint8_t onoff);
        int (*get_night_light_adaptive_switch)(struct led_manager_context *ctx);
        void (*set_night_light_adaptive_dimming)(struct led_manager_context *ctx, uint8_t onoff);
        int (*get_night_light_adaptive_dimming)(struct led_manager_context *ctx);
        void (*set_key_backlight)(struct led_manager_context *ctx, uint8_t level);
        int (*get_night_light_level)(struct led_manager_context *ctx);
        int (*get_night_light_onoff)(struct led_manager_context *ctx);
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
        void (*key_light_adaptive_level_update)(struct led_manager_context *ctx, uint8_t level);
        void (*set_key_light_adaptive_dimming)(struct led_manager_context *ctx, uint8_t onoff);
#endif
    } *hook;
/// private:
    struct __led_private {
        struct rt_device_node  *led_dev;
        broadcast_obj_t m_bcd;
        struct power_manager_context *power;
        list_head_t client_list;
        os_mutex_id m_lock;
        uint8_t night_light_onoff; /// 夜灯背光开关.
        uint8_t night_light_adaptive_level;   /// 夜灯自适应亮度.
        uint8_t night_light_setting_level;    /// 夜灯设置亮度.
        uint8_t night_light_level; /// 夜灯当前亮度.
        uint8_t night_light_adaptive_switch;  /// 夜灯自适应开关.
        uint8_t night_light_adaptive_dimming; /// 夜灯自适应调光.
        
        uint8_t key_light_level_max; /// 按键背光亮度上限.
        uint8_t key_light_level_rate; /// 按键背光调光系数，配合低电状态用
#if IS_SUPPORT_KEY_LIGHT_ADAPTIVE
        uint8_t key_light_adaptive_level; /// 按键背光自适应亮度.
        uint8_t key_light_adaptive_dimming; /// 按键背光自适应调光.
#endif
    } priv;
} led_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


