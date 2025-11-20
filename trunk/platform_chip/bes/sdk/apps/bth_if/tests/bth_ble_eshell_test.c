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
static const ble_client_callbacks_t eshell_ble_status_report_if_cb = {
    .size                                   = sizeof(ble_client_callbacks_t),
    .walkie_talkie_receive_stream_state_cb  = NULL,
    .walkie_talkie_nebor_changed_cb         = NULL,
};

static void eshell_bth_ble_open_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_ble_open_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    uint32_t enable = atoi(argv[1]);
    if (enable) {
        app_bth_ble_if_client_open((void *)&eshell_ble_status_report_if_cb, BTH_IF_COMM_TYPE_RPC);
    } else {
        app_bth_ble_if_client_close();
    }
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_ble_open",
                                "ble enable/disable", eshell_bth_ble_open_test);

static void eshell_bth_ble_adv_start_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_ble_adv_start_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }

    uint8_t adv_enable = atoi(argv[1]);

    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_ADV_ENABLE, &adv_enable, sizeof(uint8_t));

}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_ble_adv",
                                "bth_ble_adv start", eshell_bth_ble_adv_start_test);

static void eshell_bth_ble_con_intval_test(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_ble_adv_start_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }

    uint32_t con_intval = (uint32_t)(atoi(argv[1]));
    eshell_putstring_nl("%s, con interval %d", __func__, con_intval);


    app_bth_ble_if_client_ctrl_cmd(BTH_IF_CORE_COMM_SUBCMD_BLE_CON_UPDATE, (uint8_t *)&con_intval, sizeof(uint32_t));

}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_ble_con_intval",
                                "bth_ble con_intval", eshell_bth_ble_con_intval_test);
#endif
