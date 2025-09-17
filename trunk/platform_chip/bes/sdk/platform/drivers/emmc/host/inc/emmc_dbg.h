/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __EMMC_DBG_H__
#define __EMMC_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMMC_DEBUG
    #define HAL_EMMC_TRACE(n, s, ...)   TRACE(n, s , ##__VA_ARGS__)
#else
    #define HAL_EMMC_TRACE(n, s, ...)   TRACE_DUMMY(n, s , ##__VA_ARGS__)
#endif

#ifdef EMMC_ASSERT
    #define HAL_EMMC_ASSERT(n, s, ...)  ASSERT(n, s , ##__VA_ARGS__)
#else
    #define HAL_EMMC_ASSERT(...)
#endif

#if 1
    #define HAL_EMMC_ERROR(n, s, ...)   TRACE(n, s , ##__VA_ARGS__)
#else
    #define HAL_EMMC_ERROR(n, s, ...)   TRACE_DUMMY(n, s , ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
