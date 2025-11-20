#ifndef __HAL_AIR_PRESSURE_H__
#define __HAL_AIR_PRESSURE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述气压传感器操作码
*/
typedef enum {
    OP_CODE_AIR_PRESSURE_GET_STATUS = 1,
} air_pressure_dev_op_code_e;

/// 气压传感器状态
typedef struct {
    float pressure;
    float temp;
    uint32_t raw_data;
} air_pressue_status_t;

#ifdef __cplusplus
}
#endif

#endif 
