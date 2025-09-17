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
#include "cmsis_os.h"
#include "stdlib.h"
#include "string.h"
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "bth_if_evt_def.h"
#include "bth_if_cmd_def.h"
#include "bth_bt_if_client.h"
#include "bth_ble_custom_api.h"

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

//TODO: just a test api now
void bth_ble_start_adv(void)
{
    bool adv_enable = true;

    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_ADV_ENABLE, (uint8_t *)&adv_enable, sizeof(uint8_t));

}

void bth_ble_connection_params_update(uint8_t condix, uint16_t          con_interval, uint16_t con_latency, uint16_t sup_to)
{
    TRACE(1, "%s, con interval %d", __func__, con_interval);
    ble_connection_context_t connection_context;
    connection_context.conidx = condix;
    connection_context.con_interval = (uint16_t)(con_interval / 1.25);
    connection_context.con_latency = con_latency;
    connection_context.sup_to = sup_to;

    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_CON_UPDATE, (uint8_t *)&connection_context, sizeof(ble_connection_context_t));

}

