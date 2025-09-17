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
#include "bt_if.h"
#include "bth_if_evt_def.h"
#include "bth_if_cmd_def.h"
#include "bth_bt_if_client.h"
#include "bth_bt_custom_api.h"

/*********************external function declaration*************************/
extern bt_dev_addr_t current_bt_connect_addr;
extern bt_bdname_t local_bt_device_name;

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
void app_bth_bt_if_client_conn_fixed_addr(const char *address)
{
    app_bt_create_conn_cmd_t conn_info;
    memcpy(&conn_info.bt_addr, address, 6);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_CONN_FIXED_ADDR_DEVICE,
                                            (uint8_t*)&conn_info, sizeof(app_bt_create_conn_cmd_t));
}

void app_bth_bt_if_client_disconn_fixed_addr(const char *address)
{
    app_bt_cancel_conn_cmd_t conn_info;
    memcpy(&conn_info.bt_addr, address, 6);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_DISCONN_FIXED_ADDR_DEVICE,
                                            (uint8_t*)&conn_info, sizeof(app_bt_cancel_conn_cmd_t));
}

void app_bth_bt_if_client_stop_sniff(void)
{
    app_bt_sniff_cmd_t sniff_info;
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SNIFF,
                                            (uint8_t*)&sniff_info, sizeof(app_bt_sniff_cmd_t));
}

void app_bth_bt_if_client_enter_sniff(void)
{
    app_bt_sniff_cmd_t sniff_info;
    sniff_info.minInterval = 800;
    sniff_info.maxInterval = 800;
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_SNIFF,
                                            (uint8_t*)&sniff_info, sizeof(app_bt_sniff_cmd_t));
}

bool app_bth_bt_if_client_bt_visible(bool discoverable)
{
    if (discoverable)
    {
        uint8_t access_mode = BTIF_BAM_GENERAL_ACCESSIBLE;
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_SCAN_ENABLE, &access_mode, sizeof(uint8_t));
    }
    else
    {
        uint8_t access_mode = BTIF_BAM_NOT_ACCESSIBLE;
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_SCAN_ENABLE, &access_mode, sizeof(uint8_t));
    }
    return(true);
}

void app_bth_bt_if_client_a2dp_sink_play(void)
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PLAY, NULL, 0);
}

void app_bth_bt_if_client_a2dp_sink_pause(void)
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PAUSE, NULL, 0);
}

void app_bth_bt_if_client_a2dp_sink_next_track()
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_NEXT_TRACK, NULL, 0);
}

void app_bth_bt_if_client_a2dp_sink_previous_track()
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PREVIOUS_TRACK, NULL, 0);
}

void app_bth_bt_if_client_a2dp_sink_disconn_current_conn()
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_DISCONN_CURRENT_ADDR_DEVICE, NULL, 0);
}

const char* app_bth_bt_if_client_get_local_name()
{
    return (const char *)&local_bt_device_name;
}

void app_bth_bt_if_client_set_local_name( const char *name)
{
    if (name != NULL)
    {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_NAME, (uint8_t*)name, strlen(name));
    }
}

void app_bth_bt_if_client_a2dp_sink_set_volume(uint8_t local_volume)
{
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_VOLUME, &local_volume, sizeof(uint8_t));
}

void app_bth_bt_if_client_a2dp_sink_set_mute ( bool enabled)
{
    if (enabled == true)
    {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_MUTE, NULL, 0);
    }
}

const uint8_t* app_bth_bt_if_client_get_remote_address()
{
    return (uint8_t*)&current_bt_connect_addr;
}

const char* app_bth_bt_if_client_get_bt_version()
{
    return("5.2");
}

void app_bth_bt_if_client_set_local_bt_addr(uint8_t *mac)
{
    if (mac != NULL)
    {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_ADDR, mac, 6);
    }
}
