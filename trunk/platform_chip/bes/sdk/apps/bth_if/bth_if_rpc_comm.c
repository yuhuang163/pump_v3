/*************************************************************************************
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
 ************************************************************************************/

/*************************************************************************************
 *
 * File Description: this file is used to implement communication between Application
 * layer and stack layer on the different core.
 *
 ************************************************************************************/

#include <string.h>
#include "plat_types.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_rpc_comm.h"


static bth_if_rpc_comm_msg_rsp_handler_cb_t bth_if_client_send_cmd_rsp_handler = NULL;
static bth_if_rpc_comm_msg_rcv_handler_cb_t bth_if_server_recv_cmd_procee_handler = NULL;
static void app_mcu_to_bth_bridge_send_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_waiting_rsp(BTH_IF_RPC_COMM_USER_CORE, MCU_TO_BTH_TASK_CMD_BT_IF_COMM, ptr, len);
}

static void app_mcu_to_bth_bridge_recv_cmd_handler(uint8_t* ptr, uint16_t len)
{
    if (bth_if_server_recv_cmd_procee_handler) {
        bth_if_server_recv_cmd_procee_handler(ptr, len);
    }
}

static void app_mcu_to_bth_bridge_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    //TRACE(0, "%s", __func__);
}

static void app_mcu_to_bth_bridge_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Warning %s", __func__);
    if (bth_if_client_send_cmd_rsp_handler) {
        bth_if_client_send_cmd_rsp_handler(ptr, len, true);
    }
}

static void app_mcu_to_bth_bridge_recv_rsp_handler(uint8_t* ptr, uint16_t len)
{
    //TRACE(0, "%s", __func__);
    if (bth_if_client_send_cmd_rsp_handler) {
        bth_if_client_send_cmd_rsp_handler(ptr, len, false);
    }
}

void app_bth_if_rpc_msg_cmd_client_rsp_cb_register(bth_if_rpc_comm_msg_rsp_handler_cb_t handler)
{
    bth_if_client_send_cmd_rsp_handler = handler;
}

void app_bth_if_rpc_msg_cmd_server_rcv_cb_register(bth_if_rpc_comm_msg_rcv_handler_cb_t handler)
{
    bth_if_server_recv_cmd_procee_handler = handler;
}

BTIF_TASK_RPC_CMD_TO_ADD(MCU_TO_BTH_TASK_CMD_BT_IF_COMM,
                        "mcu_to_bth comm",
                        app_mcu_to_bth_bridge_send_cmd_handler,
                        app_mcu_to_bth_bridge_recv_cmd_handler,
                        1000,
                        app_mcu_to_bth_bridge_wait_rsp_timeout,
                        app_mcu_to_bth_bridge_recv_rsp_handler,
                        app_mcu_to_bth_bridge_tx_done_handler);

static bth_if_rpc_comm_msg_rcv_handler_cb_t g_bth_if_client_recv_cmd_procee_handler = NULL;

void app_bth_if_rpc_msg_cmd_client_rcv_cb_register(bth_if_rpc_comm_msg_rcv_handler_cb_t handler)
{
    g_bth_if_client_recv_cmd_procee_handler = handler;
}

static void app_bth_to_mcu_bridge_send_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_no_rsp(BTH_IF_RPC_COMM_USER_CORE, BTH_TO_MCU_TASK_CMD_BT_IF_COMM, ptr, len);
}

static void app_bth_to_mcu_bridge_recv_cmd_handler(uint8_t* ptr, uint16_t len)
{
    if (g_bth_if_client_recv_cmd_procee_handler) {
        g_bth_if_client_recv_cmd_procee_handler(ptr, len);
    }
}

BTIF_TASK_RPC_CMD_TO_ADD(BTH_TO_MCU_TASK_CMD_BT_IF_COMM,
                        "bth_to_mcu comm",
                        app_bth_to_mcu_bridge_send_cmd_handler,
                        app_bth_to_mcu_bridge_recv_cmd_handler,
                        0,
                        NULL,
                        NULL,
                        NULL);