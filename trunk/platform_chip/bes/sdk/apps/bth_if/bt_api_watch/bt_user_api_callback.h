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

#ifndef BT_USER_API_CALLBACK_H
#define BT_USER_API_CALLBACK_H

#include <vector>
#include <memory>
#include "bt_callback_observer.h"
#include "bt_api_def.h"


namespace Watch {
namespace BlueTooth {

using ObserverInterface = std::shared_ptr<CallbackObserverInterface>;

class ApiCallbackInterface {
public:
    virtual void AddObr(ObserverInterface obr) = 0;
    virtual void RemoveObr(ObserverInterface obr) = 0;
    virtual void StorageApiResult(void* result_info,uint32_t sub_cmd) = 0;
    virtual ~ApiCallbackInterface(){}
};

class BtUserApiCallback : public ApiCallbackInterface {
public:
    void AddObr(ObserverInterface obr) override;
    void RemoveObr(ObserverInterface obr) override;
    void StorageApiResult(void* result_info,uint32_t sub_cmd) override;

public:
    static BtUserApiCallback& GetIstance()
    {
        static BtUserApiCallback instance;
        return instance;
    }
    void* GetBtUserApiCallback(uint32_t sub_cmd);

private:
    BtDiscoverInfo* searchResultInfo;
    BtAclConnectInfo* aclConEvInfo;
    BtAclDisconnectInfo* aclDisEvtInfo;
    BtCallerInfo* callerEvtInfo;
    BtCallStateInfo* callStateEvtInfo;
    BtCallSetupStateInfo* callSetupStateEvtInfo;

private:
    std::vector<ObserverInterface> observers;
    BtUserApiCallback(){}
    BtUserApiCallback(const BtUserApiCallback&);
    BtUserApiCallback& operator = (const BtUserApiCallback&);
    ~BtUserApiCallback(){}
};

}  // namespace BlueTooth
}  // namespace Watch

#endif  // BT_USER_API_CALLBACK_H
