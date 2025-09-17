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
#include "audio_tests.h"

#include "hal_trace.h"
#include "app_factory_audio.h"
#include "a2dp_decoder.h"
#include "stdio.h"

void factorymode_audioloop_test(char* cmdParam, uint32_t cmdParam_len)
{
    static bool isRun_test = true;
    TRACE(0, "factorymode_audioloop_test %d", isRun_test);
    app_factorymode_audioloop(isRun_test, APP_SYSFREQ_208M);
    isRun_test = !isRun_test;
}

void a2dp_change_jitter_buf_test(char* cmdParam, uint32_t cmdParam_len)
{
    do {
        int targetMs = 0;
        sscanf(cmdParam, "%d", &targetMs);
        TRACE(0, "[%s] targetMs %d", __func__, targetMs);
        sbm_update_target_jitter_buf_length(targetMs);
    } while(0);
}
