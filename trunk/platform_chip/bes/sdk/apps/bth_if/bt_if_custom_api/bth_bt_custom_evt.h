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
 * File Description: this file defines the UI report information data struct.
 * TODO: it needs be redefined.
 *
 ****************************************************************************/

#ifndef __BTH_BT_CUSTOM_EVT_H__
#define __BTH_BT_CUSTOM_EVT_H__
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************macro defination*****************************/

/******************************type defination******************************/
enum BTH_PlayState {
    bth_play_state_stopped = 0,
    bth_play_state_transitioning = 1,
    bth_play_state_playing = 2,
    bth_play_state_paused = 3,
    bth_play_state_seekingFw = 4,
    bth_play_state_seekingRev = 5
};

enum BTH_PlayShuffle {
    bth_play_shuffle_off = 0,
    bth_play_shuffle_on = 1
};

enum BTH_PlayRepeat {
    bth_play_repeat_off = 0,
    bth_play_repeat_one = 1,
    bth_play_repeat_all = 2
};

enum BTH_DeviceState{
    bth_device_state_disconnected = 0,
    bth_device_state_connecting = 1, // Communicating but no profile connected i.e. prompting passkey
    bth_device_state_connected = 2,  // At least one profile is connected
};

typedef struct
{
   enum BTH_PlayState state;

   char title[MAX_BTH_STRING];

   char artist[MAX_BTH_STRING];

   char album[MAX_BTH_STRING];

   int32_t track;

   int64_t duration;

   enum BTH_PlayShuffle shuffle;

   enum BTH_PlayRepeat repeat;
}BTH_NowPlaying;

typedef struct
{
   uint8_t index;

   char name[MAX_BTH_STRING];

   uint8_t address[6];

   enum BTH_DeviceState state;
}BTH_PairedDevice;

// Callbacks -------------------------------------------------------------------
typedef void (*AdapterConnectionCallback)(bool connected);

typedef void (*DiscoverableChangedCallback)(bool discoverable);

typedef void (*DevicePairingRequestCallback) (
       uint32_t pairingCode, const char *devAddr, const char *devName);

typedef void (*DevicePairedCallback)(
       BTH_PairedDevice* bluetoothPairedDevice);

typedef void (*DeviceUnpairedCallback)(
       BTH_PairedDevice* bluetoothPairedDevice);

typedef void (*DeviceModifiedCallback)(
       BTH_PairedDevice* bluetoothPairedDevice);

typedef void (*DeviceConnectedCallback)(
       BTH_PairedDevice* bluetoothPairedDevice);

typedef void (*DeviceDisconnectedCallback)(
       BTH_PairedDevice* bluetoothPairedDevice);

typedef void (*MetadataChangedCallback)(
       BTH_NowPlaying* bluetoothNowPlaying);

typedef void (*PlayTimeChangedCallback)(int64_t position);

typedef void (*VolumeChangedCallback)(uint32_t volume);

typedef void (*MuteChangedCallback)(bool muted);

// Apple MFI RFCOMM Support ---------------------------------------------------------------------
#define SPP_APPLE_UUID "00000000-deca-fade-deca-deafdecacafe"
// Called by the bluetooth stack when the MFI SPP channel is connected
// or disconnected.
typedef void (*MfiConnectedCallback)( bool connected );

// Called by the bluetooth stack when it receives MFI data.
typedef void (*MfiReceiveDataCallback)( uint8_t *data, uint32_t length );

// Android RFCOMM Support ---------------------------------------------------------------------
#define SPP_ANDROID_UUID "e8bca34d-fb9e-4192-ac97-78c088ae7c21"
// Called by the bluetooth stack when it receives SPP data.
typedef void (*AndroidReceiveDataCallback)(uint8_t *data, uint32_t length);


typedef struct
{
    AdapterConnectionCallback connection_changed_cb;
    DiscoverableChangedCallback discoverable_changed_cb;
    DevicePairingRequestCallback devicePairingRequestCB;
    DevicePairedCallback devicePairedCB;
    DeviceUnpairedCallback deviceUnpairedCB;
    DeviceModifiedCallback deviceModifiedCB;
    DeviceConnectedCallback deviceConnectedCB;
    DeviceDisconnectedCallback deviceDisconnectedCB;
    MetadataChangedCallback metadataChangedCB;
    PlayTimeChangedCallback playTimeChangedCB;
    VolumeChangedCallback volumeChangedCB;
    MuteChangedCallback muteChangedCB;
    MfiConnectedCallback mfiConnectedCB;
    MfiReceiveDataCallback mfiReceivedDataCB;
    AndroidReceiveDataCallback androidReceiveDataCB;
} custom_bt_callbacks_t;

/****************************function declaration***************************/

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_IF_EVT_DEFINITION_H__
