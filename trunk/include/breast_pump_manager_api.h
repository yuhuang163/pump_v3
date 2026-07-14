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
   
#if IS_SUPPORT_MODE_WORK_TIME_ACCUMULATE
#if IS_SUPPORT_PUMP_MODE_FAST_TEST
#define TOTAL_PUMP_TIME_MAX                          (3 * 60 * 1000) /// 最大泵奶时间 3分钟
#else
#define TOTAL_PUMP_TIME_MAX                          (1.2 * 60 * 60 * 1000) /// 最大泵奶时间 1.2小时
#endif
#define SETTING_PUMP_TIME_MAX                        (2 * 60 * 60 * 1000) /// app设置的单次泵奶时间最大值 2小时
#else
#define SETTING_PUMP_TIME_MAX                        (60 * 60 * 1000)  /// app设置的单次泵奶时间最大值 1小时
#define TOTAL_PUMP_TIME_MAX                          (60 * 60 * 60 * 1000) /// 最大泵奶时间 10小时
#endif

typedef enum {
    BREAST_PUMP_BROADCAST_MODE_CHANGE = 100,  /// 模式切换.
    BREAST_PUMP_BROADCAST_PUMP_LEVEL_CHANGE,  /// 泵奶档位改变.
    BREAST_PUMP_BROADCAST_PUMP_SPEED_CHANGE,  /// 泵奶速度改变.
    BREAST_PUMP_BROADCAST_CONTROL_TYPE_CHANGE,  /// 控制类型改变.
    BREAST_PUMP_BROADCAST_COMPLETED,          /// 泵奶已完成.
    BREAST_PUMP_BROADCAST_APP_COMPLETED,      /// APP强制完成.
    BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH,   /// 泵奶即将结束.
    BREAST_PUMP_BROADCAST_STATE_CHANGE,       /// 状态改变.
    BREAST_PUMP_BROADCAST_POLICY_CHANGE,      /// 策略改变.
    BREAST_PUMP_BROADCAST_PUMP_ERROR,         /// 泵奶异常.
    BREAST_PUMP_BROADCAST_AUTO_PAUSE_REQUEST, /// 自动暂停请求.
    BREAST_PUMP_BROADCAST_RUNNING_30M_TIP, /// 运行30分钟提示.
    BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH_TIP, /// 即将结束10秒提示.
    BREAST_PUMP_BROADCAST_TIMEOUT, /// 达到最大泵奶时长超时结束.
    BREAST_PUMP_BROADCAST_ID_MAX, 
} breast_pump_broadcast_id_e;

typedef enum {
    RESULT_PUMP_WILL_FINISH_TIP = -7,
    RESULT_PUMP_RUNNING_30M_TIP = -6,
    RESULT_PUMP_AUTO_PAUSE = -5,
    RESULT_PUMP_TIMEOUT = -4,
    RESULT_PUMP_FINISH = -3,
    RESULT_PUMP_WILL_FINISH = -2,
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
} breast_pump_work_status_e;

#define PUMP_NO_FINISH(state) (state == BREAST_PUMP_START || state == BREAST_PUMP_CONTINUE || state == BREAST_PUMP_PAUSE)
#define PUMP_IS_GOING(state) (state == BREAST_PUMP_START || state == BREAST_PUMP_CONTINUE)
#define PUMP_IS_COMMON_MODE(mode) (mode == BREAST_PUMP_LACTATION_MODE || mode == BREAST_PUMP_MIX_MODE ||\
     mode == BREAST_PUMP_STIMULATION_MODE)

#if (PUMP_RHYTHM_MODE_NUM == 3)
#define PUMP_IS_BREASTFEEDING_MODE(mode) (mode == BREAST_PUMP_CUSTOM_MODE || mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||\
    mode == BREAST_PUMP_MILK_BOOST_MODE)
#else
#define PUMP_IS_BREASTFEEDING_MODE(mode) (mode == BREAST_PUMP_CUSTOM_MODE || mode == BREAST_PUMP_PP_MILK_TRACK_MODE ||\
        mode == BREAST_PUMP_MILK_BOOST_MODE || mode == BREAST_PUMP_LACTATION_START_MODE || \
        mode == BREAST_PUMP_REGULAR_PUMPING_MODE || mode == BREAST_PUMP_EMPTYING_BOOST_MODE)
#endif
     
/// 泵奶状态
#define READ_PUMP_IS_WORKING(ctx, is_working) \
    ctx->get_pump_composite_state(BREAST_PUMP_IS_WORKING, 0, 0, &is_working, sizeof(is_working))
#define READ_PUMP_WORK_STATUS(ctx, status) \
    ctx->get_pump_composite_state(BREAST_PUMP_WORK_STATE, 0, 0, &status, sizeof(status))    
#define WRITE_PUMP_WORK_STATUS(ctx, status, is_report) \
    ctx->set_pump_composite_state(BREAST_PUMP_WORK_STATE, status, is_report, 0, NULL)  

/// 泵奶模式
#define READ_PUMP_WORK_MODE(ctx, mode) \
    ctx->get_pump_composite_state(BREAST_PUMP_WORK_MODE, 0, 0, &mode, sizeof(mode))
#define WRITE_PUMP_WORK_MODE(ctx, mode, is_store, is_report) \
    ctx->set_pump_composite_state(BREAST_PUMP_WORK_MODE, mode, is_store, is_report, NULL)
/// 常规模式
#define READ_PUMP_COMMON_MODE(ctx, is_last, mode) \
    ctx->get_pump_composite_state(BREAST_PUMP_COMMON_MODE, is_last, 0, &mode, sizeof(mode))
/// 专家模式
#define READ_PUMP_EXPERT_MODE(ctx, is_last, mode) \
    ctx->get_pump_composite_state(BREAST_PUMP_EXPERT_MODE, is_last, 0, &mode, sizeof(mode))
/// 模式激活
#define READ_PUMP_MODE_ACTIVATE(ctx, mode, status) \
    ctx->get_pump_composite_state(BREAST_PUMP_MODE_ACTIVATE_STATE, mode, 0, &status, sizeof(status))    
#define WRITE_PUMP_MODE_ACTIVATE(ctx, mode, status) \
    ctx->set_pump_composite_state(BREAST_PUMP_MODE_ACTIVATE_STATE, mode, status, 0, NULL)
/// 模式步骤索引
#define READ_PUMP_MODE_STEP_INDEX(ctx, step_index) \
    ctx->get_pump_composite_state(BREAST_PUMP_STEP_INDEX, 0, 0, &step_index, sizeof(step_index))    
/// 泵奶配置 
#define READ_PUMP_SETTING_INFO(ctx, index, setting) \
    ctx->get_pump_composite_state(BREAST_PUMP_MODE_SETTING_INFO, index, 0, &setting, sizeof(setting))    
#define WRITE_PUMP_SETTING_INFO(ctx, index, is_store, setting) \
    ctx->set_pump_composite_state(BREAST_PUMP_MODE_SETTING_INFO, index, is_store, 0, &setting)  
/// 泵奶力度控制类型
#define READ_PUMP_LEVEL_CONTROL_TYPE(ctx, type) \
    ctx->get_pump_composite_state(BREAST_PUMP_LEVEL_CONTROL_TYPE, 0, 0, &type, sizeof(type))    
#define WRITE_PUMP_LEVEL_CONTROL_TYPE(ctx, type , is_report) \
    ctx->set_pump_composite_state(BREAST_PUMP_LEVEL_CONTROL_TYPE, type, is_report, 0, NULL)
/// 泵奶结果信息
#define READ_PUMP_RESULT_INFO(ctx, id, info) \
    ctx->get_pump_composite_state(BREAST_PUMP_RESULT_INFO, id, 0, info, sizeof(info))  
/// 泵奶模式名称
#define READ_PUMP_WORK_MODE_NAME(ctx, name_str) \
    ctx->get_pump_composite_state(BREAST_PUMP_WORK_MODE_NAME, 0, 0, name_str, sizeof(name_str))
/// 泵奶模式名称
#define READ_PUMP_MODE_NAME(ctx, id, name_str) \
    ctx->get_pump_composite_state(BREAST_PUMP_MODE_NAME, id, 0, name_str, sizeof(name_str)) 
/// 泵奶状态名称
#define READ_PUMP_STATE_NAME(ctx, name_str) \
    ctx->get_pump_composite_state(BREAST_PUMP_WORK_STATE_NAME, 0, 0, name_str, sizeof(name_str))        
/// 当前是否由APP启动pump控制
#define READ_PUMP_IS_START_FROM_APP(ctx, is_start_from_app) \
    ctx->get_pump_composite_state(BREAST_PUMP_IS_START_FROM_APP, 0, 0, &is_start_from_app, sizeof(is_start_from_app))
/// 当前是否由APP启动pump控制
#define WRITE_PUMP_IS_START_FROM_APP(ctx, is_start_from_app) \
    ctx->set_pump_composite_state(BREAST_PUMP_IS_START_FROM_APP, is_start_from_app, 0, 0, NULL)
/// 当前模式类型
#define READ_PUMP_WORK_MODE_TYPE(ctx, mode_type) \
    ctx->get_pump_composite_state(BREAST_PUMP_WORK_MODE_TYPE, 0, 0, &mode_type, sizeof(mode_type))
/// 下一个模式类型
#define READ_PUMP_NEXT_WORK_MODE_TYPE(ctx, mode_type) \
    ctx->get_pump_composite_state(BREAST_PUMP_NEXT_WORK_MODE_TYPE, 0, 0, &mode_type, sizeof(mode_type))
/// 工作运行时间(单位ms)
#define READ_PUMP_POLICY_RUNNING_TIME_MS(ctx, running_time_ms) \
    ctx->get_pump_composite_state(BREAST_PUMP_POLICY_RUNNING_TIME_MS, 0, 0, &running_time_ms, sizeof(running_time_ms))
/// 工作剩余时间(单位ms)
#define READ_PUMP_POLICY_REMAINING_TIME_MS(ctx, remaining_time_ms) \
    ctx->get_pump_composite_state(BREAST_PUMP_POLICY_REMAINING_TIME_MS, 0, 0, &remaining_time_ms, sizeof(remaining_time_ms))

#define READ_PUMP_CONTROL_SOURCE(ctx, control_source) \
    ctx->get_pump_composite_state(BREAST_PUMP_CONTROL_SOURCE, 0, 0, &control_source, sizeof(control_source))
#define WRITE_PUMP_CONTROL_SOURCE(ctx, control_source) \
    ctx->set_pump_composite_state(BREAST_PUMP_CONTROL_SOURCE, control_source, 0, 0, NULL)

#define ADD_BREAST_PUMP_RESULT_CB(ctx, user_data, cb) \
do { \
    if (ctx) { \
        ctx->add_breast_pump_result_listener(user_data, cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

#define REMOVE_BREAST_PUMP_RESULT_CB(ctx, cb) \
do { \
    if (ctx) { \
        ctx->remove_breast_pump_result_listener(cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

typedef struct {
    uint8_t status;
    uint8_t setting_mode;
    uint8_t working_mode;
    uint8_t control_type;
    pump_setting_t setting;
    uint32_t work_valid_time;
    uint32_t total_valid_time;
} breast_pump_result_t;

/// 泵奶数据
typedef struct pump_result_info {
    uint8_t  mode;
    uint8_t  report_flag;               /// 泵奶报告是否实时上传
    uint32_t finish_timestamp;          /// 结束泵奶时间戳
    uint32_t start_time;                /// 开始泵奶时间
    uint32_t setting_time;              /// app设置的单次泵奶时间
    uint32_t target_valid_time;         /// 目标有效泵奶时间（不含暂停）
    uint32_t work_valid_time;           /// 有效泵奶时长（不含暂停）
    uint32_t target_total_time;         /// 目标最大工作时长（含暂停）
    uint32_t work_total_time;           /// 泵奶总时长(含暂停)
    uint32_t current_mode_work_time;    /// 当前模式运行时间（不含暂停）
    uint32_t hostory_work_time;         /// 历史运行时间（不含暂停）
    pump_setting_t setting[ROOT_BREAST_PUMP_STEP_MAX];
} pump_result_info_t;

typedef void (*on_breast_pump_result_cb_ptr)(void *user_data, breast_pump_result_t *result);

/// 吸奶器信息
typedef struct breast_pump_info_t {
    uint8_t status;        /// 吸奶器状态
    uint8_t do_work;       /// 是否正在工作
    uint8_t mode;          /// 吸奶器模式
    uint8_t step_index;    /// 吸奶器档位索引
    uint8_t step_remaining_time;   /// 吸奶器当前步长剩余时间
    uint8_t control_type;          /// 吸奶器控制类型
    uint8_t breastfeeding_mode;    /// 追奶模式
    uint8_t common_mode;           /// 常规模式
    uint8_t running_tip_onoff;     /// 运行提示开关
    uint8_t custom_activate;       /// 个性化韵律激活
    uint8_t is_start_from_app;     /// 当前是否由APP启动pump控制
    uint8_t control_source;        /// 泵奶控制来源: 0-本地, 1-APP
    // pump_process_result_e result_id;    /// 结果ID
} breast_pump_info_t;

/// 吸奶结果ID
typedef enum{
    BREAST_PUMP_RESULT_PUMPING,    /// 吸奶中的信息 
    BREAST_PUMP_RESULT_HISTORY,    /// 上次有效的泵奶信息 >30秒
    BREAST_PUMP_RESULT_MAX,
} breast_pump_result_id_e;

/// 吸奶模式类型
typedef enum{
    BREAST_PUMP_MODE_TYPE_NONE,    
    BREAST_PUMP_MODE_TYPE_COMMON,    /// 常规模式
    BREAST_PUMP_MODE_TYPE_BREASTFEEDING,    /// 追奶模式
    BREAST_PUMP_MODE_TYPE_MAX,
} breast_pump_mode_type_e;

/// 吸奶状态类型
typedef enum{
    BREAST_PUMP_WORK_STATE,           /// 泵奶工作状态
    BREAST_PUMP_WORK_STATE_NAME,      /// 泵奶工作状态名称
    BREAST_PUMP_MODE_ACTIVATE_STATE,    /// 模式激活状态
    BREAST_PUMP_MODE_STEPS_UPDATE_STATE, /// 参数更新状态
    BREAST_PUMP_LEVEL_CONTROL_TYPE,      /// 泵奶力度控制类型
    BREAST_PUMP_STEP_INDEX,    /// 泵奶策略步骤索引
    BREAST_PUMP_IS_WORKING,    /// 泵奶运行中
    BREAST_PUMP_WORK_MODE,     /// 泵奶当前模式
    BREAST_PUMP_COMMON_MODE,    /// 常规模式 
    BREAST_PUMP_EXPERT_MODE,    /// 专家模式
    BREAST_PUMP_WORK_MODE_NAME,    /// 泵奶当前模式
    BREAST_PUMP_MODE_NAME,         /// 泵奶当前模式
    BREAST_PUMP_RESULT_INFO,       /// 泵奶结果信息
    BREAST_PUMP_MODE_SETTING_INFO,    /// 泵奶模式设置信息
    BREAST_PUMP_IS_START_FROM_APP,    /// 当前是否由APP启动pump控制
    BREAST_PUMP_CURRENT_MODE_TYPE,    /// 当前模式类型
    BREAST_PUMP_NEXT_MODE_TYPE,       /// 下一个模式类型
    BREAST_PUMP_TIME_POLICY_INDEX,    /// 当前时间策略索引
    BREAST_PUMP_CONTROL_SOURCE,      /// 泵奶控制来源: 0-本地, 1-APP
} pump_state_type_e;

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
        breast_pump_info_t m_info;
        breast_pump_result_t m_result;
        pump_setting_t setting[ROOT_BREAST_PUMP_STEP_MAX];
        thread_context_t breast_pump_thd;
        list_head_t client_list;
        broadcast_obj_t m_bdc;
        struct storage_manager_context *st;
        struct peripheral_bus_context *pbus;
        struct motor_manager_context *motor;
        struct battery_manager_context *battery;
        pump_result_info_t result_info[BREAST_PUMP_RESULT_MAX];
        uint32_t completed_notify_time;
        uint32_t will_finish_notify_time;
        uint32_t will_finish_tip_notify_time;
    } priv;

    ///  public
    int (*add_breast_pump_result_listener)(void *user_data, on_breast_pump_result_cb_ptr cb);
    void (*remove_breast_pump_result_listener)(on_breast_pump_result_cb_ptr cb);
    void (*get_pump_composite_state)(pump_state_type_e type, int arg1, int arg2, void *out_data, int data_szie);
    void (*set_pump_composite_state)(pump_state_type_e type, int arg1, int arg2, int arg3, void *in_data);
    ///  外部挂接点.
    struct __breast_pump_hook {
        int (*make_progress_route)(struct breast_pump_manager_context *ctx, pump_result_info_t *result_info);
        int (*breast_pump_completed_route)(struct breast_pump_manager_context *ctx);
        void (*start_breast_pump_work_route)(struct breast_pump_manager_context *ctx, int state);
        void (*stop_breast_pump_work_route)(struct breast_pump_manager_context *ctx, int state);
        void (*mode_change_route)(struct breast_pump_manager_context *ctx, uint8_t mode); /// 模式切换路由.
    } *hook;
} breast_pump_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


