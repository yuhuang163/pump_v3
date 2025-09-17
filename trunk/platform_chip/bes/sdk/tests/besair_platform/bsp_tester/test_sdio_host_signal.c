/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef TEST_SDIO_HOST_SIGNAL

#include "hal_sdio_host.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "string.h"

#define SDIO_HOST_ID    HAL_SDIO_HOST_ID_0

void sdio_host_signal_test(void)
{
    int ret;
    uint8_t data[4] = {0xAA, 0x5A, 0xA5, 0x55};
    struct HAL_SDIO_HOST_CONFIG_T sdio_cfg;

    TRACE(0, "   ");
    TRACE(0, "------sdio host signal test started------");
    TRACE(0, "***Use an oscilloscope to view the clk, cmd and data pin waveforms of sdio***");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    TRACE(0, "   ");

    memset(&sdio_cfg, 0, sizeof(sdio_cfg));
    sdio_cfg.yield = 0;
    sdio_cfg.dma_en = 1;
    sdio_cfg.ddr_mode = 0;//ip does not support
    sdio_cfg.func_en = 1;
    sdio_cfg.send_abort = 1;
    sdio_cfg.func1_only = 1;
    sdio_cfg.async_int_en = 1;
    sdio_cfg.set_func_blks = 1;
    sdio_cfg.volt_switch = 1;
    sdio_cfg.blocking_init_en = 0;
    sdio_cfg.block_size = 256;
    sdio_cfg.bus_speed = 48 * 1000 * 1000;
    sdio_cfg.bus_width = 4;
    sdio_cfg.callback = NULL;

    hal_sdio_host_signal_out(SDIO_HOST_ID, &sdio_cfg);
    TRACE(1, "------sdio bus speed:%d", hal_sdio_get_host_speed(SDIO_HOST_ID));
    TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");
    TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");
    TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");

    ret = hal_sdio_io_rw_extended(SDIO_HOST_ID, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, 4,
                                  NULL, 1, data, 4);
    TRACE(0, "******ret:%d", ret);
    while (1) {
        TRACE(0, "------Please reset the program");
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}

#endif

