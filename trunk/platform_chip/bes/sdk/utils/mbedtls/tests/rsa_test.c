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
#include "mbedtls/build_info.h"

#if defined(MBEDTLS_RSA_C)

#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "heap_api.h"

#define RSA_SIMPLE_TEST_MODE
//#define RSA_SPEED_TEST_MODE

#if defined(RSA_SIMPLE_TEST_MODE) && defined(RSA_SPEED_TEST_MODE)
    #error "RSA_SIMPLE_TEST_MODE and RSA_SPEED_TEST_MODE, the 2 macros can only open one of them"
#endif

#ifdef DEFAULT_BSS_IN_PSRAM
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_PSRAM_LOC
#else
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_LOC
#endif

#ifdef RSA_SPEED_TEST_MODE
    #define OUT_BUF_SIZE                        (KEY_SIZE_MAX / 8)
#else
    #define OUT_BUF_SIZE                        128
#endif

#define EXPONENT                                65537
#define KEY_SIZE_1024                           1024
#define KEY_SIZE_2048                           2048
#define KEY_SIZE_3072                           3072
#define KEY_SIZE_4096                           4096
#define KEY_SIZE_MAX                            KEY_SIZE_4096

#ifdef RSA_SPEED_TEST_MODE
/* If buf is not increased, a -0x4310 error will be reported */
static unsigned char alloc_buf[16384 * 2];
#else
static unsigned char alloc_buf[16384];
#endif

#ifndef MBEDTLS_MEMORY_BUFFER_ALLOC_C
static multi_heap_handle_t g_heap_handle;

void *test_calloc(size_t n, size_t size)
{
    void *ptr = NULL;
    size_t size_real = size * n;

    ptr = heap_malloc(g_heap_handle, size_real);
    ASSERT(ptr != NULL, "%s:%d, malloc failed, size=%d", __FUNCTION__, __LINE__, (int)size_real);
    mbedtls_printf("malloc done, ptr=0x%x, size=%d\n", (int)ptr, (int)size_real);

    return ptr;
}

void test_free(void *ptr)
{
    heap_free(g_heap_handle, ptr);
    mbedtls_printf("%s:%d, ptr=0x%x, free done\n", __FUNCTION__, __LINE__, (int)ptr);
}

static void mbedtls_calloc_init(void)
{
    mbedtls_printf("%s:%d\n", __FUNCTION__, __LINE__);

    g_heap_handle = heap_register((void *)alloc_buf, sizeof(alloc_buf));

    mbedtls_printf("%s:%d, g_heap_handle=0x%x, start=0x%x, size=%d\n",
                   __FUNCTION__, __LINE__, (int)g_heap_handle, (int)alloc_buf, (int)sizeof(alloc_buf));

}
#endif

static int platform_entropy_poll(void *ctx, unsigned char *output,
                                 size_t len, size_t *olen)
{
    size_t i;
    for (i = 0; i < len; i++) {
        output[i] = rand() & 0xFF;
    }

    *olen = len;
    return 0;
}

POSSIBLY_UNUSED
static void dump_rsa_key(mbedtls_rsa_context *rsa)
{
    size_t olen;
    char buf[516];

    mbedtls_printf("\n  +++++++++++++++++ rsa keypair +++++++++++++++++\n");
    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(N), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("N: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(E), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("E: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(D), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("D: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(P), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("P: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(Q), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("Q: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(DP), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("DP: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(DQ), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("DQ: %s\n", buf);

    mbedtls_mpi_write_string(&rsa->MBEDTLS_PRIVATE(QP), 16, buf, sizeof(buf), &olen);
    mbedtls_printf("QP: %s\n", buf);
    mbedtls_printf("  +++++++++++++++++++++++++++++++++++++++++++++++\n");
}

static int gen_rsa_key(mbedtls_rsa_context *rsa, mbedtls_ctr_drbg_context *ctr_drbg, uint32_t key_size)
{
    int ret;

    ret = mbedtls_rsa_gen_key(rsa, mbedtls_ctr_drbg_random, ctr_drbg, key_size, EXPONENT);
    if (ret) {
        mbedtls_printf(" failed\n  ! mbedtls_rsa_gen_key returned %d(-0x%X)\n", ret, -ret);
    }

    return ret;
}

SEC_ENG_BUF_LOC static unsigned char buf[OUT_BUF_SIZE];
SEC_ENG_BUF_LOC static unsigned char result[OUT_BUF_SIZE];

POSSIBLY_UNUSED
static void rsa_simple_test(uint32_t key_size)
{
    int ret = 1;
    size_t i;
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
    const char *pers = "rsa_test";
    const char *plaintext = "rsa encrypt test, 2022-08-19";
    uint32_t time_start, time[6];

    time_start = hal_fast_sys_timer_get();
    //memory init
#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
    mbedtls_memory_buffer_alloc_init(alloc_buf, sizeof(alloc_buf));
#else
    mbedtls_calloc_init();
    mbedtls_platform_set_calloc_free(test_calloc, test_free);
#endif

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_rsa_init(&rsa);
    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&P);
    mbedtls_mpi_init(&Q);
    mbedtls_mpi_init(&D);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_init(&DP);
    mbedtls_mpi_init(&DQ);
    mbedtls_mpi_init(&QP);

    mbedtls_entropy_init(&entropy);
    srand(hal_sys_timer_get() ^ hal_fast_sys_timer_get());
    mbedtls_entropy_add_source(&entropy, platform_entropy_poll, NULL,
                               32,//The min threshold of random numbers
                               MBEDTLS_ENTROPY_SOURCE_STRONG);

    time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    mbedtls_printf("\n\n  . Seeding the random number generator...");
    time_start = hal_fast_sys_timer_get();
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }
    time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //===========================generate RSA key===============================
    mbedtls_printf(" ok\n  . Generating the RSA key [ %d-bit ]...", key_size);
    time_start = hal_fast_sys_timer_get();
    ret = gen_rsa_key(&rsa, &ctr_drbg, key_size);
    if (ret) {
        goto exit;
    }
    time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    mbedtls_printf(" ok\n  . Exporting the public key ....");
    time_start = hal_fast_sys_timer_get();
    if ((ret = mbedtls_rsa_export(&rsa, &N, &P, &Q, &D, &E)) != 0 ||
        (ret = mbedtls_rsa_export_crt(&rsa, &DP, &DQ, &QP))      != 0) {
        mbedtls_printf(" failed\n  ! could not export RSA parameters\n\n");
        goto exit;
    }
    time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    mbedtls_printf(" ok\n\n");
    dump_rsa_key(&rsa);

    //============================encrypt data==================================
    mbedtls_printf("\n  . Generating the RSA encrypted data...");
    time_start = hal_fast_sys_timer_get();
    ret = mbedtls_rsa_pkcs1_encrypt(&rsa, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, strlen(plaintext),
                                    (const unsigned char *)plaintext, buf);
    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_rsa_pkcs1_encrypt returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }
    time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    mbedtls_printf(" ok\n");

    mbedtls_printf("  . plaintext: %s\n", plaintext);
    mbedtls_printf("  . ciphertext:\n");
    for (i = 0; i < rsa.MBEDTLS_PRIVATE(len); i++)
        mbedtls_printf("%02X%s", buf[i], (i + 1) % 16 == 0 ? "\r\n" : " ");

    //============================decrypt data==================================
    memset(result, 0, sizeof(result));
    mbedtls_printf("\n\n  . Decrypting the encrypted data...");
    time_start = hal_fast_sys_timer_get();
    ret = mbedtls_rsa_pkcs1_decrypt(&rsa, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, &i,
                                    buf, result, sizeof(result));
    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_rsa_pkcs1_decrypt returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }
    time[5] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    mbedtls_printf("  ok\n");
    mbedtls_printf("  . The decrypted result is: %s\n\n", result);

    if (strcmp((char *)result, plaintext) == 0) {
        mbedtls_printf("  =========time-consuming statistics========\n");
        mbedtls_printf("  ******************RSA%d*****************\n", key_size);
        mbedtls_printf("  init      :%uus\n", time[0]);
        mbedtls_printf("  drbg seed :%uus\n", time[1]);
        mbedtls_printf("  gen key   :%uus=%us\n", time[2], time[2] / 1000 / 1000);
        mbedtls_printf("  export crt:%uus\n", time[3]);
        mbedtls_printf("  encrypt   :%uus/%dbytes\n", time[4], strlen(plaintext) + 11);
        mbedtls_printf("  decrypt   :%uus/%dbytes\n", time[5], strlen(plaintext) + 11);
        mbedtls_printf("  ******************************************\n");
    } else {
        mbedtls_printf("  ***compare fail\n");
    }
    mbedtls_printf("\n");

exit:
    mbedtls_mpi_free(&N);
    mbedtls_mpi_free(&P);
    mbedtls_mpi_free(&Q);
    mbedtls_mpi_free(&D);
    mbedtls_mpi_free(&E);
    mbedtls_mpi_free(&DP);
    mbedtls_mpi_free(&DQ);
    mbedtls_mpi_free(&QP);
    mbedtls_rsa_free(&rsa);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

POSSIBLY_UNUSED
static void rand_generator(uint8_t *dest, size_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}

POSSIBLY_UNUSED
static void rsa_speed_test(uint32_t key_size)
{
    int ret = 1;
    size_t olen;
    mbedtls_rsa_context rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_test";
    SEC_ENG_BUF_LOC static unsigned char plaintext[OUT_BUF_SIZE];
    uint32_t time_start, time[6];
    uint32_t data_len;

    //memory init
#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
    mbedtls_memory_buffer_alloc_init(alloc_buf, sizeof(alloc_buf));
#else
    mbedtls_calloc_init();
    mbedtls_platform_set_calloc_free(test_calloc, test_free);
#endif

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_rsa_init(&rsa);

    mbedtls_entropy_init(&entropy);
    srand(hal_sys_timer_get() ^ hal_fast_sys_timer_get());
    mbedtls_entropy_add_source(&entropy, platform_entropy_poll, NULL,
                               32,//The min threshold of random numbers
                               MBEDTLS_ENTROPY_SOURCE_STRONG);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }

    //===========================generate RSA key===============================
    mbedtls_printf("  . Generating the RSA key [ %d-bit ]...", key_size);
    time_start = hal_fast_sys_timer_get();
    ret = gen_rsa_key(&rsa, &ctr_drbg, key_size);
    if (ret) {
        goto exit;
    }
    time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //============================encrypt data==================================
#ifdef MBEDTLS_PKCS1_V15
    data_len = key_size / 8 - 11;//PKCS#1 v1.5 padding takes up an additional 11 bytes
#else
    #error "Other modes are not supported yet"
#endif
    memset(plaintext, 0, sizeof(plaintext));
    rand_generator(plaintext, data_len);
    mbedtls_printf(" ok\n  . Generating the RSA encrypted data.....");
    time_start = hal_fast_sys_timer_get();
    ret = mbedtls_rsa_pkcs1_encrypt(&rsa, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, data_len,
                                    (const unsigned char *)plaintext, buf);
    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_rsa_pkcs1_encrypt returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }
    time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //============================decrypt data==================================
    memset(result, 0, sizeof(result));
    mbedtls_printf(" ok\n  . Decrypting the encrypted data.........");
    time_start = hal_fast_sys_timer_get();
    ret = mbedtls_rsa_pkcs1_decrypt(&rsa, mbedtls_ctr_drbg_random,
                                    &ctr_drbg, &olen,
                                    buf, result, sizeof(result));
    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_rsa_pkcs1_decrypt returned %d(-0x%X)\n", ret, -ret);
        goto exit;
    }
    time[5] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    mbedtls_printf(" ok\n");

    if (strcmp((char *)result, (char *)plaintext) == 0) {
        mbedtls_printf("  ******************RSA%d*****************\n", key_size);
        mbedtls_printf("  gen key: %uus=%us\n", time[2], time[2] / 1000 / 1000);
        mbedtls_printf("  encrypt: %uus/%dbytes\n", time[4], key_size / 8);
        mbedtls_printf("  decrypt: %uus/%dbytes\n", time[5], key_size / 8);
        mbedtls_printf("  ******************************************\n");
    } else {
        mbedtls_printf("  ***compare fail\n");
    }
    mbedtls_printf("\n");

exit:
    mbedtls_rsa_free(&rsa);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    TRACE_FLUSH();
}

void rsa_test(void)
{
    mbedtls_printf("CPU freq: %u\n", hal_sys_timer_calc_cpu_freq(5, 0));
#ifdef RSA_SIMPLE_TEST_MODE
    rsa_simple_test(KEY_SIZE_1024);
#endif

#ifdef RSA_SPEED_TEST_MODE
    rsa_speed_test(KEY_SIZE_1024);
    rsa_speed_test(KEY_SIZE_2048);
    rsa_speed_test(KEY_SIZE_3072);
    rsa_speed_test(KEY_SIZE_4096);
#endif
}

#endif /* MBEDTLS_RSA_C */
