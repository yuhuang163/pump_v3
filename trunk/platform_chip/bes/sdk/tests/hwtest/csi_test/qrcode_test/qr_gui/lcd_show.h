/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __QRCODE_LCD_SHOW_H__
#define __QRCODE_LCD_SHOW_H__

#ifdef __cplusplus
extern "C" {
#endif


#define LCD_WIDTH       (480)
#define LCD_HEIGHT      (640)
#define COLOR_BLUE_YUV  (0xB5525D52)
#define COLOR_WHITE     (0x03ff)
#define COLOR_BLACK     (0x0000)
#define COLOR_GRAY      (0x0200)
// #define COLOR_WHITE     (0x80ff)
// #define COLOR_BLACK     (0x8000)
#define POINT_COLOR     COLOR_WHITE
#define BACK_COLOR      COLOR_BLACK

void lcd_show_char(uint32_t dispbuf, uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);

void lcd_show_chinese(uint32_t dispbuf, uint16_t x, uint16_t y, uint32_t offset, uint8_t size, uint8_t mode);

void lcd_show_string(uint32_t dispbuf, uint16_t x0, uint16_t y0, uint32_t width, uint32_t height, \
                        uint8_t fsize, uint8_t mode, const char *str);
void lcd_fill_back(uint32_t dispbuf);

void lcd_fill_rect(uint32_t dispbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);

void lcd_show_line(uint32_t dispbuf, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);

void lcd_show_line_b(uint32_t dispbuf, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t border, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* __QRCODE_LCD_SHOW_H__ */
