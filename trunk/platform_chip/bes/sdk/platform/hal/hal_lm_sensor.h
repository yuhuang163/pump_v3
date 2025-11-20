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
#ifndef __HAL_LM_SENSOR_H__
#define __HAL_LM_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "plat_addr_map.h"

#ifdef LM_SENSOR0_BASE
#define DEFAULT_CNT_THRESHOLD                   0x2FF0
#define DEFAULT_PULSE_CNT_THRESHOLD             0xFFFF
#define MAX_PULSE_CNT                           4096
#define MAX_TEMP_RANGE                          256
#define BASE_TEMP_VALUE                         (-50)

enum HAL_LM_SENSOR_ID_T {
    HAL_LM_SENSOR_ID_0 = 0,
#ifdef LM_SENSOR1_BASE
    HAL_LM_SENSOR_ID_1 = 1,
#endif

    HAL_LM_SENSOR_ID_NUM,
};

enum HAL_LM_SENSOR_STATE_T {
    HAL_LM_SENSOR_NULL = 0,
    HAL_LM_SENSOR_ENABLE,
    HAL_LM_SENSOR_DISABLE,
};

enum HAL_LM_SENSOR_ERRCODE_T {
    HAL_LM_SENSOR_ERRCODE_NULL = 0,
    HAL_LM_SENSOR_ERRCODE_INVALID_ID,
    HAL_LM_SENSOR_ERRCODE_STATE_ERR,
};

typedef void (*HAL_LM_SENSOR_IRQ_HANDLER_T)(enum HAL_LM_SENSOR_ID_T id, float temp, uint32_t status);
int hal_lm_sensor_enable(enum HAL_LM_SENSOR_ID_T id, HAL_LM_SENSOR_IRQ_HANDLER_T cb);
int hal_lm_sensor_disable(enum HAL_LM_SENSOR_ID_T id);

#endif // LM_SENSOR0_BASE

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __HAL_LM_SENSOR_H__

