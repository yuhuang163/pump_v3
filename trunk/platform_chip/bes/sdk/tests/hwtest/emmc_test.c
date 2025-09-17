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
#ifdef EMMC_TEST
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "emmc_card_test.h"

void emmc_test(void)
{
    int ret = 0;

    TRACE_IMM(0, "  ");
    TRACE_IMM(1, "%s: start", __func__);
    ret = emmc_card_test(0);
    //ret = emmc_card_rpmb_test();
    TRACE_IMM(1, "%s: ret=%d", __func__, ret);

    TRACE_IMM(0, "  ");
    TRACE_IMM(0, "======================================================================");
    TRACE_IMM(0, "The emmc test is over, please analyze the log");
    TRACE_IMM(0, "  ");
    TRACE_IMM(0, "Note: If the log is not complete, please increase the trace buff or");
    TRACE_IMM(0, "      close the log output or change the number of reads to 1 ");
    TRACE_IMM(0, "      (MULTI_READ_CNT is changed to 1)");
    TRACE_IMM(0, "======================================================================");
    TRACE_IMM(0, "  ");
}

#endif

