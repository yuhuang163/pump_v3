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

static const unsigned char md5_test_buf[7][81] = {
    { "" },
    { "a" },
    { "abc" },
    { "message digest" },
    { "abcdefghijklmnopqrstuvwxyz" },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
    { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" }
};

static const size_t md5_test_buflen[7] = {
    0, 1, 3, 14, 26, 62, 80
};

static const unsigned char md5_test_sum[7][16] = {
    {
        0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
        0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E
    },
    {
        0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
        0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61
    },
    {
        0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
        0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72
    },
    {
        0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
        0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0
    },
    {
        0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
        0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B
    },
    {
        0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
        0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F
    },
    {
        0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
        0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A
    }
};

int shanhai_md5_test(int verbose)
{
    int i, ret = 0;
    static unsigned char md5sum[16];

    mbedtls_md_type_t md_type;

    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;
    md_type = MBEDTLS_MD_MD5;

    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(md_type);
    ret = mbedtls_md_setup(&ctx, info, 0);

    for (i = 0; i < 7; i++) {
        if (verbose != 0)
            mbedtls_printf("MD5-TEST #%d: start\n", i + 1);

        if ((ret = mbedtls_md_starts(&ctx)) != 0)
            goto fail;

        if ((ret = mbedtls_md_update(&ctx, md5_test_buf[i], md5_test_buflen[i])) != 0)
            goto fail;

        if ((ret = mbedtls_md_finish(&ctx, md5sum)) != 0)
            goto fail;

        if (memcmp(md5sum, md5_test_sum[i], 16) != 0) {
            ret = 1;
            goto fail;
        }

        if (verbose != 0)
            mbedtls_printf("MD5-TEST #%d: passed\n", i + 1);

        return 0;
    }

fail:
    if (verbose != 0)
        mbedtls_printf("MD5-TEST #%d: failed\n", i + 1);

    return ret;
}

#endif
