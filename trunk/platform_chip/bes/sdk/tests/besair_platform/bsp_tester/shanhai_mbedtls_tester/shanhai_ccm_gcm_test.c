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
#include "mbedtls/debug.h"
#include "mbedtls/cipher.h"

#define mbedtls_printf printf

static const uint8_t iv[] = {
    0x51, 0xa1, 0xee, 0xf3,
    0x3a, 0x8d, 0x99, 0x30,
    0xd3, 0x59, 0xa6, 0x92,
};

static const uint8_t key[] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
};

static const uint8_t add_data[] = {
    0x55, 0x12, 0xef, 0x26, 0x97, 0xec, 0x18, 0x58, 0xc2, 0xa8, 0xbc, 0x22, 0xff, 0x25, 0x92, 0xaa,
};

static const char test_data[] = {
    0xe3, 0xb2, 0x01, 0xa9, 0xf5, 0xb7, 0x1a, 0x7a,
    0x9b, 0x1c, 0xea, 0xec, 0xcd, 0x97, 0xe7, 0x0b,
    0x61, 0x76, 0xaa, 0xd9, 0xa4, 0x42, 0x8a, 0xa5,
    0x48, 0x43, 0x92, 0xfb, 0xc1, 0xb0, 0x99, 0x51
};

#define OUT_BUF_SIZE                        256

static int shanhai_ccm_test(int verbose)
{
    int ret;
    size_t olen = 0;
    size_t cipher_len;
    uint8_t tag_buf[16];
    static uint8_t plain_data[OUT_BUF_SIZE];
    static uint8_t cipher_data[OUT_BUF_SIZE];
    uint8_t i;
    uint16_t keybits;

    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *info;
    mbedtls_cipher_type_t cipher_type;

    mbedtls_cipher_init(&ctx);

    for (i = 0; i < 3; i++) {
        keybits = 128 + i * 64;

        mbedtls_printf("AES-%3u-CCM-TEST: start\n", keybits);

        switch (keybits) {
            case 128:
                cipher_type = MBEDTLS_CIPHER_AES_128_CCM;
                break;
            case 192:
                cipher_type = MBEDTLS_CIPHER_AES_192_CCM;
                break;
            case 256:
                cipher_type = MBEDTLS_CIPHER_AES_256_CCM;
                break;
            default:
                break;
        }

        info = mbedtls_cipher_info_from_type(cipher_type);

        if ((ret = mbedtls_cipher_setup(&ctx, info)) != 0) {
            goto exit;
        }

        if ((ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_ENCRYPT)) != 0) {
            goto exit;
        }

        ret = mbedtls_cipher_auth_encrypt(&ctx,
                                          (const unsigned char *)iv, sizeof(iv) / sizeof(iv[0]),
                                          (const unsigned char *)add_data, sizeof(add_data) / sizeof(add_data[0]),
                                          (const unsigned char *)test_data, sizeof(test_data) / sizeof(test_data[0]),
                                          cipher_data, &olen,
                                          tag_buf, sizeof(tag_buf));
        if (ret != 0) {
            goto exit;
        }

        cipher_len = olen;

        if ((ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_DECRYPT)) != 0) {
            goto exit;
        }

        ret = mbedtls_cipher_auth_decrypt(&ctx,
                                          (const unsigned char *)iv, sizeof(iv) / sizeof(iv[0]),
                                          (const unsigned char *)add_data, sizeof(add_data) / sizeof(add_data[0]),
                                          (const unsigned char *)cipher_data, cipher_len,
                                          plain_data, &olen,
                                          tag_buf, sizeof(tag_buf));
        if (ret != 0) {
            goto exit;
        }

        if (memcmp(plain_data, test_data, olen)) {
            goto exit;
        }

        if (verbose != 0)
            mbedtls_printf("AES-%3u-CCM-TEST: passed\n", keybits);

        mbedtls_cipher_free(&ctx);
    }
    return 0;
exit:
    mbedtls_cipher_free(&ctx);
    mbedtls_printf("AES-%3u-CCM-TEST: failed\n", keybits);
    return 1;
}

static int shanhai_gcm_test(int verbose)
{
    int ret;
    size_t olen = 0;
    size_t cipher_len;
    uint8_t tag_buf[16];
    static uint8_t plain_data[OUT_BUF_SIZE];
    static uint8_t cipher_data[OUT_BUF_SIZE];
    uint8_t i;
    uint16_t keybits;

    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *info;
    mbedtls_cipher_type_t cipher_type;

    mbedtls_cipher_init(&ctx);

    for (i = 0; i < 3; i++) {
        keybits = 128 + i * 64;

        mbedtls_printf("AES-%3u-GCM-TEST: start\n", keybits);

        switch (keybits) {
            case 128:
                cipher_type = MBEDTLS_CIPHER_AES_128_GCM;
                break;
            case 192:
                cipher_type = MBEDTLS_CIPHER_AES_192_GCM;
                break;
            case 256:
                cipher_type = MBEDTLS_CIPHER_AES_256_GCM;
                break;
            default:
                break;
        }

        info = mbedtls_cipher_info_from_type(cipher_type);

        if ((ret = mbedtls_cipher_setup(&ctx, info)) != 0) {
            goto exit;
        }

        if ((ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_ENCRYPT)) != 0) {
            goto exit;
        }

        ret = mbedtls_cipher_auth_encrypt(&ctx,
                                          (const unsigned char *)iv, sizeof(iv) / sizeof(iv[0]),
                                          (const unsigned char *)add_data, sizeof(add_data) / sizeof(add_data[0]),
                                          (const unsigned char *)test_data, sizeof(test_data) / sizeof(test_data[0]),
                                          cipher_data, &olen,
                                          tag_buf, sizeof(tag_buf));
        if (ret != 0) {
            goto exit;
        }

        cipher_len = olen;

        if ((ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_DECRYPT)) != 0) {
            goto exit;
        }

        ret = mbedtls_cipher_auth_decrypt(&ctx,
                                          (const unsigned char *)iv, sizeof(iv) / sizeof(iv[0]),
                                          (const unsigned char *)add_data, sizeof(add_data) / sizeof(add_data[0]),
                                          (const unsigned char *)cipher_data, cipher_len,
                                          plain_data, &olen,
                                          tag_buf, sizeof(tag_buf));
        if (ret != 0) {
            goto exit;
        }

        if (memcmp(plain_data, test_data, olen)) {
            goto exit;
        }

        if (verbose != 0)
            mbedtls_printf("AES-%3u-GCM-TEST: passed\n", keybits);

        mbedtls_cipher_free(&ctx);
    }
    return 0;
exit:
    mbedtls_cipher_free(&ctx);
    mbedtls_printf("AES-%3u-GCM-TEST: failed\n", keybits);
    return 1;
}

int shanhai_ccm_gcm_test(int verbose)
{
    int ret;

    ret |= shanhai_ccm_test(verbose);

    ret |= shanhai_gcm_test(verbose);

    return ret;
}

#endif
