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

#ifndef __BT_SERVICE_CLIENT_H__
#define __BT_SERVICE_CLIENT_H__

#include "bts_status.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum bt_service_client_event {
    DISTR_BT_SERVICE_EVENT_DATA_INDICATE = 0,
} bt_service_client_event_t;

typedef struct bt_service_client_event_param {
    union {
        struct {
            unsigned char *buff;
            unsigned int buff_len;
        } data_indicate;
    } p;
} bt_service_client_event_param_t;

typedef bts_status_t (*bt_service_client_callback_t)(bt_service_client_event_t event, bt_service_client_event_param_t *param);

transport_if_t *bt_service_client_get_transport_if(void);
bts_status_t bt_service_client_register_callback(unsigned char group, bt_service_client_callback_t cb);

#if defined(__cplusplus)
}
#endif

#endif /* __DISTR_BT_SERVICE_H__ */
