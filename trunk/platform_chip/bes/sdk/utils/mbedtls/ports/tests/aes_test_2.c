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

#if defined(MBEDTLS_AES_C)

#include <string.h>

#include "mbedtls/aes.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"
#if defined(MBEDTLS_PADLOCK_C)
#include "padlock.h"
#endif
#if defined(MBEDTLS_AESNI_C)
#include "aesni.h"
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */


#define AES_ECB_KEY "ECBTEST987654321"
#define AES_OFB_KEY "OFBTEST123456789"
#define AES_CBC_KEY "CBCTEST987654321"
#define AES_KEY_LEN 16

#define AES_TEST_DATA_LEN 64
static PSRAM_NC_DATA_LOC uint8_t aes_test_data[AES_TEST_DATA_LEN] ALIGNED(32) = {
    0xff, 0x01, 0x80, 0x0f, 0x5a, 0x11, 0x00, 0xff, 0xfe, 0x02, 0x70, 0xf0, 0xa5, 0x22, 0x00, 0xee, 0xfd, 0x03, 0x60, 0x87, 0x5a, 0x33,
    0x00, 0xdd, 0xfc, 0x04, 0x50, 0x78, 0xa5, 0x44, 0x00, 0xcc, 0xfb, 0x05, 0x40, 0xc3, 0x5a, 0x55, 0x00, 0xbb, 0xfa, 0x06, 0x30, 0x3c,
    0xa5, 0x66, 0x00, 0xaa, 0xf9, 0x07, 0x20, 0xe1, 0x5a, 0x77, 0x00, 0x99, 0xf8, 0x08, 0x10, 0x1e, 0xa5, 0x88, 0x00, 0x88,
};

#define AES_TEST_ECB_LEN 16
#define AES_TEST_OFB_LEN AES_TEST_DATA_LEN
#define AES_TEST_CBC_LEN AES_TEST_DATA_LEN

#if defined(MBEDTLS_CIPHER_MODE_ECB)

static PSRAM_NC_BSS_LOC uint8_t ecb_enc_buf[AES_TEST_ECB_LEN] ALIGNED(32);
static PSRAM_NC_BSS_LOC uint8_t ecb_dec_buf[AES_TEST_ECB_LEN] ALIGNED(32);

static int mbedtls_aes_ecb_test(int verbose)
{
    int ret = 0;
    uint8_t *enc_buf = ecb_enc_buf;
    uint8_t *dec_buf = ecb_dec_buf;
    uint8_t key[AES_KEY_LEN];
    mbedtls_aes_context aes_ctx;

    memcpy(key, AES_ECB_KEY, AES_KEY_LEN);

    if (verbose)
        mbedtls_printf("  AES-TEST-2-ECB: start\n");

    mbedtls_aes_init(&aes_ctx);

    for (int n = 0; n < 1000; n++) {
        memset(enc_buf, 0, AES_TEST_ECB_LEN);
        memset(dec_buf, 0, AES_TEST_ECB_LEN);

        mbedtls_aes_setkey_enc(&aes_ctx, key, AES_KEY_LEN * 8);
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, aes_test_data, enc_buf);
        ret = 0;
        for (int i = 0; i < AES_TEST_ECB_LEN; i++) {
            if (aes_test_data[i] != enc_buf[i] && enc_buf[i] != 0)
                ret++;
        }
        if (ret < 8)
            goto exit;

        mbedtls_aes_setkey_dec(&aes_ctx, key, AES_KEY_LEN * 8);
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, enc_buf, dec_buf);
        ret = 0;
        for (int i = 0; i < AES_TEST_ECB_LEN; i++) {
            if (aes_test_data[i] != dec_buf[i])
                ret++;
        }
        if (ret > 0)
            goto exit;
    }

exit:
    mbedtls_aes_free(&aes_ctx);
    if (verbose)
        mbedtls_printf("  AES-TEST-2-ECB: %s\n", ret ? "failed" : "passed");
    return (ret ? -1 : 0);
}

#endif

#if defined(MBEDTLS_CIPHER_MODE_OFB)

static PSRAM_NC_BSS_LOC uint8_t ofb_enc_buf[AES_TEST_OFB_LEN] ALIGNED(32);
static PSRAM_NC_BSS_LOC uint8_t ofb_dec_buf[AES_TEST_OFB_LEN] ALIGNED(32);

static int mbedtls_aes_ofb_test(int verbose)
{
    int ret = 0;
    uint8_t key[AES_KEY_LEN];
    uint8_t iv[16];
    size_t iv_off = 1;
    uint8_t *enc_buf = ofb_enc_buf;
    uint8_t *dec_buf = ofb_dec_buf;
    mbedtls_aes_context aes_ctx;

    memcpy(key, AES_OFB_KEY, AES_KEY_LEN);

    if (verbose)
        mbedtls_printf("  AES-TEST-2-OFB: start\n");

    mbedtls_aes_init(&aes_ctx);

    for (int n = 0; n < 1000; n++) {
        memset(enc_buf, 0, AES_TEST_OFB_LEN);
        memset(dec_buf, 0, AES_TEST_OFB_LEN);

        memset(iv, 0x1, 16);
        iv_off = 1;
        mbedtls_aes_setkey_enc(&aes_ctx, key, AES_KEY_LEN * 8);
        ret = mbedtls_aes_crypt_ofb(&aes_ctx, AES_TEST_OFB_LEN, &iv_off, iv, aes_test_data, enc_buf);
        if (ret)
            goto exit;

        memset(iv, 0x1, 16);
        iv_off = 1;
        mbedtls_aes_setkey_dec(&aes_ctx, key, AES_KEY_LEN*8);
        mbedtls_aes_crypt_ofb(&aes_ctx, AES_TEST_OFB_LEN, &iv_off, iv, enc_buf, dec_buf);
        ret = 0;
        for (int i = 0; i < AES_TEST_OFB_LEN; i++) {
            if (aes_test_data[i] != dec_buf[i])
                ret++;
        }
        if (ret > 0)
            goto exit;
    }

exit:
    mbedtls_aes_free(&aes_ctx);
    if (verbose)
        mbedtls_printf("  AES-TEST-2-OFB: %s\n", ret ? "failed" : "passed");
    return (ret ? -1 : 0);
}

#endif

#if defined(MBEDTLS_CIPHER_MODE_CBC)

static PSRAM_NC_BSS_LOC uint8_t cbc_enc_buf[AES_TEST_CBC_LEN] ALIGNED(32);
static PSRAM_NC_BSS_LOC uint8_t cbc_dec_buf[AES_TEST_CBC_LEN] ALIGNED(32);

static int mbedtls_aes_cbc_test(int verbose)
{
    int ret = 0;
    uint8_t key[AES_KEY_LEN];
    uint8_t iv[16];
    uint8_t *enc_buf = cbc_enc_buf;
    uint8_t *dec_buf = cbc_dec_buf;
    mbedtls_aes_context aes_ctx;

    memcpy(key, AES_CBC_KEY, AES_KEY_LEN);

    if (verbose)
        mbedtls_printf("  AES-TEST-2-CBC: start\n");

    mbedtls_aes_init(&aes_ctx);

    for (int n = 0; n < 1000; n++) {
        memset(enc_buf, 0, AES_TEST_CBC_LEN);
        memset(dec_buf, 0, AES_TEST_CBC_LEN);

        memset(iv, 0x1, 16);
        mbedtls_aes_setkey_enc(&aes_ctx, key, AES_KEY_LEN * 8);
        ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, AES_TEST_CBC_LEN, iv, aes_test_data, enc_buf);
        if (ret)
            goto exit;

        memset(iv, 0x1, 16);
        mbedtls_aes_setkey_dec(&aes_ctx, key, AES_KEY_LEN * 8);
        mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, AES_TEST_CBC_LEN, iv, enc_buf, dec_buf);
        ret = 0;
        for (int i = 0; i < AES_TEST_CBC_LEN; i++) {
            if (aes_test_data[i] != dec_buf[i])
                ret++;
        }
        if (ret > 0)
            goto exit;
    }

exit:
    mbedtls_aes_free(&aes_ctx);
    if (verbose)
        mbedtls_printf("  AES-TEST-2-CBC: %s\n", ret ? "failed" : "passed");
    return (ret ? -1 : 0);
}

#endif

int mbedtls_aes_test_2(int verbose)
{
    int ret = 0;
#if defined(MBEDTLS_CIPHER_MODE_ECB)
    ret |= mbedtls_aes_ecb_test(verbose);
#endif
#if defined(MBEDTLS_CIPHER_MODE_CBC)
    ret |= mbedtls_aes_cbc_test(verbose);
#endif
#if defined(MBEDTLS_CIPHER_MODE_OFB)
    ret |= mbedtls_aes_ofb_test(verbose);
#endif
    return ret;
}

#endif
