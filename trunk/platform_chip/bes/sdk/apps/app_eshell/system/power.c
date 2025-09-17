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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__)

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
#include "pmu.h"

static void do_reboot(int argc, char *argv[])
{
    eshell_putstring("Now reboot system...\n");
    osDelay(10);
    pmu_reboot();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "reboot", "reboot system", do_reboot);

static void do_shutdown(int argc, char *argv[])
{
    eshell_putstring("Now shut down system...\r\n");
    osDelay(10);
    pmu_shutdown();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_SYSTEM, "shutdown", "shutdown system", do_shutdown);

#endif //UTILS_ESHELL_EN
