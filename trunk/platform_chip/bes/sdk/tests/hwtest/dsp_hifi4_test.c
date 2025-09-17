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
#ifdef DSP_HIFI4_TEST

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hwtest.h"
#include "hal_key.h"
#include "hal_location.h"
#include "hal_codec.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "dsp_loader.h"
#include "dsp_hifi4_ipc.h"
#include "pmu.h"
#include "string.h"
#ifdef MCU2DSP_HIFI4_MSG_TEST
#include "rmt_msg_test.h"
#endif
#if defined(DSP_HIFI4_TRC_TO_MCU)
#include "rx_dsp_hifi4_trc.h"
#endif
#ifdef OPENAMP_ENABLE
#include <rptun/chips/rptun_chips.h>
#endif
#ifdef DMA_AUDIO_APP
#include "dma_audio_host.h"
#endif

#ifndef BTH_AS_MAIN_MCU
//#define DSP_HIFI4_RPC_TEST
#endif

static uint32_t pwr_key_click_cnt;

POSSIBLY_UNUSED
static unsigned int dsp_rx_data_handler(const void *data, unsigned int len)
{
#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
    if (dsp_hifi4_trace_rx_handler(data, len)) {
        return len;
    }
#endif

#ifdef MCU2DSP_HIFI4_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

#ifdef DMA_AUDIO_APP
    unsigned int rlen = dma_audio_rx_data_handler(data, len);
    if (rlen == len) {
        return len;
    }
#endif

#ifdef OPENAMP_ENABLE
    return bes_bth2hifi4_rx_handler(data, len);
#endif

#if defined(DSP_HIFI4_RPC_TEST) && !defined(BTH_AS_MAIN_MCU)
    static const char dsp_str[] = "DSP READY";
    if (len >= sizeof(dsp_str) &&
            memcmp(data, dsp_str, sizeof(dsp_str) - 1) == 0) {
        TRACE(0, "[%s] recv: %s", __func__, (char *)data);
        char *str = "M55 RECV OK!";
        dsp_hifi4_send((uint8_t *)str, strlen(str) + 1);
    }
#endif

    return len;
}

POSSIBLY_UNUSED
static void dsp_tx_data_done_handler(const void* data, unsigned int len)
{
    TRACE(0, "[%s] len=%d", __func__, len);

#ifdef MCU2DSP_HIFI4_MSG_TEST
    return rmt_msg_test_tx_handler(data, len);
#endif

#ifdef DMA_AUDIO_APP
    dma_audio_tx_data_handler(data, len);
#endif

#ifdef OPENAMP_ENABLE
    bes_bth2hifi4_tx_handler(data, len);
#endif
}

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(3,"%s: code=0x%X, event=%u", __FUNCTION__, key_code, key_event);

    if (key_event == HAL_KEY_EVENT_CLICK) {
        if (key_code == HAL_KEY_CODE_PWR) {
            pwr_key_click_cnt++;
        }
    }
#ifdef DMA_AUDIO_APP
    dma_audio_app_test_key_handler(key_code, key_event);
#endif

    return 0;
}

#ifdef MCU2DSP_HIFI4_MSG_TEST
static void mcu2dsp_hifi4_msg_test_server(void)
{
    rmt_msg_test(false, dsp_hifi4_send, NULL);
}
#endif

void dsp_hifi4_test(void)
{
    int ret = 0;

    hwtest_register_key_callback(key_event_process);

    dsp_open();

#if defined(BTH_AS_MAIN_MCU) && defined(BTH_USE_SYS_PERIPH)
    // For BTH as main CPU:
    // Cannot sleep when dsp is running if BTH_USE_SYS_PERIPH=1
    hal_sysfreq_req(HAL_SYSFREQ_USER_DSP, HAL_CMU_FREQ_26M);
#endif

#ifdef MCU2DSP_HIFI4_MSG_TEST
    mcu2dsp_hifi4_msg_test_server();
#endif

    ret = dsp_hifi4_ipc_open(dsp_rx_data_handler, dsp_tx_data_done_handler);
    ASSERT(ret == 0, "dsp_hifi4_ipc_open failed: %d", ret);

#ifdef DMA_AUDIO_APP
    dma_audio_app_test(DMA_AUD_TRIG_MODE_PWRKEY, true);
#endif

#ifdef OPENAMP_ENABLE
    uint32_t load_base = dsp_get_load_base();

    rptun_init_bth2hifi4(load_base);
#endif

#if defined(OPENAMP_ENABLE) && defined(OPENAMP_ECHO_EXAMPLE)
    int rpmsg_echo_bth2hifi4_master(int argc, char *argv[]);
    rpmsg_echo_bth2hifi4_master(0, NULL);
#endif
}

#endif
