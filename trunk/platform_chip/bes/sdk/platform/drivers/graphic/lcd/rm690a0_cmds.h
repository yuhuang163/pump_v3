/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifndef __RM690A0_CMDS_H__
#define __RM690A0_CMDS_H__

#ifndef LCD_HSTART
#define LCD_HSTART      14
#endif
#ifndef LCD_VSTART
#define LCD_VSTART      0
#endif
#define LCD_HEND        (LCD_WIDTH + LCD_HSTART - 1)
#define LCD_VEND        (LCD_HEIGHT + LCD_VSTART - 1)

static struct lcd_command lcd_init_cmds[] = {
    {0xFE, 1, {0x01}, 0},
    {0xF7, 1, {0x38}, 0},
    {0xFE, 1, {0x0A}, 0},
    {0x05, 1, {0x10}, 0},
    {0xFE, 1, {0x00}, 0},
    {0x2A, 4, {0x00, 0x00, 0x00, 0xC1}, 0},
    {0x30, 4, {0x00, 0x01, 0x01, 0x6E}, 0},
    {0x31, 4, {0x00, 0x01, 0x00, 0xC0}, 0},
    {0x12, 1, {0x00}, 0},
    {0x35, 1, {0x02}, 0},
    {0x51, 1, {0xFF}, 0},
    {0x11, 1, {0x00}, 120},
    {0x29, 1, {0x00}, 0},
};

#endif /* __RM690A0_CMDS_H__ */
