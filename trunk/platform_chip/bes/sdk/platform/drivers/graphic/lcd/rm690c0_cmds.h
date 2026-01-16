/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#ifndef __RM690C0_CMDS_H__
#define __RM690C0_CMDS_H__

static struct lcd_command lcd_init_cmds[] = {
#ifdef CONFIG_LCDC_QSPI
    {0xFE, 1, {0x40}, 0},
    {0x62, 1, {0x2A}, 0},
    {0x44, 1, {0x00}, 0},
    {0x4D, 1, {0x00}, 0},
    {0x4E, 1, {0x0B}, 0},
    {0x61, 1, {0x01}, 0},
    {0x82, 1, {0x60}, 0},
    {0xFE, 1, {0x00}, 0},
    {0x53, 1, {0x20}, 0},
    {0xC4, 1, {0x40}, 0},
    {0x30, 4, {0x00, 0x01, 0x00, 0xD2}, 0},
    {0x2A, 4, {0x00, 0x00, 0x00, 0xD3}, 0},
    {0x31, 4, {0x00, 0x01, 0x02, 0x06}, 0},
    {0x2B, 4, {0x00, 0x00, 0x02, 0x07}, 0},
    {0x36, 1, {0x60}, 0},
    {0x35, 1, {0x02}, 0},
    {0x67, 1, {0x02}, 0},
    {0x68, 1, {0x20}, 0},
    {0x63, 1, {0xFF}, 0},
    {0x12, 0, {0x00}, 0},
#else /* CONFIG_LCDC_DSI */
    {0xFE, 1, {0x20}, 0},
    {0x4A, 1, {0xB0}, 0},
    {0xFE, 1, {0x00}, 0},
    {0x53, 1, {0x20}, 0},
#ifdef CONFIG_DSI_VIDEO_MODE
    {0xC2, 1, {0x03}, 0},
#else
    {0x35, 1, {0x02}, 0},
#endif
    {0xFE, 1, {0x00}, 0},
    {0x44, 4, {0x01, 0x7C, 0x00, 0x00}, 0},
    {0x2A, 4, {0x00, 0x00, 0x01, 0xDF}, 0},
    {0x2B, 4, {0x00, 0x00, 0x01, 0xDF}, 0},
#endif
    {0x36, 1, {0x00}, 0}, /* 0x00 - RGB order, 0x08 - BGR order */
#ifdef CONFIG_LCD_RGB565
    {0x3A, 1, {0x55}, 0},
#elif defined(CONFIG_LCD_RGB666)
    {0x3A, 1, {0x66}, 0},
#else
    {0x3A, 1, {0x77}, 0},
#endif
    {0x51, 1, {0xF0}, 0},
    {0x11, 0, {0x00}, 60},
    {0x29, 0, {0x00}, 20},
};

#endif /* __RM690C0_CMDS_H__ */
