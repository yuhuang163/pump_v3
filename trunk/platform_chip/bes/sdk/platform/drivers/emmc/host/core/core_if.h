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
#ifndef __MMC_CORE_IF_H__
#define __MMC_CORE_IF_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "emmc_platform.h"

int is_sle(void);

void emmc_heap_init(void *start, size_t size);

void *emmc_heap_malloc(size_t size);

void emmc_heap_free(void *ptr);

void emmc_core_if_init(void);

#ifdef __cplusplus
}
#endif

#endif
