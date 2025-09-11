#ifndef __AIRPLANE_MODE_API_H__
#define __AIRPLANE_MODE_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

typedef enum {
    AIRPLANE_MODE_EXIT = 0,
    AIRPLANE_MODE_BY_USER_ACTIVE_ENTER = 1,       // 主动进入
    AIRPLANE_MODE_BY_IMU_PASSIVE_ENTER = 2,      //IMU检查移动被动进入.
} airplane_mode_state_e;

typedef enum {
    SETTINGS_AIRPLANE_MODE_ENTER = 0,
    SETTINGS_AIRPLANE_MODE_EXIT,
} settings_airplane_mode_e;

struct broadcast_obj;
struct window_manager_context;
struct power_manager_context;
struct peripheral_bus_context;
struct misc_manager_context;
struct time_system_context;

typedef struct airplane_mode_context {
// private:
    struct __airplane_private {
        bool is_on;
        bool is_backgroud_check;
        struct broadcast_obj *bcd;
        uint8_t is_breath_pump_working;
        uint8_t enter_state;
        struct window_manager_context *wm;
        struct power_manager_context *pm;
        struct peripheral_bus_context *pbus;
        struct misc_manager_context *misc;
        struct time_system_context *time_sys;
    } priv;

// public:
    int (*enter)(void);
    int (*exit)(int wakeup_reason);
    int (*is_on_airplane_mode)(void);
    void (*set_airplane_mode)(int state);
    int (*delay_enter)(const char *tag, int delay_ms);
} airplane_mode_context_t;

#ifdef __cplusplus
}
#endif

#endif 
