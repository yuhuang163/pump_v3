#ifndef __HAL_FORC_SENSOR_H__
#define __HAL_FORC_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述力传感器操作码
*/
typedef enum {
    OP_CODE_FSENSOR_IS_READY = 1,
    OP_CODE_FSENSOR_GET_VERNO = 2,
    OP_CODE_FSENSOR_GET_RAW_DATA = 3,
    OP_CODE_FSENSOR_GET_PRE_LEVEL = 4,
    OP_CODE_FSENSOR_RESET = 5,
    OP_CODE_FSENSOR_GET_PRE_DATA = 6,
    OP_CODE_FSENSOR_OTA_UPGRADE = 7,
    OP_CODE_FSENSOR_CALI_HANDLE = 8,
    OP_CODE_FSENSOR_ADD_CALI_EVENT_CALLBACK = 9,
} fsensor_dev_op_code_e;

/**
 * @brief 描述力传感器通道枚举
*/
typedef enum {
    F_SENSOR_ID_POWER_KEY,
    F_SENSOR_ID_MODE_KEY,
    F_SENSOR_ID_BREATH_PUMP_HEAD,
    F_SENSOR_ID_MAX,
}force_sensro_channel_e;

/**
 * @brief 描述力传感器数据
*/
typedef struct {
    uint16_t adc[F_SENSOR_ID_MAX];
    int16_t value[F_SENSOR_ID_MAX];
    uint16_t temperature[F_SENSOR_ID_MAX];
} f_sensor_data_t;

/**
 * @brief 描述力传感器压力等级枚举
*/
typedef enum {
    F_SENSOR_PRESS_LEVEL_UNKNOWN,
    F_SENSOR_PRESS_LEVEL_MICROSHOCK,  // 小于40g 微震
    F_SENSOR_PRESS_LEVEL_1,           // 大于等于40g且小于120g
    F_SENSOR_PRESS_LEVEL_2,           // 大于等于120g且小于210
    F_SENSOR_PRESS_LEVEL_3,           // 大于等于210g且小于300
    F_SENSOR_PRESS_LEVEL_OVERPRESS,   // 大于等于300g  过压
} force_sensor_press_level_e;

/**
 * @brief 描述力传感器校准状态枚举
*/
typedef enum {
    F_SENSOR_CALI_IDLE,
    F_SENSOR_CALI_INIT,
    F_SENSOR_CALI_SELF_CHECKING,
    F_SENSOR_CALI_WITH_WEIGHT,
    F_SENSOR_CALI_NO_WEIGHT,
    F_SENSOR_CALI_SUCCESS,
    F_SENSOR_CALI_FAIL
} f_sensor_cali_status_e;

#ifdef __cplusplus
}
#endif

#endif 
