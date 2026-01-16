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

#ifndef LV_BESJPEG_H
#define LV_BESJPEG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../src/lv_conf_internal.h"
#if LV_USE_BESJPEG

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register the BES jpeg decoder functions in LVGL
 */
void lv_besjpeg_init(void);

void lv_besjpeg_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BESJPEG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BESJPEG_H*/
