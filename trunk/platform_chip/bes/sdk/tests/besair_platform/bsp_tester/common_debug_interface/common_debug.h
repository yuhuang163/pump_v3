/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "analog.h"
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(pmu)

typedef void (*COMMON_DEBUG_HANDLER_T)(void);

typedef struct {
    const char *                name;
    COMMON_DEBUG_HANDLER_T      handler;
}common_debug_handler_t;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEBUG_H */