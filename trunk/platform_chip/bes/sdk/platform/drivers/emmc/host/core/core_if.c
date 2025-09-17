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
#include "host.h"
#include "card.h"
#include "core.h"
#include "error.h"
#include "string.h"
#include "emmc_dbg.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"
#ifdef EMMC_USE_STDLIB
    #include "stdlib.h"
#endif

#ifdef EMMC_USE_HEAP
    #include "heap_api.h"

    static multi_heap_handle_t g_heap_handle;
    static uint32_t g_heap_buff[EMMC_HEAP_BUFF_SIZE / 4];
#endif

int is_sle(void)
{
#if 1
    return 0; //not simulation platform
#else
    return 1; //simulation platform
#endif
}

void sdhciAccessLayerRangeError(const char *f, ...)
{
    HAL_EMMC_TRACE(1, "%s: %s", __FUNCTION__, f);
}

#ifdef EMMC_USE_HEAP
void emmc_heap_init(void *start, size_t size)
{
    g_heap_handle = heap_register(start, size);

    HAL_EMMC_TRACE(1, "%s:%d, g_heap_handle=0x%x, start=0x%x, size=%d",
                   __FUNCTION__, __LINE__, (int)g_heap_handle, (int)start, (int)size);
}

void *emmc_heap_malloc(size_t size)
{
    void *ptr = NULL;

    HAL_EMMC_TRACE(1, "%s:%d, size=%d", __FUNCTION__, __LINE__, (int)size);
    ptr = heap_malloc(g_heap_handle, size);
#ifdef EMMC_ASSERT
    HAL_EMMC_ASSERT(ptr != NULL, "%s:%d, malloc failed, size=%d", __FUNCTION__, __LINE__, (int)size);
#else
    if (ptr == NULL) {
        HAL_EMMC_ERROR(0, "%s, error:ptr can not be NULL", __FUNCTION__);
        return NULL;
    }
#endif
    HAL_EMMC_TRACE(1, "malloc done, ptr=0x%x, size=%d", (int)ptr, (int)size);

    return ptr;
}

void emmc_heap_free(void *ptr)
{
    heap_free(g_heap_handle, ptr);
    HAL_EMMC_TRACE(1, "%s:%d, ptr=0x%x, free done", __FUNCTION__, __LINE__, (int)ptr);
}
#endif

void emmc_core_if_init(void)
{
    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

#ifdef EMMC_USE_HEAP
    emmc_heap_init((void *)g_heap_buff, sizeof(g_heap_buff));
#endif
}
