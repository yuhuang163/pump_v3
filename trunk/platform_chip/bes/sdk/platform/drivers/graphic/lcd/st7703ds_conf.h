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

#ifndef __ST7703DS_CONF_H__
#define __ST7703DS_CONF_H__

#ifndef CONFIG_DSI_2LANE_MODE
#define CONFIG_DSI_2LANE_MODE
#endif
#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_ST7703DS
#define LCD_WIDTH               720
#define LCD_HEIGHT              720
#define LCD_HFP                 40
#define LCD_HBP                 40
#define LCD_VFP                 16
#define LCD_VBP                 14
#define LCD_HSYNC               12
#define LCD_VSYNC               12
#define LCD_NAME                "ST7703DS"

#endif /* __ST7703DS_CONF_H__ */
