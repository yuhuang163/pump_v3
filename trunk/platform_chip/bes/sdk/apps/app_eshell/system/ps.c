/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#if defined(UTILS_ESHELL_EN) && defined(KERNEL_RTX5)

#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "eshell.h"
#include "app_eshell.h"

#ifdef OS_TASKCNT
#define MAX_THREADS     OS_TASKCNT
#else
#define MAX_THREADS     20
#endif

static osThreadId_t thread_ids[MAX_THREADS] = {0};

static char *thread_state_str(osThreadId_t tid)
{
    switch(osThreadGetState(tid)) {
        case osThreadInactive:
            return "Inactive";
        case osThreadReady:
            return "Ready";
        case osThreadRunning:
            return "Running";
        case osThreadBlocked:
            return "Blocked";
        case osThreadTerminated:
            return "Terminated";
        case osThreadError:
            return "Error";
        default:
            return "Unkown";
    }
    return "Unkown";
}

static void show_thread_info(osThreadId_t tid)
{
    eshell_putstring("             ID: 0x%08x\r\n", (uint32_t)tid);
    eshell_putstring("           Name: %s\r\n", osThreadGetName(tid));
    eshell_putstring("       Priority: %d\r\n", osThreadGetPriority(tid));
    eshell_putstring("          State: %s\r\n", thread_state_str(tid));
    eshell_putstring("     Stack Size: %u bytes\r\n", osThreadGetStackSize(tid));
    eshell_putstring("    Stack Space: %u bytes\r\n", osThreadGetStackSpace(tid));
}

static void do_ps_cmd(int argc, char *argv[])
{
    int count = osThreadEnumerate(thread_ids, MAX_THREADS);
    for (int i = 0; i < count; i++) {
        eshell_putstring("\r\nThread %d:\r\n", i);
        show_thread_info(thread_ids[i]);
    }

    eshell_putstring("\r\n\r\nCurrent Thread:\r\n");
    show_thread_info(osThreadGetId());
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "ps", "show status of threads", do_ps_cmd);

#endif //UTILS_ESHELL_EN
