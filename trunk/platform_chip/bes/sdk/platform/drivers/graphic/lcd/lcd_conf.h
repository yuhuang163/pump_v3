
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
#elif defined(DISPLAY_ST7102RGB)
#include "st7102rgb_conf.h"
#elif defined(DISPLAY_RM690A0)
#include "rm690a0_conf.h"
#elif defined(DISPLAY_RM690C0)
#include "rm690c0_conf.h"
#elif defined(DISPLAY_MIPSHARP12)
#include "mipsharp12_conf.h"
#elif defined(DISPLAY_MIPSHARP14)
#include "mipsharp14_conf.h"
#elif defined(DISPLAY_ST77916)
#include "st77916_conf.h"
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
#define LCD_RST_PIN HAL_GPIO_PIN_P3_7
#define LCD_LED_PIN HAL_GPIO_PIN_P13_2
#endif

#if defined(CHIP_BEST1502X)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define LCD_RST_PIN2 HAL_GPIO_PIN_P6_6
#define LCD_1V8_PIN HAL_GPIO_PIN_P9_2
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#elif defined(DISPLAY_FL7703NI)
#define LCD_LED_PIN HAL_GPIO_PIN_P4_0
#undef LCD_RST_PIN
#define LCD_RST_PIN HAL_GPIO_PIN_P0_6
#endif
#endif

#if defined(CHIP_BEST1502P)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define LCD_RST_PIN2 HAL_GPIO_PIN_P6_6
#define LCD_1V8_PIN HAL_GPIO_PIN_P9_2
#define LCD_2V8_PIN HAL_GPIO_PIN_P9_3
#define LCD_3V3_PIN HAL_GPIO_PIN_P9_4
#define LCD_TE_PIN  HAL_GPIO_PIN_P8_1
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#endif
#if defined(DISPLAY_RM690C0)
#define LCD_LED_PIN HAL_GPIO_PIN_P6_3
#endif
#endif

#if defined(CHIP_BEST1503)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define LCD_RST_PIN2 HAL_GPIO_PIN_P6_6
#define LCD_1V8_PIN HAL_GPIO_PIN_P9_2
#define LCD_TE_PIN  HAL_GPIO_PIN_P8_1
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#endif
#if defined(DISPLAY_RM690C0)
#define LCD_LED_PIN HAL_GPIO_PIN_P6_3
#endif
#endif

#if defined(CHIP_BEST1503P)
#define LCD_RST_PIN HAL_GPIO_PIN_P6_6
#define LCD_RST_PIN2 HAL_GPIO_PIN_P9_0
#define LCD_1V8_PIN HAL_GPIO_PIN_P8_1
#define LCD_TE_PIN  HAL_GPIO_PIN_P9_1
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_P8_3
#endif
#if defined(DISPLAY_RM690C0)
#define LCD_LED_PIN HAL_GPIO_PIN_P6_3
#endif
#endif

#if defined(CHIP_BEST1306P)
#if CFG_CUSTOM_APP
#ifdef DISPLAY_RST_PIN
#define LCD_RST_PIN DISPLAY_RST_PIN
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P3_6
#endif
#ifdef DISPLAY_TE_PIN
#define LCD_TE_PIN DISPLAY_TE_PIN
#else
#define LCD_TE_PIN  HAL_GPIO_PIN_P3_7
#endif
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P4_7
#define LCD_1V8_PIN HAL_GPIO_PIN_P1_0
#define LCD_2V8_PIN HAL_GPIO_PIN_P1_1
#define LCD_TE_PIN  HAL_GPIO_PIN_P2_4
#endif
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

#if defined(CHIP_BEST1605)
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P3_1
#define LCD_TE_PIN  HAL_GPIO_PIN_P0_0
#define LCD_1V8_PIN HAL_GPIO_PIN_PMU_P0_0
#define LCD_2V8_PIN HAL_GPIO_PIN_PMU_P0_1
#define LCD_3V3_PIN HAL_GPIO_PIN_PMU_P0_5
#endif

#if defined(CHIP_BEST1700)
#if defined(ZE7_EVB) /* ZE7_EVB */
#define LCD_RST_PIN HAL_GPIO_PIN_P7_2
#define LCD_RST_PIN2 HAL_GPIO_PIN_P11_1
#define LCD_1V8_PIN HAL_GPIO_PIN_P3_2
#define LCD_2V8_PIN HAL_GPIO_PIN_P3_3
#define LCD_3V3_PIN HAL_GPIO_PIN_P1_5
#else /* old EVB */
#if defined(DISPLAY_ICNA3310)
#define LCD_LED_PIN HAL_GPIO_PIN_PMU_P1_1
#define LCD_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#elif defined(DISPLAY_EQT400) || defined(DISPLAY_ST7102RGB)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define CONFIG_LCD_SPI_CMD
#define CONFIG_LCD_SPI_SCL      HAL_GPIO_PIN_P7_4
#define CONFIG_LCD_SPI_CS       HAL_GPIO_PIN_P7_5
#define CONFIG_LCD_SPI_SDO      HAL_GPIO_PIN_P7_6
#define CONFIG_LCD_SPI_SDA      HAL_GPIO_PIN_P7_7
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define LCD_RST_PIN2 HAL_GPIO_PIN_P3_6
#define LCD_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define LCD_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define LCD_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#endif
#endif

#if defined(CHIP_BEST3601)
#if defined(DISPLAY_EQT400)
#define LCD_RST_PIN HAL_GPIO_PIN_P8_0
#define CONFIG_LCD_SPI_CMD
#define CONFIG_LCD_SPI_SCL      HAL_GPIO_PIN_P7_4
#define CONFIG_LCD_SPI_CS       HAL_GPIO_PIN_P7_5
#define CONFIG_LCD_SPI_SDO      HAL_GPIO_PIN_P7_6
#define CONFIG_LCD_SPI_SDA      HAL_GPIO_PIN_P7_7
#elif defined(DISPLAY_ST7102RGB)
#define LCD_RST_PIN HAL_GPIO_PIN_P3_2
#define CONFIG_LCD_SPI_CMD
#define CONFIG_LCD_SPI_SCL      HAL_GPIO_PIN_P2_4
#define CONFIG_LCD_SPI_CS       HAL_GPIO_PIN_P2_5
#define CONFIG_LCD_SPI_SDO      HAL_GPIO_PIN_P2_7
#define CONFIG_LCD_SPI_SDA      HAL_GPIO_PIN_P2_6
#else
#define LCD_RST_PIN HAL_GPIO_PIN_P5_6
#define LCD_TE_PIN HAL_GPIO_PIN_P5_7
#define LCD_3V3_PIN HAL_GPIO_PIN_P11_2
#endif
#endif

#endif /* !SOCKET_BOARD */

#endif /* __LCD_CONFIG_H__ */
