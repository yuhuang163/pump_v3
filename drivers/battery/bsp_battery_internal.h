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
#include "hal_battery.h" // IWYU pragma: keep

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
#define SG_UNCHARGE_FORCE_UPDATE_THRESHOLD   300    ///单位mV  偏差超阈值强制更新
#define SG_UNCHARGE_UPDATE_THRESHOLD_0       (240)  ///单位mV  偏差阈值0
#define SG_UNCHARGE_UPDATE_THRESHOLD_1       (180)  ///单位mV  偏差阈值1
#define SG_UNCHARGE_UPDATE_THRESHOLD_2       (100)  ///单位mV  偏差阈值2
#define SG_UNCHARGE_UPDATE_THRESHOLD_3       (50)   ///单位mV  偏差阈值3
#define SG_UNCHARGE_UPDATE_THRESHOLD_4       (0)    ///单位mV  偏差阈值4
#define SG_UNCHARGE_DEBOUNCE_RATE_0          (0.6)   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_1          800.0   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_2          500.0   ///去抖率 
#define SG_UNCHARGE_DEBOUNCE_RATE_3          200.0   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_4          120.0   ///去抖率  

/// 单节电池大电流放电曲线参数
#define SG_UNCHARGE_UPDATE_THRESHOLD_5       (350)  ///单位mV  偏差阈值5
#define SG_UNCHARGE_UPDATE_THRESHOLD_6       (200)  ///单位mV  偏差阈值6
#define SG_UNCHARGE_UPDATE_THRESHOLD_7       (120)  ///单位mV  偏差阈值7
#define SG_UNCHARGE_UPDATE_THRESHOLD_8       (40)   ///单位mV  偏差阈值8
#define SG_UNCHARGE_DEBOUNCE_RATE_5          6000.0    ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_6          (0.001)   ///去抖率  
#define SG_UNCHARGE_DEBOUNCE_RATE_7          (0.0005)  ///去抖率 
#define SG_UNCHARGE_DEBOUNCE_RATE_8          (0.0001)  ///去抖率 
/// 单节电池充电浮压调整阈值
#define SG_FLOAT_VOLTAGE_THRESHOLD_1         (4000)  ///单位mV  单节电池充电浮压调整阈值
#define SG_FLOAT_VOLTAGE_THRESHOLD_2         (3800)  ///单位mV  单节电池充电浮压调整阈值
#define SG_FLOAT_VOLTAGE_THRESHOLD_3         (3000)  ///单位mV  单节电池充电浮压调整阈值
#define SG_FLOAT_VOLTAGE_RATE_0              80      /// 单节电池充电浮压调整阈值 偏差  
#define SG_FLOAT_VOLTAGE_RATE_1              220     /// 单节电池充电浮压调整阈值 偏差  
#define SG_FLOAT_VOLTAGE_RATE_2              100     /// 单节电池充电浮压调整阈值 偏差 

/// 三节电池充电曲线参数
#define THREE_CHARGING_FORCE_UPDATE_THRESHOLD   460     ///单位mV  偏差超阈值强制更新
#define THREE_CHARGING_UPDATE_THRESHOLD_1       350     ///单位mV  偏差阈值1
#define THREE_CHARGING_UPDATE_THRESHOLD_2       100     ///单位mV  偏差阈值2
#define THREE_CHARGING_UPDATE_THRESHOLD_3       0       ///单位mV  偏差阈值2
#define THREE_CHARGING_DEBOUNCE_RATE_1          1500.0  ///去抖率  
#define THREE_CHARGING_DEBOUNCE_RATE_2          0.004   ///去抖率  
#define THREE_CHARGING_DEBOUNCE_RATE_3          0.001   ///去抖率
/// 三节电池放电曲线参数
#define THREE_UNCHARGE_FORCE_UPDATE_THRESHOLD   300    ///单位mV  偏差超阈值强制更新
#define THREE_UNCHARGE_UPDATE_THRESHOLD_0       (240)  ///单位mV  偏差阈值0
#define THREE_UNCHARGE_UPDATE_THRESHOLD_1       (180)  ///单位mV  偏差阈值1
#define THREE_UNCHARGE_UPDATE_THRESHOLD_2       (100)  ///单位mV  偏差阈值2
#define THREE_UNCHARGE_UPDATE_THRESHOLD_3       (50)   ///单位mV  偏差阈值3
#define THREE_UNCHARGE_UPDATE_THRESHOLD_4       (0)    ///单位mV  偏差阈值4
#define THREE_UNCHARGE_DEBOUNCE_RATE_0          (0.6)   ///去抖率  
#define THREE_UNCHARGE_DEBOUNCE_RATE_1          800.0   ///去抖率  
#define THREE_UNCHARGE_DEBOUNCE_RATE_2          500.0   ///去抖率 
#define THREE_UNCHARGE_DEBOUNCE_RATE_3          200.0   ///去抖率  
#define THREE_UNCHARGE_DEBOUNCE_RATE_4          120.0   ///去抖率  
/// 三节电池大电流放电曲线参数
#define THREE_UNCHARGE_UPDATE_THRESHOLD_5       (700)  ///单位mV  偏差阈值5
#define THREE_UNCHARGE_UPDATE_THRESHOLD_6       (400)  ///单位mV  偏差阈值6
#define THREE_UNCHARGE_UPDATE_THRESHOLD_7       (200)  ///单位mV  偏差阈值7
#define THREE_UNCHARGE_UPDATE_THRESHOLD_8       (120)   ///单位mV  偏差阈值8
#define THREE_UNCHARGE_UPDATE_THRESHOLD_9       (40)   ///单位mV  偏差阈值8
#define THREE_UNCHARGE_DEBOUNCE_RATE_5          (20000.0)  
#define THREE_UNCHARGE_DEBOUNCE_RATE_6          (0.007)   ///去抖率  
#define THREE_UNCHARGE_DEBOUNCE_RATE_7          (0.001)  ///去抖率 
#define THREE_UNCHARGE_DEBOUNCE_RATE_8          (0.0005)  ///去抖率  
#define THREE_UNCHARGE_DEBOUNCE_RATE_9          (0.0001)  ///去抖率  
/// 三节电池充电浮压调整阈值
#define THREE_FLOAT_VOLTAGE_THRESHOLD_1         (12000)  ///单位mV  三节电池充电浮压调整阈值
#define THREE_FLOAT_VOLTAGE_THRESHOLD_2         (11000)  ///单位mV  三节电池充电浮压调整阈值
#define THREE_FLOAT_VOLTAGE_THRESHOLD_3         (9000)   ///单位mV  三节电池充电浮压调整阈值
#define THREE_FLOAT_VOLTAGE_RATE_0              200      /// 三节电池充电浮压调整阈值 偏差  
#define THREE_FLOAT_VOLTAGE_RATE_1              460      /// 三节电池充电浮压调整阈值 偏差
#define THREE_FLOAT_VOLTAGE_RATE_2              600      /// 三节电池充电浮压调整阈值 偏差
/// 去抖配置
#define BATT_FORCE_UPDATE_DEBOUNCE              5       /// 强制更新去抖
#define BATT_CHARGE_FULL_DEBOUNCE               100     /// 充满去抖
#define BATT_CHARGE_FULL_DEBOUNCE_IGNORE        20      /// 充满去抖忽略阈值
/// 健康状态配置
#define BATT_CHARGING_TIME_OUT                  (162000)   ///(4.5 * 60 * 60 * 1000 / 100)
#define BATT_HEALTH_COEFF_MIN                   (90)
#define BATT_HEALTH_COEFF_MAX                   (100)

#define BATT_FULL_VOLTAGE                       12400

/// 电池容量
#define BATT_MAX_CAPACITY                       (3200)  /// 12260:28mAH   9587:2890mAH  9237:3000mAh
#define BATT_MS_PER_HOUR                        (3600000.0)  /// 毫秒每小时
#define BATT_MAX_CAPACITY_MAH                   ((double)BATT_MAX_CAPACITY * BATT_MS_PER_HOUR)
#define BATT_CHARGE_MAX_RESISTANCE              (0.18)  /// 充电电池最大内阻值
#define CHARGE_FORCE_UPDATE_THRESHOLD_OFFSET    (154)  /// 充电强制更新阈值偏移

///电池容量属性
typedef struct {
    uint8_t  percent;           ///百分比
    uint16_t vol_min;           ///最小电压
    uint16_t vol_max;           ///最大电压
} capacity_data_t;

/// 电池状态
typedef struct {
    uint8_t  health;
    uint8_t  charge;
    uint8_t  percent;
    uint8_t  type;
    int8_t   temperature;
    uint16_t voltage;
} battery_status_t;

/// 电池曲线
typedef struct {
    float rate;
    int16_t dropout_voltage;
} battery_para_t;

/// 电池浮压调整
typedef struct {
    uint16_t voltage;
    uint16_t threshold;
} battery_float_voltage_t;

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
void charge_vbus_init(void);
bool vbus_is_plug_in(void);
int bsp_charge_init(void);
int bsp_charge_resume(void);
int bsp_charge_suspend(void);
int get_charge_state(void);
int get_charging_temperature(void);
int get_charge_battery_voltage(void);
int get_charge_full_debounce(void);
void add_charge_event_callback(charge_event_cb cb);
void charge_event_internal_cb(charge_event_e event);

int battery_adc_init(void);
int battery_adc_resume(void);
int battery_adc_suspend(void);
int battery_adc_single_conversion(uint16_t *batt_voltage);
int bsp_charge_is_ready(void);
void charge_current_adjust(uint16_t current);
int bsp_charge_get_ibat_ma(uint16_t *ibat_ma);

#ifdef __cplusplus
}
#endif

#endif 
