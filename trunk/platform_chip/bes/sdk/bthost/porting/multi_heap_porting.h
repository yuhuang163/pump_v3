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
#ifndef __MULTI_HEAP_PORTING_H__
#define __MULTI_HEAP_PORTING_H__

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t bt_multi_heap_lock(void *lock);
void bt_multi_heap_unlock(void *lock, uint32_t flags);

#if defined(__cplusplus)
}
#endif


#endif /* __MULTI_HEAP_PORTING_H__ */
