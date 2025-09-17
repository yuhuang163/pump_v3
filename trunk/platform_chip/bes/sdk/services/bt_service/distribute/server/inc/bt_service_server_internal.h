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

#ifndef __BT_SERVICE_SERVER_INTERNAL_H__
#define __BT_SERVICE_SERVER_INTERNAL_H__

#include "bts_status.h"
#include "transport_if.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum bt_service_server_internal_event {
    BT_SERVICE_SERVER_INTERNAL_EVENT_DATA_INDICATE = 0,
} bt_service_server_internal_event_t;

typedef struct bt_service_server_internal_event_param {
    union {
        struct {
            unsigned char *buff;
            unsigned int buff_len;
        } data_indicate;
    } p;
} bt_service_server_internal_event_param_t;

typedef bts_status_t (*bt_service_server_internal_callback_t)(bt_service_server_internal_event_t event, bt_service_server_internal_event_param_t *param);

bts_status_t bt_service_server_internal_init(transport_if_t *transport_if);
transport_if_t *bt_service_server_internal_get_transport_if(void);
bts_status_t bt_service_server_internal_register_callback(unsigned char group, bt_service_server_internal_callback_t cb);

#if defined(__cplusplus)
}
#endif

#endif /* __BT_SERVICE_SERVER_INTERNAL_H__ */
