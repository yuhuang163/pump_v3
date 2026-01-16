/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"
#include "bsp_lcd_internal.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if 0
static const lcd_chip_config_t st7735_config[] = {
    {0xB1, 3,  {0x05, 0x3C, 0x3C}},
    {0xB2, 3,  {0x05, 0x3C, 0x3C}},
    {0xB3, 6,  {0x05, 0x3C, 0x3C}},
    {0xB4, 1,  {0x03}},
    {0xC0, 3,  {0x28, 0x08, 0x04}},
    {0xC1, 1,  {0xC0}},
    {0xC2, 2,  {0x0D}},
    {0xC3, 2,  {0x8D, 0x2A}},
    {0xC4, 2,  {0x8D, 0xEE}},
    {0xC5, 1,  {0x1A}},
    {0xE0, 16, {0x04, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A,
                0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13}},
    {0xE1, 16, {0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27,
                0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13}},
    // {0x11, 0}, // 退出休眠模式
    {0x0C, 1,  {0x05}}, // RGB565
    {0x3A, 1,  {0x05}},
    {0x36, 1,  {0x08}},

    // {0x0C, 1,  {0x06}}, // RGB666
    {0x21, 0},
    {0x29, 0}, // 显示开
};
#else
static const lcd_chip_config_t st7735_config[] = {
    {0xB1, 3,  {0x05, 0x3C, 0x3C}},
    {0xB2, 3,  {0x05, 0x3C, 0x3C}},
    {0xB3, 6,  {0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}},
    {0xB4, 2,  {0x03, 0x02}},
    {0xC0, 3,  {0xA4, 0x04, 0x84}},
    {0xC1, 1,  {0x05}},
    {0xC2, 2,  {0x0D, 0x00}},
    {0xC3, 2,  {0x8D, 0x6A}},
    {0xC4, 2,  {0x8D, 0xEE}},
    {0xC5, 1,  {0x21}},
    {0xE0, 16, {0x0D, 0x0C, 0x0C, 0x0E, 0x0E, 0x00, 0x00, 0x00,
                0x00, 0x09, 0x23, 0x31, 0x00, 0x0C, 0x03, 0x1A}},
    {0xE1, 16, {0x0A, 0x05, 0x06, 0x07, 0x08, 0x01, 0x00, 0x00,
                0x00, 0x05, 0x20, 0x2E, 0x00, 0x0A, 0x01, 0x1A}},
    {0xFC, 1,  {0x80}},
    {0xF0, 1,  {0x11}},
    {0xD6, 1,  {0xCB}},
    {0x3A, 1,  {0x05}},
    {0x36, 1,  {0x08}},
    {0x21, 0},
    {0x29, 0},
};
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int lcd_chip_init(void)   
{   
    uint8_t *reg_data = NULL, command, len;

    // 退出休眠模式
    bsp_lcd_spi_write_param(0x11, NULL, 0);
    msleep(30);

    reg_data = u_malloc(REGISTER_DATA_SIZE);
    if (reg_data == NULL) {
        LOGE("malloc failed");
        return BSP_ERR_NO_MEM;
    }

    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(st7735_config); i++) {
        memcpy(reg_data, st7735_config[i].data, st7735_config[i].data_size);
        command = st7735_config[i].command;
        len = st7735_config[i].data_size;
        bsp_lcd_spi_write_param(command, reg_data, len);
    }
    u_free(reg_data);

    return BSP_OK;
}

int lcd_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    uint8_t buff[4] = {0};

    x_start += LCD_H_OFFSET;
    x_end   += LCD_H_OFFSET;
    y_start += LCD_V_OFFSET;
    y_end   += LCD_V_OFFSET;


    buff[0] = (uint8_t)(x_start >> 8);
    buff[1] = (uint8_t)x_start;
    buff[2] = (uint8_t)(x_end >> 8);
    buff[3] = (uint8_t)x_end;
    bsp_lcd_spi_write_param(0x2a, buff, 4);
    // BSP_LOG_HEX("x", buff, 4);

    buff[0] = (uint8_t)(y_start >> 8);
    buff[1] = (uint8_t)y_start;
    buff[2] = (uint8_t)(y_end >> 8);
    buff[3] = (uint8_t)y_end;
    bsp_lcd_spi_write_param(0x2b, buff, 4);
    // BSP_LOG_HEX("y", buff, 4);

    uint32_t data_size = (x_end - x_start + 1) * (y_end - y_start + 1) * (LCD_RGB565_PIXEL / 8);
    // LOGE("data_size:%d", data_size);
    return bsp_lcd_spi_write_param(0x2c, (uint8_t *)color_data, data_size);
}

void lcd_chip_sleep(void)
{
    bsp_lcd_spi_write_param(0x28, NULL, 0);
    bsp_lcd_spi_write_param(0x10, NULL, 0);
    msleep(40);
    LOG_PBUS("lcd_chip_sleep");
}
