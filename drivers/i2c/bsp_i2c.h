#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
int bsp_i2c_write_byte(uint8_t dev_address, uint8_t reg_addr, uint8_t *data, size_t len);
int bsp_i2c_read_byte(uint8_t dev_address, uint8_t reg_addr, uint8_t *data, size_t len);
int bsp_i2c_init(void);
int bsp_i2c_deinit(void);

#ifdef __cplusplus
}
#endif

#endif 
