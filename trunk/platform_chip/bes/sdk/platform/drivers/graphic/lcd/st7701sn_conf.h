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

#ifndef __ST7701SN_CONF_H__
#define __ST7701SN_CONF_H__

#ifndef CONFIG_DSI_2LANE_MODE
#define CONFIG_DSI_2LANE_MODE
#endif
#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_ST7701SN
#define LCD_WIDTH               480
#define LCD_HEIGHT              640
#define LCD_HFP                 60
#define LCD_HBP                 60
#define LCD_VFP                 13
#define LCD_VBP                 13
#define LCD_HSYNC               10
#define LCD_VSYNC               10
#define LCD_NAME                "ST7701SN"

#endif /* __ST7701SN_CONF_H__ */
