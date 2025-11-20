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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif


#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include "string.h"
#include "hal_trng.h"
#include "sec_lock.h"
#include "random.h"

SEC_LOCK_DEC(trng_lock);

#define TRNG_LEN  HAL_TRNG_DATA_LEN

int mbedtls_hardware_poll(void *Data, unsigned char *Output, size_t Len, size_t *oLen)
{
    int ret = 0;
    size_t total = 0;
    unsigned char *p = Output;

    if (!trng_lock)
        sec_alt_lock_init(&trng_lock);

    sec_alt_lock(&trng_lock);
    while (total < Len) {
        size_t sz = (Len - total > TRNG_LEN) ? TRNG_LEN : (Len - total);
        if (random_get_data(p, sz) == 0) {
            total += sz;
            p += sz;
        } else {
            ret = -1;
            goto exit;
        }
    }
    *oLen = total;

exit:
    sec_alt_unlock(&trng_lock);
    return ret;
}

#endif

