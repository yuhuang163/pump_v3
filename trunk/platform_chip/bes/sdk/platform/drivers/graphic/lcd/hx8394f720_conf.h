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

#ifndef __HX8394F720_CONF_H__
#define __HX8394F720_CONF_H__

#if !(defined(CONFIG_DSI_2LANE_MODE) || defined(CONFIG_DSI_4LANE_MODE))
#define CONFIG_DSI_2LANE_MODE
#endif
#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_HX8394F720
#define LCD_WIDTH               720
#define LCD_HEIGHT              1280
#ifdef CONFIG_DSI_4LANE_MODE
#if 1 /* PIX 67Mhz, DSI 400Mhz, 50fps */
#define LCD_HFP                 48
#define LCD_HBP                 232
#endif
#if 0 /* PIX 34Mhz, DSI 400Mhz, 30fps */
#define LCD_HFP                 48
#define LCD_HBP                 52
#endif
#else /* 2-Lane need DSI >= 480Mhz */
#define LCD_HFP                 48
#define LCD_HBP                 52
#endif
#define LCD_VFP                 16
#define LCD_VBP                 15
#define LCD_HSYNC               8
#define LCD_VSYNC               6
#define LCD_NAME                "HX8394F720"

#endif /* __HX8394F720_CONF_H__ */
