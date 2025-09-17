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
#include <stdio.h>
#include <stdlib.h>

#include "bts_private.h"
#include "transport_if.h"
#include "bt_service_server_internal.h"
#include "bts_pack.h"
#include "bt_service.h"

#include "bts_a2dp_source_server.h"

static transport_if_t *service_transport_if = NULL;

static bt_service_server_internal_callback_t distr_cbs[BTS_PACK_CODE_GROUP_NUM];

static bts_status_t transport_if_callback(transport_if_event_t event, transport_if_event_param_t *param)
{
    struct bts_pack_head *head = NULL;
    bt_service_server_internal_event_param_t cb_param;

    switch (event) {
        case TRANSPORT_IF_EVENT_DATA_INDICATE:
        {
            head = (struct bts_pack_head *)param->p.data_indicate.data;
            if (distr_cbs[head->group] != NULL) {
                cb_param.p.data_indicate.buff     = param->p.data_indicate.data;
                cb_param.p.data_indicate.buff_len = param->p.data_indicate.data_len;
                distr_cbs[head->group](BT_SERVICE_SERVER_INTERNAL_EVENT_DATA_INDICATE, &cb_param);
            }
        }
        break;
        default:
        break;
    }

    return BTS_ST_SUCCESS;
}

bts_status_t bt_service_server_internal_init(transport_if_t *transport_if)
{
    int i = 0;

    for (i = 0; i < BTS_PACK_CODE_GROUP_NUM; ++i) {
        distr_cbs[i] = NULL;
    }

    service_transport_if = transport_if;

    transport_if->init();
    transport_if->register_callback(transport_if_callback);

    bt_service_init(NULL);

    bts_a2dp_source_server_init();

    return BTS_ST_SUCCESS;
}

transport_if_t *bt_service_server_internal_get_transport_if(void)
{
    return service_transport_if;
}

bts_status_t bt_service_server_internal_register_callback(unsigned char group, bt_service_server_internal_callback_t cb)
{
    distr_cbs[group] = cb;

    return BTS_ST_SUCCESS;
}