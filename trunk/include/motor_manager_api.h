#ifndef __MOTOR_MANAGER_API_H__
#define __MOTOR_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h"
#include "motor_policy.h"

/*******************************************************************************
 * Macro definitions
 ******************************************************************************/

// 将震动力度、扫动力度、扫动频率合并为一个值
#define CONVERT_MOTOR_LEVEL(shake_level, sweep_level, sweep_freq) \
      ((shake_level - 1) * 100 + (sweep_level - 1) * 10 + (sweep_freq - 1))

typedef enum {
    NOTIFY_BREATH_PUMP_THEME_GIFT = -1, // 主题礼物播报
    NOTIFY_SWAP_BREATH_PUMP_AREA = 0,   // 换区提醒
    NOTIFY_BREATH_PUMP_OVERPRESSURE,    // 过压提醒
    NOTIFY_BREATH_PUMP_2MIN,            // 已泵奶2min提醒.
    NOTIFY_BREATH_PUMP_EACH_2MIN,       // 每隔 2min 提醒（除了第一个2min）
    NOTIFY_BREATH_PUMP_SLOW_SPEED,    // 减慢速度
    NOTIFY_BREATH_PUMP_2MIN_COME,     // 2MIN时间到
    NOTIFY_BREATH_PUMP_1MIN_COME,     // 1MIN时间到
    NOTIFY_BREATH_PUMP_30S_COME,      // 30s提醒
    NOTIFY_BREATH_PUMP_START,         //开始泵奶
    // 已经达到预定泵奶时间, 2:00 ~ 4:00
    NOTIFY_BREATH_PUMP_DURATION_REACHED_2MIN_0S,
    NOTIFY_BREATH_PUMP_DURATION_REACHED_2MIN_30S,
    NOTIFY_BREATH_PUMP_DURATION_REACHED_3MIN_0S,
    NOTIFY_BREATH_PUMP_DURATION_REACHED_3MIN_30S,
    NOTIFY_BREATH_PUMP_DURATION_REACHED_4MIN_0S,
    NOTIFY_BREATH_PUMP_END
} breath_pump_notify_e;

typedef struct motor_cfg_map {
    motor_cfg_t white[MOTOR_WHITE_MODE_LEN];    // 亮白
    motor_cfg_t soft[MOTOR_SOFT_MODE_LEN];      // 轻柔
    motor_cfg_t clear[MOTOR_CLEAN_MODE_LEN];    // 清洁
    motor_cfg_t smart[MOTOR_SMART_MODE_LEN];    // 智能模式
} motor_cfg_map_t;

/*
马达提醒功能
*/
typedef enum {
    VIBRATE_TYPE_FORCE_STOP,            // 停止.
    VIBRATE_TYPE_SHORT,                 // 短抖动
    VIBRATE_TYPE_LONG_MELODY,           // 长旋律
    VIBRATE_TYPE_SHORT_MELODY,          // 短旋律
    VIBRATE_TYPE_SHORT_WAIT,            // 短停顿
    VIBRATE_TYPE_CONTINUOUS_SHORT_WAIT, // 连续短停顿
    VIBRATE_TYPE_CONTINUOUS_PLAY_XS,    // 自清洁连续播放15s
    VIBRATE_TYPE_BUTTON,                // 按键音效
    VIBRATE_TYPE_MAX
} motor_vibrate_notify_e;

/*
* 马达抖动状态
*/
typedef enum {
    MOTOR_INVALID = 0,
    MOTOR_SHAKE ,
    MOTOR_NORMAL,
}motor_shake_state_e;

/*
马达策略执行动作
*/
typedef enum {
    MOTOR_POLICY_START = 0,             // 初始化启动
    MOTOR_POLICY_CONTINUE = 1,          // 继续运行(同一个模式下)
    MOTOR_POLICY_CHANGE_MODE = 2,       // 切换模式运行
    MOTOR_POLICY_ACTION_MAX
} motor_policy_action_e;

typedef int (*policy_finish_cb_fptr)(void *user_data);

/*
* 泵奶通用接口，泵奶策略
*/ 
typedef struct breath_pump_policy_prop {
    void *user_data;
    uint8_t action; 
    policy_finish_cb_fptr cb;
} breath_pump_policy_prop_t;

/*
* 描述motor管理对象
*/ 
struct audio_manager_context;
struct algo_fusion_context;

/*
* 马达模式策略结构描述.
*/ 
struct motor_policy_para;
struct breath_pump_policy;
struct motor_manager_context;

typedef int (*breath_pump_policy_init_func)(struct motor_manager_context *ctx, struct motor_policy_para *policy_init);
typedef struct breath_pump_policy *(*get_policy_obj_func)(void *user_data);

typedef void (*add_breath_pump_policy_fptr)(list_head_t *head, const char *name, int breath_pump_mode, 
    void *user_data, breath_pump_policy_init_func init_cb, get_policy_obj_func policy_cb);

/* 
* 每种策略节点容器描述.
*/
typedef struct breath_pump_policy_node {
    char *name;
    void *user_data;
    int breath_pump_mode;
    breath_pump_policy_init_func init_cb;
    get_policy_obj_func get_policy_cb;
    list_head_t list;
} breath_pump_policy_node_t;

/* 
* 每种策略类型
*/
typedef enum {
    ID_MASK_PLAY_INVALID = (-1),
    ID_MASK_PLAY_MELODY_VIBRATE = 100, // 铃声振动类
    ID_MASK_PLAY_DUTY_CLIMB = 101,     // duty_climb, 渐强振动
    ID_MASK_PLAY_BREATH_PUMP_POLICY = 102,   // 各种泵奶策略
    ID_MASK_PLAY_SOUND_NOTIC = 103,    // 语音提醒
    ID_MASK_PLAY_SHAKE_VIBRATE = 104,  // 微振模式
    ID_MASK_PLAY_MUSIC = 105,          // 音乐播放
    ID_MASK_PLAY_END,
} motor_play_mask_id_e;

/* 
* 打断原因
*/
typedef enum {
    EXIT_REASON_EXCEPTION = -1,
    EXIT_REASON_NORMAL = 0,
    EXIT_REASON_PLAY_COMPLETED = 1,
    EXIT_REASON_PLAY_BY_USER = 2,
    EXIT_REASON_INTERRUPT = 99,
    EXIT_REASON_GOTO_NEXT_BASE = 100,
    EXIT_REASON_GOTO_BREATH_PUMP_POLICY,
    EXIT_REASON_GOTO_SHAKE_VIBRATE,
    EXIT_REASON_GOTO_MUSIC_POLICY,
} motor_run_exit_reason_e;

struct audio_sound_prop;
struct motor_task_play_para;
struct motor_play_task_node;
typedef int (*motor_task_run_fptr)(struct motor_play_task_node *node, void *user_data, struct motor_task_play_para *play_para);
typedef void *(*push_history_fptr)(struct motor_play_task_node *node, void *user_data, struct motor_task_play_para *play_para);
typedef int (*pop_history_fptr)(struct motor_play_task_node *node, void *user_data);
typedef int (*play_next_fptr)(struct motor_play_task_node *node, void *user_data);

// 马达播放任务节点描述
typedef struct {
    char *name;
    void *context;
    play_next_fptr play_next_cb;        // 播放一下个策略，部分任务有连续任务，比如3s渐强后需要启动普通泵奶任务.微振和正常切换等.
    motor_play_mask_id_e mask_id;       // 任务标记ID
    motor_task_run_fptr run_cb;         // 指向具体的任务执行
    push_history_fptr push_history_cb;  // 某项任务是否需要保存历史
    pop_history_fptr pop_history_cb;
} motor_play_task_obj_t;


// 添加马达播放任务对象.
void add_motor_play_task_obj(list_head_t *head, motor_play_task_obj_t *obj);

#define ADD_NEW_TASK_OBJ(__head, __name, __mask_id, __context, __play_next_cb, __push_history_cb, __pop_history_cb, __run_cb) \
    do { \
        motor_play_task_obj_t __obj = {0}; \
        __obj.name = (__name); \
        __obj.context = (__context); \
        __obj.mask_id = (__mask_id); \
        __obj.run_cb = (__run_cb); \
        __obj.play_next_cb = (__play_next_cb); \
        __obj.push_history_cb = (__push_history_cb); \
        __obj.pop_history_cb = (__pop_history_cb); \
        add_motor_play_task_obj((__head), (&__obj)); \
    } while (0)

#define MOTOR_WRITE_DATA(m, cmd0, cmd1, cmd2) \
    do { \
        uint32_t __wbuf[] = { (uint32_t)cmd0, (uint32_t)cmd1, (uint32_t)cmd2 }; \
        DEV_WRITE(m, __wbuf, sizeof(__wbuf)); \
    } while (0)

typedef struct motor_play_task_node {
    motor_play_task_obj_t obj;              // 每个任务对象实例.
    struct motor_task_play_para *play_para; // 带历史记录任务的历史参数.
    uint8_t breath_pump_mode_changed;
    uint8_t breath_pump_mode;
    uint8_t breath_pump_state_changed;
    uint8_t breath_pump_state;
    uint8_t breath_pump_start_trigger;
    uint8_t breath_pump_update_music;
    list_head_t list;
} motor_play_task_node_t;

// 马达播放请求参数.
typedef struct motor_task_play_para {
    uint32_t timestamp;

    // 播放类型ID.
    int play_mask_id;
    int music_type;
    // 播放参数.
    union {
        // 铃声.
        struct __meldoy_para {
            int meldoy_id;
        } meldoy;
        // 泵奶策略.
        struct __breath_pump_policy_para {
            int action_id;
        } breath_pump_policy;
        // 语音提醒
        struct audio_sound_prop ad_prop;
    } para;
} motor_task_play_para_t;

typedef struct {
    motor_play_mask_id_e mask_id;
    motor_task_play_para_t para;
} motor_history_node_desc_t;

struct storage_context_struct;
typedef struct motor_manager_context {
// private
    struct audio_manager_context *audio_ctx;
    struct breath_pump_context *breath_pump_ctx;
    struct storage_context_struct *st_ctx;
    struct algo_fusion_context *algo;
    // 振动标志
    uint8_t set_shake_flag;
    // 马达状态
    uint8_t motor_state;
    // 模式切换
    uint8_t mode_switch;
    // pwm爬升模式.
    uint8_t is_climb_running;
    // 用户停止
    uint8_t is_user_stoped;
    // 马达模式运行状态
    uint8_t is_bursh_policy_running;
    // 马达旋律运行状态
    uint8_t is_motor_melody_running;
    // 马达微振状态
    uint8_t motor_shake_state;
    // 驱动节点
    rt_device_node_t *motor_dev;
    // 马达参数集合.
    motor_cfg_map_t motor_cfg;
    // 指向当前马达参数.
    motor_cfg_t *curr_motor_cfg;
    // 播放音频通知
    uint8_t is_playing_audio_notify;
    // 监听泵奶模式改变广播.
    broadcast_obj_t m_bdc;
    // 客户端铃声播放完毕回调.
    int (*client_meldoy_play_finish_cb)(int meldoy_id);
    // 马达各种策略链表头指针.
    list_head_t breath_pump_policy_list;
    // 马达策略数据集合
    motor_policy_data_t policy;

    // 马达播放任务集合: 铃声，语音提醒，音乐，各种振动策略等.
    thread_context_t motor_play_thd;            // 播放线程.
    list_head_t motor_play_task_list;           // 任务list
    motor_play_mask_id_e motor_play_mask_id;    // 任务播放ID
    motor_play_task_node_t *history_node;       // 指向历史节点
    char *interrupt_play_reason;                // 中断播放标记原因.
    const uint8_t *p_over_pressure_state;       // 过压状态.

// public
    int (*get_motor_state)(void);
    int (*is_motor_shake)(void);
    int (*start_motor)(motor_policy_action_e action);
    int (*stop_motor)(void);
    int (*set_motor_shake_state)(motor_shake_state_e state);
    void (*breath_pump_notify)(struct motor_manager_context *ctx, breath_pump_notify_e notify_id);    
    // 马达振动提醒实现通用接口.
    int (*play_vibrate_notify)(const char *tag, motor_vibrate_notify_e vib_type);
    int (*play_vibrate_notify_cb)(const char *tag, motor_vibrate_notify_e vib_type, int (*play_finish_cb)(int meldoy_id));
    // 当前是否允许播放.
    int (*is_can_play_now)(motor_play_task_node_t *node, struct motor_manager_context *ctx);
    // 提供给外部组件访问的马达任务分发接口.
    void (*dispatch_play_task)(struct motor_manager_context *ctx, motor_task_play_para_t *play_para);
    // 马达任务运行状态.
    bool (*is_on_motor_task_running)(struct motor_manager_context *ctx);
    // 设置临时马达运行参数, 不会写入配置中
    void (*set_motor_running_para_by_user)(bldc_contorl_type_e type, void *user_data);
    // 设置马达开关状态, 0: 关闭, 1: 打开. 测试用
    void (*set_motor_switch_state_by_user)(int state);
    // 外部钩子函数.
    struct __motor_manager_hook {
        int (*breath_pump_policy_register)(struct motor_manager_context *ctx, add_breath_pump_policy_fptr add_policy_cb);
        // 马达力度转换表索引
        int (*motor_level_convert_table_index)(int level);
        // 获取常规模式马达默认参数
        int (*normal_mode_config_pasrse)(void);
        // 获取常规模式马达默认参数
        int (*get_normal_mode_default_para)(int mode, int mode_idx, int side, motor_para_t *motor);
        // 马达接收广播路由
        int (*on_motor_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        // 马达播放声音完成路由
        void (*play_sound_finish_route)(uint16_t sound_id);
        // 马达参数生成器
        int (*generate_motor_para)(int mode, void *user_data, int data_size);
    } *hook;

} motor_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


