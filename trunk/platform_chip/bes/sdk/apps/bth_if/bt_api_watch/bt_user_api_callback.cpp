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

#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <algorithm>
#include "plat_types.h"
#include "hal_trace.h"
#include "eshell.h"
#include "bth_bt_if_client.h"
#include "bth_if_core.h"
#include "bt_user_api_callback.h"
#include "bt_user_api.h"

using namespace Watch::BlueTooth;

namespace Watch {
namespace BlueTooth {

void BtUserApiCallback::AddObr(ObserverInterface obr)
{
    observers.push_back(obr);
}

void BtUserApiCallback::RemoveObr(ObserverInterface obr)
{
    auto pos=std::find(observers.begin(),observers.end(),obr);
    if(pos!=observers.end()) {
        observers.erase(pos);
    }
}

void BtUserApiCallback::StorageApiResult(void* result_info,uint32_t sub_cmd)
{
    if (NULL == result_info) {
        TRACE(1, "[%s]  bad parameter!!! ", __func__);
        return;
    }
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT: {
            searchResultInfo = (BtDiscoverInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT: {
            aclConEvInfo = (BtAclConnectInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT: {
            aclDisEvtInfo = (BtAclDisconnectInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND: {
            callerEvtInfo = (BtCallerInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE: {
            callStateEvtInfo = (BtCallStateInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE: {
            callSetupStateEvtInfo = (BtCallSetupStateInfo*)result_info;
            for(const auto& obs:observers){
                obs->ObserverUpdateCallback(sub_cmd);
            }
        }
        break;
        default:
        break;
    }
}

void* BtUserApiCallback::GetBtUserApiCallback(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_RESULT_BT_SEARCH_DEV_EVT: {
            return searchResultInfo;
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_CONNECT_EVT: {
            return aclConEvInfo;
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT: {
            return aclDisEvtInfo;
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLER_IND: {
            return callerEvtInfo;
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALL_STATE: {
            return callStateEvtInfo;
        }
        break;
        case BTH_IF_CORE_COMM_RESULT_HFP_CALLSETUP_STATE: {
            return callSetupStateEvtInfo;
        }
        break;
        default:
        break;
    }
    return 0;
}

}  // namespace BlueTooth
}  // namespace Watch

