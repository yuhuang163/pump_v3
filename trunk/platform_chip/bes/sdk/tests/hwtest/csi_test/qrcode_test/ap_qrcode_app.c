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
#ifdef QRCODE_TEST

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "ap_qrcode_app.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "a7_cmd.h"
#include "hal_location.h"
#include "stdio.h"
#include "app_key.h"
#include "zbar_detector.h"
#include "video_stream_rpc.h"

#define QRCODE_TEST_DEBUG

#ifdef QRCODE_TEST_DEBUG
#define QR_DBG     TRACE
#else
#define QR_DBG(...) do{}while(0)
#endif

#define AP_QRCODE_SIGNAL_RPC   (0x01)
#define AP_QRCODE_SIGNAL_IMG   (0x02)
#define AP_QRCODE_SIGNAL_KEY   (0x04)
#define AP_QRCODE_SIGNAL_DIS   (0x08)

static struct qrcode_rpc_message  qrcode_rx_msg = {0};

static osThreadId ap_qrcode_thread = NULL;
static void ap_qrcode_task(void *argument);
osThreadDef(ap_qrcode_task, osPriorityNormal, 1, 32 * 1024, "ap_qrcode_task");

static osThreadId simu_qrcode_trig_tid = NULL;
static void simu_qrcode_trig_task(void *argument);
osThreadDef(simu_qrcode_trig_task, osPriorityNormal, 1, 8 * 1024, "simu_qrcode_trig_task");

static int ap_qrcode_inited = 0;
static int ap_qrcode_open_succ = 0;

static void ap_qrcode_key(APP_KEY_STATUS *status, void *param);


static void ap_qrcode_key_init(void)
{
    uint8_t i = 0;
    const APP_KEY_HANDLE  key_cfg[] = {
        {{APP_KEY_CODE_FN9,APP_KEY_EVENT_DOWN},"qrcode start", ap_qrcode_key, NULL},
        {{APP_KEY_CODE_FN9,APP_KEY_EVENT_DOUBLECLICK},"qrcode kill",ap_qrcode_key, NULL},
        {{APP_KEY_CODE_FN9,APP_KEY_EVENT_UP},"qrcode stop",ap_qrcode_key, NULL},
    };

    TRACE(1,"%s",__func__);
    for (i=0; i<(sizeof(key_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&key_cfg[i]);
    }

    app_key_onoff(1);
}

int ap_qrcode_task_init(void)
{
    TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if(!ap_qrcode_inited)
    {
        simu_qrcode_trig_tid = osThreadCreate(osThread(simu_qrcode_trig_task), NULL);
        if (simu_qrcode_trig_tid == NULL) {
            TRACE(0, "create simu_qrcode_trig_task failed.");
            return -1;
        }

        ap_qrcode_thread = osThreadCreate(osThread(ap_qrcode_task), NULL);
        if (ap_qrcode_thread == NULL) {
            TRACE(0, "create ap_qrcode_thread failed.");
            if (simu_qrcode_trig_tid != NULL) {
                osThreadTerminate(simu_qrcode_trig_tid);
            }
            return -1;
        }

        // ap_qrcode_key_init();

        ap_qrcode_inited = 1;
    }

    return 0;
}
int ap_qrcode_task_deinit(void)
{
    // thread exit
    if (ap_qrcode_thread != NULL)  {
        osThreadTerminate(ap_qrcode_thread);
    }
    if (simu_qrcode_trig_tid != NULL) {
        osThreadTerminate(simu_qrcode_trig_tid);
    }

    TRACE(0, "%s(true),%d qrcode released.\n\r", __FUNCTION__, __LINE__);

    return 0;
}


static void ap_qrcode_msg_send(struct qrcode_rpc_message *msg)
{
    struct RPC_REQ_MSG_T req_msg = {0};

    uint32_t msg_type = msg->msg_type;
    uint32_t sync = msg->sync;
    uint32_t data_len = msg->data_len;

    req_msg.cmd = RPC_MSG_VP_QRCODE_CMD;
    req_msg.param[0] = (msg_type << 24) | (sync << 16) | (data_len);

    vplayer_fine_request_msg_send_no_ack(&req_msg);
}

static void qrcode_rpc_send_start(void)
{
    struct qrcode_rpc_message tx_msg = {0};

    tx_msg.msg_type = (uint8_t)QRCODE_START_M33_TO_A7;
    tx_msg.sync = 1;
    tx_msg.data_len = 0;

    ap_qrcode_msg_send(&tx_msg);
}

static void qrcode_rpc_send_stop(void)
{
    struct qrcode_rpc_message tx_msg = {0};

    tx_msg.msg_type = (uint8_t)QRCODE_STOP_M33_TO_A7;
    tx_msg.sync = 1;
    tx_msg.data_len = 0;

    ap_qrcode_msg_send(&tx_msg);
}

void ap_qrcode_rpc_rx_handler(void *data, int size)
{
    if ((!data) || (size < sizeof(struct RPC_REQ_MSG_T))) {
        QR_DBG(1, "ap_qrcode recieved failed.");
        return;
    }

    struct RPC_REQ_MSG_T *req_msg = (struct RPC_REQ_MSG_T *)data;

    uint32_t param0 = req_msg->param[0];
    qrcode_rx_msg.msg_type = (uint8_t)((param0 >> 24) & 0xff);
    qrcode_rx_msg.sync = (uint8_t)((param0 >> 16) & 0xff);
    qrcode_rx_msg.data_len = (uint16_t)(param0 & 0xffff);

    QR_DBG(1, "[%s:%d] req_msg:%p, type:%d.", __FUNCTION__, __LINE__, req_msg, qrcode_rx_msg.msg_type);

    osSignalSet(ap_qrcode_thread, AP_QRCODE_SIGNAL_RPC);
}

static void ap_qrcode_task(void *argument)
{
    osEvent evt;
    uint32_t signals = 0;
    //todo init

    while (1)
    {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        QR_DBG(1, "[%s]: hello signals=%x", __func__, signals);
        if (evt.status == osEventSignal) {
            if (signals & AP_QRCODE_SIGNAL_RPC) {
                switch (qrcode_rx_msg.msg_type)
                {
                case QRCODE_OPEN_A7_TO_M33:
                    // recv_qrcode_process_open();
                    break;
                case QRCODE_CLOSE_A7_TO_M33:
                    // recv_qrcode_process_close();
                    break;
                case QRCODE_START_A7_TO_M33:
                    // recv_qrcode_process_start();
                    break;
                case QRCODE_STOP_A7_TO_M33:
                    // recv_qrcode_process_stop();
                    break;
                default:
                    TRACE(0, "%s %d unsupport msg_type %d", __FUNCTION__,  __LINE__, qrcode_rx_msg.msg_type);
                    break;
                }
            }
        }
    }
}

/* --------------------------- key & trigger ---------------------------- */

static void ap_qrcode_key(APP_KEY_STATUS *status, void *param)
{
    static uint8_t qr_started = 0;

    QR_DBG(3,"%s %d,%d",__func__, status->code, status->event);

    switch(status->event)
    {
    case APP_KEY_EVENT_DOWN:
        if (qr_started) {
            qrcode_rpc_send_stop();
            qr_started = 0;
        } else {
            qrcode_rpc_send_start();
            qr_started = 1;
        }
        QR_DBG(1,"first blood!");
        break;
    case APP_KEY_EVENT_DOUBLECLICK:
        qrcode_rpc_send_stop();
        qr_started = 0;
        QR_DBG(1,"double kill");
        break;
    default:
        break;
    }
}

static void simu_qrcode_trig_task(void *argument)
{
    uint32_t t = 0;

    osDelay(5000);

    while(1){
        qrcode_rpc_send_start();
        osDelay(5000);
        qrcode_rpc_send_stop();
        osDelay(2000);
    }
}

#endif  /* QRCODE_TEST */
