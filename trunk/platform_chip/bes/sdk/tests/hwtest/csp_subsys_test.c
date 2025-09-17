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
#ifdef CSP_SUBSYS_TEST

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "csp_subsys.h"
#include "csp_subsys_msg.h"
#include "csp_subsys_msg_test.h"
#include "hal_key.h"
#include "hal_location.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtest.h"
#include "pmu.h"
#if defined(NOAPP) && defined(MCU2CSP_MSG_TEST)
#include "rmt_msg_test.h"
#endif
#include "string.h"

#ifdef NOAPP
static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done;

#ifdef SLEEP_TEST
#ifndef NO_TIMER
static void timer_handler(void *param)
{
    static uint32_t wakeup_cnt = 0;
    wakeup_cnt++;
}
#endif

static void csp_subsys_sleep_test(void)
{
#ifndef NO_TIMER
    sleep_test_register_timer_callback(timer_handler);
#endif

    // Jump to next test case and the last one is SLEEP_TEST
}
#endif
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
#ifdef NOAPP
    if (!handshake_done) {
        if (data && len == (sizeof(handshake_msg) - 1) &&
                memcmp(data, handshake_msg, len) == 0) {
            TR_INFO(0, "csp_subsys alive!");
            handshake_done = true;

            return len;
        }
    }

    if (len == sizeof(struct CSP_SUBSYS_MSG_T)) {
        const struct CSP_SUBSYS_MSG_T *msg = (const struct CSP_SUBSYS_MSG_T *)data;
        if (msg->hdr.id == CSP_SUBSYS_MSG_ID_SET_FREQ && !msg->hdr.reply) {
            TRACE(0, "CSP_SUBSYS: Req sys freq for CP: %u", msg->param[0]);
            // TODO: Add a freq user for CSP_SUBSYS
            hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, (enum HAL_CMU_FREQ_T)msg->param[0]);
        }
    }

#ifdef MCU2CSP_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

#endif
    return len;
}

static void tx_handler(const void *data, unsigned int len)
{
#ifdef NOAPP
#ifdef MCU2CSP_MSG_TEST
    rmt_msg_test_tx_handler(data, len);
#endif

    csp_subsys_msg_release(data, len);
#endif
}

#if defined(NOAPP) && defined(MCU2CSP_MSG_TEST)
static void mcu2cp_msg_test_server(void)
{
    rmt_msg_test(false, csp_subsys_msg_test_send, csp_subsys_get_msg_test_data);
}
#endif

static POSSIBLY_UNUSED int cp_key_evt_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(1, "[%s]: hello key: code=%x, event=%x", __func__, key_code, key_event);

    return 0;
}

void csp_subsys_test(void)
{
    int ret;

    TRACE(0, "%s", __func__);

#ifdef NOAPP
#ifdef MCU2CSP_MSG_TEST
    mcu2cp_msg_test_server();
#endif

#ifdef SLEEP_TEST
    csp_subsys_sleep_test();
#endif
#endif

    ret = csp_subsys_open(rx_handler, tx_handler);
    ASSERT(ret == 0, "csp_subsys_open failed: %d", ret);
}

#endif
