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
#ifdef QSPI_TEST

#include "cmsis.h"
#include "string.h"
#include "hal_qspi.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"

#define QSPI_AS_MASTER

// #define QSPI_STANDARD_MODE
#define QSPI_QUAD_MODE

#define QSPI_DMA_ENABLE

POSSIBLY_UNUSED
static int32_t qspi_write(uint32_t *pbuf, uint16_t len, uint8_t enable_dma)
{
    int ret = 0;

    if (enable_dma) {
        ret = hal_qspi_dma_send(pbuf, len, NULL);
    } else {
        ret = hal_qspi_send(pbuf, len);
    }

    return ret;
}

void qspi_test(void)
{
    struct HAL_QSPI_CFG_T qspi_cfg;
    uint32_t tx_testbuf[6], cnt;
    int ret;

    memset(&qspi_cfg, 0, sizeof(qspi_cfg));

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TR_INFO(TR_MOD(MAIN), "CPU frequency changed to: %uHz", hal_sys_timer_calc_cpu_freq(5, 0));

#ifdef QSPI_AS_MASTER
    TRACE(0, "------QSPI master test started------");
#else
    TRACE(0, "-------QSPI slave test started------");
#endif

#if defined(QSPI_STANDARD_MODE) && defined(QSPI_QUAD_MODE)
#error "QSPI MODE REPETITION"
#elif defined(QSPI_STANDARD_MODE)
    qspi_cfg.mode = HAL_QSPI_STANDARD_MODE;
#elif defined(QSPI_QUAD_MODE)
    qspi_cfg.mode = HAL_QSPI_QUAD_MODE;
#else
#error "PLEASE SELECT QSPI MODE"
#endif

    qspi_cfg.rate = 48 * 1000 * 1000;

    qspi_cfg.cs = HAL_QSPI_CS_0;

#ifdef QSPI_AS_MASTER
    qspi_cfg.slave = false;
#else
    qspi_cfg.slave = true;
#endif

#ifdef QSPI_DMA_ENABLE
    qspi_cfg.dma_rx = true;
    qspi_cfg.dma_tx = true;
#else
    qspi_cfg.dma_rx = false;
    qspi_cfg.dma_tx = false;
#endif

#if defined(QSPI_STANDARD_MODE)
    qspi_cfg.transfer_mode = HAL_QSPI_TX_AND_RX;
#elif defined(QSPI_QUAD_MODE)
    qspi_cfg.transfer_mode = HAL_QSPI_TX_ONLY;
#endif

    qspi_cfg.tx_bits = 32;

    hal_iomux_set_qspi();
    ret = hal_qspi_open(&qspi_cfg);
    if (ret != 0) {
        ASSERT(false, "%s:%d, qspi open fail: %d", __func__, __LINE__, ret);
    } else {
        TRACE(0, "qspi open success");
    }

    tx_testbuf[0] = 0x0000ffff;
    tx_testbuf[1] = 0xffff0000;
    tx_testbuf[2] = 0xf0f0f0f0;
    tx_testbuf[3] = 0x0f0f0f0f;
    tx_testbuf[4] = 0x5555aaaa;
    tx_testbuf[5] = 0xaaaa5555;

    TRACE(0, "qspi send data start");
    cnt = 0;
    while (1) {
        ret = qspi_write(&tx_testbuf[cnt], sizeof(tx_testbuf[0]), qspi_cfg.dma_tx);
        TRACE(0, "DR send to 0x%08x", tx_testbuf[cnt]);
        if (ret != 0) {
            TRACE(0, "data send fail: %d", ret);
        }
        cnt = (cnt + 1) % 6;
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}

#endif
