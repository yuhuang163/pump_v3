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
#ifdef MBEDTLS_TEST

#include "hal_trace.h"

extern void aes_test(void);
extern void rsa_test(void);
extern void bignum_test(void);

void mbedtls_test(void)
{
    REL_TRACE_NOTS(0, "  ");
    REL_TRACE_NOTS(1, "------ Enter %s ------", __func__);

    REL_TRACE_NOTS(0, "  ");
    REL_TRACE_NOTS(0, "===========aes test start===========");
    aes_test();
    REL_TRACE_NOTS(0, "============aes test end============");

    REL_TRACE_NOTS(0, "  ");
    REL_TRACE_NOTS(0, "===========bignum test start===========");
    bignum_test();
    REL_TRACE_NOTS(0, "============bignum test end============");

    REL_TRACE_NOTS(0, "  ");
    REL_TRACE_NOTS(0, "===========rsa test start===========");
    rsa_test();
    REL_TRACE_NOTS(0, "============rsa test end============");

    REL_TRACE_NOTS(1, "------ Exit %s -------", __func__);
}

#endif
