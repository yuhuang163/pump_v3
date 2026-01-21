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

/// 算法输出数据
typedef struct {
    uint32_t timestamp;
    uint8_t result[AIR_PRESSURE_NUM]; //佩戴检测结果 0:未佩戴 1:佩戴    
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
