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

#ifndef BT_API_DEF_H
#define BT_API_DEF_H

#include "plat_types.h"
#include "bth_if_evt_def.h" // Temporary use
#include "bth_if_cmd_def.h" // Temporary use

namespace Watch {
namespace BlueTooth {

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 * Part Description: This part defines the UI/APP call API interface
 * Data structure required
 * *****************************************************************/

typedef struct {
    uint8_t  addr[6];
} __attribute__((packed)) bd_addr_t;

typedef struct {
    bd_addr_t   bt_addr;            //multipoint support
    uint8_t     ctrl_key;
} __attribute__((packed)) app_a2dp_src_stream_ctrl_cmd_t;

/*******************************************************************
 * Part description: This part defines UI/APP acquisition results
 * Data structure required
 * *****************************************************************/

/*For "bt_api_user_callback"*/

typedef struct {
    bt_property_t          inq_result;
} __attribute__((packed)) BtDiscoverInfo;

typedef struct {
    bd_addr_t               bt_addr;
    bt_status_ui_t          status;
    bt_acl_state_ui_t       state;
} __attribute__((packed)) BtAclConnectInfo;

typedef struct {
    bd_addr_t              bt_addr;
    bt_status_ui_t          status;
    bt_acl_state_ui_t       state;
} __attribute__((packed)) BtAclDisconnectInfo;

typedef struct {
    int number_len;
    char number[20];
    bd_addr_t remote;
} __attribute__((packed)) BtCallerInfo;

typedef struct {
    uint8_t call_state;
    bd_addr_t remote;
} __attribute__((packed)) BtCallStateInfo;

typedef struct {
    uint8_t callsetup_state;
    bd_addr_t remote;
} __attribute__((packed)) BtCallSetupStateInfo;

typedef enum {
    BT_HFP_ANSWER_CALL = 1,
    BT_HFP_HANGUP_CALL,
    BT_HFP_REDIAL_CALL,
} BtHfpCallActionE;

#ifdef __cplusplus
}
#endif

} // namespace BlueTooth
}  // namespace Watch

#endif  // BT_API_DEF_H
