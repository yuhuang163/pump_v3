/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef TEST_VMIC
#include "cmsis_os.h"
#include "hal_psc.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "pmu.h"

#ifndef TEST_VMIC_VOLT
#define TEST_VMIC_VOLT                  (1800)
#endif

typedef struct {
uint32_t map;
uint32_t mv;
}info_t;

void test_vmic(void)
{
    uint32_t i = 0;
    info_t info[] = {
        {AUD_VMIC_MAP_VMIC1, TEST_VMIC_VOLT},
        {AUD_VMIC_MAP_VMIC2, TEST_VMIC_VOLT},
    };

    for (i=0; i<sizeof(info)/sizeof(info_t); i++) {
        pmu_codec_mic_bias_set_volt(info[i].map, info[i].mv);
        pmu_codec_mic_bias_noremap_enable(info[i].map, 1);
    }
}

#endif



