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
#include "bt_callback_observer.h"
#include "bt_user_api_callback.h"
#include "bth_if_cmd_def.h"

namespace Watch {
namespace BlueTooth {

void BtDeviceFoundReportObserver::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT);
    }else{
        return;
    }
}

void BtAclConnectReportObserver::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT);
    }else{
        return;
    }
}

void BtAclDisconnectReportObserver::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT);
    }else{
        return;
    }
}

void BtHfpCallerInfoReportObserver::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND);
    }else{
        return;
    }
}

void BtHfpCallStateReportObserser::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE);
    }else{
        return;
    }
}

void BtHfpCallSetupStateReportObserver::ObserverUpdateCallback(uint32_t sub_cmd)
{
    if(sub_cmd == BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE){
        BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE);
    }else{
        return;
    }
}

}  // namespace BlueTooth
}  // namespace Watch
