/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h"
#include "hal_timer.h"
#include "bsp_common.h" 
#include "bsp_config.h"
#include "motor_stall_detection.h" // IWYU pragma: keep
#include "motor_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_RUN_TIME_TEST          (0)
#define IS_SUPPORT_PUMP_DEBUG_LOG              (0)
#define IS_SUPPORT_PUMP_PWM_IDLE_HIGH          (1)
#define PUMP_PWM_TIME_NUMBER                   (2)   /// 泵1和泵2各一个定时器
#define MOTOR_PUMP_IDLE_PWM_DUTY               (100)
#define MOTOR_PUMP_PULSE_WIDTH_MAX             (100 * PUMP_DUTY_SCALE_FACTOR)
#define MOTOR_PUMP_START_PWM_DUTY              (90)
#define MOTOR_VALVE_START_PWM_DUTY             (100)
#define MOTOR_VALVE_IDLE_PWM_DUTY              (0)
#define PUMP_GPIO_IDLE_CTRL                    (1) /// 泵GPIO空闲状态控制，1：使能，0：禁用

typedef struct {
    enum HAL_PWM_ID_T pump_pwm_id;
    enum HAL_PWM_ID_T valve_pwm_id;
    void (*pump_pwm_iomux_set)(void);
    void (*valve_pwm_iomux_set)(void);
} motor_pump_cfg_t;

typedef struct {
    uint8_t  pump_is_running;         /// 运行状态，0：停止，1：运行
    uint8_t  valve_is_running;        /// 运行状态，0：停止，1：运行
    uint8_t  pump_irq_is_running;     /// 泵PWM中断运行状态，0：停止，1：运行
    uint8_t  valve_duty;              /// 阀门占空比
    uint16_t target_pulse_width;      /// 目标脉冲宽度
    uint16_t setting_pulse_width;     /// 设置脉冲宽度
    uint16_t current_pulse_width;     /// 当前脉冲宽度
} motor_pump_status_t;

typedef struct {
    HWTIMER_ID time_id;               /// 定时器ID
    HWTIMER_ID idle_power_time_id;    /// 电源空闲定时器ID  
    uint8_t pump_id;                  /// 泵ID
    uint8_t is_running;               /// 定时器运行状态，0：停止，1：运行
    motor_pump_status_e action;       /// 动作
    uint32_t start_time;              /// 开始时间
} motor_pump_timer_status_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static const motor_pump_cfg_t motor_pump_cfg[MOTOR_PUMP_ID_MAX] = {
    {MOTOR_PUMP1_PWM_ID, MOTOR_VALVE1_PWM_ID,  MOTOR_PUMP1_PWM_IMUX,  MOTOR_VALVE1_PWM_IMUX},
    {MOTOR_PUMP2_PWM_ID, MOTOR_VALVE2_PWM_ID,  MOTOR_PUMP2_PWM_IMUX,  MOTOR_VALVE2_PWM_IMUX},
};

static motor_pump_status_t pump_status[MOTOR_PUMP_ID_MAX] = {0};
static motor_pump_timer_status_t timer_status[MOTOR_PUMP_ID_MAX] = {0};
static pump_timer_event_cb timer_event_cb = NULL;
static pump_stalled_cb stalled_cb = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void bsp_motor_pwm_irq_control(enum HAL_GPIO_PIN_T pin, bool enable);
static int bsp_motor_pwm_duty_set(enum HAL_PWM_ID_T pwm_id, uint8_t duty);

static char *bsp_motor_pump_action_to_string(motor_pump_status_e action)
{
    switch (action) {
        case MOTOR_PUMP_ON:
            return "pump on";
        case MOTOR_PUMP_OFF:
            return "pump off";
        case MOTOR_PUMP_OFF_AND_VALVE_ON:
            return "pump off and valve on";
        case MOTOR_VALVE_ON:
            return "valve on";
        case MOTOR_VALVE_OFF:
            return "valve off";
        case MOTOR_PUMP_OFF_AND_VALVE_OFF:
            return "pump off and valve off";
        case MOTOR_PUMP_DUTY_ADJUST:
            return "pump duty adjust";
        default:
            return "unknown";
    }
}

static void bsp_motor_pump_timer_start(motor_pump_id_e id, uint32_t period, motor_pump_status_e action)
{
    int ret = 0;
    if (id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump_id is out range: %d", id);
        return;
    }
    if (hwtimer_active(timer_status[id].time_id)) {
#if IS_SUPPORT_PUMP_DEBUG_LOG
        PRINT_LOG("timer stop:%d\r\n", id);
#endif
        ret = hwtimer_stop(timer_status[id].time_id);
        if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
            LOGE("motor pump time stop failed, ret=%d", ret);
            return;
        }
    }
    timer_status[id].pump_id = id;
    timer_status[id].action = action;
    timer_status[id].start_time = get_timestamp();
    ret = hwtimer_start(timer_status[id].time_id, MS_TO_HWTICKS(period));
    if (ret != E_HWTIMER_OK) {
        LOGE("motor pwm time start failed, ret=%d, time_id=%p", ret, timer_status[id].time_id);
        return;
    }
#if IS_SUPPORT_PUMP_DEBUG_LOG
    PRINT_LOG("timer start:%d/%s/%d\r\n", id, bsp_motor_pump_action_to_string(action), period);
#endif
    timer_status[id].is_running = true;
}

static void bsp_motor_pump_timer_stop(motor_pump_id_e id)
{
    int ret = hwtimer_stop(timer_status[id].time_id);
    if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK && timer_status[id].is_running) {
        LOGE("motor pump time stop failed, ret=%d", ret);
    }
#if IS_SUPPORT_PUMP_DEBUG_LOG
    PRINT_LOG("timer stop:%d\r\n", id);
#endif
    timer_status[id].is_running = false;
}

POSSIBLY_UNUSED static void bsp_motor_pump_timer_handler(void *param)
{
    motor_pump_timer_status_t *timer_status = (motor_pump_timer_status_t *)param;
    timer_status->is_running = false;
    if (timer_status->action == MOTOR_PUMP_ON) {
        pump_status[timer_status->pump_id].pump_is_running = false;
        bsp_motor_pwm_duty_set(motor_pump_cfg[timer_status->pump_id].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
    }
#if IS_SUPPORT_PUMP_DEBUG_LOG
    PRINT_LOG("timer cb:%d/%s/%d\r\n", timer_status->pump_id,
        bsp_motor_pump_action_to_string(timer_status->action),
        get_timestamp_ms(timer_status->start_time));
#endif
    if (timer_event_cb != NULL && timer_status->action != MOTOR_PUMP_IDLE) {
        timer_event_cb(timer_status->pump_id, timer_status->action);
        timer_status->action = MOTOR_PUMP_IDLE;
    } 
}

POSSIBLY_UNUSED static int bsp_motor_pump_timer_deinit(void)
{
    for (int i = 0; i < MOTOR_PUMP_ID_MAX; i++) {
        if (timer_status[i].time_id) {
            hwtimer_free(timer_status[i].time_id);
            timer_status[i].time_id = NULL;
        }
    }
    return 0;
}

POSSIBLY_UNUSED static int bsp_motor_pump_timer_init(void)
{
    for (int i = 0; i < PUMP_PWM_TIME_NUMBER; i++) {
        timer_status[i].time_id = hwtimer_alloc(bsp_motor_pump_timer_handler, &timer_status[i]);
        if (timer_status[i].time_id == NULL) {
            bsp_motor_pump_timer_deinit();
            LOGE("motor pump timer init failed");
            return -1;
        }
        timer_status[i].is_running = false;
        LOG_PBUS("timer_status[%d].time_id=%p", i, timer_status[i].time_id);
    }
    LOG_PBUS("motor pump timer init success");
    return 0;
}

#if IS_SUPPORT_PUMP_DUTY_ADJUST
POSSIBLY_UNUSED static void bsp_motor_pump_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    int ret = 0;
    if (pin == PWM_PUMP1_PIN) {
        
        if (pump_status[MOTOR_PUMP_1].current_pulse_width == pump_status[MOTOR_PUMP_1].target_pulse_width) {
            // LOG_PBUS("after PUMP DUTY ADJUST: %d/%d", pump_status[MOTOR_PUMP_1].current_pulse_width, pump_status[MOTOR_PUMP_1].target_pulse_width);
            pump_status[MOTOR_PUMP_1].pump_irq_is_running = false;
            bsp_motor_pwm_irq_control(PWM_PUMP1_PIN, false);
        } else if (pump_status[MOTOR_PUMP_1].current_pulse_width > pump_status[MOTOR_PUMP_1].target_pulse_width) {
            pump_status[MOTOR_PUMP_1].current_pulse_width -= (1 * PUMP_DUTY_SCALE_FACTOR);
            if (pump_status[MOTOR_PUMP_1].current_pulse_width < pump_status[MOTOR_PUMP_1].target_pulse_width) {
                pump_status[MOTOR_PUMP_1].current_pulse_width = pump_status[MOTOR_PUMP_1].target_pulse_width;
            }
            // LOG_PBUS("-before PUMP DUTY ADJUST: %d/%d", pump_status[MOTOR_PUMP_1].current_pulse_width, pump_status[MOTOR_PUMP_1].target_pulse_width);
            hal_pwm_update_pulse_width(MOTOR_PUMP1_PWM_ID, 
                pump_status[MOTOR_PUMP_1].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        } else {
            pump_status[MOTOR_PUMP_1].current_pulse_width += (1 * PUMP_DUTY_SCALE_FACTOR);
            if (pump_status[MOTOR_PUMP_1].current_pulse_width > pump_status[MOTOR_PUMP_1].target_pulse_width) {
                pump_status[MOTOR_PUMP_1].current_pulse_width = pump_status[MOTOR_PUMP_1].target_pulse_width;
            }
            // LOG_PBUS("+before PUMP DUTY ADJUST: %d/%d", pump_status[MOTOR_PUMP_1].current_pulse_width, pump_status[MOTOR_PUMP_1].target_pulse_width);
            hal_pwm_update_pulse_width(MOTOR_PUMP1_PWM_ID, 
                pump_status[MOTOR_PUMP_1].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        }
        
    } else if (pin == PWM_PUMP2_PIN) {
        if (pump_status[MOTOR_PUMP_2].current_pulse_width == pump_status[MOTOR_PUMP_2].target_pulse_width) {
            pump_status[MOTOR_PUMP_2].pump_irq_is_running = false;
            bsp_motor_pwm_irq_control(PWM_PUMP2_PIN, false);
        } else if (pump_status[MOTOR_PUMP_2].current_pulse_width > pump_status[MOTOR_PUMP_2].target_pulse_width) {
            pump_status[MOTOR_PUMP_2].current_pulse_width -= (1 * PUMP_DUTY_SCALE_FACTOR);
            if (pump_status[MOTOR_PUMP_2].current_pulse_width < pump_status[MOTOR_PUMP_2].target_pulse_width) {
                pump_status[MOTOR_PUMP_2].current_pulse_width = pump_status[MOTOR_PUMP_2].target_pulse_width;
            }
            hal_pwm_update_pulse_width(MOTOR_PUMP2_PWM_ID, 
                pump_status[MOTOR_PUMP_2].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        } else {
            pump_status[MOTOR_PUMP_2].current_pulse_width += (1 * PUMP_DUTY_SCALE_FACTOR);
            if (pump_status[MOTOR_PUMP_2].current_pulse_width > pump_status[MOTOR_PUMP_2].target_pulse_width) {
                pump_status[MOTOR_PUMP_2].current_pulse_width = pump_status[MOTOR_PUMP_2].target_pulse_width;
            }
            hal_pwm_update_pulse_width(MOTOR_PUMP2_PWM_ID, 
                pump_status[MOTOR_PUMP_2].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        }
    }
}
#else
POSSIBLY_UNUSED static void bsp_motor_pump_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    int ret = 0;
    if (pin == PWM_PUMP1_PIN) {
        if (pump_status[MOTOR_PUMP_1].current_pulse_width <= pump_status[MOTOR_PUMP_1].target_pulse_width) {
            ret = hal_pwm_update_pulse_width(MOTOR_PUMP1_PWM_ID, 
                pump_status[MOTOR_PUMP_1].target_pulse_width, BREAST_PUMP_PUMP_FREQ);
            bsp_motor_pwm_irq_control(PWM_PUMP1_PIN, false);
            return;
        }
        hal_pwm_update_pulse_width(MOTOR_PUMP1_PWM_ID, 
            pump_status[MOTOR_PUMP_1].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        pump_status[MOTOR_PUMP_1].current_pulse_width -= (1 * PUMP_DUTY_SCALE_FACTOR);
    } else if (pin == PWM_PUMP2_PIN) {
        if (pump_status[MOTOR_PUMP_2].current_pulse_width <= pump_status[MOTOR_PUMP_2].target_pulse_width) {
            hal_pwm_update_pulse_width(MOTOR_PUMP2_PWM_ID, 
                pump_status[MOTOR_PUMP_2].target_pulse_width, BREAST_PUMP_PUMP_FREQ);
            bsp_motor_pwm_irq_control(PWM_PUMP2_PIN, false);
            return;
        } 
        hal_pwm_update_pulse_width(MOTOR_PUMP2_PWM_ID, 
            pump_status[MOTOR_PUMP_2].current_pulse_width, BREAST_PUMP_PUMP_FREQ);
        pump_status[MOTOR_PUMP_2].current_pulse_width -= (1 * PUMP_DUTY_SCALE_FACTOR);
    }
}
#endif

POSSIBLY_UNUSED static void bsp_motor_pwm_irq_control(enum HAL_GPIO_PIN_T pin, bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable, 
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, 
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING, 
        bsp_motor_pump_irq_callback
    };
    hal_gpio_setup_irq(pin, &gpio_irq_cfg);
#if IS_SUPPORT_PUMP_DEBUG_LOG
    PRINT_LOG("irq control:%d/%d\r\n", pin, enable);
#endif
}

#if PUMP_GPIO_IDLE_CTRL
static int motor_pwm_gpio_control(enum HAL_PWM_ID_T pwm_id ,bool enable, bool is_output)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)(pwm_id == MOTOR_PUMP1_PWM_ID ? PWM_PUMP1_PIN : PWM_PUMP2_PIN),  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_NOPULL,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(pwm_id == MOTOR_PUMP1_PWM_ID ? PWM_PUMP1_PIN : PWM_PUMP2_PIN, \
            is_output ? HAL_GPIO_DIR_OUT : HAL_GPIO_DIR_IN, enable ? 1 : 0);

    return BSP_OK;
}
#endif

static int bsp_motor_pwm_duty_set(enum HAL_PWM_ID_T pwm_id, uint8_t duty)
{   
    if ((pwm_id == MOTOR_PUMP1_PWM_ID || pwm_id == MOTOR_PUMP2_PWM_ID) \
        && (duty == MOTOR_PUMP_IDLE_PWM_DUTY))
    {
#if PUMP_GPIO_IDLE_CTRL
        motor_pwm_gpio_control(pwm_id, true, true); //使能GPIO 输出高
#endif
        hal_pwm_disable(pwm_id);
#if PUMP_GPIO_IDLE_CTRL
        return BSP_OK;
#endif
    }
#if PUMP_GPIO_IDLE_CTRL
    ///如果pwm_id是泵1或泵2，重新初始化为pwm输出
    if ((pwm_id == MOTOR_PUMP1_PWM_ID || pwm_id == MOTOR_PUMP2_PWM_ID)) {
        motor_pwm_gpio_control(pwm_id, true, false); //使能GPIO 输入
        for (uint8_t i = 0; i < LENGTH_OF_ARRAY(motor_pump_cfg); i++) {
            if (motor_pump_cfg[i].pump_pwm_id == pwm_id) {
                motor_pump_cfg[i].pump_pwm_iomux_set();
                break;
            }
        }
    }
#endif
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = BREAST_PUMP_PUMP_FREQ,
        .ratio = duty,
        .inv = false,
        .sleep_on = false,
    };
    return hal_pwm_enable(pwm_id, &pwm_cfg);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_motor_pump_init(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(motor_pump_cfg); i++) {
        motor_pump_cfg[i].pump_pwm_iomux_set();
        motor_pump_cfg[i].valve_pwm_iomux_set();
        bsp_motor_pwm_duty_set(motor_pump_cfg[i].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
        bsp_motor_pwm_duty_set(motor_pump_cfg[i].valve_pwm_id, MOTOR_VALVE_IDLE_PWM_DUTY);
    }
    bsp_motor_pump_timer_init();
    return BSP_OK;
}

int bsp_motor_pump_deinit(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(motor_pump_cfg); i++) {
        hal_pwm_disable(motor_pump_cfg[i].valve_pwm_id);
        bsp_motor_pwm_duty_set(motor_pump_cfg[i].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
        bsp_motor_pwm_duty_set(motor_pump_cfg[i].valve_pwm_id, MOTOR_VALVE_IDLE_PWM_DUTY);
    }
    bsp_motor_pump_timer_deinit();
    return BSP_OK;
}

int bsp_motor_pump_parameter_set(void *args)
{
    int ret = 0;
    bool is_stalled = false;
    motor_para_t *motor_para = (motor_para_t *)args;
    if (motor_para == NULL) {
        LOGE("motor_ptr is NULL");
        return -1;
    }
    motor_pump_para_t *pump_para = &motor_para->un.pump;
    uint8_t pump_id = pump_para->pump_id;
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        LOGE("pump_id is out range: %d", pump_id);
        return -1;
    }

    switch (pump_para->action) {
        case MOTOR_PUMP_ON:
            pump_motor_stall_detection_start(pump_id, pump_para->duration_ms, pump_para->pulse_width);
            uint16_t start_pulse_width = MOTOR_PUMP_START_PWM_DUTY * PUMP_DUTY_SCALE_FACTOR;
            pump_status[pump_id].pump_is_running = true;
            pump_status[pump_id].setting_pulse_width = MOTOR_PUMP_PULSE_WIDTH_MAX - pump_para->pulse_width;
            if (pump_status[pump_id].setting_pulse_width > start_pulse_width && pump_para->pulse_width != 0) {
                pump_status[pump_id].target_pulse_width = start_pulse_width;
                LOGE("pump %d error, pump_para->pulse_width:%d", pump_id, pump_para->pulse_width);
            } else {
                pump_status[pump_id].target_pulse_width = pump_status[pump_id].setting_pulse_width;
            }  

            if (pump_para->pulse_width != 0) {
                pump_status[pump_id].current_pulse_width = start_pulse_width;
                ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].pump_pwm_id, MOTOR_PUMP_START_PWM_DUTY);
                if (ret != BSP_OK) {
                    LOGE("set pump duty failed, ret=%d", ret);
                }
                bsp_motor_pwm_irq_control(pump_id == MOTOR_PUMP_1 ? PWM_PUMP1_PIN : PWM_PUMP2_PIN, true);
                pump_status[pump_id].pump_irq_is_running = true;
            } else {
                pump_status[pump_id].current_pulse_width = MOTOR_PUMP_PULSE_WIDTH_MAX;
            }
  
            if (pump_para->duration_ms > 0) {
                bsp_motor_pump_timer_start(pump_id, pump_para->duration_ms, MOTOR_PUMP_ON);
            } else {
                LOGW("pump %d pump on duration_ms is 0", pump_id);
            }
            break;
        case MOTOR_PUMP_OFF:
            pump_motor_stall_detection_stop(pump_id);
            is_stalled = pump_motor_is_stalled(pump_id);
            if (stalled_cb != NULL && is_stalled) {
                stalled_cb(pump_id, MOTOR_STATUS_STALL);
            }
            pump_status[pump_id].target_pulse_width = 0;
            pump_status[pump_id].current_pulse_width = 0;
            pump_status[pump_id].pump_is_running = false;
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
            if (ret != BSP_OK) {    
                LOGE("set pump duty failed, ret=%d", ret);
            }
            if (pump_para->duration_ms > 0) {
                bsp_motor_pump_timer_start(pump_id, pump_para->duration_ms, MOTOR_PUMP_OFF);
            } else {
                LOGW("pump %d pump off duration_ms is 0", pump_id);
            }
            bsp_motor_pwm_irq_control(pump_id == MOTOR_PUMP_1 ? PWM_PUMP1_PIN : PWM_PUMP2_PIN, false);
            pump_status[pump_id].pump_irq_is_running = false;
            break;
        case MOTOR_PUMP_OFF_AND_VALVE_ON:
            pump_motor_stall_detection_stop(pump_id);
            is_stalled = pump_motor_is_stalled(pump_id);
            if (stalled_cb != NULL && is_stalled) {
                stalled_cb(pump_id, MOTOR_STATUS_STALL);
            }
            pump_status[pump_id].target_pulse_width = 0;
            pump_status[pump_id].current_pulse_width = 0;
            pump_status[pump_id].pump_is_running = false;
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
            if (ret != BSP_OK) {
                LOGE("set pump off failed, ret=%d", ret);
            }
            pump_status[pump_id].valve_is_running = true;
            pump_status[pump_id].valve_duty = pump_para->pulse_width / PUMP_DUTY_SCALE_FACTOR;
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].valve_pwm_id, pump_status[pump_id].valve_duty);
            if (ret != BSP_OK) {
                LOGE("set valve on failed, ret=%d", ret);
            }
            if (pump_para->duration_ms > 0) {
                bsp_motor_pump_timer_start(pump_id, pump_para->duration_ms, MOTOR_PUMP_OFF_AND_VALVE_ON);
            } else {
                LOGW("pump %d pump off and valve on duration_ms is 0", pump_id);
            }
            break;
        case MOTOR_VALVE_ON:
            pump_status[pump_id].valve_is_running = true;
            pump_status[pump_id].valve_duty = pump_para->pulse_width  / PUMP_DUTY_SCALE_FACTOR;
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].valve_pwm_id, pump_status[pump_id].valve_duty);
            if (ret != BSP_OK) {
                LOGE("set valve duty failed, ret=%d", ret);
            } 
            if (pump_para->duration_ms > 0) {
                bsp_motor_pump_timer_start(pump_id, pump_para->duration_ms, MOTOR_VALVE_ON);
            } else {
                LOGW("pump %d valve on duration_ms is 0", pump_id);
            }
            break;
        case MOTOR_VALVE_OFF:
            pump_status[pump_id].valve_is_running = false;
            pump_status[pump_id].valve_duty = MOTOR_VALVE_IDLE_PWM_DUTY;
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].valve_pwm_id, MOTOR_VALVE_IDLE_PWM_DUTY);
            if (ret != BSP_OK) {
                LOGE("set valve duty failed, ret=%d", ret);
            } else {
                pump_status[pump_id].valve_is_running = false;
                pump_status[pump_id].valve_duty = MOTOR_VALVE_IDLE_PWM_DUTY;
            }
            if (pump_para->duration_ms > 0) {
                bsp_motor_pump_timer_start(pump_id, pump_para->duration_ms, MOTOR_VALVE_OFF);
            } else {
                LOGW("pump %d valve off duration_ms is 0", pump_id);
            }
            break;
        case MOTOR_PUMP_OFF_AND_VALVE_OFF:
            memset(&pump_status[pump_id], 0, sizeof(motor_pump_status_t));
            bsp_motor_pump_timer_stop(pump_id);
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].pump_pwm_id, MOTOR_PUMP_IDLE_PWM_DUTY);
            if (ret != BSP_OK) {
                LOGE("set pump off failed, ret=%d", ret);
            }
            ret = bsp_motor_pwm_duty_set(motor_pump_cfg[pump_id].valve_pwm_id, MOTOR_VALVE_IDLE_PWM_DUTY);
            if (ret != BSP_OK) {
                LOGE("set valve off failed, ret=%d", ret);
            }
            break;
        case MOTOR_PUMP_DUTY_ADJUST:
#if IS_SUPPORT_PUMP_DUTY_ADJUST
            if (pump_status[pump_id].pump_is_running) {
                /// 计算目标脉冲宽度 ，防止溢出
                int32_t target_pulse_width_raw =
                    (int32_t)pump_status[pump_id].setting_pulse_width - (int32_t)pump_para->pulse_width;
                if (target_pulse_width_raw <= 0) {
                    LOGW("pump %d level adjust ignored, pulse_width:%d", pump_id, pump_para->pulse_width);
                    return 0;
                }
                if (target_pulse_width_raw > MOTOR_PUMP_PULSE_WIDTH_MAX) {
                    LOGW("pump %d level adjust ignored(high), pulse_width:%d, target:%d",
                        pump_id, pump_para->pulse_width, target_pulse_width_raw);
                    return 0;
                }
                uint16_t target_pulse_width = (uint16_t)target_pulse_width_raw;
                //PWM翻转后目标值 占空比值不能小于500 主要看泵最大能承载多大pwm周期，暂定500
                if (target_pulse_width < 500) {
                    target_pulse_width = 500;
                    LOGE("pump %d level adjust error, pulse_width:%d", pump_id, pump_para->pulse_width);
                } 
                if (pump_status[pump_id].target_pulse_width != target_pulse_width) {
#if IS_SUPPORT_PUMP_DEBUG_LOG
                    LOG_PBUS("pump %d level adjust:%d/%d", pump_id, pump_para->pulse_width, target_pulse_width);
#endif
                    pump_status[pump_id].target_pulse_width = target_pulse_width;
                    if (!pump_status[pump_id].pump_irq_is_running) {
                        bsp_motor_pwm_irq_control(pump_id == MOTOR_PUMP_1 ? PWM_PUMP1_PIN : PWM_PUMP2_PIN, true);
                        pump_status[pump_id].pump_irq_is_running = true;
                    }
                } else {
                    return 0;
                }
            }
#endif
            break;
        default:
            LOGE("unknown pump action: %d", pump_para->action);
            ret = -1;
            break;
    }
#if IS_SUPPORT_PUMP_DEBUG_LOG
    LOG_PBUS("pump:%d, action:%s, pw:%d, d_ms:%d; pump: %d/%d/%d; valve: %d/%d", 
        pump_id, bsp_motor_pump_action_to_string(pump_para->action), pump_para->pulse_width, pump_para->duration_ms,
        pump_status[pump_id].pump_is_running, pump_status[pump_id].target_pulse_width, pump_status[pump_id].current_pulse_width, 
        pump_status[pump_id].valve_is_running, pump_status[pump_id].valve_duty);
#endif

    return ret;
}

int bsp_motor_pump_add_timer_event_cb(pump_timer_event_cb cb)
{
    timer_event_cb = cb;
    LOG_PBUS("add timer event cb:%p", timer_event_cb);
    return timer_event_cb != NULL ? BSP_OK : BSP_ERROR;
}

int bsp_motor_pump_add_stalled_cb(pump_stalled_cb callback)
{
    stalled_cb = callback;
    LOG_PBUS("add stalled cb:%p", stalled_cb);
    return 0;
}

int bsp_motor_pump_is_running(void *args)
{
    motor_pump_id_e pump_id = *(motor_pump_id_e *)args;
    if (pump_id >= MOTOR_PUMP_ID_MAX) {
        return -1;
    }
    return pump_status[pump_id].pump_is_running;
}