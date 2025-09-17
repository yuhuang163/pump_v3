
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

/****************************************************************************
 *
 * File Description: this file defines the UI calls client interface which needs
 * data struct.
 *
 ****************************************************************************/

#ifndef __BTH_BT_CUSTOM_API_H__
#define __BTH_BT_CUSTOM_API_H__

/*****************************header include********************************/
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************macro defination*****************************/


/******************************type defination******************************/


/****************************function declaration***************************/

/**
 ****************************************************************************************
 * @brief Register the customer callback function and enable the BT information transfer
 *  between M55 and BTH
 *
 * @param[in] custom_bt_ui_cb      bt event callback
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_enable(void* custom_bt_ui_cb);

/**
 ****************************************************************************************
 * @brief Disable the BT information transfer between M55 and BTH, unregister the customer
 *  callback function
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_disable(void);

/**
 ****************************************************************************************
 * @brief Connect the specified remote device
 *
 * @param[in] address      The Bluetooth address of the remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_conn_fixed_addr(const char *address);

/**
 ****************************************************************************************
 * @brief Disconnect the specified remote device
 *
 * @param[in] address      The Bluetooth address of the remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_disconn_fixed_addr(const char *address);

/**
 ****************************************************************************************
 * @brief End Sniff mode for a Connection_Handle, which is currently in Sniff mode.
 *  Currently there is only one connection by default.
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_stop_sniff(void);

/**
 ****************************************************************************************
 * @brief Place the ACL Baseband connection associated with the specified Connection_Handle
 *  into Sniff mode. Currently there is only one connection by default.
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_enter_sniff(void);

/**
 ****************************************************************************************
 * @brief Scan enable
 *
 * @param[in] discoverable     true: Enable Page Scan and Inquiry Scan, false: disable both
 *
 ****************************************************************************************
 */
bool app_bth_bt_if_client_bt_visible(bool discoverable);

/**
 ****************************************************************************************
 * @brief Send the play command to the connected remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_play(void);

/**
 ****************************************************************************************
 * @brief Send the pause command to the connected remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_pause(void);

/**
 ****************************************************************************************
 * @brief Send the stop command to the connected remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_disconn_current_conn(void);

/**
 ****************************************************************************************
 * @brief Send the skip track command to the connected remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_next_track(void);

/**
 ****************************************************************************************
 * @brief Send the previous track command to the connected remote device
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_previous_track(void);

/**
 ****************************************************************************************
 * @brief Set the name of our device (not the remote device)
 *
 * @param[in] name          String of name want to set
 ****************************************************************************************
 */
void app_bth_bt_if_client_set_local_name( const char *name);

/**
 ****************************************************************************************
 * @brief Get the name of our device (not the remote device)
 *
 * @return Local bluetooth name
 ****************************************************************************************
 */
const char* app_bth_bt_if_client_get_local_name(void);

/**
 ****************************************************************************************
 * @brief Set the volume of a2dp sink
 *
 * @param[in] local_volume          volum value
 *                                  ranges from 0 to 16
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_set_volume(uint8_t local_volume);

/**
 ****************************************************************************************
 * @brief Set the mute of a2dp sink
 *
 * @param[in] enabled          true: mute, false: unmute
 *
 ****************************************************************************************
 */
void app_bth_bt_if_client_a2dp_sink_set_mute ( bool enabled);

/**
 ****************************************************************************************
 * @brief Get the address of remote device
 *
 * @return Local bluetooth address
 ****************************************************************************************
 */
const uint8_t* app_bth_bt_if_client_get_remote_address(void);

/**
 ****************************************************************************************
 * @brief Get the bluetooth version
 *
 * @return Bluetooth version
 ****************************************************************************************
 */
const char* app_bth_bt_if_client_get_bt_version(void);

/**
 ****************************************************************************************
 * @brief Set the address of our device (not the remote device)
 *
 * @param[in] mac          Bluetooth address
 ****************************************************************************************
 */
void app_bth_bt_if_client_set_local_bt_addr(uint8_t *mac);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_IF_CMD_DEFINITION_H__
