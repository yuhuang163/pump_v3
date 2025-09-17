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

#if defined(UTILS_ESHELL_EN)

#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "cmsis_os2.h"
#include "eshell.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "cmsis_os.h"
#include "hal_sysfreq.h"
//#include "pmu_best1600.h"
#include "mcu_sensor_hub_app.h"

#define LOW_POWER_SLEEP_START_SIGNAL 0x01

static osThreadId_t lowpower_thread_id = NULL;
static void lowpower_thread(void const *argument);
static osEvent evt;


static void lowpower_thread(void const *argument)
{
    uint32_t *sleep = (uint32_t *)argument;
    while(1)
    {
        evt = osSignalWait(0x0, osWaitForever);
        if(osEventSignal == evt.status)
        {
            if(evt.value.signals & LOW_POWER_SLEEP_START_SIGNAL)
            {
                hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
                eshell_platform_sleep();
                // osDelay((*sleep) * 1000);
                hal_sleep_enter_sleep();
                osThreadFlagsClear(0xff);
                osThreadFlagsWait(0x11, osFlagsWaitAll, (*sleep) * 1000);
                TRACE(0,"exit sleep");
                eshell_platform_wakeup();
            }

        }

    }


}

static void into_lowpower(uint32_t sleep)
{
    osThreadAttr_t threadattr_lowpower;
    if(NULL == lowpower_thread_id)
    {
        memset(&threadattr_lowpower, 0, sizeof(osThreadAttr_t));
        threadattr_lowpower.name = "test_lowpower_thread1";
        threadattr_lowpower.attr_bits = osThreadDetached;
        threadattr_lowpower.stack_size = 2048;
        threadattr_lowpower.priority = osPriorityNormal;
        lowpower_thread_id = osThreadNew((osThreadFunc_t)lowpower_thread, &sleep, &threadattr_lowpower);
    }
    osSignalSet(lowpower_thread_id, LOW_POWER_SLEEP_START_SIGNAL);
}

static void test_sleep_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring("  utest_sleep gpio [pin_num ,pin_pull ,pin_inout, out_val]\r\n");
    eshell_putstring("  [pin_pull:0 nopull/1 up/2 down, pin_inout:0 in/1 out, out_val]\r\n");
#if defined(__SYS_AS_MAIN__)
    eshell_putstring("  utest_sleep pmu_gpio [pin_num ,pin_pull ,pin_inout, out_val]\r\n");
    eshell_putstring("  [pin_pull:0 nopull/1 up/2 down, pin_inout:0 in/1 out, out_val]\r\n");
#endif
    eshell_putstring("  utest_sleep sleep \r\n");
}

static void unitest_sleep(int argc, char *argv[])
{
    if (argc < 3)
    {
        goto usage;
    }
    if (strncmp(argv[1], "sleep", 5) == 0)
    {
        if (argc > 3)
        {
            goto usage;
        }
        uint32_t sleep_time;
        sleep_time = atoi(argv[2]);
        into_lowpower(sleep_time);
        return;
    }
    else
    {
        goto usage;
    }

usage:
    test_sleep_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_sleep", "usage: utest_sleep help",
                   unitest_sleep);

#endif // UTILS_ESHELL_EN
