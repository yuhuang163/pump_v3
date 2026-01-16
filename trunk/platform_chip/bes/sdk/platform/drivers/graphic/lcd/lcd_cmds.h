
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
#ifndef __LCD_COMMANDS_H__
#define __LCD_COMMANDS_H__

struct lcd_command {
    uint8_t cmd;
    uint8_t param_cnt;
    uint8_t param_data[64];
    uint8_t delay_ms;
};

#if defined(DISPLAY_RM69330)
#include "rm69330_cmds.h"
#elif defined(DISPLAY_GC9503NP)
#include "gc9503np_cmds.h"
#elif defined(DISPLAY_ST7701SN)
#include "st7701sn_cmds.h"
#elif defined(DISPLAY_ST7701S)
#include "st7701s_cmds.h"
#elif defined(DISPLAY_ST7703)
#include "st7703_cmds.h"
#elif defined(DISPLAY_ST7703DS)
#include "st7703ds_cmds.h"
#elif defined(DISPLAY_ST7785M)
#include "st7785m_cmds.h"
#elif defined(DISPLAY_A064)
#include "a064_cmds.h"
#elif defined(DISPLAY_HX8394F)
#include "hx8394f_cmds.h"
#elif defined(DISPLAY_HX8394F720)
#include "hx8394f720_cmds.h"
#elif defined(DISPLAY_HX8399C)
#include "hx8399c_cmds.h"
#elif defined(DISPLAY_ICNA3310)
#include "icna3310_cmds.h"
#elif defined(DISPLAY_RM69090)
#include "rm69090_cmds.h"
#elif defined(DISPLAY_9C01)
#include "9c01_cmds.h"
#elif defined(DISPLAY_ILI9881C)
#include "ili9881c_cmds.h"
#elif defined(DISPLAY_FL7703NI)
#include "fl7703ni_cmds.h"
#elif defined(DISPLAY_EQT400)
#include "eqt400_cmds.h"
#elif defined(DISPLAY_ST7102RGB)
#include "st7102rgb_cmds.h"
#elif defined(DISPLAY_RM690A0)
#include "rm690a0_cmds.h"
#elif defined(DISPLAY_RM690C0)
#include "rm690c0_cmds.h"
#elif defined(DISPLAY_MIPSHARP12)
#include "mipsharp12_cmds.h"
#elif defined(DISPLAY_MIPSHARP14)
#include "mipsharp14_cmds.h"
#elif defined(DISPLAY_ST77916)
#include "st77916_cmds.h"
#endif

#endif /* __LCD_CONFIG_H__ */
