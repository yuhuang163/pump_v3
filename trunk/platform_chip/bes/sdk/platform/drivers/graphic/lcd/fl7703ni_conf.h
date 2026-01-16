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

#ifndef __FL7703NI_CONF_H__
#define __FL7703NI_CONF_H__

#ifndef CONFIG_DSI_VIDEO_MODE
#define CONFIG_DSI_VIDEO_MODE
#endif
#ifndef CONFIG_LCD_SWAP_RB
#define CONFIG_LCD_SWAP_RB
#endif

#define LCD_FL7703NI
#define LCD_WIDTH               640
#define LCD_HEIGHT              480
#define LCD_HFP                 117
#define LCD_HBP                 110
#define LCD_HSYNC               110
#define LCD_VFP                 16
#define LCD_VBP                 12
#define LCD_VSYNC               4
#define LCD_NAME                "FL7703NI"

#endif /* __FL7703NI_CONF_H__ */
