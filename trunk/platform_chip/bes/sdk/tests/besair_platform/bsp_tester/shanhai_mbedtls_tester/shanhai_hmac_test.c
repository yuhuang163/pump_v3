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

#define mbedtls_printf printf

#include "test_hash_data.h"

static unsigned char md5sum[16];

static int shanhai_hmac_md5_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if (verbose != 0) {
        mbedtls_printf("HMAC-MD5-TEST: start\n");
    }
    md_type = MBEDTLS_MD_MD5;
    mbedtls_md_init(&ctx);

    info = mbedtls_md_info_from_type(md_type);

    if ((ret = mbedtls_md_setup(&ctx, info, 1)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_starts(&ctx, key, key_len)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_finish(&ctx, md5sum)) != 0)
        goto exit;

    if (memcmp(md5sum, test_hmac_md5_sum, sizeof(md5sum))) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("HAMC-MD5-TEST: %s\n", ret ? "failed" : "passed");

    mbedtls_md_free(&ctx);
    return ret;
}

static unsigned char sha1sum[20];

static int shanhai_hmac_sha1_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if (verbose != 0) {
        mbedtls_printf("HMAC-SAH1-TEST: start\n");
    }
    md_type = MBEDTLS_MD_SHA1;
    mbedtls_md_init(&ctx);

    info = mbedtls_md_info_from_type(md_type);

    if ((ret = mbedtls_md_setup(&ctx, info, 1)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_starts(&ctx, key, key_len)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_finish(&ctx, sha1sum)) != 0)
        goto exit;

    if (memcmp(sha1sum, test_hmac_sha1_sum, sizeof(sha1sum))) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("HAMC-SHA1-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}

static unsigned char sha256sum[32];

static int shanhai_hmac_sha256_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if (verbose != 0) {
        mbedtls_printf("HMAC-SAH256-TEST: start\n");
    }
    md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);

    info = mbedtls_md_info_from_type(md_type);

    if ((ret = mbedtls_md_setup(&ctx, info, 1)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_starts(&ctx, key, key_len)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_finish(&ctx, sha256sum)) != 0)
        goto exit;

    if (memcmp(sha256sum, test_hmac_sha256_sum, sizeof(sha256sum)) != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("HAMC-SHA256-TEST: %s\n", ret ? "failed" : "passed");

    mbedtls_md_free(&ctx);
    return ret;
}

static unsigned char sha512sum[64];

static int shanhai_hmac_sha512_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);

    mbedtls_md_type_t md_type;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if (verbose != 0) {
        mbedtls_printf("HMAC-SAH512-TEST: start\n");
    }
    md_type = MBEDTLS_MD_SHA512;
    mbedtls_md_init(&ctx);

    info = mbedtls_md_info_from_type(md_type);

    if ((ret = mbedtls_md_setup(&ctx, info, 1)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_starts(&ctx, key, key_len)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_update(&ctx, data, total_sz)) != 0)
        goto exit;

    if ((ret = mbedtls_md_hmac_finish(&ctx, sha512sum)) != 0)
        goto exit;

    if (memcmp(sha512sum, test_hmac_sha512_sum, sizeof(sha512sum)) != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("HAMC-SHA512-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}

int shanhai_hmac_test(int verbose)
{
    int ret = 0;

    ret |= shanhai_hmac_md5_test(verbose);

    ret |= shanhai_hmac_sha1_test(verbose);

    ret |= shanhai_hmac_sha256_test(verbose);

    ret |= shanhai_hmac_sha512_test(verbose);

    return ret;
}

#endif
