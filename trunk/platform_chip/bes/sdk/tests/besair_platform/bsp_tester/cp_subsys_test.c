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
#ifdef CP_SUBSYS_TEST
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cp_subsys.h"
#include "cp_subsys_msg.h"
#include "cp_subsys_msg_test.h"
#include "hal_key.h"
#include "hal_location.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"

#include "pmu.h"
#if defined(NOAPP) && defined(MCU2CP_MSG_TEST)
#include "rmt_msg_test.h"
#endif
#include "string.h"
#ifdef DMA_AUDIO_APP
#include "dma_audio_host.h"
#endif

static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done;

#ifdef SLEEP_TEST
#ifndef NO_TIMER
static void timer_handler(void *param)
{
    static uint32_t wakeup_cnt = 0;
    cp_subsys_msg_test_send(&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
}
#endif

static void cp_subsys_sleep_test(void)
{
#ifndef NO_TIMER
    sleep_test_register_timer_callback(timer_handler);
#endif

    // Jump to next test case and the last one is SLEEP_TEST
}
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
    if (!handshake_done) {
        if (data && len == (sizeof(handshake_msg) - 1) &&
                memcmp(data, handshake_msg, len) == 0) {
            TR_INFO(0, "cp_subsys alive!");
            handshake_done = true;

            return len;
        }
    }

    if (len == sizeof(struct CP_SUBSYS_MSG_T)) {
        const struct CP_SUBSYS_MSG_T *msg = (const struct CP_SUBSYS_MSG_T *)data;
        if (msg->hdr.id == CP_SUBSYS_MSG_ID_SET_FREQ && !msg->hdr.reply) {
            TRACE(0, "CP_SUBSYS: Req sys freq for CP: %u", msg->param[0]);
            // TODO: Add a freq user for CP_SUBSYS
            hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, (enum HAL_CMU_FREQ_T)msg->param[0]);
        }
    }

#ifdef MCU2CP_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

#ifdef DMA_AUDIO_APP
    unsigned int rlen = len;
    rlen = dma_audio_rx_data_handler(data, len);
    if (rlen == len) {
        return len;
    }
#endif

    return len;
}

static void tx_handler(const void *data, unsigned int len)
{
#ifdef NOAPP
#ifdef MCU2CP_MSG_TEST
    rmt_msg_test_tx_handler(data, len);
#endif

#ifdef DMA_AUDIO_APP
    dma_audio_tx_data_handler(data, len);
#endif

    cp_subsys_msg_release(data, len);
#endif
}

#if defined(NOAPP) && defined(MCU2CP_MSG_TEST)
static void mcu2cp_msg_test_server(void)
{
    rmt_msg_test(false, cp_subsys_msg_test_send, cp_subsys_get_msg_test_data);
}
#endif

static POSSIBLY_UNUSED int cp_key_evt_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(1, "[%s]: hello key: code=%x, event=%x", __func__, key_code, key_event);

#ifdef DMA_AUDIO_APP
    dma_audio_app_test_key_handler(key_code, key_event);
#endif

    return 0;
}

void cp_subsys_test(void)
{
    int ret;

    TRACE(0, "%s", __func__);

#ifdef DMA_AUDIO_APP
    hwtest_register_key_callback(cp_key_evt_process);
#endif

#ifdef NOAPP
#ifdef MCU2CP_MSG_TEST
    mcu2cp_msg_test_server();
#endif

#ifdef SLEEP_TEST
    cp_subsys_sleep_test();
#endif
#endif

    ret = cp_subsys_open(rx_handler, tx_handler);
    ASSERT(ret == 0, "cp_subsys_open failed: %d", ret);

#ifdef DMA_AUDIO_APP
    dma_audio_app_test(DMA_AUD_TRIG_MODE_PWRKEY, true);
#endif

}

#endif
