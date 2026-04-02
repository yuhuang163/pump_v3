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
#include "project_config.h" // IWYU pragma: keep
#include "project_support_overlay.h" // IWYU pragma: keep (for IS_SUPPORT_TOUCH_KEY_HYN)

/// GPIO控制宏定义
#define GPIO_PIN_CTRL(pin, status)      ((status) ? hal_gpio_pin_set(pin) : hal_gpio_pin_clr(pin))

#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
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
#if IS_SUPPORT_TOUCH_KEY_HYN
#define HYN_TOUCH_RST_PIN               HAL_GPIO_PIN_P2_0  /// 海栎创触摸IC复位脚，请根据实际硬件修改
#endif
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
/// 夜灯相关定义
#define NIGHT_LIGHT_PIN                 HAL_GPIO_PIN_P4_2  
#define NIGHT_LIGHT_PWM_ID              HAL_PWM_ID_2
#define NIGHT_LIGHT_PWM_IMUX            hal_iomux_set_pwm2
/// 泵奶电源控制引脚
#define PUMP_POWER_PIN                  HAL_GPIO_PIN_P0_3
/// LCD电源控制引脚
#define USR_LCD_POWER_PIN               HAL_GPIO_PIN_P4_7
/// I2C
#define I2C2_SCL_PIN                    HAL_GPIO_PIN_P0_4
#define I2C2_SDA_PIN                    HAL_GPIO_PIN_P0_5
#define I2C3_SCL_PIN                    HAL_GPIO_PIN_P0_6
#define I2C3_SDA_PIN                    HAL_GPIO_PIN_P0_7
#define I2C4_SCL_PIN                    HAL_GPIO_PIN_P1_0
#define I2C4_SDA_PIN                    HAL_GPIO_PIN_P1_1
#define AIR_PRESSURE_I2C_ID1            I2C_MASTER_ID_1
#define AIR_PRESSURE_I2C_ID2            I2C_MASTER_ID_2
#define CHARGE_I2C_ID                   I2C_MASTER_ID_0
#define TOUCH_KEY_I2C_ID                I2C_MASTER_ID_0
/// 充电中断引脚
#define CHARGE_INT_PIN                  HAL_GPIO_PIN_P0_1
/// 充电电源控制引脚
#define CHARGE_POWER_PIN                HAL_GPIO_PIN_P0_2
/// 马达PWM相关定义
#define PWM_PUMP1_PIN                   HAL_GPIO_PIN_P3_7
#define PWM_PUMP2_PIN                   HAL_GPIO_PIN_P4_1
#define PWM_VALVE1_PIN                  HAL_GPIO_PIN_P3_6
#define PWM_VALVE2_PIN                  HAL_GPIO_PIN_P4_0
#define MOTOR_PUMP1_PWM_ID              HAL_PWM1_ID_3
#define MOTOR_PUMP1_PWM_IMUX            hal_iomux_set_pwm7
#define MOTOR_PUMP2_PWM_ID              HAL_PWM_ID_1
#define MOTOR_PUMP2_PWM_IMUX            hal_iomux_set_pwm1
#define MOTOR_VALVE1_PWM_ID             HAL_PWM1_ID_2
#define MOTOR_VALVE1_PWM_IMUX           hal_iomux_set_pwm6
#define MOTOR_VALVE2_PWM_ID             HAL_PWM_ID_0
#define MOTOR_VALVE2_PWM_IMUX           hal_iomux_set_pwm0
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3) || (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V4)
/// ADC相关定义
#define BATT_ADC_CHANNEL                HAL_GPADC_CHAN_4
#define PUMP1_ADC_CHANNEL               HAL_GPADC_CHAN_2
#define PUMP2_ADC_CHANNEL               HAL_GPADC_CHAN_3
#define LIGHT_ADC_CHANNEL               HAL_GPADC_CHAN_4
/// LCD相关定义
#define USR_LCD_POWER_PIN               HAL_GPIO_PIN_P3_6
#define USR_LCD_RST_PIN                 HAL_GPIO_PIN_P4_1
#define USR_LCD_TE_PIN                  HAL_GPIO_PIN_P4_0
#define USR_LCD_CLK_PIN                 HAL_GPIO_PIN_P3_0
#define USR_LCD_CS_PIN                  HAL_GPIO_PIN_P3_1
#define USR_LCD_D0_PIN                  HAL_GPIO_PIN_P3_2
#define USR_LCD_D1_PIN                  HAL_GPIO_PIN_P3_3
#define USR_LCD_D2_PIN                  HAL_GPIO_PIN_P3_4
#define USR_LCD_D3_PIN                  HAL_GPIO_PIN_P3_5
#define LCD_LCD_BL_PIN                  HAL_GPIO_PIN_P3_6
#define LCD_LCD_BL_PWM_ID               HAL_PWM1_ID_2
#define LCD_LCD_BL_PWM_IMUX             hal_iomux_set_pwm6
/// nor flash相关定义
#define NOR_FLASH_CS_PIN                HAL_GPIO_PIN_P2_5
#define NOR_FLASH_CLK_PIN               HAL_GPIO_PIN_P2_4
#define NOR_FLASH_MISO_PIN              HAL_GPIO_PIN_P2_6
#define NOR_FLASH_MOSI_PIN              HAL_GPIO_PIN_P2_7
/// 泵阀相关定义
#define PWM_PUMP_EN_PIN                 HAL_GPIO_PIN_P1_5
#define PWM_PUMP1_PIN                   HAL_GPIO_PIN_P4_2
#define PWM_PUMP2_PIN                   HAL_GPIO_PIN_P3_7
#define PWM_VALVE1_PIN                  HAL_GPIO_PIN_P4_5
#define PWM_VALVE2_PIN                  HAL_GPIO_PIN_P4_4
#define MOTOR_PUMP1_PWM_ID              HAL_PWM_ID_2
#define MOTOR_PUMP1_PWM_IMUX            hal_iomux_set_pwm2
#define MOTOR_PUMP2_PWM_ID              HAL_PWM1_ID_3
#define MOTOR_PUMP2_PWM_IMUX            hal_iomux_set_pwm7
#define MOTOR_VALVE1_PWM_ID             HAL_PWM1_ID_1
#define MOTOR_VALVE1_PWM_IMUX           hal_iomux_set_pwm5
#define MOTOR_VALVE2_PWM_ID             HAL_PWM1_ID_0
#define MOTOR_VALVE2_PWM_IMUX           hal_iomux_set_pwm4
/// 按键相关定义
#define POWER_KEY_PIN                   HAL_GPIO_PIN_P1_7
#define TOUCH_INT_PIN                   HAL_GPIO_PIN_P4_6
#if IS_SUPPORT_TOUCH_KEY_RESET_PIN
#define HYN_TOUCH_RST_PIN               HAL_GPIO_PIN_P2_0  /// 海栎创触摸IC复位脚，请根据实际硬件修改
#endif
#define KNOB_B_PIN                      HAL_GPIO_PIN_P1_0
#define KNOB_A_PIN                      HAL_GPIO_PIN_P1_1
/// USB相关定义
#define CHARGE_PD_INT_PIN               HAL_GPIO_PIN_P1_6
/// 日志串口
#define LOG_UART_TX_PIN                 HAL_GPIO_PIN_P2_3
#define LOG_UART_RX_PIN                 HAL_GPIO_PIN_P2_2
/// 夜灯相关定义
#define NIGHT_LIGHT_PIN                 HAL_GPIO_PIN_P4_3
#define NIGHT_LIGHT_PWM_ID              HAL_PWM_ID_3
#define NIGHT_LIGHT_PWM_IMUX            hal_iomux_set_pwm3
/// I2C相关定义
#define I2C0_SCL_PIN                    HAL_GPIO_PIN_P0_0
#define I2C0_SDA_PIN                    HAL_GPIO_PIN_P0_1
#define I2C1_SCL_PIN                    HAL_GPIO_PIN_P0_2
#define I2C1_SDA_PIN                    HAL_GPIO_PIN_P0_3
#define I2C2_SCL_PIN                    HAL_GPIO_PIN_P0_4
#define I2C2_SDA_PIN                    HAL_GPIO_PIN_P0_5
#define I2C3_SCL_PIN                    HAL_GPIO_PIN_P0_6
#define I2C3_SDA_PIN                    HAL_GPIO_PIN_P0_7
#define LED_I2C_ID                      I2C_MASTER_ID_1
#define TOUCH_KEY_I2C_ID                I2C_MASTER_ID_1
#if 0
#define AIR_PRESSURE_I2C_ID1            I2C_MASTER_ID_0
#define AIR_PRESSURE_I2C_ID2            I2C_MASTER_ID_3
#else
#define AIR_PRESSURE_I2C_ID1            I2C_MASTER_ID_3
#define AIR_PRESSURE_I2C_ID2            I2C_MASTER_ID_0
#endif
#define CHARGE_I2C_ID                   I2C_MASTER_ID_1
#define CHARGE_PD_I2C_ID                I2C_MASTER_ID_2
/// 充电芯片相关定义
#define CHARGE_POWER_PIN                HAL_GPIO_PIN_P1_2
#define CHARGE_SLEEP_PIN                HAL_GPIO_PIN_P1_4
#define CHARGE_PD_I2C_CLK_PIN           HAL_GPIO_PIN_P0_4
#define CHARGE_PD_I2C_DATA_PIN          HAL_GPIO_PIN_P0_5
/// 版本识别定义
#define HW_VERSION_DETECT_PIN           HAL_GPIO_PIN_P1_5
/// 外设电源相关定义
#define PERIPHERAL_POWER_PIN            HAL_GPIO_PIN_P1_3
#else
#error "Unsupported hardware version"
#endif

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
