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

#include "system_API_tests.h"

#include "hal_trace.h"
#include "hal_timer.h"
#include "app_battery.h"

void get_current_ticks_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint32_t current_ticks = GET_CURRENT_TICKS();

    TRACE(0, "Current ticks: %d", current_ticks);
}

void get_current_ms_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint32_t current_ms = GET_CURRENT_MS();

    TRACE(0, "Current ms: %d", current_ms);
}

void get_current_battery_level_test(char* cmdParam, uint32_t cmdParam_len)
{
    int8_t battery_level = app_battery_current_level();

    TRACE(0, "Current battery level:%d", battery_level);
}
