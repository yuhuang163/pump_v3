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

#ifndef __ST7102RGB_CONF_H__
#define __ST7102RGB_CONF_H__

#ifndef CONFIG_LCD_RGB565
#define CONFIG_LCD_RGB565
#endif
#ifndef CONFIG_LCD_CLK_INV
#define CONFIG_LCD_CLK_INV
#endif
#ifndef CONFIG_LCD_HSYNC_INV
#define CONFIG_LCD_HSYNC_INV
#endif
#ifndef CONFIG_LCD_VSYNC_INV
#define CONFIG_LCD_VSYNC_INV
#endif

#define LCD_ST7102RGB
#define LCD_WIDTH               480
#define LCD_HEIGHT              640
#define LCD_HFP                 30
#define LCD_HBP                 30
#define LCD_VFP                 170
#define LCD_VBP                 8
#define LCD_HSYNC               8
#define LCD_VSYNC               2
#define LCD_NAME                "ST7102-RGB"

#endif /* __ST7102RGB_CONF_H__ */
