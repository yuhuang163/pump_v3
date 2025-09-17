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
#include "qrcode_gui.h"
#include "lcd_show.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "vplayer_app_common.h"
#include "camera_drv.h"

#define CAMERA_PIXEL_SIZE   (2)
#define CAMERA_WIDTH        (480)
#define CAMERA_HEIGHT       (640)
#define CAMERA_PIXEL_NUM    (CAMERA_WIDTH * CAMERA_HEIGHT)

#define GUI_PIXEL_SIZE      (2)
#define GUI_PIXEL_NUM       (LCD_WIDTH * LCD_HEIGHT)


#define GUI_DIS_TYPE_BLANK      (0x00)
#define GUI_DIS_TYPE_VIDOE      (0x01)
#define GUI_DIS_TYPE_SCAN       (0X02)
#define GUI_DIS_TYPE_RESULT     (0x03)

static uint8_t dispcache[GUI_PIXEL_NUM * GUI_PIXEL_SIZE] = {0};
static uint8_t *img_cache = NULL;

static uint8_t gui_dis_type_id = GUI_DIS_TYPE_VIDOE;

static qrcode_result_t gui_qrcode_result = {0};

static void qr_gui_show_data_result(uint32_t dispbuf, qrcode_result_t *result);

void qr_gui_show_data_refresh(uint32_t dst_addr, uint32_t src_addr, uint32_t size)
{
    switch (gui_dis_type_id)
    {
    case GUI_DIS_TYPE_BLANK:
        camera_data_raw10_to_yuv422(dst_addr, (uint32_t)dispcache, size);
        break;
    case GUI_DIS_TYPE_VIDOE:
        camera_data_raw10_to_yuv422(dst_addr, src_addr, size);
        break;
    case GUI_DIS_TYPE_SCAN:
        camera_data_raw10_to_yuv422(dst_addr, src_addr, size);
        break;
    case GUI_DIS_TYPE_RESULT:
        camera_data_raw10_to_yuv422(dst_addr, (uint32_t)dispcache, size);
        break;

    default:
        camera_data_raw10_to_yuv422(dst_addr, src_addr, size);
        break;
    }
    // TRACE(1, "qr_gui_dis_id:%d.", gui_dis_type_id);
}

void qr_gui_show_blank(void)
{
    lcd_fill_back((uint32_t)dispcache);
    gui_dis_type_id = GUI_DIS_TYPE_BLANK;
}

void qr_gui_show_video_player(void)
{
    gui_dis_type_id = GUI_DIS_TYPE_VIDOE;
}

void qr_gui_show_scan(void)
{
    gui_dis_type_id = GUI_DIS_TYPE_SCAN;
}

void qr_gui_show_result(void)
{
    qr_gui_show_data_result((uint32_t)img_cache, &gui_qrcode_result);
    memcpy(dispcache, img_cache, sizeof(dispcache));
    gui_dis_type_id = GUI_DIS_TYPE_RESULT;
}

void qr_gui_get_result(qrcode_result_t *buf)
{
    memcpy(&gui_qrcode_result, buf, sizeof(qrcode_result_t));
}

void qr_gui_img_capture_get(uint32_t addr )
{
    img_cache = (uint8_t *)addr;
}

static void qr_gui_show_result_code_border(uint32_t dispbuf, qrcode_result_t *result)
{
#define SPACE_D     (15)
    typedef struct {
        uint16_t x;
        uint16_t y;
    }point_t;
    point_t point[8] = {0};
    point_t center = {0};
    uint32_t i, p_count = result->location_count;
    for (i=0; i<p_count; i++) {
        point[i].x = result->location_x[i];
        point[i].y = result->location_y[i];
        center.x += point[i].x;
        center.y += point[i].y ;
    }
    center.x /= p_count;
    center.y /= p_count;
    for (i=0; i<p_count; i++) {
        float dx = (float)point[i].x - center.x;
        float dy = (float)point[i].y - center.y;
        float distance = sqrtf(dx*dx + dy*dy);
        point[i].x = (uint16_t)(point[i].x + SPACE_D * dx / distance);
        point[i].y = (uint16_t)(point[i].y + SPACE_D * dy / distance);
    }

    lcd_show_line_b(dispbuf, point[0].x , point[0].y , point[p_count-1].x, point[p_count-1].y , 5, COLOR_BLACK);
    for (i=1; i<p_count; i++) {
        lcd_show_line_b(dispbuf, point[i-1].x, point[i-1].y, point[i].x, point[i].y, 5, COLOR_BLACK);
    }
}

static void qr_gui_show_data_result(uint32_t dispbuf, qrcode_result_t *result)
{
#define LEFT_POS    16
#define RIGHT_POS   16
    const char *str = (const char *)result->data;
    uint32_t len = strlen(str);
    char label[30] = {0};
    uint16_t x, y;

    sprintf(label, "Scanned %d Chars", len);
    x = (LCD_WIDTH - strlen(label)*16) / 2;
    y = 8;
    lcd_show_string(dispbuf, x, y, LCD_WIDTH-x, LCD_HEIGHT-y, 32, 0, (const char *)label);

    if (0==len) {
        return;
    }
    x = LEFT_POS;
    y += 48;
    lcd_show_string(dispbuf, x, y, LCD_WIDTH-RIGHT_POS, LCD_HEIGHT-y, 32, 0, str);

    if (result->location_count > 3) {
        qr_gui_show_result_code_border(dispbuf, result);
    }
}
