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
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cp_subsys.h"
#include "cp_subsys_core.h"
#include "cp_subsys_msg.h"
#include "cp_subsys_trc_to_mcu.h"
#include "cp_subsys_msg_test.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_rmt_ipc.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#ifdef HWTEST
#include "hwtest.h"
#endif
#if defined(APP_MCPP_SRV) && !defined(RTOS)
#include "mcpp_cmd.h"
extern void mcpp_server_baremetal_main(void);
#endif
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "test_hal_main.h"

static cp_subsys_core_rx_irq_handler_t core_rx_handler[CORE_IRQ_HDLR_ID_QTY]={0,};
static cp_subsys_core_tx_done_irq_handler_t core_tx_handler[CORE_IRQ_HDLR_ID_QTY]={0,};
POSSIBLY_UNUSED static cp_subsys_core_set_sys_freq_cb core_sys_freq_cb;

static uint8_t cp_subsys_busy = false;

uint8_t cp_subsys_is_busy(void)
{
    return __sync_fetch_and_add(&cp_subsys_busy, 0);
}

void cp_subsys_set_busy(uint8_t en)
{
    __sync_lock_test_and_set(&cp_subsys_busy, en);
}

void cp_subsys_wait_busy(void)
{
    while (1) {
        if (__sync_fetch_and_add(&cp_subsys_busy, 0) == false)
            break;
    }
}

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

int cp_subsys_open(CP_SUBSYS_RX_IRQ_HANDLER rx_hdlr, CP_SUBSYS_TX_IRQ_HANDLER tx_hdlr)
{
    int ret;

    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_CORE0, HAL_RMT_IPC_CHAN_0, rx_hdlr, tx_hdlr, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

    return 0;
}

int cp_subsys_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int cp_subsys_send(const void *data, unsigned int len)
{
    cp_subsys_wait_busy();
    cp_subsys_set_busy(true);
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int cp_subsys_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

int cp_subsys_close(void)
{
    return hal_rmt_ipc_close(rmt_ipc_ep);
}

void cp_subsys_core_register_rx_irq_handler(enum CORE_IRQ_HDLR_ID_T id, cp_subsys_core_rx_irq_handler_t irqHandler)
{
    ASSERT(id < CORE_IRQ_HDLR_ID_QTY, "[%s]: Bad id %d", __func__, id);
    core_rx_handler[id] = irqHandler;
}

void cp_subsys_core_register_tx_done_irq_handler(enum CORE_IRQ_HDLR_ID_T id, cp_subsys_core_tx_done_irq_handler_t irqHandler)
{
    ASSERT(id < CORE_IRQ_HDLR_ID_QTY, "[%s]: Bad id %d", __func__, id);
    core_tx_handler[id] = irqHandler;
}

#if !defined(RPMSG_SYSFREQ)
void cp_subsys_core_register_sys_freq_cb(cp_subsys_core_set_sys_freq_cb cb)
{
    core_sys_freq_cb = cb;
    if (cb) {
        cb(hal_sysfreq_get());
    }
}
#endif

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
static void sleep_test_exec_loop_callback(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (loop_cb[i]) {
            loop_cb[i]();
        }
    }
}

#if !defined(OPENAMP_ENABLE) && !defined(USE_BES_RPCHAN)
static const uint8_t handshake_msg[] = "Hi";

static void set_sys_freq_test(unsigned int freq)
{
    static struct CP_SUBSYS_MSG_T msg;
    int ret;
    uint32_t seq;

    msg.hdr.id = CP_SUBSYS_MSG_ID_SET_FREQ;
    msg.hdr.reply = 0;
    msg.seq = 0;
    msg.param[0] = freq;

    ret = cp_subsys_send_seq(&msg, sizeof(msg), &seq);
    ASSERT(ret == 0, "Failed to send SET_FERQ msg: %d", ret);
    while (cp_subsys_tx_active(seq)) {
        hal_rmt_ipc_tx_irq_run(rmt_ipc_ep);
    }
}
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
    uint32_t ret_len = len;
    uint32_t i = CORE_IRQ_HDLR_PRIO_LOW;

    if (core_rx_handler[i]) {
        ret_len = core_rx_handler[i](data, len);
    }

    return ret_len;
}

static void tx_handler(const void *data, unsigned int len)
{
    uint32_t i = CORE_IRQ_HDLR_PRIO_LOW;

    {
        if (core_tx_handler[i]) {
            core_tx_handler[i](data, len);
        }
    }

    if (cp_subsys_is_busy()) {
        cp_subsys_set_busy(false);
    }
}

static int cp_subsys_set_sys_freq(enum HAL_CMU_FREQ_T freq)
{
    TRACE(0, "%s: Req sys freq: %u (cb_valid=%d)", __func__, freq, !!core_sys_freq_cb);

    if (core_sys_freq_cb) {
        core_sys_freq_cb(freq);
    }
    return 0;
}

static void cp_subsys_test_modules_init(void)
{
    POSSIBLY_UNUSED int ret = 0;

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

    hal_sysfreq_print_user_freq();

}

static void chip_init(void)
{
    hwtimer_init();
    hal_dma_open();

#ifdef CP_SUBSYS_TRC_TO_MCU
    cp_subsys_trace_to_mcu();
#endif

#ifdef DEBUG
    enum HAL_TRACE_TRANSPORT_T transport;

#ifdef CP_SUBSYS_TRC_TO_MCU
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
}


int MAIN_ENTRY(void)
{
    POSSIBLY_UNUSED int ret;

    chip_init();

    cp_subsys_open(rx_handler, tx_handler);

#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);
#endif

    hal_sleep_start_stats(10000, 10000);

#if /*!defined(RTOS) && */!defined(OPENAMP_ENABLE) && !defined(USE_BES_RPCHAN)
    ret = hal_rmt_ipc_send(rmt_ipc_ep, handshake_msg, (sizeof(handshake_msg) - 1));
    ASSERT(ret == 0, "hal_rmt_ipc_send failed: %d", ret);

    cp_subsys_core_register_sys_freq_cb(set_sys_freq_test);
#endif

    hal_sysfreq_subsys_set_freq_register(cp_subsys_set_sys_freq);

    cp_subsys_test_modules_init();

    //test hal pattern
#ifdef TEST_HAL_PATTERN
    test_hal_pattern();
#endif

    while (1) {
        sleep_test_exec_loop_callback();

#ifdef RTOS
        osDelay(5000);
        TRACE(0,"CP ALIVE");
        //osSignalWait(0x0, osWaitForever);
#else
        hal_sleep_enter_sleep();
#endif
    }

    return 0;
}
