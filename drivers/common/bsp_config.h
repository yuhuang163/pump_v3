#ifndef __BSP_CONFIG_H__
#define __BSP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "hal_iomux.h" // IWYU pragma: keep
#include "hal_gpio.h" // IWYU pragma: keep
#include "hal_gpadc.h" // IWYU pragma: keep

/// GPIO控制宏定义
#define GPIO_PIN_CTRL(pin, status)      ((status) ? hal_gpio_pin_set(pin) : hal_gpio_pin_clr(pin))

/// ADC相关定义
#define BATT_ADC_POWER_PIN              HAL_GPIO_PIN_P4_4
#define BATT_ADC_CHANNEL                HAL_GPADC_CHAN_2
#define NTC_ADC_CHANNEL                 HAL_GPADC_CHAN_4
/// 系统电源相关定义
#define SYS_POWER_PIN                   HAL_GPIO_PIN_P4_5
/// 电源按键相关定义
#define POWER_KEY_PIN                   HAL_GPIO_PIN_P4_6
/// 触摸按键相关定义
#define TOUCH_INT_PIN                   HAL_GPIO_PIN_P0_0
/// 启动与暂停背光灯
#define PUMP_START_BL_PIN               HAL_GPIO_PIN_P1_2  
/// 模式选择背光灯
#define PUMP_MODE_BL_PIN                HAL_GPIO_PIN_P1_3  
/// 频率选择背光灯
#define PUMP_FREQ_SET_BL_PIN            HAL_GPIO_PIN_P1_4  
/// 追奶模式背光灯
#define PUMP_BREASTFEEDING_BL_PIN       HAL_GPIO_PIN_P1_5  
/// 双边控制背光灯
#define PUMP_BILATERRAL_CONTROL_BL_PIN  HAL_GPIO_PIN_P1_6  
/// 左边控制背光灯
#define PUMP_LEFT_CONTROL_BL_PIN        HAL_GPIO_PIN_P1_7  
/// 右边控制背光灯
#define PUMP_RIGHT_CONTROL_BL_PIN       HAL_GPIO_PIN_P4_3 
/// 泵奶电源控制引脚
#define PUMP_POWER_PIN                  HAL_GPIO_PIN_P0_3
/// LCD电源控制引脚
#define LCD_POWER_PIN                   HAL_GPIO_PIN_P4_7
/// 充电中断引脚
#define CHARGE_INT_PIN                  HAL_GPIO_PIN_P0_1
/// 充电电源控制引脚
#define CHARGE_POWER_PIN                HAL_GPIO_PIN_P0_2
/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
