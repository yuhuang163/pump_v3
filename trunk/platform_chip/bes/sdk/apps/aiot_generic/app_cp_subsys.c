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
#ifdef CP_SUBSYS_TEST
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "cp_subsys.h"
#include "cp_subsys_msg.h"
#include "app_utils.h"

enum {
    CP_MSG_DIRECTION_MCU2CP,
    CP_MSG_DIRECTION_CP2MCU,
};

typedef struct {
    uint32_t                direction;
    struct CP_SUBSYS_MSG_T  msg;
} CP_SUBSYS_MSG_APP_T;

// Mailbox
#define APP_CP_SUBSYS_MSG_MAILBOX_MAX           (5)
static osMailQId app_cp_subsys_msg_mailbox = NULL;
osMailQDef (app_cp_subsys_msg_mailbox, APP_CP_SUBSYS_MSG_MAILBOX_MAX, CP_SUBSYS_MSG_APP_T);
static int cp_subsys_mailbox_put(const CP_SUBSYS_MSG_APP_T *msg_src);

// Thread
#define CP_SUBSYS_THREAD_STACK_SIZE    (1024)
static osThreadId cp_subsys_thread_tid = NULL;
static void cp_subsys_thread(void const *argument);
osThreadDef(cp_subsys_thread, osPriorityNormal, 1, CP_SUBSYS_THREAD_STACK_SIZE, "cp_subsys");

static cp_subsys_proc_buf_t cp_msg_buf;
void cp_subsys_process_ctrl(bool start)
{
    CP_SUBSYS_MSG_APP_T cp_msg_send;

    static uint32_t seq_cnt = 0;
    if (start) {
        uint32_t generatedSeed = hal_sys_timer_get();
        srand(generatedSeed);
        for (uint8_t ii = 0; ii < CP_SUBSYS_PROC_BUF_MAX_SIZE; ii++) {
            cp_msg_buf.in_buf[ii] = rand();
        }
        cp_msg_buf.in_size = CP_SUBSYS_PROC_BUF_MAX_SIZE;

        cp_msg_send.direction = CP_MSG_DIRECTION_MCU2CP;
        cp_msg_send.msg.hdr.id = CP_SUBSYS_MSG_ID_DEMO_PROCESS_START;
        cp_msg_send.msg.hdr.reply = 0;
        cp_msg_send.msg.seq = seq_cnt++;
        cp_msg_send.msg.param[0] = &cp_msg_buf;
        cp_msg_send.msg.param[1] = sizeof(cp_msg_buf);
        cp_subsys_mailbox_put(&cp_msg_send);
    } else {
        cp_msg_send.direction = CP_MSG_DIRECTION_MCU2CP;
        cp_msg_send.msg.hdr.id = CP_SUBSYS_MSG_ID_DEMO_PROCESS_STOP;
        cp_msg_send.msg.hdr.reply = 0;
        seq_cnt = 0;
        cp_subsys_mailbox_put(&cp_msg_send);
    }
}

static int cp_subsys_mailbox_put(const CP_SUBSYS_MSG_APP_T *msg_src)
{
    osStatus status = osOK;

    CP_SUBSYS_MSG_APP_T *msg = NULL;
    msg = (CP_SUBSYS_MSG_APP_T *)osMailAlloc(app_cp_subsys_msg_mailbox, 0);
    ASSERT(msg, "CP_SUBSYS: alloc mailbox error");

    memcpy(msg, msg_src, sizeof(CP_SUBSYS_MSG_APP_T));
    status = osMailPut(app_cp_subsys_msg_mailbox, msg);

    return (int)status;
}

static int cp_subsys_mailbox_get(CP_SUBSYS_MSG_APP_T **msg)
{
    int ret = 0;
    osEvent evt;
    evt = osMailGet(app_cp_subsys_msg_mailbox, osWaitForever);

    if (evt.status == osEventMail) {
        *msg = (CP_SUBSYS_MSG_APP_T *)evt.value.p;
    } else {
        ret = -1;
    }
    return ret;
}

static int cp_subsys_mailbox_free(CP_SUBSYS_MSG_APP_T *msg)
{
    osStatus status;

    status = osMailFree(app_cp_subsys_msg_mailbox, msg);
    if (osOK != status) {
        TRACE(2, "CP_SUBSYS: free mailbox error: %d", status);
    }
    return (int)status;
}

static void cp_subsys_thread(void const *argument)
{
    TRACE(1, "%s", __func__);
    CP_SUBSYS_MSG_APP_T *msg_p = NULL;
    int ret;
    uint32_t retry = 0;
    while(1) {
        ret = cp_subsys_mailbox_get(&msg_p);
        if (!ret && msg_p) {
            if (msg_p->direction == CP_MSG_DIRECTION_MCU2CP) {
                retry = 0;
                do {
                    ret = cp_subsys_send(&msg_p->msg, sizeof(struct CP_SUBSYS_MSG_T));
                    if (ret) {
                        osDelay(1);
                    }
                } while (ret && ++retry < 10);
                ASSERT(ret == 0, "%s: Failed to send msg: %d", __func__, ret);
            } else if (msg_p->direction == CP_MSG_DIRECTION_CP2MCU) {
                switch (msg_p->msg.hdr.id) {
                    case CP_SUBSYS_MSG_ID_SET_FREQ:
                    {
                        TRACE(0, "CP_SUBSYS: req sys freq for CP: %u", msg_p->msg.param[0]);
                        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, (enum HAL_CMU_FREQ_T)msg_p->msg.param[0]);
                    }
                    break;
                    case CP_SUBSYS_MSG_ID_DEMO_PROCESS_RET:
                    {
                        uint32_t demo_result = (uint32_t)cp_msg_buf.out_buf[0];
                        TRACE(0, "CP_SUBSYS: process return: 0x%x", demo_result);
                    }
                    break;
                    default:
                    break;
                }
            }
            cp_subsys_mailbox_free(msg_p);
            msg_p = NULL;
        }
    }
}

static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done = false;
static unsigned int rx_handler(const void *data, unsigned int len)
{
    TRACE(2, "[%s] len=%d", __func__, len);
    if (!handshake_done) {
        if (data && (len == (sizeof(handshake_msg) - 1)) && !memcmp(data, handshake_msg, len)) {
            TR_INFO(0, "cp_subsys alive!");
            handshake_done = true;
            return len;
        }
    }

    if (len == sizeof(struct CP_SUBSYS_MSG_T)) {
        const struct CP_SUBSYS_MSG_T *msg_p = (const struct CP_SUBSYS_MSG_T *)data;
        CP_SUBSYS_MSG_APP_T cp_msg_recv;
        cp_msg_recv.direction = CP_MSG_DIRECTION_CP2MCU;
        memcpy((uint8_t*)&cp_msg_recv.msg, (uint8_t*)msg_p, sizeof(struct CP_SUBSYS_MSG_T));
        cp_subsys_mailbox_put(&cp_msg_recv);
    }

    return len;
}

static void tx_handler(const void *data, unsigned int len)
{

}

void cp_subsys_init(void)
{
    int ret;
    TRACE(0, "%s", __func__);

    ret = cp_subsys_open(rx_handler, tx_handler);
    ASSERT(ret == 0, "cp_subsys_open failed: %d", ret);

    app_cp_subsys_msg_mailbox = osMailCreate(osMailQ(app_cp_subsys_msg_mailbox), NULL);
    ASSERT(app_cp_subsys_msg_mailbox != NULL, "cp_subsys create mailbox failure");
    cp_subsys_thread_tid = osThreadCreate(osThread(cp_subsys_thread), NULL);
}
#endif