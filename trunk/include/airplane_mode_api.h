#ifndef __AIRPLANE_MODE_API_H__
#define __AIRPLANE_MODE_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"

typedef enum {
    AIRPLANE_MODE_EXIT = 0,
    AIRPLANE_MODE_BY_USER_ACTIVE_ENTER = 1,       /// 主动进入
    AIRPLANE_MODE_BY_IMU_PASSIVE_ENTER = 2,      ///IMU检查移动被动进入.
} airplane_mode_state_e;

struct broadcast_obj;
struct window_manager_context;
struct power_manager_context;
struct peripheral_bus_context;
struct misc_manager_context;
struct time_system_context;

typedef struct airplane_mode_context {
/// private:
    struct __airplane_private {
        bool is_on;
        bool is_backgroud_check;
        bool is_count_down_popup_show;
        uint8_t is_breast_pump_working;
        uint8_t enter_state;
        struct broadcast_obj *bcd;
        struct window_manager_context *wm;
        struct power_manager_context *pm;
        struct peripheral_bus_context *pbus;
        struct misc_manager_context *misc;
        struct time_system_context *time_sys;
    } priv;
/// public:
    int (*enter)(int enter_reason);
    int (*exit)(int exit_reason);
    int (*is_on_airplane_mode)(void);
    void (*set_airplane_mode)(int state);
/// 外部hook点.
    struct __airplane_mode_hook {
        int (*on_post_init_route)(struct airplane_mode_context *ctx);
        int (*on_airplane_mode_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        int (*enter_airplane_mode_route)(struct airplane_mode_context *ctx, int enter_reason);
        int (*exit_airplane_mode_route)(struct airplane_mode_context *ctx, int exit_reason);
        void (*travel_lock_key_event_handler)(struct airplane_mode_context *ctx, void *user_data); /// 旅行锁按键事件处理
    } *hook;
} airplane_mode_context_t;

#ifdef __cplusplus
}
#endif

#endif 
