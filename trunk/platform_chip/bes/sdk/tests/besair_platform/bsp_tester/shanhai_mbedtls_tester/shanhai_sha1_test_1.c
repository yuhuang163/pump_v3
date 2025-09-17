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

static const unsigned char sha1_test_buf[3][57] = {
    { "abc" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" },
    { "" }
};

static const size_t sha1_test_buflen[3] = {
    3, 56, 1000
};

static unsigned char sha1_test_sum[3][20] = {
    {
        0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
        0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D
    },
    {
        0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
        0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1
    },
    {
        0x34, 0xAA, 0x97, 0x3C, 0xD4, 0xC4, 0xDA, 0xA4, 0xF6, 0x1E,
        0xEB, 0x2B, 0xDB, 0xAD, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6F
    }
};

static unsigned char buf[1024];
static unsigned char sha1sum[20];

int shanhai_sha1_test_1(int verbose)
{
    int i, j, buflen, ret = 0;
    mbedtls_md_type_t md_type;

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;
    md_type = MBEDTLS_MD_SHA1;

    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(md_type);
    ret = mbedtls_md_setup(&ctx, info, 0);
    if (ret)
        goto fail;

    /*SHA-1 Start*/
    for (i = 0; i < 3; i++) {
        if (verbose != 0)
            mbedtls_printf("SHA-TEST-1 test #%d: start\n", i + 1);

        ret = mbedtls_md_starts(&ctx);
        if (ret) {
            goto fail;
        }

        if (i == 2) {
            memset(buf, 'a', buflen = 1000);

            for (j = 0; j < 1000; j++) {
                ret = mbedtls_md_update(&ctx, buf, buflen);
                if (ret != 0)
                    goto fail;
            }
        } else {
            ret = mbedtls_md_update(&ctx, sha1_test_buf[i],
                                    sha1_test_buflen[i]);
            if (ret != 0)
                goto fail;
        }

        if ((ret = mbedtls_md_finish(&ctx, sha1sum)) != 0)
            goto fail;

        if (memcmp(sha1sum, sha1_test_sum[i], 20) != 0) {
            ret = 1;
            goto fail;
        }

        if (verbose != 0)
            mbedtls_printf("SHA1-TEST-1 test #%d: passed\n", i + 1);
    }

    goto exit;

fail:
    if (verbose != 0)
        mbedtls_printf("SHA1-TEST-1 test #%d: failed\n", i + 1);

exit:
    mbedtls_md_free(&ctx);
}

#endif
