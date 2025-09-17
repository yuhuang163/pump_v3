/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef AOB_MOBILE_ENABLED
#ifdef BLE_USB_AUDIO_SUPPORT
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_dma.h"
#include "hal_trace.h"
#include "app_trace_rx.h"
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "plat_types.h"
#include "audioflinger.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"
#include "app_ble_usb_audio.h"
#include "app_ble_usb_stream_stm.h"
#include "usb_audio_app.h"
#include "gaf_mobile_media_stream.h"
#include "aob_media_api.h"
#include "aob_mgr_gaf_evt.h"
#include "ble_audio_dbg.h"
#include "gaf_media_sync.h"
#include "gaf_media_pid.h"
#include "lc3_process.h"
#include "gaf_codec_lc3.h"
#include "app_bap_data_path_itf.h"
#include "gaf_media_common.h"
#include "app_gaf_custom_api.h"
#include "ble_audio_ase_stm.h"
#include "app_bt_sync.h"
#include "rwble_config.h"
#include "aob_volume_api.h"
#include "app_ble_tws_sync.h"
#include "gaf_mobile_usb_audio.h"

static void app_ble_usb_start_debounce_handler(void const *param);
osTimerDef (GAF_USB_CAPTURE_START_DEBOUNCE, app_ble_usb_start_debounce_handler);
static osTimerId gaf_mobile_usb_start_debounce_timer = NULL;

//TODO: need a better way to replace USB_CAPTURE_START_DEBOUNCE_TIME_MS
#define USB_CAPTURE_START_DEBOUNCE_TIME_MS          1000

static void app_ble_usb_audio_update_timing_test_handler(void)
{
    CIS_TIMING_CONFIGURATION_T updatedCisTimig;
    updatedCisTimig.m2s_bn = 1,
    updatedCisTimig.m2s_nse = 2;
    updatedCisTimig.s2m_bn = 1,
    updatedCisTimig.s2m_nse = 2,
    updatedCisTimig.frame_cnt_per_sdu = 1,
#ifdef BLE_USB_AUDIO_SUPPORT
    updatedCisTimig.m2s_ft = 2,
    updatedCisTimig.s2m_ft = 2,
#ifdef DYNAMIC_SET_PB_TIME
    updatedCisTimig.m2s_ft = 20,  
#endif
#else
    updatedCisTimig.m2s_ft = 3,
    updatedCisTimig.s2m_ft = 3,
#endif

#ifdef AOB_LOW_LATENCY_MODE
    updatedCisTimig.iso_interval = 4,  // 5ms = 4 * 1.25ms
#else
#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    updatedCisTimig.iso_interval = 6,  // 7.5ms = 6 * 1.25ms
#else
    updatedCisTimig.iso_interval = 8,  // 5ms = 8 * 1.25ms
#endif
#endif
    TRACE(0, "Update USB AUDIO CIS timing:");
    TRACE(0, "m2s_bn %d", updatedCisTimig.m2s_bn);
    TRACE(0, "m2s_nse %d", updatedCisTimig.m2s_nse);
    TRACE(0, "m2s_ft %d", updatedCisTimig.m2s_ft);
    TRACE(0, "s2m_bn %d", updatedCisTimig.s2m_bn);
    TRACE(0, "s2m_nse %d", updatedCisTimig.s2m_nse);
    TRACE(0, "s2m_ft %d", updatedCisTimig.s2m_ft);
    TRACE(0, "frame_cnt_per_sdu %d", updatedCisTimig.frame_cnt_per_sdu);
    TRACE(0, "iso_interval %d", updatedCisTimig.iso_interval);

    app_bap_update_cis_timing(&updatedCisTimig);
}

extern uint8_t aob_media_mobile_get_cur_streaming_ase_lid(uint8_t con_lid, \
    AOB_MGR_DIRECTION_E direction);
extern void aob_media_mobile_release_stream(uint8_t ase_id);

void app_ble_usb_media_ascc_release_stream_test(void)
{
    TRACE(1,"%s",__func__);
    aob_media_mobile_release_stream(0);
    aob_media_mobile_release_stream(1);
}

void app_ble_usb_media_ascc_start_stream_test(void)
{
    TRACE(1,"%s",__func__);
    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start =
    {
        APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 120, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_MEDIA
    };

    for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
        aob_media_mobile_start_stream(&ase_to_start, i, 1);
    }
}

AOB_MGR_STREAM_STATE_E app_ble_usb_media_get_cur_ase_state(void)
{
    uint8_t ase_lid = aob_media_mobile_get_cur_streaming_ase_lid(0, AOB_MGR_DIRECTION_SINK);
    return aob_media_mobile_get_cur_ase_state(ase_lid);
}


/**
 ****************************************************************************************
 * @brief Called when cis establishment,send data to usb
 *
 * @return void
 ****************************************************************************************
 */
static bool app_ble_is_usb_playback_on = false;
static bool app_ble_is_usb_capture_on = false;

static void app_ble_usb_playback_vol_changed(uint32_t level)
{
    for (uint32_t con_lid = 0; con_lid < BLE_AUDIO_CONNECTION_CNT; con_lid++)
    {
        uint8_t ase_lid = app_bap_uc_cli_get_ase_lid(con_lid, APP_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
        if (ase_lid != 0xFF)
        {
            aob_mobile_vol_set_abs(con_lid, level);
        }
    }
}

static void app_ble_usb_start_debounce_handler(void const *param)
{
    if (!app_ble_is_usb_capture_on) {
        app_ble_is_usb_capture_on = true;
        ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_CAPTURE_START, 0, 0);
    }
}

static void app_ble_usb_playback_start(void)
{
    app_ble_is_usb_playback_on = true;
    ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_PLAYBACK_START, 0, 0);
}

static void app_ble_usb_playback_stop(void)
{
    app_ble_is_usb_playback_on = false;
    gaf_mobile_usb_dma_playback_stop();
    ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_PLAYBACK_STOP, 0, 0);
}

static void app_ble_usb_capture_start(void)
{
    LOG_D("%s", __func__);
    osTimerStart(gaf_mobile_usb_start_debounce_timer, USB_CAPTURE_START_DEBOUNCE_TIME_MS);
}

static void app_ble_usb_capture_stop(void)
{
    LOG_D("%s", __func__);
    osTimerStop(gaf_mobile_usb_start_debounce_timer);
    if (app_ble_is_usb_capture_on) {
        app_ble_is_usb_capture_on = false;
        gaf_mobile_usb_dma_capture_stop();
        ble_usb_stream_send_message(ble_usb_get_stream_sm(), USB_REQ_STREAM_CAPTURE_STOP, 0, 0);
    }
}

static const USB_AUDIO_SOURCE_EVENT_CALLBACK_T ble_usb_audio_stream_func_cb_list =
{
    .src_type         = USB_AUDIO_SOURCE_BLE_CIS,
    .data_playback_cb = gaf_mobile_usb_processing_received_data,
    .data_capture_cb = gaf_mobile_usb_feed_data,
    .data_prep_cb = NULL,

    .playback_start_cb = app_ble_usb_playback_start,
    .playback_stop_cb = app_ble_usb_playback_stop,

    .capture_start_cb = app_ble_usb_capture_start,
    .capture_stop_cb = app_ble_usb_capture_stop,

    .playback_vol_change_cb = app_ble_usb_playback_vol_changed,
};

void app_ble_usb_set_leaudio_source()
{
    usb_audio_source_config_init(&ble_usb_audio_stream_func_cb_list);
    TRACE(0, "register le audio as the usb audio success!");
}

USB_AUDIO_SOURCE_EVENT_CALLBACK_T app_ble_usb_get_leaudio_source(void)
{
    return ble_usb_audio_stream_func_cb_list;
}

void app_ble_usb_audio_init(void)
{
    ble_usb_stream_stm_init();
    app_ble_usb_audio_update_timing_test_handler();
    usb_audio_source_config_init(&ble_usb_audio_stream_func_cb_list);

    usb_stream_stm_t* usb_stream_stm_ptr = ble_usb_get_stream_sm();
    if (NULL != usb_stream_stm_ptr)
    {
        ble_usb_stream_stm_startup(usb_stream_stm_ptr);
    }

    if (gaf_mobile_usb_start_debounce_timer == NULL) {
        gaf_mobile_usb_start_debounce_timer = osTimerCreate (osTimer(GAF_USB_CAPTURE_START_DEBOUNCE), osTimerOnce, NULL);
    }
}

#endif
#endif
