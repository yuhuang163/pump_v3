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

#ifndef MBEDTLS_THREADING_ALT_H
#define MBEDTLS_THREADING_ALT_H

typedef struct mbedtls_threading_mutex_t {
    void *lock;
} mbedtls_threading_mutex_t;

void mbedtls_threading_init( void );

/* Compatible with wifi-app usage */
void mbedtls_threading_set_alt( void ( * mutex_init )( mbedtls_threading_mutex_t * ),
                                       void ( * mutex_free )( mbedtls_threading_mutex_t * ),
                                       int ( * mutex_lock )( mbedtls_threading_mutex_t * ),
                                       int ( * mutex_unlock )( mbedtls_threading_mutex_t * ) );

void mbedtls_threading_free_alt( void );

void mbedtlsThreading_MutexInit( mbedtls_threading_mutex_t* mbedtlsMutex );

void mbedtlsThreading_MutexFree( mbedtls_threading_mutex_t* mbedtlsMutex );

int mbedtlsThreading_MutexLock( mbedtls_threading_mutex_t* mbedtlsMutex );

int mbedtlsThreading_MutexUnlock( mbedtls_threading_mutex_t* mbedtlsMutex );

#endif
