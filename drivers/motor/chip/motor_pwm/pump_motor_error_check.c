/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_config.h"
#include "bsp_common.h"
#include "bsp_motor_internal.h"
#include "pump_motor_error_check.h"
#include "hal_gpadc.h"
#include "root_common_config.h"
/*
* DEFINES
*****************************************************************************************
*/
///打印日志
#define IS_PRINT_LOG_ENABLE                  (0)

/// 滑动窗口大小（用于判断堵转）
#ifndef PUMP_ALARM_STATUS_COUNT
#define PUMP_ALARM_STATUS_COUNT             (5U)
#endif

/// 电机错误检测线程轮询周期（ms）
#ifndef PUMP_MOTOR_CHECK_POLLING_PERIOD
#define PUMP_MOTOR_CHECK_POLLING_PERIOD      (4U)
#endif

/// ADC采样超时时间（ms），超过此时间认为采样失败，需要重置
#ifndef PUMP_MOTOR_ADC_TIMEOUT_MS
#define PUMP_MOTOR_ADC_TIMEOUT_MS            (30U)
#endif


typedef struct {
    uint8_t target_times;  /// 目标次数
    uint8_t work_times;   /// 工作次数
    uint16_t last_voltage; /// 最后一次电压
    uint32_t sum_voltage;  /// 总电压
    uint32_t start_time;   /// 采样开始时间（用于超时检测）
} pump_motor_adc_result_t;

/// 每个泵的堵转检测状态（参照旧代码 pump_adc_data_t 结构）
typedef struct {
    uint8_t adc_sample_count;      /// ADC采样计数
    uint8_t adc_alarm_count;       /// ADC报警计数
    uint8_t alarm_status_index;    /// 滑动窗口索引
    uint8_t error_count;           /// 滑动窗口中错误总数
    uint8_t alarm_status[PUMP_ALARM_STATUS_COUNT];  /// 滑动窗口状态数组（循环监控连续五次报警状态）
    uint8_t count_avg_flag;        /// 是否需要统计平均值的标志
    uint8_t last_running_state;   /// 上次运行状态，用于检测状态变化
} pump_motor_stall_detect_t;

typedef struct {
    thread_context_t thd;        /// 线程上下文
    os_message_id pump1_adc_msg_id;    /// 泵1电机ADC消息队列
    os_message_id pump2_adc_msg_id;    /// 泵2电机ADC消息队列
    os_timer_id polling_timer;   /// 轮询定时器
    uint8_t do_polling;           /// 是否正在轮询
    uint8_t pump_is_stall[MOTOR_PWM_ID_MAX];       /// 电机是否堵转
    void (*callback)(uint8_t pump_id, motor_dev_status_e status);    /// 回调函数
} pump_motor_check_context_t;

///堵转检测阈值属性
typedef struct {
    uint8_t  pump_pwm_vol_duty_max;           ///泵PWM占空比最大值
    uint8_t  pump_pwm_vol_duty_min;           ///泵PWM占空比最小值
    uint16_t  pump_pwm_adc_value_threshold;           ///泵PWMADC阈值
    uint16_t  pump_pwm_stop_error_count;           ///泵PWM堵转错误次数阈值
    uint16_t  pump_pwm_alarm_error_ratio;           ///泵PWM报警错误比例
} pump_motor_stall_detect_threshold_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_motor_check_context_t s_pump_motor_check_ctx = {0};
static pump_motor_adc_result_t s_pump1_motor_adc_result = {0};
static pump_motor_adc_result_t s_pump2_motor_adc_result = {0};
static pump_motor_stall_detect_t s_pump_stall_detect[MOTOR_PWM_ID_MAX] = {0};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
///堵转检测阈值表
pump_motor_stall_detect_threshold_t s_pump_pwm_threshold[10] = {
    {
       10, 0, 250, 3, 3,
    },
    {
       20, 11, 280, 3, 3,
    },
    {
       30, 21, 400, 3, 3,
    },
    {
       40, 31, 500, 3, 3,
    },
    {
       50, 41, 600, 3, 3,
    },
    {
       60, 51, 660, 3, 3,
    },
    {
       70, 61, 730, 3, 3,
    },
    {
       80, 71, 760, 3, 3,
    },
    {
       90, 81, 820, 3, 3,
    },
    {
       100, 91, 860, 3, 3,
    }
};
/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/// 根据占空比查找对应的阈值配置
static const pump_motor_stall_detect_threshold_t* pump_motor_get_threshold_by_duty(uint8_t duty)
{
    /// 遍历阈值表，找到匹配的占空比范围
    for (int i = 0; i < 10; i++) {
        if (duty >= s_pump_pwm_threshold[i].pump_pwm_vol_duty_min && 
            duty <= s_pump_pwm_threshold[i].pump_pwm_vol_duty_max) {
            return &s_pump_pwm_threshold[i];
        }
    }
    /// 如果找不到匹配的，返回第一个作为默认值
    return &s_pump_pwm_threshold[0];
}

/// 参照旧代码 pump_adc_sample_fun 实现的堵转检测函数
static int pump_motor_error_check_handler_internal(motor_pwm_control_id_e pump_id, uint16_t adc_value)
{
    if (pump_id >= MOTOR_PWM_ID_MAX) {
        return 0;
    }
    
    /// 获取当前泵的目标占空比
    int target_duty = bsp_motor_get_pump_target_duty(pump_id);
    if (target_duty < 0) {
        target_duty = 100;  /// 如果获取失败，使用默认值100
    }
    
    /// 根据占空比动态获取阈值配置
    const pump_motor_stall_detect_threshold_t *threshold = pump_motor_get_threshold_by_duty((uint8_t)target_duty);
    
    /// 提取动态阈值参数
    uint16_t adc_threshold = threshold->pump_pwm_adc_value_threshold;
    uint16_t stop_error_count = threshold->pump_pwm_stop_error_count;
    uint16_t alarm_error_ratio = threshold->pump_pwm_alarm_error_ratio;
    
    pump_motor_stall_detect_t *detect = &s_pump_stall_detect[pump_id];
    uint8_t is_running = (bsp_motor_is_running(&pump_id) == 1);
    
    /// 检测运行状态变化：从运行变为停止时，设置 count_avg_flag
    if (detect->last_running_state == 1 && is_running == 0) {
        /// 从运行变为停止，设置统计标志
        detect->count_avg_flag = 1;
    }
    detect->last_running_state = is_running;
    
    /// 如果电机正在运行
    if (is_running) {
        /// 采样ADC值并计数
        detect->adc_sample_count++;
#if IS_PRINT_LOG_ENABLE
        LOG_PBUS("pump[%d] adc_value: %d, target_duty: %d, adc_threshold: %d", 
            pump_id, adc_value, target_duty, adc_threshold);
#endif
        /// 如果ADC值超过动态阈值，增加报警计数
        if (adc_value >= adc_threshold) {
            detect->adc_alarm_count++;
        }
    } 
    /// 如果电机停止运行
    else {
        /// 如果 count_avg_flag 为真且采样计数不为0，进行统计判断
        if (detect->count_avg_flag && detect->adc_sample_count != 0) {
            /// 如果报警次数超过采样次数的比例阈值，认为本次周期有错误
            if (detect->adc_alarm_count > detect->adc_sample_count / alarm_error_ratio) {
                detect->alarm_status[detect->alarm_status_index] = 1;  /// PUMP_ERROR
                LOG_PBUS("pump[%d] error: sample_count:%d alarm_count:%d, threshold_duty:%d", 
                    pump_id, detect->adc_sample_count, detect->adc_alarm_count, target_duty);
            } else {
                detect->alarm_status[detect->alarm_status_index] = 0;  /// PUMP_NORMAL
            }
            
            /// 重置统计标志和计数
            detect->count_avg_flag = 0;
            detect->adc_sample_count = 0;
            detect->adc_alarm_count = 0;
            detect->error_count = 0;
            
            /// 计算滑动窗口中错误总数（循环监控连续五次报警状态）
            for (int i = 0; i < PUMP_ALARM_STATUS_COUNT; i++) {
                detect->error_count += detect->alarm_status[i];
            }
            
            /// 如果滑动窗口中错误次数达到动态阈值，判定为堵转并触发回调
            if (detect->error_count >= stop_error_count) {
                LOG_PBUS("pump[%d] will be stopped, error_count:%d, threshold_duty:%d, stop_error_count:%d", 
                    pump_id, detect->error_count, target_duty, stop_error_count);
                s_pump_motor_check_ctx.pump_is_stall[pump_id] = 1;
                if (s_pump_motor_check_ctx.callback) {
                    s_pump_motor_check_ctx.callback(pump_id, MOTOR_STATUS_STALL);
                }
            } else {
                /// 如果错误次数减少，清除堵转状态

            }
            
            /// 更新滑动窗口索引
            detect->alarm_status_index++;
            if (detect->alarm_status_index >= PUMP_ALARM_STATUS_COUNT) {
                detect->alarm_status_index = 0;
            }
        }
    }
    
    return s_pump_motor_check_ctx.pump_is_stall[pump_id];
}

static void pump1_motor_error_check_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    s_pump1_motor_adc_result.work_times++;
    s_pump1_motor_adc_result.sum_voltage += volt;
    if (s_pump1_motor_adc_result.work_times >= s_pump1_motor_adc_result.target_times) {
        hal_gpadc_close(PUMP1_ADC_CHANNEL);
#if IS_PRINT_LOG_ENABLE
        LOG_PBUS("pump[%d] adc_value: %d", MOTOR_PWM_PUMP_1, volt);
#endif
        s_pump1_motor_adc_result.last_voltage = s_pump1_motor_adc_result.work_times > 0 ? \
            s_pump1_motor_adc_result.sum_voltage / s_pump1_motor_adc_result.work_times : 0;
        MSG_QUEUE_SEND(s_pump_motor_check_ctx.pump1_adc_msg_id, &volt, 0);
    }
}

static int pump1_motor_error_check_adc_open(void)
{
    /// 检查上一次采样是否完成
    if (s_pump1_motor_adc_result.work_times < s_pump1_motor_adc_result.target_times) {
        /// 检查是否超时（采样时间超过阈值）
        uint32_t current_time = get_timestamp();
        uint32_t elapsed_time = current_time - s_pump1_motor_adc_result.start_time;
        
        if (elapsed_time > PUMP_MOTOR_ADC_TIMEOUT_MS) {
            /// 超时重置：关闭ADC，重置状态
            LOGW_TAG("PUMP_ADC", "pump1 motor adc timeout, reset. work_times:%d, target_times:%d, elapsed:%dms", 
                s_pump1_motor_adc_result.work_times, s_pump1_motor_adc_result.target_times, elapsed_time);
            hal_gpadc_close(PUMP1_ADC_CHANNEL);
            s_pump1_motor_adc_result.work_times = 0;
            s_pump1_motor_adc_result.sum_voltage = 0;
            s_pump1_motor_adc_result.target_times = 0;
        } else {
            /// 采样进行中，等待完成（不打印日志，避免刷屏）
            return BSP_OK;
        }
    }
    s_pump1_motor_adc_result.work_times   = 0;
    s_pump1_motor_adc_result.sum_voltage  = 0;
    s_pump1_motor_adc_result.start_time   = get_timestamp();  /// 记录采样开始时间
#if IS_ADC_FILTER_ENABLE
    s_pump1_motor_adc_result.target_times = ADC_FILTER_TIMES;
    int ret = hal_gpadc_open(PUMP1_ADC_CHANNEL, HAL_GPADC_ATP_125US, pump1_motor_error_check_handler);
#else
    s_pump1_motor_adc_result.target_times = 1;
    int ret = hal_gpadc_open(PUMP1_ADC_CHANNEL, HAL_GPADC_ATP_ONESHOT, pump1_motor_error_check_handler);
#endif
    if (ret != 0) {
        LOGE("gpadc open failed: %d", ret);
        return ret;
    }
    return BSP_OK;
}

static void pump2_motor_error_check_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    s_pump2_motor_adc_result.work_times++;
    s_pump2_motor_adc_result.sum_voltage += volt;
    if (s_pump2_motor_adc_result.work_times >= s_pump2_motor_adc_result.target_times) {
        hal_gpadc_close(PUMP2_ADC_CHANNEL);
#if IS_PRINT_LOG_ENABLE
        LOG_PBUS("pump[%d] adc_value: %d", MOTOR_PWM_PUMP_2, volt);
#endif
        s_pump2_motor_adc_result.last_voltage = s_pump2_motor_adc_result.work_times > 0 ? \
            s_pump2_motor_adc_result.sum_voltage / s_pump2_motor_adc_result.work_times : 0;
        MSG_QUEUE_SEND(s_pump_motor_check_ctx.pump2_adc_msg_id, &volt, 0);
    }
}

static int pump2_motor_error_check_adc_open(void)
{
    /// 检查上一次采样是否完成
    if (s_pump2_motor_adc_result.work_times < s_pump2_motor_adc_result.target_times) {
        /// 检查是否超时（采样时间超过阈值）
        uint32_t current_time = get_timestamp();
        uint32_t elapsed_time = current_time - s_pump2_motor_adc_result.start_time;
        
        if (elapsed_time > PUMP_MOTOR_ADC_TIMEOUT_MS) {
            /// 超时重置：关闭ADC，重置状态
            LOGW_TAG("PUMP_ADC", "pump2 motor adc timeout, reset. work_times:%d, target_times:%d, elapsed:%dms", 
                s_pump2_motor_adc_result.work_times, s_pump2_motor_adc_result.target_times, elapsed_time);
            hal_gpadc_close(PUMP2_ADC_CHANNEL);
            s_pump2_motor_adc_result.work_times = 0;
            s_pump2_motor_adc_result.sum_voltage = 0;
            s_pump2_motor_adc_result.target_times = 0;
        } else {
            /// 采样进行中，等待完成（不打印日志，避免刷屏）
            return BSP_OK;
        }
    }
    s_pump2_motor_adc_result.work_times   = 0;
    s_pump2_motor_adc_result.sum_voltage  = 0;
    s_pump2_motor_adc_result.start_time   = get_timestamp();  /// 记录采样开始时间
#if IS_ADC_FILTER_ENABLE
    s_pump2_motor_adc_result.target_times = ADC_FILTER_TIMES;
    int ret = hal_gpadc_open(PUMP2_ADC_CHANNEL, HAL_GPADC_ATP_125US, pump2_motor_error_check_handler);
#else
    s_pump2_motor_adc_result.target_times = 1;
    int ret = hal_gpadc_open(PUMP2_ADC_CHANNEL, HAL_GPADC_ATP_ONESHOT, pump2_motor_error_check_handler);
#endif
    if (ret != 0) {
        LOGE("gpadc open failed: %d", ret);
        return ret;
    }
    return BSP_OK;
}


/// 定时器回调函数，发送消息到线程触发采样
static void pump_motor_check_polling_timer_handler(void *handle)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(s_pump_motor_check_ctx.thd.msg_id, &msg, 0);
}

/// 线程主函数，周期性执行ADC采样和堵转检测
static void pump_motor_check_dispatch_thread(const void *args)
{
    uint8_t msg = 0;
    uint16_t adc_value1 = 0;
    uint16_t adc_value2 = 0;
    uint16_t log_time = 0;
    uint32_t start_time = get_timestamp();
    thread_context_t *thd = (thread_context_t *)args;
    pump_motor_check_context_t *ctx = &s_pump_motor_check_ctx;
    
    /// 先创建消息队列，确保定时器回调可以安全发送消息
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, PUMP_MOTOR_CHECK_MESQ_NUM, 1);
    
    /// 确保消息队列创建成功后再启动定时器
    if (thd->msg_id == NULL) {
        LOGE("Failed to create main message queue");
        return;
    }
    
    CREATE_TIMER(&ctx->polling_timer, "pump_motor_check_timer", ctx, OS_TIMER_PERIODIC, 
        pump_motor_check_polling_timer_handler);
    ///START_TIMER(ctx->polling_timer, PUMP_MOTOR_CHECK_POLLING_PERIOD);
    ///ctx->do_polling = 1;
    
    for (;;) {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        
        if (!ctx->do_polling) {
            continue;
        }
        
        /// 执行泵1电机ADC采样和堵转检测
        if (pump1_motor_error_check_adc_open() == BSP_OK) {
            /// 等待ADC采样完成（从消息队列接收采样值）
            adc_value1 = 0;  /// 清零，避免上次采样值影响
            if (MSG_QUEUE_RECV(ctx->pump1_adc_msg_id, &adc_value1, 100) == 0) {
                adc_value1 = s_pump1_motor_adc_result.last_voltage > 0 ? s_pump1_motor_adc_result.last_voltage : 0;
#if IS_PRINT_LOG_ENABLE
                LOG_PBUS("pump[%d] adc_value: %d", MOTOR_PWM_PUMP_1, adc_value1);
#endif
                ctx->pump_is_stall[MOTOR_PWM_PUMP_1] = pump_motor_error_check_handler_internal(MOTOR_PWM_PUMP_1, adc_value1);
            }
        }
        
        /// 执行泵2电机ADC采样和堵转检测
        if (pump2_motor_error_check_adc_open() == BSP_OK) {
            /// 等待ADC采样完成（从消息队列接收采样值）
            adc_value2 = 0;  /// 清零，避免上次采样值影响
            if (MSG_QUEUE_RECV(ctx->pump2_adc_msg_id, &adc_value2, 100) == 0) {
                adc_value2 = s_pump2_motor_adc_result.last_voltage > 0 ? s_pump2_motor_adc_result.last_voltage : 0;
#if IS_PRINT_LOG_ENABLE
                LOG_PBUS("pump[%d] adc_value: %d", MOTOR_PWM_PUMP_2, adc_value2);
#endif
                ctx->pump_is_stall[MOTOR_PWM_PUMP_2] = pump_motor_error_check_handler_internal(MOTOR_PWM_PUMP_2, adc_value2);
            }
        }
        
        log_time++;
        if (log_time >= 500) {  /// 每500次采样打印一次日志（约5秒，500 * 10ms = 5000ms）
            LOG_PBUS("pump motor check polling: %d, %d", ctx->pump_is_stall[MOTOR_PWM_PUMP_1], ctx->pump_is_stall[MOTOR_PWM_PUMP_2]);
            log_time = 0;
        }
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

int pump_motor_error_check_init(void)
{
    pump_motor_check_context_t *ctx = &s_pump_motor_check_ctx;
    
    CREATE_MSG_TYPE_QUEUE(&ctx->pump1_adc_msg_id, 1, 1);
    CREATE_MSG_TYPE_QUEUE(&ctx->pump2_adc_msg_id, 1, 1);
    /// 启动线程
    RUN_NEW_THREAD(&ctx->thd, "pump_motor_check_thd", pump_motor_check_dispatch_thread,
         RT_PRIO_PUMP_MOTOR_CHECK_THREAD, PUMP_MOTOR_CHECK_THREAD_STACKSIZE);
    
    LOG_PBUS("pump motor error check init ok");
    return 0;
}

int pump_motor_error_check_stop(void)
{
    pump_motor_check_context_t *ctx = &s_pump_motor_check_ctx;
    
    if (ctx->polling_timer == NULL) {
        LOGE("pump motor error check timer is not initialized");
        return -1;
    }
    if (ctx->do_polling) {
        /// 停止定时器
        STOP_TIMER(ctx->polling_timer);
        ctx->do_polling = 0;
        LOG_PBUS("stop timer");
    }
    
    /// 清零统计结果（先设置 do_polling = 0 确保线程不再处理新数据）
    s_pump1_motor_adc_result.work_times = 0;
    s_pump1_motor_adc_result.sum_voltage = 0;
    s_pump2_motor_adc_result.work_times = 0;
    s_pump2_motor_adc_result.sum_voltage = 0;
    
    /// 清零所有泵的检测状态（合并循环，提高效率）
    for (int i = 0; i < MOTOR_PWM_ID_MAX; i++) {
        s_pump_stall_detect[i].adc_sample_count = 0;
        s_pump_stall_detect[i].adc_alarm_count = 0;
        s_pump_stall_detect[i].alarm_status_index = 0;
        s_pump_stall_detect[i].error_count = 0;
        s_pump_stall_detect[i].count_avg_flag = 0;
        s_pump_stall_detect[i].last_running_state = 0;
        /// 清零滑动窗口状态数组
        memset(s_pump_stall_detect[i].alarm_status, 0, sizeof(s_pump_stall_detect[i].alarm_status));
    }
    
    /// 清零堵转状态
    memset(ctx->pump_is_stall, 0, sizeof(ctx->pump_is_stall));
    
    LOG_PBUS("pump motor error check stop ok");
    return 0;
}

int pump_motor_error_check_start(void)
{
    pump_motor_check_context_t *ctx = &s_pump_motor_check_ctx;
    if (ctx->polling_timer == NULL) {
        LOGE("pump motor error check timer is not initialized");
        return -1;
    }
    if (!ctx->do_polling) {
        STOP_TIMER(ctx->polling_timer);
        START_TIMER(ctx->polling_timer, PUMP_MOTOR_CHECK_POLLING_PERIOD);
        ctx->do_polling = 1;
        LOG_PBUS("start check timer");
        return 0;
    }
    return -1; /// 如果定时器未初始化或正在运行，返回错误
}

int pump_motor_error_check_is_running(void)
{
    return s_pump_motor_check_ctx.do_polling ? 1 : 0;
}

int pump_motor_error_check_register_callback(void (*callback)(uint8_t pump_id, motor_dev_status_e status))
{
    s_pump_motor_check_ctx.callback = callback;
    return 0;
}