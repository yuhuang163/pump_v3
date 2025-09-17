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
#if defined(IQ_CALI_TEST) || defined(BTH_BT_TEST)

#include "plat_types.h"
#include "bth_comp_test.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "bt_host.h"
#include "string.h"
#ifdef BTH_BT_TEST
#include "hwtest.h"
#endif

extern int iq_cali_test(void);

static uint32_t result_msg[2];

void bth_bt_test(void)
{
    int ret;

    TR_INFO(0, "%s", __func__);

#ifdef IQ_CALI_TEST
    result_msg[0] = 0xCA11B;
    result_msg[1] = iq_cali_test();

    TR_INFO(0, "iq calib result: %d", result_msg[1]);
#else
    result_msg[0] = 0xB7B7;
    result_msg[1] = 0;

    bt_conn_test();

    TR_INFO(0, "bt_conn_test started");
#endif

    ret = bt_host_send(&result_msg, sizeof(result_msg));
    ASSERT(ret == 0, "bt_host_send failed: %d", ret);
}

#endif
