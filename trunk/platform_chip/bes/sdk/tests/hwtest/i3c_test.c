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
#ifdef I3C_TEST

#include "hal_trace.h"
#include "lsm6dso_test.h"
#include "lsm6dsr_test.h"

#if 1
    #define I3C_TEST_TRACE(s, ...)       TRACE_IMM(0, s, ##__VA_ARGS__)
#else
    #define I3C_TEST_TRACE(s, ...)       TRACE_DUMMY(0, s, ##__VA_ARGS__)
#endif

void i3c_test(void)
{
    I3C_TEST_TRACE("   ");
    I3C_TEST_TRACE("------I3C tx/rx test started------");
    I3C_TEST_TRACE("  ");

    lsm6dsr_read_data_polling();

    //lsm6dso_self_test();
    //lsm6dso_read_data_polling();
}

#endif

