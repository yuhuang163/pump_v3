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
#ifdef ESHELL_USB_CDC
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hwtimer_list.h"
#include "safe_queue.h"
#include "string.h"
#include "usb_cdc.h"
#include "pmu.h"

#define UART_CMD_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_11

#define ALIGNED4                        ALIGNED(4)

enum UART_CMD_T {
    UART_CMD_TERM_CONN                  = 0,
    UART_CMD_TERM_DISCONN,
    UART_CMD_SET_LINE_CODING,
    UART_CMD_USB_RX_BREAK_START,
    UART_CMD_USB_RX_BREAK_STOP,
    UART_CMD_UART_RX_BREAK,
    UART_CMD_USB_TX_DONE,
    UART_CMD_USB_RX_DONE,
    UART_CMD_UART_TX_DONE,
    UART_CMD_UART_RX_DONE,

    UART_CMD_QTY,
};

static const struct USB_SERIAL_CFG_T cdc_cfg;

struct USB_SERIAL_LINE_CODING_T uart_line_coding = {
    .rate = 921600,
    .stop_bits = USB_SERIAL_STOP_BITS_1,
    .parity = USB_SERIAL_PARITY_NONE,
    .data_bits = 8,
};

typedef uint32_t (*cdc_rx_cb)(uint8_t *buf, uint32_t len);
cdc_rx_cb eshell_cdc_rx_cb = NULL;

#define ESHELL_CDC_BUF_SIZE    (512)
ALIGNED4
static uint8_t usb_rx_buf[ESHELL_CDC_BUF_SIZE];
ALIGNED4
static uint8_t usb_tx_buf[ESHELL_CDC_BUF_SIZE * 4];

static uint32_t usb_rx_size;
static bool usb_sending;

static bool term_connected;

static uint16_t usb_break_ms;
static HWTIMER_ID usb_rx_break_timer;

#define CMD_LIST_SIZE                   30
static uint32_t cmd_list[CMD_LIST_SIZE];
static uint32_t cmd_watermark;
static struct SAFE_QUEUE_T cmd_queue;

#define EXTRACT_CMD(d)                  ((d) & 0xFF)
#define EXTRACT_SEQ(d)                  (((d) >> 8) & 0xFF)
#define EXTRACT_ARG(d)                  (((d) >> 16) & 0xFF)
#define MAKE_QUEUE_DATA(c, s, a)        (((c) & 0xFF) | ((s) & 0xFF) << 8 | ((a) & 0xFF) << 16)
STATIC_ASSERT(UART_CMD_QTY <= 0xFF, "uart cmd num exceeds size in queue");

#define SIGNAL_OF_CDC_DATA_RX_READY        (1<<10)
static osThreadId_t eshell_cdc_id = NULL;
static osSemaphoreId_t cdc_tx_sema;

static void usb_uart_enqueue_cmd(uint32_t data)
{
    int ret;
    uint32_t mark;

    ret = safe_queue_put(&cmd_queue, data);
    ASSERT(ret == 0 || safe_queue_dump(&cmd_queue), "%s: cmd queue overflow", __FUNCTION__);

    hal_cpu_wake_lock(UART_CMD_WAKE_LOCK);

    mark = safe_queue_watermark(&cmd_queue);
    if (mark > cmd_watermark) {
        cmd_watermark = mark;
        TR_INFO(0, "%s: new watermark %u", __FUNCTION__, mark);
    }

    /* set signal cmd available */
    osThreadFlagsSet(eshell_cdc_id, SIGNAL_OF_CDC_DATA_RX_READY);
}

static void usb_uart_enqueue_unique_cmd(enum UART_CMD_T cmd, uint8_t seq, uint8_t arg)
{
    uint32_t last;
    uint32_t cur;
    uint32_t lock;
    bool enqueue = true;

    cur = MAKE_QUEUE_DATA(cmd, seq, arg);

    lock = int_lock();
    if (safe_queue_peek(&cmd_queue, -1, &last) == 0) {
        if (last == cur) {
            enqueue = false;
        }
    }
    int_unlock(lock);

    if (enqueue) {
        usb_uart_enqueue_cmd(cur);
    } else {
        TR_INFO(0, "%s: Skip duplicate cmd %d", __FUNCTION__, cmd);
    }
}

static void usb_break_handler(uint16_t ms)
{
    if (ms) {
        usb_break_ms = ms;
        usb_uart_enqueue_unique_cmd(UART_CMD_USB_RX_BREAK_START, 0, 0);
    } else {
        usb_uart_enqueue_unique_cmd(UART_CMD_USB_RX_BREAK_STOP, 0, 0);
    }
    TR_INFO(0, "USB rx break: %u ms", ms);
}

static void usb_tx_handler(const void *data, uint32_t size, int error)
{
    //TR_INFO(0, "%s: size=%u error=%d", __func__, size, error);

    if (error) {
        TR_ERROR(0, "*** ERROR: %s: size=%u error=%d", __func__, size, error);
    }

    /* send in sync way */
#if 0
    usb_uart_enqueue_unique_cmd(UART_CMD_USB_TX_DONE, 0, 0);
#else
    usb_sending = false;
    osSemaphoreRelease(cdc_tx_sema);
#endif
}

static void usb_rx_handler(const void *data, uint32_t size, int error)
{
    //TR_INFO(0, "%s: size=%u error=%d", __func__, size, error);

    if (error) {
        TR_ERROR(0, "*** ERROR: %s: size=%u error=%d", __func__, size, error);
    }
    usb_rx_size = size;
    usb_uart_enqueue_unique_cmd(UART_CMD_USB_RX_DONE, 0, 0);
}

static void usb_start_rx(void)
{
    int result;
    if (!term_connected) {
        return;
    }
    usb_rx_size = 0;
    result = usb_serial_direct_recv(usb_rx_buf, sizeof(usb_rx_buf), 0, NULL, usb_rx_handler);
    ASSERT(result == 0, "Failed to start usb serial rx: %d", result);
}

static void usb_uart_state_handler(enum USB_SERIAL_STATE_EVENT_T event)
{
    TR_INFO(0, "%s: event=%d", __func__, event);

    if (event == USB_SERIAL_STATE_TERMINAL_CONNECTED) {
        usb_uart_enqueue_unique_cmd(UART_CMD_TERM_CONN, 0, 0);
    } else if (event == USB_SERIAL_STATE_TERMINAL_DISCONNECTED ||
            event == USB_SERIAL_STATE_RESET ||
            event == USB_SERIAL_STATE_DISCONNECT) {
        usb_uart_enqueue_unique_cmd(UART_CMD_TERM_DISCONN, 0, 0);
    }
}

static int usb_uart_get_line_coding_callback(struct USB_SERIAL_LINE_CODING_T *coding)
{
    *coding = uart_line_coding;
    return 0;
}

static int usb_uart_set_line_coding_callback(const struct USB_SERIAL_LINE_CODING_T *coding)
{
    if (coding->stop_bits != USB_SERIAL_STOP_BITS_1 && coding->stop_bits != USB_SERIAL_STOP_BITS_2) {
        return 1;
    }

    if (coding->parity != USB_SERIAL_PARITY_NONE && coding->parity != USB_SERIAL_PARITY_ODD &&
            coding->parity != USB_SERIAL_PARITY_EVEN) {
        return 2;
    }

    if (coding->data_bits < 5 || coding->data_bits > 8) {
        return 3;
    }

    uart_line_coding = *coding;

    TR_INFO(0, "%s: rate=%u stop_bits=%u parity=%u data_bits=%u",
        __func__, coding->rate, (coding->stop_bits == USB_SERIAL_STOP_BITS_1) ? 1 : 2, coding->parity, coding->data_bits);

    return 0;
}

static void line_coding_to_uart_cfg(uint32_t *baud, enum HAL_UART_STOP_BITS_T *stop,
                                    enum HAL_UART_PARITY_T *parity, enum HAL_UART_DATA_BITS_T *data)
{
    uint32_t lock;

    lock = int_lock();

    *baud = uart_line_coding.rate;

    if (uart_line_coding.stop_bits == USB_SERIAL_STOP_BITS_1) {
        *stop = HAL_UART_STOP_BITS_1;
    } else if (uart_line_coding.stop_bits == USB_SERIAL_STOP_BITS_2) {
        *stop = HAL_UART_STOP_BITS_2;
    } else {
        *stop = HAL_UART_STOP_BITS_1;
        ASSERT(false, "Bad stop_bits=%u", uart_line_coding.stop_bits);
    }

    if (uart_line_coding.parity == USB_SERIAL_PARITY_NONE) {
        *parity = HAL_UART_PARITY_NONE;
    } else if (uart_line_coding.parity == USB_SERIAL_PARITY_ODD) {
        *parity = HAL_UART_PARITY_ODD;
    } else if (uart_line_coding.parity == USB_SERIAL_PARITY_EVEN) {
        *parity = HAL_UART_PARITY_EVEN;
    } else {
        ASSERT(false, "Bad parity=%u", uart_line_coding.parity);
    }

    if (uart_line_coding.data_bits < 5 || uart_line_coding.data_bits > 8) {
        ASSERT(false, "Bad data_bits=%u", uart_line_coding.data_bits);
    }
    *data = HAL_UART_DATA_BITS_5 + (uart_line_coding.data_bits - 5);

    int_unlock(lock);
}

static void usb_rx_break_timer_handler(void *param)
{
    usb_uart_enqueue_unique_cmd(UART_CMD_USB_RX_BREAK_STOP, 0, 0);
}

static void usb_uart_start(void)
{
    int ret;

    TR_INFO(0, "usb_uart starts");

    safe_queue_init(&cmd_queue, cmd_list, CMD_LIST_SIZE);

    usb_rx_break_timer = hwtimer_alloc(usb_rx_break_timer_handler, NULL);
    ASSERT(usb_rx_break_timer, "Failed to alloc usb rx break timer");

    ret = usb_serial_open(&cdc_cfg);
    ASSERT(ret == 0, "Failed to open usb serial: %d", ret);

    usb_serial_flush_recv_buffer();
}

static void usb_uart_cmd_term_connected(void)
{
    if (term_connected) {

    } else {
        term_connected = true;
        usb_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);
        usb_start_rx();
    }
    hwtimer_stop(usb_rx_break_timer);
}

static void usb_uart_cmd_term_disconnected(void)
{
    term_connected = false;
    usb_serial_cancel_recv();
    usb_serial_cancel_send();
    usb_sending = false;

    hwtimer_stop(usb_rx_break_timer);

    usb_rx_size = 0;
}

static void usb_uart_cmd_set_line_coding(void)
{
    uint32_t baud;
    enum HAL_UART_STOP_BITS_T stop;
    enum HAL_UART_PARITY_T parity;
    enum HAL_UART_DATA_BITS_T data;

    line_coding_to_uart_cfg(&baud, &stop, &parity, &data);

    TR_INFO(0, "%s: baud=%u stop_bits=%u parity=%u data_bits=%u",
        __func__, baud, stop + 1, parity, (data - HAL_UART_DATA_BITS_5 + 5));
}

static void usb_uart_cmd_usb_rx_break_start(void)
{
    enum E_HWTIMER_T ret;

    if (!term_connected) {
        return;
    }

    if (usb_break_ms != 0xFFFF) {
        hwtimer_stop(usb_rx_break_timer);
        ret = hwtimer_start(usb_rx_break_timer, MS_TO_TICKS(usb_break_ms));
        ASSERT(ret == E_HWTIMER_OK, "Failed to start break timer: %d", ret);
    }
}

static void usb_uart_cmd_usb_rx_break_stop(void)
{
    TR_INFO(0, "%s", __func__);
}

static void usb_uart_cmd_uart_rx_break(void)
{
    usb_serial_send_break();
    TR_INFO(0, "%s: Send break to usb", __func__);
}

static void usb_uart_cmd_usb_tx_done(void)
{

}

static void usb_uart_cmd_usb_rx_done(void)
{
    if (eshell_cdc_rx_cb)
        eshell_cdc_rx_cb(usb_rx_buf, usb_rx_size);
    usb_start_rx();
}

static void usb_uart_cmd_handler(void)
{
    uint32_t data;
    enum UART_CMD_T cmd;
    uint8_t POSSIBLY_UNUSED seq;
    uint8_t POSSIBLY_UNUSED arg;

    hal_cpu_wake_unlock(UART_CMD_WAKE_LOCK);

    while (safe_queue_get(&cmd_queue, &data) == 0) {
        cmd = EXTRACT_CMD(data);
        seq = EXTRACT_SEQ(data);
        arg = EXTRACT_ARG(data);

        if (cmd < UART_CMD_USB_TX_DONE || cmd > UART_CMD_UART_RX_DONE) {
            TR_INFO(0, "%s: cmd=%d seq=%d arg=%d", __FUNCTION__, cmd, seq, arg);
        }

        switch (cmd) {
        case UART_CMD_TERM_CONN:
            usb_uart_cmd_term_connected();
            break;

        case UART_CMD_TERM_DISCONN:
            usb_uart_cmd_term_disconnected();
            break;

        case UART_CMD_SET_LINE_CODING:
            usb_uart_cmd_set_line_coding();
            break;

        case UART_CMD_USB_RX_BREAK_START:
            usb_uart_cmd_usb_rx_break_start();
            break;

        case UART_CMD_USB_RX_BREAK_STOP:
            usb_uart_cmd_usb_rx_break_stop();
            break;

        case UART_CMD_UART_RX_BREAK:
            usb_uart_cmd_uart_rx_break();
            break;

        case UART_CMD_USB_TX_DONE:
            usb_uart_cmd_usb_tx_done();
            break;

        case UART_CMD_USB_RX_DONE:
            usb_uart_cmd_usb_rx_done();
            break;

        default:
            ASSERT(false, "%s: Invalid cmd %d", __FUNCTION__, cmd);
            break;
        }
    }
}

static void eshell_cdc(const void *arg)
{
    while (1) {
        /* wait signal */
        uint32_t flags = osThreadFlagsWait(CMSIS_V1_SIGNAL_ANY, osFlagsWaitAny, osWaitForever);
        if((flags > 0U) && (flags < 0x80000000U)) {
            if ((int32_t)flags & SIGNAL_OF_CDC_DATA_RX_READY) {
                usb_uart_cmd_handler();
            }
        }
    }
}
static uint64_t os_thread_def_stack_eshell_cdc [ROUND_UP(1024, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_cdc = {
    .name = "eshell_cdc",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_eshell_cdc,
    .stack_size =ROUND_UP(1024, 8),
    .priority = osPriorityNormal,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

int eshell_cdc_send(uint8_t *buf, uint32_t size)
{
    int ret;

    if (size > sizeof(usb_tx_buf)) {
        TR_ERROR(0, "*** ERROR: cdc buf size %d exceed", size);
        return -1;
    }

    ret = osSemaphoreAcquire(cdc_tx_sema, osWaitForever);
    if (ret != osOK && (osSemaphoreAcquire(cdc_tx_sema, 0) != osOK)) {
        /* can not block irs */
        return -1;
    }
    usb_sending = true;
    memcpy(usb_tx_buf, buf, size);
    ret = usb_serial_direct_send(usb_tx_buf, size, usb_tx_handler);
    if (ret) {
        //TR_ERROR(0, "*** ERROR: Failed to start usb direct send: %d", ret);
        usb_sending = false;
        osSemaphoreRelease(cdc_tx_sema);
        return -1;
    }
    return 0;
}

int eshell_cdc_init(cdc_rx_cb cb)
{
    hwtimer_init();

    usb_uart_start();

    eshell_cdc_rx_cb = cb;
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_2, NULL, (HAL_TRACE_APP_OUTPUT_T)eshell_cdc_send);

    /* create task to recv usb cdc cmd */
    eshell_cdc_id = osThreadNew((osThreadFunc_t)eshell_cdc, NULL, &ThreadAttr_cdc);
    cdc_tx_sema = osSemaphoreNew(1, 1, NULL);

    return 0;
}

void eshell_cdc_close(void)
{
    usb_serial_close();
}

static const struct USB_SERIAL_CFG_T cdc_cfg = {
    .state_callback = usb_uart_state_handler,
    .break_callback = usb_break_handler,
    .get_line_coding_callback = usb_uart_get_line_coding_callback,
    .set_line_coding_callback = usb_uart_set_line_coding_callback,
    .mode = USB_SERIAL_API_NONBLOCKING,
};

#endif //ESHELL_USB_CDC
