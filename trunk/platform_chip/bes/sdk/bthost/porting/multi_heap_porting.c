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
#include "multi_heap_platform.h"
#include "multi_heap.h"
#include "multi_heap_internal.h"
#include "multi_heap_porting.h"

uint32_t bt_multi_heap_lock(void *lock)
{
    return MULTI_HEAP_LOCK(lock);
}

void bt_multi_heap_unlock(void *lock, uint32_t flags)
{
    MULTI_HEAP_UNLOCK(lock, flags);
}

