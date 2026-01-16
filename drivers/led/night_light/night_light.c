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

/*
* DEFINES
*****************************************************************************************
*/
#define NIGHT_LIGHT_SIZE                          1
#define IS_SUPPORT_NIGHT_LIGHT_SUSPEND            (0)
#define NIGHT_LIGHT_PERIOD                        1000

typedef struct {
    enum HAL_GPIO_PIN_T pin;
    enum HAL_PWM_ID_T id;
    void (*iomux_set)(void);
    uint16_t freq;
} night_light_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static led_status_t led_status[NIGHT_LIGHT_SIZE];

#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
static const night_light_cfg_t night_light_cfg[NIGHT_LIGHT_SIZE] = {
    {NIGHT_LIGHT_PIN, NIGHT_LIGHT_PWM_ID, NIGHT_LIGHT_PWM_IMUX, 1000},
};
uint8_t night_light_pwm_duty[LED_LEVEL_MAX + 1] = {0,  10, 20, 30, 40, 50,
    60, 70, 80, 90, 100};
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1)
static HWTIMER_ID night_light_hwt = NULL;
static uint8_t night_light_time_is_running = 0;
static uint8_t night_light_duty = 0;
static uint8_t night_light_count = 0;
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
static const night_light_cfg_t night_light_cfg[NIGHT_LIGHT_SIZE] = {
    {NIGHT_LIGHT_PIN, NIGHT_LIGHT_PWM_ID, NIGHT_LIGHT_PWM_IMUX, 1000},
};
uint8_t night_light_pwm_duty[LED_LEVEL_MAX + 1] = {0,  10, 20, 30, 40, 50,
    60, 70, 80, 90, 100};
#else
#error "Unsupported hardware version"
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
static void night_light_level_set(uint8_t index, led_luminous_t luminous)
{
    if (luminous.level == 0) {
        hal_pwm_disable(night_light_cfg[index].id);
        return;
    }
    
    //uint8_t duty = luminous.level >= LED_LEVEL_MAX ? 100 : (luminous.level * 100 / LED_LEVEL_MAX);
    uint8_t duty = night_light_pwm_duty[luminous.level];
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = night_light_cfg[index].freq,
        .ratio = duty,
        .inv = false,
        .sleep_on = false,
    };
    led_status[index].mode = LED_MODE_LEVEL;
    hal_pwm_enable(night_light_cfg[index].id, &pwm_cfg);
    LOG_PBUS("index: %d, duty: %d", index, duty);
}
#else
static void night_light_timer_stop(void)
{
    if (night_light_hwt) {
        hwtimer_stop(night_light_hwt);
    }
    night_light_time_is_running = 0;
    night_light_count = 0;
}

static void night_light_timer_start(uint32_t period)
{
    uint32_t period_ticks = US_TO_TICKS(period);
    if (night_light_hwt) {
        hwtimer_start(night_light_hwt, period_ticks);
    }
    night_light_time_is_running = 1;
}

static void night_light_timeout_handler(void *param)
{
    if (night_light_count == 0 || night_light_count >= LED_LEVEL_MAX) {
        night_light_count = 0;
        hal_gpio_pin_set_dir(NIGHT_LIGHT_PIN, HAL_GPIO_DIR_OUT, 1);
    } else if (night_light_count == night_light_duty) {
        hal_gpio_pin_set_dir(NIGHT_LIGHT_PIN, HAL_GPIO_DIR_OUT, 0);
    }
    night_light_count++;
    night_light_timer_start(NIGHT_LIGHT_PERIOD);
}

static void night_light_level_set(uint8_t index, led_luminous_t luminous)
{
    if (luminous.level == 0) {
        hal_gpio_pin_set_dir(NIGHT_LIGHT_PIN, HAL_GPIO_DIR_OUT, 0);
        night_light_timer_stop();
        return;
    }
    if (luminous.level >= LED_LEVEL_MAX) {
        hal_gpio_pin_set_dir(NIGHT_LIGHT_PIN, HAL_GPIO_DIR_OUT, 1);
        night_light_timer_stop();
        return;
    }
    night_light_duty = luminous.level;
    night_light_timer_start(NIGHT_LIGHT_PERIOD);
}

static void night_light_timer_init(void)
{
    if (night_light_hwt == NULL) {
        night_light_hwt = hwtimer_alloc(night_light_timeout_handler, 0);
    }
    LOG_PBUS("night_light_timer_init done");
}

static void led_timer_deinit(void)
{
    if (night_light_hwt) {
        hwtimer_stop(night_light_hwt);
        hwtimer_free(night_light_hwt);
        night_light_hwt = NULL;
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_night_light_init(void)
{
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(night_light_cfg); i++) {
        night_light_cfg[i].iomux_set();
    }
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1)
    night_light_timer_init();
#else
    #error "Unsupported hardware version"
#endif
}

void bsp_night_light_resume(void)
{
#if IS_SUPPORT_NIGHT_LIGHT_SUSPEND
    bsp_night_light_init();
#endif
}

void bsp_night_light_suspend(void)
{
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    #if IS_SUPPORT_NIGHT_LIGHT_SUSPEND
        struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
            (enum HAL_IOMUX_PIN_T)NIGHT_LIGHT_PIN,  
            HAL_IOMUX_FUNC_AS_GPIO, 
            HAL_IOMUX_PIN_VOLTAGE_VIO, 
            HAL_IOMUX_PIN_PULLUP_ENABLE
        };

        for (int i = 0; i < ARRAY_SIZE(night_light_cfg); i++) {
            hal_pwm_disable(night_light_cfg[i].id);
            gpio_map.pin = (enum HAL_IOMUX_PIN_T)night_light_cfg[i].pin;
            hal_iomux_init(&gpio_map, 1);
            hal_gpio_pin_set_dir(night_light_cfg[i].pin, HAL_GPIO_DIR_OUT, 0);
        }
    #endif
#endif
}

int bsp_night_light_status_set(void *args)
{
    led_status_msg_t *msg = (led_status_msg_t *)args;
    if (msg == NULL) {
        LOGE("msg is NULL");
        return BSP_ERROR;
    }
    
    if (msg->led_index >= NIGHT_LIGHT_SIZE) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }

    LOG_PBUS("index: %d, mode: %d, level: %d", msg->led_index, msg->led_status.mode, msg->led_status.un.luminous.level);
    if (msg->led_status.mode == LED_MODE_LEVEL) {
        night_light_level_set(msg->led_index, msg->led_status.un.luminous);
    }

    return 0;
}

int bsp_night_light_pwm_duty_set(void *args)
{
    night_light_pwm_duty_set_msg_t *msg = (night_light_pwm_duty_set_msg_t *)args;
    if (msg == NULL) {
        LOGE("msg is NULL");
        return BSP_ERROR;
    }
    
    if (msg->led_index >= NIGHT_LIGHT_SIZE) {
        LOGE("index is out of range: %d", msg->led_index);
        return BSP_ERROR;
    }
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
    if (msg->pwm_duty == 0) {
        hal_pwm_disable(night_light_cfg[msg->led_index].id);
        return BSP_OK;
    }
    
    //uint8_t duty = luminous.level >= LED_LEVEL_MAX ? 100 : (luminous.level * 100 / LED_LEVEL_MAX);
    uint8_t duty = msg->pwm_duty;
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = night_light_cfg[msg->led_index].freq,
        .ratio = duty,
        .inv = false,
        .sleep_on = false,
    };
    led_status[msg->led_index].mode = LED_MODE_LEVEL;
    hal_pwm_enable(night_light_cfg[msg->led_index].id, &pwm_cfg);
    LOG_PBUS("index: %d, duty: %d", msg->led_index, duty);
    return BSP_OK;
#else
    LOG_PBUS("index: %d, pwm_duty: %d", msg->led_index, msg->pwm_duty);
    return BSP_OK;
#endif
}

int bsp_night_light_test(void)
{
    static uint8_t level = 0;
    level++;
    if (level > 10) {
        level = 0;
    }

    led_status_msg_t night_light_msg = {
        .led_index = 0,
        .led_status = {
            .mode = LED_MODE_LEVEL,
            .un.luminous = {
                .level = level,
            },
        },
    };
    return bsp_night_light_status_set(&night_light_msg);
}