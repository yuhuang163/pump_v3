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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "lcd_show.h"
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"

#define IF_IMPORT_CHINESE

#ifdef IF_IMPORT_CHINESE
const unsigned char font_3232[]={ // font3232[1046784]
    #include "font_3232_1.txt"
    #include "font_3232_2.txt"
};
#endif

const unsigned char asc2_3216[95][64] ={
    #include "asc2_3216.txt"
};

const unsigned char BES[2][128] = {
{0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x03,0x80,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFE,0x1F,0xFF,0xFF,0xFC,
0x10,0xC0,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x38,0x00,0x08,0x00,0x10,0x00,0x08,0x04,0x00,0x00,0x08,0x04,0x00,0x00,0x08,0x04,0x3F,0xFF,0x88,0x04,0x3F,0xFF,0x88,
0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,0x04,0x20,0x82,0x08,
0x04,0x3F,0xFF,0x88,0x04,0x7F,0xFF,0x08,0x0C,0x20,0x00,0x08,0x0C,0x00,0x00,0x18,0x04,0x00,0x00,0x18,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"恒",0*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x10,0x01,0x00,0x20,0x1C,
0x01,0x00,0x70,0x1C,0x01,0x00,0xF0,0x3C,0x01,0x03,0xA0,0x68,0x01,0x06,0x20,0xC8,0x01,0x1C,0x21,0x88,0x21,0x78,0x23,0x08,0x19,0xE0,0x26,0x18,0x1F,0x40,0x4C,0x18,
0x0F,0x00,0x58,0x18,0x05,0x00,0x70,0x18,0x01,0x00,0xE0,0x10,0x01,0x01,0xC0,0x10,0x01,0x07,0x82,0x10,0x01,0x0F,0x01,0x10,0x01,0x06,0x01,0xD0,0x01,0x04,0x00,0xF8,
0x01,0x00,0x00,0x7E,0x03,0x00,0x00,0x1C,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"玄",1*/
};


void lcd_show_char(uint32_t dispbuf, uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode)
{
    uint8_t temp,t1,t;
	uint16_t y0 = y;
    uint16_t *buf = (uint16_t *)dispbuf;
	uint8_t csize = (size/8 + ((size%8)?1:0)) * (size/2);
 	num = num - ' ';
	for(t = 0; t < csize; t++) {
		temp = asc2_3216[num][t];
		for(t1 = 0; t1 < 8; t1++) {
            if(temp&0x80) buf[y*LCD_WIDTH + x] = POINT_COLOR;
			else if(mode==0) buf[y*LCD_WIDTH + x] = BACK_COLOR;
			temp<<=1;
			y++;
			if(y >= LCD_HEIGHT) {
                return;
            }
			if((y - y0) == size) {
				y = y0;
				x++;
				if(x >= LCD_WIDTH) {
                    return;
                }
				break;
			}
		}
	}
}

void lcd_show_chinese(uint32_t dispbuf, uint16_t x, uint16_t y, uint32_t offset, uint8_t size, uint8_t mode)
{
    uint32_t t1, t;
    uint16_t yw = y, xw = x;
    uint32_t csize = (size/8 + ((size%8)?1:0)) * size;
    uint16_t *pbuf = (uint16_t *)dispbuf;
    uint8_t tmp;

    for(t=0; t<csize; t++) {
#ifdef IF_IMPORT_CHINESE
        tmp = font_3232[t + offset];
#else
        tmp = BES[1][t];
#endif
        for(t1=0; t1<8; t1++) {
            if(tmp&0x80) {
                pbuf[yw*LCD_WIDTH + xw] = POINT_COLOR;
            }
            else if(mode == 0) {
                pbuf[yw*LCD_WIDTH + xw] = BACK_COLOR;
            }
            tmp <<= 1;
            yw++;
            if(yw > LCD_HEIGHT)return;
            if((yw-y) >= size) {
                yw = y;
                xw++;
                if(xw > LCD_WIDTH)return;
            }
        }
    }
}

void lcd_show_string(uint32_t dispbuf, uint16_t x0, uint16_t y0, uint32_t width, \
                    uint32_t height, uint8_t fsize, uint8_t mode, const char *str)
{
    uint8_t *ch = (uint8_t *)str;
    uint32_t qh, wh;
    uint32_t offset;
    uint16_t x = x0;
    uint16_t y = y0;
    uint32_t i = 0;
    uint8_t line_end = 0;

    while (ch[i]!='\0') {
        if ((ch[i] == '\r') || (ch[i] == '\n')) {
            line_end = 1;
            i++;
            continue;
        }
        if (line_end) {
            line_end = 0;
            x = x0;
            y += fsize;
            if (y > height-fsize) {
                return;
            }
        }
        if (ch[i]<127) {
            if (x > width-fsize/2) {
                x = x0;
                y += fsize;
                if (y > height-fsize) {
                    return;
                }
            }
            lcd_show_char(dispbuf, x, y, ch[i], fsize, mode);
            i++;
            x += fsize / 2;
        }
        else {
            if (x > width-fsize) {
                x = x0;
                y += fsize;
                if (y > height-fsize) {
                    return;
                }
            }
            qh = ch[i] - 0xa0;
            wh = ch[i+1] - 0xa0;
            offset = (94*(qh-1)+(wh-1))*128;
            lcd_show_chinese(dispbuf, x, y, offset, fsize, mode);
            i+=2;
            x += fsize;
        }
    }
}

void lcd_fill_back(uint32_t dispbuf)
{
    #define BS              (8)

    uint32_t i;
    uint16_t *buf = (uint16_t *)dispbuf;
    uint32_t num = LCD_WIDTH * LCD_HEIGHT;
    uint32_t blk = num / BS;
    uint32_t lst = num % BS;

    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            buf[0] = BACK_COLOR;
            buf[1] = BACK_COLOR;
            buf[2] = BACK_COLOR;
            buf[3] = BACK_COLOR;
            buf[4] = BACK_COLOR;
            buf[5] = BACK_COLOR;
            buf[6] = BACK_COLOR;
            buf[7] = BACK_COLOR;
            buf += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            buf[i] = BACK_COLOR;
        }
    }
}

void lcd_fast_fill_back(uint32_t dispbuf)
{
    #define BS              (8)
    #define BACK_COLOR_D    ((((uint32_t)BACK_COLOR)<<16)|((uint32_t)BACK_COLOR))

    uint32_t i, j;
    uint32_t *buf = (uint32_t *)dispbuf;
    uint32_t num = LCD_WIDTH*LCD_HEIGHT/2;
    uint32_t blk = num / BS;
    uint32_t lst = num % BS;

    if (blk > 0) {
        for (i = 0; i < blk; i++) {
            buf[0] = BACK_COLOR_D;
            buf[1] = BACK_COLOR_D;
            buf[2] = BACK_COLOR_D;
            buf[3] = BACK_COLOR_D;
            buf[4] = BACK_COLOR_D;
            buf[5] = BACK_COLOR_D;
            buf[6] = BACK_COLOR_D;
            buf[7] = BACK_COLOR_D;
            buf += BS;
        }
    }
    if (lst > 0) {
        for (i = 0; i < lst; i++) {
            buf[i] = BACK_COLOR_D;
        }
    }
}

void lcd_fill_rect(uint32_t dispbuf, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color)
{
    uint32_t i, j;
    uint16_t *pbuf = (uint16_t *)dispbuf;

    if ((x1 < x0) || (y1 < y0) || (x1 >= LCD_WIDTH) || (y1 >= LCD_HEIGHT)) {
        return;
    }

    for (i = y0; i <= y1; i++) {
        for (j = x0; j <= x1; j++) {
			pbuf[i*LCD_WIDTH + j] = color;
        }
    }
}

void lcd_show_line_b(uint32_t dispbuf, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t border, uint32_t color)
{
    if ((x1+border >= LCD_WIDTH) || (y1+border >= LCD_HEIGHT) || (x2+border >= LCD_WIDTH) || (y2+border >= LCD_HEIGHT)) {
        return;
    }
    if ((x1 < border) || (y1 < border) || (x2 < border) || (y2 < border) || (border < 1)) {
        return;
    }
    int dx = x2 - x1;
    int dy = y2 - y1;
    dx = dx >= 0 ? dx : -dx;
    dy = dy >= 0 ? dy : -dy;

    if (dx > dy) {
        uint16_t i, f = (border - 1) / 2, r = (border - 1) % 2;
        lcd_show_line(dispbuf, x1, y1, x2, y2, color);
        for (i=1; i<=f; i++) {
            lcd_show_line(dispbuf, x1, y1+i, x2, y2+i, color);
            lcd_show_line(dispbuf, x1, y1-i, x2, y2-i, color);
        }
        if(r){
            lcd_show_line(dispbuf, x1, y1+i, x2, y2+i, color);
        }
    }
    else {
        uint16_t i, f = (border - 1) / 2, r = (border - 1) % 2;
        lcd_show_line(dispbuf, x1, y1, x2, y2, color);
        for (i=1; i<=f; i++) {
            lcd_show_line(dispbuf, x1+i, y1, x2+i, y2, color);
            lcd_show_line(dispbuf, x1-i, y1, x2-i, y2, color);
        }
        if(r){
            lcd_show_line(dispbuf, x1+i, y1, x2+i, y2, color);
        }
    }
}

void lcd_show_line(uint32_t dispbuf, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color)
{
    uint16_t *pbuf = (uint16_t *)dispbuf;
	int dx, dy, e;

    if ((x1 >= LCD_WIDTH) || (y1 >= LCD_HEIGHT) || (x2 >= LCD_WIDTH) || (y2 >= LCD_HEIGHT)) {
        return;
    }

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx>=0) {
		if (dy >= 0) {
			if (dx >= dy) { // 1/8 octant
				e = dy - dx/2;
				while (x1 <= x2) {
					pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {y1+=1; e-=dx;}
					x1 += 1;
					e += dy;
				}
			}
			else {		// 2/8 octant
				e = dx - dy/2;
				while(y1 <= y2) {
                    pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {x1 += 1; e -= dy;}
					y1 += 1;
					e += dx;
				}
			}
		}
		else {		   // dy<0
			dy=-dy;         // dy=abs(dy)
			if (dx >= dy) {  // 8/8 octant
				e = dy - dx/2;
				while (x1 <= x2) {
                    pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {y1 -= 1; e -= dx;}
					x1 += 1;
					e += dy;
				}
			}
			else {	 // 7/8 octant
				e = dx - dy/2;
				while (y1 >= y2) {
                    pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {x1 += 1; e -= dy;}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
	else {       //dx<0
		dx = -dx;		//dx=abs(dx)
		if (dy >= 0) {    // dy>=0
			if (dx >= dy) {    // 4/8 octant
				e = dy - dx/2;
				while (x1 >= x2) {
					pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {y1 += 1; e -= dx;}
					x1 -= 1;
					e += dy;
				}
			}
			else {		// 3/8 octant
				e = dx - dy/2;
				while (y1 <= y2) {
					pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {x1 -= 1; e -= dy;}
					y1 += 1;
					e += dx;
				}
			}
		}
		else {		            // dy<0
			dy = -dy;           // dy=abs(dy)
			if (dx >= dy) {       // 5/8 octant
				e = dy - dx/2;
				while (x1 >= x2) {
					pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {y1 -= 1; e -= dx;}
					x1 -= 1;
					e += dy;
				}
			}
			else {		// 6/8 octant
				e = dx - dy/2;
				while (y1 >= y2) {
					pbuf[y1*LCD_WIDTH + x1] = color;
					if (e > 0) {x1 -= 1; e -= dy;}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
}
