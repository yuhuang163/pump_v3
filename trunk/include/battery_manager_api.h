#ifndef __BATTERY_MANAGER_API_H__
#define __BATTERY_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
 #include "root_common_base.h" // IWYU pragma: keep
 #include "broadcast_manager_api.h" // IWYU pragma: keep
 #include "root_message.h" // IWYU pragma: keep

/// 空电自动关机阈值
#ifndef EMPTY_BAT_PERCENTAGE
#define EMPTY_BAT_PERCENTAGE                     1
#endif

/// 低电自动关机阈值
#ifndef LOW_BAT_PERCENTAGE
#define LOW_BAT_PERCENTAGE                       5
#endif

/// 禁止启动电量阈值
#ifndef FORBID_START_BAT_PERCENTAGE
#define FORBID_START_BAT_PERCENTAGE              10
#endif

/// 低功耗运行阈值
#ifndef LOW_POWER_RUN_PERCENTAGE
#define LOW_POWER_RUN_PERCENTAGE                 20
#endif

typedef enum {
    BATTERY_STATE_NONE = 0, /// 正常.
    BATTERY_STATE_EMPTY = 1, /// 空电量.
    BATTERY_STATE_LOW_POWER_SHUTDOWN = 2, /// 低电量关机.
    BATTERY_STATE_FORBID_START = 3, /// 禁止启动.
    BATTERY_STATE_LOW_POWER_RUN = 4, /// 低功耗运行.
    BATTERY_STATE_MAX,
} battery_state_e;

#define BATTERY_IS_CHARGING(status)       ((status) == CHARGE_STATE_CHARGING || (status) == CHARGE_STATE_FULL)
#define USB_IS_PLUGIN(status)             ((status) == 1)

/// 电池状态广播ID
typedef enum {
    BATTERY_BROADCAST_CHARGE_PLUG_IN = 100, /// 充电插头插入.
    BATTERY_BROADCAST_CHARGE_PLUG_OUT,      /// 充电插头拔出.
    BATTERY_BROADCAST_CHARGE_STATE_CHANGE,  /// 充电状态改变. 
    BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH, /// 充电适配器不匹配.
    BATTERY_BROADCAST_CHARGE_ADAPTER_MISMATCH_RECOVER, /// 充电适配器不匹配恢复.
    BATTERY_BROADCAST_ID_MAX,
} battery_broadcast_id_e;

/// 电池信息
typedef struct {
    uint8_t battery_state;     /// 电池状态.
    uint8_t battery_percent;   /// 电池电量.
    uint8_t charge_status;     /// 充电状态.
    uint8_t adapter_mismatch;  /// 适配器不匹配.
} battery_state_info_t;

/// 电池状态改变回调函数类型
typedef void (*on_battery_state_cb_ptr)(void *user_data, battery_state_info_t *info);

/// 描述电池管理器上下文对象
struct rt_device_node;
struct broadcast_obj;
typedef struct battery_manager_context {
/// private:
    struct __battery_private {
        uint8_t is_vbus_plug_in;
        uint8_t is_charging;
        uint8_t battery_percent;
        uint8_t battery_state;
        uint8_t is_adapter_mismatch;   /// 0 正常；1 适配器不匹配；
        struct rt_device_node *battery;
        struct misc_manager_context *misc;
        list_head_t client_list;
        os_mutex_id m_lock;
    } priv;
/// public:
    uint8_t (*is_vbus_plug_in)(void);
    uint8_t (*get_charge_status)(void);
    uint8_t (*is_low_battery)(void);
    uint8_t (*get_battery_state)(void);
    uint8_t (*get_battery_percent)(void);
    uint8_t (*is_adapter_mismatch)(void); 
    void (*add_battery_state_cb)(void *user_data, on_battery_state_cb_ptr cb);
    void (*remove_battery_state_cb)(on_battery_state_cb_ptr cb);
} battery_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


