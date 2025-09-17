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
#ifndef __CLASSIC_BT_TESTS_H__
#define __CLASSIC_BT_TESTS_H__

#include "plat_types.h"

#ifdef IBRT
void app_ibrt_if_get_bt_local_address_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_bt_peer_address_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_write_bt_local_address_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_a2dp_state_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_avrcp_state_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_hfp_state_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_hfp_call_status_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_connected_mobile_count_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_mobile_connected_dev_list_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_tws_role_switch_on_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_tws_role_switch_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_nvrecord_get_mobile_addr_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_nvrecord_delete_all_mobile_record_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_nvrecord_get_mobile_paired_dev_list_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_tws_link_connected_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_in_freeman_mode_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_tws_disconnect_request_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_conn_is_tws_in_pairing_state_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_left_side_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_right_side_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_is_unknown_side_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_set_side_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_get_ui_role_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_uirole2str_test(char* cmdParam, uint32_t cmdParam_len);

void app_slave_update_tws_pair_info_test(char* cmdParam, uint32_t cmdParam_len);

void app_master_update_tws_pair_info_test(char* cmdParam, uint32_t cmdParam_len);

void app_master_start_tws_pairing_test(char* cmdParam, uint32_t cmdParam_len);

void app_slave_start_tws_pairing_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_open_box_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_fetch_out_box_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_put_in_box_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_close_box_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_wear_up_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_wear_down_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_pairing_mode_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_freeman_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_event_entry_dump_info_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_customif_ui_tws_switch_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_customif_ui_is_tws_switching_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_register_client_callback_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_prevent_sniff_set_test(char* cmdParam, uint32_t cmdParam_len);

void app_ibrt_if_prevent_sniff_clear_test(char* cmdParam, uint32_t cmdParam_len);
#endif

#endif /* #ifndef __CLASSIC_BT_TESTS_H__ */
