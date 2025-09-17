
#if defined(UTILS_ESHELL_EN)
#ifdef EVR_TRACING
#include <stdio.h>
#include "stdlib.h"
#include "hal_trace.h"
#include "eshell.h"
#include "app_eshell.h"
#include "app_utils.h"
#include "SEGGER_SYSVIEW.h"

#ifndef CHIP_SUBSYS_BTH
static void systemview_start_stop(int argc, char *argv[])
{
#ifdef EVR_TRACING
    uint8_t enable = 0;
    enable = atoi(argv[1]);

    if (enable){
        TRACE_IMM(0, "%s start", __func__);
        extern uint32_t g_sysview_len;
        g_sysview_len = 0;
        segger_systemview_start();
    }
    else {
        TRACE_IMM(0, "%s stop", __func__);
        segger_systemview_stop();
    }
#endif
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "systemview_ctrl", "Set and get CPU frequency", systemview_start_stop);
#endif
#endif
#endif