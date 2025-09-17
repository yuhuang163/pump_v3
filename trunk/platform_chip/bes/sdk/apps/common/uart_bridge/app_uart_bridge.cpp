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
#ifdef APP_UART_BRIDGE_MODULE
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "app_uart_bridge.h"
#include "cqueue.h"
#include "heap_api.h"
#include "app_uart_dma_thread.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_wdt.h"

#ifdef TB_LED_UI_ENABLE
#include "apps.h"
#endif

extern uint32_t __uart_bridge_task_cmd_table_start[];
extern uint32_t __uart_bridge_task_cmd_table_end[];

extern uint32_t __uart_bridge_instant_cmd_table_start[];
extern uint32_t __uart_bridge_instant_cmd_table_end[];

#define UART_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)   \
    ((app_uart_bridge_task_cmd_instance_t *)                    \
    ((uint32_t)__uart_bridge_task_cmd_table_start +             \
    (entryIndex)*sizeof(app_uart_bridge_task_cmd_instance_t)))

#define UART_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(entryIndex)    \
    ((app_uart_bridge_instant_cmd_instance_t *)                     \
    ((uint32_t)__uart_bridge_instant_cmd_table_start +              \
    (entryIndex)*sizeof(app_uart_bridge_instant_cmd_instance_t)))

typedef struct
{
    uint16_t  cmdcode;
    uint16_t  cmdseq;
    uint16_t  packetlen;
    uint8_t   content[APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) app_uart_bridge_data_packet_t;

typedef struct
{
    uint16_t  cmdcode;
    uint16_t  cmdseq;
    uint8_t   content[APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) app_uart_bridge_data_packet_handle_t;

static bool app_uart_bridge_inited = false;

static CQueue app_uart_bridge_receive_queue;
static uint8_t app_uart_bridge_rx_buff[APP_UART_BRIDGE_RX_BUFF_SIZE];
static uint16_t g_uart_cmdseq = 0;

osSemaphoreDef(app_uart_bridge_wait_tx_done);
osSemaphoreId app_uart_bridge_wait_tx_done_id = NULL;

osSemaphoreDef(app_uart_bridge_wait_cmd_rsp);
osSemaphoreId app_uart_bridge_wait_cmd_rsp_id = NULL;

osSemaphoreDef(app_uart_bridge_wait_peer_device_ready);
osSemaphoreId app_uart_bridge_wait_peer_device_ready_id = NULL;

static osThreadId app_uart_bridge_rx_thread_id = NULL;
static void app_uart_bridge_rx_thread(const void *arg);
osThreadDef(app_uart_bridge_rx_thread, osPriorityHigh, 1,
    (APP_UART_BRIDGE_RX_THREAD_STACK_SIZE), "uart_bridge_rx_thread");
#define APP_UART_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED   (1 << 0)
#define APP_UART_BRIDGE_RX_THREAD_SIGNAL_IRQ_RECEIVED    (1 << 1)

static osThreadId app_uart_bridge_tx_thread_id = NULL;
static void app_uart_bridge_tx_thread(const void *arg);
osThreadDef(app_uart_bridge_tx_thread, osPriorityHigh, 1,
    (APP_UART_BRIDGE_TX_THREAD_STACK_SIZE), "uart_bridge_tx_thread");

static app_uart_bridge_task_cmd_instance_t* app_uart_bridge_get_task_cmd_entry(uint16_t cmdcode);
static int32_t app_uart_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length);
static uint16_t app_uart_bridge_queue_get_next_entry_length(CQueue* ptrQueue);
static void app_uart_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len);
static int32_t app_uart_bridge_get_queue_length(CQueue *ptrQueue);
static app_uart_bridge_instant_cmd_instance_t* app_uart_bridge_get_instant_cmd_entry(uint16_t cmdcode);

#ifdef UART_TOGGLE_MODE
static bool uart_status_flag = false;
static APP_UART_INFO_T app_uart_info;

static void app_uart_bridge_keep_rx_timer_handler(void const *param);
static void app_uart_bridge_keep_rx_timer_start(void);
static void app_wake_up_irq_enable(void);
static void app_wake_up_irq_disable(void);
static void app_uart_save_send_info(uint8_t* buf, uint32_t len);
static SEND_DATA_INFO_T app_uart_get_send_info(void);

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_uart_irq_pin_cfg[] = {
    {MASTER_WAKE_UP_SLAVE_IRQ_TX_IO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    {MASTER_WAKE_UP_SLAVE_IRQ_RX_IO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};

osTimerDef(UART_KEEP_RX, app_uart_bridge_keep_rx_timer_handler);
static osTimerId uart_keep_rx_timer_id = NULL;
static uint16_t uart_keep_rx_timeout = 1000;

static void app_uart_bridge_keep_rx_timer_handler(void const *param)
{
    TRACE(1, "%s keep uart1 rx timeout", __func__);
    app_uart_stop_uart_rx();
}

static void app_uart_bridge_keep_rx_timer_start(void)
{
    osTimerStart(uart_keep_rx_timer_id, uart_keep_rx_timeout);
}

void app_uart_toggle_irq_gpio(enum UART_IRQ_PIN_NUM_T irq_pin)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)app_uart_irq_pin_cfg[irq_pin].pin);
    hal_sys_timer_delay_us(200);
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)app_uart_irq_pin_cfg[irq_pin].pin);
}

static void app_uart_save_send_info(uint8_t* buf, uint32_t len)
{
    app_uart_info.data_info.buf = buf;
    app_uart_info.data_info.len = len;
}

static SEND_DATA_INFO_T app_uart_get_send_info(void)
{
    return app_uart_info.data_info;
}

static void app_uart_bridge_send_data(uint8_t* buf, uint32_t len)
{
    hal_wdt_ping(HAL_WDT_ID_0);
    app_uart_info.uart_status = UART_STATUS_SENDING;

    // Toggle GPIO to wake up peer device
    app_uart_toggle_irq_gpio((enum UART_IRQ_PIN_NUM_T)IRQ_PIN_TRANS_OUT_REC_IN);
    app_uart_save_send_info(buf, len);
}

static void app_wake_up_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    //// Process in thread
    TRACE(1, "%s UART1 wake up staus: %d", __func__, app_uart_info.uart_status);
    osSignalSet(app_uart_bridge_rx_thread_id, APP_UART_BRIDGE_RX_THREAD_SIGNAL_IRQ_RECEIVED);
}

static void app_wake_up_irq_enable(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    enum HAL_IOMUX_PIN_T pin = app_uart_irq_pin_cfg[IRQ_PIN_TRANS_IN_REC_OUT].pin;
    ASSERT(pin != HAL_IOMUX_PIN_NUM, "[%s]: Invalid wake up mosi pin %d", __func__, pin);

    // enable pin for transmitter wake up receiver
    gpiocfg.irq_enable   = true;
    gpiocfg.irq_debounce = true;
    gpiocfg.irq_type     = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING; //edge trigger: 0->1
    gpiocfg.irq_handler  = app_wake_up_irq_handler;

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)pin, &gpiocfg);
}

static void app_wake_up_irq_disable(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    enum HAL_IOMUX_PIN_T pin = app_uart_irq_pin_cfg[IRQ_PIN_TRANS_IN_REC_OUT].pin;
    ASSERT(pin != HAL_IOMUX_PIN_NUM, "[%s]: Invalid wake up mosi pin %d", __func__, pin);

    // disable pin for master wake up slave
    gpiocfg.irq_enable   = false;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_type     = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING; //edge trigger: 0->1
    gpiocfg.irq_handler  = NULL;

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)pin, &gpiocfg);
}
#endif

static void app_uart_bridge_transmit_data(
    app_uart_bridge_task_cmd_instance_t* pCmdInstance,
    uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_uart_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING, not inited", __func__);
        return;
    }

    app_uart_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_uart_cmdseq++;
    dataPacket.packetlen = length;

    ASSERT(length <= APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE,
        "core bridge tx size %d > max %d", length,
        APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);
#ifdef UART_TOGGLE_MODE
    if(!uart_status_flag)
    {
        app_uart_bridge_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);
    }
    else
#endif
    {
        app_uart_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);
    }

    if (pCmdInstance->app_uart_bridge_transmisson_done_handler)
    {
        pCmdInstance->app_uart_bridge_transmisson_done_handler(
            cmdcode, p_buff, length);
    }
}

void app_uart_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    if (!app_uart_bridge_inited)
    {
        return;
    }

    app_uart_bridge_data_packet_t dataPacket;
    dataPacket.cmdcode = cmdcode;
    dataPacket.cmdseq = g_uart_cmdseq++;
    dataPacket.packetlen = length;

    ASSERT(length <= APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: uart bridge tx size %d > max %d", __func__, length,
        APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE);

    memcpy(dataPacket.content, p_buff, length);
#ifdef UART_TOGGLE_MODE
    if(!uart_status_flag)
    {
        app_uart_bridge_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);
    }
    else
#endif
    {
        app_uart_send_data((uint8_t *)&dataPacket, length + APP_UART_HEADER_DATA_SIZE);
    }
}

void app_uart_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_uart_bridge_task_cmd_instance_t* pInstance =
            app_uart_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(0 == pInstance->wait_rsp_timeout_ms, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_uart_bridge_transmit_data(pInstance, cmdcode, p_buff, length);
}

void app_uart_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    app_uart_bridge_task_cmd_instance_t* pInstance =
            app_uart_bridge_get_task_cmd_entry(cmdcode);

    ASSERT(pInstance->wait_rsp_timeout_ms > 0, \
           "%s: cmdCode:0x%x wait rsp timeout:%d should be > 0", __func__, \
           pInstance->wait_rsp_timeout_ms, \
           cmdcode);

    app_uart_bridge_transmit_data(pInstance, cmdcode, p_buff, length);

    uint32_t stime = 0, etime = 0;

    stime = hal_sys_timer_get();

    int32_t returnValue = 0;
    returnValue = osSemaphoreWait(app_uart_bridge_wait_cmd_rsp_id,
        pInstance->wait_rsp_timeout_ms);

    etime = hal_sys_timer_get();

    if ((0 == returnValue) || (-1 == returnValue))
    {
        TRACE(2,"%s err = %d",__func__,returnValue);
        TRACE(2,"uart bridge:wait rsp to cmdcode=%s timeout(%d)", \
              pInstance->log_cmd_code_str, \
              (etime-stime)/16000);

        if (pInstance->app_uart_bridge_wait_rsp_timeout_handle)
        {
            pInstance->app_uart_bridge_wait_rsp_timeout_handle(
                p_buff,length);
        }
    }
}

#define BES_UART_BT_DATA_PACKET_MAX_SIZE    500
static CQueue app_uart_data_receive_queue;
static uint8_t accumulatedUartBuf[BES_UART_BT_DATA_PACKET_MAX_SIZE * 5];
#define UART_PACKET_LENGTH_HEADER_LEN 6
static void app_uart_bridge_data_received(uint8_t* data, uint32_t len)
{
    if (!app_uart_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING: not inited", __func__);
        return;
    }

    static uint8_t extractedOtaRequestBuf[BES_UART_BT_DATA_PACKET_MAX_SIZE];
    static bool isOnceInitOtaCmdQueue = false;

    if (!isOnceInitOtaCmdQueue)
    {
        InitCQueue(&app_uart_data_receive_queue, sizeof(accumulatedUartBuf), accumulatedUartBuf);
        isOnceInitOtaCmdQueue = true;
    }

    EnCQueue(&app_uart_data_receive_queue, data, len);
    TRACE(1, "received len:%d", len);
    uint8_t rxcmdHeader[UART_PACKET_LENGTH_HEADER_LEN];
    uint16_t rxqueueLeftDataLen = LengthOfCQueue(&app_uart_data_receive_queue);
    TRACE(1, "app_uart_data_receive_queue len:%d", rxqueueLeftDataLen);
    while (rxqueueLeftDataLen >= UART_PACKET_LENGTH_HEADER_LEN)
    {
        PeekCQueueToBuf(&app_uart_data_receive_queue, rxcmdHeader, UART_PACKET_LENGTH_HEADER_LEN);
        app_uart_bridge_data_packet_t *packet = (app_uart_bridge_data_packet_t *)rxcmdHeader;
        uint16_t payloadLen = packet->packetlen;
        if ((rxqueueLeftDataLen - UART_PACKET_LENGTH_HEADER_LEN) >= payloadLen)
        {
            DeCQueue(&app_uart_data_receive_queue, extractedOtaRequestBuf, payloadLen + UART_PACKET_LENGTH_HEADER_LEN);
            rxqueueLeftDataLen = LengthOfCQueue(&app_uart_data_receive_queue);

            TRACE(1, "Queue Left len:%d, data:", rxqueueLeftDataLen);
            DUMP8("0x%02x ", extractedOtaRequestBuf, (rxqueueLeftDataLen > 20 ? 20: rxqueueLeftDataLen));


            app_uart_bridge_data_packet_t* pDataPacket = (app_uart_bridge_data_packet_t *)extractedOtaRequestBuf;
            if (UART_BRIDGE_CMD_GROUP_INSTANT == UART_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
            {
                app_uart_bridge_instant_cmd_instance_t* pInstance =
                    app_uart_bridge_get_instant_cmd_entry(pDataPacket->cmdcode);

                if (pInstance->cmdhandler)
                {
                    pInstance->cmdhandler(pDataPacket->content, pDataPacket->packetlen);
                }
                else
                {
                    TRACE(1,"uart_bridge:%s cmd not handled",__func__);
                }
            }
            else
            {
                app_uart_bridge_queue_push(&app_uart_bridge_receive_queue, pDataPacket,
                    pDataPacket->packetlen + UART_PACKET_LENGTH_HEADER_LEN);
            }
        }
        else
        {
            break;
        }
    }
}

void app_uart_bridge_data_tx_done(const void* data, unsigned int len)
{
    if (app_uart_bridge_wait_tx_done_id)
    {
        osSemaphoreRelease(app_uart_bridge_wait_tx_done_id);
    }
}

static void app_uart_bridge_rx_handler(uint8_t* p_data_buff, uint16_t length)
{
    app_uart_bridge_data_packet_t* pDataPacket = (app_uart_bridge_data_packet_t *)p_data_buff;

    if (UART_BRIDGE_CMD_GROUP_TASK == UART_BRIDGE_CMD_GROUP(pDataPacket->cmdcode))
    {
        app_uart_bridge_task_cmd_instance_t* pInstance =
            app_uart_bridge_get_task_cmd_entry(pDataPacket->cmdcode);

        if (pInstance->cmdhandler)
        {
            pInstance->cmdhandler(pDataPacket->content, pDataPacket->packetlen);
        }
        else
        {
            TRACE(1,"uart_bridge:%s cmd not handled",__func__);
        }
    }
    else
    {
        ASSERT(false, "uart_bridge instant cmd shouldn't get here.");
    }
}

static void app_uart_bridge_rx_thread(const void *arg)
{
    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (osEventSignal == evt.status)
        {
            if (evt.value.signals & APP_UART_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED)
            {
                while (app_uart_bridge_get_queue_length(&app_uart_bridge_receive_queue) > 0)
                {
                    static uint8_t rcv_tmp_buffer[APP_UART_BRIDGE_RX_THREAD_TMP_BUF_SIZE];

                    uint16_t rcv_length =
                        app_uart_bridge_queue_get_next_entry_length(
                            &app_uart_bridge_receive_queue);

                    ASSERT(rcv_length <= sizeof(rcv_tmp_buffer),
                        "[%s]: Bad rcv_length=%d, > %d", __func__, rcv_length, sizeof(rcv_tmp_buffer));

                    if (rcv_length > 0)
                    {
                        app_uart_bridge_queue_pop(
                            &app_uart_bridge_receive_queue,
                            rcv_tmp_buffer, rcv_length);
                        app_uart_bridge_rx_handler(rcv_tmp_buffer, rcv_length);
                    }
                }
            }

#ifdef UART_TOGGLE_MODE
            if (evt.value.signals & APP_UART_BRIDGE_RX_THREAD_SIGNAL_IRQ_RECEIVED)
            {
                app_uart_start_receiving_data();
                TRACE(1, "%s keep uart1 rx timer start, status: %d", __func__, app_uart_info.uart_status);
                app_uart_bridge_keep_rx_timer_start();

                if (UART_STATUS_SENDING == app_uart_info.uart_status) {
                    // Send data
                    SEND_DATA_INFO_T data_info = app_uart_get_send_info();
                    app_uart_send_data(data_info.buf, data_info.len);
                    app_uart_info.uart_status = UART_STATUS_IDLE;
                } else {
                    // Response to peer device
                    app_uart_toggle_irq_gpio((enum UART_IRQ_PIN_NUM_T)IRQ_PIN_TRANS_OUT_REC_IN);
                    //app_uart_info.uart_status = UART_STATUS_DONE;
                }
            }
#endif
        }
    }
}

typedef struct
{
    uint16_t cmdCode;
    uint16_t cmd_len;
    uint8_t *cmd_buffer;
} APP_UART_BRIDGE_TX_MAILBOX_PARAM_T;

osMailQDef (app_uart_bridge_tx_mailbox, APP_UART_BRIDGE_TX_MAILBOX_MAX,
            APP_UART_BRIDGE_TX_MAILBOX_PARAM_T);

static osMutexId app_uart_bridge_tx_mutex_id = NULL;
osMutexDef(app_uart_bridge_tx_mutex);

static osMailQId app_uart_bridge_tx_mailbox_id = NULL;

static heap_handle_t app_uart_bridge_tx_mailbox_heap;
static uint8_t app_uart_bridge_tx_mailbox_heap_pool[APP_UART_BRIDGE_MAX_XFER_DATA_SIZE*(APP_UART_BRIDGE_TX_MAILBOX_MAX/2)];

void app_uart_bridge_tx_mailbox_heap_init(void)
{
    app_uart_bridge_tx_mailbox_heap =
        heap_register(app_uart_bridge_tx_mailbox_heap_pool,
        sizeof(app_uart_bridge_tx_mailbox_heap_pool));
}

void *app_uart_bridge_tx_mailbox_heap_malloc(uint32_t size)
{
    void *ptr = NULL;

    if (size) {
        ptr = heap_malloc(app_uart_bridge_tx_mailbox_heap, size);
        ASSERT(ptr!=NULL, "[%s]: null ptr, size:%d", __func__, size);
    }
    return ptr;
}

void app_uart_bridge_tx_mailbox_heap_free(void *rmem)
{
    if (rmem){
        heap_free(app_uart_bridge_tx_mailbox_heap, rmem);
    }
}

static int32_t app_uart_bridge_tx_mailbox_get(APP_UART_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;

    evt = osMailGet(app_uart_bridge_tx_mailbox_id, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_uart_bridge_tx_mailbox_raw(APP_UART_BRIDGE_TX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(app_uart_bridge_tx_mailbox_id, 0);
    if (evt.status == osEventMail)
    {
        *msg_p = (APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t app_uart_bridge_tx_mailbox_free(APP_UART_BRIDGE_TX_MAILBOX_PARAM_T* msg_p)
{
    osStatus status;
    app_uart_bridge_tx_mailbox_heap_free((void *)msg_p->cmd_buffer);
    status = osMailFree(app_uart_bridge_tx_mailbox_id, msg_p);
    return (int32_t)status;
}

static int32_t app_uart_bridge_tx_mailbox_put(APP_UART_BRIDGE_TX_MAILBOX_PARAM_T* msg_src)
{
#define ALLOC_TRYCNT 7
    int i;
    APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;
    osStatus status;
    uint32_t dly_list[] = {5,10,25,50,50,50,50};

    for (i = 0; i < ALLOC_TRYCNT; i++) {
        msg_p = (APP_UART_BRIDGE_TX_MAILBOX_PARAM_T*)osMailAlloc(app_uart_bridge_tx_mailbox_id, 0);
        if (msg_p != NULL) {
            break;
        }
        TRACE(0, "[%s]: WARNING, alloc mbox failed, try %d", __func__, i);
        osDelay(dly_list[i]);
    }
    if (msg_p == NULL) {
        app_uart_bridge_task_cmd_instance_t *cmdEntry;

        TRACE(0, "DUMP MBOX DATA:");
        for (i=0; i < APP_UART_BRIDGE_TX_MAILBOX_MAX; i++){
            app_uart_bridge_tx_mailbox_raw(&msg_p);
            cmdEntry = app_uart_bridge_get_task_cmd_entry(msg_p->cmdCode);
            TRACE(0, "cltr_maillbox[%d]: cmdcode=%s",i, cmdEntry->log_cmd_code_str);
        }
        TRACE(0,"DUMP MBOX END");
        ASSERT(false, "[%s]: alloc mbox error", __func__);
    }
    msg_p->cmdCode = msg_src->cmdCode;
    msg_p->cmd_len = msg_src->cmd_len ;
    msg_p->cmd_buffer = msg_src->cmd_buffer;
    status = osMailPut(app_uart_bridge_tx_mailbox_id, msg_p);
    return (int32_t)status;
}

static int32_t app_uart_bridge_tx_mailbox_init(void)
{
    app_uart_bridge_tx_mailbox_id = osMailCreate(osMailQ(app_uart_bridge_tx_mailbox), NULL);
    if (NULL == app_uart_bridge_tx_mailbox_id)
    {
        // should increase OS_DYNAMIC_MEM_SIZE
        ASSERT(0, "Failed to Create core bridge mailbox\n");
        return -1;
    }
    return 0;
}

static void app_uart_bridge_tx_thread_init(void)
{
    if (app_uart_bridge_tx_mutex_id == NULL) {
        app_uart_bridge_tx_mutex_id = osMutexCreate(osMutex(app_uart_bridge_tx_mutex));
    }

    app_uart_bridge_tx_mailbox_heap_init();
    app_uart_bridge_tx_mailbox_init();
    if (app_uart_bridge_tx_thread_id == NULL) {
        app_uart_bridge_tx_thread_id =
            osThreadCreate(osThread(app_uart_bridge_tx_thread), NULL);
    }
}

static app_uart_bridge_task_cmd_instance_t* app_uart_bridge_get_task_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__uart_bridge_task_cmd_table_end-
        (uint32_t)__uart_bridge_task_cmd_table_start)/sizeof(app_uart_bridge_task_cmd_instance_t);
        index++) {
        if (UART_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return UART_BRIDGE_TASK_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

static app_uart_bridge_instant_cmd_instance_t* app_uart_bridge_get_instant_cmd_entry(uint16_t cmdcode)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__uart_bridge_instant_cmd_table_end-
        (uint32_t)__uart_bridge_instant_cmd_table_start)/sizeof(app_uart_bridge_instant_cmd_instance_t);
        index++) {
        if (UART_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index)->cmdcode == cmdcode) {
            return UART_BRIDGE_INSTANT_CMD_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    ASSERT(0, "[%s] find cmdcode index failed, wrong_cmdcode=%x", __func__, cmdcode);
    return NULL;
}

void app_uart_bridge_tx_thread(const void *arg)
{
    int err = 0;
    APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        err = app_uart_bridge_tx_mailbox_get(&msg_p);
        if (err)
        {
            TRACE(0, "[%s]: WARNING: err=%d", __func__, err);
            continue;
        }
        if (UART_BRIDGE_CMD_GROUP_TASK == UART_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            //TRACE(0,"%s cmd_buffer data:",__func__);
            //DUMP8("0x%02x ", msg_p->cmd_buffer, msg_p->cmd_len);
            app_uart_bridge_task_cmd_instance_t* pTaskCmdInstance =
                app_uart_bridge_get_task_cmd_entry(msg_p->cmdCode);

            pTaskCmdInstance->uart_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else if (UART_BRIDGE_CMD_GROUP_INSTANT == UART_BRIDGE_CMD_GROUP(msg_p->cmdCode))
        {
            app_uart_bridge_instant_cmd_instance_t* pInstantCmdInstance =
                app_uart_bridge_get_instant_cmd_entry(msg_p->cmdCode);

            pInstantCmdInstance->uart_bridge_cmd_transmit_handler(
                msg_p->cmd_buffer, msg_p->cmd_len);
        }
        else
        {
            ASSERT(false, "Wrong core bridge cmd code 0x%x", msg_p->cmdCode);
        }

        app_uart_bridge_tx_mailbox_free(msg_p);
        if (msg_p != NULL)
        {
            msg_p = NULL;
        }
    }
}

int32_t app_uart_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length)
{
    int nRet = 0;
    uint8_t *cmd_buffer = NULL;
    APP_UART_BRIDGE_TX_MAILBOX_PARAM_T msg;

    if (!app_uart_bridge_inited)
    {
        TRACE(0, "[%s]: WARNING: not inited", __func__);
        return 0;
    }
    ASSERT(length <= APP_UART_BRIDGE_MAX_XFER_DATA_SIZE, "%s: Bad length=%d", __func__, length);
    if (length > 0)
    {
        cmd_buffer = (uint8_t *)app_uart_bridge_tx_mailbox_heap_malloc(length);
        ASSERT(cmd_buffer!=NULL, "[%s]: null ptr", __func__);
        memcpy(cmd_buffer, p_buff, length);
    }
    msg.cmdCode = cmd_code;
    msg.cmd_len = length;
    msg.cmd_buffer = cmd_buffer;

    osMutexWait(app_uart_bridge_tx_mutex_id, osWaitForever);
    nRet = app_uart_bridge_tx_mailbox_put(&msg);
    osMutexRelease(app_uart_bridge_tx_mutex_id);
    return nRet;
}

int32_t app_uart_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length)
{
    if (!app_uart_bridge_inited)
    {
        return 0;
    }

    APP_UART_BRIDGE_TX_MAILBOX_PARAM_T msg = {0,};
    app_uart_bridge_task_cmd_instance_t* cmdEntry;

    ASSERT(length + sizeof(rsp_code) <= APP_UART_BRIDGE_MAX_XFER_DATA_SIZE, \
           "%s p_buff overflow: %u", \
           __func__, \
           length);

    msg.cmd_len = length + sizeof(rsp_code);
    msg.cmd_buffer = (uint8_t *)app_uart_bridge_tx_mailbox_heap_malloc(msg.cmd_len);
    *(uint16_t *)msg.cmd_buffer = rsp_code;
    memcpy(msg.cmd_buffer + sizeof(rsp_code), p_buff, length);

    msg.cmdCode = UART_TASK_CMD_RSP;

    cmdEntry = app_uart_bridge_get_task_cmd_entry(msg.cmdCode);
    cmdEntry->uart_bridge_cmd_transmit_handler(msg.cmd_buffer, msg.cmd_len);

    app_uart_bridge_tx_mailbox_free(&msg);

    return 0;
}

static void app_uart_bridge_send_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    app_uart_bridge_task_cmd_instance_t* cmdEntry;
    app_uart_bridge_data_packet_t cmddata;

    ASSERT(length <= APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE,
        "%s: length(%d) out of range(%d)", __func__,
        length, APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE);

    cmddata.cmdcode = UART_TASK_CMD_RSP;
    cmddata.cmdseq = g_uart_cmdseq++;
    cmddata.packetlen = length;
    memcpy(cmddata.content, p_buff, length);
#ifdef UART_TOGGLE_MODE
    if(!uart_status_flag){
        app_uart_bridge_send_data((uint8_t *)&cmddata, length + APP_UART_HEADER_DATA_SIZE);
    }
    else
#endif
    {
        app_uart_send_data((uint8_t *)&cmddata, length + APP_UART_HEADER_DATA_SIZE);
    }
    cmdEntry = app_uart_bridge_get_task_cmd_entry(*(uint16_t *)p_buff);

    TRACE(2,"uart bridge:-------TRS RSP to cmdcode=%s seq = %d------->", \
          cmdEntry->log_cmd_code_str,cmddata.cmdseq);
}

static void app_uart_bridge_cmd_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    //real cmd code responsed
    uint16_t rspcode = ((p_buff[1] << 8) | p_buff[0]);
    app_uart_bridge_task_cmd_instance_t* pInstance =
                app_uart_bridge_get_task_cmd_entry(rspcode);

     TRACE(2,"uart bridge:<-----------RCV_RSP: cmdcode=%s-----------", \
          pInstance->log_cmd_code_str);

    uint32_t timeout_ms = pInstance->wait_rsp_timeout_ms;
    if (pInstance->app_uart_bridge_rsp_handle)
    {
        pInstance->app_uart_bridge_rsp_handle(&p_buff[2], length - 2);

        if (timeout_ms > 0)
        {
            osSemaphoreRelease(app_uart_bridge_wait_cmd_rsp_id);
        }
    }
    else
    {
        TRACE(0,"ibrt_ui_log:rsp no handler");
    }
}

UART_BRIDGE_TASK_COMMAND_TO_ADD(UART_TASK_CMD_RSP,
                                "uart bridge task cmd rsp",
                                app_uart_bridge_send_cmd_rsp_handler,
                                app_uart_bridge_cmd_rsp_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);

#define APP_UART_BRIDGE_QUEUE_DATA_LEN_BYTES    2

static void app_uart_bridge_queue_init(CQueue* ptrQueue, uint8_t* ptrBuf, uint32_t bufLen)
{
    uint32_t lock = int_lock();
    memset(ptrBuf, 0, bufLen);
    InitCQueue(ptrQueue, bufLen, (CQItemType *)ptrBuf);
    int_unlock(lock);
}

static int32_t app_uart_bridge_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length)
{
    int32_t nRet = -1;
    uint32_t lock = int_lock();
    if (length > 0)
    {
        uint16_t dataLen = (uint16_t)length;
        int queueAvailableLen = AvailableOfCQueue(ptrQueue);
        if ((dataLen+APP_UART_BRIDGE_QUEUE_DATA_LEN_BYTES) <= queueAvailableLen)
        {
            EnCQueue(ptrQueue, (CQItemType *)&dataLen, APP_UART_BRIDGE_QUEUE_DATA_LEN_BYTES);
            EnCQueue(ptrQueue, (CQItemType *)ptrData, length);
            nRet = 0;
        }
    }
    int_unlock(lock);
    osSignalSet(app_uart_bridge_rx_thread_id, APP_UART_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED);

    return nRet;
}

static uint16_t app_uart_bridge_queue_get_next_entry_length(CQueue* ptrQueue)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;
    uint16_t length = 0;
    uint8_t* ptr;

    uint32_t lock = int_lock();

    ptr = (uint8_t *)&length;
    // get the length of the fake message
    PeekCQueue(ptrQueue, APP_UART_BRIDGE_QUEUE_DATA_LEN_BYTES, &e1, &len1, &e2, &len2);

    memcpy(ptr,e1,len1);
    memcpy(ptr+len1,e2,len2);

    int_unlock(lock);

    return length;
}

static void app_uart_bridge_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;

    uint32_t lock = int_lock();
    // overcome the two bytes of msg length
    DeCQueue(ptrQueue, 0, APP_UART_BRIDGE_QUEUE_DATA_LEN_BYTES);

    PeekCQueue(ptrQueue, len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2)){
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(ptrQueue, 0, len);

        // reset the poped data to ZERO
        memset(e1, 0, len1);
        memset(e2, 0, len2);
    }else{
        memset(buff, 0x00, len);
    }
    int_unlock(lock);
}

static int32_t app_uart_bridge_get_queue_length(CQueue *ptrQueue)
{
    int32_t nRet = 0;

    uint32_t lock = int_lock();
    nRet = LengthOfCQueue(ptrQueue);
    int_unlock(lock);

    return nRet;
}

void app_uart_bridge_free_tx_mailbox(void)
{
    APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *msg_p = NULL;

    while(1)
    {
        osEvent evt;

        evt = osMailGet(app_uart_bridge_tx_mailbox_id, 5);
        if (evt.status == osEventMail)
        {
            msg_p = (APP_UART_BRIDGE_TX_MAILBOX_PARAM_T *)evt.value.p;
            app_uart_bridge_tx_mailbox_free(msg_p);
        }
        else
        {
            break;
        }
    }
}

void app_uart_bridge_init(void)
{
    if (app_uart_bridge_inited) {
        return;
    }
    ASSERT(APP_UART_BRIDGE_MAX_XFER_DATA_SIZE \
        > (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_UART_BRIDGE_TX_MAILBOX_MAX/2,
        "core bridge init: xfer data size %d too small, should larger than %d",
        APP_UART_BRIDGE_MAX_XFER_DATA_SIZE,
        (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4)/APP_UART_BRIDGE_TX_MAILBOX_MAX/2);

    ASSERT(APP_UART_BRIDGE_RX_BUFF_SIZE > (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge init: rx buff size %d too small, should large than %d",
        APP_UART_BRIDGE_RX_BUFF_SIZE, (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_UART_BRIDGE_TX_THREAD_STACK_SIZE > (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge tx thread: stack size %d too small, should larger than %d",
        APP_UART_BRIDGE_TX_THREAD_STACK_SIZE,(APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_UART_BRIDGE_RX_THREAD_STACK_SIZE > (APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2,
        "core bridge rx thread: stack size %d too small, should larger than %d",
        APP_UART_BRIDGE_RX_THREAD_STACK_SIZE,(APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE+4) * 2);

    ASSERT(APP_UART_BRIDGE_RX_THREAD_STACK_SIZE > APP_UART_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2,
        "core bridge rx thread: stack size %d too small, should larger than tmp size %d",
        APP_UART_BRIDGE_RX_THREAD_STACK_SIZE, APP_UART_BRIDGE_RX_THREAD_TMP_BUF_SIZE * 2);

    memset(&app_uart_bridge_receive_queue, 0, sizeof(app_uart_bridge_receive_queue));

    app_uart_bridge_queue_init(&app_uart_bridge_receive_queue, \
                            app_uart_bridge_rx_buff, \
                            sizeof(app_uart_bridge_rx_buff));
    g_uart_cmdseq = 0;

    app_uart_bridge_wait_tx_done_id =
        osSemaphoreCreate(osSemaphore(app_uart_bridge_wait_tx_done), 0);

    app_uart_bridge_wait_cmd_rsp_id =
        osSemaphoreCreate(osSemaphore(app_uart_bridge_wait_cmd_rsp), 0);

    app_uart_bridge_wait_peer_device_ready_id =
        osSemaphoreCreate(osSemaphore(app_uart_bridge_wait_peer_device_ready), 0);

    app_uart_bridge_tx_thread_init();

    if (app_uart_bridge_rx_thread_id == NULL) {
        app_uart_bridge_rx_thread_id =
            osThreadCreate(osThread(app_uart_bridge_rx_thread), NULL);
    }
    app_uart_register_data_receiver(app_uart_bridge_data_received);

#ifdef UART_TOGGLE_MODE
    if (!uart_keep_rx_timer_id) {
        uart_keep_rx_timer_id = osTimerCreate(osTimer(UART_KEEP_RX), osTimerOnce, (void *)0);
    }
    ASSERT(uart_keep_rx_timer_id, "create UART keep RX timer failed");

    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)app_uart_irq_pin_cfg, ARRAY_SIZE(app_uart_irq_pin_cfg));
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_uart_irq_pin_cfg[IRQ_PIN_TRANS_OUT_REC_IN].pin, HAL_GPIO_DIR_OUT, 0);
    app_wake_up_irq_enable();
    app_uart_info.uart_status = UART_STATUS_IDLE;
#else
    app_uart_start_receiving_data();
#endif
    app_uart_bridge_inited = true;
}

#ifdef UART_TOGGLE_MODE
/*
* UART1 mode switch
*
*/
static void app_uart_mode_switch_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    char uart_normal_mode[] = "normal";
    char uart_toggle_mode[] = "toggle";
    app_uart_bridge_send_data_without_waiting_rsp(UART_MODE_SWITCH_CMD, (uint8_t *)p_buff, length);

    if(*uart_normal_mode==*p_buff){
        uart_status_flag = true;
        app_uart_register_data_receiver(app_uart_bridge_data_received);
        app_uart_start_receiving_data();
        TRACE(0, "[%s] normal mode", __func__);
    }else if(*uart_toggle_mode==*p_buff){
        uart_status_flag = false;
        app_uart_stop_uart_rx();
        app_wake_up_irq_enable();
        TRACE(0, "[%s] toggle mode", __func__);
    }
}

static void app_uart_mode_switch_cmd_receive_handler(uint8_t *p_buff, uint16_t length)
{
    char uart_normal_mode[] = "normal";
    char uart_toggle_mode[] = "toggle";
    if(*uart_normal_mode==*p_buff){
        uart_status_flag = true;
        app_wake_up_irq_disable();
        TRACE(0, "[%s] normal mode", __func__);
    }else if(*uart_toggle_mode==*p_buff){
        uart_status_flag = false;
        app_uart_stop_uart_rx();
        app_wake_up_irq_enable();
        TRACE(0, "[%s] toggle mode", __func__);
    }
}

UART_BRIDGE_TASK_COMMAND_TO_ADD(UART_MODE_SWITCH_CMD,
                                "uart mode switch",
                                app_uart_mode_switch_cmd_handler,
                                app_uart_mode_switch_cmd_receive_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);
#endif

/*
* Test case for UART1 communication
*
*/

static void app_set_test_mode_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(0, "[%s] len = %d", __func__, length);
    app_uart_bridge_send_data_with_waiting_rsp(SET_TEST_MODE_CMD, (uint8_t *)p_buff, length);
}

static void app_set_test_mode_cmd_receive_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
    TRACE(1,"uart receive: %s, len=%d", p_buff, length);
    app_uart_bridge_send_rsp(SET_TEST_MODE_CMD, p_buff, length);

}

static void app_set_test_mode_cmd_wait_rsp_timeout(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);

    static uint8_t resend_time = RESEND_TIME;
    if(resend_time > 0)
    {
        app_uart_bridge_send_cmd(SET_TEST_MODE_CMD, p_buff, length);
        resend_time--;
    }
    else
    {
        resend_time = RESEND_TIME;
    }
}

static void app_set_test_mode_cmd_send_rsp_handler(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"%s", __func__);
}

UART_BRIDGE_TASK_COMMAND_TO_ADD(SET_TEST_MODE_CMD,
                                "test for uart bridge",
                                app_set_test_mode_cmd_handler,
                                app_set_test_mode_cmd_receive_handler,
                                APP_UART_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_set_test_mode_cmd_wait_rsp_timeout,
                                app_set_test_mode_cmd_send_rsp_handler,
                                NULL);
#endif
