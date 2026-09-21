#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include <stddef.h> // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep

typedef enum {
    I2C_MASTER_ID_0 = 0,
    I2C_MASTER_ID_1 = 1,
    I2C_MASTER_ID_2 = 2,
    I2C_MASTER_ID_3 = 3,
    I2C_MASTER_ID_MAX = 4,
} i2c_master_id_e;

typedef struct {
    uint8_t dev_address;
    uint8_t *reg_addr;
    uint8_t reg_addr_len;
    uint8_t *data;
    uint16_t len;
} i2c_master_data_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_i2c_write_byte(i2c_master_id_e device_id, i2c_master_data_t *data);
int bsp_i2c_read_byte(i2c_master_id_e device_id, i2c_master_data_t *data);
int bsp_i2c_init(i2c_master_id_e device_id);
int bsp_i2c_deinit(i2c_master_id_e device_id);
void i2c_abnormal_recovery(i2c_master_id_e device_id);
#ifdef __cplusplus
}
#endif

#endif 
