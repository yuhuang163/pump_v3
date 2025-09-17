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

#ifndef __BTH_BT_IF_CLIENT_H__
#define __BTH_BT_IF_CLIENT_H__
#include "plat_types.h"
#include "bth_if_evt_def.h"

#ifdef __cplusplus
extern "C" {
#endif

//bt basic
const char* bt_ret2str(uint32_t sub_cmd);
int32_t _btif_client_bt_result_rsp_callback(void *buf, uint32_t len, void *other, uint32_t sub_cmd);
int32_t app_bth_bt_if_client_init(void);
int32_t app_bth_bt_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_bt_if_client_close(void);
int32_t app_bth_bt_if_client_reset(void);
int32_t app_bth_bt_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

//a2dp source
int32_t app_bth_a2dp_if_client_init(void);
int32_t app_bth_a2dp_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_a2dp_if_client_close(void);
int32_t app_bth_a2dp_if_client_reset(void);
int32_t app_bth_a2dp_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

//hfp client
int32_t app_bth_hfp_cli_if_client_init(void);
int32_t app_bth_hfp_cli_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_hfp_cli_if_client_close(void);
int32_t app_bth_hfp_cli_if_client_reset(void);
int32_t app_bth_hfp_cli_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

//ble client
int32_t app_bth_ble_if_client_init(void);
int32_t app_bth_ble_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_ble_if_client_close(void);
int32_t app_bth_ble_if_client_reset(void);
int32_t app_bth_ble_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

void btif_client_register_walkie_talkie_callback(ble_client_walkie_talkie_callback_t *cb);
void btif_client_walkie_talkie_start_capture(void);
void btif_client_walkie_talkie_stop_capture(void);

//gatt client
int32_t app_bth_gatt_if_client_init(void);
int32_t app_bth_gatt_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_gatt_if_client_close(void);
int32_t app_bth_gatt_if_client_reset(void);
int32_t app_bth_gatt_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

//a2dp sink client
int32_t app_bth_a2dp_sink_if_client_init(void);
int32_t app_bth_a2dp_sink_if_client_open(void* ui_callback, uint8_t comm_type);
int32_t app_bth_a2dp_sink_if_client_close(void);
int32_t app_bth_a2dp_sink_if_client_reset(void);
int32_t app_bth_a2dp_sink_if_client_ctrl_cmd(uint32_t ctrl_cmd, uint8_t* payload, uint32_t payload_len);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_BT_IF_CLIENT_H__
