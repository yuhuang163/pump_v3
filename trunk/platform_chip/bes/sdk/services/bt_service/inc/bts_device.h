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
#ifndef __BTS_DEVICE_H__
#define __BTS_DEVICE_H__

#include "bts_status.h"
#include "bts_defines.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void bts_local_dev_t;
typedef void bts_peer_dev_t;

typedef enum bts_device_event {
    BTS_DEV_EVENT_CONNECT_IND = 0,
} bts_device_event_t;

typedef struct bts_device_event_param {
    union {
        struct {
            bts_dev_addr_t addr;
        } connect_ind;
    } p;
} bts_device_event_param_t;

typedef bts_status_t (*bts_device_callback_t)(bts_device_event_t event, bts_device_event_param_t *param);

bts_local_dev_t *bts_device_get_local_dev(void);
bts_status_t bts_device_register_callback(bts_local_dev_t *local_dev, bts_device_callback_t cb);

bts_peer_dev_t *bts_device_get_peer_dev(bts_dev_addr_t *addr);
bts_dev_addr_t *bts_device_get_local_addr(bts_local_dev_t *local_dev);
bts_dev_addr_t *bts_device_get_peer_addr(bts_peer_dev_t *peer_dev);

bts_status_t bts_device_set_accessible_mode(bts_local_dev_t *local_dev, bts_accessible_mode_t access_mode);

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_DEVICE_H__ */