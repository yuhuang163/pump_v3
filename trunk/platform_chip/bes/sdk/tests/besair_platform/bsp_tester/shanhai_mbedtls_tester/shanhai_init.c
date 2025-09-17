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

#include "hal_trace.h"
#include "hal_timer.h"
#include "pal_heap.h"
#include "pal_log.h"
#include "hal_location.h"

#define BUF_SIZE (16 * 1024)
static unsigned char __attribute__((aligned(32))) alloc_buf[BUF_SIZE];

void mbedtls_shanhai_init(void)
{
    pal_log_set_level(PAL_LOG_LEVEL_DATA);
    pal_heap_init(alloc_buf, BUF_SIZE);
}
