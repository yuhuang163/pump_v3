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

#ifndef __LCD_PANEL_H__
#define __LCD_PANEL_H__

/* Initialize LCD panel */
int board_lcd_initialize(void);

/* Un-initialize LCD panel */
int board_lcd_uninitialize(void);

/* Set display area of LCD panel */
void lcd_set_disp_area(int x, int y, int w, int h);

/* Enter idle mode of LCD panel */
void lcd_enter_idle_mode(void);

/* Exit idle mode of LCD panel */
void lcd_exit_idle_mode(void);

/* Enter standby mode of LCD panel */
void lcd_enter_standby_mode(void);

/* Exit standby mode of LCD panel */
void lcd_exit_standby_mode(void);

/* Set the brightness of LCD panel */
void lcd_set_brightness(uint8_t val);

/* Get status of LCD panel */
int lcd_get_status(int flag);

#endif /* __LCD_COMMAND_H__ */
