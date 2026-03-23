#ifndef __MOTOR_POLICY_H__
#define __MOTOR_POLICY_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_list_api.h" // IWYU pragma: keep

#define PUMP_POLICY_RUN_TIME_INTERVAL            10  /// 策略运行时间间隔，单位ms
#define IS_SUPPORT_RELATIVE_RUN_TIME             1 /// 支持相对运行时间

/// 马达调试ID
typedef enum {
    MOTOR_USER_DEBUG_PUMP_ON = 0,
    MOTOR_USER_DEBUG_PUMP_OFF,
    MOTOR_USER_DEBUG_VALVE_ON,
    MOTOR_USER_DEBUG_VALVE_OFF,
    MOTOR_USER_DEBUG_POLICY,
    MOTOR_USER_TASTE_STIMULATION_POLICY,
    MOTOR_USER_TASTE_PUMP_POLICY,
    MOTOR_USER_TASTE_MIX1_POLICY,
    MOTOR_USER_TASTE_MIX2_POLICY,
    MOTOR_USER_TASTE_POLICY_STOP,
    MOTOR_USER_TASTE_POLICY_START,
    MOTOR_USER_TASTE_POLICY_EXIT,
    MOTOR_USER_DEBUG_ID_MAX,
} motor_user_debug_id_e;

/// 马达动作类型
typedef enum {
    MOTOR_ACTION_IDLE = 0,
    MOTOR_ACTION_PUMP_ON,
    MOTOR_ACTION_PUMP_OFF,
    MOTOR_ACTION_VALVE_ON,
    MOTOR_ACTION_VALVE_OFF,
    MOTOR_ACTION_PUMP_OFF_AND_VALVE_ON,
} motor_action_type_e;

/// 吸力状态
typedef enum {
    SUCTION_STATUS_IDLE = 0,
    SUCTION_STATUS_GENERATE,    /// 生成
    SUCTION_STATUS_HOLD,        /// 保持
    SUCTION_STATUS_RELEASE,     /// 解除
    SUCTION_STATUS_SLEEP,       /// 休眠
} suction_status_e;

/// 吸力模式类型
typedef enum {
    SUCTION_MODE_IDLE = 0,
    SUCTION_MODE_SHORT,
    SUCTION_MODE_LONG, 
    SUCTION_MODE_MIX_SHORT,
    SUCTION_MODE_MIX_LONG,
} suction_mode_type_e;

/// 吸力速度等级
typedef enum {
    SUCTION_SPEED_LOW = 0,
    SUCTION_SPEED_MID,
    SUCTION_SPEED_HIGH,
    SUCTION_SPEED_MAX
} suction_speed_e;

/// 吸力等级
#define SUCTION_DEFAULT_LEVEL_VALUE   5  /// 默认吸力等级，默认5KPA为初始值
typedef enum {
    SUCTION_LEVEL_0KPA = 0,
    SUCTION_LEVEL_6KPA,
    SUCTION_LEVEL_7KPA,
    SUCTION_LEVEL_8KPA,
    SUCTION_LEVEL_9KPA,
    SUCTION_LEVEL_10KPA,
    SUCTION_LEVEL_11KPA,
    SUCTION_LEVEL_12KPA,
    SUCTION_LEVEL_13KPA,
    SUCTION_LEVEL_14KPA,
    SUCTION_LEVEL_15KPA,
    SUCTION_LEVEL_16KPA,
    SUCTION_LEVEL_17KPA,
    SUCTION_LEVEL_18KPA,
    SUCTION_LEVEL_19KPA,
    SUCTION_LEVEL_20KPA,
    SUCTION_LEVEL_21KPA,
    SUCTION_LEVEL_22KPA,
    SUCTION_LEVEL_23KPA,
    SUCTION_LEVEL_24KPA,
    SUCTION_LEVEL_25KPA,
    SUCTION_LEVEL_26KPA,
    SUCTION_LEVEL_27KPA,
    SUCTION_LEVEL_28KPA,
    SUCTION_LEVEL_29KPA,
    SUCTION_LEVEL_30KPA,
    SUCTION_LEVEL_31KPA,
    SUCTION_LEVEL_32KPA,
    SUCTION_LEVEL_33KPA,
    SUCTION_LEVEL_34KPA,
    SUCTION_LEVEL_35KPA,
    SUCTION_LEVEL_36KPA,
    SUCTION_LEVEL_37KPA,
    SUCTION_LEVEL_38KPA,
    SUCTION_LEVEL_39KPA,
    SUCTION_LEVEL_40KPA,
    SUCTION_LEVEL_MAX
} suction_level_e;

/// 吸奶频次参数
typedef struct {
    // uint16_t sleep_time;  /// 循环间隔
    uint16_t pump_freq;   /// 泵频次 1分/次
    uint16_t hold_time;   /// 吸力保持时间，单位ms
} pump_freq_parameters_t;

/// 吸力参数，电磁阀动作时间与吸力强度成正比
typedef struct {
    uint8_t  pump_level;    /// 泵力度
    uint16_t pumping_time;  /// 抽气时间
    uint16_t valve_on_time; /// 电磁阀动作时间
} suction_parameters_t;

typedef struct {
    uint8_t loop_num;     /// 循环次数
    uint16_t period;      /// 周期时间，单位ms, 由频次计算得出
} suction_policy_config_t;

/// 吸力策略映射参数，用来生成吸力参数
typedef struct {
    uint8_t speed;             /// 吸力生成速度，suction_speed_e
    uint8_t level;             /// 吸力等级 suction_level_e
    uint8_t mode_type;         /// 吸力模式类型
} suction_strategy_mapping_t;

/// 体验模式设置
typedef struct {
    uint8_t level[BREAST_PUMP_NUM];
    uint8_t speed;
} taste_setting_t;

/// 马达参数类型
typedef enum {
    MOTOR_PARAM_TABLE = 0,      ///< 查表模式
    MOTOR_PARAM_PASS_THROUGH,   ///< 数据透传模式
} motor_param_type_e;

/// 马达策略原始参数
typedef struct {
    uint8_t  pump_level;    /// 泵力度
    uint16_t pumping_time;  /// 抽气时间
    uint16_t valve_on_time; /// 电磁阀动作时间
    uint16_t sleep_time;    /// 循环间隔
    uint16_t hold_time;     /// 吸力保持时间，单位ms
} motor_policy_original_para_t;

/// 马达策略核心参数
typedef struct {
    uint8_t type;                  /// 参数类型 motor_param_type_e
    union {
        suction_strategy_mapping_t mapping;
        motor_policy_original_para_t original;
    } un;
    suction_policy_config_t config;
} motor_policy_core_t;

/// 马达策略运行状态
typedef struct {
    uint8_t n_pressure_flag;      /// 负压标志，负压状态下不允许切换策略
    uint8_t policy_update_req;    /// 策略更新请求
    uint8_t cycles_count;         /// 循环计数
    uint8_t action;               /// 动作 motor_action_type_e
    uint8_t status;               /// 吸力状态 suction_status_e
    uint8_t pump_duty;            /// 泵奶占空比
    uint8_t valve_duty;           /// 电磁阀门占空比
    uint8_t target_pressure;      /// 目标压力
#if IS_SUPPORT_RELATIVE_RUN_TIME
    uint32_t run_time;            /// 运行时间，单位ms
#else
    uint32_t start_time;          /// 参数启用时间，单位ms
    uint32_t run_time;            /// 运行时间，单位ms
#endif
} motor_policy_state_t;

/// 马达组件状态, 给算法采集用，顺序与motor_policy_state_t保持一致
typedef struct {
    uint8_t state;    /// 吸力状态 suction_status_e
    uint8_t pump_duty;       /// 泵奶力度
    uint8_t valve_duty;      /// 吸乳器电磁阀门力度
    uint8_t target_pressure;  /// 目标压力
    uint16_t run_time;     /// 运行时间，单位ms
 } motor_component_state_t;

/// 马达策略信息
typedef struct {
    motor_policy_original_para_t original;
    suction_policy_config_t config;
    motor_policy_state_t state;
} motor_policy_info_t;

/// 马达调试参数
typedef struct {
    uint8_t  duty;    /// 占空比
    uint16_t hold_time;    /// 保持时间
} motor_debug_para_t;

/// 马达策略类型枚举
typedef enum {
    MOTOR_TIME_POLICY = 0,
    MOTOR_TIME_SUB_POLICY,
} motor_policy_type_e;

/// 马达策略配置
typedef struct {  
    uint8_t policy_type;       /// 策略类型 motor_policy_type_e
    uint16_t duration;         /// 持续时间，单位s
    motor_policy_core_t core;  /// 策略实体
} motor_policy_cfg_t;

/// 马达多策略核心参数
typedef struct {
    motor_policy_cfg_t *policy_cfg_list;       /// 策略核心参数列表
    uint8_t policy_cfg_count;           /// 策略数量
} motor_multiple_policy_cfg_t;

/// 马达总策略配置
typedef struct {  
    uint8_t time_policy_num; /// 时间策略数量
    uint8_t sub_policy_num; /// 子策略数量
    motor_policy_core_t time_policy[]; 
} motor_policy_total_cfg_t;

/// 吸乳马达策略（常规模式）
typedef struct motor_policy_node {
    motor_policy_core_t core;         /// 当前参数配置
    struct motor_policy_node *next;   /// 下一个子策略
} motor_policy_node_t;

/// 吸乳马达定时策略（追奶模式）
typedef struct motor_timed_policy_node {
    uint8_t sub_policy_count;             /// 子策略参数数量
    uint32_t duration_seconds;            /// 当前定时策略持续时间，单位s
    motor_policy_node_t policy;           /// 策略参数
    struct motor_timed_policy_node *next; /// 下一个定时策略
} motor_timed_policy_node_t;

/// 吸乳马达总策略
typedef struct motor_policy_manager {
    uint8_t timed_policy_count;      /// 定时策略数量
    motor_timed_policy_node_t head;  /// 定时策略实体
} motor_policy_manager_t;

/// 吸乳马达状态
typedef struct {
    uint8_t is_climb_policy; /// 是否启用爬升策略
    uint8_t breast_pump_mode; /// 泵奶模式
    uint8_t current_timed_policy_index; /// 当前定时策略索引
    uint8_t all_timed_policy_finish_flag; /// 所有定时策略执行完毕标志
    uint8_t current_policy_index; /// 当前子策略索引
    motor_policy_manager_t *manager; /// 总策略管理器
    motor_timed_policy_node_t *timed_policy; /// 时间策略
    motor_policy_node_t policy[BREAST_PUMP_NUM]; /// 策略
    motor_policy_info_t info[BREAST_PUMP_NUM];    /// 当前吸力信息，由多个策略综合生成
    uint32_t time_policy_running_time; /// 当前时间策略已运行时间，单位ms
    uint32_t time_policy_remaining_time; /// 时间策略剩余时间，单位s
    uint32_t timed_policy_start_time;  /// 时间策略开始时间，单位ms
    uint32_t manager_start_time; /// 总策略开始时间，单位ms
    taste_setting_t taste_setting; /// 体验模式设置
    uint8_t taste_run_flag; /// 体验模式运行标志
    uint8_t current_mode_type; /// 当前模式类型
    uint8_t next_mode_type; /// 下一个模式类型
} motor_policy_runtime_t;

/// 吸乳策略处理
typedef void (*breast_pump_policy_handler_cb_ptr)(void *user_data);

struct motor_manager_context;
typedef int (*breast_pump_policy_init_func)(void);
typedef const motor_policy_manager_t *(*get_policy_obj_func)(void *user_data);
typedef void (*add_breast_pump_policy_fptr)(list_head_t *head, const char *name, int breast_pump_mode, 
    void *user_data, breast_pump_policy_init_func init_cb, get_policy_obj_func policy_cb);

/// 每种策略节点容器描述.
typedef struct breast_pump_policy_node {
    char *name;
    void *user_data;
    int breast_pump_mode;
    breast_pump_policy_init_func init_cb;
    get_policy_obj_func get_policy_cb;
    list_head_t list;
} breast_pump_policy_node_t;

#ifdef __cplusplus
}
#endif

#endif 
