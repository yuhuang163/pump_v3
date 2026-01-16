#ifndef __BSP_LCD_QSPI_H__
#define __BSP_LCD_QSPI_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h>
#include "bsp_common.h" // IWYU pragma: keep

#define IS_SUPPORT_LCD_QSPI_DRIVER      (1)

#if IS_SUPPORT_LCD_QSPI_DRIVER
#define LCD_INTERFACE_INIT()                      bsp_lcd_qspi_init()
#define LCD_WRITE_CMD_PARAM(cmd, data, data_size) bsp_lcd_qspi_write_cmd_param(cmd, data, data_size)
#define LCD_WRITE_IMG_PARAM(cmd, data, data_size) bsp_lcd_qspi_write_img_param(cmd, data, data_size)
#else
#define LCD_INTERFACE_INIT()                      bsp_lcd_spi_init()
#define LCD_WRITE_CMD_PARAM(cmd, data, data_size) bsp_lcd_spi_write_cmd_param(cmd, data, data_size)
#define LCD_WRITE_IMG_PARAM(cmd, data, data_size) bsp_lcd_spi_write_img_param(cmd, data, data_size)
#endif

/// 寄存器数据大小
#define  REGISTER_DATA_SIZE             16

///  lcd 芯片配置结构体
typedef struct {
    uint8_t command;
    uint8_t data_size;
    uint8_t data[REGISTER_DATA_SIZE];   
} lcd_chip_config_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
