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
#ifdef BT_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "bt_service.h"
#include "bts_private.h"

// Implement includes
#include "hal_trace.h"

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

// Implement functions
static void dump_instance(void)
{
	int i = 0;
	bts_dev_addr_t *addr = NULL;
	__bts_a2dp_source_t *a2dp_source = NULL;

	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		a2dp_source = &a2dp_sources[i];
		addr = &a2dp_source->addr;
		TRACE(2, "a2dp_source:i=%d,idle=%d,0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
			i, a2dp_source->idle, addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
		TRACE(2, "a2dp_source:i=%d,device_id=%d,priv=0x%p", i, a2dp_source->device_id, a2dp_source->priv);
	}
}

static __bts_a2dp_source_t *find_instance_by_addr(bt_bdaddr_t *addr)
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

#if 0
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
#endif

static __bts_a2dp_source_t *alloc_instance(bt_bdaddr_t *addr, int device_id, void *priv)
{
	int i = 0;
	__bts_a2dp_source_t *a2dp_source = NULL;
	a2dp_source = find_instance_by_addr(addr);

	if (a2dp_source != NULL) {
		ASSERT(a2dp_source==NULL, "bts_a2dp_source:error,found a2dp source instance under same address,device_id=%d", device_id);
	}

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

static int bts_a2dp_source_bt_source_event_callback(bt_source_event_t event, bt_source_event_param_t *param)
{
	bts_dev_addr_t addr;
	__bts_a2dp_source_t *a2dp_source = NULL;
	bts_a2dp_source_event_param_t cb_param;
    switch (event) {
        case BT_SOURCE_EVENT_SEARCH_RESULT:
		{
			memcpy(&addr, param->p.search_result.result->addr, sizeof(bts_dev_addr_t));

			cb_param.p.search_result.addr     = &addr;
			cb_param.p.search_result.name     = param->p.search_result.result->name;
			cb_param.p.search_result.name_len = param->p.search_result.result->name_len;
			BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_SEARCH_RESULT, &cb_param);
		}
        break;
        case BT_SOURCE_EVENT_SEARCH_COMPLETE:
		{
			BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_SEARCH_COMPLETE, &cb_param);
		}
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_CONNECT_FAIL:
		{
			memcpy(&addr, param->p.a2dp_source_connect_fail.addr, sizeof(bts_dev_addr_t));

			cb_param.p.connect_fail.addr = &addr;
			BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_CONNECT_FAIL, &cb_param);
		}
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_OPEN:
		{
			a2dp_source = alloc_instance(param->p.a2dp_source_stream_open.addr, param->p.a2dp_source_stream_open.device_id, NULL);
			ASSERT(a2dp_source!=NULL, "bts_a2dp_source:error,A2DP_SOURCE_STREAM_OPEN, instance is NULL!");

			a2dp_source->device_id = param->p.a2dp_source_stream_open.device_id;
			memcpy(&addr, param->p.a2dp_source_stream_open.addr, sizeof(bts_dev_addr_t));

			cb_param.a2dp_source = a2dp_source;
			cb_param.p.stream_open.addr = &addr;
			BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_STREAM_OPEN, &cb_param);
		}
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_CLOSE:
		{
			a2dp_source = find_instance_by_addr(param->p.a2dp_source_stream_close.addr);
			ASSERT(a2dp_source!=NULL, "bts_a2dp_source:error,A2DP_SOURCE_STREAM_CLOSE, instance is NULL!");

			memcpy(&addr, param->p.a2dp_source_stream_close.addr, sizeof(bts_dev_addr_t));

			cb_param.a2dp_source = a2dp_source;
			cb_param.p.stream_close.addr = &addr;
			BTS_A2DP_SOURCE_CB(BTS_A2DP_SOURCE_EVENT_STREAM_CLOSE, &cb_param);

			free_instance(a2dp_source);
		}
        break;
    }
    return 0;
}

// API
bts_status_t bts_a2dp_source_init(void)
{
    init_instance();
    bes_bt_source_register_bt_callback(bts_a2dp_source_bt_source_event_callback);
    return BTS_ST_SUCCESS;
}

bts_status_t bts_a2dp_source_register_callback(bts_a2dp_source_callback_t cb)
{
    a2dp_source_cb = cb;
    return BTS_ST_SUCCESS;
}

bts_status_t bts_a2dp_source_search(void)
{
    bes_bt_source_search_device();
    return BTS_ST_PENDING;
}

bts_status_t bts_a2dp_source_connect(bts_dev_addr_t *addr)
{
    bes_bt_source_reconnect_a2dp_profile((bt_bdaddr_t *)addr);
    return BTS_ST_PENDING;
}

bts_status_t bts_a2dp_source_disconnect(bts_a2dp_source_t *a2dp_source)
{
    return BTS_ST_PENDING;
}

bts_status_t bts_a2dp_source_start_stream(bts_a2dp_source_t *a2dp_source)
{
    GET_A2DP_SOURCE(__a2dp_source,a2dp_source);
    bes_bt_source_start_a2dp_stream(__a2dp_source->device_id);
    return BTS_ST_SUCCESS;
}

bts_status_t bts_a2dp_source_write_pcm(bts_a2dp_source_t *a2dp_source, unsigned char *pcm, unsigned int pcm_len)
{
    bes_bt_source_write_a2dp_pcm_data(pcm, pcm_len);
    return BTS_ST_SUCCESS;
}
#endif