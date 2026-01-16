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

#ifndef __HX8399C_CONF_H__
#define __HX8399C_CONF_H__

#if !defined(CONFIG_DSI_4LANE_MODE) && !defined(CONFIG_DSI_2LANE_MODE)
#define CONFIG_DSI_4LANE_MODE
#endif
#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_HX8399C
#define LCD_WIDTH               1080
#define LCD_HEIGHT              1920
#if 0 /* DSI 400Mhz + PIX 67Mhz */
#define LCD_HFP                 120
#define LCD_HBP                 120
#define LCD_HSYNC               20
#endif
#if 1 /* DSI 400Mhz + PIX 34Mhz */
#define LCD_HFP                 32
#define LCD_HBP                 32
#define LCD_HSYNC               8
#endif
#define LCD_VFP                 16
#define LCD_VBP                 16
#define LCD_VSYNC               8
#define LCD_NAME                "HX8399C"

#ifdef DISPLAY_NR960x960
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               960
#define LCD_HEIGHT              960
#endif

#endif /* __HX8399C_CONF_H__ */
