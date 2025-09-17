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
#include "bts_private.h"

#include "bts_pack.h"

// Implement includes
#include "transport_if.h"
#include "bt_service_client.h"

// Implement declare
typedef struct __bts_a2dp_source {
    int idle;
    void *priv;
    int device_id;
    bts_dev_addr_t addr;
} __bts_a2dp_source_t;
#define GET_A2DP_SOURCE(out,in) \
    __bts_a2dp_source_t *out = (__bts_a2dp_source_t *)in; \
    (void)out;

#define BTS_A2DP_SOURCE_CB(event,param) \
    do { \
        if (a2dp_source_cb != NULL) \
            a2dp_source_cb(event, param); \
    } while (0)

// Implement defines
static bts_a2dp_source_callback_t a2dp_source_cb = NULL;
static __bts_a2dp_source_t a2dp_sources[BT_SERVICE_PEER_DEVICE_NUM_MAX];

#if 0
static void dump_instance(void)
{
    int i = 0;
    bts_dev_addr_t *addr = NULL;
    __bts_a2dp_source_t *a2dp_source = NULL;

    (void)addr;
    (void)a2dp_source;

    for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
        a2dp_source = &a2dp_sources[i];
        addr = &a2dp_source->addr;
    }
}

static __bts_a2dp_source_t *find_instance_by_addr(bts_dev_addr_t *addr)
{
    int i = 0;
    __bts_a2dp_source_t *a2dp_source = NULL;

    for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
        a2dp_source = &a2dp_sources[i];
        if (bts_addr_cmp((bts_dev_addr_t *)addr, &a2dp_source->addr) == 0)
            break;
        else
            a2dp_source = NULL;
    }

    if (a2dp_source == NULL) {
        dump_instance();
    }

    return a2dp_source;
}

static __bts_a2dp_source_t *find_instance_by_device_id(int device_id)
{
    int i = 0;
    __bts_a2dp_source_t *a2dp_source = NULL;

    for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
        a2dp_source = &a2dp_sources[i];
        if (device_id == a2dp_source->device_id)
            break;
        else
            a2dp_source = NULL;
    }

    if (a2dp_source == NULL) {
        dump_instance();
    }

    return a2dp_source;
}

static __bts_a2dp_source_t *find_instance_by_device_priv(void *priv)
{
    int i = 0;
    __bts_a2dp_source_t *a2dp_source = NULL;

    for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
        a2dp_source = &a2dp_sources[i];
        if (priv == a2dp_source->priv)
            break;
        else
            a2dp_source = NULL;
    }

    if (a2dp_source == NULL) {
        dump_instance();
    }

    return a2dp_source;
}

static __bts_a2dp_source_t *alloc_instance(bts_dev_addr_t *addr, int device_id, void *priv)
{
    int i = 0;
    __bts_a2dp_source_t *a2dp_source = NULL;
    a2dp_source = find_instance_by_addr(addr);

    if (a2dp_source == NULL) {
        for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
            a2dp_source = &a2dp_sources[i];
            if (a2dp_source->idle) {
                a2dp_source->idle      = 0;
                a2dp_source->priv      = priv;
                a2dp_source->device_id = device_id;
                memcpy(&a2dp_source->addr, addr, sizeof(bts_dev_addr_t));
                break;
            }
            else
                a2dp_source = NULL;
        }
    }

    if (a2dp_source == NULL) {
        dump_instance();
    }

    return a2dp_source;
}

static void free_instance(__bts_a2dp_source_t *a2dp_source)
{
    a2dp_source->idle      = 1;
    a2dp_source->priv      = NULL;
    a2dp_source->device_id = -1;
    memset(&a2dp_source->addr, 0, sizeof(bts_dev_addr_t));
}
#endif

static void init_instance(void)
{
    int i = 0;
    __bts_a2dp_source_t *a2dp_source = NULL;

    for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
        a2dp_source = &a2dp_sources[i];
        a2dp_source->idle      = 1;
        a2dp_source->priv      = NULL;
        a2dp_source->device_id = -1;
        memset(&a2dp_source->addr, 0, sizeof(bts_dev_addr_t));
    }
}

static void distr_bts_a2dp_source_process_pack(unsigned char *buff, unsigned int buff_len)
    {
    bts_dev_addr_t addr;
    __bts_a2dp_source_t *a2dp_source = NULL;
    bts_a2dp_source_event_param_t cb_param;

    struct bts_pack_head *head = NULL;

    head = (struct bts_pack_head *)buff;

    (void)a2dp_source;

    switch (head->code) {
    case BTS_PACK_CODE_A2DP_SOURCE_SEARCH_RESULT: {
        struct bts_pack_a2dp_source_search_result *pack = (struct bts_pack_a2dp_source_search_result *)buff;

        memcpy(&addr, &pack->addr, sizeof(bts_dev_addr_t));

        cb_param.p.search_result.addr     = &addr;
        cb_param.p.search_result.name     = pack->name;
        cb_param.p.search_result.name_len = pack->name_len;
        BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_SEARCH_RESULT, &cb_param);
    }
    break;
    case BTS_PACK_CODE_A2DP_SOURCE_CONNECT_FAIL: {
        struct bts_pack_a2dp_source_connect_fail *pack = (struct bts_pack_a2dp_source_connect_fail*)buff;

        memcpy(&addr, &pack->addr, sizeof(bts_dev_addr_t));

        cb_param.p.connect_fail.addr = &addr;
        BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_CONNECT_FAIL, &cb_param);
    }
    break;
    default:
    break;
    }
}

static bts_status_t bt_service_client_callback(bt_service_client_event_t event,
                                                bt_service_client_event_param_t *param)
{
    switch (event) {
        case DISTR_BT_SERVICE_EVENT_DATA_INDICATE: {
            distr_bts_a2dp_source_process_pack(param->p.data_indicate.buff, param->p.data_indicate.buff_len);
        }
        break;
        default:
        break;
    }

    return BTS_ST_SUCCESS;
}

static bts_status_t bts_transport_send(void *buff, int buff_len)
{
    transport_if_t *transport_if = bt_service_client_get_transport_if();
    transport_if->send(buff, buff_len);
    return BTS_ST_SUCCESS;
}

// API
bts_status_t bts_a2dp_source_init(void)
{
    init_instance();
    bt_service_client_register_callback(BTS_PACK_CODE_A2DP_SOURCE_G, bt_service_client_callback);
    return BTS_ST_SUCCESS;
}

bts_status_t bts_a2dp_source_register_callback(bts_a2dp_source_callback_t cb)
{
    a2dp_source_cb = cb;
    return BTS_ST_SUCCESS;
}

bts_status_t bts_a2dp_source_search(void)
{
    struct bts_pack_a2dp_source_search pack;

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_SEARCH;

    return bts_transport_send(&pack, sizeof(pack));
}

bts_status_t bts_a2dp_source_connect(bts_dev_addr_t *addr)
{
    struct bts_pack_a2dp_source_connect pack;

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_CONNECT;
    bts_addr_cpy(&pack.addr, addr);

    return bts_transport_send(&pack, sizeof(pack));
}

bts_status_t bts_a2dp_source_disconnect(bts_a2dp_source_t *a2dp_source)
{
    struct bts_pack_a2dp_source_disconnect pack;
    GET_A2DP_SOURCE(__a2dp_source,a2dp_source);

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_DISCONNECT;
    bts_addr_cpy(&pack.addr, &__a2dp_source->addr);

    return bts_transport_send(&pack, sizeof(pack));
}

bts_status_t bts_a2dp_source_start_stream(bts_a2dp_source_t *a2dp_source)
{
    struct bts_pack_a2dp_source_start_stream pack;
    GET_A2DP_SOURCE(__a2dp_source,a2dp_source);

    pack.head.group = BTS_PACK_CODE_A2DP_SOURCE_G;
    pack.head.code  = BTS_PACK_CODE_A2DP_SOURCE_START_STREAM;
    pack.device_id  = __a2dp_source->device_id;

    return bts_transport_send(&pack, sizeof(pack));
}

bts_status_t bts_a2dp_source_write_pcm(bts_a2dp_source_t *a2dp_source, unsigned char *pcm, unsigned int pcm_len)
{
    return BTS_ST_FAILED;
}