/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_led.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "bsp_common.h" 
#include "bsp_config.h"
#include "bsp_led_internal.h"
/*
* DEFINES
*****************************************************************************************
*/
#define LED_TOTAL_SIZE               (6)
#define LED_PWM_PERIOD               80  /// 每档 tick，约 80us；10 档 ≈ 800us 周期
#define LED_SOFT_PWM_STEPS          (10)
#define LED_THREAD_POLL_INTERVAL     (10)
#define LED_BREATH_PERIOD_DEFAULT     (10)
#define LED_BREATH_STEP_DEFAULT      (1)

typedef struct {
    enum HAL_GPIO_PIN_T pin;
} led_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static HWTIMER_ID hwt = NULL;
static uint8_t led_time_is_running = 0;
static led_status_t led_status[LED_TOTAL_SIZE];
static thread_context_t s_led_device_thd;
static os_timer_id s_led_timer_id;
static uint8_t pwm_phase = 0;
static uint32_t s_led_tick_count = 0;

static const led_cfg_t led_cfg[LED_TOTAL_SIZE] = {
    {PUMP_START_BL_PIN},
    {PUMP_MODE_BL_PIN},
    {PUMP_FREQ_SET_BL_PIN},
    {PUMP_BREASTFEEDING_BL_PIN},
    {PUMP_LEFT_CONTROL_BL_PIN},
    {PUMP_RIGHT_CONTROL_BL_PIN},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void led_timer_stop(void)
{
    if (hwt) {
        hwtimer_stop(hwt);
    }
    led_time_is_running = 0;
}

static void led_timer_start(uint32_t period)
{
    uint32_t period_ticks = US_TO_TICKS(period);
    hwtimer_start(hwt, period_ticks);
    led_time_is_running = 1;
}

static uint8_t led_pwm_level_get(uint8_t index)
{
    switch (led_status[index].mode) {
        case LED_MODE_LEVEL:
            return led_status[index].un.luminous.level;
        case LED_MODE_BREATH:
            return led_status[index].un.breath.current_level;
        default:
            return 0;
    }
}

static void led_timeout_handler(void *param)
{
    uint8_t level_refresh_flag = 0;

    (void)param;
    pwm_phase++;
    if (pwm_phase >= LED_SOFT_PWM_STEPS) {
        pwm_phase = 0;
    }
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        uint8_t level;
        uint8_t threshold;

        if (led_status[i].mode != LED_MODE_LEVEL &&
            led_status[i].mode != LED_MODE_BREATH) {
            continue;
        }
        level_refresh_flag = 1;
        level = led_pwm_level_get(i);
        threshold = (uint8_t)((level * LED_SOFT_PWM_STEPS + 50) / 100);
        GPIO_PIN_CTRL(led_cfg[i].pin, pwm_phase < threshold ? 1 : 0);
    }
    if (level_refresh_flag > 0) {
        led_timer_start(LED_PWM_PERIOD);
    } else {
        led_time_is_running = 0;
    }
}

static void led_timer_init(void)
{
    if (hwt == NULL) {
        hwt = hwtimer_alloc(led_timeout_handler, 0);
        if (hwt == NULL) {
            LOGE("Oops! hwtimer_alloc(led): fail");
        }
    }
}

static void led_timer_deinit(void)
{
    if (hwt) {
        hwtimer_stop(hwt);
        hwtimer_free(hwt);
        hwt = NULL;
    }
}

static void led_pin_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)PUMP_START_BL_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE
    };

    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        gpio_map.pin = (enum HAL_IOMUX_PIN_T)led_cfg[i].pin;
        hal_iomux_init(&gpio_map, 1);
        hal_gpio_pin_set_dir(led_cfg[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
}

static uint32_t get_led_tick(void)
{
    return s_led_tick_count;
}

static uint32_t get_led_tick_elapsed_ms(uint32_t start_time)
{
    if (s_led_tick_count < start_time) {
        return (s_led_tick_count + (0xFFFFFFFF - start_time)) * LED_THREAD_POLL_INTERVAL;
    }
    return (s_led_tick_count - start_time) * LED_THREAD_POLL_INTERVAL;
}

static uint8_t led_breath_peak_get(uint8_t index, const led_breath_t *br)
{
    uint8_t peak = br->pluse_max;

    if (peak == 0 || peak > led_status[index].level_max) {
        peak = led_status[index].level_max;
    }
    if (peak > LED_LEVEL_MAX) {
        peak = LED_LEVEL_MAX;
    }
    if (peak == 0) {
        peak = 1;
    }
    return peak;
}

static void led_pwm_kick(void)
{
    if (led_time_is_running == 0) {
        pwm_phase = 0;
        led_timer_start(LED_PWM_PERIOD);
    }
}

static void led_breath_finish(uint8_t index)
{
    led_breath_finish_cb cb = led_status[index].un.breath.cb;

    led_status[index].mode = LED_MODE_ONOFF;
    GPIO_PIN_CTRL(led_cfg[index].pin, 0);
    if (cb) {
        cb();
    }
}

static void led_breath_handle(uint8_t index)
{
    led_breath_t *br = &led_status[index].un.breath;
    uint8_t peak;
    uint16_t step;
    uint16_t period;
    uint8_t next_level;

    if (led_status[index].mode != LED_MODE_BREATH) {
        return;
    }
    if (br->continue_time != 0 &&
        get_led_tick_elapsed_ms(br->start_time) >= br->continue_time) {
        led_breath_finish(index);
        return;
    }

    period = br->period ? br->period : LED_BREATH_PERIOD_DEFAULT;
    if (get_led_tick_elapsed_ms(br->step_time) < period) {
        return;
    }
    br->step_time = get_led_tick();

    peak = led_breath_peak_get(index, br);
    step = br->step ? br->step : LED_BREATH_STEP_DEFAULT;
    if (br->direction != 2) {
        br->direction = 1;
    }
    if (br->current_level > peak) {
        br->current_level = peak;
    }

    if (br->direction == 1) {
        next_level = (uint8_t)(br->current_level + step);
        if (next_level >= peak || next_level < br->current_level) {
            next_level = peak;
            br->direction = 2;
        }
    } else {
        if (br->current_level <= step) {
            next_level = 0;
            br->direction = 1;
        } else {
            next_level = (uint8_t)(br->current_level - step);
        }
    }
    br->current_level = next_level;
    led_pwm_kick();
}

static void led_blink_handle(uint8_t index)
{
    if (led_status[index].mode != LED_MODE_BLINK) {
        return;
    }
    led_blink_t *led = &led_status[index].un.blink;
    /// 闪烁完成
    if (led->times_count >= led->times && led->times != 0) {
        led_status[index].mode = LED_MODE_ONOFF;
        if (led->finish_status != led->status) {
            GPIO_PIN_CTRL(led_cfg[index].pin, led->finish_status != LED_STATUS_ON ? 0 : 1);
        }
        return;
    }
    /// 闪烁计时
    led->start_time += LED_THREAD_POLL_INTERVAL;
    if (led->start_time >= led->period) {
        led->status = led->status > 0 ? 0 : 1;
        GPIO_PIN_CTRL(led_cfg[index].pin, led->status);
        led->start_time = 0;
        if (led->status == LED_STATUS_ON) {
            led->times_count++;
        }
    }
}

static void led_thread_timer_handle(void *arg)
{
    uint8_t msg = 0;
    s_led_tick_count++;
    MSG_QUEUE_SEND(s_led_device_thd.msg_id, (void *)&msg, 0);
}

static void led_device_thread(void *arg)
{
    uint8_t msg = 0;
    CREATE_MSG_TYPE_QUEUE(&s_led_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_led_timer_id, "led", NULL, OS_TIMER_PERIODIC, led_thread_timer_handle);
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
    s_led_tick_count = 0;
    while (1) {
        MSG_QUEUE_RECV(s_led_device_thd.msg_id, (void *)&msg, OS_WAIT_FOREVER);
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            switch (led_status[i].mode) {
                case LED_MODE_BLINK:
                    led_blink_handle(i);
                    break;
                case LED_MODE_BREATH:
                    led_breath_handle(i);
                    break;
                default:
                    break;
            }
        }
    }
}

static void led_onoff_set(uint8_t index, led_onoff_t onoff)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_ONOFF; 
            GPIO_PIN_CTRL(led_cfg[i].pin, onoff.status);
        }
        return;
    }
    led_status[index].mode = LED_MODE_ONOFF; 
    GPIO_PIN_CTRL(led_cfg[index].pin, onoff.status);
}

static void led_level_set(uint8_t index, led_luminous_t luminous)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            if (luminous.level == 0 || luminous.level >= LED_LEVEL_MAX) {
                led_status[i].mode = LED_MODE_ONOFF; 
                GPIO_PIN_CTRL(led_cfg[i].pin, luminous.level == 0 ? 0 : 1);
            } else {
                led_status[i].mode = LED_MODE_LEVEL; 
                led_status[i].un.luminous.level = luminous.level;
                led_pwm_kick();
            }
        }
        return;
    }

    if (luminous.level == 0 || luminous.level >= LED_LEVEL_MAX) {
        led_status[index].mode = LED_MODE_ONOFF; 
        GPIO_PIN_CTRL(led_cfg[index].pin, luminous.level == 0 ? 0 : 1);
    } else {
        led_status[index].mode = LED_MODE_LEVEL; 
        led_status[index].un.luminous.level = luminous.level;
        led_pwm_kick();
    }
}

static void led_blink_set(uint8_t index, led_blink_t blink)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].mode = LED_MODE_BLINK; 
            memcpy(&led_status[i].un.blink, &blink, sizeof(led_blink_t));
            GPIO_PIN_CTRL(led_cfg[i].pin, LED_STATUS_OFF);
            led_status[i].un.blink.start_time = 0;
            led_status[i].un.blink.status = LED_STATUS_OFF;
            led_status[i].un.blink.times_count = 0;
        }
        return;
    }

    led_status[index].mode = LED_MODE_BLINK; 
    memcpy(&led_status[index].un.blink, &blink, sizeof(led_blink_t));
    GPIO_PIN_CTRL(led_cfg[index].pin, LED_STATUS_OFF);
    led_status[index].un.blink.start_time = 0;
    led_status[index].un.blink.status = LED_STATUS_OFF;
    led_status[index].un.blink.times_count = 0;
}

static void led_breath_config_one(uint8_t index, const led_breath_t *src)
{
    led_breath_t *br = &led_status[index].un.breath;
    uint8_t peak;

    memcpy(br, src, sizeof(led_breath_t));
    if (br->period == 0) {
        br->period = LED_BREATH_PERIOD_DEFAULT;
    }
    if (br->step == 0) {
        br->step = LED_BREATH_STEP_DEFAULT;
    }
    peak = led_breath_peak_get(index, br);
    if (br->polarity) {
        br->current_level = peak;
        br->direction = 2;
    } else {
        br->current_level = 0;
        br->direction = 1;
    }
    br->start_time = get_led_tick();
    br->step_time = br->start_time;
    led_status[index].mode = LED_MODE_BREATH;
    GPIO_PIN_CTRL(led_cfg[index].pin, 0);
    led_pwm_kick();
}

static void led_breath_set(uint8_t index, led_breath_t breath)
{
    if (index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_breath_config_one(i, &breath);
        }
        return;
    }
    led_breath_config_one(index, &breath);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_led_init(void)
{
    led_timer_init();
    led_pin_init();
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        led_status[i].level_max = LED_LEVEL_MAX;
    }
    RUN_NEW_THREAD(&s_led_device_thd, "led", led_device_thread, RT_PRIO_LED_DEVICE_THREAD, (LED_DEVICE_THREAD_STACKSIZE));
    return BSP_OK;
}

void bsp_led_resume(void)
{
    led_timer_init();
    START_TIMER(s_led_timer_id, LED_THREAD_POLL_INTERVAL);
}

void bsp_led_suspend(void)
{
    STOP_TIMER(s_led_timer_id);
    led_timer_deinit();
}

int bsp_led_status_set(led_status_msg_t *msg)
{
    EXCEPTION_CHECK_INT(msg);
    if (msg->led_index >= LED_TOTAL_SIZE && msg->led_index != PUMP_LED_ALL_ID) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    switch (msg->led_status.mode) {
        case LED_MODE_ONOFF:
            led_onoff_set(msg->led_index, msg->led_status.un.onoff);
            break;

        case LED_MODE_BLINK:
            led_blink_set(msg->led_index, msg->led_status.un.blink);
            break;

        case LED_MODE_LEVEL:
            led_level_set(msg->led_index, msg->led_status.un.luminous);
        break;

        case LED_MODE_BREATH:
            led_breath_set(msg->led_index, msg->led_status.un.breath);
            break;
    }

    return BSP_OK;
}

int bsp_led_level_max_update(led_level_msg_t *msg)
{
    EXCEPTION_CHECK_INT(msg);
    if (msg->led_index >= LED_TOTAL_SIZE && msg->led_index != PUMP_LED_ALL_ID) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    if (msg->level_max > LED_LEVEL_MAX) {
        msg->level_max = LED_LEVEL_MAX;
    } else if (msg->level_max < 10) {
        msg->level_max = 10;
    }

    if (msg->led_index == PUMP_LED_ALL_ID) {
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
            led_status[i].level_max = msg->level_max;
        }
    } else {
        led_status[msg->led_index].level_max = msg->level_max;
    }
    return BSP_OK;
}

int bsp_led_is_ready(void *args)
{
    uint8_t *led_is_ready_status = (uint8_t *)args;
    *led_is_ready_status = 1;
    return BSP_OK;
}
