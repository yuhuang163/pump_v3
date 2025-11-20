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

#include "hal_location.h"
#include "common.h"

#if defined(MBEDTLS_MD_C)

#include <string.h>

#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */

#define TEST_BLKSZ  300

#include "test_hash_data.h"

#if defined(MBEDTLS_MD5_C)

static PSRAM_BSS_LOC unsigned char md5sum[16];

static int mbedtls_hmac_md5_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if(verbose != 0)
        mbedtls_printf("  HMAC-MD5-TEST: start\n");

    mbedtls_md_init( &ctx );

    info = mbedtls_md_info_from_type(MBEDTLS_MD_MD5);
    if (!info) {
        ret = -1;
        goto exit;
    }

    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, key, key_len);
    if (ret != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_md_hmac_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    ret = mbedtls_md_hmac_finish(&ctx, md5sum);
    if (ret != 0)
        goto exit;

    mbedtls_printf("  HMAC-MD5-TEST: MD5SUM [");
    for (int i = 0; i < sizeof(md5sum); i++)
        mbedtls_printf("%02x ", md5sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(md5sum, test_hmac_md5_sum, sizeof(md5sum)) != 0) {
        mbedtls_printf("  HMAC-MD5-TEST: EXPECT [");
        for (int i = 0; i < sizeof(test_hmac_md5_sum); i++)
            mbedtls_printf("%02x ", test_hmac_md5_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("  HMAC-MD5-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}
#endif

#if defined(MBEDTLS_SHA1_C)

static PSRAM_BSS_LOC unsigned char sha1sum[20];

static int mbedtls_hmac_sha1_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if(verbose != 0)
        mbedtls_printf("  HMAC-SHA1-TEST: start\n");

    mbedtls_md_init( &ctx );

    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (!info) {
        ret = -1;
        goto exit;
    }

    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, key, key_len);
    if (ret != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_md_hmac_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    ret = mbedtls_md_hmac_finish(&ctx, sha1sum);
    if (ret != 0)
        goto exit;

    mbedtls_printf("  HMAC-SHA1-TEST: SHASUM [");
    for (int i = 0; i < sizeof(sha1sum); i++)
        mbedtls_printf("%02x ", sha1sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha1sum, test_hmac_sha1_sum, sizeof(sha1sum)) != 0) {
        mbedtls_printf("  HMAC-SHA1-TEST: EXPECT [");
        for (int i = 0; i < sizeof(test_hmac_sha1_sum); i++)
            mbedtls_printf("%02x ", test_hmac_sha1_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("  HMAC-SHA1-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}
#endif

#if defined(MBEDTLS_SHA256_C)

static PSRAM_BSS_LOC unsigned char sha256sum[32];

static int mbedtls_hmac_sha256_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if(verbose != 0)
        mbedtls_printf("  HMAC-SHA256-TEST: start\n");

    mbedtls_md_init( &ctx );

    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!info) {
        ret = -1;
        goto exit;
    }

    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, key, key_len);
    if (ret != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_md_hmac_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    ret = mbedtls_md_hmac_finish(&ctx, sha256sum);
    if (ret != 0)
        goto exit;

    mbedtls_printf("  HMAC-SHA256-TEST: SHASUM [");
    for (int i = 0; i < sizeof(sha256sum); i++)
        mbedtls_printf("%02x ", sha256sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha256sum, test_hmac_sha256_sum, sizeof(sha256sum)) != 0) {
        mbedtls_printf("  HMAC-SHA256-TEST: EXPECT [");
        for (int i = 0; i < sizeof(test_hmac_sha256_sum); i++)
            mbedtls_printf("%02x ", test_hmac_sha256_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("  HMAC-SHA256-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}
#endif

#if defined(MBEDTLS_SHA512_C)

static PSRAM_BSS_LOC unsigned char sha512sum[64];

static int mbedtls_hmac_sha512_test(int verbose)
{
    int ret = 0;
    unsigned char *key = test_hmac_key;
    int key_len = test_hmac_key_len;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;

    if(verbose != 0)
        mbedtls_printf("  HMAC-SHA512-TEST: start\n");

    mbedtls_md_init( &ctx );

    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    if (!info) {
        ret = -1;
        goto exit;
    }

    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, key, key_len);
    if (ret != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_md_hmac_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    ret = mbedtls_md_hmac_finish(&ctx, sha512sum);
    if (ret != 0)
        goto exit;

    mbedtls_printf("  HMAC-SHA512-TEST: SHASUM [");
    for (int i = 0; i < sizeof(sha512sum); i++)
        mbedtls_printf("%02x ", sha512sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha512sum, test_hmac_sha512_sum, sizeof(sha512sum)) != 0) {
        mbedtls_printf("  HMAC-SHA512-TEST: EXPECT [");
        for (int i = 0; i < sizeof(test_hmac_sha512_sum); i++)
            mbedtls_printf("%02x ", test_hmac_sha512_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if (verbose != 0)
        mbedtls_printf("  HMAC-SHA512-TEST: %s\n", ret ? "failed" : "passed");
    mbedtls_md_free(&ctx);
    return ret;
}
#endif

int mbedtls_hmac_test(int verbose)
{
    int ret = 0;
#if defined(MBEDTLS_MD5_C)
    ret |= mbedtls_hmac_md5_test(verbose);
#endif
#if defined(MBEDTLS_SHA1_C)
    ret |= mbedtls_hmac_sha1_test(verbose);
#endif
#if defined(MBEDTLS_SHA256_C)
    ret |= mbedtls_hmac_sha256_test(verbose);
#endif
#if defined(MBEDTLS_SHA512_C)
    ret |= mbedtls_hmac_sha512_test(verbose);
#endif
    return ret;
}

#endif
