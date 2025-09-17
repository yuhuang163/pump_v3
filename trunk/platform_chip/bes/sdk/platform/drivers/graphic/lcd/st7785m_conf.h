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

#ifndef __ST7785M_CONF_H__
#define __ST7785M_CONF_H__

#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_ST7785M
#define LCD_WIDTH               240
#define LCD_HEIGHT              320
#define LCD_HFP                 50
#define LCD_HBP                 66
#define LCD_VFP                 200
#define LCD_VBP                 200
#define LCD_HSYNC               10
#define LCD_VSYNC               200
#define LCD_NAME                "ST7785M"

#endif /* __ST7785M_CONF_H__ */
