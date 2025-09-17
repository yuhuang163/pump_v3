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
#ifndef __BES_A2DP_API_H__
#define __BES_A2DP_API_H__
#include "a2dp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BES_BT_A2DP_PROFILE_STATE_CONNECTING = 0x01,
    BES_BT_A2DP_PROFILE_STATE_CONNECTED = 0x02,
    BES_BT_A2DP_PROFILE_STATE_DISCONNECTING = 0x03,
    BES_BT_A2DP_PROFILE_STATE_DISCONNECTED = 0x04
} bes_bt_connect_state_e;

typedef enum
{
    BES_BT_A2DP_STATE_NOT_PLAYING = 0x00,
    BES_BT_A2DP_STATE_PLAYING = 0x01,
} bes_bt_a2dp_play_state_e;

typedef enum
{
    BES_BT_A2DP_EVENT_CONNECTION_STATE = 0x00,
    BES_BT_A2DP_EVENT_PLAY_STATUS = 0x01,
} bes_bt_a2dp_event_e;

typedef enum {
    BES_BT_A2DP_AUDIO_CHANNEL_SELECT_STEREO,
    BES_BT_A2DP_AUDIO_CHANNEL_SELECT_LRMERGE,
    BES_BT_A2DP_AUDIO_CHANNEL_SELECT_LCHNL,
    BES_BT_A2DP_AUDIO_CHANNEL_SELECT_RCHNL,
} BES_BT_A2DP_AUDIO_CHANNEL_SELECT_E;

typedef enum
{
    BES_BT_A2DP_CUSTOM_AF_CTRL_NONE = 0,
    BES_BT_A2DP_CUSTOM_AF_CTRL_START,
    BES_BT_A2DP_CUSTOM_AF_CTRL_STOP,
} bes_bt_a2dp_custom_af_ctrl_e;

typedef void (*bes_bt_a2dp_event_callback)(bes_bt_a2dp_event_e type, uint8_t* addr, uint32_t para);


bt_status_t bes_bt_a2dp_profile_connect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_a2dp_profile_disconnect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_a2dp_open_stream(const bt_bdaddr_t *remote);
bt_a2dp_state_t bes_bt_a2dp_get_state(int device_id);
const uint8_t *bes_bt_a2dp_get_codec_element(int device_id);
void bes_bt_a2dp_set_field(int device_id, BT_A2DP_FIELD_ENUM_T field, uint32_t value);
uint32_t bes_bt_a2dp_get_field(int device_id, BT_A2DP_FIELD_ENUM_T field);
void bes_bt_a2dp_abs_volume_mix_version_handled(int device_id);
bool bes_bt_a2dp_is_initiator(const bt_bdaddr_t *remote);
bool bes_bt_a2dp_has_device_streaming(void);
void bes_bt_a2dp_report_speak_gain(void);
void bes_bt_a2dp_key_handler(uint8_t a2dp_key);
void bes_bt_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume);
void bes_bt_a2dp_send_key_request(uint8_t device_id, uint8_t a2dp_key);
void bes_bt_a2dp_accept_stream_request_command(bt_bdaddr_t* remote, uint8_t transaction, uint8_t signal_id);
void bes_bt_a2dp_stream_open_mock(uint8_t device_id, bt_bdaddr_t *remote);
void bes_bt_a2dp_get_device_codec_info(uint8_t dev_num, uint8_t *codec);
bt_status_t bes_bt_a2dp_set_sink_delay(int device_id, U16 delayMs);
void bes_bt_audio_trigger_a2dp(void);
#if defined(A2DP_LHDC_ON)
bool bes_bt_a2dp_lhdc_get_ext_flags(uint32_t flags);
#endif
#if defined(A2DP_LDAC_ON)
int bes_bt_a2dp_ldac_get_channelmode(void);
int bes_bt_a2dp_ldac_get_sample_rate(void);
#endif
uint8_t bes_bt_a2dp_get_curr_sample_bit(void);
void bes_bt_a2dp_set_current_abs_volume(int device_id, uint8_t volume);
void bes_bt_a2dp_set_local_vol(int device_id, uint8_t local_volume);
uint8_t bes_bt_a2dp_get_local_vol(int device_id);
uint8_t bes_bt_a2dp_get_bt_vol(int device_id);
void bes_bt_a2dp_set_volume(int device_id, uint8_t volume);
uint8_t bes_bt_a2dp_get_abs_vol(int device_id);


void bes_bt_a2dp_state_change_callback_register(bes_bt_a2dp_event_callback cb);

void bes_bt_a2dp_state_change_callback_deregister(void);

void bes_bt_a2dp_set_custom_af_ctrl_state(bes_bt_a2dp_custom_af_ctrl_e state);

bes_bt_a2dp_custom_af_ctrl_e bes_bt_a2dp_get_custom_af_ctrl_state(void);

uint8_t bes_bt_a2dp_local_vol_to_bt_vol(uint8_t localVol);
bool bes_bt_a2dp_has_device_streaming(void);
bool bes_bt_a2dp_is_first_packet_after_stream_start(int device_id);
uint8_t bes_bt_a2dp_get_curr_codec_type(void);
uint8_t bes_bt_a2dp_get_curr_sample_bit(void);

void bes_bt_a2dp_notify_receive_stream_callback_register(void (*cb)(void));

void bes_bt_a2dp_notify_receive_stream_callback_deregister(void);

bool bes_bt_a2dp_is_notify_receive_stream_cb_registed(void);

#ifdef __cplusplus
}
#endif
#endif /* __BES_A2DP_API_H__ */
