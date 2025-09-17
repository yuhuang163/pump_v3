/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __BTS_UTILS_H__
#define __BTS_UTILS_H__

#include <string.h>
#include <bts_defines.h>

#if defined(__cplusplus)
extern "C" {
#endif

static inline int bts_addr_cmp(const bts_dev_addr_t *ba1, const bts_dev_addr_t *ba2)
{
    return memcmp(ba1, ba2, sizeof(bts_dev_addr_t));
}

static inline void bts_addr_cpy(bts_dev_addr_t *dst, const bts_dev_addr_t *src)
{
    memcpy(dst, src, sizeof(bts_dev_addr_t));
}

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_UTILS_H__ */