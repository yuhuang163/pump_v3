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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "dsp_qrcode_process.h"
#include <string.h>
#include "a7_cmd.h"
#include "speech_msg.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "ap_qrcode_app.h"
#include "zbar_detector.h"
#include "qrcode_gui.h"
#include "vplayer_app_common.h"
#include "hal_timer.h"
#include "camera_drv.h"
#include "video_stream_rpc.h"


#define QRCODE_TEST_DEBUG

#ifdef QRCODE_TEST_DEBUG
#define QR_DBG     TRACE
#else
#define QR_DBG(...) do{}while(0)
#endif

#define QRCODE_SIGNAL_RPC   (0x01)

#define QRCODE_STATUS_STOP      (0x01)
#define QRCODE_STATUS_IDLE      (0x02)
#define QRCODE_STATUS_BUSY      (0x04)
#define QRCODE_STATUS_RESULT    (0x08)

#define CAMERA_PIXEL_SIZE   (2)
#define CAMERA_WIDTH        (480)
#define CAMERA_HEIGHT       (640)
#define CAMERA_PIXEL_NUM    (CAMERA_WIDTH * CAMERA_HEIGHT)

static struct qrcode_rpc_message qrcode_process_msg = {0};

osThreadId_t qrcode_rpc_msg_thread = NULL;
static void qrcode_rpc_msg_task(void *argument);
osThreadDef(qrcode_rpc_msg_task, osPriorityNormal, 1, 32 * 1024, "qrcode_rpc_msg_task");

osSemaphoreDef(qrcode_process_sem);
static osSemaphoreId qrcode_process_sem_id = NULL;

static void qrcode_process_task(void *argument);
osThreadDef(qrcode_process_task, osPriorityNormal, 1, 32 * 1024, "qrcode_process_task");

static int qrcode_process_enable = 0;
static int qrcode_process_inited = 0;

static struct qrcode_img_cfg qrcode_img = {0};
static struct qrcode_img_cfg img_capture_cfg = {0};
static uint8_t img_capture[CAMERA_PIXEL_NUM * CAMERA_PIXEL_SIZE] = {0};

static int qrcode_proc_status = QRCODE_STATUS_STOP;

static void qrcode_process_display_result(void);
static void qrcode_process_display_video(void);
static void qrcode_process_display_blank(void);

static int qrcode_process_open(void)
{
    if (qrcode_process_enable == 0) {
        qrcode_process_enable = 1;
    }
    return 0;
}

static int qrcode_process_close(void)
{
    if (qrcode_process_enable) {
        qrcode_process_enable = 0;
    }
    return 0;
}

static void qrcode_process_start(void)
{
    qrcode_proc_status = QRCODE_STATUS_IDLE;
    qrcode_process_display_video();
}

static void qrcode_process_stop(void)
{
    qrcode_proc_status = QRCODE_STATUS_STOP;
    qrcode_process_display_blank();
}

static void qrcode_msg_send(struct qrcode_rpc_message *msg)
{
    struct RPC_REQ_MSG_T req_msg = {0};

    uint32_t msg_type = msg->msg_type;
    uint32_t sync = msg->sync;
    uint32_t data_len = msg->data_len;

    req_msg.cmd = RPC_MSG_VP_QRCODE_CMD;
    req_msg.param[0] = (msg_type << 24) | (sync << 16) | (data_len);

    vplayer_fine_request_msg_send_no_ack(&req_msg);
}

int dsp_qrcode_process_init(void)
{
    if (qrcode_process_inited == 0)
    {
        qrcode_process_sem_id = osSemaphoreCreate(osSemaphore(qrcode_process_sem), 0);
        if (qrcode_process_sem_id == NULL) {
            TRACE(0, "create qrcode_process_sem_id failed");
            return -1;
        }

        osThreadId_t qrcode_process_thread_id = osThreadCreate(osThread(qrcode_process_task), NULL);
        if (qrcode_process_thread_id == NULL) {
            TRACE(0, "create qrcode_process_thread failed");
            if (qrcode_process_sem_id)
            {
                osSemaphoreDelete(qrcode_process_sem_id);
            }
            return -1;
        }

        qrcode_rpc_msg_thread = osThreadCreate(osThread(qrcode_rpc_msg_task), NULL);
        if (qrcode_rpc_msg_thread == NULL) {
            TRACE(0, "create qrcode_rpc_msg_thread failed");
            if (qrcode_process_sem_id)
            {
                osSemaphoreDelete(qrcode_process_sem_id);
            }
            if (qrcode_process_thread_id != NULL) {
                osThreadTerminate(qrcode_process_thread_id);
            }
            return -1;
        }

        // reg dsp_qrcode_msg_handler invoked in a7_cmd & mcu_cmd;

        qrcode_process_inited = 1;

        qrcode_process_open();

        zbar_init();
    }

    return 0;
}

void dsp_qrcode_rpc_rx_handler(void *data, int size)
{
    if ((!data) || (size < sizeof(struct RPC_REQ_MSG_T)))  {
        TRACE(0, "dsp_qrcode recieved failed.");
        return;
    }
    if (!qrcode_process_enable) {
        return;
    }
    struct RPC_REQ_MSG_T *req_msg = (struct RPC_REQ_MSG_T *)data;

    uint32_t param0 = req_msg->param[0];
    qrcode_process_msg.msg_type = (uint8_t)((param0 >> 24) & 0xff);
    qrcode_process_msg.sync = (uint8_t)((param0 >> 16) & 0xff);
    qrcode_process_msg.data_len = (uint16_t)(param0 & 0xffff);

    QR_DBG(0, "[%s:%d] req_msg:%p, type:%d.", __FUNCTION__, __LINE__, req_msg, qrcode_process_msg.msg_type);

    osSignalSet(qrcode_rpc_msg_thread, QRCODE_SIGNAL_RPC);
}

static void qrcode_rpc_msg_task(void *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1)
    {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        QR_DBG(1, "[%s]: hello signals=%x", __func__, signals);
        if (evt.status == osEventSignal) {
            if (signals & QRCODE_SIGNAL_RPC) {
                switch (qrcode_process_msg.msg_type)
                {
                case QRCODE_OPEN_M33_TO_A7:
                    qrcode_process_open();
                    break;
                case QRCODE_CLOSE_M33_TO_A7:
                    qrcode_process_close();
                    break;
                case QRCODE_START_M33_TO_A7:
                    qrcode_process_start();
                    break;
                case QRCODE_STOP_M33_TO_A7:
                    qrcode_process_stop();
                    break;
                default:
                    TRACE(0, "%s %d unsupport msg_type %d",  __FUNCTION__,  __LINE__, qrcode_process_msg.msg_type);
                    break;
                }
                osDelay(10);
                if (qrcode_process_msg.sync) {
                    struct qrcode_rpc_message tx_msg = {0};
                    tx_msg.msg_type = qrcode_process_msg.msg_type + 1;
                    tx_msg.sync = 0x0;
                    qrcode_msg_send(&tx_msg);
                }
            }
        }
    }
}

static void qrcode_process_display_result(void)
{
    qr_gui_get_result(&qrcode_result);
    qr_gui_show_result();
}

static void qrcode_process_display_video(void)
{
    qr_gui_show_video_player();
}

static void qrcode_process_display_blank(void)
{
    qr_gui_show_blank();
}

static uint8_t qrcode_process(void)
{
    memcpy((void*)img_capture, (void*)(img_capture_cfg.addr), sizeof(img_capture));
    memcpy(&qrcode_img, &img_capture_cfg, sizeof(struct qrcode_img_cfg));
    qrcode_img.addr = (uint32_t)img_capture;
    qr_gui_img_capture_get(qrcode_img.addr );

    qrcode_img_copy(&qrcode_img);
    int res = zbar_detect(&qrcode_img);
    if (res) {
        QR_DBG(1, "decoded:%s symbol:[%s]", qrcode_result.type, qrcode_result.data);
        for (int i=0; i<qrcode_result.location_count; i++) {
            QR_DBG(1, "symbol location point%d:(%d, %d)", i, \
                qrcode_result.location_x[i],qrcode_result.location_y[i]);
        }
        return 1;
    } else {
        return 0;
    }
}

void qrcode_get_img(uint32_t addr, uint16_t height, uint16_t width, uint16_t bpp, uint16_t fmt)
{
    if (QRCODE_STATUS_IDLE == qrcode_proc_status) {
        img_capture_cfg.addr = addr;
        img_capture_cfg.height = height;
        img_capture_cfg.width = width;
        img_capture_cfg.bpp = bpp;
        img_capture_cfg.fmt = fmt;

        osSemaphoreRelease(qrcode_process_sem_id);

        qrcode_proc_status = QRCODE_STATUS_BUSY;
    }
}

static void qrcode_process_task(void *argument)
{
    uint32_t t = 0;
    uint8_t res = 0;
    while (1)
    {
        osSemaphoreWait(qrcode_process_sem_id, osWaitForever);

        res = qrcode_process();
        t++;
        osDelay(100);
        if (res == 1) {
            t = 0;
            if (qrcode_proc_status != QRCODE_STATUS_STOP) {
                qrcode_proc_status = QRCODE_STATUS_RESULT;
                qrcode_process_display_result();
            }
        } else {
            if (QRCODE_STATUS_BUSY == qrcode_proc_status) {
                if (t > 50) {
                    t = 0;
                    qrcode_proc_status = QRCODE_STATUS_STOP;
                    qrcode_process_display_blank();
                } else {
                    qrcode_proc_status = QRCODE_STATUS_IDLE;
                    qrcode_process_display_video();
                }
            }
        }
    }
}
