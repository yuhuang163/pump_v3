#ifndef __ROOT_ALGO_H__
#define __ROOT_ALGO_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "motor_policy.h" // IWYU pragma: keep
/// 佩戴检测结果枚举
typedef enum {
    ALGO_WEAR_DETECTION_RESULT_INVALID = 0,
    ALGO_WEAR_DETECTION_RESULT_NORMAL = 1,
    ALGO_WEAR_DETECTION_RESULT_SMALL_LEAK = 2,
    ALGO_WEAR_DETECTION_RESULT_LARGE_LEAK = 3,
} algo_wear_detection_result_e;

/// 佩戴状态（weared_status）
typedef enum {
    ALGO_WEARED_STATUS_INVALID = 0,  /// 检测未启动
    ALGO_WEARED_STATUS_WORN = 1,     /// 佩戴
    ALGO_WEARED_STATUS_NOT_WORN = 2, /// 未佩戴
} algo_weared_status_e;

/// 算法输出数据
typedef struct {
    uint32_t timestamp;
    uint8_t wear_result[AIR_PRESSURE_NUM]; /// 检测未启动无效状态：0，佩戴：1，未佩戴：2  
    uint8_t result[AIR_PRESSURE_NUM]; // 泵奶未启动结果无效状态:0,佩戴正常无漏气：1,佩戴微小漏气:2，佩戴强漏气:3    
    uint8_t compensation_status[AIR_PRESSURE_NUM]; //气压补偿状态 0:不补偿, 1:减小气压, 2:加大气压
    int compensation_delta_pwm[AIR_PRESSURE_NUM]; //气压补偿delta_pwm -10000~-1:PWM减小, 1~10000: PWM增加
} algo_wear_detection_data_output_t;

/// 泵奶数据
typedef struct algo_breast_pump_data_t {
    algo_wear_detection_data_output_t wear_detection; // 佩戴检测结果 0:未佩戴 1:佩戴    
} algo_breast_pump_data_t;

/// 算法采集数据
typedef struct {
    uint32_t timestamp;
    float air_pressure[AIR_PRESSURE_NUM];
    motor_component_state_t pump_state[BREAST_PUMP_NUM];
} algo_sampling_data_t;

#ifdef __cplusplus
}
#endif

#endif 
