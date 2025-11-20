/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "cmsis.h"
#include "hal_location.h"
#include "plat_types.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_key.h"
#include "hal_sleep.h"
#include "pmu.h"

#include "cqueue.h"
#include "eshell.h"

static POSSIBLY_UNUSED enum HAL_UART_ID_T eshell_uart_id = HAL_UART_ID_QTY;
static bool uart_enable = false;

#define SIGNAL_OF_UART_DATA_TX_DONE         (1<<9)
#define SIGNAL_OF_UART_DATA_RX_READY        (1<<10)

#define UART_TX_TIMEOUT_MS          100

#ifdef ESHELL_BAUDRATE
#define UART_BAUDRATE ESHELL_BAUDRATE
#else
#define UART_BAUDRATE 1152000
#endif

static char oneline_buff[ONELINE_BUFF_SIZE] __attribute__((aligned(4)));
static osMutexId_t trace_send_mutex_id = NULL;
static const osMutexAttr_t MutexAttr_trace = {
    .name = "trace_send_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = NULL,
    .cb_size = 0U,
};

//#define ESHELL_ON_TRACE

#ifdef ESHELL_ON_TRACE
#define ESHELL_SRAM_BSS_LOC SRAM_BSS_LOC
#else
#define ESHELL_SRAM_BSS_LOC
#endif /* ESHELL_ON_TRACE */

#ifdef ESHELL_USB_CDC
int eshell_cdc_init(uint32_t (*cb)(uint8_t *buf, uint32_t len));
int eshell_cdc_send(uint8_t *buf, uint32_t size);
void eshell_cdc_close(void);
#else
static ESHELL_SRAM_BSS_LOC unsigned char rx_buff[ONELINE_BUFF_SIZE];
#endif

osThreadId_t tx_thread_id = NULL;
#ifndef ESHELL_ON_TRACE
static union HAL_UART_IRQ_T uart_irq_mask;
#endif
static CQueue rx_queue;
static unsigned char rx_queue_buff[ONELINE_BUFF_SIZE];

static osThreadId eshell_thread_id = NULL;
static void eshell_thread(const void *arg);

static uint64_t os_thread_def_stack_eshell_thread [ROUND_UP(1024*4, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_eshell = {
    .name = "eshell_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_eshell_thread,
    .stack_size = ROUND_UP(1024*4, 8),
    .priority = osPriorityHigh,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

static uint32_t eshell_rx_cb(uint8_t *buf, uint32_t len)
{
    if (buf[0] == 0)
    {
        return 0;
    }

    EnCQueue(&rx_queue, buf, len);
    osThreadFlagsSet(eshell_thread_id, SIGNAL_OF_UART_DATA_RX_READY);
    return 0;
}

#ifndef ESHELL_ON_TRACE
static void uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    //TRACE(2, "uart_rx_irq:xfer_size=%d,dma_error=0x%x", xfer_size, dma_error);
    if (uart_enable) {
        if (dma_error) {
        } else if (status.BE || status.FE || status.OE || status.PE) {
        } else {
            eshell_rx_cb(rx_buff, xfer_size);
            hal_uart_dma_recv_mask(eshell_uart_id, rx_buff, sizeof(rx_buff), NULL, NULL, &uart_irq_mask);
        }
    }
}

static void uart_tx(uint32_t xfer_size, int dma_error)
{
    if (uart_enable) {
        if (tx_thread_id != NULL) {
            osThreadFlagsSet(tx_thread_id, SIGNAL_OF_UART_DATA_TX_DONE);
        }
    }
}
#endif

static int uart_send_data(unsigned char *buf, unsigned int len)
{
    int ret = 0;
    if (uart_enable) {
#ifdef ESHELL_ON_TRACE
#ifdef ESHELL_USB_CDC
        ret = eshell_cdc_send(buf, len);
#else
        ret = hal_trace_output(buf, len);
#endif
#else
        osMutexAcquire(trace_send_mutex_id, osWaitForever);
        tx_thread_id = osThreadGetId();
        osThreadFlagsClear(SIGNAL_OF_UART_DATA_TX_DONE);
        hal_uart_dma_send(eshell_uart_id, buf, len, NULL, NULL);
        uint32_t flags = osThreadFlagsWait((uint32_t)SIGNAL_OF_UART_DATA_TX_DONE, osFlagsWaitAll, UART_TX_TIMEOUT_MS);
        osMutexRelease(trace_send_mutex_id);
        //TRACE(3, "%s ret=%d tx_len=%d", __func__, (int32_t)evt.status, len);
        if ((flags > 0U) && (flags < 0x80000000U)) {
            ret = 0;
        } else {
            ret = -1;
        }
#endif /* ESHELL_ON_TRACE */
    } else {
        eshell_rpc_run_cmd_func output_func = eshell_rpc_run_result_output_func_get();
        if (output_func) {
            output_func((char*)buf, len);
        }
    }

    return ret;
}

static void eshell_thread(const void *arg)
{
    TRACE(0, "eshell_thread...");
    eshell_process();
#ifdef KERNEL_FREERTOS
    osThreadExit();
#endif
}

static void eshell_uart_rx_sleep(void)
{
#ifdef ESHELL_ON_TRACE
    hal_trace_rx_sleep();
#else
    union HAL_UART_IRQ_T mask;
    hal_uart_stop_dma_recv(eshell_uart_id);
    mask.reg = 0;
    hal_uart_irq_set_mask(eshell_uart_id, mask);
#endif
}

static void eshell_uart_rx_wakeup(void)
{
#ifdef ESHELL_ON_TRACE
    hal_trace_rx_wakeup();
#else
    int ret = hal_uart_dma_recv_mask(eshell_uart_id, rx_buff, sizeof(rx_buff), NULL, NULL, &uart_irq_mask);
    ASSERT(ret == 0, "%s: Failed to start dma rx: %d", __func__, ret);
#endif
}

void eshell_platform_wakeup(void)
{
    TRACE(0, "%s", __FUNCTION__);
    if (uart_enable)
        eshell_uart_rx_wakeup();
}

void eshell_platform_sleep(void)
{
    TRACE(0, "%s", __FUNCTION__);
    if (uart_enable)
        eshell_uart_rx_sleep();
}

#ifdef ESHELL_WAKEUP_TEST
static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(0, "%s: code=0x%X event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (0) {
        } else if (key_event == HAL_KEY_EVENT_CLICK) {
            eshell_platform_wakeup();
        } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {

        }
    }

    return 0;
}
#endif

void eshell_platform_init(uint8_t uart_id, bool uart_open)
{
    uart_enable = uart_open;
    if (uart_open) {
#ifndef ESHELL_ON_TRACE
        struct HAL_UART_CFG_T uart_cfg;
#endif

        InitCQueue(&rx_queue, sizeof(rx_queue_buff), rx_queue_buff);

#ifdef ESHELL_ON_TRACE
#ifdef ESHELL_USB_CDC
        eshell_cdc_init(eshell_rx_cb);
#else
        hal_trace_rx_open(rx_buff, ONELINE_BUFF_SIZE, eshell_rx_cb);
#endif
#else
        eshell_uart_id = (enum HAL_UART_ID_T)uart_id;

        uart_cfg.parity = HAL_UART_PARITY_NONE,
        uart_cfg.stop = HAL_UART_STOP_BITS_1,
        uart_cfg.data = HAL_UART_DATA_BITS_8,
        uart_cfg.flow = HAL_UART_FLOW_CONTROL_NONE,
        uart_cfg.tx_level = HAL_UART_FIFO_LEVEL_1_2,
        uart_cfg.rx_level = HAL_UART_FIFO_LEVEL_1_4,
        uart_cfg.baud = UART_BAUDRATE,
        uart_cfg.dma_rx = true,
        uart_cfg.dma_tx = true,
        uart_cfg.dma_rx_stop_on_err = false;

        hal_uart_open(uart_id, &uart_cfg);
        hal_uart_irq_set_dma_handler(uart_id, uart_rx, uart_tx);

        uart_irq_mask.reg = 0;
        uart_irq_mask.BE = 1;
        uart_irq_mask.FE = 1;
        uart_irq_mask.OE = 1;
        uart_irq_mask.PE = 1;
        uart_irq_mask.RT = 1;
        hal_uart_irq_set_mask(uart_id, uart_irq_mask);

        if (uart_id == HAL_UART_ID_0)
        {
            hal_iomux_set_uart0();
        }
#ifdef UART1_BASE
        else if (uart_id == HAL_UART_ID_1)
        {
            hal_iomux_set_uart1();
        }
#endif
#ifdef UART2_BASE
        else if (uart_id == HAL_UART_ID_2)
        {
            hal_iomux_set_uart2();
        }
#endif
#endif /* ESHELL_ON_TRACE */

        eshell_thread_id = osThreadNew((osThreadFunc_t)eshell_thread, NULL, &ThreadAttr_eshell);
    }
    if (trace_send_mutex_id == NULL) {
        trace_send_mutex_id = osMutexNew(&MutexAttr_trace);
    }

#ifdef ESHELL_WAKEUP_TEST
    hal_key_open(0, key_event_process);
#endif

    TRACE(1, "eshell_paltform_init open=%d", uart_open);
    TRACE(1, "eshell uart info: id=%d, baud=%d,parity=%d,stop=%d,data=%d,flow=%d",
        eshell_uart_id,UART_BAUDRATE,HAL_UART_PARITY_NONE, HAL_UART_STOP_BITS_1,
        HAL_UART_DATA_BITS_8, HAL_UART_FLOW_CONTROL_NONE);
}

void eshell_platform_deinit(void)
{
    hal_trace_rx_close();
}

void eshell_platform_reboot(void)
{
    osDelay(20); // wait echo finished
#ifdef ESHELL_USB_CDC
    eshell_cdc_close();
#endif
    pmu_reboot();
}

void eshell_putchar(char c)
{
    oneline_buff[0] = c;
    uart_send_data((unsigned char *)oneline_buff, 1);
}

unsigned char eshell_getchar(void)
{
    int len = 0;
    uint32_t lock;
    unsigned char ch = 0;

    lock = int_lock();
    len = LengthOfCQueue(&rx_queue);
    int_unlock(lock);

    if (len <= 0) {
        return 0;
    }
    else {
        lock = int_lock();
        DeCQueue(&rx_queue, &ch, 1);
        int_unlock(lock);
    }

    return ch;
}

int eshell_tstc(void)
{
    int len = 0;
    uint32_t lock;

    lock = int_lock();
    len = LengthOfCQueue(&rx_queue);
    int_unlock(lock);
    return len;
}

bool eshell_wait_data_ready(void)
{
    bool ret = false;

    uint32_t flags = osThreadFlagsWait((uint32_t)SIGNAL_OF_UART_DATA_RX_READY, osFlagsWaitAll, osWaitForever);
    if((flags > 0U) && (flags < 0x80000000U)) {
        if ((int32_t)flags & SIGNAL_OF_UART_DATA_RX_READY) {
            ret = true;
        }
        osThreadFlagsClear(0xFFFF);
    }
    return ret;
}

void eshell_enqueue(uint8_t *buf, uint32_t size)
{
    uint32_t lock;
    lock = int_lock();
    EnCQueue(&rx_queue, buf, size);
    int_unlock(lock);
}

int eshell_putstring(const char *fmt, ...)
{
    int len = 0;
    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(oneline_buff, ONELINE_BUFF_SIZE, fmt, ap);
    va_end(ap);

    if (len < 0) {
        len = 0;
    } else if (len + 1 >= ONELINE_BUFF_SIZE) {
        len = (ONELINE_BUFF_SIZE > 1) ? (ONELINE_BUFF_SIZE - 1) : 0;
    }

    if (len > 0)
        uart_send_data((unsigned char *)oneline_buff, len);

    return 0;
}

int eshell_putstring_nl(const char *fmt, ...)
{
    int len = 0;
    va_list ap;
    va_start(ap, fmt);
    len = vsnprintf(oneline_buff, ONELINE_BUFF_SIZE, fmt, ap);
    va_end(ap);

    if (len < 0) {
        len = 0;
    } else if (len + 3 >= ONELINE_BUFF_SIZE) {
        len = (ONELINE_BUFF_SIZE > 3) ? (ONELINE_BUFF_SIZE - 3) : 0;
    }
    if (len + 2 < ONELINE_BUFF_SIZE) {
        oneline_buff[len] = '\r';
        oneline_buff[len+1] = '\n';
        len += 2;
    }

    if (len > 0)
        uart_send_data((unsigned char *)oneline_buff, len);

    return 0;
}

int eshell_output_rawdata(char *data, unsigned int data_len)
{
    int ret = 0;
    if (data_len > 0)
        ret = uart_send_data((unsigned char *)data, data_len);
    return ret;
}
