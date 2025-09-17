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
#include "plat_addr_map.h"

#ifdef CPUDUMP_BASE
#include "hal_trace.h"
#include "hal_cpudump.h"
#include "hal_timer.h"
#include <stdlib.h>
#ifdef RTOS
    #include "cmsis_os.h"
#endif

enum TEST_MODE {
    USAGE_FAULT = 0,
    BUS_FAULT,
    MEM_FAULT,
    RUNNING,
} ;

static void (*p0)(void) = (void (*)(void))(RAMX_BASE - 4);
static uint32_t * write_addr = (uint32_t*)RAMX_BASE;
static int divider = 0;

void cpudump_test(void)
{
    volatile const enum TEST_MODE mode = RUNNING;
    uint32_t cpu_data[16];
    int read_size;

    switch (mode) {
        case USAGE_FAULT:
            TRACE_IMM(0, "TEST USAGE_FAULT");
            TRACE(0, "%d", 10 / divider);
            break;
        case BUS_FAULT:
            TRACE_IMM(0, "TEST BUS_FAULT");
            p0();
            break;
        case MEM_FAULT:
            TRACE_IMM(0, "TEST MEM_FAULT");
            *write_addr = 0;
            break;
        case RUNNING:
            TRACE_IMM(0, "TEST RUNNING");
            TRACE(0, "DUMP LR");
            while (1) {
                int local_cnt = rand() % 20;
                osDelay(local_cnt);
                read_size = hal_cpudump_lr_read(0, cpu_data, ARRAY_SIZE(cpu_data));
                DUMP32("%08x ", cpu_data, read_size);
            }
    }
}

#endif
