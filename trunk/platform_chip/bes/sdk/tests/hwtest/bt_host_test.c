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
#ifdef BT_HOST_TEST

#include "bt_host.h"
#include "bt_host_test.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_key.h"
#include "hal_location.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtest.h"
#include "pmu.h"
#ifdef SYS2BTH_MSG_TEST
#include "rmt_msg_test.h"
#endif
#include "string.h"

static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done;

#ifdef BTH_BT_TEST
static volatile bool bt_test_done;
#endif
#ifdef BTH_MEM_TEST
static volatile bool bth_mt_done;
#endif

#ifdef SLEEP_TEST
#ifndef NO_TIMER
static void timer_handler(void *param)
{
    static uint32_t wakeup_cnt = 0;
    bt_host_send((uint8_t *)&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
}
#endif

static void bt_host_sleep_test(void)
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
            TR_INFO(0, "bt_host alive!");
            handshake_done = true;

            return len;
        }
    }

#ifdef SYS2BTH_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

    POSSIBLY_UNUSED uint32_t rlen = 0;

#ifdef BTH_MEM_TEST
    if (bth_rx_data_handler[BTH_RX_DATA_HDLR_MT]) {
        rlen = bth_rx_data_handler[BTH_RX_DATA_HDLR_MT](data, len);
        if (rlen == len) {
            bth_mt_done = true;
            return len;
        }
    }
#endif

#ifdef BTH_BT_TEST
    if (bth_rx_data_handler[BTH_RX_DATA_HDLR_BT]) {
        rlen = bth_rx_data_handler[BTH_RX_DATA_HDLR_BT](data, len);
        if (rlen == len) {
            bt_test_done = true;
            return len;
        }
    }
#endif

    return len;
}

static void tx_handler(const void *data, unsigned int len)
{
#ifdef SYS2BTH_MSG_TEST
    rmt_msg_test_tx_handler(data, len);
#endif
}

#ifdef SYS2BTH_MSG_TEST
static void sys2bth_msg_test_server(void)
{
    rmt_msg_test(false, bt_host_send, NULL);
}
#endif

void bt_host_test(void)
{
    int ret;

    TRACE(0, "%s", __func__);

#ifdef SLEEP_TEST
    bt_host_sleep_test();
#endif

#ifdef SYS2BTH_MSG_TEST
    sys2bth_msg_test_server();
#endif

    ret = bt_host_open(rx_handler, tx_handler);
    ASSERT(ret == 0, "bt_host_open failed: %d", ret);

#ifdef BTH_BT_TEST
    while (!bt_test_done);
    TRACE(0, "bt_test_done");
#endif
#ifdef BTH_MEM_TEST
    while (!bth_mt_done);
    TRACE(0, "bth_mt_done");
#endif
}

#endif
