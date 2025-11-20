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
#ifndef BT_CALLBACK_OBSERVER_H
#define BT_CALLBACK_OBSERVER_H

#include "plat_types.h"

namespace Watch {
namespace BlueTooth {

class CallbackObserverInterface {
public:
    virtual void ObserverUpdateCallback(uint32_t sub_cmd) = 0;
    virtual ~CallbackObserverInterface(){}
};

class BtDeviceFoundReportObserver : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtDeviceFoundReportObserver(){}
    ~BtDeviceFoundReportObserver(){}
};

class BtAclConnectReportObserver : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtAclConnectReportObserver(){}
    ~BtAclConnectReportObserver(){}
};

class BtAclDisconnectReportObserver : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtAclDisconnectReportObserver(){}
    ~BtAclDisconnectReportObserver(){}
};

class BtHfpCallerInfoReportObserver : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtHfpCallerInfoReportObserver(){}
    ~BtHfpCallerInfoReportObserver(){}
};

class BtHfpCallStateReportObserser : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtHfpCallStateReportObserser(){}
    ~BtHfpCallStateReportObserser(){}
};

class BtHfpCallSetupStateReportObserver : public CallbackObserverInterface {
public:
    void ObserverUpdateCallback(uint32_t sub_cmd) override;
    BtHfpCallSetupStateReportObserver(){}
    ~BtHfpCallSetupStateReportObserver(){}
};

}  // namespace BlueTooth
}  // namespace Watch

#endif  // BT_CALLBACK_OBSERVER_H
