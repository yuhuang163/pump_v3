#ifndef __MOTOR_MANAGER_API_H__
#define __MOTOR_MANAGER_API_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "motor_policy.h" // IWYU pragma: keep

/*
* 马达策略执行动作
*/
typedef enum {
    MOTOR_POLICY_IDLE,
    MOTOR_POLICY_START,             /// 初始化启动
    MOTOR_POLICY_CONTINUE,          /// 继续运行(同一个模式下)
    MOTOR_POLICY_CHANGE_MODE,       /// 切换模式运行
    MOTOR_USER_DEBUG_POLICY_START,  /// 用户调试策略启动
    MOTOR_POLICY_ACTION_MAX
} motor_policy_action_e;

typedef int (*policy_finish_cb_fptr)(void *user_data);

/*
* 马达策略通用接口
*/ 
typedef struct motor_policy_prop {
    void *user_data;
    uint8_t action; 
    policy_finish_cb_fptr cb;
} motor_policy_prop_t;

/* 
* 每种策略类型
*/
typedef enum {
    ID_MASK_PLAY_INVALID = (-1),
    ID_MASK_PLAY_DUTY_CLIMB = 100,     /// duty_climb, 渐强振动
    ID_MASK_PLAY_BREAST_PUMP_POLICY = 101,   /// 各种泵奶策略
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
    EXIT_REASON_GOTO_BREAST_PUMP_POLICY,
} motor_run_exit_reason_e;

/// 马达播放请求参数.
typedef struct motor_task_para {
    uint32_t timestamp;
    /// 播放类型ID.
    int play_mask_id;
    /// 播放参数
    uint8_t action_id;
} motor_task_para_t;

#define MOTOR_WRITE_DATA(m, cmd0, cmd1, cmd2) \
    do { \
        uint32_t __wbuf[] = { (uint32_t)cmd0, (uint32_t)cmd1, (uint32_t)cmd2 }; \
        DEV_WRITE(m, __wbuf, sizeof(__wbuf)); \
    } while (0)

struct rt_device_node;
struct breast_pump_manager_context;
typedef struct motor_manager_context {
/// private
    struct __motor_private {
        struct breast_pump_manager_context *pump_ctx;
        struct rt_device_node *motor_dev;
        broadcast_obj_t m_bdc;
        thread_context_t motor_play_thd;  /// 播放线程.
        list_head_t policy_list; /// 马达各种策略链表头指针.
        motor_policy_runtime_t runtime; /// 马达各种策略状态.
        motor_policy_manager_t *user_debug_policy; /// 用户调试策略.
        list_head_t policy_handler_list; /// 马达各种策略处理.
        char *interrupt_play_reason;  /// 中断播放标记原因.     
        uint8_t motor_state;          /// 马达状态
        uint8_t is_user_stoped;       /// 用户停止
        uint8_t user_debug_status;    /// 用户调试状态
    } priv;

/// public
    int (*get_motor_state)(void);
    int (*start_motor)(motor_policy_action_e action);
    int (*stop_motor)(void);
    int (*synchronous_stop_motor)(void);
    /// 当前是否允许播放.
    int (*is_can_play_now)(void);  
    /// 马达任务运行状态.
    bool (*is_on_motor_task_running)(struct motor_manager_context *ctx);
    /// 获取马达策略索引
    int (*get_runtime_policy_index)(motor_policy_type_e type);
    /// 外部钩子函数.
    struct __motor_manager_hook {
        int (*on_post_init_route)(struct motor_manager_context *ctx);
        /// 马达接收广播路由
        int (*on_motor_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        /// 马达参数生成器
        int (*generate_motor_para)(struct motor_manager_context *ctx, int mode, void *user_data, int data_size);
        /// 策略启动
        void (*motor_policy_prepare)(struct motor_manager_context *thiz, uint8_t action);
        /// 策略处理回调
        int (*mode_policy_handler)(struct motor_manager_context *thiz, motor_task_para_t *play_para);
        /// 马达带参调试处理
        void (*motor_debug_with_user_para)(struct motor_manager_context *ctx, uint8_t id, void *user_data);
         /// 马达体验模式设置处理
         void (*motor_debug_with_taste_setting)(struct motor_manager_context *ctx, void *user_data);
    } *hook;

} motor_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


