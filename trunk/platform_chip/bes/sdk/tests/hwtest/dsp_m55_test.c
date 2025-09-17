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
#ifdef DSP_M55_TEST

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "dsp_m55.h"
#include "hal_key.h"
#include "hal_location.h"
#include "hal_codec.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtest.h"
#include "pmu.h"
#ifdef MCU2DSP_M55_MSG_TEST
#include "rmt_msg_test.h"
#endif
#include "string.h"
#ifdef DMA_AUDIO_APP
#include "dma_audio_host.h"
#endif
#ifdef OPENAMP_ENABLE
#include <rptun/chips/rptun_chips.h>
#endif

static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done;

static uint32_t pwr_key_click_cnt;

#ifdef SLEEP_TEST
#ifndef NO_TIMER
static void timer_handler(void *param)
{
    static uint32_t wakeup_cnt = 0;
    dsp_m55_send((uint8_t *)&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
}
#endif

static void dsp_m55_sleep_test(void)
{
#ifndef NO_TIMER
    sleep_test_register_timer_callback(timer_handler);
#endif

    // Jump to next test case and the last one is SLEEP_TEST
}
#endif

POSSIBLY_UNUSED static unsigned int rx_handler(const void *data, unsigned int len)
{
    if (!handshake_done) {
        if (data && len == (sizeof(handshake_msg) - 1) &&
                memcmp(data, handshake_msg, len) == 0) {
            TR_INFO(0, "dsp_m55 alive!");
            handshake_done = true;

            return len;
        }
    }

#ifdef DMA_AUDIO_APP
    unsigned int rlen = dma_audio_rx_data_handler(data, len);
    if (rlen == len) {
        return len;
    }
#endif

#ifdef MCU2DSP_M55_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

#ifdef OPENAMP_ENABLE
    return bes_bth2m55_rx_handler(data, len);
#endif

    return len;
}

POSSIBLY_UNUSED static void tx_handler(const void *data, unsigned int len)
{
#ifdef DMA_AUDIO_APP
    return dma_audio_tx_data_handler(data, len);
#endif

#ifdef MCU2DSP_M55_MSG_TEST
    return rmt_msg_test_tx_handler(data, len);
#endif

#ifdef OPENAMP_ENABLE
    return bes_bth2m55_tx_handler(data, len);
#endif
}

#ifdef MCU2DSP_M55_MSG_TEST
static void mcu2dsp_m55_msg_test_server(void)
{
    rmt_msg_test(false, dsp_m55_send, NULL);
}
#endif

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    if (key_event == HAL_KEY_EVENT_CLICK) {
        if (key_code == HAL_KEY_CODE_PWR) {
            pwr_key_click_cnt++;
        }
    }
#ifdef DMA_AUDIO_APP
    dma_audio_app_test_key_handler(key_code, key_event);
    return 0;
#endif

    return 1;
}

void dsp_m55_test(void)
{
    int ret;

    TRACE(0, "%s", __func__);

    hwtest_register_key_callback(key_event_process);

#ifdef SLEEP_TEST
    dsp_m55_sleep_test();
#endif

#ifdef MCU2DSP_M55_MSG_TEST
    mcu2dsp_m55_msg_test_server();
#endif

#ifdef MAIN_RAM_USE_SYS_RAM
    ret = dsp_m55_open_in_sys_ram(rx_handler, tx_handler);
#else
    ret = dsp_m55_open(rx_handler, tx_handler);
#endif
    ASSERT(ret == 0, "dsp_m55_open failed: %d", ret);

#ifdef DMA_AUDIO_APP
    dma_audio_app_test(DMA_AUD_TRIG_MODE_PWRKEY, true);
#endif

#if defined(OPENAMP_ENABLE) && defined(OPENAMP_ECHO_EXAMPLE)
    extern int rpmsg_echo_bth2m55_master(int argc, char *argv[]);
    rpmsg_echo_bth2m55_master(0, NULL);
#endif
}

#endif
