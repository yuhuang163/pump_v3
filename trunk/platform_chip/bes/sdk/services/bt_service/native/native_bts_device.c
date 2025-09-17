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

#include "bluetooth_bt_api.h"
#include "bts_private.h"
#include "bts_status.h"
#include "bts_defines.h"
#include "bts_utils.h"
#include "bts_device.h"

// Implement declare
typedef struct __bts_local_dev {
	bts_dev_addr_t addr;
	bts_device_callback_t cb;
} __bts_local_dev_t;
#define GET_LOCAL_DEV(out,in) \
	__bts_local_dev_t *out = (__bts_local_dev_t *)in; \
	(void)out;

typedef struct __bts_peer_dev {
	bts_dev_addr_t addr;
} __bts_peer_dev_t;
#define GET_PEER_DEV(out,in) \
	__bts_peer_dev_t *out = (__bts_peer_dev_t *)in; \
	(void)out;

// Implement defines
static __bts_local_dev_t local_devs;
static __bts_peer_dev_t peer_devs[BT_SERVICE_PEER_DEVICE_NUM_MAX];

// Implement functions

// API
bts_local_dev_t *bts_device_get_local_dev(void)
{
	return &local_devs;
}

bts_status_t bts_device_register_callback(bts_local_dev_t *local_dev, bts_device_callback_t cb)
{
	GET_LOCAL_DEV(__dev,local_dev);
	__dev->cb = cb;
	return BTS_ST_SUCCESS;
}

bts_peer_dev_t *bts_device_get_peer_dev(bts_dev_addr_t *addr)
{
	int i = 0;
	__bts_peer_dev_t *dev = NULL;

	// check local
	for (i = 0; i < BT_SERVICE_PEER_DEVICE_NUM_MAX; ++i) {
		dev = &peer_devs[i];
		if (bts_addr_cmp(addr, &dev->addr) == 0)
			break;
	}

	// check

	return dev;
}

bts_dev_addr_t *bts_device_get_local_addr(bts_local_dev_t *local_dev)
{
	GET_LOCAL_DEV(__dev,local_dev);
	return &__dev->addr;
}

bts_dev_addr_t *bts_device_get_peer_addr(bts_peer_dev_t *peer_dev)
{
	GET_PEER_DEV(__dev,peer_dev);
	return &__dev->addr;
}

bts_status_t bts_device_set_accessible_mode(bts_local_dev_t *local_dev, bts_accessible_mode_t access_mode)
{
	GET_LOCAL_DEV(__dev,local_dev);
	bes_bt_me_write_access_mode(access_mode, true);

	return BTS_ST_PENDING;
}