/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#define SEC_SIMPLE_TEST_MODE    /*default PADDING_PKCS7*/
//#define SEC_MASS_DATA_TEST_MODE

#ifdef SEC_MASS_DATA_TEST_MODE
    /*
    mode 0: fixed length
    mode 1: sequence length
    mode 2: random length
    */
    //After the following two macros are closed, it is a fixed-length test mode
    //#define LOOP_SEQUENCE_TEST
    //#define LOOP_RANDOM_TEST

    #define CIPHER_DATA_SIZE                    (32 * 1024)
    #define MASS_DATA_BUF_SIZE                  (CIPHER_DATA_SIZE + AES_BLK_SIZE)
#endif

#define DEBUG_LEVEL                             1
#define AES_BLK_SIZE                            16
#define MAX_LOG_LEN                             32
#define CALC_RATE_TIME_BYTE                     64

#if defined(SEC_SIMPLE_TEST_MODE) && defined(SEC_MASS_DATA_TEST_MODE)
    #error "SEC_SIMPLE_TEST_MODE and SEC_MASS_DATA_TEST_MODE, the 2 macros can only open one of them"
#endif

#if defined(SEC_SIMPLE_TEST_MODE) && !defined(MBEDTLS_CIPHER_PADDING_PKCS7)
    #error "MBEDTLS_CIPHER_PADDING_PKCS7 macro is not defined"
#endif

#if defined(LOOP_SEQUENCE_TEST) && defined(LOOP_RANDOM_TEST)
    #error "LOOP_SEQUENCE_TEST and LOOP_RANDOM_TEST, the 2 macros can only open one of them"
#endif

#ifdef DEFAULT_BSS_IN_PSRAM
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_PSRAM_LOC
#else
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_LOC
#endif

#ifdef SEC_MASS_DATA_TEST_MODE
    #define OUT_BUF_SIZE                        MASS_DATA_BUF_SIZE
#else
    #define OUT_BUF_SIZE                        256
#endif

#if 1
    #define PRINT(s, ...)                       TRACE(0, s, ##__VA_ARGS__)
#else
    #define PRINT(s, ...)                       TRACE_DUMMY(0, s, ##__VA_ARGS__)
#endif

static void add_pkcs_padding(uint8_t *output, size_t output_len, size_t data_len)
{
    uint8_t i;
    size_t padding_len = output_len - data_len;

    for (i = 0; i < padding_len; i++)
        output[data_len + i] = (uint8_t) padding_len;
}

static int aes_encrypt_decrypt_test(mbedtls_cipher_type_t cipher_type,
                                    const uint8_t *key, size_t key_len,
                                    const uint8_t *iv, size_t iv_len,
                                    const uint8_t *input, size_t in_len)
{
    int ret;
    size_t olen = 0;
    size_t cipher_len;
    size_t time_start, time;
    SEC_ENG_BUF_LOC static uint8_t plain_data[OUT_BUF_SIZE];
    SEC_ENG_BUF_LOC static uint8_t cipher_data[OUT_BUF_SIZE];

    //for ecb mode
    size_t index;
    size_t plain_len = 0;
    size_t olen_single;
    uint8_t padding_len = 0;

    mbedtls_cipher_context_t ctx;
    const mbedtls_cipher_info_t *info;

    /* 1. init cipher structuer */
    mbedtls_cipher_init(&ctx);

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    /* 2. get info structuer from type */
    info = mbedtls_cipher_info_from_type(cipher_type);

    /* 3. setup cipher structuer */
    ret = mbedtls_cipher_setup(&ctx, info);
    if (ret) {
        PRINT("  ");
        PRINT("%s, line:%d", __func__, __LINE__);
        PRINT("mbedtls_cipher_setup failed, ret:-0x%X", -ret);
        goto __exit;
    }

    if ((cipher_type == MBEDTLS_CIPHER_AES_128_CBC) ||
        (cipher_type == MBEDTLS_CIPHER_AES_192_CBC) ||
        (cipher_type == MBEDTLS_CIPHER_AES_256_CBC)) {
#if defined(MBEDTLS_CIPHER_PADDING_PKCS7)
        ret = mbedtls_cipher_set_padding_mode(&ctx, MBEDTLS_PADDING_PKCS7);
#else
        #error "MBEDTLS_CIPHER_PADDING_PKCS7 is not defined"
#endif
        if (ret) {
            PRINT("  ");
            PRINT("%s, line:%d", __func__, __LINE__);
            PRINT("mbedtls_cipher_set_padding_mode failed, ret:-0x%X", -ret);
            goto __exit;
        }
    }

    PRINT("  ");
    PRINT("++++++encrypt start");
    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////encrypt///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /* 4. set key */
    ret = mbedtls_cipher_setkey(&ctx, key, key_len * 8, MBEDTLS_ENCRYPT);
    if (ret) {
        PRINT("  ");
        PRINT("%s, line:%d", __func__, __LINE__);
        PRINT("mbedtls_cipher_setkey failed, ret:-0x%X", -ret);
        goto __exit;
    }

    hal_cmu_simu_fail();
    /* 5. all-in-one encryption/decryption */
    if ((cipher_type == MBEDTLS_CIPHER_AES_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE3_ECB)) {

        memcpy(plain_data, input, in_len);
        plain_len = in_len;
        padding_len = in_len % ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size);
        if (padding_len) {
            padding_len = ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size) - padding_len;
            plain_len += padding_len;

            add_pkcs_padding(plain_data, plain_len, in_len);
        }
        PRINT("ecb padding_len:%d, in_len:%d, plain_len:%d", padding_len, in_len, plain_len);

        index = 0;
        time_start = hal_fast_sys_timer_get();
        while (index < plain_len) {
            ret = mbedtls_cipher_crypt(&ctx,
                                       (const uint8_t *)iv, iv_len,//useless
                                       (const uint8_t *)(plain_data + index), ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size),
                                       cipher_data + index, &olen_single);
            if (ret) {
                PRINT("  ");
                PRINT("%s, line:%d", __func__, __LINE__);
                PRINT("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
                goto __exit;
            }

            index += ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size);
            olen += olen_single;
        }
    } else {
        time_start = hal_fast_sys_timer_get();
        ret = mbedtls_cipher_crypt(&ctx,
                                   (const uint8_t *)iv, iv_len,
                                   (const uint8_t *)input, in_len,
                                   cipher_data, &olen);
        if (ret) {
            PRINT("  ");
            PRINT("%s, line:%d", __func__, __LINE__);
            PRINT("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
            goto __exit;
        }
    }

    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    hal_cmu_simu_pass();
    cipher_len = olen;
    PRINT("++++++encrypt end");

    /* encrypt show */
    PRINT("  ");
#ifdef SEC_SIMPLE_TEST_MODE
    PRINT("plaintext:%s", input);
#else
    if (in_len < MAX_LOG_LEN) {
        PRINT("input plaintext:");
        dump_buffer(input, in_len);
    }
#endif
    PRINT("block size:%d, CPU freq: %u", mbedtls_cipher_get_block_size(&ctx), hal_sys_timer_calc_cpu_freq(5, 0));
    PRINT("=== %s Encrypt Passed ===", mbedtls_cipher_get_name(&ctx));
    PRINT("=== time cost:%uus/%ubytes, %u.%uus/%ubytes",
          time,
          in_len,
#ifdef SEC_SIMPLE_TEST_MODE
          (time * CALC_RATE_TIME_BYTE) / in_len, (time * CALC_RATE_TIME_BYTE * 10) / in_len % 10,
#else
          //time / (in_len / CALC_RATE_TIME_BYTE), time * 10 / (in_len / CALC_RATE_TIME_BYTE) % 10,
          (time * CALC_RATE_TIME_BYTE) / in_len, (time * CALC_RATE_TIME_BYTE * 10) / in_len % 10,
#endif
          CALC_RATE_TIME_BYTE);

#ifdef SEC_SIMPLE_TEST_MODE
    PRINT("ciphertext: Start of dump,len:%d", olen);
    dump_buffer(cipher_data, olen);
    PRINT("ciphertext: End of dump");
#else
    if (olen < MAX_LOG_LEN) {
        PRINT("ciphertext: Start of dump,len:%d", olen);
        dump_buffer(cipher_data, olen);
        PRINT("ciphertext: End of dump");
    }
#endif

    PRINT("  ");
    PRINT("++++++decrypt start");
    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////decrypt///////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /* 4. set key */
    ret = mbedtls_cipher_setkey(&ctx, key, key_len * 8, MBEDTLS_DECRYPT);
    if (ret) {
        PRINT("  ");
        PRINT("%s, line:%d", __func__, __LINE__);
        PRINT("mbedtls_cipher_setkey failed, ret:-0x%X", -ret);
        goto __exit;
    }

    hal_cmu_simu_fail();
    /* 5. all-in-one encryption/decryption */
    if ((cipher_type == MBEDTLS_CIPHER_AES_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_AES_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_128_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_192_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_CAMELLIA_256_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE_ECB) ||
        (cipher_type == MBEDTLS_CIPHER_DES_EDE3_ECB)) {

        memset(plain_data, 0, plain_len);

        index = 0;
        olen = 0;
        time_start = hal_fast_sys_timer_get();
        while (index < cipher_len) {
            ret = mbedtls_cipher_crypt(&ctx,
                                       (const uint8_t *)iv, iv_len,//useless
                                       (const uint8_t *)(cipher_data + index), ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size),
                                       plain_data + index, &olen_single);
            if (ret) {
                PRINT("  ");
                PRINT("%s, line:%d", __func__, __LINE__);
                PRINT("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
                goto __exit;
            }

            index += ctx.MBEDTLS_PRIVATE(cipher_info)->MBEDTLS_PRIVATE(block_size);
            olen += olen_single;
        }
        if (olen > padding_len) {
            olen -= padding_len;
        }
    } else {
        time_start = hal_fast_sys_timer_get();
        ret = mbedtls_cipher_crypt(&ctx,
                                   (const uint8_t *)iv, iv_len,
                                   (const uint8_t *)cipher_data, cipher_len,
                                   plain_data, &olen);
        if (ret) {
            PRINT("  ");
            PRINT("%s, line:%d", __func__, __LINE__);
            PRINT("mbedtls_cipher_crypt failed, ret:-0x%X", -ret);
            goto __exit;
        }
    }

    time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    hal_cmu_simu_pass();
    PRINT("++++++decrypt end");

    /* decrypt show */
    PRINT("  ");
    ret = memcmp((void *)plain_data, (void *)input, olen);
    if (ret || olen != in_len) {
        PRINT("  ");
        PRINT("%s, line:%d", __func__, __LINE__);
        PRINT("*** Decrypt Test Failed ***, ret:%d, olen:%d, in_len:%d", ret, olen, in_len);

        if (olen < 1024) {
            PRINT("original plaintext:");
            dump_buffer(input, olen);
            PRINT("decrypted plaintext:");
            dump_buffer(plain_data, olen);
        }
        goto __exit;
    } else {
        PRINT("=== %s Decrypt Passed ===", mbedtls_cipher_get_name(&ctx));
    }

    PRINT("=== time cost:%uus/%ubytes, %u.%uus/%ubytes",
          time,
          cipher_len,
#ifdef SEC_SIMPLE_TEST_MODE
          (time * CALC_RATE_TIME_BYTE) / cipher_len, (time * CALC_RATE_TIME_BYTE * 10) / cipher_len % 10,
#else
          //time / (cipher_len / CALC_RATE_TIME_BYTE), time * 10 / (cipher_len / CALC_RATE_TIME_BYTE) % 10,
          (time * CALC_RATE_TIME_BYTE) / cipher_len, (time * CALC_RATE_TIME_BYTE * 10) / cipher_len % 10,
#endif
          CALC_RATE_TIME_BYTE);

#ifdef SEC_SIMPLE_TEST_MODE
    PRINT("plaintext:%s", plain_data);
#else
    if (olen < MAX_LOG_LEN) {
        PRINT("plaintext after decryption:");
        dump_buffer(plain_data, olen);
    }
#endif

__exit:
    /* 6. free cipher structure */
    mbedtls_cipher_free(&ctx);
    PRINT("  ");

    return ret;
}

/* Intialization Vector */
POSSIBLY_UNUSED static const uint8_t iv[] = {
    0xE8, 0xE7, 0x0F, 0x4C, 0x23, 0x78, 0x00, 0x2B, 0xEF, 0xD0, 0x2B, 0x43, 0x23, 0x70, 0xE1, 0xE7,
};

/* Private Key */
POSSIBLY_UNUSED static const uint8_t key_128[] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
};

POSSIBLY_UNUSED static const uint8_t key_192[] = {
    0x94, 0x0F, 0x02, 0x10, 0x91, 0x0F, 0x02, 0x10, 0x8D, 0x0F, 0x02, 0x10, 0x2F, 0x28, 0x4F, 0xEA,
    0x10, 0x11, 0x38, 0xB5, 0x1D, 0xD8, 0x02, 0x07,
};

POSSIBLY_UNUSED static const uint8_t key_256[] = {
    0x10, 0x4B, 0x1A, 0x68, 0x01, 0x9B, 0x5A, 0x40, 0x4F, 0xF0, 0x00, 0x03, 0x16, 0xD1, 0x03, 0xB0,
    0x5D, 0xF8, 0x04, 0xFB, 0x03, 0xF0, 0xAA, 0xFC, 0xEC, 0xE7, 0x0D, 0xF1, 0x02, 0x01, 0x41, 0xF2,
};

#ifdef SEC_MASS_DATA_TEST_MODE
static void rand_generator(uint8_t *dest, size_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}
#endif

void aes_test(void)
{
    #define RET_CNT 15
    POSSIBLY_UNUSED int ret[RET_CNT] = {0};
    size_t test_len;
    SEC_ENG_BUF_LOC static uint8_t test_data[OUT_BUF_SIZE];

#ifdef SEC_SIMPLE_TEST_MODE
    static const char *input = "Advanced Encryption Standard is the most common symmetric encryption algorithm!!";
    test_len = strlen(input);
    memcpy(test_data, input, test_len);
#else
    srand(hal_sys_timer_get() ^ hal_fast_sys_timer_get());
    rand_generator(test_data, CIPHER_DATA_SIZE);
#ifdef LOOP_SEQUENCE_TEST
    size_t loop_cnt = 0;
    test_len = 0;
#elif defined(LOOP_RANDOM_TEST)
    size_t test_cnt = 0;
    test_len = 0;
#else
    size_t test_cnt = 0;
    test_len = CIPHER_DATA_SIZE;//fixed length test mode
#endif
#endif

    while (1) {
#ifdef SEC_MASS_DATA_TEST_MODE
#ifdef LOOP_SEQUENCE_TEST
        test_len++;
        if (test_len > CIPHER_DATA_SIZE) {
            test_len = 1;
            loop_cnt++;
        }
#elif defined(LOOP_RANDOM_TEST)
        test_len = rand() % (CIPHER_DATA_SIZE - 1 + 1) + 1;
        test_cnt++;
#else
        test_cnt++;
#endif
#endif

        ret[ 0] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_128_ECB, (uint8_t *)key_128, sizeof(key_128), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 1] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_192_ECB, (uint8_t *)key_192, sizeof(key_192), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 2] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_256_ECB, (uint8_t *)key_256, sizeof(key_256), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);

        ret[ 3] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_128_CBC, (uint8_t *)key_128, sizeof(key_128), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 4] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_192_CBC, (uint8_t *)key_192, sizeof(key_192), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 5] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_256_CBC, (uint8_t *)key_256, sizeof(key_256), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);

        ret[ 6] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_128_CTR, (uint8_t *)key_128, sizeof(key_128), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 7] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_192_CTR, (uint8_t *)key_192, sizeof(key_192), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[ 8] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_256_CTR, (uint8_t *)key_256, sizeof(key_256), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);

        ret[ 9] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_128_OFB, (uint8_t *)key_128, sizeof(key_128), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[10] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_192_OFB, (uint8_t *)key_192, sizeof(key_192), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[11] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_256_OFB, (uint8_t *)key_256, sizeof(key_256), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);

        ret[12] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_128_CFB128, (uint8_t *)key_128, sizeof(key_128), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[13] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_192_CFB128, (uint8_t *)key_192, sizeof(key_192), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);
        ret[14] = aes_encrypt_decrypt_test(MBEDTLS_CIPHER_AES_256_CFB128, (uint8_t *)key_256, sizeof(key_256), iv, sizeof(iv) / sizeof(iv[0]), (uint8_t *)test_data, test_len);

#ifdef SEC_MASS_DATA_TEST_MODE
#ifdef LOOP_SEQUENCE_TEST
        PRINT("-------------------[sequence test]data len:%u, loop cnt:%u, test cnt: %u * %u + %u", test_len, loop_cnt, CIPHER_DATA_SIZE, loop_cnt, test_len);
#elif defined(LOOP_RANDOM_TEST)
        PRINT("-------------------[random test]data len:%u, test cnt: %u", test_len, test_cnt);
#else
        PRINT("-------------------[fixed len test]data len:%u, test cnt: %u", test_len, test_cnt);
#endif
        //check return value
        for (int i = 1; i < RET_CNT; i++) {
            ret[0] += ret[i];
        }
        /* exit at the error, otherwise continue to test */
        if (ret[0]) {
            break;
        }
#else   /* SEC_SIMPLE_TEST_MODE */
        break;
#endif
    }
}

#endif /* MBEDTLS_CIPHER_C */
