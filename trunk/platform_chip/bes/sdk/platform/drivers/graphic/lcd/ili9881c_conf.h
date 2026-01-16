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

#ifndef __ILI9881C_CONF_H__
#define __ILI9881C_CONF_H__

#ifndef CONFIG_DSI_2LANE_MODE
#define CONFIG_DSI_2LANE_MODE
#endif
#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif

#define LCD_ILI9881C
#define LCD_WIDTH               800
#define LCD_HEIGHT              800
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#define LCD_NAME                "ILI9881C"

#endif /* __ILI9881C_CONF_H__ */
