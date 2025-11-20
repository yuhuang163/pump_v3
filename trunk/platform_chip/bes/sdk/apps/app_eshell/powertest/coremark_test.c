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

#ifdef UTILS_ESHELL_EN

#include <stdio.h>
#include <stdarg.h>
#include "plat_types.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_sleep.h"
#include "eshell.h"
#include "app_eshell.h"
#include "pmu.h"
#include "coremark.h"

typedef struct
{
    uint32_t coremark_start;
    uint32_t wfi_start;
    uint16_t sys_freq;
} _POWER_CONSUMPTION_START_FLAG_T;

_POWER_CONSUMPTION_START_FLAG_T __attribute((section(".reboot_param"))) _power_consumption_info = 
{0, 0, HAL_CMU_FREQ_24M};

static void do_coremark_test(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: coremark_test <1>\r\n");
        return;
    }
    enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

    if (strncmp(argv[1], "24M", 3) == 0) {
        freq = HAL_CMU_FREQ_24M;
        eshell_putstring("set sys freq to 24M");
    } else if (strncmp(argv[1], "48M", 3) == 0) {
        freq = HAL_CMU_FREQ_48M;
        eshell_putstring("set sys freq to 48M");
    } else if (strncmp(argv[1], "72M", 3) == 0) {
        freq = HAL_CMU_FREQ_72M;
        eshell_putstring("set sys freq to 72M");
    } else if (strncmp(argv[1], "96M", 3) == 0) {
        freq = HAL_CMU_FREQ_96M;
        eshell_putstring("set sys freq to 96M");
    } else if (strncmp(argv[1], "144M", 4) == 0) {
        freq = HAL_CMU_FREQ_144M;
        eshell_putstring("set sys freq to 144M");
    } else if (strncmp(argv[1], "192M", 4) == 0) {
        freq = HAL_CMU_FREQ_192M;
        eshell_putstring("set sys freq to 192M");
    }else {
        eshell_putstring("set sys freq error");
        return;
    }

    _power_consumption_info.coremark_start = 1;
    _power_consumption_info.sys_freq = freq;
    pmu_reboot();

}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sys_coremark_test", "sys coremark test", do_coremark_test);

static void do_coremark_and_app_test(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: coremark_and_app_test <1>\r\n");
        return;
    }
    enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

    if (strncmp(argv[1], "24M", 3) == 0) {
        freq = HAL_CMU_FREQ_24M;
        eshell_putstring("set sys freq to 24M");
    } else if (strncmp(argv[1], "48M", 3) == 0) {
        freq = HAL_CMU_FREQ_48M;
        eshell_putstring("set sys freq to 48M");
    } else if (strncmp(argv[1], "72M", 3) == 0) {
        freq = HAL_CMU_FREQ_72M;
        eshell_putstring("set sys freq to 72M");
    } else if (strncmp(argv[1], "96M", 3) == 0) {
        freq = HAL_CMU_FREQ_96M;
        eshell_putstring("set sys freq to 96M");
    } else if (strncmp(argv[1], "144M", 4) == 0) {
        freq = HAL_CMU_FREQ_144M;
        eshell_putstring("set sys freq to 144M");
    } else if (strncmp(argv[1], "192M", 4) == 0) {
        freq = HAL_CMU_FREQ_192M;
        eshell_putstring("set sys freq to 192M");
    }else {
        eshell_putstring("set sys freq error");
        return;
    }

    _power_consumption_info.coremark_start = 2;
    _power_consumption_info.sys_freq = freq;
    pmu_reboot();

}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sys_coremark_and_app_test", "sys coremark and app test", do_coremark_and_app_test);

static void sys_freq_test(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: wfi_under_freq_test <1>\r\n");
        return;
    }

    enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

    if (strncmp(argv[1], "32K", 3) == 0) {
        freq = HAL_CMU_FREQ_32K;
        eshell_putstring("set sys freq to 32K");
    } else if (strncmp(argv[1], "24M", 3) == 0) {
        freq = HAL_CMU_FREQ_24M;
        eshell_putstring("set sys freq to 24M");
    } else if (strncmp(argv[1], "48M", 3) == 0) {
        freq = HAL_CMU_FREQ_48M;
        eshell_putstring("set sys freq to 48M");
    } else if (strncmp(argv[1], "72M", 3) == 0) {
        freq = HAL_CMU_FREQ_72M;
        eshell_putstring("set sys freq to 72M");
    } else if (strncmp(argv[1], "96M", 3) == 0) {
        freq = HAL_CMU_FREQ_96M;
        eshell_putstring("set sys freq to 96M");
    } else if (strncmp(argv[1], "144M", 4) == 0) {
        freq = HAL_CMU_FREQ_144M;
        eshell_putstring("set sys freq to 144M");
    } else if (strncmp(argv[1], "192M", 4) == 0) {
        freq = HAL_CMU_FREQ_192M;
        eshell_putstring("set sys freq to 192M");
    }else {
        eshell_putstring("set sys freq error");
        return;
    }

    _power_consumption_info.wfi_start = 1;
    _power_consumption_info.sys_freq = freq;
    pmu_reboot();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sys_wfi_test", "sys wfi test", sys_freq_test);

#if defined(SENSOR_HUB)
extern void app_mcu_sensor_hub_coremark_wfi_power_test_req(uint32_t coremark_test_mode, enum HAL_CMU_FREQ_T freq);

static void sensor_hub_coremark_test(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: wfi_under_freq_test <1>\r\n");
        return;
    }

    enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

    if (strncmp(argv[1], "24M", 3) == 0) {
        freq = 6;
        eshell_putstring("set sensor hub freq to 24M");
    } else if (strncmp(argv[1], "48M", 3) == 0) {
        freq = 10;
        eshell_putstring("set sensor hub freq to 48M");
    } else if (strncmp(argv[1], "96M", 3) == 0) {
        freq = 14;
        eshell_putstring("set sensor hub freq to 96M");
    } else {
        eshell_putstring("set sensor hub freq error");
        return;
    }

    app_mcu_sensor_hub_coremark_wfi_power_test_req(1, freq);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sensor_hub_coremark_test",
                                              "sensor hub coremark test",
                                               sensor_hub_coremark_test);

static void sensor_hub_coremark_and_app_test(int argc, char *argv[])
{
   if (argc < 2) {
       eshell_putstring("Usage: wfi_under_freq_test <1>\r\n");
       return;
   }

   enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

   if (strncmp(argv[1], "24M", 3) == 0) {
       freq = 6;
       eshell_putstring("set sensor hub freq to 24M");
   } else if (strncmp(argv[1], "48M", 3) == 0) {
       freq = 10;
       eshell_putstring("set sensor hub freq to 48M");
   } else if (strncmp(argv[1], "96M", 3) == 0) {
       freq = 14;
       eshell_putstring("set sensor hub freq to 96M");
   } else {
       eshell_putstring("set sensor hub freq error");
       return;
   }

   app_mcu_sensor_hub_coremark_wfi_power_test_req(2, freq);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sensor_hub_coremark_and_app_test",
                                              "sensor hub coremark and app test",
                                              sensor_hub_coremark_and_app_test);

static void sensor_hub_wfi_test(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: wfi_under_freq_test <1>\r\n");
        return;
    }

    enum HAL_CMU_FREQ_T freq = HAL_CMU_FREQ_QTY;

    if (strncmp(argv[1], "24M", 3) == 0) {
        freq = 6;
        eshell_putstring("set sensor hub freq to 24M");
    } else if (strncmp(argv[1], "48M", 3) == 0) {
        freq = 10;
        eshell_putstring("set sensor hub freq to 48M");
    } else if (strncmp(argv[1], "96M", 3) == 0) {
        freq = 14;
        eshell_putstring("set sensor hub freq to 96M");
    } else {
        eshell_putstring("set sensor hub freq error");
        return;
    }

    app_mcu_sensor_hub_coremark_wfi_power_test_req(0, freq);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_POWER, "sensor_hub_wfi_test",
                                              "sensor hub wfi test",
                                                          sensor_hub_wfi_test);
#endif

uint32_t get_coremark_start_info(void)
{
    uint32_t start = _power_consumption_info.coremark_start;
    TRACE(1, "%s start %d %d", __func__, _power_consumption_info.coremark_start, start);
    _power_consumption_info.coremark_start = 0;
    return start;
}

uint32_t get_wfi_start_info(void)
{
    uint32_t start = _power_consumption_info.wfi_start;
    TRACE(1, "%s start %d %d", __func__, _power_consumption_info.wfi_start, start);
    _power_consumption_info.wfi_start = 0;
    return start;
}

uint16_t get_sys_freq(void)
{
    TRACE(1, "%s sys ferq %d", __func__, _power_consumption_info.sys_freq);
    return _power_consumption_info.sys_freq;
}

#endif //UTILS_ESHELL_EN
