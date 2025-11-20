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
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <algorithm>
#include "plat_types.h"
#include "hal_trace.h"
#include "eshell.h"
#include "bth_if_evt_def.h"
#include "bth_bt_if_client.h"
#include "bth_if_core.h"
#include "bt_user_api_callback.h"
#include "bt_user_api.h"

namespace Watch {
namespace BlueTooth {

static void EshellBtConnectDevice(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_connect_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    const char *param = argv[1];
    app_bt_create_conn_cmd_t conn_info;
    eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)conn_info.bt_addr.addr);
    BtUserApi::GetIstance().Enable();
    auto ConnectObr = std::make_shared<BtAclConnectReportObserver>();
    BtUserApiCallback::GetIstance().AddObr(ConnectObr);//Add Observer
    BtUserApi::GetIstance().BtConnectDevice((const char *)conn_info.bt_addr.addr);//Call API
    BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT);//Actively obtain API result
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_connect_device",
                                "bt_connect addr[**:**:**:**:**:**]",EshellBtConnectDevice);

static void EshellBtDisconnectDevice(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_connect_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    const char *param = argv[1];
    app_bt_create_conn_cmd_t conn_info;
    eshell_param_macstr_get_array(argv[1], 0, (unsigned char *)conn_info.bt_addr.addr);
    BtUserApi::GetIstance().Enable();
    auto DisconnectObr = std::make_shared<BtAclDisconnectReportObserver>();
    BtUserApiCallback::GetIstance().AddObr(DisconnectObr);
    BtUserApi::GetIstance().BtDisconnectDevice((const char *)conn_info.bt_addr.addr);
    BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_disconnect_device",
                                "bt_disconnect addr[**:**:**:**:**:**]",EshellBtDisconnectDevice);

static void EshellBtSearchDevice(int argc, char *argv[])
{
    eshell_putstring_nl("eshell_bth_bt_search_test...");
    if (argc < 2) {
        eshell_putstring_nl("Input parameter invalid!!!");
        return;
    }
    int start = atoi(argv[1]);
    BtUserApi::GetIstance().Enable();
    auto SearchObr = std::make_shared<BtDeviceFoundReportObserver>();
    BtUserApiCallback::GetIstance().AddObr(SearchObr);
    BtUserApi::GetIstance().BtSearchDevice(start);
    BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bth_bt_search_device",
                                "bt_inquiry start/stop-->1/0", EshellBtSearchDevice);

}  // namespace BlueTooth
}  // namespace Watch
#endif