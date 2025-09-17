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

#ifndef BT_USER_API_H
#define BT_USER_API_H

#include "bt_api_def.h"

namespace Watch {
namespace BlueTooth {

class BtUserApi {
public:
    static BtUserApi& GetIstance()
    {
        static BtUserApi instance;
        return instance;
    }

    void Enable(void);
    void BtConnectDevice(const void *addr);
    void BtDisconnectDevice(const void *addr);
    void BtSearchDevice(bool start,uint32_t search_last_time = 120);
    void BtA2dpSourceConnectInit(uint32_t buf_ptr,uint32_t buf_size = 32 * 1024);
    void BtA2dpSourceStreamCtrl(const bd_addr_t addr,uint8_t key);
    void BtHfpSetCallAciton(uint8_t actn,bd_addr_t current_call_remote_addr);

private:
    BtUserApi(){}
    BtUserApi(const BtUserApi&);
    BtUserApi& operator = (const BtUserApi&);
    ~BtUserApi(){}
};

}  // namespace BlueTooth
}  // namespace Watch

#endif  // BT_USER_API_H
