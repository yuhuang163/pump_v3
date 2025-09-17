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
    #include "mbedtls/config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_CIPHER_C)

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/md.h"
#include "mbedtls/debug.h"
#include "mbedtls/cipher.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#define mbedtls_printf printf

#define OUT_BUF_SIZE                        256

int shanhai_trng_test(int verbose)
{
    int ret;
    size_t test_len = OUT_BUF_SIZE;
    static uint8_t test_data[OUT_BUF_SIZE];

    const char *pers = "crbg_20221223";
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_printf("TRNG-TEST: start\n");

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        goto exit;
    }

    if ((ret = mbedtls_ctr_drbg_random(&ctr_drbg, test_data, test_len)) != 0) {
        goto exit;
    }

    dump_buffer(test_data, test_len);
    mbedtls_printf("TRNG-TEST: passed\n");

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    if (ret && verbose != 0)
        mbedtls_printf("TRNG-TEST: failed\n");

    return ret;
}

#endif
