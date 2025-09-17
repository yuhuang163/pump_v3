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

#include "bt_service.h"

#include "bts_pack.h"

#include "transport_if.h"
#include "bt_service_server_internal.h"
#include "bts_a2dp_source.h"

static void bts_a2dp_source_server_process_pack(unsigned char *buff, unsigned int buff_len)
{
    struct bts_pack_head *head = NULL;

    head = (struct bts_pack_head *)buff;

    switch (head->code) {
        case BTS_PACK_CODE_A2DP_SOURCE_SEARCH:
        {
            bts_a2dp_source_search();
        }
        break;
        case BTS_PACK_CODE_A2DP_SOURCE_CONNECT:
        {
            struct bts_pack_a2dp_source_connect *pack = (struct bts_pack_a2dp_source_connect *)buff;
            bts_a2dp_source_connect(&pack->addr);
        }
        break;
        default:
        break;
    }
}

static bts_status_t bt_service_server_internal_callback(bt_service_server_internal_event_t event,
                                                bt_service_server_internal_event_param_t *param)
{
    switch (event) {
        case BT_SERVICE_SERVER_INTERNAL_EVENT_DATA_INDICATE:
        {
            bts_a2dp_source_server_process_pack(param->p.data_indicate.buff, param->p.data_indicate.buff_len);
        }
        break;
        default:
        break;
    }

    return BTS_ST_SUCCESS;
}

static bts_status_t bts_transport_send(void *buff, int buff_len)
{
    transport_if_t *transport_if = bt_service_server_internal_get_transport_if();
    transport_if->send(buff, buff_len);
    return BTS_ST_SUCCESS;
}

static bts_status_t bts_a2dp_source_search_result(bts_dev_addr_t *addr, const char *name, int name_len)
{
    struct bts_pack_a2dp_source_search_result pack;

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_SEARCH_RESULT;

    bts_addr_cpy(&pack.addr, addr);
    memcpy(pack.name, name, BTS_PACK_A2DP_SOURCE_SEARCH_RESULT_NAME_LEN_MAX);
    pack.name_len = name_len;

    return bts_transport_send(&pack, sizeof(pack));
}

static bts_status_t bts_a2dp_source_connect_fail(bts_dev_addr_t *addr)
{
    struct bts_pack_a2dp_source_connect_fail pack;

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_CONNECT_FAIL;
    bts_addr_cpy(&pack.addr, addr);

    return bts_transport_send(&pack, sizeof(pack));
}

static bts_status_t bts_a2dp_source_callback(bts_a2dp_source_event_t event, bts_a2dp_source_event_param_t *param)
{
    switch (event) {
        case BTS_A2DP_SOURCE_EVENT_SEARCH_RESULT:
        {
            bts_a2dp_source_search_result(param->p.search_result.addr, param->p.search_result.name, param->p.search_result.name_len);
        }
        break;
        case BTS_A2DP_SOURCE_EVENT_SEARCH_COMPLETE:
        {
        }
        break;
        case BTS_A2DP_SOURCE_EVENT_CONNECT_FAIL:
        {
            bts_a2dp_source_connect_fail(param->p.connect_fail.addr);
        }
        break;
        case BTS_A2DP_SOURCE_EVENT_STREAM_OPEN:
        {
        }
        break;
        case BTS_A2DP_SOURCE_EVENT_STREAM_CLOSE:
        {
        }
        break;
        default:
        {
        }
        break;
    }

    return BTS_ST_SUCCESS;
}

// API
bts_status_t bts_a2dp_source_server_init(void)
{
    // a2dp source callback
    bts_a2dp_source_register_callback(bts_a2dp_source_callback);

    // bt service server callback
    bt_service_server_internal_register_callback(BTS_PACK_CODE_A2DP_SOURCE_G, bt_service_server_internal_callback);
    return BTS_ST_SUCCESS;
}