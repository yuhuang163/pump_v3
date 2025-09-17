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

#ifdef UTILS_ESHELL_EN

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

static void do_msleep(int argc, char *argv[])
{
    unsigned int delay;
    if (argc < 2) {
        eshell_putstring("\r\nUsage: msleep delay_ms\r\n");
        return;
    }
    delay = atoi(argv[1]);
    osDelay(delay);
    eshell_putstring("sleep %ums\n", delay);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "msleep", "sleep miliseconds", do_msleep);

static void do_sleep(int argc, char *argv[])
{
    unsigned int delay;
    if (argc != 2) {
        eshell_putstring("\r\nUsage: sleep delay_secs\r\n");
        return;
    }
    delay = atoi(argv[1]);
    osDelay(delay*1000);
    eshell_putstring("sleep %us\n", delay);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "sleep", "sleep seconds", do_sleep);

static void power_utils_shell_sleep_cmd(int argc, char ** argv)
{
    eshell_platform_sleep();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "shell_sleep", "shell sleep", power_utils_shell_sleep_cmd);

#endif //UTILS_ESHELL_EN
