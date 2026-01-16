/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_key_internal.h" // IWYU pragma: keep
#include "bsp_knob.h" // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define KNOB_DIR_INVERT                     0     /// 旋钮方向：0-顺时针，1-逆时针
#define IS_SUPPORT_KNOB_LOG                 0   /// 旋钮日志打印开关

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_knob_left_count = 0;
static uint16_t s_knob_right_count = 0;
static uint32_t s_knob_a_trigger_time = 0;
static uint32_t s_knob_b_trigger_time = 0;
static uint32_t s_knob_offset = 0;
static uint8_t s_knob_direction = 0;
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
POSSIBLY_UNUSED static void knob_a_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    (void)pin;
    s_knob_a_trigger_time = get_timestamp();
#if IS_SUPPORT_KNOB_LOG
    PRINT_LOG("a time:%d, a:%d, b:%d\r\n", s_knob_a_trigger_time, hal_gpio_pin_get_val(KNOB_A_PIN), hal_gpio_pin_get_val(KNOB_B_PIN));
#endif
    if (hal_gpio_pin_get_val(KNOB_B_PIN) == 0) {
        s_knob_direction = 1; /// 顺时针
    } else {
        s_knob_direction = 2; /// 逆时针
    }
}

POSSIBLY_UNUSED static void knob_b_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    s_knob_b_trigger_time = get_timestamp();
#if IS_SUPPORT_KNOB_LOG
    PRINT_LOG("b time:%d, a:%d, b:%d\r\n", s_knob_b_trigger_time, hal_gpio_pin_get_val(KNOB_A_PIN), hal_gpio_pin_get_val(KNOB_B_PIN));
#endif
    if (hal_gpio_pin_get_val(KNOB_A_PIN) == 0 && s_knob_direction == 1) {
        s_knob_offset = s_knob_b_trigger_time - s_knob_a_trigger_time;
#if IS_SUPPORT_KNOB_LOG
        PRINT_LOG("offset: %d\r\n", s_knob_offset);
#endif
        s_knob_left_count++;
        bsp_irq_msg_send(INT_NUM_KNOB_RIGHT);    /// 顺时针
        s_knob_direction = 0;
    } else if (hal_gpio_pin_get_val(KNOB_A_PIN) == 1 && s_knob_direction == 2) {
        s_knob_offset = s_knob_b_trigger_time - s_knob_a_trigger_time;
#if IS_SUPPORT_KNOB_LOG
        PRINT_LOG("offset: %d\r\n", s_knob_offset);
#endif
        s_knob_right_count++;
        bsp_irq_msg_send(INT_NUM_KNOB_LEFT);    /// 逆时针
        s_knob_direction = 0;
    } else {
        s_knob_offset = s_knob_b_trigger_time - s_knob_a_trigger_time;
    #if IS_SUPPORT_KNOB_LOG
        PRINT_LOG("knob direction: %d, offset: %d\r\n", s_knob_direction, s_knob_offset);
    #endif
        s_knob_direction = 0;
    }
}

POSSIBLY_UNUSED static void knob_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T cfg = {
        enable,
        1,
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        HAL_GPIO_IRQ_POLARITY_HIGH_RISING,  
        knob_a_irq_callback
    };
    cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    hal_gpio_setup_irq(KNOB_A_PIN, &cfg);  
    cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    cfg.irq_handler = knob_b_irq_callback;
    hal_gpio_setup_irq(KNOB_B_PIN, &cfg);
    LOG_PBUS("knob irq control: %d, a: %d, b: %d", enable, hal_gpio_pin_get_val(KNOB_A_PIN), hal_gpio_pin_get_val(KNOB_B_PIN));
}

POSSIBLY_UNUSED static void knob_irq_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)KNOB_A_PIN,  
        HAL_IOMUX_FUNC_AS_GPIO, 
        HAL_IOMUX_PIN_VOLTAGE_VIO, 
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(KNOB_A_PIN,  HAL_GPIO_DIR_IN,  1);
    gpio_map.pin = (enum HAL_IOMUX_PIN_T)KNOB_B_PIN;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(KNOB_B_PIN,  HAL_GPIO_DIR_IN,  1); 
    LOG_PBUS("knob irq init");
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_knob_init(void)
{
    knob_irq_init();
    knob_irq_control(true);
}

uint16_t bsp_knob_get_count(uint8_t index)
{
    if (index == 0) {
        return s_knob_left_count;
    } else {
        return s_knob_right_count;
    }
}
