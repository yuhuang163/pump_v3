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

// Implement includes
#include "hal_trace.h"

#define CASE_ENUM(e) case e: return "["#e"]";

// Implement declare
typedef struct __bts_hfp_hf {
	int idle;
	void *priv;
	int device_id;
	bts_dev_addr_t addr;
} __bts_hfp_hf_t;
#define GET_HFP_HF(out,in) \
	__bts_hfp_hf_t *out = (__bts_hfp_hf_t *)in; \
	(void)out;

#define BTS_HFP_HF_CB(event,param) \
    do { \
        if (hfp_hf_cb != NULL) \
            hfp_hf_cb(event, param); \
    } while (0)

// Implement defines
static bts_hfp_hf_callback_t hfp_hf_cb = NULL;
static __bts_hfp_hf_t hfp_hfs[BT_SERVICE_PEER_DEVICE_NUM_MAX];

// Implement functions
static void dump_instance(void)
{
	int i = 0;
	bts_dev_addr_t *addr = NULL;
	__bts_hfp_hf_t *hfp_hf = NULL;

	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		hfp_hf = &hfp_hfs[i];
		addr = &hfp_hf->addr;
		TRACE(2, "hfp_hf:i=%d,idle=%d,0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
			i, hfp_hf->idle, addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
		TRACE(2, "hfp_hf:i=%d,device_id=%d,priv=0x%p", i, hfp_hf->device_id, hfp_hf->priv);
	}
}

static __bts_hfp_hf_t *find_instance_by_addr(bt_bdaddr_t *addr)
{
	int i = 0;
	__bts_hfp_hf_t *hfp_hf = NULL;

	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		hfp_hf = &hfp_hfs[i];
		if (bts_addr_cmp((bts_dev_addr_t *)addr, &hfp_hf->addr) == 0)
			break;
		else
			hfp_hf = NULL;
	}

	if (hfp_hf == NULL) {
		dump_instance();
	}

	return hfp_hf;
}

//static __bts_hfp_hf_t *find_instance_by_device_id(int device_id)
//{
//	int i = 0;
//	__bts_hfp_hf_t *hfp_hf = NULL;
//
//	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
//		hfp_hf = &hfp_hfs[i];
//		if (device_id == hfp_hf->device_id)
//			break;
//		else
//			hfp_hf = NULL;
//	}
//
//	if (hfp_hf == NULL) {
//		dump_instance();
//	}
//
//	return hfp_hf;
//}

static __bts_hfp_hf_t *find_instance_by_device_priv(void *priv)
{
	int i = 0;
	__bts_hfp_hf_t *hfp_hf = NULL;

	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		hfp_hf = &hfp_hfs[i];
		if (priv == hfp_hf->priv)
			break;
		else
			hfp_hf = NULL;
	}

	if (hfp_hf == NULL) {
		dump_instance();
	}

	return hfp_hf;
}

static __bts_hfp_hf_t *alloc_instance(bt_bdaddr_t *addr, int device_id, void *priv)
{
	int i = 0;
	__bts_hfp_hf_t *hfp_hf = NULL;
	hfp_hf = find_instance_by_addr(addr);

	if (hfp_hf != NULL) {
		ASSERT(hfp_hf==NULL, "bts_hfp_hf:error,found hfp hf instance under same address,device_id=%d", device_id);
	}

	if (hfp_hf == NULL) {
		for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
			hfp_hf = &hfp_hfs[i];
			if (hfp_hf->idle) {
				hfp_hf->idle      = 0;
				hfp_hf->priv      = priv;
				hfp_hf->device_id = device_id;
				memcpy(&hfp_hf->addr, addr, sizeof(bts_dev_addr_t));
				break;
			}
			else {
				hfp_hf = NULL;
			}
		}
	}

	if (hfp_hf == NULL) {
		dump_instance();
	}

	return hfp_hf;
}

static void free_instance(__bts_hfp_hf_t *hfp_hf)
{
	hfp_hf->idle      = 1;
	hfp_hf->priv      = NULL;
	hfp_hf->device_id = -1;
	memset(&hfp_hf->addr, 0, sizeof(bts_dev_addr_t));
}

static void init_instance(void)
{
	int i = 0;
	__bts_hfp_hf_t *hfp_hf = NULL;

	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		hfp_hf = &hfp_hfs[i];
		hfp_hf->idle      = 1;
		hfp_hf->priv      = NULL;
		hfp_hf->device_id = -1;
		memset(&hfp_hf->addr, 0, sizeof(bts_dev_addr_t));
	}
}

static const char *app_hfp_hf_event_2_str(app_hfp_hf_event_t event)
{
	switch (event) {
		CASE_ENUM(APP_HFP_HF_EVENT_SERVICE_CONNECTED)
		CASE_ENUM(APP_HFP_HF_EVENT_SERVICE_DISCONNECTED)
		CASE_ENUM(APP_HFP_HF_EVENT_RING_IND)
		CASE_ENUM(APP_HFP_HF_EVENT_CALLER_ID_IND)
		CASE_ENUM(APP_HFP_HF_EVENT_AUDIO_CONNECTED)
		CASE_ENUM(APP_HFP_HF_EVENT_AUDIO_DISCONNECTED)
	}

	return "[]";
}

static int bts_app_hfp_hf_event_callback(app_hfp_hf_event_t event, app_hfp_hf_event_param_t *param)
{
	bts_dev_addr_t addr;
	__bts_hfp_hf_t *hfp_hf = NULL;
	bts_hfp_hf_event_param_t cb_param;

	TRACE(2, "bts_app_hfp_hf_cb: 0x%x %s", event, app_hfp_hf_event_2_str(event));

	switch (event) {
		case APP_HFP_HF_EVENT_SERVICE_CONNECTED:
		{
			hfp_hf = alloc_instance(param->p.service_connected.addr,
				param->p.service_connected.device_id, param->p.service_connected.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_SERVICE_CONNECTED, instance is NULL!");

			TRACE(2, "APP_HFP_HF_EVENT_SERVICE_CONNECTED:priv=0x%p", param->p.service_connected.channel);

			memcpy(&addr, param->p.service_connected.addr, sizeof(bts_dev_addr_t));

			cb_param.hfp_hf = hfp_hf;
			cb_param.p.service_connected.addr = &addr;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_SERVICE_CONNECTED, &cb_param);
		}
		break;
		case APP_HFP_HF_EVENT_SERVICE_DISCONNECTED:
		{
			hfp_hf = find_instance_by_device_priv(param->p.service_disconnected.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_SERVICE_DISCONNECTED, instance is NULL,channel=0x%p",
				param->p.service_disconnected.channel);

			cb_param.hfp_hf = hfp_hf;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_SERVICE_DISCONNECTED, &cb_param);
			free_instance(hfp_hf);
		}
		break;
		case APP_HFP_HF_EVENT_RING_IND:
		{
			hfp_hf = find_instance_by_device_priv(param->p.ring_ind.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_RING_IND, instance is NULL!");

			cb_param.hfp_hf = hfp_hf;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_RING_IND, &cb_param);
		}
		break;
		case APP_HFP_HF_EVENT_CALLER_ID_IND:
		{
			hfp_hf = find_instance_by_device_priv(param->p.caller_id_ind.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_CALLER_ID_IND, instance is NULL!");

			cb_param.hfp_hf = hfp_hf;
			cb_param.p.caller_id_ind.number     = param->p.caller_id_ind.number;
			cb_param.p.caller_id_ind.number_len = param->p.caller_id_ind.number_len;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_RING_IND, &cb_param);
		}
		break;
		case APP_HFP_HF_EVENT_AUDIO_CONNECTED:
		{
			hfp_hf = find_instance_by_device_priv(param->p.audio_connected.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_AUDIO_CONNECTED, instance is NULL!");

			cb_param.hfp_hf = hfp_hf;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_AUDIO_CONNECTED, &cb_param);
		}
		break;
		case APP_HFP_HF_EVENT_AUDIO_DISCONNECTED:
		{
			hfp_hf = find_instance_by_device_priv(param->p.audio_disconnected.channel);
			ASSERT(hfp_hf!=NULL, "bts_hfp_hf:error,HFP_HF_AUDIO_DISCONNECTED, instance is NULL!");

			cb_param.hfp_hf = hfp_hf;
			BTS_HFP_HF_CB(BTS_HFP_HF_EVENT_AUDIO_DISCONNECTED, &cb_param);
		}
		break;
		default:
		break;
	}

	return 0;
}

// API
bts_status_t bts_hfp_hf_init(void)
{
	init_instance();
	bes_bt_register_hfp_hf_callback(bts_app_hfp_hf_event_callback);
	return BTS_ST_SUCCESS;
}

bts_status_t bts_hfp_hf_register_callback(bts_hfp_hf_callback_t cb)
{
	hfp_hf_cb = cb;
	return BTS_ST_SUCCESS;
}