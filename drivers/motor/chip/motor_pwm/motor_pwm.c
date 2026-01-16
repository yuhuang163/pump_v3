/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_motor.h"
#include "hal_pwm.h"
#include "hal_timer.h"
#include "bsp_common.h" 
#include "bsp_config.h"
#include "bsp_motor_internal.h"
#include "motor_pwm.h"
#include "hal_gpio.h"
#include "hal_sysfreq.h"
#include <stdint.h>

// 前向声明，确保编译器识别函数
int hal_pwm_update_duty_only(enum HAL_PWM_ID_T id, uint8_t duty, uint32_t freq);
/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_RUN_TIME_TEST          (0)
#define IS_SUPPORT_PUMP_DEBUG_LOG              (0)

#define IS_SUPPORT_PUMP_PWM_IDLE_HIGH          (1)
#define IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE     (1)
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
#define PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY      (90) /// 初始占空比
#else
#define PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY      (10) /// 初始占空比
#endif
#define PUMP_PWM_GRADUAL_CHANGE_PERIOD         (2) /// 变化周期 2ms
#define PUMP_PWM_GRADUAL_CHANGE_TIME_SCALE     (1) /// 变化时间比例系数：change_time = duty_diff * SCALE (ms)

typedef struct {
    enum HAL_PWM_ID_T id;
    void (*iomux_set)(void);
} motor_pwm_cfg_t;

#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
static void bsp_motor_pump_1_timer_timeout_handler(void *param);
static void bsp_motor_pump_2_timer_timeout_handler(void *param);
void bsp_motor_pwm_test_all_step_counts(void);
#endif
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const motor_pwm_cfg_t motor_pwm_cfg[] = {
    {MOTOR_PUMP1_PWM_ID,   MOTOR_PUMP1_PWM_IMUX},
    {MOTOR_PUMP2_PWM_ID,   MOTOR_PUMP2_PWM_IMUX},
    {MOTOR_VALVE1_PWM_ID,  MOTOR_VALVE1_PWM_IMUX},
    {MOTOR_VALVE2_PWM_ID,  MOTOR_VALVE2_PWM_IMUX},
};

static uint8_t motor_pwm_status[MOTOR_PWM_ID_MAX];
#if IS_SUPPORT_PUMP_RUN_TIME_TEST
static uint8_t count = 0;
static uint32_t start_time = 0;
static uint32_t run_time[20] = {0};
#endif

#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
static motor_pwm_timer_cfg_t timer_info[MOTOR_PWM_PUMP_2 + 1] = {
    [MOTOR_PWM_PUMP_1] =
        {
            .time_id = NULL,
            .time_handler = bsp_motor_pump_1_timer_timeout_handler,
            .time_period = PUMP_PWM_GRADUAL_CHANGE_PERIOD, //2ms
            .pwm_cfg =
                {
                    .freq = 0,
                    .ratio = 0,
                    .inv = false,
                    .sleep_on = false,
                },
            .init_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY,
            .target_duty = 0,
            .duty_step = 0,
            .current_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY,
            .gpio_irq_update = 0,
        },
    [MOTOR_PWM_PUMP_2] = {
        .time_id = NULL,
        .time_handler = bsp_motor_pump_2_timer_timeout_handler,
        .time_period = PUMP_PWM_GRADUAL_CHANGE_PERIOD, //2ms
        .pwm_cfg =
            {
                .freq = 0,
                .ratio = 0,
                .inv = false,
                .sleep_on = false,
            },
        .init_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY,
        .target_duty = 0,
        .duty_step = 0,
        .current_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY,
        .gpio_irq_update = 0,
    }};
#endif
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
/**
 * @brief PWM步进更新函数
 * @param pwm_id PWM ID
 */
static void bsp_motor_pwm_step_update_pump1(motor_pwm_control_id_e pwm_id)
{
    if (pwm_id != MOTOR_PWM_PUMP_1) {
        //LOGE("pwm_id is not MOTOR_PWM_PUMP_1");
        return;
    }
    
    // 保存当前占空比（用于日志）
    uint8_t old_duty = timer_info[pwm_id].current_duty;
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 计算新的占空比（从90向下渐变到目标值）
    uint8_t new_duty = timer_info[pwm_id].current_duty - timer_info[pwm_id].duty_step;
    // 防止低于目标值（向下渐变时，new_duty应该 >= target_duty）
    if (new_duty < timer_info[pwm_id].target_duty) {
        new_duty = timer_info[pwm_id].target_duty;
    }
#else
    // 计算新的占空比（从10向上渐变到目标值）
    uint8_t new_duty = timer_info[pwm_id].current_duty + timer_info[pwm_id].duty_step;
    // 防止超过目标值（向上渐变时，new_duty应该 <= target_duty）
    if (new_duty > timer_info[pwm_id].target_duty) {
        new_duty = timer_info[pwm_id].target_duty;
    }
#endif
    
    // 更新PWM配置
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = timer_info[pwm_id].pwm_cfg.freq,
        .ratio = new_duty,
        .inv = timer_info[pwm_id].pwm_cfg.inv,
        .sleep_on = timer_info[pwm_id].pwm_cfg.sleep_on,
    };
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 向下渐变：new_duty应该 >= target_duty
    if ((timer_info[pwm_id].current_duty != new_duty) && (new_duty >= timer_info[pwm_id].target_duty)) {
        timer_info[pwm_id].gpio_irq_update = 1;
    }
#else
    // 向上渐变：new_duty应该 <= target_duty
    if ((timer_info[pwm_id].current_duty != new_duty) && (new_duty <= timer_info[pwm_id].target_duty)) {
        timer_info[pwm_id].gpio_irq_update = 1;
    }
#endif
    // 更新当前占空比
    timer_info[pwm_id].current_duty = new_duty;
    // 如果还没达到目标占空比，且定时器应该运行，则重新启动定时器
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 向下渐变：继续条件 new_duty > target_duty
    if (new_duty > timer_info[pwm_id].target_duty && timer_info[pwm_id].time_status == 1) {
#else
    // 向上渐变：继续条件 new_duty < target_duty
    if (new_duty < timer_info[pwm_id].target_duty && timer_info[pwm_id].time_status == 1) {
#endif
        if (timer_info[pwm_id].time_id != NULL) {
            enum E_HWTIMER_T ret = hwtimer_start(timer_info[pwm_id].time_id, MS_TO_HWTICKS(timer_info[pwm_id].time_period));
            if (ret != E_HWTIMER_OK) {
                //LOGE("Failed to restart timer in callback: pwm_id=%d, ret=%d", pwm_id, ret);
                // 如果启动失败，清除运行标志，避免无限重试
                timer_info[pwm_id].time_status = 0;
            }
        }
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    } else if (new_duty <= timer_info[pwm_id].target_duty) {
        // 已达到目标占空比（向下渐变），清除运行标志
        timer_info[pwm_id].time_status = 0;
#else
    } else if (new_duty >= timer_info[pwm_id].target_duty) {
        // 已达到目标占空比（向上渐变），清除运行标志
        timer_info[pwm_id].time_status = 0;
#endif
        //LOG_PBUS("Reached target_duty: pwm_id=%d, new_duty=%d, target_duty=%d", pwm_id, new_duty, timer_info[pwm_id].target_duty);
    }
}

static void bsp_motor_pwm_step_update_pump2(motor_pwm_control_id_e pwm_id)
{
    if (pwm_id != MOTOR_PWM_PUMP_2) {
        //LOGE("pwm_id is not MOTOR_PWM_PUMP_2");
        return;
    }
    
    // 保存当前占空比（用于日志）
    uint8_t old_duty = timer_info[pwm_id].current_duty;
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 计算新的占空比（从90向下渐变到目标值）
    uint8_t new_duty = timer_info[pwm_id].current_duty - timer_info[pwm_id].duty_step;
    // 防止低于目标值（向下渐变时，new_duty应该 >= target_duty）
    if (new_duty < timer_info[pwm_id].target_duty) {
        new_duty = timer_info[pwm_id].target_duty;
    }
#else
    // 计算新的占空比（从10向上渐变到目标值）
    uint8_t new_duty = timer_info[pwm_id].current_duty + timer_info[pwm_id].duty_step;
    // 防止超过目标值（向上渐变时，new_duty应该 <= target_duty）
    if (new_duty > timer_info[pwm_id].target_duty) {
        new_duty = timer_info[pwm_id].target_duty;
    }
#endif
    
    // 更新PWM配置
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = timer_info[pwm_id].pwm_cfg.freq,
        .ratio = new_duty,
        .inv = timer_info[pwm_id].pwm_cfg.inv,
        .sleep_on = timer_info[pwm_id].pwm_cfg.sleep_on,
    };
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 向下渐变：new_duty应该 >= target_duty
    if ((timer_info[pwm_id].current_duty != new_duty) && (new_duty >= timer_info[pwm_id].target_duty)) {
        timer_info[pwm_id].gpio_irq_update = 1;
    }
#else
    // 向上渐变：new_duty应该 <= target_duty
    if ((timer_info[pwm_id].current_duty != new_duty) && (new_duty <= timer_info[pwm_id].target_duty)) {
        timer_info[pwm_id].gpio_irq_update = 1;
    }
#endif

    // 更新当前占空比
    timer_info[pwm_id].current_duty = new_duty;
    // 如果还没达到目标占空比，且定时器应该运行，则重新启动定时器
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 向下渐变：继续条件 new_duty > target_duty
    if (new_duty > timer_info[pwm_id].target_duty && timer_info[pwm_id].time_status == 1) {
#else
    // 向上渐变：继续条件 new_duty < target_duty
    if (new_duty < timer_info[pwm_id].target_duty && timer_info[pwm_id].time_status == 1) {
#endif
        if (timer_info[pwm_id].time_id != NULL) {
            enum E_HWTIMER_T ret = hwtimer_start(timer_info[pwm_id].time_id, MS_TO_HWTICKS(timer_info[pwm_id].time_period));
            if (ret != E_HWTIMER_OK) {
                //LOGE("Failed to restart timer in callback: pwm_id=%d, ret=%d", pwm_id, ret);
                // 如果启动失败，清除运行标志，避免无限重试
                timer_info[pwm_id].time_status = 0;
            }
        }
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    } else if (new_duty <= timer_info[pwm_id].target_duty) {
        // 已达到目标占空比（向下渐变），清除运行标志
        timer_info[pwm_id].time_status = 0;
#else
    } else if (new_duty >= timer_info[pwm_id].target_duty) {
        // 已达到目标占空比（向上渐变），清除运行标志
        timer_info[pwm_id].time_status = 0;
#endif
        //LOG_PBUS("Reached target_duty: pwm_id=%d, new_duty=%d, target_duty=%d", pwm_id, new_duty, timer_info[pwm_id].target_duty);
    }
}

static void bsp_motor_pump_1_timer_timeout_handler(void *param)
{
    bsp_motor_pwm_step_update_pump1(MOTOR_PWM_PUMP_1);
}

static void bsp_motor_pump_2_timer_timeout_handler(void *param)
{
    bsp_motor_pwm_step_update_pump2(MOTOR_PWM_PUMP_2);
}

static void bsp_motor_start_periodic_task(motor_pwm_control_id_e pwm_id)
{
    if (timer_info[MOTOR_PWM_PUMP_1].time_id == NULL ||
        timer_info[MOTOR_PWM_PUMP_2].time_id == NULL) {
        LOGE("Failed to start periodic task");
        return;
    }
    LOG_PBUS("Start periodic task: %d", pwm_id);
    // 将毫秒转换为tick
    uint32_t ticks = MS_TO_HWTICKS(timer_info[pwm_id].time_period);
    
    // 如果定时器正在运行，先停止它
    if (hwtimer_active(timer_info[pwm_id].time_id)) {
        enum E_HWTIMER_T ret = hwtimer_stop(timer_info[pwm_id].time_id);
        if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
            LOGE("Failed to stop timer before start: pwm_id=%d, ret=%d", pwm_id, ret);
        }
        // 如果定时器正在回调中，等待回调完成（通过检查状态）
        // 注意：E_HWTIMER_IN_CALLBACK 表示定时器正在回调中，这是正常的
    }
    // 启动定时器
    enum E_HWTIMER_T ret = hwtimer_start(timer_info[pwm_id].time_id, ticks);
    if (ret != E_HWTIMER_OK) {
        LOGE("Failed to start timer: pwm_id=%d, ret=%d", pwm_id, ret);
        timer_info[pwm_id].time_status = 0;
        return;
    }
    timer_info[pwm_id].time_status = 1;
}

static void bsp_motor_stop_periodic_task(motor_pwm_control_id_e pwm_id)
{
    if (timer_info[pwm_id].time_id != NULL && timer_info[pwm_id].time_status == 1) {
        LOG_PBUS("Stop periodic task: %d", pwm_id);
        // 先清除运行标志，防止回调中重新启动
        timer_info[pwm_id].time_status = 0;
        enum E_HWTIMER_T ret = hwtimer_stop(timer_info[pwm_id].time_id);
        if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
            LOGE("Failed to stop timer: pwm_id=%d, ret=%d", pwm_id, ret);
        }
        // 注意：E_HWTIMER_IN_CALLBACK 表示定时器正在回调中，这是正常的
        // 由于我们已经清除了 time_status，回调中不会重新启动定时器
    }
}

static int8_t bsp_motor_init_timer_system(void)
{
    // 分配定时器
    timer_info[MOTOR_PWM_PUMP_1].time_id = hwtimer_alloc(bsp_motor_pump_1_timer_timeout_handler, NULL);
    if (timer_info[MOTOR_PWM_PUMP_1].time_id == NULL) {
        LOGE("Failed to allocate timer");
        return -1;
    }
    timer_info[MOTOR_PWM_PUMP_2].time_id = hwtimer_alloc(bsp_motor_pump_2_timer_timeout_handler, NULL);
    if (timer_info[MOTOR_PWM_PUMP_2].time_id == NULL) {
        LOGE("Failed to allocate timer");
        return -1;
    }
    return 0;
}

static void bsp_motor_deinit_timer_system(void)
{
    bsp_motor_stop_periodic_task(MOTOR_PWM_PUMP_1);
    bsp_motor_stop_periodic_task(MOTOR_PWM_PUMP_2);
    if (timer_info[MOTOR_PWM_PUMP_1].time_id != NULL) {
        hwtimer_free(timer_info[MOTOR_PWM_PUMP_1].time_id);
        timer_info[MOTOR_PWM_PUMP_1].time_id = NULL;
    }
    if (timer_info[MOTOR_PWM_PUMP_2].time_id != NULL) {
        hwtimer_free(timer_info[MOTOR_PWM_PUMP_2].time_id);
        timer_info[MOTOR_PWM_PUMP_2].time_id = NULL;
    }
}

static int8_t bsp_motor_pwm_gradual_change_start(motor_para_t *motor_para, motor_pwm_control_id_e pwm_id)
{
    if (pwm_id >= MOTOR_PWM_ID_MAX) {
        LOGE("pwm_id is out of range: %d", pwm_id);
        return -1;
    }
    //不需要渐变，设置后直接退出函数，并返回错误码
    if (pwm_id == MOTOR_PWM_VALVE_1 || pwm_id == MOTOR_PWM_VALVE_2) {
        motor_para_t *motor = (motor_para_t *)motor_para;
        struct HAL_PWM_CFG_T pwm_cfg = {
            .freq = motor->un.pwm.freq,
            .ratio = motor->un.pwm.duty,
            .inv = false,
            .sleep_on = false,
        };
        return hal_pwm_enable(motor_pwm_cfg[pwm_id].id, &pwm_cfg);
    } else { ///需要渐变，设置渐变参数
        //先以初始占空比启动PWM，然后根据目标占空比和变化时间计算出步长，
        // 然后启动定时器，定时器到期后，将占空比增加步长，直到达到目标占空比
        motor_para_t *motor = (motor_para_t *)motor_para;
        struct HAL_PWM_CFG_T pwm_cfg = {
            .freq = motor->un.pwm.freq,
            .ratio = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY,
            .inv = false,
            .sleep_on = false,
        };
        int ret = hal_pwm_enable(motor_pwm_cfg[pwm_id].id, &pwm_cfg);
        if (ret != 0) {
            LOGE("Failed to enable PWM: %d", pwm_id);
            return ret;
        }

        timer_info[pwm_id].pwm_cfg.freq = motor_para->un.pwm.freq;
        timer_info[pwm_id].pwm_cfg.ratio = motor_para->un.pwm.duty;
        timer_info[pwm_id].pwm_cfg.inv = false;
        timer_info[pwm_id].pwm_cfg.sleep_on = false;
        timer_info[pwm_id].init_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY;
        timer_info[pwm_id].target_duty = motor_para->un.pwm.duty;
        timer_info[pwm_id].current_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY; // 初始化当前占空比
        timer_info[pwm_id].duty_step = 0; // 初始化步长
        timer_info[pwm_id].time_period = PUMP_PWM_GRADUAL_CHANGE_PERIOD; // 初始化定时器周期
        timer_info[pwm_id].gpio_irq_update = 0;
        
        // 计算步长和定时器周期
        // 先计算需要变化的占空比差值
        #if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
        int16_t duty_diff = (int16_t)timer_info[pwm_id].init_duty - (int16_t)timer_info[pwm_id].target_duty;
        #else
        int16_t duty_diff = (int16_t)timer_info[pwm_id].target_duty - (int16_t)timer_info[pwm_id].init_duty;
        #endif
        // 变化时间与占空比差值成比例：change_time = duty_diff * SCALE
        // 步数 = 总变化时间 / 定时器周期
        uint16_t change_time = (duty_diff > 0 ? duty_diff : 0) * PUMP_PWM_GRADUAL_CHANGE_TIME_SCALE; // 总变化时间（ms），与占空比差值成正比
        uint16_t time_period = PUMP_PWM_GRADUAL_CHANGE_PERIOD; // 定时器周期（ms）
        
        // 计算步数（向上取整，确保能覆盖整个变化范围）
        uint16_t step_count = (change_time + time_period - 1) / time_period; // 向上取整
        if (step_count == 0) {
            step_count = 1; // 至少1步
        }
        
        // 计算步长（无符号，总是增加）
        // 默认目标占空比不会小于初始占空比
        if (duty_diff > 0) {
            // 向上取整：(duty_diff + step_count - 1) / step_count
            timer_info[pwm_id].duty_step = (duty_diff + step_count - 1) / step_count;
        } else {
            // 目标占空比小于等于初始占空比（异常情况），默认以初始占空比启动PWM
            LOGE("target_duty (%d) < init_duty (%d), this should not happen!", 
                 timer_info[pwm_id].target_duty, timer_info[pwm_id].init_duty);
            timer_info[pwm_id].duty_step = 0;
            // 清除定时器运行状态
            timer_info[pwm_id].time_status = 0;
            return 0;
        }
        
        // 设置定时器周期
        timer_info[pwm_id].time_period = time_period;
        
        // 计算定时器最大可能运行时间
        // 注意：定时器会在 current_duty >= target_duty 时提前停止
        // 所以实际运行时间可能小于 step_count * time_period
        // total_time 表示最大可能的运行时间（如果所有步都执行完）
        uint16_t total_time_max = step_count * time_period;
        
        // 估算实际运行时间（基于步长计算）
        // 实际需要的步数 = ceil(duty_diff / duty_step)
        uint16_t actual_steps_needed = 0;
        if (duty_diff > 0 && timer_info[pwm_id].duty_step > 0) {
            actual_steps_needed = (duty_diff + timer_info[pwm_id].duty_step - 1) / timer_info[pwm_id].duty_step;
        }
        uint16_t total_time_estimated = actual_steps_needed * time_period;
        
        LOG_PBUS("PWM gradual change: init=%d, target=%d, diff=%d, change_time=%dms, step_count=%d, step=%d, period=%dms",
             timer_info[pwm_id].init_duty, timer_info[pwm_id].target_duty, duty_diff,
             change_time, step_count, timer_info[pwm_id].duty_step, timer_info[pwm_id].time_period);
        LOG_PBUS("  Time estimate: max=%dms, estimated=%dms (actual_steps_needed=%d)",
             total_time_max, total_time_estimated, actual_steps_needed);
        
        bsp_motor_start_periodic_task(pwm_id);
        return 0; // 成功
    }
}

static void pump_1_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    //LOG_PBUS("power key pin[%d] irq triggered", pin);
    if (timer_info[MOTOR_PWM_PUMP_1].gpio_irq_update == 1) {
        timer_info[MOTOR_PWM_PUMP_1].gpio_irq_update = 0;
        hal_pwm_update_duty_only(motor_pwm_cfg[MOTOR_PWM_PUMP_1].id, timer_info[MOTOR_PWM_PUMP_1].current_duty, timer_info[MOTOR_PWM_PUMP_1].pwm_cfg.freq);
    }
    
}

static void pump_2_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    //LOG_PBUS("power key pin[%d] irq triggered", pin);
    if (timer_info[MOTOR_PWM_PUMP_2].gpio_irq_update == 1) {
        timer_info[MOTOR_PWM_PUMP_2].gpio_irq_update = 0;
        hal_pwm_update_duty_only(motor_pwm_cfg[MOTOR_PWM_PUMP_2].id, timer_info[MOTOR_PWM_PUMP_2].current_duty, timer_info[MOTOR_PWM_PUMP_2].pwm_cfg.freq);
    }
}
static int8_t bsp_motor_pwm_gradual_gpio_irq_init(int8_t enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
      enable, 0, HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
      HAL_GPIO_IRQ_POLARITY_LOW_FALLING, pump_1_irq_callback};

    hal_gpio_setup_irq(PWM_PUMP1_PIN, &gpio_irq_cfg);
    gpio_irq_cfg.irq_handler = pump_2_irq_callback;
    hal_gpio_setup_irq(PWM_PUMP2_PIN, &gpio_irq_cfg);
  return 0;
}
#endif

#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
 int bsp_motor_pwm_idle_high(motor_pwm_control_id_e id)
 {
    if (id >= MOTOR_PWM_VALVE_1) {
        LOGE("pwm_id is out of MOTOR_PWM_PUMP_2 range: %d", id);
        return -1;
    }
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = BREAST_PUMP_PUMP_FREQ,
        .ratio = 100,
        .inv = false,
        .sleep_on = false,
    };
    return hal_pwm_enable(motor_pwm_cfg[id].id, &pwm_cfg);
 }
#endif
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_motor_init(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(motor_pwm_cfg); i++) {
        motor_pwm_cfg[i].iomux_set();
    }
    memset(motor_pwm_status, 0, sizeof(motor_pwm_status));
#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
    if (bsp_motor_init_timer_system() != 0) {
        LOGE("Failed to initialize timer system");
        return BSP_ERROR;
    }
    bsp_motor_pwm_gradual_gpio_irq_init(1);
    // 直接运行PWM测试函数
    // bsp_motor_pwm_test_all_step_counts();
#endif

#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_1);
    bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_2);
#endif
    return BSP_OK;
}

int bsp_motor_deinit(void)
{
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_1);
    bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_2);
    hal_pwm_disable(motor_pwm_cfg[MOTOR_PWM_VALVE_1].id);
    hal_pwm_disable(motor_pwm_cfg[MOTOR_PWM_VALVE_2].id);
#else
    for (int i = 0; i < ARRAY_SIZE(motor_pwm_cfg); i++) {
        hal_pwm_disable(motor_pwm_cfg[i].id);
    }
#endif

#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
    bsp_motor_deinit_timer_system();
    bsp_motor_pwm_gradual_gpio_irq_init(0);
#endif
    return BSP_OK;
}

int bsp_motor_parameter_set(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    motor_pwm_control_id_e pwm_id = (motor_pwm_control_id_e )tmp[0];
    if (pwm_id >= MOTOR_PWM_ID_MAX) {
        LOGE("pwm_id is out of range: %d", pwm_id);
        return -1;
    }
    int ret = 0;
    motor_para_t motor_para = {0};
    // tmp[1] 存储的是 motor_para_t* 指针值（地址），需要转换为指针类型
    motor_para_t *motor_ptr = (motor_para_t *)(uintptr_t)tmp[1];
    memcpy(&motor_para, motor_ptr, sizeof(motor_para_t));
    motor_para.un.pwm.duty = motor_para.un.pwm.duty > 100 ? 100 : motor_para.un.pwm.duty;
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    if (pwm_id == MOTOR_PWM_PUMP_1 || pwm_id == MOTOR_PWM_PUMP_2) {
        motor_para.un.pwm.duty = 100 - motor_para.un.pwm.duty;
    }
#endif
#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
    hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_208M);
    ret = bsp_motor_pwm_gradual_change_start(&motor_para, pwm_id);
#else
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = motor_para.un.pwm.freq,
        .ratio = motor_para.un.pwm.duty,
        .inv = false,
        .sleep_on = false,
    };
    ret = hal_pwm_enable(motor_pwm_cfg[pwm_id].id, &pwm_cfg);
#endif
#if IS_SUPPORT_PUMP_RUN_TIME_TEST
    if (pwm_id == MOTOR_PWM_PUMP_1) {
        start_time = get_timestamp();
    }
#endif
    motor_pwm_status[pwm_id] = 1;
#if IS_SUPPORT_PUMP_DEBUG_LOG
    BSP_LOG("parameter", "pwm_id: %d, duty: %d, freq: %d, ret: %d", pwm_id, 
                motor_para.un.pwm.duty, motor_para.un.pwm.freq, ret);
#endif
    return ret;
}

int bsp_motor_stop(void *args)
{
    int ret = 0;
    motor_pwm_control_id_e pwm_id = *(motor_pwm_control_id_e *)args;
#if IS_SUPPORT_PUMP_DEBUG_LOG
    BSP_LOG("stop", "pwm_id: %d", pwm_id);
#endif
    // BSP_LOG("stop", "pwm_id: %d", pwm_id);
    if (pwm_id >= MOTOR_PWM_ID_MAX) {
#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
        bsp_motor_stop_periodic_task(MOTOR_PWM_PUMP_1);
        bsp_motor_stop_periodic_task(MOTOR_PWM_PUMP_2);
#endif
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
        ret |= bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_1);
        ret |= bsp_motor_pwm_idle_high(MOTOR_PWM_PUMP_2);
        ret |= hal_pwm_disable(motor_pwm_cfg[MOTOR_PWM_VALVE_1].id);
        ret |= hal_pwm_disable(motor_pwm_cfg[MOTOR_PWM_VALVE_2].id);
#else
      for (uint8_t i = 0; i < MOTOR_PWM_ID_MAX; i++) {
        ret |= hal_pwm_disable(motor_pwm_cfg[i].id);
        motor_pwm_status[i] = 0;
      }
#endif
        return ret;
    }
#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
    if (pwm_id == MOTOR_PWM_PUMP_1 || pwm_id == MOTOR_PWM_PUMP_2) {
        bsp_motor_stop_periodic_task(pwm_id);
    }
#endif
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    if (pwm_id == MOTOR_PWM_PUMP_1 || pwm_id == MOTOR_PWM_PUMP_2) {
        ret = bsp_motor_pwm_idle_high(pwm_id);
    }
    if (pwm_id == MOTOR_PWM_VALVE_1 || pwm_id == MOTOR_PWM_VALVE_2) {
        ret = hal_pwm_disable(motor_pwm_cfg[pwm_id].id);
    }
#else
    ret = hal_pwm_disable(motor_pwm_cfg[pwm_id].id);
#endif
#if IS_SUPPORT_PUMP_RUN_TIME_TEST
    if (pwm_id == MOTOR_PWM_PUMP_1) {
        if (count < 20) {
            run_time[count] = get_timestamp() - start_time;
            count++;
        } else {
            PRINT_LOG("run_time %d: ", TICKS_TO_MS(run_time[0]));
            for (uint8_t i = 0; i < 20; i++) {
                PRINT_LOG("%d ", run_time[i]);
            }
            PRINT_LOG("\r\n");
            count = 0;
        }
    }
#endif
    motor_pwm_status[pwm_id] = 0;
    return ret;
}

int bsp_motor_start(void *args)
{
    bsp_motor_parameter_set(args);
    return BSP_OK;
}

int bsp_motor_is_running(void *args)
{
    motor_pwm_control_id_e pwm_id = *(motor_pwm_control_id_e *)args;
    if (pwm_id >= MOTOR_PWM_ID_MAX) {
        return -1;
    }
    return motor_pwm_status[pwm_id] > 0 ? 1 : 0;
}

#if IS_SUPPORT_PUMP_PWM_GRADUAL_CHANGE
/**
 * @brief 测试所有可能的步数组合
 * 系统地测试各种 step_count 值（1-128），确保计算逻辑正确
 */
void bsp_motor_pwm_test_all_step_counts(void)
{
    LOG_PBUS("========== PWM Step Count Comprehensive Test ==========");
    LOG_PBUS("Config: INIT_DUTY=%d, PERIOD=%dms, SCALE=%d", 
         PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY, 
         PUMP_PWM_GRADUAL_CHANGE_PERIOD,
         PUMP_PWM_GRADUAL_CHANGE_TIME_SCALE);
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    LOG_PBUS("Testing DOWNWARD gradient: target_duty from 0 to %d", PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY);
#else
    LOG_PBUS("Testing UPWARD gradient: target_duty from %d to 100", PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY);
#endif
    LOG_PBUS("------------------------------------------------------");
    
    uint8_t init_duty = PUMP_PWM_GRADUAL_CHANGE_INIT_DUTY;
    uint16_t time_period = PUMP_PWM_GRADUAL_CHANGE_PERIOD;
    uint16_t scale = PUMP_PWM_GRADUAL_CHANGE_TIME_SCALE;
    
    int pass_count = 0;
    int fail_count = 0;
    
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    // 向下渐变：测试从 init_duty (90) 向下到 0
    for (uint8_t target_duty = 0; target_duty <= init_duty; target_duty++) {
#else
    // 向上渐变：测试从 init_duty (10) 向上到 100
    for (uint8_t target_duty = init_duty; target_duty <= 100; target_duty++) {
#endif
        // 先计算需要变化的占空比差值
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
        // 向下渐变：duty_diff = init_duty - target_duty
        int16_t duty_diff = (int16_t)init_duty - (int16_t)target_duty;
#else
        // 向上渐变：duty_diff = target_duty - init_duty
        int16_t duty_diff = (int16_t)target_duty - (int16_t)init_duty;
#endif
        // 计算变化时间：基于占空比差值，而不是目标值
        uint16_t change_time = (duty_diff > 0 ? duty_diff : 0) * scale;
        
        // 计算步数
        uint16_t calculated_step_count = (change_time + time_period - 1) / time_period;
        if (calculated_step_count == 0) {
            calculated_step_count = 1;
        }
        uint8_t duty_step = 0;
        if (duty_diff > 0) {
            duty_step = (duty_diff + calculated_step_count - 1) / calculated_step_count;
        }
        
        // 模拟执行验证
        uint8_t final_duty = init_duty;
        uint16_t actual_steps = 0;
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
        // 向下渐变：从 init_duty 向下到 target_duty
        for (uint16_t j = 0; j < calculated_step_count && final_duty > target_duty; j++) {
            uint8_t new_duty = final_duty - duty_step;
            if (new_duty < target_duty) {
                new_duty = target_duty;
            }
            final_duty = new_duty;
            actual_steps++;
            if (final_duty <= target_duty) {
                break; // 达到目标，提前停止（模拟定时器行为）
            }
        }
        bool can_reach_target = (final_duty <= target_duty);
#else
        // 向上渐变：从 init_duty 向上到 target_duty
        for (uint16_t j = 0; j < calculated_step_count && final_duty < target_duty; j++) {
            uint8_t new_duty = final_duty + duty_step;
            if (new_duty > target_duty) {
                new_duty = target_duty;
            }
            final_duty = new_duty;
            actual_steps++;
            if (final_duty >= target_duty) {
                break; // 达到目标，提前停止（模拟定时器行为）
            }
        }
        bool can_reach_target = (final_duty >= target_duty);
#endif
        
        // 计算实际运行时间（定时器会在达到目标时提前停止）
        uint16_t total_time = actual_steps * time_period;
        uint16_t total_time_max = calculated_step_count * time_period;
        
        if (can_reach_target) {
            pass_count++;
            // 输出测试数据（单行格式，避免日志过多）
            LOG_PBUS("target_duty=%3d: change_time=%3dms, step_count=%3d, diff=%3d, step=%2d, final=%3d, actual_steps=%3d, total_time=%3dms (max=%3dms) [PASS]",
                 target_duty, change_time, calculated_step_count, duty_diff, duty_step, final_duty, 
                 actual_steps, total_time, total_time_max);
        } else {
            fail_count++;
            LOG_PBUS("target_duty=%3d: change_time=%3dms, step_count=%3d, diff=%3d, step=%2d, final=%3d [FAIL]",
                 target_duty, change_time, calculated_step_count, duty_diff, duty_step, final_duty);
        }
    }
    
    LOG_PBUS("------------------------------------------------------");
    LOG_PBUS("========== Test Summary ==========");
#if IS_SUPPORT_PUMP_PWM_IDLE_HIGH
    LOG_PBUS("Total tested: %d (target_duty from 0 to %d)", init_duty + 1, init_duty);
    LOG_PBUS("Success rate: %.2f%%", (float)pass_count * 100.0f / (init_duty + 1));
#else
    LOG_PBUS("Total tested: %d (target_duty from %d to 100)", 100 - init_duty + 1, init_duty);
    LOG_PBUS("Success rate: %.2f%%", (float)pass_count * 100.0f / (100 - init_duty + 1));
#endif
    LOG_PBUS("Passed: %d", pass_count);
    LOG_PBUS("Failed: %d", fail_count);
    LOG_PBUS("==================================");
}
#endif