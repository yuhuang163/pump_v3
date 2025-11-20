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
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(RTOS) && defined(MBEDTLS_THREADING_ALT)
#include "stdlib.h"

#include "threading_alt.h"
#include "sec_lock.h"

#include "mbedtls/threading.h"

static void os_mutex_init(mbedtls_threading_mutex_t *mutex)
{
    if (mutex) {
        sec_alt_lock_init(&mutex->lock);
    }
}

static void os_mutex_free(mbedtls_threading_mutex_t *mutex)
{
    if (mutex) {
        sec_alt_lock_free(&mutex->lock);
    }
}

static int os_mutex_lock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex)
        sec_alt_lock(mutex->lock);
    return 0;
}

static int os_mutex_unlock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex)
        sec_alt_unlock(mutex->lock);
    return 0;
}

void mbedtls_threading_init(void)
{
    static char inited = 0;
    if (!inited) {
        mbedtls_threading_set_alt(os_mutex_init, os_mutex_free,
                        os_mutex_lock, os_mutex_unlock);
        inited = 1;
    }
}

void mbedtlsThreading_MutexInit( mbedtls_threading_mutex_t* mbedtlsMutex)
{
    os_mutex_init(mbedtlsMutex);
}

void mbedtlsThreading_MutexFree( mbedtls_threading_mutex_t* mbedtlsMutex)
{
    os_mutex_free(mbedtlsMutex);
}

int mbedtlsThreading_MutexLock( mbedtls_threading_mutex_t* mbedtlsMutex)
{
    return os_mutex_lock(mbedtlsMutex);
}

int mbedtlsThreading_MutexUnlock( mbedtls_threading_mutex_t* mbedtlsMutex)
{
    return os_mutex_unlock(mbedtlsMutex);
}

#endif

