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

#ifndef __BTH_IF_RPC_COMM_H__
#define __BTH_IF_RPC_COMM_H__
#include "plat_types.h"
#include "app_rpc_api.h"

#if defined(APP_RPC_BTH_M55_EN)
#include "app_dsp_m55.h"
#define BTH_IF_RPC_COMM_USER_CORE      (APP_RPC_CORE_BTH_M55)
#define BTIF_TASK_RPC_CMD_TO_ADD      CORE_BRIDGE_TASK_COMMAND_TO_ADD
#endif

#if defined(APP_RPC_BTH_DSP_EN)
#include "rpc_bth_dsp.h"
#define BTH_IF_RPC_COMM_USER_CORE      (APP_RPC_CORE_BTH_DSP)
#define BTIF_TASK_RPC_CMD_TO_ADD      RPC_BTH_DSP_TASK_CMD_TO_ADD
#endif

#if defined(APP_RPC_MCU_SENSOR_EN)
#include "app_sensor_hub.h"
#define BTH_IF_RPC_COMM_USER_CORE      (APP_RPC_CORE_MCU_SENSOR)
#define BTIF_TASK_RPC_CMD_TO_ADD      CORE_BRIDGE_TASK_COMMAND_TO_ADD
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* BTH interface RPC APIs */
typedef void(*bth_if_rpc_comm_msg_rsp_handler_cb_t)(uint8_t * ptr, uint32_t len, uint8_t is_timeout);
typedef void(*bth_if_rpc_comm_msg_rcv_handler_cb_t)(uint8_t * ptr, uint32_t len);
void app_bth_if_rpc_msg_cmd_client_rsp_cb_register(bth_if_rpc_comm_msg_rsp_handler_cb_t handler);
void app_bth_if_rpc_msg_cmd_client_rcv_cb_register(bth_if_rpc_comm_msg_rcv_handler_cb_t handler);
void app_bth_if_rpc_msg_cmd_server_rcv_cb_register(bth_if_rpc_comm_msg_rcv_handler_cb_t handler);

#ifdef __cplusplus
}
#endif
#endif //end of __BTH_IF_RPC_COMM_H__