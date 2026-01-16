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

#ifndef __RM69330_CONF_H__
#define __RM69330_CONF_H__

#define LCD_RM69330
#define LCD_WIDTH               454
#define LCD_HEIGHT              454
#ifdef CONFIG_DSI_VIDEO_MODE
#define LCD_HFP                 32
#define LCD_HBP                 32
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
#define LCD_NAME                "RM69330"

#ifdef DISPLAY_NR320x320
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               320
#define LCD_HEIGHT              320
#define LCD_HSTART              82
#define LCD_VSTART              66
#elif defined(DISPLAY_NR320x240)
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               320
#define LCD_HEIGHT              240
#define LCD_HSTART              82
#define LCD_VSTART              106
#elif defined(DISPLAY_NR240x320)
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               240
#define LCD_HEIGHT              320
#define LCD_HSTART              120
#define LCD_VSTART              66
#elif defined(DISPLAY_NR240x240)
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               240
#define LCD_HEIGHT              240
#define LCD_HSTART              120
#define LCD_VSTART              106
#elif defined(DISPLAY_NR160x160)
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH               160
#define LCD_HEIGHT              160
#define LCD_HSTART              162
#define LCD_VSTART              146
#endif

#endif /* __RM69330_CONF_H__ */
