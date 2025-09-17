/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef TEST_EMMC
#include "hal_trace.h"
#include "hal_timer.h"
#include "emmc_card_test.h"

void test_emmc(void)
{
    int ret = 0;

    TRACE(1, "%s: start", __func__);
    ret = emmc_card_test(0);
    TRACE(1, "%s: ret=%d", __func__, ret);

    //while (1);
}

#endif
