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

#if defined(MBEDTLS_ECDSA_C)

#include <stdlib.h>
#include <string.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"

#define mbedtls_printf printf

static uint8_t buf[256];

static int shanhai_ecdsa_test_base(int verbose, mbedtls_md_type_t md_type, mbedtls_ecp_group_id ecp_group_id)
{
    int ret;
    size_t qlen, dlen;
    size_t rlen, slen;
    uint8_t hash[64];

    const char *msg = "Happy New Year of the Rabbit,2023-01-19";
    const char *pers = "ecdsa_dubhe_test";

    mbedtls_mpi r, s;
    mbedtls_md_context_t md_ctx;
    mbedtls_ecdsa_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    const mbedtls_md_info_t *md_info;
    const mbedtls_ecp_curve_info *curve_info;

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_md_init(&md_ctx);
    mbedtls_ecdsa_init(&ctx);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    md_info = mbedtls_md_info_from_type(md_type);
    curve_info = mbedtls_ecp_curve_info_from_grp_id(ecp_group_id);

    mbedtls_printf("ECDSA-TEST-%s + %s: start\n", curve_info->name, mbedtls_md_get_name(md_info));

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *) pers,
                                strlen(pers));
    if (ret != 0) {
        mbedtls_printf("mbedtls_ctr_drbg_seed failed\n");
        goto exit;
    }

    ret = mbedtls_md(md_info, (uint8_t *)msg, strlen(msg), hash);
    if (ret != 0) {
        mbedtls_printf("mbedtls_md failed\n");
        goto exit;
    }

    ret = mbedtls_ecdsa_genkey(&ctx, ecp_group_id, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("%s mbedtls_ecdsa_genkey failed ret = %d\n", __func__, ret);
        goto exit;
    }

    ret = mbedtls_ecdsa_sign(&ctx.grp, &r, &s, &ctx.d, hash, sizeof(hash),
                             mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecdsa_sign failed\n");
        goto exit;
    }

    ret = mbedtls_ecdsa_verify(&ctx.grp, hash, sizeof(hash), &ctx.Q, &r, &s);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecdsa_verify failed\n");
        goto exit;
    }

    mbedtls_printf("ECDSA-TEST-%s + %s: passed\n", curve_info->name, mbedtls_md_get_name(md_info));

exit:
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_md_free(&md_ctx);
    mbedtls_ecdsa_free(&ctx);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    if (ret && verbose)
        mbedtls_printf("ECDSA-TEST-%s + %s: failed\n", curve_info->name, mbedtls_md_get_name(md_info));

    return ret;
}

int shanhai_ecdsa_test(int verbose)
{
    int ret;

    ret |= shanhai_ecdsa_test_base(verbose, MBEDTLS_MD_SHA224, MBEDTLS_ECP_DP_SECP224R1);

    ret |= shanhai_ecdsa_test_base(verbose, MBEDTLS_MD_SHA256, MBEDTLS_ECP_DP_SECP256R1);

    ret |= shanhai_ecdsa_test_base(verbose, MBEDTLS_MD_SHA384, MBEDTLS_ECP_DP_SECP384R1);
    return ret;
}

#endif
