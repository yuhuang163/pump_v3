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
#ifndef __APP_UART_BRIDGE_H__
#define __APP_UART_BRIDGE_H__

#ifndef APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE
#define APP_UART_BRIDGE_MAX_DATA_PACKET_SIZE    (516)
#endif

#ifndef APP_UART_BRIDGE_RX_BUFF_SIZE
#define APP_UART_BRIDGE_RX_BUFF_SIZE            (5160)
#endif

#ifndef APP_UART_BRIDGE_TX_MAILBOX_MAX
#define APP_UART_BRIDGE_TX_MAILBOX_MAX          (8)
#endif

#ifndef APP_UART_BRIDGE_MAX_XFER_DATA_SIZE
#define APP_UART_BRIDGE_MAX_XFER_DATA_SIZE      (516)
#endif

// every transfer has the fixed size
#define APP_UART_HEADER_DATA_SIZE               (6)

#ifndef APP_UART_BRIDGE_RX_THREAD_TMP_BUF_SIZE
#define APP_UART_BRIDGE_RX_THREAD_TMP_BUF_SIZE  (516)
#endif

#ifndef APP_UART_BRIDGE_TX_THREAD_STACK_SIZE
#define APP_UART_BRIDGE_TX_THREAD_STACK_SIZE    (2048+1024-512)
#endif

#ifndef APP_UART_BRIDGE_RX_THREAD_STACK_SIZE
#define APP_UART_BRIDGE_RX_THREAD_STACK_SIZE    (1024*4+1024-512)
#endif

#define RESEND_TIME 2

#define APP_UART_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS 3000


#define UART_BRIDGE_CMD_GROUP_TASK      0x00
#define UART_BRIDGE_CMD_GROUP_INSTANT   0x01

#define UART_BRIDGE_CMD_GROUP_OFFSET           (8)
#define UART_BRIDGE_CMD_SUBCODE_BIT(subCode)   ((subCode)&((1<<UART_BRIDGE_CMD_GROUP_OFFSET)-1))
#define UART_BRIDGE_CMD_CODE(group, subCode)   (((group) << UART_BRIDGE_CMD_GROUP_OFFSET)|UART_BRIDGE_CMD_SUBCODE_BIT(subCode))
#define UART_BRIDGE_CMD_SUBCODE(cmdCode)       ((cmdCode)&((1 << UART_BRIDGE_CMD_GROUP_OFFSET)-1))
#define UART_BRIDGE_CMD_GROUP(cmdCode)         ((cmdCode) >> UART_BRIDGE_CMD_GROUP_OFFSET)

#define UART_BRIDGE_CMD_INSTANT(subCode) UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_INSTANT, (subCode))
#define UART_BRIDGE_CMD_TASK(subCode)    UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, (subCode))

typedef void (*app_uart_bridge_cmd_transmit_handler_t)(uint8_t*, uint16_t);
typedef void (*app_uart_bridge_cmd_receivd_handler_t)(uint8_t*, uint16_t);
typedef void (*app_uart_bridge_wait_rsp_timeout_handle_t)(uint8_t*, uint16_t);
typedef void (*app_uart_bridge_rsp_handle_t)(uint8_t*, uint16_t);
typedef void (*app_uart_bridge_cmd_transmission_done_handler_t) (uint16_t, uint8_t*, uint16_t);

typedef struct
{
    uint16_t                                        cmdcode;
    const char                                      *log_cmd_code_str;
    app_uart_bridge_cmd_transmit_handler_t           uart_bridge_cmd_transmit_handler;
    app_uart_bridge_cmd_receivd_handler_t            cmdhandler;
    uint32_t                                        wait_rsp_timeout_ms;
    app_uart_bridge_wait_rsp_timeout_handle_t        app_uart_bridge_wait_rsp_timeout_handle;
    app_uart_bridge_rsp_handle_t                     app_uart_bridge_rsp_handle;
    app_uart_bridge_cmd_transmission_done_handler_t  app_uart_bridge_transmisson_done_handler;
} __attribute__((aligned(4))) app_uart_bridge_task_cmd_instance_t;

typedef struct
{
    uint16_t                                cmdcode;
    app_uart_bridge_cmd_transmit_handler_t   uart_bridge_cmd_transmit_handler;
    app_uart_bridge_cmd_receivd_handler_t    cmdhandler;
} __attribute__((aligned(4))) app_uart_bridge_instant_cmd_instance_t;


#define UART_BRIDGE_TASK_COMMAND_TO_ADD(cmdCode,                            \
                                        log_cmd_code_str,                   \
                                        uart_bridge_cmd_transmit_handler,   \
                                        cmdhandler,                         \
                                        wait_rsp_timeout_ms,                \
                                        app_uart_bridge_wait_rsp_timeout_handle,    \
                                        app_uart_bridge_rsp_handle,                 \
                                        app_uart_bridge_transmisson_done_handler)   \
                                        static const app_uart_bridge_task_cmd_instance_t cmdCode##task##_entry  \
                                        __attribute__((used, section(".uart_bridge_task_cmd_table"))) =     \
                                        {(cmdCode),       \
                                        (log_cmd_code_str),                         \
                                        (uart_bridge_cmd_transmit_handler),         \
                                        (cmdhandler),                               \
                                        (wait_rsp_timeout_ms),                      \
                                        (app_uart_bridge_wait_rsp_timeout_handle),  \
                                        (app_uart_bridge_rsp_handle),               \
                                        (app_uart_bridge_transmisson_done_handler)};


#define UART_BRIDGE_INSTANT_COMMAND_TO_ADD(cmdCode,                         \
                                        uart_bridge_cmd_transmit_handler,   \
                                        cmdhandler)                         \
                                        static const app_uart_bridge_instant_cmd_instance_t cmdCode##task##_entry   \
                                        __attribute__((used, section(".uart_bridge_instant_cmd_table"))) =      \
                                        {(cmdCode),        \
                                        (uart_bridge_cmd_transmit_handler), \
                                        (cmdhandler)};

typedef enum
{
    UART_TASK_CMD_RSP                           = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x00),
    SET_TEST_MODE_CMD                           = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x01),
    TDM_RELAY_CMD_RSP                           = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x02),
    TDM_RELAY_CMD_WITHOUT_RSP                   = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x03),
#ifdef UART_TOGGLE_MODE
    UART_MODE_SWITCH_CMD                        = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x04),
#endif
    SET_OTA_MODE_CMD                            = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x05),
#ifdef BLE_WIFI_DUAL_CHIP_MODE
    BLE_WIFI_AUDIO_MANAGER_CMD                  = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x07),
    BLE_WIFI_PAIRING_INFO_EXCHANGE              = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x08),
    BLE_WIFI_MEDIA_CONTROL_CMD                  = UART_BRIDGE_CMD_CODE(UART_BRIDGE_CMD_GROUP_TASK, 0x09),
#endif
} UART_BRIDGE_CMD_CODE_E;

#ifdef UART_TOGGLE_MODE
#define MASTER_WAKE_UP_SLAVE_IRQ_RX_IO HAL_IOMUX_PIN_P1_5
#define MASTER_WAKE_UP_SLAVE_IRQ_TX_IO HAL_IOMUX_PIN_P1_4

enum UART_IRQ_PIN_NUM_T {
    IRQ_PIN_TRANS_OUT_REC_IN = 0,
    IRQ_PIN_TRANS_IN_REC_OUT,
    IRQ_PIN_NUM
};

enum UART_BRIDGE_STATUS {
    UART_STATUS_IDLE = 0,
    UART_STATUS_SENDING,
};

typedef struct
{
    uint8_t* buf;
    uint32_t len;
} SEND_DATA_INFO_T;

typedef struct
{
    // uart status (see #UART_BRIDGE_STATUS enumeration)
    uint8_t uart_status;
    SEND_DATA_INFO_T data_info;
} APP_UART_INFO_T;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void app_uart_bridge_init(void);
void app_uart_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_uart_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
int32_t app_uart_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length);
int32_t app_uart_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length);
unsigned int app_uart_bridge_data_received(const void* data, unsigned int len);
void app_uart_bridge_data_tx_done(const void* data, unsigned int len);
void app_uart_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_uart_bridge_free_tx_mailbox(void);
void app_send_bt_status_handler(uint8_t *p_buff, uint16_t length);
void app_bt_status_receive_handler(uint8_t *p_buff, uint16_t length);
#ifdef __cplusplus
}
#endif

#endif