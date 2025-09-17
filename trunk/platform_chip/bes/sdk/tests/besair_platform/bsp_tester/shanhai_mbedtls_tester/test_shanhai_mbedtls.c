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
#ifdef TEST_SHANHAI_MBEDTLS

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "shanhai_mbedtls_test.h"

typedef struct {
    int (*func)(int); /*test function*/
    const char *name; /*test nameription*/
} test_thread_t;

static inline void test_thread_func(void *p)
{
    test_thread_t t;
    if (!p)
        return;
    memcpy(&t, p, sizeof(test_thread_t));
    while (1) {
        ASSERT((t.func(1) == 0), "%s FAILED!!!", t.name);
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
TEST_THREAD_DEF(8);
TEST_THREAD_DEF(9);

static osThreadFunc_t funcs[10] = {
    test_thread_0,
    test_thread_1,
    test_thread_2,
    test_thread_3,
    test_thread_4,
    test_thread_5,
    test_thread_6,
    test_thread_7,
    test_thread_8,
    test_thread_9,
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

static int shanhai_mbedtls_shax_test_1(int verbose)
{
    int ret = 0;

    ret |= shanhai_sha1_test_1(verbose);
    ret |= shanhai_md5_test(verbose);
    ret |= shanhai_sha256_test_1(verbose);
    ret |= shanhai_sha512_test_1(verbose);

    return ret;
}

static int shanhai_mbedtls_shax_test_2()
{
    int ret = 0;
    int verbose = 1;
    ret |= shanhai_sha1_test_2(verbose);
    ret |= shanhai_sha256_test_2(verbose);
    ret |= shanhai_sha512_test_2(verbose);
    return ret;
}

static bool all_test_running = false;

POSSIBLY_UNUSED
static void mbedtls_test()
{
    int ret = 0;
    int i = 0;
    TRACE(0, "################MBEDTLS-FUNCTION-TEST %d ######################\n", i);
    ASSERT(shanhai_aes_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha1_test_1(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha1_test_2(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha256_test_1(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha256_test_2(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha512_test_1(1) == 0, "FAILED!!!");
    ASSERT(shanhai_sha512_test_2(1) == 0, "FAILED!!!");
    ASSERT(shanhai_md5_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_hmac_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_ccm_gcm_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_bignum_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_trng_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_rsa_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_ecdh_test(1) == 0, "FAILED!!!");
    ASSERT(shanhai_ecdsa_test(1) == 0, "FAILED!!!");
    all_test_running = false;
    osThreadExit();
}

POSSIBLY_UNUSED
static void mbedtls_run_test(void)
{
    osThreadAttr_t attr;

    all_test_running = true;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = "MBEDTLS-TEST";
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;
    osThreadNew(mbedtls_test, NULL, &attr);

    while (all_test_running)
        osDelay(1000);
}

void test_shanhai_mbedtls(void)
{
    mbedtls_shanhai_init();
    TRACE(0, "###########Start function test.\n");
    mbedtls_run_test();
    TRACE(0, "###### Start multi-threads stability test.");
    start_test_thread(shanhai_mbedtls_shax_test_1, "SHAX TEST-1", 0, 2048, osPriorityNormal);
    start_test_thread(shanhai_mbedtls_shax_test_2, "SHAX TEST-2", 1, 2048, osPriorityNormal);
    start_test_thread(shanhai_aes_test, "AES TEST-1", 2, 4096, osPriorityNormal);
    start_test_thread(shanhai_hmac_test, "HAMC TEST", 3, 2048, osPriorityNormal);
    start_test_thread(shanhai_ccm_gcm_test, "CCM-GCM TEST", 4, 2048, osPriorityNormal);
    start_test_thread(shanhai_bignum_test, "BIGNUM TEST", 5, 2048, osPriorityNormal);
    start_test_thread(shanhai_trng_test, "TRNG TEST", 6, 4096, osPriorityNormal);
    start_test_thread(shanhai_rsa_test, "RSA TEST", 7, 4096, osPriorityNormal);
    start_test_thread(shanhai_ecdh_test, "ECDH TEST", 8, 4096, osPriorityNormal);
    start_test_thread(shanhai_ecdsa_test, "ECDSA TEST", 9, 4096, osPriorityNormal);

    while (1)
        osDelay(10000);
}

#endif
