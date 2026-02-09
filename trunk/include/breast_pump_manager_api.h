#ifndef __BREAST_PUMP_MANAGER_API_H__
#define __BREAST_PUMP_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "persist_storage.h" // IWYU pragma: keep

#define PUMP_WORK_TIME_INC                           (60000)      
#define SINGLE_PUMP_TIME_MAX                         (600000 * 120)   /// 单次泵奶(包含续刷)最多120分钟
#define SETTING_PUMP_TIME_MAX                        (3600000)  /// app设置的单次泵奶时间最大值 1小时

typedef enum {
    BREAST_PUMP_BROADCAST_MODE_CHANGE = 100,  /// 模式切换.
    BREAST_PUMP_BROADCAST_PUMP_LEVEL_CHANGE,  /// 泵奶档位改变.
    BREAST_PUMP_BROADCAST_PUMP_SPEED_CHANGE,  /// 泵奶速度改变.
    BREAST_PUMP_BROADCAST_CONTROL_TYPE_CHANGE,  /// 控制类型改变.
    BREAST_PUMP_BROADCAST_COMPLETED,          /// 泵奶已完成.
    BREAST_PUMP_BROADCAST_STATE_CHANGE,       /// 状态改变.
    BREAST_PUMP_BROADCAST_POLICY_CHANGE,      /// 策略改变.
    BREAST_PUMP_BROADCAST_ID_MAX, 
} breast_pump_broadcast_id_e;

typedef enum {
    RESULT_PUMP_TIMEOUT = -3,
    RESULT_PUMP_FINISH = -2,
    RESULT_PUMP_MOTOR_SHAKE = -1,
    RESULT_PUMP_NORMAL = 0,
    RESULT_PUMP_DURATION_REACHED = 1,  //达到泵奶设定时长
} pump_process_result_e;

typedef enum {
    BREAST_PUMP_IDLE = 0,
    BREAST_PUMP_START,
    BREAST_PUMP_PAUSE,
    BREAST_PUMP_CONTINUE,
    BREAST_PUMP_FINISH,
    BREAST_PUMP_FACTORY,
    BREAST_PUMP_RESET,
    BREAST_PUMP_ERROR,
} breast_pump_status_e;

#define PUMP_IS_GOING(state) (state == BREAST_PUMP_START || state == BREAST_PUMP_CONTINUE)
#define PUMP_IS_COMMON_MODE(mode) (mode == BREAST_PUMP_LACTATION_MODE || mode == BREAST_PUMP_MIX_MODE ||\
     mode == BREAST_PUMP_STIMULATION_MODE)

typedef struct {
    uint8_t breast_pump_state;
    uint32_t real_valid_time_sec;        ///  实际有效泵奶时间 (s)
    uint32_t expected_valid_time_sec;    ///  目标有效泵奶时间(max time) (s)
    uint32_t elapse_time_sec;            ///  自然流逝时间.
    uint32_t expected_elapse_time_sec;   ///  目标自然流逝时间
    uint32_t work_total_time;
} breast_pump_result_t;

/// 泵奶数据
typedef struct pump_result_info {
    uint8_t  mode;
    pump_setting_t setting[ROOT_BREAST_PUMP_STEP_MAX];
    uint8_t  report_flag;               /// 泵奶报告是否实时上传
    uint32_t timestamp;                 /// 开始泵奶时间
    uint32_t setting_time;              /// app设置的单次泵奶时间
    uint32_t target_valid_time;         /// 目标有效泵奶时间
    uint32_t target_total_time;         /// 目标最大工作时长 电子压感存在时使用
    uint32_t work_total_time;           /// 泵奶总时长
    uint32_t work_valid_time;           /// 有效泵奶时长, 预留后续扩展使用
    uint32_t current_mode_work_time;    /// 当前模式运行时间
} pump_result_info_t;

typedef void (*on_breast_pump_result_cb_ptr)(void *user_data, breast_pump_result_t *result);

/// 吸奶器信息
typedef struct breast_pump_info_t {
    uint8_t status;        /// 吸奶器状态
    uint8_t do_work;       /// 是否正在工作
    uint8_t mode;          /// 吸奶器模式
    uint8_t step_index;    /// 吸奶器档位索引
    uint8_t control_type;  /// 吸奶器控制类型
    uint8_t is_custom_trial_mode;    /// custom试用模式开关
} breast_pump_info_t;

/// 吸奶结果ID
typedef enum{
    BREAST_PUMP_RESULT_PUMPING,    /// 吸奶中的信息 
    BREAST_PUMP_RESULT_HISTORY,    /// 上次有效的泵奶信息 >30秒
    BREAST_PUMP_RESULT_MAX,
} breast_pump_result_id_e;

struct peripheral_bus_context;
struct motor_manager_context;
struct storage_context_struct;

typedef struct breast_pump_manager_context {
///  private
    struct __breast_pump_manager_private {
        os_mutex_id m_lock;
        os_mutex_id breast_pump_state_lock;
        os_mutex_id breast_pump_mode_lock;
        os_timer_id breast_pump_work_timer;
        breast_pump_result_t m_result;
        breast_pump_info_t m_info;
        pump_setting_t setting[ROOT_BREAST_PUMP_STEP_MAX];
        thread_context_t breast_pump_thd;
        list_head_t client_list;
        broadcast_obj_t m_bdc;
        struct storage_manager_context *st;
        struct peripheral_bus_context *pbus;
        struct motor_manager_context *motor;
        pump_result_info_t result_info[BREAST_PUMP_RESULT_MAX];  
    } priv;

///  public
    int (*is_on_working)(void);
    void (*reset_breast_pump_result)(void);
    int (*add_breast_pump_result_listener)(void *user_data, on_breast_pump_result_cb_ptr cb);
    void (*remove_breast_pump_result_listener)(on_breast_pump_result_cb_ptr cb);
    int (*get_breast_pump_mode)(void);
    void (*set_breast_pump_mode)(uint32_t mode, uint8_t is_store_mode);
    int (*get_breast_pump_control_type)(void);
    void (*set_breast_pump_control_type)(uint8_t control_type);
    void (*get_breast_pump_setting)(pump_setting_t *setting, uint8_t step_index);
    void (*set_breast_pump_setting)(pump_setting_t *setting, uint8_t step_index);
    void (*set_breast_pump_status)(int status);
    int (*get_breast_pump_status)(void);
    int (*get_breast_pump_status_async)(void);
    uint8_t (*get_breast_pump_step_index)(void);
    int (*get_last_breast_pump_result)(breast_pump_result_t *result, uint8_t breast_pump_data_id);
    const breast_pump_result_t *(*get_curr_breast_pump_result)(void);
    pump_result_info_t *(*get_breast_pump_result)(breast_pump_result_id_e result_id);
    const char *(*get_breast_pump_status_name)(uint8_t status);
    const char *(*get_breast_pump_mode_name)(uint8_t mode);
    uint8_t (*get_is_custom_trial_mode)(void);
    void (*set_is_custom_trial_mode)(uint8_t is_custom_trial_mode);

    ///  外部挂接点.
    struct __breast_pump_hook {
        int (*make_progress_route)(struct breast_pump_manager_context *ctx, pump_result_info_t *result_info);
        int (*breast_pump_completed_route)(struct breast_pump_manager_context *ctx);
        int (*start_breast_pump_work_route)(struct breast_pump_manager_context *ctx, int state);
        int (*stop_breast_pump_work_route)(struct breast_pump_manager_context *ctx, int state);
        int (*breast_pump_result_info_app_reset)(struct breast_pump_manager_context *ctx, pump_result_info_t *result_info);
    } *hook;
} breast_pump_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


