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

#if defined(MBEDTLS_BIGNUM_C)

#include <stdlib.h>
#include <string.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/bignum.h"
#include "mbedtls/platform.h"

#define mbedtls_printf printf

static char buf[256];
static mbedtls_mpi A, E, N, X;

int shanhai_bignum_test(int verbose)
{
    size_t olen;
    int ret;

    TRACE(0, "  ");
    mbedtls_printf("BIGNUM-TEST: start\n");
    extern int mbedtls_mpi_self_test(int verbose);
    if ((ret = mbedtls_mpi_self_test(verbose)) != 0)
        goto exit;

    mbedtls_printf("BIGNUM-TEST: passed\n");
    return 0;

exit:
    mbedtls_printf("BIGNUM-TEST: passed\n");
    return 1;
}

#endif
