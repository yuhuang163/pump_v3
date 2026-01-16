#ifndef __HAL_LIGHT_SENSOR_H__
#define __HAL_LIGHT_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

/**
 * @brief 描述IMU操作码
*/
typedef enum {
    OP_CODE_LIGHT_SENSOR_GET_RAW_DATA = 1,
} light_sensor_dev_op_code_e;

/**
 * @brief 描述光传感器数据
*/
typedef struct {
    uint16_t lux_level;          //光传感器采集的亮度等级
} light_sensor_data_t;

#ifdef __cplusplus
}
#endif

#endif 
