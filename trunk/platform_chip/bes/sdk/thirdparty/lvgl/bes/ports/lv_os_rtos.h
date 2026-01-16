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

#ifndef _LV_OS_RTOS_H_
#define _LV_OS_RTOS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_OS == LV_OS_CUSTOM

#include "cmsis_os2.h"
#ifdef KERNEL_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef osThreadId_t lv_thread_t;

#ifdef KERNEL_FREERTOS
typedef struct {
    BaseType_t xIsInitialized;
    SemaphoreHandle_t xMutex;
} lv_mutex_t;
#else
typedef osMutexId_t lv_mutex_t;
#endif

typedef osEventFlagsId_t lv_thread_sync_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_OS == LV_OS_CUSTOM */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LV_OS_RTOS_H_ */
