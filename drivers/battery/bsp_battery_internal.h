#ifndef __BSP_BATTARY_INTERNAL_H__
#define __BSP_BATTARY_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "bsp_internal.h" // IWYU pragma: keep

/**@brief Firmware information define*/

/// 单节电池充电曲线参数
#define SG_CHARGING_FORCE_UPDATE_THRESHOLD   300     ///单位mV  偏差超阈值强制更新
#define SG_CHARGING_UPDATE_THRESHOLD_1       240     ///单位mV  偏差阈值1
#define SG_CHARGING_UPDATE_THRESHOLD_2       80      ///单位mV  偏差阈值2
#define SG_CHARGING_UPDATE_THRESHOLD_3       0       ///单位mV  偏差阈值2
#define SG_CHARGING_DEBOUNCE_RATE_1          1000.0  ///去抖率  
#define SG_CHARGING_DEBOUNCE_RATE_2          0.004   ///去抖率  
#define SG_CHARGING_DEBOUNCE_RATE_3          0.001   ///去抖率 

/// 单节电池放电曲线参数
#define SG_UNCHARGE_FORCE_UPDATE_THRESHOLD   300     ///单位mV  偏差超阈值强制更新
#define SG_UNCHARGE_UPDATE_THRESHOLD_0       (-240)  ///单位mV  偏差阈值0
#define SG_UNCHARGE_UPDATE_THRESHOLD_1       (-180)  ///单位mV  偏差阈值1
#define SG_UNCHARGE_UPDATE_THRESHOLD_2       (-100)  ///单位mV  偏差阈值2
#define SG_UNCHARGE_UPDATE_THRESHOLD_3       (-50)   ///单位mV  偏差阈值3
#define SG_UNCHARGE_UPDATE_THRESHOLD_4       (0)     ///单位mV  偏差阈值4
#define SG_UNCHARGE_DEBOUNCE_RATE_0          (-0.6)  ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_1          800.0   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_2          500.0   ///去抖率 
#define SG_UNCHARGE_DEBOUNCE_RATE_3          200.0   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_4          120.0   ///去抖率  

/// 单节电池大电流放电曲线参数
#define SG_UNCHARGE_UPDATE_THRESHOLD_5       (-350)  ///单位mV  偏差阈值5
#define SG_UNCHARGE_UPDATE_THRESHOLD_6       (-200)  ///单位mV  偏差阈值6
#define SG_UNCHARGE_UPDATE_THRESHOLD_7       (-120)  ///单位mV  偏差阈值7
#define SG_UNCHARGE_UPDATE_THRESHOLD_8       (-40)   ///单位mV  偏差阈值8
#define SG_UNCHARGE_DEBOUNCE_RATE_5          6000.0  ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_6          (-0.001)   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_7          (-0.0005)   ///去抖率 
#define SG_UNCHARGE_DEBOUNCE_RATE_8          (-0.0001)   ///去抖率  

/// 去抖配置
#define BATT_FORCE_UPDATE_DEBOUNCE           5       /// 强制更新去抖
#define BATT_CHARGE_FULL_DEBOUNCE            100     

/// 健康状态配置
#define BATT_CHARGING_TIME_OUT               (162000)   ///(4.5 * 60 * 60 * 1000 / 100)
#define BATT_HEALTH_COEFF_MIN                (90)
#define BATT_HEALTH_COEFF_MAX                (100)

///电池容量属性
typedef struct {
    uint8_t  percent;           ///百分比
    uint16_t vol_min;           ///最小电压
    uint16_t vol_max;           ///最大电压
} capacity_data_t;

///电池状态
typedef struct {
    uint8_t  health;
    uint8_t  charge;
    uint8_t  percent;
    uint8_t  type;
    uint16_t voltage;
} battery_status_t;

/// 电池曲线
typedef struct {
    float rate;
    int16_t dropout_voltage;
} battery_para_t;

/// adc 参数
typedef struct {
    float coeff;
    uint16_t  voltage_min;
    uint16_t  voltage_max;
} battery_adc_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
bool vbus_is_plug_in(void);
int get_charge_state(void);
int get_charge_full_debounce(void);
void add_charge_irq_callback(void (*cb)(void *param));

int battery_adc_init(void);
int battery_adc_resume(void);
int battery_adc_suspend(void);
int battery_adc_single_conversion(uint16_t *batt_voltage);
int bsp_motor_is_running(void);
#ifdef __cplusplus
}
#endif

#endif 
