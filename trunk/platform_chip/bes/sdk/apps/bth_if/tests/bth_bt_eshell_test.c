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

/************************** Test Cases ****************************************/
typedef struct
{
    bool coremark_start;
    uint16_t sys_feq;
} POWER_CONSUMPTION_INFO_FLAG_T;

static const bt_callbacks_t eshell_bt_status_report_if_cb = {
    .size                           = sizeof(bt_callbacks_t),
    .adapter_state_changed_cb       = NULL,
    .adapter_properties_cb          = NULL,
    .remote_device_properties_cb    = NULL,
    .device_found_cb                = NULL,
    .discovery_state_changed_cb     = NULL,
    .pin_request_cb                 = NULL,
    .ssp_request_cb                 = NULL,
    .bond_state_changed_cb          = NULL,
    .acl_state_changed_cb           = NULL,
};

static void eshell_bth_bt_open_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_open_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    uint32_t enable = atoi(argv[1]);
    if (enable) {
        app_bth_bt_if_client_open((void*)&eshell_bt_status_report_if_cb, BTH_IF_COMM_TYPE_RPC);
    } else {
        app_bth_bt_if_client_close();
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_open",
                                "bt enable/disable", eshell_bth_bt_open_test);

static void eshell_bth_bt_search_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_search_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    uint32_t start = atoi(argv[1]);
    if (start) {
        uint32_t search_last_time = 120;
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_SEARCH_DEVICE,
                                                    (uint8_t*)&search_last_time, 4);
    } else {
        app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_STOP_SERACH_DEVICE, NULL, 0);
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_search",
                                "bt_inquiry start/stop", eshell_bth_bt_search_test);

static void eshell_bth_bt_connect_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_connect_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    app_bt_create_conn_cmd_t conn_info;
    eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)conn_info.bt_addr.addr);
    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_CONN_DEVICE,
                                            (uint8_t*)&conn_info, sizeof(app_bt_create_conn_cmd_t));
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_conn",
                                "bt_connect addr[**:**:**:**:**:**]",eshell_bth_bt_connect_test);

static void eshell_bth_bt_sniff_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_sniff_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }

    uint32_t inetrval = (uint32_t)(atoi(argv[1]) / 0.625);
    eshell_putstring_nl("%s, interval %d", __func__, inetrval);

    app_bt_sniff_cmd_t sniff_info;
    sniff_info.minInterval = inetrval;
    sniff_info.maxInterval = inetrval;
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;

    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_START_SNIFF,
                                            (uint8_t*)&sniff_info, sizeof(app_bt_sniff_cmd_t));
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_sniff",
                                "bth_bt enter sniff",eshell_bth_bt_sniff_test);

static void eshell_bth_bt_scan_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_scan_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }

    uint8_t access_mode = atoi(argv[1]);

    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BT_SCAN_ENABLE, &access_mode, sizeof(uint8_t));
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_scan",
                                "bth_bt scan enable",eshell_bth_bt_scan_test);

static void eshell_bth_start_coremark_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_start_coremark_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }

    enum HAL_CMU_FREQ_T freq;
    uint32_t start = atoi(argv[1]);

    if (strncmp(argv[2], "32K", 3) == 0) {
        freq = HAL_CMU_FREQ_32K;
        eshell_putstring("set sys freq to 24M");
    } else if (strncmp(argv[2], "24M", 3) == 0) {
        freq = HAL_CMU_FREQ_24M;
        eshell_putstring("set sys freq to 24M");
    } else if (strncmp(argv[2], "48M", 3) == 0) {
        freq = HAL_CMU_FREQ_48M;
        eshell_putstring("set sys freq to 48M");
    } else if (strncmp(argv[2], "96M", 3) == 0) {
        freq = HAL_CMU_FREQ_96M;
        eshell_putstring("set sys freq to 96M");
    } else {
        eshell_putstring("set sys freq error");
        return;
    }

    POWER_CONSUMPTION_INFO_FLAG_T power_consumption_test;
    power_consumption_test.coremark_start = start;
    power_consumption_test.sys_feq = freq;

    app_bth_bt_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_COREMARK_START, (uint8_t *)&power_consumption_test,
                                                                        sizeof(POWER_CONSUMPTION_INFO_FLAG_T));

}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_start_coremark",
                                "bth start coremark",eshell_bth_start_coremark_test);

#endif
