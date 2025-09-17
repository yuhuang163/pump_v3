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
#ifdef UTILS_ESHELL_EN
#include "cmsis_os.h"
#include "stdlib.h"
#include "string.h"
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "eshell.h"
#include "bth_if_evt_def.h"
#include "bth_if_cmd_def.h"
#include "bth_bt_if_client.h"
#include "bth_bt_custom_api.h"

/************************** Test Cases ****************************************/
static void eshell_bth_start_conn_fixed_addr_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_start_conn_fixed_addr_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    app_bt_create_conn_cmd_t conn_info;
    eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)conn_info.bt_addr.addr);
    app_bth_bt_if_client_conn_fixed_addr((char *)&conn_info.bt_addr.addr);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "conn_fixed_addr",
                                "connect with fixed address",eshell_bth_start_conn_fixed_addr_test);


static void eshell_bth_start_disconn_fixed_addr_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_start_disconn_fixed_addr_test...");
    if (argc < 2) {
    eshell_putstring_nl("Input parameter invalid!!!");
    return;
    }
    app_bt_cancel_conn_cmd_t conn_info;
    eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)conn_info.bt_addr.addr);
    app_bth_bt_if_client_disconn_fixed_addr((char *)&conn_info.bt_addr.addr);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "disconn_fixed_addr",
                        "disconnect with fixed address",eshell_bth_start_disconn_fixed_addr_test);

static void eshell_app_bth_bt_if_client_stop_sniff_enter_high_power_mode_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_app_bth_bt_if_client_stop_sniff_enter_high_power_mode_test...");
    if (argc < 2) {
    eshell_putstring_nl("Input parameter invalid!!!");
    return;
    }
    app_bth_bt_if_client_stop_sniff();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_bt_high_power_mode",
                        "quit sniff mode",eshell_app_bth_bt_if_client_stop_sniff_enter_high_power_mode_test);

static void eshell_app_bth_bt_if_client_enter_sniff_enter_low_power_mode_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_app_bth_bt_if_client_enter_sniff_enter_low_power_mode_test...");
    if (argc < 2) {
    eshell_putstring_nl("Input parameter invalid!!!");
    return;
    }
    app_bth_bt_if_client_enter_sniff();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_bt_low_power_mode",
                        "enter sniff mode",eshell_app_bth_bt_if_client_enter_sniff_enter_low_power_mode_test);

static void eshell_bth_set_discoverable_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_set_discoverable_test...");
    if (argc < 2) {
    eshell_putstring_nl("Input parameter invalid!!!");
    return;
    }
    if (strncmp(argv[1], "dis", 3) == 0)
    {
        app_bth_bt_if_client_bt_visible(true);
    }
    else
    {
        app_bth_bt_if_client_bt_visible(false);
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_discoverable",
                        "bth_bt pagescan enable",eshell_bth_set_discoverable_test);

static void eshell_bth_a2dp_sink_music_play_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_play_test...");
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PLAY, NULL, 0);
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_play",
                        "bth set a2dp sink music play",eshell_bth_a2dp_sink_music_play_test);


static void eshell_bth_a2dp_sink_music_pause_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_pause_test...");
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PAUSE, NULL, 0);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_pause",
                        "bth set a2dp sink music pause",eshell_bth_a2dp_sink_music_pause_test);


static void eshell_bth_a2dp_sink_music_NextTrack_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_NextTrack_test...");
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_NEXT_TRACK, NULL, 0);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_NextTrack",
                        "bth set a2dp sink music NextTrack",eshell_bth_a2dp_sink_music_NextTrack_test);

static void eshell_bth_a2dp_sink_music_PreviousTrack_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_PreviousTrack_test...");
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_PREVIOUS_TRACK, NULL, 0);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_PreviousTrack",
                        "bth set a2dp sink music PreviousTrack",eshell_bth_a2dp_sink_music_PreviousTrack_test);

static void eshell_bth_a2dp_sink_music_set_volume_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_set_volume_test...");

    uint8_t local_volume = atoi(argv[1]);

    if((local_volume > 16) && (local_volume < 0))
    {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_VOLUME, &local_volume, sizeof(uint8_t));
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_set_volume",
                        "bth set a2dp sink music set volume",eshell_bth_a2dp_sink_music_set_volume_test);

static void eshell_bth_a2dp_sink_music_set_mute_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_set_mute_test...");
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_MUTE, NULL, 0);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_set_mute",
                        "bth set a2dp sink music set mute",eshell_bth_a2dp_sink_music_set_mute_test);


static void eshell_bth_a2dp_sink_music_set_name_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_set_name_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    eshell_putstring_nl((const char *)argv[1]);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_NAME, (uint8_t*)argv[1], strlen(argv[1]));
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_set_name",
                        "bth set a2dp sink music set name,Input string of name",eshell_bth_a2dp_sink_music_set_name_test);

static void eshell_bth_a2dp_sink_music_get_name_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_get_name_test...");
    const char *local_name = app_bth_bt_if_client_get_local_name();
    eshell_putstring_nl(local_name);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_get_name",
                        "bth set a2dp sink music get name",eshell_bth_a2dp_sink_music_get_name_test);

//bth_set_a2dp_sink_music_set_mac_addr [**:**:**:**:**:**]
static void eshell_bth_a2dp_sink_music_set_mac_addr_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_a2dp_sink_music_set_mac_addr_test...");
   if (argc < 2) {
       eshell_putstring_nl("Input parameter invalid!!!");
       return;
   }
   uint8_t addr[6] = {0};
   eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)addr);
   app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_A2DP_SINK_MUSIC_SET_ADDR, addr, 6);
   app_bth_bt_if_client_bt_visible(false);
   app_bth_bt_if_client_bt_visible(true);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_set_a2dp_sink_music_set_mac_addr",
                        "bth_set_a2dp_sink_music_set_mac_addr [**:**:**:**:**:**]",eshell_bth_a2dp_sink_music_set_mac_addr_test);
#endif
