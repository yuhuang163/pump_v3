/*
 *  FIPS-180-1 compliant SHA-1 implementation
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
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#include "hal_location.h"
#include "common.h"

#if defined(MBEDTLS_SHA1_C)

#include "mbedtls/sha1.h"
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

static PSRAM_BSS_LOC unsigned char sha1sum[20];

int mbedtls_sha1_test_2(int verbose)
{
    int ret = 0;
    unsigned char *data = test_hash_data;
    int total_sz = sizeof(test_hash_data);
    int update_sz = 0;
    mbedtls_sha1_context ctx;

    mbedtls_sha1_init(&ctx);

    if(verbose != 0)
        mbedtls_printf("  SHA1-TEST-2: start\n");

    if((ret = mbedtls_sha1_starts(&ctx)) != 0)
        goto exit;

    while (update_sz < total_sz) {
        int sz = (total_sz - update_sz > TEST_BLKSZ) ? TEST_BLKSZ : (total_sz - update_sz);
        ret = mbedtls_sha1_update(&ctx, data + update_sz, sz);
        if(ret != 0)
            goto exit;
        update_sz += sz;
    }

    if((ret = mbedtls_sha1_finish(&ctx, sha1sum)) != 0)
        goto exit;

    mbedtls_printf("  SHA1-TEST-2: SHASUM [");
    for (int i = 0; i < sizeof(sha1sum); i++)
        mbedtls_printf("%02x ", sha1sum[i]);
    mbedtls_printf("]\n");

    if(memcmp(sha1sum, test_hash_sha1_sum, 20) != 0) {
        mbedtls_printf("  SHA1-TEST-2: EXPECT [");
        for (int i = 0; i < sizeof(test_hash_sha1_sum); i++)
            mbedtls_printf("%02x ", test_hash_sha1_sum[i]);
        mbedtls_printf("]\n");
        ret = -1;
        goto exit;
    }

exit:
    if(verbose != 0)
        mbedtls_printf("  SHA1-TEST-2: %s\n", ret ? "failed" : "passed");

    mbedtls_sha1_free(&ctx);
    return ret;
}

#endif /* MBEDTLS_SHA1_C */
