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
#ifdef TEST_DWT
#include "hal_trace.h"
#include "dwt.h"
#include "hal_timer.h"

static uint32_t dwt_data = 0x11223344;
static uint8_t index = 0;

void test_dwt(void)
{
    TRACE(0, "%s start", __func__);
    dwt_enable();

    watchpoint_enable(index, &dwt_data, WPT_WRITE);
    TRACE(0, "watchpoint_enable dwt_data addr:%p data:0x%08X", &dwt_data, dwt_data);

    hal_sys_timer_delay(MS_TO_TICKS(1000));

    TRACE(0, "Disable dwt, moidfy data, do not trigger crash");
    watchpoint_disable(index);
    dwt_data = 0x12345678;
    TRACE(0, "dwt_data=0x%08X", dwt_data);

    hal_sys_timer_delay(MS_TO_TICKS(1000));

    TRACE(0, "Enable dwt, moidfy data, trigger crash");
    watchpoint_enable(index, &dwt_data, WPT_WRITE);
    dwt_data = 0;
}

#endif
