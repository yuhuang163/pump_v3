/*
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
* Author: jianchen@bestechnic.com
* Date: 2023-03-09
*/

#include <stdlib.h>
#include "hal_trace.h"
#include "mbedtls/threading.h"
#include "mbedtls/memory_buffer_alloc.h"

void mbedtls_init(void)
{
#if defined(MBEDTLS_THREADING_ALT)
    /* Set system mutex resource to mbedtls. */
    mbedtls_threading_set_alt(mbedtlsThreading_MutexInit, mbedtlsThreading_MutexFree,
                              mbedtlsThreading_MutexLock, mbedtlsThreading_MutexUnlock);
#endif
#ifdef KERNEL_RTX5
/*
* There are defects in RTX5 memory management, resulting in problems with HTTPS functions.
* Use the built-in memory management function of mbedtls to replace the memory management function
* of RTX5.
* HTTPS with full certificate resolution and request requires at least 372 KB of memory.
*/
#if defined(MBEDTLS_MEMORY_BUFFER_SIZE) && (MBEDTLS_MEMORY_BUFFER_SIZE > 0)
    unsigned char *mem = (unsigned char*)malloc(MBEDTLS_MEMORY_BUFFER_SIZE);
    if(mem) {
        mbedtls_memory_buffer_alloc_init(mem, MBEDTLS_MEMORY_BUFFER_SIZE);
    }
#endif
#endif
}
