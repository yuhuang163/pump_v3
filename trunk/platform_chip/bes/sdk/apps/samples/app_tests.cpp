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
#include "apps.h"

#include "app_trace_rx.h"
#include "string.h"

#include "hal_tests.h"
#include "system_API_tests.h"
#include "RTOS_tests.h"
#include "audio_tests.h"
#include "media_callback_tests.h"
#include "ble_tests.h"
#include "classic_BT_tests.h"
#include "crash_analysis_tests.h"

typedef void (*app_tests_function_handle_with_param) (char* cmdParam, uint32_t cmdParamLen);
static app_tests_function_handle_with_param find_handle_with_param(unsigned char* buf);
static POSSIBLY_UNUSED uint32_t app_tests_callback(unsigned char *cmd, unsigned int cmd_length);

typedef struct
{
    const char* string;
    app_tests_function_handle_with_param function;
} APP_TESTS_HANDLE_WITH_PARAM_T;

static const APP_TESTS_HANDLE_WITH_PARAM_T app_tests_handle_with_param[]=
{
    // 5.2 Hardware Abstraction Layer Drivers
    {"gpio_wakeup_demo",                            gpio_wakeup_test},
    {"uart1_transmit_demo",                         uart1_demo_test},
    {"gpadc_test",                                  gpadc_test},
    {"test_hw_timer",                               hw_timer_test},

    // 5.4 System API
    {"get_current_ticks",                           get_current_ticks_test},
    {"get_current_ms",                              get_current_ms_test},
    {"current_battery_level",                       get_current_battery_level_test},

    // 5.5 RTOS
    {"os_thread_create_demo",                       os_thread_create_demo_test},
    {"os_event_demo",                               os_event_demo_test},
    {"os_timer_demo",                               os_timer_demo_test},
    {"os_semaphore_demo",                           os_semaphore_demo_test},

    // 5.6 Audio Function Usage Guide
    {"factorymode_audioloop",                       factorymode_audioloop_test},
    {"a2dp_change_jitter_buf",                      a2dp_change_jitter_buf_test},

    // 5.7 Media Callback
    {"prompt_stop_all",                             prompt_stop_all_test},
    {"media_PlayAudio_test",                        media_PlayAudio_test},
    {"media_PlayAudio_locally_test",                media_PlayAudio_locally_test},
    {"media_PlayAudio_remotely_test",               media_PlayAudio_remotely_test},
    {"media_PlayAudio_standalone_test",             media_PlayAudio_standalone_test},
    {"media_PlayAudio_standalone_locally_test",     media_PlayAudio_standalone_locally_test},
    {"media_PlayAudio_standalone_remotely_test",    media_PlayAudio_standalone_remotely_test},
    {"media_PlayAudio_continuous",                  media_PlayAudio_continuous_test},
    {"prompt_finish_callback_register",             app_prompt_finish_callback_register_test},
#ifdef IBRT
    {"ib_send_volumeup",                            app_ibrt_send_volumeup_test},
    {"ib_send_volumedn",                            app_ibrt_send_volumedown_test},
    {"ib_send_setabsvol",                           app_ibrt_send_set_absvolume_test},
    {"ib_adj_local_volup",                          app_ibrt_adjust_local_volumeup_test},
    {"ib_adj_local_voldn",                          app_ibrt_adjust_local_volumedown_test},
#endif

    // 5.8.2 Classic BT
#ifdef IBRT
    {"get_bt_local_address",                        app_ibrt_if_get_bt_local_address_test},
    {"get_bt_peer_address",                         app_ibrt_if_get_bt_peer_address_test},
    {"write_bt_local_address",                      app_ibrt_if_write_bt_local_address_test},
    {"get_a2dp_state_test",                         app_ibrt_if_get_a2dp_state_test},
    {"get_avrcp_state_test",                        app_ibrt_if_get_avrcp_state_test},
    {"get_hfp_state_test",                          app_ibrt_if_get_hfp_state_test},
    {"get_hfp_call_status",                         app_ibrt_if_get_hfp_call_status_test},
    {"connected_mobile_count",                      app_ibrt_if_get_connected_mobile_count_test},
    {"mobile_connected_dev_list",                   app_ibrt_if_get_mobile_connected_dev_list_test},
    {"is_tws_role_switch_on",                       app_ibrt_if_is_tws_role_switch_on_test},
    {"switch_role",                                 app_ibrt_if_tws_role_switch_test},
    {"nvrecord_get_mobile_addr",                    app_ibrt_if_nvrecord_get_mobile_addr_test},
    {"delete_all_mobile_record",                    app_ibrt_if_nvrecord_delete_all_mobile_record_test},
    {"nvrecord_get_mobile_paired_dev_list",         app_ibrt_if_nvrecord_get_mobile_paired_dev_list_test},
    {"is_tws_link_connected",                       app_ibrt_if_is_tws_link_connected_test},
    {"is_in_freeman_mode",                          app_ibrt_if_is_in_freeman_mode_test},
    {"tws_disconnect_request",                      app_ibrt_if_tws_disconnect_request_test},
    {"is_tws_in_pairing_state",                     app_ibrt_conn_is_tws_in_pairing_state_test},
    {"is_left_side",                                app_ibrt_if_is_left_side_test},
    {"is_right_side",                               app_ibrt_if_is_right_side_test},
    {"is_unknown_side",                             app_ibrt_if_is_unknown_side_test},
    {"ibrt_if_set_side",                            app_ibrt_if_set_side_test},
    {"get_ui_role",                                 app_ibrt_if_get_ui_role_test},
    {"ibrt_if_uirole2str",                          app_ibrt_if_uirole2str_test},
    {"slave_update_tws_pair",                       app_slave_update_tws_pair_info_test},
    {"master_update_tws_pair",                      app_master_update_tws_pair_info_test},
    {"master_start_tws_pair",                       app_master_start_tws_pairing_test},
    {"slave_start_tws_pair",                        app_slave_start_tws_pairing_test},
    {"open_box_event_test",                         app_ibrt_if_event_entry_open_box_test},
    {"fetch_out_box_event_test",                    app_ibrt_if_event_entry_fetch_out_box_test},
    {"put_in_box_event_test",                       app_ibrt_if_event_entry_put_in_box_test},
    {"close_box_event_test",                        app_ibrt_if_event_entry_close_box_test},
    {"wear_up_event_test",                          app_ibrt_if_event_entry_wear_up_test},
    {"wear_down_event_test",                        app_ibrt_if_event_entry_wear_down_test},
    {"pairing_mode_test",                           app_ibrt_if_pairing_mode_test},
    {"free_man_test",                               app_ibrt_if_event_entry_freeman_test},
    {"dump_mgr_info_test",                          app_ibrt_if_event_entry_dump_info_test},
    {"ui_tws_switch",                               app_ibrt_customif_ui_tws_switch_test},
    {"check_ui_tws_switch",                         app_ibrt_customif_ui_is_tws_switching_test},
    {"register_client_callback",                    app_ibrt_if_register_client_callback_test},
    {"prevent_sniff_set",                           app_ibrt_if_prevent_sniff_set_test},
    {"prevent_sniff_clear",                         app_ibrt_if_prevent_sniff_clear_test},
#endif

    // 5.8.3 BLE
    {"get_ble_addr",                                get_ble_addr_test},
    {"get_ble_name",                                get_ble_name_test},
    {"set_ble_address",                             set_ble_address_test},
    {"set_ble_name",                                set_ble_name_test},
    {"ble_adv_user_register",                       ble_adv_user_register_test},
    {"close_adv",                                   ble_audio_close_adv_test},
    {"open_adv",                                    ble_audio_open_adv_test},
    {"ble_set_adv_type",                            ble_set_adv_type_test},
    {"ble_param_set_adv_interval",                  ble_param_set_adv_interval_test},
    {"ble_set_adv_txpwr_by_actv_user",              ble_set_adv_txpwr_by_actv_user_test},
    {"ble_set_adv_txpwr_by_adv_user",               ble_set_adv_txpwr_by_adv_user_test},
    {"ble_set_all_adv_txpwr",                       ble_set_all_adv_txpwr_test},
    {"ble_set_adv_local_addr_by_actv_user",         ble_set_adv_local_addr_by_actv_user_test},
    {"ble_set_adv_local_addr_by_adv_user",          ble_set_adv_local_addr_by_adv_user_test},
    {"ble_user_adv_start",                          ble_user_adv_start_test},
    {"ble_custom_adv_start",                        ble_custom_adv_start_test},
    {"ble_start_scan",                              ble_start_scan_test},
    {"ble_stop_scan",                               ble_stop_scan_test},
    {"ble_start_connect",                           ble_start_connect_test},
    {"ble_start_connect_with_init_type",            ble_start_connect_with_init_type_test},
    {"ble_is_connection_on_by_addr",                ble_is_connection_on_by_addr_test},
    {"ble_is_connection_on_by_conIdx",              ble_is_connection_on_by_conIdx_test},
    {"ble_cancel_connecting",                       ble_cancel_connecting_test},
    {"ble_start_disconnect",                        ble_start_disconnect_test},
    {"ble_update_conn_param_mode",                  ble_update_conn_param_mode_test},
    {"ble_get_conn_param",                          ble_get_conn_param_test},
    {"dts_config",                                  aob_dts_configure_test},
    {"dts_send_data",                               aob_dts_send_data_test},
    {"dts_disconnect",                              aob_dts_disconnect_test},
#ifdef AOB_MOBILE_ENABLED
    {"dtc_connect",                                 aob_dtc_connect_test},
    {"dtc_send_data",                               aob_dtc_send_data_test},
    {"dtc_disconnect",                              aob_dtc_disconnect_test},
#endif

    // 5.12.2 crash analysis
    {"data_access_error",                           data_access_error_test},
    {"instruction_bus_error",                       instruction_bus_error_test},
    {"data_bus_error",                              data_bus_error_test},
    {"usage_fault",                                 usage_fault_test},
    {"hard_fault",                                  hard_fault_test},
    {"dead_lock",                                   dead_lock_test},
    {"stack_overflow",                              stack_overflow_test},
};

static app_tests_function_handle_with_param find_handle_with_param(unsigned char* buf)
{
    app_tests_function_handle_with_param p = NULL;
    for(uint32_t i = 0; i < ARRAY_SIZE(app_tests_handle_with_param); i++)
    {
        if (strncmp((char*)buf, app_tests_handle_with_param[i].string,
            strlen(app_tests_handle_with_param[i].string))==0 ||
            strstr(app_tests_handle_with_param[i].string, (char*)buf))
        {
            TRACE(1, "TEST cmd:%s", app_tests_handle_with_param[i].string);
            p = app_tests_handle_with_param[i].function;
            break;
        }
    }
    return p;
}

static uint32_t app_tests_callback(unsigned char *cmd, unsigned int cmd_length)
{
    int param_len = 0;
    char* cmd_param = NULL;
    char* cmd_end = (char *)cmd + cmd_length;

    cmd_param = strstr((char*)cmd, (char*)"|");

    if (cmd_param)
    {
        *cmd_param = '\0';
        cmd_length = cmd_param - (char *)cmd;
        cmd_param += 1;

        param_len = cmd_end - cmd_param;
    }
    else
    {
        param_len = 0;
    }

    app_tests_function_handle_with_param handle_function = find_handle_with_param(cmd);
    if (handle_function)
    {
        handle_function(cmd_param, param_len);
    }
    else
    {
        TRACE(0, "can not find handle function");
    }

    return 0;
}

void app_tests_init(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0, "%s", __FUNCTION__);
    app_trace_rx_register("TEST", app_tests_callback);
#endif
}
