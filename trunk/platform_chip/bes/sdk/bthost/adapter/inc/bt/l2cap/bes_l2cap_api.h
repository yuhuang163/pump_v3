/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_L2CAP_API_H__
#define __BES_L2CAP_API_H__
#include "btgatt.h"
#ifdef __cplusplus
extern "C" {
#endif

bool bes_bt_l2cap_is_channel_connected(const bt_bdaddr_t *remote, uint8_t psm_mask);

void bes_bt_l2cap_reset_sigid(const bt_bdaddr_t *addr);

uint32_t bes_bt_l2cap_get_rfcomm_session_handle(const bt_bdaddr_t *remote);

bool bes_bt_l2cap_is_dlci_channel_connected(uint32_t session_l2c_handle,uint64_t app_id);

bool bes_bt_gatt_is_enabled(void);

#if defined(__GATT_OVER_BR_EDR__)

void bes_bt_gatt_client_create(const bt_bdaddr_t *remote);

void bes_bt_gatt_add_sdp(uint16_t pServiceUUID, uint16_t startHandle, uint16_t endHandle);

bool bes_bt_gatt_is_connected(uint8_t device_id);

bool bes_bt_gatt_is_connected_by_conidx(uint8_t conidx);

void bes_bt_gatt_disconnect(uint8_t device_id);

BtgattChannel *bes_bt_gatt_get_channel_by_conidx(uint8_t conidx);

void bes_bt_gatt_get_device_address(uint8_t conidx,uint8_t *addr);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __BES_L2CAP_API_H__ */
