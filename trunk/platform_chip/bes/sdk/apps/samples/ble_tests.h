/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BLE_TESTS_H__
#define __BLE_TESTS_H__

#include "plat_types.h"

void get_ble_addr_test(char* cmdParam, uint32_t cmdParam_len);

void get_ble_name_test(char* cmdParam, uint32_t cmdParam_len);

void set_ble_address_test(char* cmdParam, uint32_t cmdParam_len);

void set_ble_name_test(char* cmdParam, uint32_t cmdParam_len);

void ble_adv_user_register_test(char* cmdParam, uint32_t cmdParam_len);

void ble_audio_close_adv_test(char* cmdParam, uint32_t cmdParam_len);

void ble_audio_open_adv_test(char* cmdParam, uint32_t cmdParam_len);

void ble_set_adv_type_test(char *cmdParam, uint32_t cmdParam_len);

void ble_param_set_adv_interval_test(char *cmdParam, uint32_t cmdParam_len);

void ble_set_adv_txpwr_by_actv_user_test(char *cmdParam, uint32_t cmdParam_len);

void ble_set_adv_txpwr_by_adv_user_test(char *cmdParam, uint32_t cmdParam_len);

void ble_set_all_adv_txpwr_test(char *cmdParam, uint32_t cmdParam_len);

void ble_set_adv_local_addr_by_actv_user_test(char *cmdParam, uint32_t cmdParam_len);

void ble_set_adv_local_addr_by_adv_user_test(char *cmdParam, uint32_t cmdParam_len);

void ble_user_adv_start_test(char *cmdParam, uint32_t cmdParam_len);

void ble_custom_adv_start_test(char *cmdParam, uint32_t cmdParam_len);

void ble_start_scan_test(char *cmdParam, uint32_t cmdParam_len);

void ble_stop_scan_test(char *cmdParam, uint32_t cmdParam_len);

void ble_start_connect_test(char *cmdParam, uint32_t cmdParam_len);

void ble_start_connect_with_init_type_test(char *cmdParam, uint32_t cmdParam_len);

void ble_is_connection_on_by_addr_test(char *cmdParam, uint32_t cmdParam_len);

void ble_is_connection_on_by_conIdx_test(char *cmdParam, uint32_t cmdParam_len);

void ble_cancel_connecting_test(char *cmdParam, uint32_t cmdParam_len);

void ble_start_disconnect_test(char *cmdParam, uint32_t cmdParam_len);

void ble_update_conn_param_mode_test(char *cmdParam, uint32_t cmdParam_len);

void ble_get_conn_param_test(char *cmdParam, uint32_t cmdParam_len);

void aob_dts_configure_test(char *cmdParam, uint32_t cmdParam_len);

void aob_dts_send_data_test(char *cmdParam, uint32_t cmdParam_len);

void aob_dts_disconnect_test(char *cmdParam, uint32_t cmdParam_len);

#ifdef AOB_MOBILE_ENABLED
void aob_dtc_connect_test(char *cmdParam, uint32_t cmdParam_len);

void aob_dtc_send_data_test(char *cmdParam, uint32_t cmdParam_len);

void aob_dtc_disconnect_test(char *cmdParam, uint32_t cmdParam_len);
#endif

#endif /* #ifndef __BLE_TESTS_H__ */
