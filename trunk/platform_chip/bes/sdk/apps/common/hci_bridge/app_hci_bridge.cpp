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
#include "stdbool.h"
#include "hal_intersys.h"
#include "hal_iomux.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_chipid.h"
#include "cqueue.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "string.h"
#include "app_hci_bridge.h"

#define HCI_BRIDGE_BUF_LEN                      (1024)
#define HCI_BRIDGE_WAIT_TX_DONE_MAX_TIMES       (2500)
#define HCI_BRIDGE_WAIT_TX_DONE_UNIT_US         (20)
#define HCI_CMD_PREAMBLE_SIZE                   (3)
#define HCI_EVT_PREAMBLE_SIZE                   (2)
#define HCI_SCO_PREAMBLE_SIZE                   (3)
#define HCI_ACL_PREAMBLE_SIZE                   (4)
#if BLE_AUDIO_ENABLED
#define HCI_ISO_PREAMBLE_SIZE                   (4)
#define HCI_DEBUG_TRACE_PREAMBLE_SIZE           (3)
#endif
#define HCI_BRIDGE_INTERSYS_TX_SIGNAL           (0x010000)
#define HCI_BRIDGE_INTERSYS_RX_SIGNAL           (0x010001)
#define HCI_BRIDGE_THREAD_STACKSIZE             (2048)
#define HCI_BRIDGE_THREAD_PRIORITY              (osPriorityHigh)
#define HCI_BRIDGE_MSG_CNT                      (10)

typedef enum {
    H4_RX_MSGTYPE_ST,
    H4_RX_LEN_ST,
    H4_RX_DATA_ST,
    H4_RX_IGNORE_ST
} HCI_BRIDGE_RCV_STATE_T;

typedef struct
{
    uint16_t rcv_len;                   /* Size of current incoming message */
    uint8_t rcv_msg_type;               /* Current incoming message type */
    uint8_t preload_count;              /* Count numbers of preload bytes */
    HCI_BRIDGE_RCV_STATE_T rcv_state;    /* Receive state of current rx message */
    uint8_t buff[1024];                 /* Data */
} HCI_BRIDGE_DATA_CB_T;

typedef enum {
    HCI_COMMAND_DATA_PACKET = 0x01,
    HCI_ACL_DATA_PACKET     = 0x02,
    HCI_SCO_DATA_PACKET     = 0x03,
    HCI_EVENT_PACKET        = 0x04,
#if BLE_AUDIO_ENABLED
    HCI_ISO_DATA_PACKET     = 0x05,
#endif
    HCI_DEBUG_TRACE_INFO    = 0x06,
    HCI_PACKET_TYPE_INVAILD,
} HCI_BRIDGE_DATA_TYPE_E;

static const struct HAL_UART_CFG_T hci_bridge_uart_cfg = {
    .parity = HAL_UART_PARITY_NONE,
    .stop = HAL_UART_STOP_BITS_1,
    .data = HAL_UART_DATA_BITS_8,
    .flow = HAL_UART_FLOW_CONTROL_NONE,
    .rx_level = HAL_UART_FIFO_LEVEL_1_4,
    .tx_level = HAL_UART_FIFO_LEVEL_1_2,
    .baud = 115200,
    .dma_rx = true,
    .dma_tx = true,
    .dma_rx_stop_on_err = false,
};

#if (DEBUG_PORT == 1)
enum HAL_UART_ID_T hci_bridge_uart = HAL_UART_ID_1;
#else
enum HAL_UART_ID_T hci_bridge_uart = HAL_UART_ID_0;
#endif
static enum HAL_INTERSYS_ID_T hci_bridge_intersys = HAL_INTERSYS_ID_0;
static bool hci_bridge_mode = false;

// Process data from UART
static unsigned char hci_bridge_rx_buffer_p[HCI_BRIDGE_BUF_LEN];
static unsigned char hci_bridge_rx_q_p[HCI_BRIDGE_BUF_LEN];
static CQueue hci_bridge_rx_queue;
static osThreadId hci_bridge_rx_thread_id = NULL ;
osMessageQId hci_bridge_rx_msg_id;
static void hci_bridge_rx_thread(void const *argument);
static const osThreadAttr_t hci_bridge_rx_thread_attr = {
    .name = "hci_bridge_rx_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_size = HCI_BRIDGE_THREAD_STACKSIZE,
    .priority = HCI_BRIDGE_THREAD_PRIORITY,
    .tz_module = 1U,            // indicate calls to secure mode
    .reserved = 0U,
};
static const osMessageQueueAttr_t MQAttr_hci_bridge_rx_msg = {
    .name = "hci_bridge_rx_msg",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
    .mq_mem = NULL,
    .mq_size = 0,
};

// Process data that needs to be sent through UART
static unsigned char hci_bridge_tx_data_p[HCI_BRIDGE_BUF_LEN];
static CQueue hci_bridge_tx_queue;
static osThreadId hci_bridge_tx_thread_id = NULL ;
osMessageQId hci_bridge_tx_msg_id;
static void hci_bridge_tx_thread(void const *argument);
static const osThreadAttr_t hci_bridge_tx_thread_attr = {
    .name = "hci_bridge_tx_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_size = HCI_BRIDGE_THREAD_STACKSIZE,
    .priority = HCI_BRIDGE_THREAD_PRIORITY,
    .tz_module = 1U,            // indicate calls to secure mode
    .reserved = 0U,
};
static const osMessageQueueAttr_t MQAttr_hci_bridge_tx_msg = {
    .name = "hci_bridge_tx_msg",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
    .mq_mem = NULL,
    .mq_size = 0,
};

static app_hci_bridge_data_handler hci_bridge_data_receiver_cb = NULL;

static HCI_BRIDGE_DATA_CB_T p_hci_bridge_data_cb = {0};

static const unsigned char hci_bridge_preamble_table[] =
{
    HCI_CMD_PREAMBLE_SIZE,
    HCI_ACL_PREAMBLE_SIZE,
    HCI_SCO_PREAMBLE_SIZE,
    HCI_EVT_PREAMBLE_SIZE,
#if BLE_AUDIO_ENABLED
    HCI_ISO_PREAMBLE_SIZE,
    HCI_DEBUG_TRACE_PREAMBLE_SIZE,
#endif
};

static unsigned int hci_bridge_bt_rx(const unsigned char *data, unsigned int len)
{
    int nRet = 0;
    hal_intersys_stop_recv(hci_bridge_intersys);

    uint32_t lock = int_lock();
    nRet = EnCQueue(&hci_bridge_tx_queue, (unsigned char *)data, len);
    int_unlock(lock);

    ASSERT(nRet == CQ_OK, "%s overflow, input:%d available:%d len:%d", __func__, len,
        AvailableOfCQueue(&hci_bridge_tx_queue), LengthOfCQueue(&hci_bridge_tx_queue));

    osStatus_t status = osMessageQueuePut(hci_bridge_tx_msg_id, &len, 0U, 0U);
    ASSERT(status == osOK, "bt_rx_queue msg queue overflow:%d, len:%d", nRet, len);

    return len;
}

static void hci_bridge_bt_tx(const unsigned char *data, unsigned int len)
{
    osThreadFlagsSet(hci_bridge_rx_thread_id, HCI_BRIDGE_INTERSYS_TX_SIGNAL);
}

static void hci_bridge_uart_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    volatile unsigned int uart_rx_len = 0;

    if (dma_error) {
        TRACE(1,"uart_rx dma error: xfer_size=%d", xfer_size);
    } else if (status.BE || status.FE || status.OE || status.PE) {
        TRACE(2,"uart_rx uart error: xfer_size=%d, status=0x%08x", xfer_size, status.reg);
    } else {
        uart_rx_len = xfer_size;
    }

    if (uart_rx_len > 0) {
        int nRet = 0;
        uint32_t lock = int_lock();
        nRet = EnCQueue(&hci_bridge_rx_queue, (unsigned char *)hci_bridge_rx_buffer_p, uart_rx_len);
        int_unlock(lock);

        ASSERT(nRet == CQ_OK, "%s Q overflow:%d, input:%d available:%d len:%d", __func__, nRet, uart_rx_len,
            AvailableOfCQueue(&hci_bridge_rx_queue), LengthOfCQueue(&hci_bridge_rx_queue));

        osStatus_t osStatus = osMessageQueuePut(hci_bridge_rx_msg_id, (void *)&uart_rx_len, 0U, 0U);
        ASSERT(osStatus == osOK, "%s msg queue overflow:%d-0x%d, len:%d", __func__, nRet, status.reg, uart_rx_len);
    }
}

static void hci_bridge_uart_tx(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        TRACE(1,"uart_tx dma error: xfer_size=%d", xfer_size);
    }

    osThreadFlagsSet(hci_bridge_tx_thread_id, HCI_BRIDGE_INTERSYS_RX_SIGNAL);
}

static void bridge_send_data_to_remote(const unsigned char *data, unsigned int len)
{
    int32_t flags;

    TRACE(0, "UART_TX:");
    DUMP8("%02x ", data, len);
    if ((data[0] == 0x4) && (data[1] == 0xff) && (data[3] == 0x1)) {
        return;
    }
    if (len > 0) {
        hal_uart_dma_send(hci_bridge_uart, data, len, NULL, NULL);
        flags = osThreadFlagsWait(HCI_BRIDGE_INTERSYS_TX_SIGNAL, osFlagsWaitAll, 5000);
        if (flags == osErrorTimeout){
            ASSERT(0, "%s osEventTimeout", __func__);
        }
    }
}

static void hci_bridge_tx_thread(void const *argument)
{
//    enum APP_SYSFREQ_FREQ_T sysfreq;
//    sysfreq = APP_SYSFREQ_15M;

    while(1) {
        osStatus_t status;
        uint32_t len;
        unsigned char *e1 = NULL, *e2 = NULL;
        unsigned int len1 = 0, len2 = 0;
        uint32_t lock;
        int nRet = 0;
        uint8_t buff_combine[1024];

//        app_sysfreq_req(APP_SYSFREQ_USER_HCI, APP_SYSFREQ_32K);
        status = osMessageQueueGet(hci_bridge_tx_msg_id, &len, NULL, osWaitForever);
//        app_sysfreq_req(APP_SYSFREQ_USER_HCI, sysfreq);
        if (status == osOK)
        {
            lock = int_lock();
            nRet = PeekCQueue(&hci_bridge_tx_queue, len, &e1, &len1, &e2, &len2);
            int_unlock(lock);
            ASSERT(nRet == CQ_OK, "%s, PeekCQueue error", __func__);

            if (len1 && !len2)
            {
                bridge_send_data_to_remote(e1, len1);
            }
            else if (!len1 && len2)
            {
                bridge_send_data_to_remote(e2, len2);
            }
            else if (len1 && len2)
            {
                if (len > 1024) {
                    // sanity check
                    TRACE(0, "(warning)%s, len:%d", __func__, len);
                } else {
                    memcpy(buff_combine, e1, len1);
                    memcpy(buff_combine + len1, e2, len2);
                    bridge_send_data_to_remote(buff_combine, len1 + len2);
                }
            }
            else
            {
                //do nothing
            }

            if (hci_bridge_mode) {
                hal_intersys_start_recv(hci_bridge_intersys);
            }
            lock = int_lock();
            DeCQueue(&hci_bridge_tx_queue, 0, len);
            int_unlock(lock);
        }
    }
}

uint16_t hci_bridge_unpacket_uart_data(const uint8_t *buf_in, uint32_t size)
{
    uint32_t bytes_parsed = 0;
    bool msg_received = false;
    uint8_t data = 0;
    uint8_t *p_buf_out = NULL;
    uint16_t msg_len = 0;
    HCI_BRIDGE_DATA_CB_T *p_cb = &p_hci_bridge_data_cb;

    while (bytes_parsed < size)
    {
        msg_received = false;
        data = buf_in[bytes_parsed];
        p_buf_out = &p_hci_bridge_data_cb.buff[bytes_parsed];

        switch (p_cb->rcv_state)
        {
            case H4_RX_MSGTYPE_ST:
            {
                if (data < HCI_COMMAND_DATA_PACKET || data >= HCI_PACKET_TYPE_INVAILD)
                {
                    TRACE(1, "wrong type:0x%x, Ignore", data);
                    return 0;
                }

                /* Initialize rx parameters */
                p_cb->rcv_msg_type = data;
                p_cb->rcv_len = hci_bridge_preamble_table[data-1];
                memset(p_cb->buff, 0, 1024);
                *p_buf_out = data;
                p_cb->preload_count = 0;
                p_cb->rcv_state = H4_RX_LEN_ST; /* Next, wait for length to come */

                ++bytes_parsed;
            } break;
            case H4_RX_LEN_ST:
            {
                /* Receiving preamble */
                *p_buf_out = data;
                if (p_cb->rcv_len > 0) {
                    p_cb->rcv_len--;
                }

                ++bytes_parsed;

                /* Check if received entire preamble yet */
                if (p_cb->rcv_len == 0)
                {
                    if (p_cb->rcv_msg_type == HCI_ACL_DATA_PACKET)
                    {
                        /* ACL data lengths are 16-bits */
                        msg_len = p_cb->buff[4];
                        msg_len = (msg_len << 8) + p_cb->buff[3];
                    }
#if BLE_AUDIO_ENABLED
                    else if(p_cb->rcv_msg_type == HCI_DEBUG_TRACE_INFO)
                    {
                        //TRC LLCP data lengths are 16-bits
                        msg_len = p_cb->buff[3];
                        msg_len = (msg_len << 8) + p_cb->buff[2];
                    }
                    else if(p_cb->rcv_msg_type == HCI_ISO_DATA_PACKET)
                    {
                        /* iso data lengths are 14-bits */
                        msg_len = p_cb->buff[4] & 0x3f;
                        msg_len = (msg_len << 8) + p_cb->buff[3];
                    }
#endif
                    else
                    {
                        /* other type */
                        msg_len = data;
                    }
                    // data part
                    p_cb->rcv_len = msg_len;

                    /* Message length is valid */
                    if (msg_len)
                    {
                        /* Read rest of message */
                        p_cb->rcv_state = H4_RX_DATA_ST;
                    }
                    else
                    {
                        msg_received = true;
                        /* Next, wait for next message */
                        p_cb->rcv_state = H4_RX_MSGTYPE_ST;
                    }
                }
            } break;
            case H4_RX_DATA_ST:
            {
                /* Read in the rest of the message */
                if (p_cb->rcv_len > 0)
                {
                    /* Read in the rest of the message */
                    uint16_t remain_len = size - bytes_parsed;
                    //TRACE(1, "%s, len:%d-%d", __func__, size, bytes_parsed);
                    if(remain_len >= p_cb->rcv_len) {
                        remain_len = p_cb->rcv_len;
                    } else {
                        //TRACE(1, "%s, len:%d-%d", __func__, size, bytes_parsed);
                        p_cb->rcv_state = H4_RX_IGNORE_ST;
                        break;
                    }

                    memcpy(p_buf_out, buf_in + bytes_parsed, remain_len);
                    p_cb->rcv_len -= remain_len;
                    bytes_parsed += remain_len;
                }

                /* Check if we read in entire message yet */
                if (p_cb->rcv_len == 0)
                {
                    msg_received = true;
                    /* Next, wait for next message */
                    p_cb->rcv_state = H4_RX_MSGTYPE_ST;
                }
            } break;
            case H4_RX_IGNORE_ST:
            {
                /* Ignore reset of packet */
                p_cb->rcv_len = 0;
                bytes_parsed = 0;
                /* Next, wait for next message */
                p_cb->rcv_state = H4_RX_MSGTYPE_ST;
                msg_received = true;
            } break;
        }

        /* If we received entire message, then send it to the task */
        if (msg_received)
        {
            break;
        }
    }

    return bytes_parsed;
}

// Process the data from remote via UART and send to local
static void hci_bridge_rx_thread(void const *argument)
{
//    enum APP_SYSFREQ_FREQ_T sysfreq;
//    sysfreq = APP_SYSFREQ_15M;
    static uint16_t remain_len = 0;

    while(1) {
        uint32_t len;
        unsigned char *e1 = NULL, *e2 = NULL;
        unsigned int len1 = 0, len2 = 0;
        uint32_t lock;
        int nRet = 0;
        uint16_t bytes_consumed = 0;
        uint8_t buff_combine[1024];
        uint32_t wait_rounds = 0;
        osStatus_t status;
        int32_t flags;

//        app_sysfreq_req(APP_SYSFREQ_USER_HCI, APP_SYSFREQ_32K);
        status = osMessageQueueGet(hci_bridge_rx_msg_id, &len, NULL, osWaitForever);
//        app_sysfreq_req(APP_SYSFREQ_USER_HCI, sysfreq);

        if (status == osOK)
        {
parser_again:
            lock = int_lock();
            nRet = PeekCQueue(&hci_bridge_rx_queue, len + remain_len, &e1, &len1, &e2, &len2);
            int_unlock(lock);

            ASSERT(nRet == CQ_OK, "%s, PeekCQueue error:%d", __func__, nRet);
            union HAL_UART_IRQ_T mask;
            hal_uart_dma_recv_mask(hci_bridge_uart, hci_bridge_rx_buffer_p, HCI_BRIDGE_BUF_LEN, NULL, NULL, &mask);

            if (len1 && !len2)
            {
                bytes_consumed = hci_bridge_unpacket_uart_data(e1, len1);
            }
            else if (!len1 && len2)
            {
                bytes_consumed = hci_bridge_unpacket_uart_data(e2, len2);
            }
            else if (len1 && len2)
            {
                if (len > 1024) {
                    // sanity check
                    TRACE(0, "(warning)%s, len:%d", __func__, len);
                } else {
                    memcpy(buff_combine, e1, len1);
                    memcpy(buff_combine + len1, e2, len2);
                    bytes_consumed = hci_bridge_unpacket_uart_data(buff_combine, len1 + len2);
                }
            }
            else
            {
                //do nothing
            }

            if (bytes_consumed > 0) {
                TRACE(0, "UART_RX:");
                DUMP8("%02x ", p_hci_bridge_data_cb.buff, bytes_consumed);

                if (hci_bridge_mode) {
                    hal_intersys_send(hci_bridge_intersys, HAL_INTERSYS_MSG_HCI, p_hci_bridge_data_cb.buff, bytes_consumed);
                } else {
                    if (hci_bridge_data_receiver_cb) {
                        hci_bridge_data_receiver_cb(p_hci_bridge_data_cb.buff, bytes_consumed);
                    }
                }
            }

            if (len >= bytes_consumed) {
                remain_len += len - bytes_consumed;
            } else if (remain_len >= bytes_consumed - len) {
                remain_len -= bytes_consumed - len;
            } else {
                TRACE(0, "(warning)%s, len wrong:%d-%d-%d", __func__, len, remain_len, bytes_consumed);
            }

            //TRACE(0, "%s, len:%d-%d-%d", __func__, len, remain_len, bytes_consumed);

            lock = int_lock();
            DeCQueue(&hci_bridge_rx_queue, 0, bytes_consumed);
            int_unlock(lock);

            if (hci_bridge_mode) {
                flags = osThreadFlagsWait(HCI_BRIDGE_INTERSYS_TX_SIGNAL, osFlagsWaitAll, 5000);
                if (flags == osErrorTimeout){
                    ASSERT(0, "%s osEventTimeout", __func__);
                }
            } else {
                while (wait_rounds++ < 200)
                {
                    hal_sys_timer_delay_us(HCI_BRIDGE_WAIT_TX_DONE_UNIT_US);
                }
            }

            if ((remain_len > 0) && (bytes_consumed > 0)) {
                len = 0;
                goto parser_again;
            }
        }
    }
}

int app_hci_bridge_send_data(const uint8_t *data, uint16_t len)
{
    int nRet = 0;
    uint32_t lock = int_lock();
    nRet = EnCQueue(&hci_bridge_tx_queue, (unsigned char *)data, len);
    int_unlock(lock);

    ASSERT(nRet == CQ_OK, "%s Q overflow:%d, input:%d available:%d len:%d", __func__, nRet, len,
        AvailableOfCQueue(&hci_bridge_tx_queue), LengthOfCQueue(&hci_bridge_tx_queue));

    nRet = osMessagePut(hci_bridge_tx_msg_id, len, 0);
    ASSERT(nRet == osOK, "%s msg queue overflow:%d, len:%d", __func__, nRet, len);
    return nRet;
}

void app_hci_bridge_register_data_handler(app_hci_bridge_data_handler func)
{
    hci_bridge_data_receiver_cb = func;
}

void app_hci_bridge_close(void)
{
    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    hal_uart_irq_set_mask(hci_bridge_uart, mask);
    hal_uart_stop_dma_recv(hci_bridge_uart);
}

void app_hci_bridge_init(bool mode)
{
    int ret;
    union HAL_UART_IRQ_T mask;

    // Communication interface initialization
    if (hci_bridge_uart == HAL_UART_ID_0) {
        hal_iomux_set_uart0();
        hal_iomux_set_uart1();
        hal_trace_switch(HAL_TRACE_TRANSPORT_UART1);
    } else {
        hal_iomux_set_uart1();
        hal_trace_switch(HAL_TRACE_TRANSPORT_UART0);
    }

    ret = hal_uart_open(hci_bridge_uart, &hci_bridge_uart_cfg);
    TRACE(0, "%s, mode:%d, ret:%d", __func__, mode, ret);
    if (ret) {
        return;
    }

    hci_bridge_mode = mode;

    // Remote -> UART rx buffer -> Local BTH -> Local BTC
    InitCQueue(&hci_bridge_rx_queue, HCI_BRIDGE_BUF_LEN, hci_bridge_rx_q_p);

    if (!hci_bridge_rx_msg_id)
    {
        hci_bridge_rx_msg_id =  osMessageQueueNew(HCI_BRIDGE_MSG_CNT, sizeof(uint32_t), &MQAttr_hci_bridge_rx_msg);
    }
    if (!hci_bridge_rx_thread_id)
    {
        hci_bridge_rx_thread_id = osThreadNew((osThreadFunc_t)hci_bridge_rx_thread, NULL, &hci_bridge_rx_thread_attr);
    }

    InitCQueue(&hci_bridge_tx_queue, HCI_BRIDGE_BUF_LEN, hci_bridge_tx_data_p);

    if (!hci_bridge_tx_msg_id)
    {
        hci_bridge_tx_msg_id =  osMessageQueueNew(HCI_BRIDGE_MSG_CNT, sizeof(uint32_t), &MQAttr_hci_bridge_tx_msg);
    }

    if (!hci_bridge_tx_thread_id)
    {
        hci_bridge_tx_thread_id = osThreadNew((osThreadFunc_t)hci_bridge_tx_thread, NULL, &hci_bridge_tx_thread_attr);
    }

    hal_uart_irq_set_dma_handler(hci_bridge_uart, hci_bridge_uart_rx, hci_bridge_uart_tx);
    mask.reg = 0;
    mask.BE = 1;
    mask.FE = 1;
    mask.OE = 1;
    mask.PE = 1;
    mask.RT = 1;
    hal_uart_dma_recv_mask(hci_bridge_uart, hci_bridge_rx_buffer_p, HCI_BRIDGE_BUF_LEN, NULL, NULL, &mask);

    if (hci_bridge_mode) {
        ret = hal_intersys_open(hci_bridge_intersys, HAL_INTERSYS_MSG_HCI, hci_bridge_bt_rx, hci_bridge_bt_tx, false);
        if (ret) {
            TRACE(0, "%s, failed open intersys,ret:%d", __func__, ret);
            return;
        }
        hal_intersys_start_recv(hci_bridge_intersys);
    }

}


