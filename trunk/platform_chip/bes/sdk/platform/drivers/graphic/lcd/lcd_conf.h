
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
#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__

/* LCD panel parameters */

#if defined(DISPLAY_RM69330)
#include "rm69330_conf.h"
#elif defined(DISPLAY_GC9503NP)
#include "gc9503np_conf.h"
#elif defined(DISPLAY_ST7701SN)
#include "st7701sn_conf.h"
#elif defined(DISPLAY_ST7701S)
#include "st7701s_conf.h"
#elif defined(DISPLAY_ST7703)
#include "st7703_conf.h"
#elif defined(DISPLAY_ST7703DS)
#include "st7703ds_conf.h"
#elif defined(DISPLAY_ST7785M)
#include "st7785m_conf.h"
#elif defined(DISPLAY_A064)
#include "a064_conf.h"
#elif defined(DISPLAY_HX8394F)
#include "hx8394f_conf.h"
#elif defined(DISPLAY_HX8394F720)
#include "hx8394f720_conf.h"
#elif defined(DISPLAY_HX8399C)
#include "hx8399c_conf.h"
#elif defined(DISPLAY_ICNA3310)
#include "icna3310_conf.h"
#elif defined(DISPLAY_RM69090)
#include "rm69090_conf.h"
#elif defined(DISPLAY_9C01)
#include "9c01_conf.h"
#elif defined(DISPLAY_ILI9881C)
#include "ili9881c_conf.h"
#elif defined(DISPLAY_FL7703NI)
#include "fl7703ni_conf.h"
#elif defined(DISPLAY_EQT400)
#include "eqt400_conf.h"
#endif

/* LCD panel power/reset pins */

//#define SOCKET_BOARD

#ifdef SOCKET_BOARD
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#define LCD_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define LCD_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define LCD_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#else

#if defined(CHIP_BEST2003)
#define LCD_RST_PIN HAL_GPIO_PIN_P0_3
#define LCD_LED_PIN HAL_GPIO_PIN_P2_6
#endif

#if defined(CHIP_BEST2009)
#define LCD_RST_PIN HAL_GPIO_PIN_P5_2
#define LCD_TE_PIN  HAL_GPIO_PIN_P7_4
#define LCD_LED_PIN HAL_GPIO_PIN_P13_2
#endif

#if defined(CHIP_BEST1502X)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define LCD_1V8_PIN HAL_GPIO_PIN_P9_1
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#elif defined(DISPLAY_FL7703NI)
#define LCD_LED_PIN HAL_GPIO_PIN_P4_0
#undef LCD_RST_PIN
#define LCD_RST_PIN HAL_GPIO_PIN_P0_6
#endif
#endif

#if defined(CHIP_BEST1503)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define LCD_1V8_PIN HAL_GPIO_PIN_P9_2
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#endif
#endif

#if defined(CHIP_BEST1306P)
#define LCD_RST_PIN HAL_GPIO_PIN_P4_7
#define LCD_1V8_PIN HAL_GPIO_PIN_P1_0
#define LCD_2V8_PIN HAL_GPIO_PIN_P1_1
#define LCD_TE_PIN  HAL_GPIO_PIN_P2_4
#endif

#if defined(CHIP_BEST1600)
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_PMU_P1_1
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#elif defined(DISPLAY_GC9503NP)
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define LCD_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define LCD_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define LCD_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#endif

#if defined(CHIP_BEST1700)
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_PMU_P1_1
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#elif defined(DISPLAY_EQT400)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define LCD_RST_PIN2 HAL_GPIO_PIN_P3_6
#define LCD_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define LCD_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define LCD_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#endif

#endif /* !SOCKET_BOARD */

#endif /* __LCD_CONFIG_H__ */
