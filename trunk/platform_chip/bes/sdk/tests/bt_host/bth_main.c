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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "bt_host.h"
#include "bt_host_core.h"
#include "bth_comp_test.h"
#include "bth_trc_to_sys.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
#include "hal_norflash.h"
#endif
#include "hal_rmt_ipc.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#ifdef HWTEST
#include "hwtest.h"
#endif
#include "hwtimer_list.h"
#include "main_entry.h"
#ifdef SYS2BTH_MSG_TEST
#include "rmt_msg_test.h"
#endif
#if defined(BTH_TRC_TO_SYS) && defined(RMT_TRC_IN_MSG_CHAN)
#include "rmt_trace_client.h"
#endif
#if defined(BTH_MEM_TEST) || defined(SINGLE_UART_TEST)
#include "mem_test.h"
#endif
#ifdef PT_APP
#include "pt_app.h"
#endif

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

int bt_host_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int bt_host_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int bt_host_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

static bt_host_core_rx_irq_handler_t core_rx_handler;
static bt_host_core_tx_done_irq_handler_t core_tx_handler;

void bt_host_core_register_rx_irq_handler(bt_host_core_rx_irq_handler_t irqHandler)
{
    core_rx_handler = irqHandler;
}

void bt_host_core_register_tx_done_irq_handler(bt_host_core_tx_done_irq_handler_t irqHandler)
{
    core_tx_handler = irqHandler;
}

static HWTIMER_CALLBACK_T timer_cb[3];
static void (*loop_cb[3])(void);

void sleep_test_register_timer_callback(HWTIMER_CALLBACK_T cb)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (!timer_cb[i]) {
            timer_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_timer_callback(void *param)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (timer_cb[i]) {
            timer_cb[i](param);
        }
    }
}

void sleep_test_register_loop_callback(void (*cb)(void))
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (!loop_cb[i]) {
            loop_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_loop_callback(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (loop_cb[i]) {
            loop_cb[i]();
        }
    }
}

#ifdef NOAPP
static const uint8_t handshake_msg[] = "Hi";
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
static HWTIMER_ID wakeup_timer;
static uint32_t wakeup_cnt;
static const uint32_t timer_ms = 7;

static void timer_handler(void *param)
{
    bt_host_send(&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;

    sleep_test_exec_timer_callback(param);

    hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
}
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
#if defined(BTH_TRC_TO_SYS) && defined(RMT_TRC_IN_MSG_CHAN)
    if (rmt_trace_client_msg_handler(data, len)) {
        return len;
    }
#endif

    if (core_rx_handler) {
        return core_rx_handler(data, len);
    }

    return len;
}

static void tx_handler(const void *data, unsigned int len)
{
#if defined(BTH_TRC_TO_SYS) && defined(RMT_TRC_IN_MSG_CHAN)
    if (rmt_trace_check_trace(data, len)) {
        return;
    }
#endif

    if (core_tx_handler) {
        core_tx_handler(data, len);
        return;
    }
}

#ifdef SYS2BTH_MSG_TEST
static void sys2bth_msg_test_client(void)
{
    bt_host_core_register_rx_irq_handler(rmt_msg_test_rx_handler);
    bt_host_core_register_tx_done_irq_handler(rmt_msg_test_tx_handler);
    rmt_msg_test(true, bt_host_send, NULL);
}
#endif

static void bth_test_modules_init(void)
{
    POSSIBLY_UNUSED int ret = 0;

#if defined(SLEEP_TEST) || !defined(NOAPP)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
#endif
    hal_sysfreq_print_user_freq();

#ifdef SYS2BTH_MSG_TEST
    sys2bth_msg_test_client();
#endif

#ifdef BTH_ISPI_RF_ENABLE
#ifdef NO_ISPI
#error "BTH_ISPI_RF_ENABLE conflicts with NO_ISPI"
#endif
    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MOD_RF, HAL_IOMUX_ISPI_OWNER_BTH);
#endif

#ifdef BTH_ISPI_XTAL_ENABLE
#ifdef NO_ISPI
#error "BTH_ISPI_XTAL_ENABLE conflicts with NO_ISPI"
#endif
    hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MOD_XTAL, HAL_IOMUX_ISPI_OWNER_BTH);
#endif

/* production test init */
#ifdef PT_APP
    pt_app_entry(NULL);
#else /* !PT_APP */

#ifdef COREMARK_TEST
    coremark_test();
#endif

#ifdef DHRYSTONE_TEST
    dhrystone_test();
#endif

#ifdef SBC_TEST
    sbc_test();
#endif

#if defined(IQ_CALI_TEST) || defined(BTH_BT_TEST)
#if !defined(SIMU) && defined(NO_ISPI)
#error "IQ_CALI_TEST/BTH_BT_TEST conflicts with NO_ISPI unless SIMU=1"
#endif
    bth_bt_test();
#endif /* defined(IQ_CALI_TEST) || defined(BTH_BT_TEST) */

#ifdef BTH_MEM_TEST
    bth_mem_test();
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
    wakeup_timer = hwtimer_alloc(timer_handler, NULL);
    ASSERT(wakeup_timer, "Failed to alloc wakeup_timer");

    ret = hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
#endif

#ifdef SINGLE_UART_TEST
    single_uart_test();
#endif

#endif /* PT_APP */
}

int MAIN_ENTRY(void)
{
    int ret;

    hwtimer_init();
    hal_dma_open();

#ifdef BTH_TRC_TO_SYS
    bth_trace_to_sys();
#endif

#ifdef DEBUG
    enum HAL_TRACE_TRANSPORT_T transport;

#ifdef BTH_TRC_TO_SYS
    transport = HAL_TRACE_TRANSPORT_NULL;
#elif (DEBUG_PORT == 1)
    hal_iomux_set_uart0();

    transport = HAL_TRACE_TRANSPORT_UART0;
#elif (DEBUG_PORT == 2)
    hal_iomux_set_uart1();

    transport = HAL_TRACE_TRANSPORT_UART1;
#else
    transport = HAL_TRACE_TRANSPORT_QTY;
#endif
    hal_trace_open(transport);
#endif

    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SYS_M55C0, HAL_RMT_IPC_CHAN_0, rx_handler, tx_handler, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);
#endif

    hal_sleep_start_stats(10000, 10000);

#ifdef NOAPP
    ret = hal_rmt_ipc_send(rmt_ipc_ep, handshake_msg, (sizeof(handshake_msg) - 1));
    ASSERT(ret == 0, "hal_rmt_ipc_send failed: %d", ret);
#endif

    bth_test_modules_init();

    while (1) {
        sleep_test_exec_loop_callback();
#ifdef NOAPP
#ifdef RTOS
        osDelay(2);
#else
        hal_sleep_enter_sleep();
#endif
#else
        osSignalWait(0x0, osWaitForever);
#endif
    }

    return 0;
}
