#ifndef __HAL_IMU_H__
#define __HAL_IMU_H__

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
    OP_CODE_IMU_ADD_IRQ_CALLBACK = 1,
    OP_CODE_IMU_ENABLE_IRQ = 2,
    OP_CODE_IMU_DISABLE_IRQ = 3,
    OP_CODE_IMU_GET_CHIP_ID = 4,
    OP_CODE_IMU_GET_IMU_FIFO_DATA = 5,
} imu_dev_op_code_e;

/**
 * @brief 描述IMU坐标数据
*/
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} coord_data_t;

/**
 * @brief 描述IMU原始数据
*/
typedef struct {
    coord_data_t acc;
    coord_data_t gyro;
} imu_raw_data_t;

/**
 * @brief 描述IMU fifo数据
*/
typedef struct {
    coord_data_t acc;
    coord_data_t gyro;
} imu_fifo_data_t;

/**
 * @brief 描述IMU fifo数据集合
*/
typedef struct {
    uint16_t data_cnt;
    // uint16_t fifo_times;
    imu_fifo_data_t *data;
} imu_fifo_t;

/**
 * @brief 描述IMU数据集合
*/
typedef struct {
    union {
        imu_raw_data_t raw;
        imu_fifo_t fifo;
    } un; 
} imu_wrapper_data_t;

/**
 * @brief 描述IMU加速度校准数据
*/
typedef struct { 
    float kx; 
    float ky; 
    float kz; 
    float syx; 
    float szx; 
    float szy; 
    float bx; 
    float by; 
    float bz; 
} acc_cali_data_t;

/**
 * @brief 描述IMU加速度校准数据
*/
typedef struct { 
    coord_data_t acc;
    coord_data_t gyro;
} imu_static_cali_data_t;

/**
 * @brief 描述IMU校准数据
*/
typedef struct {
    union {
        acc_cali_data_t acc_calib_theta;
        imu_static_cali_data_t static_cali;
    } un;
} imu_cali_data_t;

#ifdef __cplusplus
}
#endif

#endif 
