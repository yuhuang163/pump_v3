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

#ifndef __RM690A0_CONF_H__
#define __RM690A0_CONF_H__

#define RM690A0
#define LCD_WIDTH               194
#define LCD_HEIGHT              368
#ifdef CONFIG_DSI_VIDEO_MODE
#define LCD_HFP                 120
#define LCD_HBP                 120
#define LCD_VFP                 20
#define LCD_VBP                 20
#define LCD_HSYNC               1
#define LCD_VSYNC               1
#else /* for command mode */
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#endif
#define LCD_NAME                "RM690A0"

#endif /* __RM690A0_CONF_H__ */
