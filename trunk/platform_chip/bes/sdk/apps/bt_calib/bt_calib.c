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
#include "cmsis_os.h"
#include "list.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include <string.h>
#include "bt_drv_interface.h"
#include "hal_uart.h"
#include "watchdog/watchdog.h"
#include "hal_sysfreq.h"
#include "app_thread.h"
#include "factory_section.h"
#include "nvrecord_env.h"
#include "nvrecord_bt.h"
#include "nvrecord_dev.h"
#include "app_pwl.h"
#include "tool_msg.h"
#include "pmu.h"
#include "hal_sleep.h"

#define APP_FACT_CPU_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_3
#define APP_FACTORYMODE_RETRY_LIMITED (2)

static bool isInFactoryFreqCalibMode = false;
static void app_factorymode_timehandler(void const *param);
static struct message_t send_msg = { { PREFIX_CHAR, }, };
static unsigned char send_seq = 0;
osTimerId app_factory_timer = NULL;
osTimerDef (APP_FACTORY_TIMER, app_factorymode_timehandler);

typedef enum APP_FACTORYMODE_STATUS_INDICATION_T {
    APP_FACTORYMODE_STATUS_INDICATION_RUNNING = 0,
    APP_FACTORYMODE_STATUS_INDICATION_PASS,
    APP_FACTORYMODE_STATUS_INDICATION_FAILED,
    APP_FACTORYMODE_STATUS_INDICATION_INVALID,

    APP_FACTORYMODE_STATUS_INDICATION_NUM
}APP_FACTORYMODE_STATUS_INDICATION_T;

static int app_factorymode_send_code (uint32_t progress)
{
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODULE_OHTER;
    msg.msg_body.message_id = 3;
    msg.msg_body.message_Param0 = progress;
    app_mailbox_put(&msg);

    return 0;
}

static int app_factorymode_bt_xtalcalib_proc(void)
{
    uint32_t capval = 0x80;
    int nRet;

    TRACE(1,"%s",__func__);
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    TRACE(1,"calib default, capval:%d", capval);
    btdrv_hciopen();
    btdrv_hci_reset();
    btdrv_hcioff();
    capval = 0x80;
    bt_drv_calib_open();
    nRet = bt_drv_calib_result_porc(&capval);
    bt_drv_calib_close();
    TRACE(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!calib ret:%d, capval:%d", nRet, capval);
    if (!nRet)
        nvrec_dev_set_xtal_fcap((unsigned int)capval);

    return nRet;
}

static int app_factorymode_send_progress (uint8_t progress)
{
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODULE_OHTER;
    msg.msg_body.message_id = 2;
    msg.msg_body.message_Param0 = progress;
    app_mailbox_put(&msg);

    return 0;
}

static uint8_t app_factorymode_status_indication(APP_FACTORYMODE_STATUS_INDICATION_T status)
{
    switch (status)
    {
        case APP_FACTORYMODE_STATUS_INDICATION_RUNNING:
            break;
        case APP_FACTORYMODE_STATUS_INDICATION_PASS:
            break;
        case APP_FACTORYMODE_STATUS_INDICATION_FAILED:
            break;
        default:
            break;
    }
    return 0;
}

static void app_factorymode_timehandler(void const *param)
{
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODULE_OHTER;
    msg.msg_body.message_id = 1;
    app_mailbox_put(&msg);
}

static int app_factorymode_msg_uart_send(const unsigned char *buf, size_t len)
{
    uint32_t sent = 0;

    while (sent < len) {
        hal_uart_blocked_putc(HAL_UART_ID_0, buf[sent++]);
    }

    if (sent != len) {
        return 1;
    }

    return 0;
}

static unsigned char app_factorymode_msg_check_sum(unsigned char *buf, unsigned char len)
{
    int i;
    unsigned char sum = 0;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }

    return sum;
}

static int app_factorymode_msg_send_32bitcode(uint32_t code)
{
    int ret;

    send_msg.hdr.type = 0x88;
    send_msg.hdr.seq = send_seq++;
    send_msg.hdr.len = 4;
    send_msg.data[0] = 0xf2;
    *(uint32_t *)&(send_msg.data[1]) = code;
    send_msg.data[4] = ~app_factorymode_msg_check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = app_factorymode_msg_uart_send((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
}

static int app_factorymode_msg_send_progress (uint8_t progress)
{
    int ret;

    send_msg.hdr.type = 0x88;
    send_msg.hdr.seq = send_seq++;
    send_msg.hdr.len = 2;
    send_msg.data[0] = progress;
    send_msg.data[1] = 100;
    send_msg.data[2] = ~app_factorymode_msg_check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = app_factorymode_msg_uart_send((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
}

static int app_factorymode_msg_send_ping(void)
{
    int ret;

    send_msg.hdr.type = 0x88;
    send_msg.hdr.seq = send_seq++;
    send_msg.hdr.len = 2;
    send_msg.data[0] = 0xaa;
    send_msg.data[1] = 0x55;
    send_msg.data[2] = ~app_factorymode_msg_check_sum((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg) - 1);

    ret = app_factorymode_msg_uart_send((unsigned char *)&send_msg, MSG_TOTAL_LEN(&send_msg));

    return ret;
}

static int app_factorymode_process(APP_MESSAGE_BODY *msg_body)
{
    if (msg_body->message_id == 1){
        app_factorymode_msg_send_ping();
    }
    if (msg_body->message_id == 2){
        app_factorymode_msg_send_progress(msg_body->message_Param0);
    }
    if (msg_body->message_id == 3){
        app_factorymode_msg_send_32bitcode(msg_body->message_Param0);
    }
    return 0;
}

static uint8_t app_factorymode_indication_init(void)
{
    struct APP_PWL_CFG_T cfg;

    memset(&cfg, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_open();
    app_pwl_setup(APP_PWL_ID_0, &cfg);
    app_pwl_setup(APP_PWL_ID_1, &cfg);
    return 0;
}

static int app_factorymode_uart_init(void)
{
    struct HAL_UART_CFG_T uart_cfg;
    enum HAL_TRACE_TRANSPORT_T transport;

    memset(&uart_cfg, 0, sizeof(struct HAL_UART_CFG_T));
    uart_cfg.parity = HAL_UART_PARITY_NONE,
    uart_cfg.stop = HAL_UART_STOP_BITS_1,
    uart_cfg.data = HAL_UART_DATA_BITS_8,
    uart_cfg.flow = HAL_UART_FLOW_CONTROL_NONE,//HAL_UART_FLOW_CONTROL_RTSCTS,
    uart_cfg.tx_level = HAL_UART_FIFO_LEVEL_1_2,
    uart_cfg.rx_level = HAL_UART_FIFO_LEVEL_1_4,
    uart_cfg.baud = 921600,
    uart_cfg.dma_rx = false,
    uart_cfg.dma_tx = false,
    uart_cfg.dma_rx_stop_on_err = false;

    transport = hal_trace_get_transport();
    if (transport == HAL_TRACE_TRANSPORT_UART0) {
        hal_trace_close();
    }

    hal_iomux_set_uart0();
    hal_uart_open(HAL_UART_ID_0, &uart_cfg);

    return 0;
}

static bool app_freqcalib_mode_get_handler(void)
{
    TRACE(2, "%s isInFactoryFreqCalibMode=%d",__func__, isInFactoryFreqCalibMode);
    return isInFactoryFreqCalibMode ? true : false;
}

int app_factorymode_calib_only(void)
{
    uint8_t cnt = 0;
    int nRet = 0;
    uint32_t capval = 0x00;

    isInFactoryFreqCalibMode = true;
    btdrv_register_freqcalib_mode_get_callback(app_freqcalib_mode_get_handler);

    app_factorymode_uart_init();
    app_wdt_reopen(60);
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, HAL_CMU_FREQ_52M);
    list_init();
    app_os_init();

    factory_section_init();
    nv_record_env_init();
    nvrec_dev_data_open();
    app_factorymode_indication_init();

    app_set_threadhandle(APP_MODULE_OHTER, app_factorymode_process);
    app_factory_timer = osTimerCreate(osTimer(APP_FACTORY_TIMER),osTimerPeriodic,NULL);
    osTimerStart(app_factory_timer,300);
    app_factorymode_status_indication(APP_FACTORYMODE_STATUS_INDICATION_RUNNING);
    app_factorymode_send_progress(10);
    btdrv_start_bt();
    osDelay(20);
    app_factorymode_send_progress(20);
    do {
        nRet = app_factorymode_bt_xtalcalib_proc();
    }while(nRet && ++cnt < APP_FACTORYMODE_RETRY_LIMITED);
    if(nRet)
        goto err;

    nvrec_dev_get_xtal_fcap((unsigned int*)&capval);
    app_factorymode_send_code(capval);

    app_factorymode_send_progress(50);
    osDelay(200);
    app_factorymode_send_progress(80);
    osDelay(100);
    app_factorymode_send_progress(100);
    app_factorymode_status_indication(APP_FACTORYMODE_STATUS_INDICATION_PASS);
    osSignalWait(0x01, osWaitForever);
    goto exit;

err:
    osTimerStop(app_factory_timer);
    app_factorymode_status_indication(APP_FACTORYMODE_STATUS_INDICATION_FAILED);
    app_factorymode_send_code(0xff);
    osSignalWait(0x01, osWaitForever);
exit:

    isInFactoryFreqCalibMode = false;
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, HAL_CMU_FREQ_32K);
    pmu_shutdown();
    return nRet;
}