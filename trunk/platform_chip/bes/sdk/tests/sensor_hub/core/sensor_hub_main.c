/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include "plat_addr_map.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_rmt_ipc.h"
#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
#include "hal_norflash.h"
#endif
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "sens_comp_test.h"
#include "sens_mem.h"
#include "sens_trc_to_mcu.h"
#include "sensor_hub.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"

#include "hal_uart.h"
#include "eshell.h"
#include "app_eshell.h"

#ifdef OPENAMP_ENABLE
#include <rptun/chips/rptun_chips.h>
#endif

#ifdef DMA_AUDIO_APP
#include "dma_audio_app.h"
#endif

#if defined(__BT_APP__)
#include "norflash_api.h"

static osThreadId main_thread_tid = NULL;

int system_shutdown(void)
{
    TR_INFO(TR_MOD(MAIN), "system_shutdown!!");
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    return 0;
}

int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x8);
    return 0;
}

int signal_send_to_main_thread(uint32_t signals)
{
    osSignalSet(main_thread_tid, signals);
    return 0;
}
#endif

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

int sensor_hub_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int sensor_hub_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int sensor_hub_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}

static sensor_hub_core_rx_irq_handler_t sensorHubCoreRxIrqHandler[CORE_IRQ_HDLR_ID_QTY] = {NULL};
static sensor_hub_core_tx_done_irq_handler_t sensorHubCoreTxDoneIrqHandler[CORE_IRQ_HDLR_ID_QTY] = {NULL};

void sensor_hub_core_register_rx_irq_handler(enum CORE_IRQ_HDLR_ID_T id,
    sensor_hub_core_rx_irq_handler_t irqHandler)
{
    sensorHubCoreRxIrqHandler[id] = irqHandler;
}

void sensor_hub_core_register_tx_done_irq_handler(enum CORE_IRQ_HDLR_ID_T id,
    sensor_hub_core_tx_done_irq_handler_t irqHandler)
{
    sensorHubCoreTxDoneIrqHandler[id] = irqHandler;
}

#ifdef NOAPP
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
SRAM_RODATA_LOC
#endif
static const uint8_t handshake_msg[] = "Hi";
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
    uint32_t i = CORE_IRQ_HDLR_PRIO_LOW;
    uint32_t ret = len;

#ifdef OPENAMP_ENABLE
    bes_bth2sens_rx_handler(data, len);
#endif

#ifdef DMA_AUDIO_APP
    for (i = 0; i < CORE_IRQ_HDLR_ID_QTY; i++) {
        if (sensorHubCoreRxIrqHandler[i]) {
            ret = sensorHubCoreRxIrqHandler[i](data, len);
            if (ret == len) {
                break;
            }
        }
    }
#else
    if (sensorHubCoreRxIrqHandler[i]) {
        ret = sensorHubCoreRxIrqHandler[i](data, len);
    }
#endif

    return ret;
}

static void tx_handler(const void *data, unsigned int len)
{
    uint32_t i = CORE_IRQ_HDLR_PRIO_LOW;

#ifdef DMA_AUDIO_APP
    for (i = 0; i < CORE_IRQ_HDLR_ID_QTY; i++)
#endif
    {
        if (sensorHubCoreTxDoneIrqHandler[i]) {
            sensorHubCoreTxDoneIrqHandler[i](data, len);
        }
    }

#ifdef OPENAMP_ENABLE
    bes_bth2sens_tx_handler(data, len);
#endif
}

#if (!defined(NOAPP))&&defined(OS_THREAD_TIMING_STATISTICS_ENABLE)
#if defined(KERNEL_RTX5)
extern void rtx_show_all_threads_usage(void);
#else
extern void freertos_show_all_threads(void);
#endif
static void cpu_usage_timer_handler(void const *param);
osTimerDef(cpu_usage_timer, cpu_usage_timer_handler);
static osTimerId cpu_usage_timer_id = NULL;
static void cpu_usage_timer_handler(void const *param)
{
#if defined(KERNEL_RTX5)
    rtx_show_all_threads_usage();
#else
    freertos_show_all_threads();
#endif
}
#endif

#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE)
static void slow_timer_calibration_thread(const void *arg)
{
    int count = 0;
    while(1) {
        hal_sys_timer_calib();
        if (count < 10) {
            //calibrate slow time more often after power up, utils 32K stabilized
            osDelay(500);
            count ++;
        } else {
            osDelay(3000);
        }
    }
}

osThreadDef(slow_timer_calibration_thread, osPriorityBelowNormal, 1,
    APP_THREAD_TIMER_CALIB_STACK_SIZE, "slow_timer_calibration_thread");

static osThreadId POSSIBLY_UNUSED slow_timer_thread_id = NULL;

static void POSSIBLY_UNUSED sensor_hub_init_slow_timer_calib_thread(void)
{
    if (slow_timer_thread_id == NULL) {
        slow_timer_thread_id = osThreadCreate(osThread(slow_timer_calibration_thread), NULL);
    }
}
#endif

int MAIN_ENTRY(void)
{
    int ret;

    hwtimer_init();
    hal_dma_open();

#ifdef SENS_TRC_TO_MCU
    sens_trace_to_mcu();
#endif

#if defined(DEBUG) && !defined(__NuttX__)
    enum HAL_TRACE_TRANSPORT_T transport;

#ifdef SENS_TRC_TO_MCU
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

    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_MCU, HAL_RMT_IPC_CHAN_0, rx_handler, tx_handler, false, &rmt_ipc_ep);
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
#else
#ifdef CUSTOM_SENSOR
    sensor_hub_core_custom_app_init();
#else
    sensor_hub_core_app_init();
#endif
#endif

#if (!defined(NOAPP))&&defined(OS_THREAD_TIMING_STATISTICS_ENABLE)
    cpu_usage_timer_id = osTimerCreate(osTimer(cpu_usage_timer), osTimerPeriodic, NULL);
    if (cpu_usage_timer_id != NULL) {
        osTimerStart(cpu_usage_timer_id, OS_THREAD_TIMING_STATISTICS_PEROID_MS);
    }
#endif

#ifdef FULL_WORKLOAD_MODE_ENABLED
    sensor_hub_init_workload_thread();
#endif

#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE) && !defined(SLEEP_PD_CURRENT_TEST)
    sensor_hub_init_slow_timer_calib_thread();
#endif

#ifdef APP_MCPP_SRV
    extern void mcpp_srv_open(void);
    mcpp_srv_open();
#endif

#if defined(__BT_APP__)
    extern int app_init(void);
    main_thread_tid = osThreadGetId();
#ifndef RAM_NV_RECORD
    norflash_api_init();
#endif
    ret = app_init();
    TRACE(0,"app_init done:%d",ret);
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

#ifdef OPENAMP_ENABLE
    rptun_init_bth2sens();

#ifdef OPENAMP_ECHO_EXAMPLE
    extern int rpmsg_echo_bth2sens_slave(int argc, char *argv[]);
    rpmsg_echo_bth2sens_slave(0, NULL);
#endif
#endif

#ifdef DMA_AUDIO_APP
    dma_audio_app_init(true);
#ifndef DMA_AUDIO_APP_DYN_ON
    dma_audio_app_on(true);
#endif
#endif

#ifdef UTILS_ESHELL_EN
    app_eshell_rpc_init();
    eshell_rpc_run_cmd_func_cb_register(app_eshell_rpc_send_op_cmd_func, app_eshell_rpc_send_op_result_func);
    eshell_open(HAL_UART_ID_0, false);
#ifdef OPENAMP_TARGET
    extern int rpmsg_target_init(char *str);
    rpmsg_target_init(NULL);
#endif
#endif

    sensor_hub_test_cases();

    while (1) {
#ifdef DMA_AUDIO_APP
        dma_audio_app_thread();
#endif /* DMA_AUDIO_APP */

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
