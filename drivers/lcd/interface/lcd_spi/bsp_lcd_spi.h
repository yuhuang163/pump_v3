#ifndef __BSP_LCD_SPI_H__
#define __BSP_LCD_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>
#include "bsp_common.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int bsp_lcd_spi_init(void);
int bsp_lcd_spi_resume(void);
int bsp_lcd_spi_suspend(void);
int bsp_lcd_spi_write_cmd_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size);
int bsp_lcd_spi_write_img_param(uint8_t cmd, const uint8_t *p_data, uint32_t data_size);

#ifdef __cplusplus
}
#endif

#endif 
