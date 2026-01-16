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

#ifndef __BES_TP_HAL_H__
#define __BES_TP_HAL_H__

#include "bes_tp_utils.h"

#include "hal_i2c.h"
#ifdef LVGL_TP_ZINITIX
#include "bes_tp_zinitix.h"
#endif

#ifdef LVGL_TP_TMA525C
#include "bes_tp_tma525c.h"
#endif

#ifdef LVGL_TP_CHSC6X
#include "tp_chsc6x.h"
#include "chsc6x_platform.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif

#define I2C_SPEED           100000

#ifdef LVGL_TP_ZINITIX
#define TP_RST_PIN   HAL_GPIO_PIN_P7_4
#define TP_INT_PIN   HAL_GPIO_PIN_P1_3
#endif

#ifdef LVGL_TP_TMA525C
#if defined(CHIP_BEST1600)
#define TP_RST_PIN   HAL_GPIO_PIN_P4_3
#define TP_INT_PIN   HAL_GPIO_PIN_P4_1
#elif defined(CHIP_BEST1700)
#ifdef ZE7_EVB
#define TP_RST_PIN   HAL_GPIO_PIN_P10_3
#define TP_INT_PIN   HAL_GPIO_PIN_P10_4
#else
#define TP_RST_PIN   HAL_GPIO_PIN_P4_3
#define TP_INT_PIN   HAL_GPIO_PIN_P4_1
#endif
#elif defined(CHIP_BEST1502X)
#define TP_RST_PIN   HAL_GPIO_PIN_P6_2
#define TP_INT_PIN   HAL_GPIO_PIN_P3_2
#elif defined(CHIP_BEST1502P)
#define TP_RST_PIN   HAL_GPIO_PIN_P6_2
#define TP_INT_PIN   HAL_GPIO_PIN_P3_2
#elif defined(CHIP_BEST1503)
#define TP_RST_PIN   HAL_GPIO_PIN_P6_2
#define TP_INT_PIN   HAL_GPIO_PIN_P3_2
#define TP_PWR_PIN   HAL_GPIO_PIN_P9_2
#elif defined(CHIP_BEST1306P)
#define TP_RST_PIN   HAL_GPIO_PIN_P1_7
#define TP_INT_PIN   HAL_GPIO_PIN_P1_5
#define TP_PWR_PIN   HAL_GPIO_PIN_P1_6
#elif defined(CHIP_BEST2003)
#define TP_RST_PIN   HAL_GPIO_PIN_P0_5
#define TP_INT_PIN   HAL_GPIO_PIN_P2_7
#elif defined(CHIP_BEST3601)
#define TP_RST_PIN   HAL_GPIO_PIN_P5_0
#define TP_INT_PIN   HAL_GPIO_PIN_P5_1
#endif
#endif

#ifdef LVGL_TP_CHSC6X
#define TP_RST_PIN   HAL_GPIO_PIN_P4_4
#define TP_INT_PIN   HAL_GPIO_PIN_P4_5
#endif

typedef int32_t (*tp_probe)(uint32_t i2c_bus);
typedef int32_t (*tp_init)(uint32_t i2c_bus);
typedef int32_t (*tp_coor)(tp_sample_t* tp_sample);

typedef struct i2c_dev {
    const char *      name;
    enum HAL_I2C_ID_T bus;
    uint32_t          addr; //device address
    uint32_t          speed;
    tp_probe          fn_probe;
    tp_coor           fn_coor;
    tp_init           fn_init;
}i2c_dev_t;

int bes_tp_hal_init(void);
void bes_tp_hal_exit(void);

bool bes_tp_is_pressed(void);
void bes_tp_get_xy(int32_t * x, int32_t * y);

#ifdef __cplusplus
}
#endif

#endif
