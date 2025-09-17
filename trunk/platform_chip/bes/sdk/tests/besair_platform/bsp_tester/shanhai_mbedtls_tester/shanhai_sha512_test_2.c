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
#include "test_hash_data.h"

#define mbedtls_printf printf

static unsigned char sha384sum[48];
static unsigned char sha512sum[64];

static int shanhai_sha384_test(int verbose)
{
    int ret = 0;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    md_type = MBEDTLS_MD_SHA384;
    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(md_type);
    ret = mbedtls_md_setup(&ctx, info, 0);

    if (verbose != 0)
        mbedtls_printf("SHA384-TEST-2: start\n");

    if ((ret = mbedtls_md_starts(&ctx)) != 0)
        goto exit;

    if ((ret = mbedtls_md_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_finish(&ctx, sha384sum)) != 0)
        goto exit;

    if (memcmp(sha384sum, test_hash_sha384_sum, 48) != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("SHA384-TEST-2: %s\n", ret ? "failed" : "passed");

    mbedtls_md_free(&ctx);
    return ret;
}

static int shanhai_sha512_test(int verbose)
{
    int ret = 0;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    md_type = MBEDTLS_MD_SHA512;
    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(md_type);
    ret = mbedtls_md_setup(&ctx, info, 0);

    if (verbose != 0)
        mbedtls_printf("SHA512-TEST-2: start\n");

    if ((ret = mbedtls_md_starts(&ctx)) != 0)
        goto exit;

    if ((ret = mbedtls_md_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_finish(&ctx, sha512sum)) != 0)
        goto exit;

    if (memcmp(sha512sum, test_hash_sha512_sum, 64) != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("SHA512-TEST-2: %s\n", ret ? "failed" : "passed");

    mbedtls_md_free(&ctx);
    return ret;
}

int shanhai_sha512_test_2(int verbose)
{
    int ret = 0;
    ret |= shanhai_sha384_test(verbose);
    ret |= shanhai_sha512_test(verbose);
    return ret;
}

#endif
