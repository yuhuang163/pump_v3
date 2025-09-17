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
#include "heap_api.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_server.h"
#include "bth_if_rpc_comm.h"
#include "app_rpc_api.h"

static const app_bth_if_ipc_msg_t *bth_if_srv_cbs_table[APP_BTH_IF_USER_QTY] = {NULL};
static uint32_t g_user_status_bitmap = 0;

int32_t bth_if_send_result_to_app(app_bth_if_user_t user, uint8_t *buf, uint32_t len, uint32_t sub_cmd)
{
    app_bth_if_ipc_cmd_msg_info_t tx_cmd;
    uint32_t send_len = sizeof(app_bth_if_ipc_cmd_msg_info_t) - sizeof(app_bth_if_ipc_cmd_msg_context_info_t) + len;

    memset(&tx_cmd, 0, sizeof(tx_cmd));
    tx_cmd.user = user;
    tx_cmd.cmd = sub_cmd;
    tx_cmd.payload_len = len;
    if (len > 0) {
        send_len += sizeof(tx_cmd.payload.sub_cmd);
        tx_cmd.payload_len += sizeof(tx_cmd.payload.sub_cmd);
        tx_cmd.payload.sub_cmd = sub_cmd;
        memcpy(tx_cmd.payload.param.context, buf, len);
    }
    app_rpc_send_cmd(BTH_IF_RPC_COMM_USER_CORE, BTH_TO_MCU_TASK_CMD_BT_IF_COMM, (uint8_t *)&tx_cmd, send_len);
    TRACE(4, "[%s] user: %d, cmd: %d send_len: %d", __func__, user, sub_cmd, send_len);

    return 0;
}

int32_t bth_if_thread_server_register(app_bth_if_user_t user, const app_bth_if_ipc_msg_t* bth_if_handle_table)
{
    TRACE(0, "[%s] user: %d", __func__, user);
    bth_if_srv_cbs_table[user] = bth_if_handle_table;
    return 0;
}

typedef struct {
    uint16_t                        user;
    uint16_t                        cmd;
    uint32_t                        ctrl;
    uint32_t                        ctrl_buf_len;
    uint8_t                         *ctrl_buf;
} __attribute__((packed)) BTH_IF_THREAD_SERVER_IPC_MSG_T;

#define APP_BTH_IF_THREAD_MAIL_MAX                 (20)
static osMailQId app_bth_if_thread_mailbox = NULL;
osMailQDef (app_bth_if_thread_mailbox, APP_BTH_IF_THREAD_MAIL_MAX, BTH_IF_THREAD_SERVER_IPC_MSG_T);

// Mailbox Heap
static heap_handle_t app_bth_if_mailbox_heap = NULL;
static uint8_t app_bth_if_mailbox_heap_pool[1024] = {0};

// Thread
#define BTH_IF_THREAD_STACK_SIZE    (1024 * 3)
static osThreadId bth_if_thread_tid = NULL;
static void bth_if_thread(void const *argument);
osThreadDef(bth_if_thread, osPriorityNormal, 1, BTH_IF_THREAD_STACK_SIZE, "app_bth_if");

static uint32_t bth_if_mailbox_cnt(void)
{
    return osMailGetCount(app_bth_if_thread_mailbox);
}

static void bth_if_mailbox_heap_init(void)
{
    app_bth_if_mailbox_heap = heap_register(app_bth_if_mailbox_heap_pool,
                                                    sizeof(app_bth_if_mailbox_heap_pool));
}

static void *bth_if_mailbox_heap_malloc(uint32_t size)
{
    void *ptr = NULL;
    if (size) {
        ptr = heap_malloc(app_bth_if_mailbox_heap, size);
        ASSERT(ptr, "%s size:%d, mailboxCnt:%d", __func__, size, bth_if_mailbox_cnt());
    }
    return ptr;
}

static void bth_if_mailbox_heap_free(void *rmem)
{
    if (rmem) {
        heap_free(app_bth_if_mailbox_heap, rmem);
    }
}

static int bth_if_mailbox_put(BTH_IF_THREAD_SERVER_IPC_MSG_T *msg_src)
{
    osStatus status = osOK;

    BTH_IF_THREAD_SERVER_IPC_MSG_T *msg = NULL;
    msg = (BTH_IF_THREAD_SERVER_IPC_MSG_T *)osMailAlloc(app_bth_if_thread_mailbox, 0);

    if (!msg) {
        osEvent evt;
        TRACE(0, "[BTH_IF] osMailAlloc error dump");
        for (uint8_t i = 0; i < APP_BTH_IF_THREAD_MAIL_MAX; i++)
        {
            evt = osMailGet(app_bth_if_thread_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE(4, "cnt:%d user:%d param:%d/%p",
                      i,
                      ((BTH_IF_THREAD_SERVER_IPC_MSG_T *)(evt.value.p))->user,
                      ((BTH_IF_THREAD_SERVER_IPC_MSG_T *)(evt.value.p))->cmd,
                      ((BTH_IF_THREAD_SERVER_IPC_MSG_T *)(evt.value.p))->ctrl_buf);
            } else {
                TRACE(2, "cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE(0, "osMailAlloc error dump end");
        ASSERT(0, "[BTH_IF] osMailAlloc error");
    } else {
        msg->user = msg_src->user;
        msg->cmd = msg_src->cmd;
        msg->ctrl_buf = msg_src->ctrl_buf;
        msg->ctrl = msg_src->ctrl;
        msg->ctrl_buf_len = msg_src->ctrl_buf_len;

        status = osMailPut(app_bth_if_thread_mailbox, msg);
        if (osOK != status) {
            TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
        }
    }

    return (int)status;
}

static int bth_if_mailbox_get(BTH_IF_THREAD_SERVER_IPC_MSG_T **msg)
{
    int ret = 0;
    osEvent evt;
    evt = osMailGet(app_bth_if_thread_mailbox, osWaitForever);

    if (evt.status == osEventMail) {
        *msg = (BTH_IF_THREAD_SERVER_IPC_MSG_T *)evt.value.p;
    } else {
        ret = -1;
    }

    return ret;
}

static int bth_if_mailbox_free(BTH_IF_THREAD_SERVER_IPC_MSG_T *msg)
{
    osStatus status;

    bth_if_mailbox_heap_free(msg->ctrl_buf);
    status = osMailFree(app_bth_if_thread_mailbox, msg);
    if (osOK != status) {
        TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
    }

    return (int)status;
}

static uint32_t user_open(app_bth_if_user_t user, void *callback_func_if, uint8_t comm_type)
{
    uint32_t ret = 0;
    TRACE(4, "[%s] user=%d callback_if=0x%x comm_type=%d", __func__, user, (uint32_t)callback_func_if, comm_type);
    if ((bth_if_srv_cbs_table[user] != NULL) && ((g_user_status_bitmap & (0x1 << user)) == 0)) {
        if (bth_if_srv_cbs_table[user]->open != NULL) {
            ret = bth_if_srv_cbs_table[user]->open(callback_func_if, comm_type);
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register open func", __func__, user);
        }
        g_user_status_bitmap = g_user_status_bitmap | (0x1 << user);
    }
    return ret;
}

static uint32_t user_close(app_bth_if_user_t user)
{
    if ((bth_if_srv_cbs_table[user] != NULL) && ((g_user_status_bitmap & (0x1 << user)) != 0)) {
        if (bth_if_srv_cbs_table[user]->close != NULL) {
            bth_if_srv_cbs_table[user]->close();
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register close func", __func__, user);
        }
        g_user_status_bitmap = g_user_status_bitmap & ~(0x1 << user);
    }
    return 0;
}

static uint32_t user_reset(app_bth_if_user_t user)
{
    return 0;
}

static uint32_t user_ctrl(app_bth_if_user_t user, uint32_t ctrl, uint8_t *ptr, uint32_t ptr_len)
{
    if ((bth_if_srv_cbs_table[user] != NULL) && ((g_user_status_bitmap & (0x1 << user)) != 0)) {
        if (bth_if_srv_cbs_table[user]->ctrl != NULL) {
            bth_if_srv_cbs_table[user]->ctrl(ctrl, ptr, ptr_len);
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register ctrl func", __func__, user);
        }
    }
    return 0;
}

static void bth_if_thread(void const *argument)
{
    TRACE(0, "%s", __func__);
    BTH_IF_THREAD_SERVER_IPC_MSG_T *msg_p = NULL;
    while(1) {
        if (!bth_if_mailbox_get(&msg_p) && msg_p) {
            TRACE(4, "[%s] user:%d cmd:%d g_user_status_bitmap:0x%x",
                        __func__, msg_p->user, msg_p->cmd, g_user_status_bitmap);
            if (msg_p->cmd == APP_BTH_IF_CORE_CMD_OPEN) {
                uint8_t  comm_type = msg_p->ctrl_buf[0];
                uint32_t report_cb_ptr = *((uint32_t*)(&msg_p->ctrl_buf[1]));
                user_open(msg_p->user, (void*)report_cb_ptr, comm_type);
                app_rpc_send_cmd_rsp(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, NULL, 0);
            } else if (msg_p->cmd == APP_BTH_IF_CORE_CMD_CLOSE) {
                user_close(msg_p->user);
                app_rpc_send_cmd_rsp(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, NULL, 0);
            } else if (msg_p->cmd == APP_BTH_IF_CORE_CMD_RESTART) {
                user_reset(msg_p->user);
                app_rpc_send_cmd_rsp(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, NULL, 0);
            } else if (msg_p->cmd == APP_BTH_IF_CORE_CMD_CTRL) {
                user_ctrl(msg_p->user, msg_p->ctrl, msg_p->ctrl_buf, msg_p->ctrl_buf_len);
                app_rpc_send_cmd_rsp(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, NULL, 0);
            }
            bth_if_mailbox_free(msg_p);
        }
    }
}

void bth_if_thread_cmd_handle(uint8_t *buf, uint32_t len)
{
    BTH_IF_THREAD_SERVER_IPC_MSG_T msg;
    app_bth_if_ipc_cmd_msg_info_t *rx_cmd = (app_bth_if_ipc_cmd_msg_info_t *)buf;

    TRACE(4, "[%s] user: %d, cmd: %d payload_len: %d", __func__, rx_cmd->user, rx_cmd->cmd, rx_cmd->payload_len);
    //DUMP8("%02x ", buf, len);
    memset(&msg, 0, sizeof(msg));
    msg.user = rx_cmd->user;
    msg.cmd = rx_cmd->cmd;
    if (rx_cmd->payload_len > 0) {
        msg.ctrl = rx_cmd->payload.sub_cmd;
        msg.ctrl_buf_len = rx_cmd->payload_len - sizeof(uint32_t);
        msg.ctrl_buf = (uint8_t *)bth_if_mailbox_heap_malloc(msg.ctrl_buf_len);
        memcpy(msg.ctrl_buf, rx_cmd->payload.param.context, msg.ctrl_buf_len);
    } else {
        msg.ctrl = 0;
        msg.ctrl_buf_len = 0;
        msg.ctrl_buf = NULL;
    }
    bth_if_mailbox_put(&msg);
}

int32_t bth_if_thread_server_init(void)
{
    g_user_status_bitmap = 0;
    bth_if_mailbox_heap_init();
    app_bth_if_rpc_msg_cmd_server_rcv_cb_register(bth_if_thread_cmd_handle);
    app_bth_if_thread_mailbox = osMailCreate(osMailQ(app_bth_if_thread_mailbox), NULL);
    ASSERT(app_bth_if_thread_mailbox != NULL, "[%s] Can not create mailbox", __func__);

    bth_if_thread_tid = osThreadCreate(osThread(bth_if_thread), NULL);
    return 0;
}