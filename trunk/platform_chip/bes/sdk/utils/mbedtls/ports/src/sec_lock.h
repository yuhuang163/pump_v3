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
#ifndef __SEC_LOCK_H__
#define __SEC_LOCK_H__

typedef void *sec_lock_t;

#define SEC_LOCK_DEC(x) static sec_lock_t x = NULL

int sec_alt_lock_init(sec_lock_t *lock);
void sec_alt_lock_free(sec_lock_t *lock);

int sec_alt_lock(sec_lock_t lock);
int sec_alt_unlock(sec_lock_t lock);

#endif
