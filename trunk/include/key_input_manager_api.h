#ifndef __KEY_INPUT_MANAGER_API_H__
#define __KEY_INPUT_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_message.h" 

typedef int (*on_key_event_handler_fptr)(void *context, uint16_t keycode, uint16_t keytype);

/// 按键事件类型
typedef enum {
    KEY_EVENT_DOWN    = (1 << 0),          /// 按下
    KEY_EVENT_UP      = (1 << 1),          /// 抬起.
    KEY_EVENT_CLICK   = (1 << 2),          /// 单击
    KEY_EVENT_DOUBLE  = (1 << 3),          /// 双击
    KEY_EVENT_LONG_2S = (1 << 4),          /// 长按2s
    KEY_EVENT_LONG_3S = (1 << 5),
    KEY_EVENT_LONG_4S = (1 << 6),
    KEY_EVENT_LONG_5S = (1 << 7),
    KEY_EVENT_LONG_6S = (1 << 8),
    KEY_EVENT_LONG_7S = (1 << 9),
    KEY_EVENT_LONG_8S = (1 << 10),
    KEY_EVENT_LONG_9S = (1 << 11),
    KEY_EVENT_LONG_10S = (1 << 12),
    KEY_EVENT_LONG_AFTER_DOWN = (1 << 13),
    KEY_EVENT_LONG_AFTER_CLICK = (1 << 14),
    KEY_EVENT_MAX
} key_event_enum_e;

/// 按键类型
typedef enum {
    KEY_CODE_1  = (1 << 0),   /// 支持唤醒
    KEY_CODE_2  = (1 << 1),  
    KEY_CODE_3  = (1 << 2),   
    KEY_CODE_4  = (1 << 3),   
    KEY_CODE_5  = (1 << 4),   
    KEY_CODE_6  = (1 << 5),   
    KEY_CODE_7  = (1 << 6),  
    KEY_CODE_8  = (1 << 7),  
    KEY_CODE_9  = (1 << 8),  
    KEY_CODE_10 = (1 << 9),  
    KEY_CODE_11 = (1 << 10),  
    KEY_CODE_MAX
} key_type_enum_e;

struct rt_device_node;
struct broadcast_obj;
typedef struct key_input_context {
/// private
    struct __key_input_private {
        /// 按键.
        os_mutex_id key_evt_lock;
        input_key_event_data_t key_event;
        struct rt_device_node *key_dev;
        thread_context_t key_evt_thd;
        list_head_t key_evt_list;
    } priv;

/// public
    /// 按键接口.
    int (*read_btn_id)(void *ptr);
    int (*is_key_triggered)(uint16_t key_index);
    int (*dispatch_key_event_wrapper)(void *context, int keycode, int keytype);
    int (*dispatch_client_input_key)(uint16_t keycode, uint16_t keytype);
    void (*add_keyevent_handler)(const char *caller, void *user_data, uint16_t keycode, uint16_t keytype,
         on_key_event_handler_fptr cb);
    void (*remove_keyevent_handler)(on_key_event_handler_fptr cb);
    /// 定义外部钩子函数.
    struct __key_input_hook {
        int (*input_keyevent_route)(struct key_input_context *ctx, uint16_t keycode, uint16_t keytype, int flag);
        const char *(*get_key_code_name)(uint16_t keycode);
    } *hook;

} key_input_context_t;


#ifdef __cplusplus
}
#endif

#endif 


