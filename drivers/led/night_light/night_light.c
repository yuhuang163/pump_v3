/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h" // IWYU pragma: keep
#include "hal_timer.h" // IWYU pragma: keep
#include "hwtimer_list.h" // IWYU pragma: keep
#include "bsp_common.h"
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_led_internal.h"

#if IS_SUPPORT_NIGHT_LIGHT

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_NIGHT_LIGHT_CLIMB              (1)  /// 支持爬升
#define NIGHT_LIGHT_PERIOD                        1000
#define NIGHT_LIGHT_TIME_PERIOD                   35
#if NIGHT_LIGHT_TIME_PERIOD > 35
#error "NIGHT_LIGHT_TIME_PERIOD must not exceed 35ms"
#endif
#ifndef IS_SUPPORT_NIGHT_LIGHT_CLIMB_DIM_END_BLEND
#define IS_SUPPORT_NIGHT_LIGHT_CLIMB_DIM_END_BLEND (1) /// 变暗末段半阶插值
#endif
#ifndef IS_SUPPORT_NIGHT_LIGHT_CLIMB_BRIGHT_END_BLEND
#define IS_SUPPORT_NIGHT_LIGHT_CLIMB_BRIGHT_END_BLEND (1) /// 变亮末段半阶插值
#endif
#ifndef NIGHT_LIGHT_CLIMB_END_BLEND_THRESHOLD
#define NIGHT_LIGHT_CLIMB_END_BLEND_THRESHOLD     4u  /// 距 target 剩余档位数
#endif
#define NIGHT_LIGHT_LOG_DEBUG                     (1)

typedef struct {
    HWTIMER_ID time_id;
    uint8_t is_running;   /// 定时器状态，0：停止，1：运行
    uint8_t current_duty;   /// 当前占空比
    uint8_t target_duty;   /// 当前占空比
    uint8_t onoff;   /// 开关状态，0：关，1：开
    uint8_t duty_update_flag;    /// 占空比更新标志，0：不更新，1：更新
    uint8_t blend_phase;         /// 末段半阶：0 整档，1 半阶待落档
    uint32_t pulse_width;   /// 当前脉冲宽度
} night_light_timer_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_NIGHT_LIGHT_CLIMB
static night_light_timer_cfg_t time_info;
#endif

#define NIGHT_LIGHT_PWM_TABLE_INDEX_MIN                2
#define NIGHT_LIGHT_PWM_TABLE_INDEX_MAX                98

static const uint16_t pwm_table[101] = {
    0, 0, 2, 5, 9, 14, 21, 29, 39, 51,
    65, 80, 96, 115, 135, 157, 181, 207, 235, 265,
    296, 330, 366, 403, 443, 484, 528, 574, 621, 671,
    723, 777, 834, 892, 952, 1015, 1080, 1147, 1217, 1288,
    1362, 1438, 1516, 1597, 1680, 1765, 1852, 1942, 2034, 2128,
    2225, 2324, 2426, 2529, 2636, 2744, 2855, 2968, 3084, 3202,
    3323, 3446, 3572, 3700, 3830, 3963, 4098, 4236, 4376, 4519,
    4665, 4813, 4963, 5116, 5271, 5429, 5590, 5753, 5918, 6087,
    6257, 6431, 6607, 6785, 6967, 7150, 7337, 7526, 7717, 7912,
    8108, 8308, 8510, 8715, 8922, 9133, 9345, 9561, 9779, 10000,
    10000
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_NIGHT_LIGHT_CLIMB
static void night_light_gpio_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    if (time_info.duty_update_flag == 1) {
        time_info.duty_update_flag = 0;
        if (time_info.pulse_width == 0) {
            time_info.onoff = 0;
#if NIGHT_LIGHT_LOG_DEBUG
            LOG_PBUS("night light off...");
#endif
        }
        hal_pwm_update_pulse_width(NIGHT_LIGHT_PWM_ID, time_info.pulse_width, NIGHT_LIGHT_PERIOD);
    }
}

static void bsp_night_light_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable,
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
        night_light_gpio_irq_callback
    };
    hal_gpio_setup_irq(NIGHT_LIGHT_PIN, &gpio_irq_cfg);
}

static void bsp_night_light_climb_start(void)
{
    int ret = 0;
    if (hwtimer_active(time_info.time_id) == 0) {
        ret = hwtimer_start(time_info.time_id, MS_TO_HWTICKS(NIGHT_LIGHT_TIME_PERIOD));
        if (ret != E_HWTIMER_OK) {
            LOGE("night light time start fail ret=%d", ret);
            return;
        }
    }
    time_info.is_running = true;
}

static void bsp_night_light_climb_stop(void)
{
    int ret = hwtimer_stop(time_info.time_id);

    if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
        LOGE("night light time stop fail ret=%d", ret);
    }
    time_info.is_running = false; 
}

static void bsp_night_light_climb_schedule(void)
{
    hwtimer_start(time_info.time_id, MS_TO_HWTICKS(NIGHT_LIGHT_TIME_PERIOD));
}

#if IS_SUPPORT_NIGHT_LIGHT_CLIMB_DIM_END_BLEND
static bool bsp_night_light_dim_end_blend_needed(void)
{
    uint8_t gap = time_info.current_duty - time_info.target_duty;

    return gap > 0 && gap <= NIGHT_LIGHT_CLIMB_END_BLEND_THRESHOLD;
}
#endif

#if IS_SUPPORT_NIGHT_LIGHT_CLIMB_BRIGHT_END_BLEND
static bool bsp_night_light_bright_end_blend_needed(void)
{
    uint8_t gap = time_info.target_duty - time_info.current_duty;

    return gap > 0 && gap <= NIGHT_LIGHT_CLIMB_END_BLEND_THRESHOLD;
}
#endif

static void bsp_night_light_timer_handler(void *param)
{
    (void)param;

    if (time_info.current_duty > time_info.target_duty && time_info.current_duty > NIGHT_LIGHT_PWM_TABLE_INDEX_MIN) {
#if IS_SUPPORT_NIGHT_LIGHT_CLIMB_DIM_END_BLEND
        if (time_info.blend_phase) {
            time_info.blend_phase = 0;
            time_info.current_duty--;
            if (time_info.current_duty == time_info.target_duty) {
                LOG_PBUS("nl dim blend land: cur=tgt=%u, pulse=%lu",
                    (unsigned)time_info.current_duty,
                    (unsigned long)time_info.pulse_width);
                return;
            }
            time_info.pulse_width = pwm_table[time_info.current_duty];
            time_info.duty_update_flag = 1;
            bsp_night_light_climb_schedule();
            return;
        }

        if (bsp_night_light_dim_end_blend_needed()) {
            uint8_t next_duty = time_info.current_duty - 1;

            time_info.blend_phase = 1;
            time_info.pulse_width =
                (pwm_table[time_info.current_duty] + pwm_table[next_duty]) / 2;
            time_info.duty_update_flag = 1;
            LOG_PBUS("nl dim blend: %u -> %u, tgt=%u",
                (unsigned)time_info.current_duty,
                (unsigned)next_duty,
                (unsigned)time_info.target_duty);
            bsp_night_light_climb_schedule();
            return;
        }
#endif
        time_info.current_duty--;
        time_info.pulse_width = pwm_table[time_info.current_duty];
        time_info.duty_update_flag = 1;
        if (time_info.current_duty == time_info.target_duty) {
            return;
        }
        bsp_night_light_climb_schedule();
    } else if (time_info.current_duty < time_info.target_duty &&
               time_info.current_duty < NIGHT_LIGHT_PWM_TABLE_INDEX_MAX) {
#if IS_SUPPORT_NIGHT_LIGHT_CLIMB_BRIGHT_END_BLEND
        if (time_info.blend_phase) {
            time_info.blend_phase = 0;
            time_info.current_duty++;
            if (time_info.current_duty == time_info.target_duty) {
                LOG_PBUS("nl bright blend land: cur=tgt=%u, pulse=%lu",
                    (unsigned)time_info.current_duty,
                    (unsigned long)time_info.pulse_width);
                return;
            }
            time_info.pulse_width = pwm_table[time_info.current_duty];
            time_info.duty_update_flag = 1;
            bsp_night_light_climb_schedule();
            return;
        }

        if (bsp_night_light_bright_end_blend_needed()) {
            uint8_t next_duty = time_info.current_duty + 1;

            time_info.blend_phase = 1;
            time_info.pulse_width =
                (pwm_table[time_info.current_duty] + pwm_table[next_duty]) / 2;
            time_info.duty_update_flag = 1;
            LOG_PBUS("nl bright blend: %u -> %u, tgt=%u",
                (unsigned)time_info.current_duty,
                (unsigned)next_duty,
                (unsigned)time_info.target_duty);
            bsp_night_light_climb_schedule();
            return;
        }
#endif
        time_info.blend_phase = 0;
        time_info.current_duty++;
        time_info.pulse_width = pwm_table[time_info.current_duty];
        time_info.duty_update_flag = 1;
        if (time_info.current_duty == time_info.target_duty) {
            return;
        }
        bsp_night_light_climb_schedule();
    } else {
#if NIGHT_LIGHT_LOG_DEBUG
        LOG_PBUS("time is over, current_duty: %d, target_duty: %d",
            time_info.current_duty, time_info.target_duty);
#endif
    }
}

static int8_t bsp_night_light_timer_register(void)
{
    time_info.time_id = hwtimer_alloc(bsp_night_light_timer_handler, NULL);
    if (time_info.time_id == NULL) {
        LOGE("register night light timer failed");
        return -1;
    }
    time_info.is_running = false;
    return 0;
}

static void night_light_level_set(uint8_t level)
{
    LOG_PBUS("nl bsp in: level=%u, onoff=%u, cur=%u, tgt=%u, pulse=%lu",
        (unsigned)level,
        (unsigned)time_info.onoff,
        (unsigned)time_info.current_duty,
        (unsigned)time_info.target_duty,
        (unsigned long)time_info.pulse_width);

    if (time_info.onoff == 0 && level > 0) {
        struct HAL_PWM_CFG_T pwm_cfg = {
            .freq = NIGHT_LIGHT_PERIOD,
            .ratio = 2,
            .inv = false,
            .sleep_on = false,
        };
        time_info.onoff = 1;
        time_info.current_duty = 2; /// 2 才亮灯
        time_info.pulse_width = pwm_table[time_info.current_duty];
        hal_pwm_enable(NIGHT_LIGHT_PWM_ID, &pwm_cfg);
        hal_pwm_update_pulse_width(NIGHT_LIGHT_PWM_ID, time_info.pulse_width, NIGHT_LIGHT_PERIOD);
        bsp_night_light_irq_control(true);
        LOG_PBUS("night light on");
        LOG_PBUS("nl bsp reinit: level=%u, cur->2, tgt pending", (unsigned)level);
    }

    uint8_t duty = level;

    if (duty > NIGHT_LIGHT_PWM_TABLE_INDEX_MAX) {
        duty = NIGHT_LIGHT_PWM_TABLE_INDEX_MAX;
    }
    if (duty < NIGHT_LIGHT_PWM_TABLE_INDEX_MIN)   // 防止target_duty小于2，因无法触发中断而无法更新PWM占空比
    {
        duty = NIGHT_LIGHT_PWM_TABLE_INDEX_MIN;
    }

    // if ((time_info.target_duty > (target_duty + 5)) || (time_info.target_duty < (target_duty - 5))) {
    if (time_info.current_duty != duty) {
        time_info.target_duty = duty;
        time_info.blend_phase = 0;
        bsp_night_light_climb_start();
        LOG_PBUS("nl bsp climb: level=%u, duty=%u, cur=%u->tgt=%u",
            (unsigned)level,
            (unsigned)duty,
            (unsigned)time_info.current_duty,
            (unsigned)time_info.target_duty);     
    }
    else {
        time_info.duty_update_flag = 0;
        time_info.target_duty = time_info.current_duty;
        LOG_PBUS("nl bsp noop: level=%u, duty=%u, cur=tgt=%u",
            (unsigned)level, (unsigned)duty, (unsigned)time_info.current_duty);  
    }
    // LOG_PBUS("nightlight target_duty: %d, current_duty: %d", time_info.target_duty, time_info.current_duty);
}
#else
static void night_light_level_set(uint8_t level)
{
    if (level == 0) {
        hal_pwm_disable(NIGHT_LIGHT_PWM_ID);
        return;
    } 
    uint8_t duty = level > 100 ? 100 : level;
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = NIGHT_LIGHT_PERIOD,
        .ratio = duty > 100 ? 100 : duty,
        .inv = false,
        .sleep_on = false,
    };
    hal_pwm_enable(NIGHT_LIGHT_PWM_ID, &pwm_cfg);
    LOG_PBUS("duty: %d, level: %d", duty, level);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_night_light_init(void)
{
    NIGHT_LIGHT_PWM_IMUX();
#if IS_SUPPORT_NIGHT_LIGHT_CLIMB
    if (bsp_night_light_timer_register() != 0) {
        return;
    }
    time_info.current_duty = 0;
    time_info.target_duty = 0;
    time_info.onoff = 0;
#endif
}

void bsp_night_light_resume(void)
{
    time_info.current_duty = 0;
    time_info.target_duty = 0;
    bsp_night_light_init();
}

void bsp_night_light_suspend(void)
{
    time_info.current_duty = 0;
    time_info.target_duty = 0;
    hal_pwm_disable(NIGHT_LIGHT_PWM_ID);
}

int bsp_night_light_status_set(led_status_msg_t *msg)
{
    EXCEPTION_CHECK_INT(msg);
    if (msg->led_index != NIGHT_LIGHT_LED) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    if (msg->led_status.mode == LED_MODE_LEVEL) {
        night_light_level_set(msg->led_status.un.luminous.level);
    } else if (msg->led_status.mode == LED_MODE_ONOFF) {
        night_light_level_set(msg->led_status.un.onoff.status > 0 ? LED_LEVEL_MAX / 2 : 0);
    }
    return 0;
}

int bsp_night_light_level_max_update(led_level_msg_t *msg)
{
    return 0;
}

#endif /* IS_SUPPORT_NIGHT_LIGHT */