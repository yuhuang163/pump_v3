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

#if defined(MBEDTLS_RSA_C)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"
#include "mbedtls/memory_buffer_alloc.h"

#define KEY_SIZE 1024
#define EXPONENT 65537

static unsigned char buf[1024];
static unsigned char result[256];

static int shanhai_rsa_sign_verify_test(int verbose)
{
    int ret;

    mbedtls_printf("RSA-TEST-SIGN-VERIFY: start\n");

    const char *pers = "rsa_sign_test!!!";
    const char *msg = "rsa dubhe encrypt, 2023-11-06!!!";

    mbedtls_rsa_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_rsa_init(&ctx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        mbedtls_printf("mbedtls_ctr_drbg_seed failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_gen_key(&ctx, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE, EXPONENT);
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_gen_key failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_pkcs1_sign(&ctx, mbedtls_ctr_drbg_random,
                                 &ctr_drbg, MBEDTLS_RSA_PRIVATE,
                                 MBEDTLS_MD_SHA256, strlen(msg), (uint8_t *)msg, buf);
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_pkcs1_sign failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_pkcs1_verify(&ctx, mbedtls_ctr_drbg_random,
                                   &ctr_drbg, MBEDTLS_RSA_PUBLIC,
                                   MBEDTLS_MD_SHA256, strlen(msg),
                                   (uint8_t *)msg, buf);
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_pkcs1_verify failed\n");
        goto exit;
    }

    if (verbose != 0)
        mbedtls_printf("RSA-TEST-SIGN-VERIFY: passed\n");

    return 0;

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_rsa_free(&ctx);

    if (verbose != 0)
        mbedtls_printf("RSA-TEST-SIGN-VERIFY: faild\n");

    return 1;
}

int shanhai_rsa_enc_dec_test(int verbose)
{
    int ret;
    size_t olen;
    mbedtls_printf("RSA-TEST-ENCRYPT-DECRYPT: start\n");

    const char *pers = "rsa_enc_dec_test!!!";
    const char *msg = "rsa dubhe encrypt, 2023-11-06!!!";

    mbedtls_rsa_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_rsa_init(&ctx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        mbedtls_printf("mbedtls_ctr_drbg_seed failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_gen_key(&ctx, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE, EXPONENT);
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_gen_key failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_pkcs1_encrypt(&ctx, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, MBEDTLS_RSA_PUBLIC, strlen(msg),
                                    (const unsigned char *)msg, buf);
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_pkcs1_encrypt failed\n");
        goto exit;
    }

    ret = mbedtls_rsa_pkcs1_decrypt(&ctx, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, MBEDTLS_RSA_PRIVATE, &olen,
                                    buf, result, sizeof(result));
    if (ret != 0) {
        mbedtls_printf("mbedtls_rsa_pkcs1_decrypt failed\n");
        goto exit;
    }

    if ((ret = strcmp((char *)result, msg)) != 0) {
        mbedtls_printf("commpare failed\n");
    }

    if (verbose != 0)
        mbedtls_printf("RSA-TEST-ENCRYPT-DECRYPT: passed\n");

    return 0;

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_rsa_free(&ctx);

    if (verbose != 0)
        mbedtls_printf("RSA-TEST-ENCRYPT-DECRYPT: faild\n");

    return 1;
}

int shanhai_rsa_test(int verbose)
{
    int ret = 0;
    ret |= shanhai_rsa_sign_verify_test(verbose);

    ret |= shanhai_rsa_enc_dec_test(verbose);

    return ret;
}

#endif
