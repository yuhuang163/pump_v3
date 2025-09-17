/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#ifndef __BTS_PACK_H__
#define __BTS_PACK_H__

#include "bts_defines.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define __STRUCT_PACK__ __attribute__((packed))

// code
#define BTS_PACK_CODE_DEVICE_G                         (0x00)
#define BTS_PACK_CODE_SET_ACCESSIBLE_MODE              (0x01)

#define BTS_PACK_CODE_A2DP_SOURCE_G                    (0x01)
#define BTS_PACK_CODE_A2DP_SOURCE_SEARCH               (0x01)
#define BTS_PACK_CODE_A2DP_SOURCE_SEARCH_RESULT        (0x02)
#define BTS_PACK_CODE_A2DP_SOURCE_CONNECT              (0x03)
#define BTS_PACK_CODE_A2DP_SOURCE_CONNECT_FAIL         (0x04)
#define BTS_PACK_CODE_A2DP_SOURCE_DISCONNECT           (0x05)
#define BTS_PACK_CODE_A2DP_SOURCE_START_STREAM         (0x07)

#define BTS_PACK_CODE_GROUP_NUM                        (0x02)

// pack
struct bts_pack_head {
    unsigned char group;
    unsigned char code;
} __STRUCT_PACK__;

struct bts_pack_set_accessible_mode {
    struct bts_pack_head head;
    bts_accessible_mode_t mode;
} __STRUCT_PACK__;

struct bts_pack_a2dp_source_search {
    struct bts_pack_head head;
} __STRUCT_PACK__;

#define BTS_PACK_A2DP_SOURCE_SEARCH_RESULT_NAME_LEN_MAX (64)

struct bts_pack_a2dp_source_search_result {
    struct bts_pack_head head;
    bts_dev_addr_t addr;
    char name[BTS_PACK_A2DP_SOURCE_SEARCH_RESULT_NAME_LEN_MAX];
    int name_len;
} __STRUCT_PACK__;

struct bts_pack_a2dp_source_connect {
    struct bts_pack_head head;
    bts_dev_addr_t addr;
} __STRUCT_PACK__;

struct bts_pack_a2dp_source_connect_fail {
    struct bts_pack_head head;
    bts_dev_addr_t addr;
} __STRUCT_PACK__;

struct bts_pack_a2dp_source_disconnect {
    struct bts_pack_head head;
    bts_dev_addr_t addr;
} __STRUCT_PACK__;

struct bts_pack_a2dp_source_start_stream {
    struct bts_pack_head head;
    int device_id;
} __STRUCT_PACK__;

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_PACK_H__ */
