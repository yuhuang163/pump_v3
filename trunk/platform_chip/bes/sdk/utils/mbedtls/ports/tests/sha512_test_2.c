/*
 *  FIPS-180-2 compliant SHA-384/512 implementation
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*
 *  The SHA-512 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include "hal_location.h"
#include "common.h"

#if defined(MBEDTLS_SHA512_C)

#include "mbedtls/sha512.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include <string.h>

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */

#define TEST_BLKSZ  300

#include "test_hash_data.h"

static PSRAM_BSS_LOC unsigned char sha384sum[48];
static PSRAM_BSS_LOC unsigned char sha512sum[64];

static int mbedtls_sha384_test(int verbose)
{
    int ret = 0;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);

    if(verbose != 0)
        mbedtls_printf("  SHA384-TEST-2: start\n");

    if((ret = mbedtls_sha512_starts(&ctx, 1)) != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_sha512_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    if((ret = mbedtls_sha512_finish(&ctx, sha384sum)) != 0)
        goto exit;

    mbedtls_printf("  SHA384-TEST-2: SHASUM [");
    for (int i = 0; i < sizeof(sha384sum); i++)
        mbedtls_printf("%02x ", sha384sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha384sum, test_hash_sha384_sum, 48) != 0) {
        mbedtls_printf("  SHA384-TEST-2:  EXPECT [");
        for (int i = 0; i < sizeof(test_hash_sha384_sum); i++)
            mbedtls_printf("%02x ", test_hash_sha384_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if(verbose != 0)
        mbedtls_printf("  SHA384-TEST-2: %s\n", ret ? "failed" : "passed");

    mbedtls_sha512_free(&ctx);
    return ret;
}

static int mbedtls_sha512_test(int verbose)
{
    int ret = 0;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_sha512_context ctx;

    mbedtls_sha512_init(&ctx);

    if(verbose != 0)
        mbedtls_printf("  SHA512-TEST-2: start\n");

    if((ret = mbedtls_sha512_starts(&ctx, 0)) != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_sha512_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    if((ret = mbedtls_sha512_finish(&ctx, sha512sum)) != 0)
        goto exit;

    mbedtls_printf("  SHA512-TEST-2: SHASUM [");
    for (int i = 0; i < sizeof(sha512sum); i++)
        mbedtls_printf("%02x ", sha512sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha512sum, test_hash_sha512_sum, 64) != 0) {
        mbedtls_printf("  SHA512-TEST-2:  EXPECT [");
        for (int i = 0; i < sizeof(test_hash_sha512_sum); i++)
            mbedtls_printf("%02x ", test_hash_sha512_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if(verbose != 0)
        mbedtls_printf("  SHA512-TEST-2: %s\n", ret ? "failed" : "passed");

    mbedtls_sha512_free(&ctx);
    return ret;
}

int mbedtls_sha512_test_2(int verbose)
{
    int ret = 0;
    ret |= mbedtls_sha512_test(verbose);
    ret |= mbedtls_sha384_test(verbose);
    return ret;
}

#endif /* MBEDTLS_SHA512_C */
