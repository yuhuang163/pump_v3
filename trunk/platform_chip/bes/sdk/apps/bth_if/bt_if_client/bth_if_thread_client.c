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

#include <string.h>
#include "plat_types.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_client.h"
#include "bth_if_rpc_comm.h"
#include "app_rpc_api.h"

typedef struct {
    uint16_t                user;
    uint16_t                cmd;
    uint8_t                 params[128];
} __attribute__((packed)) BTH_IF_CLIENT_CALL_MESSAGE_T;

static bth_if_user_rsp_result_callback_t user_result_callback[APP_BTH_IF_USER_QTY] = {NULL};
static uint32_t g_user_eanble_bitmap = 0;
static uint8_t server_alive_status = 0;

#define BTH_IF_CLIENT_THREAD_STACK_SIZE   (1024 * 6)
static osThreadId bth_if_client_thread_tid = NULL;
static void bth_if_client_thread(void const *argument);
osThreadDef(bth_if_client_thread, osPriorityAboveNormal, 1, BTH_IF_CLIENT_THREAD_STACK_SIZE, "bth_if_client");

#define BTH_IF_CLIENT_MAILBOX_MAX (41)
static osMailQId bth_if_client_mailbox = NULL;
osMailQDef (bth_if_client_mailbox, BTH_IF_CLIENT_MAILBOX_MAX, BTH_IF_CLIENT_CALL_MESSAGE_T);

#define     BTH_IF_CLIENT_THREAD_WAIT_CMD_RSP_TIMEROUT_MS      (1000)
static osSemaphoreId _bth_if_client_cmd_msg_semaphore_id = NULL;
osSemaphoreDef(_bth_if_client_cmd_msg_semaphore);

static int bth_if_client_mailbox_put(BTH_IF_CLIENT_CALL_MESSAGE_T *msg_src);
static int _bth_if_thread_client_comm_semap_init(void)
{
    if (_bth_if_client_cmd_msg_semaphore_id == NULL) {
        _bth_if_client_cmd_msg_semaphore_id = osSemaphoreCreate(osSemaphore(_bth_if_client_cmd_msg_semaphore), 0);
    }
    ASSERT(_bth_if_client_cmd_msg_semaphore_id, "create _bth_if_thread_client_comm_semap_init fail!");
    return 0;
}

static int _bth_if_thread_client_comm_wait_done(uint32_t timeout)
{
    int ret = 0;
    if (_bth_if_client_cmd_msg_semaphore_id) {
        ret = osSemaphoreAcquire(_bth_if_client_cmd_msg_semaphore_id, timeout);
    }
    return ret;
}

static void _bth_if_thread_client_comm_set_done(void)
{
    if (_bth_if_client_cmd_msg_semaphore_id) {
        osSemaphoreRelease(_bth_if_client_cmd_msg_semaphore_id);
    }
}

static void _bth_if_thread_client_rpc_process_done(unsigned char *ptr, uint32_t len, uint8_t is_timeout)
{
    if (!is_timeout) {
        if ((ptr != NULL) && (len != 0)) {
        }
    }

    _bth_if_thread_client_comm_set_done();
}

static void _bth_if_thread_client_rpc_recv_handler(unsigned char *ptr, uint32_t len)
{
    app_bth_if_ipc_cmd_msg_info_t *rcv_data = (app_bth_if_ipc_cmd_msg_info_t *)ptr;
    if (rcv_data != NULL) {
    //Notes: push the result rsp msg into client thread to process.
#if (0)
        app_bth_if_user_t user = (app_bth_if_user_t)rcv_data->user;
        if (user_result_callback[user] != NULL) {
            user_result_callback[user](rcv_data->payload.param.context, rcv_data->payload_len-4, NULL, rcv_data->cmd);
        }
#else
        BTH_IF_CLIENT_CALL_MESSAGE_T msg;
        memset(&msg, 0, sizeof(msg));
        msg.user = rcv_data->user;
        msg.cmd = APP_BTH_IF_CORE_RESULT_RSP;
        msg.params[0] = rcv_data->payload_len;
        ASSERT(sizeof(msg.params) >= (rcv_data->payload_len + 4), "[%s] payload len(%d) is too big!!",
                        __func__, rcv_data->payload_len);
        memcpy(&msg.params[4], (uint8_t*)&(rcv_data->payload), rcv_data->payload_len);
        bth_if_client_mailbox_put(&msg);
#endif
    }
}

static int _bth_if_thread_client_rpc_init(void)
{
    _bth_if_thread_client_comm_semap_init();
    app_bth_if_rpc_msg_cmd_client_rcv_cb_register(_bth_if_thread_client_rpc_recv_handler);
    app_bth_if_rpc_msg_cmd_client_rsp_cb_register(_bth_if_thread_client_rpc_process_done);
    return 0;
}

static int bth_if_client_mailbox_put(BTH_IF_CLIENT_CALL_MESSAGE_T *msg_src)
{
    osStatus status = osOK;

    BTH_IF_CLIENT_CALL_MESSAGE_T *msg = (BTH_IF_CLIENT_CALL_MESSAGE_T *)osMailAlloc(bth_if_client_mailbox, 0);

    if (!msg) {
        osEvent evt;
        TRACE(0, "[BTH_IF_CLIENT] osMailAlloc error dump");
        for (uint8_t i = 0; i < BTH_IF_CLIENT_MAILBOX_MAX; i++) {
            evt = osMailGet(bth_if_client_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE(5, "cnt: %d user: %d cmd: %d params: %d, %d",
                      i,
                      ((BTH_IF_CLIENT_CALL_MESSAGE_T *)(evt.value.p))->user,
                      ((BTH_IF_CLIENT_CALL_MESSAGE_T *)(evt.value.p))->cmd,
                      ((BTH_IF_CLIENT_CALL_MESSAGE_T *)(evt.value.p))->params[0],
                      ((BTH_IF_CLIENT_CALL_MESSAGE_T *)(evt.value.p))->params[1]);
            } else {
                TRACE(2, "cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE(0, "osMailAlloc error dump end");
        ASSERT(0, "[BTH_IF_CLIENT] osMailAlloc error");
    } else {
        memcpy(msg, msg_src, sizeof(BTH_IF_CLIENT_CALL_MESSAGE_T));
        status = osMailPut(bth_if_client_mailbox, msg);
        if (osOK != status) {
            TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
        }
    }
    return (int)status;
}

static int bth_if_client_mailbox_get(BTH_IF_CLIENT_CALL_MESSAGE_T **msg)
{
    int ret = 0;
    osEvent evt;
    evt = osMailGet(bth_if_client_mailbox, osWaitForever);

    if (evt.status == osEventMail) {
        *msg = (BTH_IF_CLIENT_CALL_MESSAGE_T *)evt.value.p;
    } else {
        ret = -1;
    }
    return ret;
}

static int bth_if_client_mailbox_free(BTH_IF_CLIENT_CALL_MESSAGE_T *msg)
{
    osStatus status;

    status = osMailFree(bth_if_client_mailbox, msg);
    if (osOK != status) {
        TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
    }
    return (int)status;
}

static void bth_if_client_send_cmd_msg_handler(app_bth_if_user_t user,app_bth_if_cmd_t cmd,
                                                                uint32_t sub_cmd, uint8_t *buf, uint32_t len)
{
    app_bth_if_ipc_cmd_msg_info_t tx_cmd;
    uint32_t send_len = sizeof(app_bth_if_ipc_cmd_msg_info_t) - sizeof(app_bth_if_ipc_cmd_msg_context_info_t) + len;

    memset(&tx_cmd, 0, sizeof(tx_cmd));
    tx_cmd.user = user;
    tx_cmd.cmd = cmd;
    tx_cmd.payload_len = len;

    if (len > 0) {
        tx_cmd.payload.sub_cmd = sub_cmd;
        memcpy(tx_cmd.payload.param.context, buf, len);
    }
    //DUMP8("%02x ", (uint8_t *)&tx_cmd, send_len);
    app_rpc_send_cmd(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, (uint8_t *)&tx_cmd, send_len);
    int ret = _bth_if_thread_client_comm_wait_done(BTH_IF_CLIENT_THREAD_WAIT_CMD_RSP_TIMEROUT_MS);

    TRACE(6, "[%s] user: %d, cmd: %d sub_cmd: %d send_len: %d ret: %d", __func__, user, cmd, sub_cmd, send_len, ret);
}

static int bth_if_client_thread_handler(BTH_IF_CLIENT_CALL_MESSAGE_T *msg_body)
{
    app_bth_if_user_t user = msg_body->user;
    app_bth_if_cmd_t cmd = msg_body->cmd;
    uint8_t *buf = NULL;
    uint32_t len = 0;
    uint32_t sub_cmd = BTH_IF_CORE_COMM_SUBCMD_INVALID;
    uint8_t *param = NULL;
    switch (cmd) {
        case APP_BTH_IF_CORE_CMD_CLOSE: {
            g_user_eanble_bitmap &= ~(0x1 << user);
            bth_if_client_send_cmd_msg_handler(user, cmd, 0, NULL, 0);
        }
        break;
        case APP_BTH_IF_CORE_CMD_OPEN: {
            g_user_eanble_bitmap |= (0x1 << user);
            param = msg_body->params;
            len = *((uint32_t*)param);
            param += 4;
            sub_cmd = *((uint32_t*)param);
            param += 4;
            if (len != 0) {
                buf = (uint8_t *)param;
            }
            bth_if_client_send_cmd_msg_handler(user, cmd, sub_cmd, buf, len);
        }
        break;
        case APP_BTH_IF_CORE_CMD_RESTART:
        break;
        case APP_BTH_IF_CORE_CMD_CTRL: {
            if (g_user_eanble_bitmap & (0x1 << user)) {
                param = msg_body->params;
                len = *((uint32_t*)param);
                param += 4;
                sub_cmd = *((uint32_t*)param);
                param += 4;
                if (len != 0) {
                    buf = (uint8_t *)param;
                }
                bth_if_client_send_cmd_msg_handler(user, cmd, sub_cmd, buf, len);
            }
        }
        break;
        case APP_BTH_IF_CORE_RESULT_RSP: {
            param = msg_body->params;
            len = *((uint32_t*)param);
            param += 4;
            app_bth_if_ipc_cmd_msg_context_info_t *payload_context = (app_bth_if_ipc_cmd_msg_context_info_t *)param;
            if (user_result_callback[user] != NULL) {
                user_result_callback[user](payload_context->param.context, len - 4, NULL, payload_context->sub_cmd);
            }
        }
        break;
        default:
        break;
    }
    return 0;
}

static void bth_if_client_thread(void const *argument)
{
    BTH_IF_CLIENT_CALL_MESSAGE_T *msg_p = NULL;
    while(1) {
        if (!server_alive_status) {
            osDelay(100);
            continue;
        }
        if (!bth_if_client_mailbox_get(&msg_p)) {
            bth_if_client_thread_handler(msg_p);
            bth_if_client_mailbox_free(msg_p);
        }
    }
}

static void app_bth_server_state_monitor_handle(uint8_t alive_status)
{
    TRACE(2, "[%s] alive=%d", __func__, alive_status);
    server_alive_status = alive_status;
    if (server_alive_status) {
        osThreadYield();
    }
}

void app_bth_server_ready_notify(uint8_t ready_flag)
{
    server_alive_status = ready_flag;
    if (server_alive_status) {
        osThreadYield();
    }
}

int32_t app_bth_if_client_init(bth_server_state_monitor_cb* server_state_cb)
{
    g_user_eanble_bitmap = 0;
    _bth_if_thread_client_rpc_init();

    osStatus status;
    if (bth_if_client_mailbox == NULL) {
        bth_if_client_mailbox = osMailCreate(osMailQ(bth_if_client_mailbox), NULL);
        ASSERT(bth_if_client_mailbox != NULL, "[%s] Can not create mailbox", __func__);
    }
    if (bth_if_client_thread_tid != NULL) {
        status = osThreadTerminate(bth_if_client_thread_tid);
        ASSERT(status == osOK, "[%s] Can not create thread. status: %d", __func__, status);
        bth_if_client_thread_tid = NULL;
    }
    bth_if_client_thread_tid = osThreadCreate(osThread(bth_if_client_thread), NULL);
    ASSERT(bth_if_client_thread_tid != NULL, "[%s] Can not create thread", __func__);
    *server_state_cb = app_bth_server_state_monitor_handle;
    return 0;
}

int32_t app_bth_if_thread_client_result_register(app_bth_if_user_t user, bth_if_user_rsp_result_callback_t func)
{
    TRACE(0, "[%s] user: %d", __func__, user);
    ASSERT(user < APP_BTH_IF_USER_QTY, "[%s] user (%d) not suppotted", __FUNCTION__, user);

    user_result_callback[user] = func;
    return 0;
}

int32_t bth_if_thread_client_open(app_bth_if_user_t user, uint32_t callback, uint8_t comm_type)
{
    BTH_IF_CLIENT_CALL_MESSAGE_T msg;

    memset(&msg, 0, sizeof(msg));
    msg.user = user;
    msg.cmd = APP_BTH_IF_CORE_CMD_OPEN;
    msg.params[0] = 4 + sizeof(comm_type) + sizeof(callback);            //payload len
    msg.params[4] = BTH_IF_CORE_COMM_SUBCMD_INIT;                        //sub_cmd, 4bytes
    msg.params[8] = comm_type;
    memcpy(&msg.params[9], (uint8_t*)&callback, 4);
    bth_if_client_mailbox_put(&msg);

    return 0;
}

int32_t bth_if_thread_client_close(app_bth_if_user_t user)
{
    BTH_IF_CLIENT_CALL_MESSAGE_T msg;

    memset(&msg, 0, sizeof(msg));
    msg.user = user;
    msg.cmd = APP_BTH_IF_CORE_CMD_CLOSE;
    bth_if_client_mailbox_put(&msg);

    return 0;
}

int32_t bth_if_thread_client_reset(app_bth_if_user_t user)
{
    BTH_IF_CLIENT_CALL_MESSAGE_T msg;

    memset(&msg, 0, sizeof(msg));
    msg.user = user;
    msg.cmd = APP_BTH_IF_CORE_CMD_RESTART;
    bth_if_client_mailbox_put(&msg);

    return 0;
}

int32_t bth_if_thread_client_ctrl(app_bth_if_user_t user, uint32_t ctrl_cmd, uint8_t *buf, uint32_t payload_len)
{
    BTH_IF_CLIENT_CALL_MESSAGE_T msg;
    ASSERT(sizeof(msg.params) >= sizeof(ctrl_cmd) + sizeof(payload_len) + payload_len,
                "[%s] Invalid len: %d", __func__, payload_len);

    memset(&msg, 0, sizeof(msg));
    msg.user = user;
    msg.cmd = APP_BTH_IF_CORE_CMD_CTRL;
    msg.params[0] = 4 + payload_len;
    msg.params[4] = ctrl_cmd;
    if (payload_len > 0) {
        memcpy(&msg.params[8], buf, payload_len);
    }
    bth_if_client_mailbox_put(&msg);

    return 0;
}

