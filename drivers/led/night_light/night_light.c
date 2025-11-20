/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h"
#include "hal_iomux.h"
#include "hal_gpio.h" 
#include "bsp_led_internal.h"

/*
* DEFINES
*****************************************************************************************
*/
#define NIGHT_LIGHT_SIZE                          1
#define NIGHT_LIGHT_PIN                       HAL_GPIO_PIN_P4_2  // 右边控制背光灯
#define LED_PIN_CTRL(pin, status)             ((status) ? hal_gpio_pin_set(pin) : hal_gpio_pin_clr(pin))

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

static const night_light_cfg_t night_light_cfg[NIGHT_LIGHT_SIZE] = {
    {NIGHT_LIGHT_PIN, HAL_PWM_ID_2,  hal_iomux_set_pwm2, 1000},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void night_light_level_set(uint8_t index, led_luminous_t luminous)
{
    if (luminous.level == 0) {
        hal_pwm_disable(night_light_cfg[index].id);
        return;
    }
    
    uint8_t duty = luminous.level >= LED_LEVEL_MAX ? 100 : (luminous.level * 100 / LED_LEVEL_MAX);
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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_night_light_init(void)
{
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(night_light_cfg); i++) {
        night_light_cfg[i].iomux_set();
    }
}

void bsp_night_light_resume(void)
{
    // bsp_night_light_init();
}

void bsp_night_light_suspend(void)
{
    // struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
    //     (enum HAL_IOMUX_PIN_T)NIGHT_LIGHT_PIN,  
    //     HAL_IOMUX_FUNC_AS_GPIO, 
    //     HAL_IOMUX_PIN_VOLTAGE_VIO, 
    //     HAL_IOMUX_PIN_PULLUP_ENABLE
    // };

    // for (int i = 0; i < ARRAY_SIZE(night_light_cfg); i++) {
    //     hal_pwm_disable(night_light_cfg[i].id);
    //     gpio_map.pin = (enum HAL_IOMUX_PIN_T)night_light_cfg[i].pin;
    //     hal_iomux_init(&gpio_map, 1);
    //     hal_gpio_pin_set_dir(night_light_cfg[i].pin, HAL_GPIO_DIR_OUT, 0);
    // }
}

int bsp_night_light_status_set(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    if (tmp == NULL) {
        LOGE("tmp is NULL");
        return BSP_ERROR;
    }
    
    uint8_t index = tmp[0];
    if (index >= NIGHT_LIGHT_SIZE) {
        LOGE("index is out of range: %d", index);
        return BSP_ERROR;
    }

    led_status_t *led = (led_status_t *)tmp[1];
    if (led == NULL) {
        LOGE("led is NULL");
        return BSP_ERROR;
    }

    LOG_PBUS("index: %d, mode: %d, level: %d", index, led->mode, led->un.luminous.level);
    if (led->mode == LED_MODE_LEVEL) {
        night_light_level_set(index, led->un.luminous);
    }

    return 0;
}

void *bsp_night_light_status_get(void *args)
{
    uint32_t *tmp = (uint32_t *)args;
    
    if (!tmp) {
        return NULL;
    }

    if (tmp[0] >= NIGHT_LIGHT_SIZE) {
        return NULL;
    }
    
    return (void *)&led_status[tmp[0]];
}

