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

#if defined(MBEDTLS_BIGNUM_C)

#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"

#include "mbedtls/bignum.h"
#include "mbedtls/platform.h"

#ifdef DEFAULT_BSS_IN_PSRAM
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_PSRAM_LOC
#else
    #define SEC_ENG_BUF_LOC                     SYNC_FLAGS_LOC
#endif

SEC_ENG_BUF_LOC static char buf[256];

static void dump_buf(char *buffer, size_t len)
{
    for (int i = 0; i < len; i++) {
        mbedtls_printf("%c%s", buffer[i], (i + 1) % 32 ? "" : "\n\t");
    }
    mbedtls_printf("\n");
}

void bignum_test(void)
{
    size_t olen;
    mbedtls_mpi A, E, N, X;

    mbedtls_mpi_init(&A);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&X);

    mbedtls_mpi_read_string(&A, 16,
                            "EFE021C2645FD1DC586E69184AF4A31E" \
                            "D5F53E93B5F123FA41680867BA110131" \
                            "944FE7952E2517337780CB0DB80E61AA" \
                            "E7C8DDC6C5C6AADEB34EB38A2F40D5E6");

    mbedtls_mpi_read_string(&E, 16,
                            "B2E7EFD37075B9F03FF989C7C5051C20" \
                            "34D2A323810251127E7BF8625A4F49A5" \
                            "F3E27F4DA8BD59C47D6DAABA4C8127BD" \
                            "5B5C25763222FEFCCFC38B832366C29E");

    mbedtls_mpi_read_string(&N, 16,
                            "0066A198186C18C10B2F5ED9B522752A" \
                            "9830B69916E535C8F047518A889A43A5" \
                            "94B6BED27A168D31D4A52F88925AA8F5");

    mbedtls_mpi_mul_mpi(&X, &A, &N);
    mbedtls_mpi_write_string(&X, 16, buf, 256, &olen);
    mbedtls_printf("\n  X = A * N = \n\t");
    dump_buf(buf, olen);

    mbedtls_mpi_exp_mod(&X, &A, &E, &N, NULL);
    mbedtls_mpi_write_string(&X, 16, buf, 256, &olen);
    mbedtls_printf("\n  X = A^E mode N = \n\t");
    dump_buf(buf, olen);

    mbedtls_mpi_inv_mod(&X, &A, &N);
    mbedtls_mpi_write_string(&X, 16, buf, 256, &olen);
    mbedtls_printf("\n  X = A^-1 mod N = \n\t");
    dump_buf(buf, olen);

    mbedtls_mpi_free(&A);
    mbedtls_mpi_free(&E);
    mbedtls_mpi_free(&N);
    mbedtls_mpi_free(&X);
}

#endif
