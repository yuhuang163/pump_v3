/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "bsp_common.h" 
#include "bsp_led_internal.h"
#include <stdint.h>

/*
* DEFINES
*****************************************************************************************
*/
#define LED_PWM_SIZE                          2
#define LED_SOFT_PWM_SIZE                     4
#define LED_TOTAL_SIZE                        (LED_PWM_SIZE + LED_SOFT_PWM_SIZE)
#define LED_SOFT_PWM_PERIOD                   800 // 800 - 1ms  350 - 0.5ms  120 - 0.25ms

#define LED1_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_2
#define LED2_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_3
#define LED3_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_4
#define LED4_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_5
#define LED5_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_6
#define LED6_SOFT_PWM_PIN                     HAL_GPIO_PIN_P1_7

#define LED_PIN_CTRL(pin, status)             ((status) ? hal_gpio_pin_set(pin) : hal_gpio_pin_clr(pin))

typedef struct {
    enum HAL_PWM_ID_T id;
    void (*iomux_set)(void);
    uint16_t freq;
} led_pwm_cfg_t;

typedef struct {
    enum HAL_GPIO_PIN_T pin;
} led_soft_pwm_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static HWTIMER_ID hwt = NULL;
static led_status_t led_status[LED_TOTAL_SIZE];
static thread_context_t s_led_device_thd;
static os_timer_id s_led_timer_id;

static const led_pwm_cfg_t led_pwm_cfg[LED_PWM_SIZE] = {
    {HAL_PWM_ID_2,  hal_iomux_set_pwm2, 1000},
    {HAL_PWM_ID_3,  hal_iomux_set_pwm3, 1000},
};

static const led_soft_pwm_cfg_t led_soft_pwm_cfg[LED_SOFT_PWM_SIZE] = {
    {LED1_SOFT_PWM_PIN},
    {LED2_SOFT_PWM_PIN},
    {LED3_SOFT_PWM_PIN},
    {LED4_SOFT_PWM_PIN},
};

static uint8_t is_soft_pwm[LED_TOTAL_SIZE] = {0};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void led_soft_pwm_timer_stop(void)
{
    if (hwt) {
        hwtimer_stop(hwt);
    }
}
static void led_soft_pwm_timer_start(uint32_t period)
{
    uint32_t period_ticks = US_TO_TICKS(period);

    hwtimer_start(hwt, period_ticks);
}

static bool led_soft_pwm_time_is_continue(void)
{
    for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        if (is_soft_pwm[i] == 0) {
            continue;
        }

        if (led_status[i].mode == LED_MODE_LEVEL) {
            return true;
        }
    }
    return false;
}

static void led_soft_pwm_timeout_handler(void *param)
{
    for (uint8_t i = 0; i < LED_SOFT_PWM_SIZE; i++) {
        if (is_soft_pwm[i] == 0) {
            continue;
        }
        led_status[i].un.onoff.status = !led_status[i].un.onoff.status;
        LED_PIN_CTRL(led_soft_pwm_cfg[i].pin, led_status[i].un.onoff.status > 0 ? 1 : 0);
    }

    if (led_soft_pwm_time_is_continue()) {
        led_soft_pwm_timer_start(LED_SOFT_PWM_PERIOD);
    }
}

static void led_soft_pwm_timer_init(void)
{
    if (hwt == NULL) {
        hwt = hwtimer_alloc(led_soft_pwm_timeout_handler, 0);
    }
}

static void led_soft_pwm_timer_deinit(void)
{
    if (hwt) {
        hwtimer_stop(hwt);
        hwtimer_free(hwt);
        hwt = NULL;
    }
}

static void led_soft_pwm_pin_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)LED1_SOFT_PWM_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE
    };

    for (uint8_t i = 0; i < LED_SOFT_PWM_SIZE; i++) {
        gpio_map.pin = (enum HAL_IOMUX_PIN_T)led_soft_pwm_cfg[i].pin;
        hal_iomux_init(&gpio_map, 1);
        hal_gpio_pin_set_dir(led_soft_pwm_cfg[i].pin, HAL_GPIO_DIR_OUT, 1);
        LED_PIN_CTRL(led_soft_pwm_cfg[i].pin, 0);
    }
}

static void led_soft_pwm_pin_deinit(void)
{

}

static void led_soft_pwm_init(void)
{
    led_soft_pwm_timer_init();
    led_soft_pwm_pin_init();
}

static void led_soft_pwm_deinit(void)
{
    led_soft_pwm_timer_deinit();
    led_soft_pwm_pin_deinit();
}

static void led_pwm_init(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(led_pwm_cfg); i++) {
        led_pwm_cfg[i].iomux_set();
    }
}

static void led_pwm_deinit(void)
{
    for (int i = 0; i < ARRAY_SIZE(led_pwm_cfg); i++) {
        hal_pwm_disable(led_pwm_cfg[i].id);
    }
}

static void led_soft_pwm_level_set(uint8_t index, uint8_t level)
{
    if (index >= LED_SOFT_PWM_SIZE) {
        return;
    }
}

static void led_soft_pwm_blink_set(uint8_t index, uint8_t mode)
{
    if (index >= LED_SOFT_PWM_SIZE) {
        return;
    }
}

static void led_soft_pwm_blink_handle(void)
{

}

static void led_pwm_level_set(uint8_t index, uint8_t level)
{
    uint8_t duty = (level % 11) * 10;

    if (index >= LED_PWM_SIZE) {
        return;
    }

    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = led_pwm_cfg[index].freq,
        .ratio = duty,
        .inv = false,
        .sleep_on = false,
    };
    hal_pwm_enable(led_pwm_cfg[index].id, &pwm_cfg);
}

static void led_pwm_blink_set(uint8_t index, uint8_t mode)
{
    if (index >= LED_PWM_SIZE) {
        return;
    }
}

static void led_pwm_blink_handle(void)
{

}

static void led_thread_timer_handle(void)
{
    uint8_t msg = 0;
    MSG_QUEUE_SEND(s_led_device_thd.msg_id, (void *)&msg, 0);
}

static void led_device_thread(void *arg)
{
    uint8_t msg = 0;
    CREATE_MSG_TYPE_QUEUE(&s_led_device_thd.msg_id, 1, 1);
    CREATE_TIMER(&s_led_timer_id, "led", NULL, OS_TIMER_ONCE, led_thread_timer_handle);
    while (1) {
        MSG_QUEUE_RECV(s_led_device_thd.msg_id, (void *)&msg, OS_WAIT_FOREVER);
        for (uint8_t i = 0; i < LED_TOTAL_SIZE; i++) {
        }
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int bsp_led_init(void)
{
    led_pwm_init();
    led_soft_pwm_init();
    RUN_NEW_THREAD(&s_led_device_thd, "led", led_device_thread, RT_PRIO_LED_DEVICE_THREAD, (LED_DEVICE_THREAD_STACKSIZE));
    return BSP_OK;
}

int bsp_led_deinit(void)
{
    led_soft_pwm_deinit();
    led_pwm_deinit();
    return 0;
}

int bsp_led_status_set(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    uint8_t index = tmp[0];
    uint8_t mode = tmp[1];
    if (index >= LED_TOTAL_SIZE) {
        return BSP_ERROR;
    }
    led_status[index].mode = mode;
    // todo: 根据模式设置led状态

    return BSP_OK;
}

void *bsp_led_status_get(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    
    if (!tmp) {
        return NULL;
    }

    if (tmp[0] >= LED_TOTAL_SIZE) {
        return NULL;
    }
    
    return (void *)&led_status[tmp[0]];
}