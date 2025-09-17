/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "hal_trace.h"
#include "heap_api.h"

extern uint8_t __aon_mbx_heap_start__[];
extern uint8_t __aon_mbx_heap_end__[];

static multi_heap_handle_t mbx_heap;

int aon_mbx_heap_init(void)
{
    void *start = __aon_mbx_heap_start__;
    size_t size = __aon_mbx_heap_end__ - __aon_mbx_heap_start__;

    mbx_heap = multi_heap_register(start, size);
    ASSERT(mbx_heap, "%s: Failed to register aon mbx heap: start=%p size=0x%08X", __func__, start, size);
    return 0;
}

void *aon_mbx_heap_malloc(size_t size)
{
    return multi_heap_malloc(mbx_heap, size);
}

void aon_mbx_heap_free(void *p)
{
    multi_heap_free(mbx_heap, p);
}

void *aon_mbx_heap_realloc(void *p, size_t size)
{
    return multi_heap_realloc(mbx_heap, p, size);
}

bool aon_mbx_heap_check(bool print_errors)
{
    return multi_heap_check(mbx_heap, print_errors);
}

void aon_mbx_heap_dump(void)
{
    multi_heap_dump(mbx_heap);
}

size_t aon_mbx_heap_free_size(void)
{
    return multi_heap_free_size(mbx_heap);
}
