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

#if defined(MBEDTLS_ECDH_C)

#include <stdlib.h>
#include <string.h>
#include "hexdump.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"

static int shanhai_ecdh_test_base(int verbose, mbedtls_ecp_group_id ecp_group_id)
{
    int ret;
    const char *pers = "ecdh_dubhe_test";

    const mbedtls_ecp_curve_info *curve_info;
    mbedtls_mpi client_secret, server_secret;
    mbedtls_mpi client_pri, server_pri;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ecp_group grp;
    mbedtls_ecp_point client_pub, server_pub;

    mbedtls_mpi_init(&client_pri);
    mbedtls_mpi_init(&server_pri);
    mbedtls_mpi_init(&client_secret);
    mbedtls_mpi_init(&server_secret);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&client_pub);
    mbedtls_ecp_point_init(&server_pub);

    curve_info = mbedtls_ecp_curve_info_from_grp_id(ecp_group_id);

    mbedtls_printf("EDCH-TEST-%s: start\n", curve_info->name);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers,
                                strlen(pers));
    if (ret != 0) {
        mbedtls_printf("mbedtls_ctr_drbg_seed failed\n");
        goto exit;
    }


    ret = mbedtls_ecp_group_load(&grp, ecp_group_id);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecp_group_load failed\n");
        goto exit;
    }

    ret = mbedtls_ecdh_gen_public(&grp, &client_pri, &client_pub,
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecdh_gen_public client failed ret = %d\n", ret);
        goto exit;
    }

    ret = mbedtls_ecdh_gen_public(&grp, &server_pri, &server_pub,
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("%s mbedtls_ecdh_gen_public server failed\n", __func__);
        goto exit;
    }

    ret = mbedtls_ecdh_compute_shared(&grp, &client_secret,
                                      &server_pub, &client_pri,
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecdh_compute_shared client_secret failed\n");
        goto exit;
    }

    ret = mbedtls_ecdh_compute_shared(&grp, &server_secret,
                                      &client_pub, &server_pri,
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        mbedtls_printf("mbedtls_ecdh_compute_shared server_secret failed\n");
        goto exit;
    }

    ret = mbedtls_mpi_cmp_mpi(&server_secret, &client_secret);
    if (ret != 0) {
        mbedtls_printf("mbedtls_mpi_cmp_mpi failed\n");
        goto exit;
    }

    mbedtls_printf("EDCH-TEST-%s: passed\n", curve_info->name);

exit:
    mbedtls_mpi_free(&client_pri);
    mbedtls_mpi_free(&server_pri);
    mbedtls_mpi_free(&client_secret);
    mbedtls_mpi_free(&server_secret);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&client_pub);
    mbedtls_ecp_point_free(&server_pub);

    if (ret != 0 && verbose != 0)
        mbedtls_printf("EDCH-TEST-%s: failed\n", curve_info->name);

    return ret;
}

int shanhai_ecdh_test(int verbose)
{
    int ret = 0;

    ret |= shanhai_ecdh_test_base(verbose, MBEDTLS_ECP_DP_SECP224R1);

    ret |= shanhai_ecdh_test_base(verbose, MBEDTLS_ECP_DP_SECP256R1);

    ret |= shanhai_ecdh_test_base(verbose, MBEDTLS_ECP_DP_SECP384R1);

    return ret;
}

#endif
