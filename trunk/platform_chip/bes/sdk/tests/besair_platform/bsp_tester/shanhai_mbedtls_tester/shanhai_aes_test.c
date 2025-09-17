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
#include "mbedtls/aes.h"
#define mbedtls_printf printf

static const uint8_t iv[] = {
    0x51, 0xa1, 0xee, 0xf3, 0x3a, 0x8d, 0x99, 0x30, 0xd3, 0x59, 0xa6, 0x92, 0x7f, 0xf2, 0xaa, 0x5a,
};

static const uint8_t aes_test1[15] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x10, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee,
};

static const uint8_t aes_test2[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x10, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

static const uint8_t aes_test3[20] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x10, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33,
};

static const uint8_t key[] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
};

static uint8_t cipher_data[256];
static uint8_t plain_data[256];

static void add_pkcs_padding(unsigned char *output, size_t output_len, size_t data_len)
{
    unsigned char i;
    size_t padding_len = output_len - data_len;

    for (i = 0; i < padding_len; i++)
        output[data_len + i] = (unsigned char) padding_len;
}

static int shanhai_aes_test_base(int verbose, mbedtls_cipher_type_t cipher_type, unsigned int keybits, unsigned char *input, unsigned int in_len, char *test_name)
{
    int ret = 0;
    size_t olen_single;
    size_t olen;
    size_t plain_len = 0;
    size_t index;
    size_t cipher_len;
    size_t iv_len = sizeof(iv);
    unsigned char padding_len = 0;
    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *info;

    mbedtls_cipher_init(&ctx);
    memset(plain_data, 0, sizeof(plain_data));
    memset(cipher_data, 0, sizeof(cipher_data));
    memset(key, 0, sizeof(key));

    if (verbose != 0) {
        mbedtls_printf("%s len[%d] : start\n", test_name, in_len);
    }

    info = mbedtls_cipher_info_from_type(cipher_type);

    ret = mbedtls_cipher_setup(&ctx, info);
    if (ret) {
        mbedtls_printf("mbedtls_cipher_setup failed, ret:-0x%X", -ret);
        goto exit;
    }

    ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_ENCRYPT);
    if (ret) {
        mbedtls_printf("mbedtls_cipher_setkey failed, ret:-0x%X", -ret);
        goto exit;
    }

    if ((cipher_type == MBEDTLS_CIPHER_AES_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE3_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_BLOWFISH_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_SM4_ECB)) {

        memcpy(plain_data, input, in_len);
        plain_len = in_len;
        padding_len = in_len % ctx.cipher_info->block_size;
        if (padding_len) {
            padding_len = ctx.cipher_info->block_size - padding_len;
            plain_len += padding_len;

            add_pkcs_padding(plain_data, plain_len, in_len);
        }
#ifdef MBEDTLS_CIPHER_PADDING_PKCS7
        else {
            padding_len = ctx.cipher_info->block_size;
            plain_len += padding_len;

            add_pkcs_padding(plain_data, plain_len, in_len);
        }
#endif
        olen = 0;
        index = 0;
        while (index < plain_len) {
            ret = mbedtls_cipher_crypt(&ctx,
                                       (const unsigned char *)iv, iv_len,
                                       (const unsigned char *)(plain_data + index), ctx.cipher_info->block_size,
                                       cipher_data + index, &olen_single);
            if (ret) {
                mbedtls_printf("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
                goto exit;
            }

            index += ctx.cipher_info->block_size;
            olen += olen_single;
        }
    } else {
        ret = mbedtls_cipher_crypt(&ctx,
                                   (const unsigned char *)iv, iv_len,
                                   (const unsigned char *)input, in_len,
                                   cipher_data, &olen);
        if (ret) {
            mbedtls_printf("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
            goto exit;
        }
    }

    cipher_len = olen;
    ret = mbedtls_cipher_setkey(&ctx, key, keybits, MBEDTLS_DECRYPT);
    if (ret) {
        mbedtls_printf("mbedtls_cipher_setkey failed, ret:-0x%X", -ret);
        goto exit;
    }

    if ((cipher_type == MBEDTLS_CIPHER_AES_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE3_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_BLOWFISH_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_SM4_ECB)) {

        memset(plain_data, 0, plain_len);

        index = 0;
        olen = 0;
        while (index < cipher_len) {
            ret = mbedtls_cipher_crypt(&ctx,
                                       (const unsigned char *)iv, iv_len,
                                       (const unsigned char *)(cipher_data + index), ctx.cipher_info->block_size,
                                       plain_data + index, &olen_single);
            if (ret) {
                mbedtls_printf("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
                goto exit;
            }

            index += ctx.cipher_info->block_size;
            olen += olen_single;
        }
        if (olen > padding_len) {
            olen -= padding_len;
        }
    } else {
        ret = mbedtls_cipher_crypt(&ctx,
                                   (const unsigned char *)iv, iv_len,
                                   (const unsigned char *)cipher_data, cipher_len,
                                   plain_data, &olen);
        if (ret) {
            mbedtls_printf("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
            goto exit;
        }
    }

    ret = memcmp((void *)plain_data, (void *)input, olen);
    if (ret) {
        ret = 1;
        mbedtls_printf("%s len[%d] : failed\n", test_name, in_len);
        goto exit;
    }

exit:
    mbedtls_cipher_free(&ctx);
    if (!ret && verbose != 0) {
        mbedtls_printf("%s len[%d] : passed\n", test_name, in_len);
    }
    return ret;
}

int shanhai_aes_test(int verbose)
{
    int ret = 0;
    int i;
    unsigned char *test_data[3] = {aes_test1, aes_test2, aes_test3};
    unsigned int test_len[3] = {sizeof(aes_test1), sizeof(aes_test2), sizeof(aes_test3)};

    for (i = 0; i < 3; i++) {
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_128_ECB, 128, test_data[i], test_len[i], "AES-TEST-ECB-128");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_192_ECB, 192, test_data[i], test_len[i], "AES-TEST-ECB-192");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_256_ECB, 256, test_data[i], test_len[i], "AES-TEST-ECB-256");

        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_128_CBC, 128, test_data[i], test_len[i], "AES-TEST-CBC-128");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_192_CBC, 192, test_data[i], test_len[i], "AES-TEST-CBC-192");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_256_CBC, 256, test_data[i], test_len[i], "AES-TEST-CBC-256");

        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_128_CTR, 128, test_data[i], test_len[i], "AES-TEST-CTR-128");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_192_CTR, 192, test_data[i], test_len[i], "AES-TEST-CTR-192");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_256_CTR, 256, test_data[i], test_len[i], "AES-TEST-CTR-256");

        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_128_OFB, 128, test_data[i], test_len[i], "AES-TEST-OFB-128");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_192_OFB, 192, test_data[i], test_len[i], "AES-TEST-OFB-192");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_256_OFB, 256, test_data[i], test_len[i], "AES-TEST-OFB-256");

        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_128_CFB128, 128, test_data[i], test_len[i], "AES-TEST-CFB128-128");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_192_CFB128, 192, test_data[i], test_len[i], "AES-TEST-CFB128-192");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_AES_256_CFB128, 256, test_data[i], test_len[i], "AES-TEST-CFB128-256");

        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_SM4_ECB, 128, test_data[i], test_len[i], "AES-TEST-SM4_ECB");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_SM4_CBC, 128, test_data[i], test_len[i], "AES-TEST-SM4_CBC");
        ret |= shanhai_aes_test_base(verbose, MBEDTLS_CIPHER_SM4_CTR, 128, test_data[i], test_len[i], "AES-TEST-SM4_CTR");
    }
}

#endif
