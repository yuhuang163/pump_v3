/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __HAL_CAPSENSEIF_H__
#define __HAL_CAPSENSEIF_H__

#ifdef __cplusplus
extern "C" {
#endif

enum HAL_CAPSENSEIF_RF_ACCESS_T {
    HAL_CAPSENSEIF_RF_ACCESS_ALL,
    HAL_CAPSENSEIF_RF_ACCESS_NO_PAGE,
    HAL_CAPSENSEIF_RF_ACCESS_NONE,
};

#ifdef NO_SENS_SPI

#include "cmsis.h"

__STATIC_FORCEINLINE int hal_capsenseif_open(void) { return 0; }

__STATIC_FORCEINLINE int hal_capsenseif_reg_write(unsigned short reg, unsigned short val) { return 0; }

__STATIC_FORCEINLINE int hal_capsenseif_reg_read(unsigned short reg, unsigned short *val) { *val = 0; return 0; }

__STATIC_FORCEINLINE enum HAL_CAPSENSEIF_RF_ACCESS_T hal_capsenseif_rf_access_config(enum HAL_CAPSENSEIF_RF_ACCESS_T type) { return HAL_CAPSENSEIF_RF_ACCESS_ALL; }

#else

int hal_capsenseif_open(void);

int hal_capsenseif_reg_write(unsigned short reg, unsigned short val);

int hal_capsenseif_reg_read(unsigned short reg, unsigned short *val);

enum HAL_CAPSENSEIF_RF_ACCESS_T hal_capsenseif_rf_access_config(enum HAL_CAPSENSEIF_RF_ACCESS_T type);

#endif

#ifdef __cplusplus
}
#endif

#endif

