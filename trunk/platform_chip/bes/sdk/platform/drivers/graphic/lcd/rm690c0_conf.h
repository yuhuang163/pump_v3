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

#ifndef __RM690C0_CONF_H__
#define __RM690C0_CONF_H__

#if defined(CONFIG_LCDC_QSPI) && !defined(CONFIG_LCDC_QSPI_DTR)
#define CONFIG_LCDC_QSPI_DTR
#endif

#define LCD_RM690C0
#ifdef CONFIG_LCDC_QSPI
#define LCD_WIDTH               212
#define LCD_HEIGHT              520
#else /* DSI */
#define LCD_WIDTH               480
#define LCD_HEIGHT              480
#endif
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#define LCD_NAME                "RM690C0"

#endif /* __RM690C0_CONF_H__ */
