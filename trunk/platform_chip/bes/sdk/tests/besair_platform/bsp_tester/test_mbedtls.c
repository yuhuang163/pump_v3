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
#ifdef TEST_MBEDTLS
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "mbedtls_test.h"

#ifndef __ICCARM__
extern uint32_t __HeapBase[];
extern uint32_t __HeapLimit[];
extern uint32_t __StackTop[];
extern uint32_t __StackLimit[];
#endif

static bool have_large_heap = false;
static bool have_large_stack = false;

typedef struct {
    int (*func)(int);    /* test function */
    const char *name;    /* test nameription */
} test_thread_t;

static inline void test_thread_func(void *p)
{
    test_thread_t t;
    if (!p)
        return;
    memcpy(&t, p, sizeof(test_thread_t));
    while (1) {
        ASSERT((t.func(1) == 0), "%s FAILED!!", t.name);
    }
}

#define TEST_THREAD_DEF(idx) \
    POSSIBLY_UNUSED static void test_thread_##idx(void *data) \
    { \
        test_thread_func(data); \
        osThreadExit(); \
    }

TEST_THREAD_DEF(0);
TEST_THREAD_DEF(1);
TEST_THREAD_DEF(2);
TEST_THREAD_DEF(3);
TEST_THREAD_DEF(4);
TEST_THREAD_DEF(5);
TEST_THREAD_DEF(6);
TEST_THREAD_DEF(7);

static osThreadFunc_t funcs[8] = {
    test_thread_0,
    test_thread_1,
    test_thread_2,
    test_thread_3,
    test_thread_4,
    test_thread_5,
    test_thread_6,
    test_thread_7,
};

static osThreadId_t start_test_thread(int (*func)(int), const char *name, int index,
                            int stacksize, osPriority_t priority)
{
    osThreadId_t id = NULL;
    osThreadAttr_t attr;
    test_thread_t t = {
        .func = func,
        .name = name,
    };

    if (!func)
        return NULL;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = name;
    attr.stack_size = stacksize;
    attr.priority = priority;
    id = osThreadNew(funcs[index], (void *)&t, &attr);
    osDelay(100);
    return id;
}

POSSIBLY_UNUSED
static int mbedtls_shax_test_1(int verbose)
{
    int ret = 0;
    ret |= mbedtls_md5_test(verbose);
    ret |= mbedtls_sha1_test_1(verbose);
    ret |= mbedtls_sha256_test_1(verbose);
    ret |= mbedtls_sha512_test_1(verbose);
    return ret;
}

POSSIBLY_UNUSED
static int mbedtls_shax_test_2(int verbose)
{
    int ret = 0;
    ret |= mbedtls_sha512_test_2(verbose);
    ret |= mbedtls_sha1_test_2(verbose);
    ret |= mbedtls_sha256_test_2(verbose);
    return ret;
}

static bool all_test_running = false;

POSSIBLY_UNUSED
static void mbedtls_all_test(void *notused)
{
    for (int i = 0; i < 20; i++) {
        TRACE(0, "##################### MBEDTLS-FUNCTION-TEST %d #########################\n", i);
        ASSERT(mbedtls_aes_test_1(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_aes_test_2(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_md5_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha1_test_1(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha1_test_2(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha256_test_1(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha256_test_2(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha512_test_1(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_sha512_test_2(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_hmac_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_base64_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_ctr_drbg_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_entropy_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_gcm_test_1(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_gcm_test_2(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_ccm_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_cmac_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_pkcs5_test(1) == 0, "FAILED!!!");
        ASSERT(mbedtls_hmac_drbg_test(1) == 0, "FAILED!!!");

        /* x509_crt and rsa tests need increase heap size to 0x8000;
         * HEAP_SECTION_SIZE in best1000.lds.S */
        if (have_large_heap) {
            ASSERT(mbedtls_rsa_test(1) == 0, "FAILED!!!");
            ASSERT(mbedtls_x509_crt_test(1) == 0, "FAILED!!!");
            ASSERT(mbedtls_ecjpake_test(1) == 0, "FAILED!!!");
        }
    }

    all_test_running = false;
    osThreadExit();
}

POSSIBLY_UNUSED
static void mbedtls_run_all_test(void)
{
    osThreadAttr_t attr;

    all_test_running = true;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = "MBEDTLS-ALL-TEST";
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;
    osThreadNew(mbedtls_all_test, NULL, &attr);

    while(all_test_running)
        osDelay(1000);
}

void test_mbedtls(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);

#ifndef __ICCARM__
    if (((uint32_t)__HeapLimit - (uint32_t)__HeapBase) >= 0x8000)
        have_large_heap = true;
    if (((uint32_t)__StackTop - (uint32_t)__StackLimit) >= 0x8000)
        have_large_stack = true;
#endif

    if (!have_large_stack)
        TRACE(0, "WARNING: stack size is too small, suggestion is 32KB.");

    TRACE(0, "###### Start function test.");

    mbedtls_run_all_test();

    TRACE(0, "###### Start multi-threads stability test.");

    start_test_thread(mbedtls_aes_test_1, "AES TEST-1", 0, 2048, osPriorityNormal);
    start_test_thread(mbedtls_aes_test_2, "AES TEST-2", 1, 2048, osPriorityNormal);
    start_test_thread(mbedtls_shax_test_1, "SHAX TEST-1", 2, 2048, osPriorityNormal);
    start_test_thread(mbedtls_hmac_test, "HMAC TEST", 3, 2048, osPriorityNormal);
    start_test_thread(mbedtls_ctr_drbg_test, "CTR_DRBG TEST", 4, 4096, osPriorityNormal);
    start_test_thread(mbedtls_gcm_test_1, "GCM TEST-1", 5, 4096, osPriorityNormal);

    /* x509_crt and rsa tests need increase heap size to 0x8000;
     * HEAP_SECTION_SIZE in best1000.lds.S */
    if (have_large_heap)
        start_test_thread(mbedtls_x509_crt_test, "X509-CRT TEST", 6, 4096, osPriorityNormal);

    while (1)
        osDelay(10000);
}

#endif
