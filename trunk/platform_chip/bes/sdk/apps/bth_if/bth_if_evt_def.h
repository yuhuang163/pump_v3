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

#ifndef __BTH_IF_EVT_DEFINITION_H__
#define __BTH_IF_EVT_DEFINITION_H__
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************ Bluetooth Basic ******************************/
#define MAX_BTH_STRING                   80
#define MAX_BTH_MAC_ADDRESS_STRING_LEN   18  // Format of "XX:XX:XX:XX:XX:XX" = 17

typedef struct
{
    /// 6-byte array address value
    uint8_t addr[6];
} __attribute__((packed)) bt_rawaddr_t;

/** Bluetooth Device Name */
typedef struct
{
    uint8_t name[249];
} __attribute__((packed)) bt_bdname_t;

/** Bluetooth Adapter Visibility Modes*/
typedef enum
{
    BT_SCAN_MODE_NONE,
    BT_SCAN_MODE_CONNECTABLE,
    BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE
} bt_scan_mode_t;

/** Bluetooth Adapter State */
typedef enum
{
    BT_STATE_OFF,
    BT_STATE_ON
} bt_state_t;

/** Bluetooth Error Status */
typedef enum
{
    BT_STATUS_SUCCESS_UI = 0,
    BT_STATUS_FAIL_UI,
    BT_STATUS_NOT_READY_UI,
    BT_STATUS_NOMEM_UI,
    BT_STATUS_BUSY_UI,
    BT_STATUS_DONE_UI, /* request already completed */
    BT_STATUS_UNSUPPORTED_UI,
    BT_STATUS_PARM_INVALID_UI,
    BT_STATUS_UNHANDLED_UI,
    BT_STATUS_AUTH_FAILURE_UI,
    BT_STATUS_RMT_DEV_DOWN_UI,
    BT_STATUS_AUTH_REJECTED_UI
} bt_status_ui_t;

/** Bluetooth PinKey Code */
typedef struct
{
    uint8_t pin[16];
} __attribute__((packed)) bt_pin_code_t;

/** Bluetooth Adapter Discovery state */
typedef enum
{
    BT_DISCOVERY_STOPPED,
    BT_DISCOVERY_STARTED
} bt_discovery_state_t;

/** Bluetooth ACL connection state */
typedef enum
{
    BT_ACL_STATE_CONNECTED,
    BT_ACL_STATE_DISCONNECTED
} bt_acl_state_ui_t;

typedef struct
{
    uint8_t uuid_type; // 2byte/4byte/16byte
    uint8_t uuid_value[16];
} __attribute__((packed)) bt_uuid_t;

/** Bluetooth SDP service record */
typedef struct
{
    bt_uuid_t uuid;
    uint16_t channel;
    char name[256]; // what's the maximum length
} bt_service_record_t;

/** Bluetooth Remote Version info */
typedef struct
{
    int version;
    int sub_ver;
    int manufacturer;
} bt_remote_version_t;

/* Bluetooth Adapter and Remote Device property types */
typedef enum
{
    /* Properties common to both adapter and remote device */
    /**
     * Description - Bluetooth Device Name
     * Access mode - Adapter name can be GET/SET. Remote device can be GET
     * Data type   - bt_bdname_t
     */
    BT_PROPERTY_BDNAME = 0x1,
    /**
     * Description - Bluetooth Device Address
     * Access mode - Only GET.
     * Data type   - bt_rawaddr_t
     */
    BT_PROPERTY_BDADDR,
    /**
     * Description - Bluetooth Service 128-bit UUIDs
     * Access mode - Only GET.
     * Data type   - Array of bt_uuid_t (Array size inferred from property
     *               length).
     */
    BT_PROPERTY_UUIDS,
    /**
     * Description - Bluetooth Class of Device as found in Assigned Numbers
     * Access mode - Only GET.
     * Data type   - uint32_t.
     */
    BT_PROPERTY_CLASS_OF_DEVICE,
    /**
     * Description - Device Type - BREDR, BLE or DUAL Mode
     * Access mode - Only GET.
     * Data type   - bt_device_type_t
     */
    BT_PROPERTY_TYPE_OF_DEVICE,
    /**
     * Description - Bluetooth Service Record
     * Access mode - Only GET.
     * Data type   - bt_service_record_t
     */
    BT_PROPERTY_SERVICE_RECORD,
    /* Properties unique to adapter */
    /**
     * Description - Bluetooth Adapter scan mode
     * Access mode - GET and SET
     * Data type   - bt_scan_mode_t.
     */
    BT_PROPERTY_ADAPTER_SCAN_MODE,
    /**
     * Description - List of bonded devices
     * Access mode - Only GET.
     * Data type   - Array of bt_rawaddr_t of the bonded remote devices
     *               (Array size inferred from property length).
     */
    BT_PROPERTY_ADAPTER_BONDED_DEVICES,
    /**
     * Description - Bluetooth Adapter Discovery timeout (in seconds)
     * Access mode - GET and SET
     * Data type   - uint32_t
     */
    BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
    /* Properties unique to remote device */
    /**
     * Description - User defined friendly name of the remote device
     * Access mode - GET and SET
     * Data type   - bt_bdname_t.
     */
    BT_PROPERTY_REMOTE_FRIENDLY_NAME,
    /**
     * Description - RSSI value of the inquired remote device
     * Access mode - Only GET.
     * Data type   - int32_t.
     */
    BT_PROPERTY_REMOTE_RSSI,
    /**
     * Description - Remote version info
     * Access mode - SET/GET.
     * Data type   - bt_remote_version_t.
     */
    BT_PROPERTY_REMOTE_VERSION_INFO,
    /**
     * Description - Local LE features
     * Access mode - GET.
     * Data type   - bt_local_le_features_t.
     */
    BT_PROPERTY_LOCAL_LE_FEATURES,

    BT_PROPERTY_INQUIRY_EIR_RESULT,
    BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP = 0xFF,
} bt_property_type_t;

/** Bluetooth Adapter Property data structure */
typedef struct
{
    bt_property_type_t type;
    int len;
    void *val;
} bt_property_t;

/** Bluetooth Device Type */
typedef enum
{
    BT_DEVICE_DEVTYPE_BREDR = 0x1,
    BT_DEVICE_DEVTYPE_BLE,
    BT_DEVICE_DEVTYPE_DUAL
} bt_device_type_t;

/** Bluetooth Bond state */
typedef enum
{
    BT_BOND_STATE_NONE,
    BT_BOND_STATE_BONDING,
    BT_BOND_STATE_BONDED
} bt_bond_state_t;

/** Bluetooth Encrypted state */
typedef enum
{
    BT_ENC_DISABLE,
    BT_ENC_ENABLE,
} bt_encrypted_state_t;

/** Bluetooth SSP Bonding Variant */
typedef enum
{
    BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
    BT_SSP_VARIANT_PASSKEY_ENTRY,
    BT_SSP_VARIANT_CONSENT,
    BT_SSP_VARIANT_PASSKEY_NOTIFICATION
} bt_ssp_variant_t;

/** Bluetooth Interface callbacks */
/** Bluetooth Enable/Disable Callback. */
typedef void (*adapter_state_changed_callback)(bt_state_t state);

/** GET/SET Adapter Properties callback */
/* TODO: For the GET/SET property APIs/callbacks, we may need a session
 * identifier to associate the call with the callback. This would be needed
 * whenever more than one simultaneous instance of the same adapter_type
 * is get/set.
 *
 * If this is going to be handled in the Java framework, then we do not need
 * to manage sessions here.
 */
typedef void (*adapter_properties_callback)(bt_status_ui_t status,
                                            int num_properties,
                                            bt_property_t *properties);

/** GET/SET Remote Device Properties callback */
/** TODO: For remote device properties, do not see a need to get/set
 * multiple properties - num_properties shall be 1
 */
typedef void (*remote_device_properties_callback)(bt_status_ui_t status,
                                                  bt_rawaddr_t *bd_addr,
                                                  int num_properties,
                                                  bt_property_t *properties);

/** New device discovered callback */
/** If EIR data is not present, then BD_NAME and RSSI shall be NULL and -1
 * respectively */
typedef void (*device_found_callback)(int num_properties,
                                      bt_property_t *properties);
/** Discovery state changed callback */
typedef void (*discovery_state_changed_callback)(bt_discovery_state_t state);
/** Bluetooth Legacy PinKey Request callback */
typedef void (*pin_request_callback)(bt_rawaddr_t *remote_bd_addr,
                                     bt_bdname_t *bd_name, uint32_t cod, bool min_16_digit);

/** Bluetooth SSP Request callback - Just Works & Numeric Comparison*/
/** pass_key - Shall be 0 for BT_SSP_PAIRING_VARIANT_CONSENT &
 *  BT_SSP_PAIRING_PASSKEY_ENTRY */
/* TODO: Passkey request callback shall not be needed for devices with display
 * capability. We still need support this in the stack for completeness */
typedef void (*ssp_request_callback)(bt_rawaddr_t *remote_bd_addr,
                                     bt_bdname_t *bd_name,
                                     uint32_t cod,
                                     bt_ssp_variant_t pairing_variant,
                                     uint32_t pass_key);
/** Bluetooth Bond state changed callback */
/* Invoked in response to create_bond, cancel_bond or remove_bond */
typedef void (*bond_state_changed_callback)(bt_status_ui_t status,
                                            bt_rawaddr_t *remote_bd_addr,
                                            bt_bond_state_t state);

/** Bluetooth Encrypted state changed callback */
typedef void (*encrypted_state_changed_callback)(bt_status_ui_t status,
                                            bt_rawaddr_t *remote_bd_addr,
                                            bt_encrypted_state_t state);

/** Bluetooth ACL connection state changed callback */
typedef void (*acl_state_changed_callback)(bt_status_ui_t status, bt_rawaddr_t *remote_bd_addr,
                                           bt_acl_state_ui_t state);

typedef struct
{
    /** set to sizeof(bt_callbacks_t) */
    size_t size;
    adapter_state_changed_callback adapter_state_changed_cb;
    adapter_properties_callback adapter_properties_cb;
    remote_device_properties_callback remote_device_properties_cb;
    device_found_callback device_found_cb;
    discovery_state_changed_callback discovery_state_changed_cb;
    pin_request_callback pin_request_cb;
    ssp_request_callback ssp_request_cb;
    bond_state_changed_callback bond_state_changed_cb;
    encrypted_state_changed_callback enc_state_changed_cb;
    acl_state_changed_callback acl_state_changed_cb;
} bt_callbacks_t;

/************************************ A2DP Profile ******************************/
/* Bluetooth AV connection states */
typedef enum
{
    BTAV_CONNECTION_STATE_DISCONNECTED = 0,
    BTAV_CONNECTION_STATE_CONNECTING,
    BTAV_CONNECTION_STATE_CONNECTED,
    BTAV_CONNECTION_STATE_DISCONNECTING
} btav_connection_state_t;

/* Bluetooth AV datapath states */
typedef enum
{
    BTAV_AUDIO_STATE_REMOTE_SUSPEND = 0,
    BTAV_AUDIO_STATE_STOPPED,
    BTAV_AUDIO_STATE_STARTED,
} btav_audio_state_t;

/** Callback for connection state change.
 *  state will have one of the values from btav_connection_state_t
 */
typedef void (*btav_connection_state_callback)(const bt_rawaddr_t *bd_addr, btav_connection_state_t state);

/** Callback for audiopath state change.
 *  state will have one of the values from btav_audio_state_t
 */
typedef void (*btav_audio_state_callback)(const bt_rawaddr_t *bd_addr, btav_audio_state_t state);

/** BT-AV A2DP Source callback structure. */
typedef struct
{
    /** set to sizeof(btav_source_callbacks_t) */
    size_t size;
    btav_connection_state_callback connection_state_cb;
    btav_audio_state_callback audio_state_cb;
} btav_source_callbacks_t;

/************************************ HFP Profile ******************************/
typedef enum
{
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTING,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING
} bthf_client_connection_state_t;

typedef enum
{
    BTHF_CLIENT_AUDIO_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_AUDIO_STATE_CONNECTING,
    BTHF_CLIENT_AUDIO_STATE_CONNECTED,
    BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC,
} bthf_client_audio_state_t;

typedef enum
{
    BTHF_CLIENT_CALL_NONE,
    BTHF_CLIENT_CALL_ACTIVE,
} bthf_client_call_state_t;



typedef enum
{
    BTHF_CLIENT_CALL_SETUP_NONE,
    BTHF_CLIENT_CALL_SETUP_IN,
    BTHF_CLIENT_CALL_SETUP_OUT,
    BTHF_CLIENT_CALL_SETUP_ALERT,
} bthf_client_call_setup_state_t;

/** Callback for connection state change.
 *  state will have one of the values from BtHfConnectionState
 *  peer/chld_features are valid only for
 * BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED state
 */
typedef void (*bthf_client_connection_state_callback)(
    const bt_rawaddr_t *bd_addr, bthf_client_connection_state_t state,
    unsigned int peer_feat, unsigned int chld_feat);

/** Callback for audio connection state change.
 *  state will have one of the values from BtHfAudioState
 */
typedef void (*bthf_client_audio_state_callback)(
    const bt_rawaddr_t *bd_addr, bthf_client_audio_state_t state);

/** Callback for caller ind info.
 */
typedef void (*bthf_client_caller_ind_callback)(
    const bt_rawaddr_t *bd_addr, const uint8_t *number, int number_len);

/** Callback for call state change.
 */
typedef void (*bthf_client_call_state_callback)(
    const bt_rawaddr_t *bd_addr, bthf_client_call_state_t state);

/** Callback for call setup state change.
 */
typedef void (*bthf_client_call_setup_state_callback)(
    const bt_rawaddr_t *bd_addr, bthf_client_call_setup_state_t state);

/** BT-HF callback structure. */
typedef struct
{
    /** set to sizeof(BtHfClientCallbacks) */
    size_t size;
    bthf_client_connection_state_callback connection_state_cb;
    bthf_client_audio_state_callback audio_state_cb;
    bthf_client_caller_ind_callback caller_info_cb;
    bthf_client_call_state_callback call_state_cb;
    bthf_client_call_setup_state_callback call_setup_state_cb;
} bthf_client_callbacks_t;

/************************************ BLE Profile ******************************/
typedef void (*gatt_client_walkie_talkie_receive_stream_state_callback)(uint8_t devied_id, uint8_t state);
typedef void (*gatt_client_walkie_talkie_nebor_changed_callback)(uint8_t count);
typedef struct
{
    /** set to sizeof(ble_client_callbacks_t) */
    size_t size;
    gatt_client_walkie_talkie_receive_stream_state_callback walkie_talkie_receive_stream_state_cb;
    gatt_client_walkie_talkie_nebor_changed_callback walkie_talkie_nebor_changed_cb;

} ble_client_callbacks_t;

/************************************ GATT Profile ******************************/

typedef void (*gatt_client_datapath_connection_state_callback)(uint8_t state, uint8_t conidx);

typedef void (*gatt_client_datapath_receive_data_callback)(uint8_t *data, uint16_t data_len);

typedef struct
{
    /** set to sizeof(gatt_client_callbacks_t) */
    size_t size;
    gatt_client_datapath_connection_state_callback datapath_connection_state_cb;
    gatt_client_datapath_receive_data_callback  datapath_receive_data_cb;

} gatt_client_callbacks_t;


/************************************ ble walkie talkie  ******************************/
typedef void (*ble_client_walkie_talkie_init_callback)(void);
typedef void (*ble_client_walkie_talkie_deinit_callback)(void);
typedef void (*ble_client_walkie_talkie_playback_callback)(uint8_t device_id);
typedef void (*ble_client_walkie_talkie_capture_callback)(uint8_t * pcm_buf, uint32_t len);
typedef void (*ble_client_walkie_talkie_playback_stream_start_callback)(uint8_t device_id, void *kfifo);
typedef void (*ble_client_walkie_talkie_playback_stream_stop_callback)(uint8_t device_id);
typedef void (*ble_client_walkie_talkie_capture_stream_start_callback)(void);
typedef void (*ble_client_walkie_talkie_capture_stream_stop_callback)(void);

typedef struct {
    ble_client_walkie_talkie_init_callback ble_client_walkie_talkie_init_cb;
    ble_client_walkie_talkie_deinit_callback ble_client_walkie_talkie_deinit_cb;
    ble_client_walkie_talkie_playback_callback ble_client_walkie_talkie_playback_cb;
    ble_client_walkie_talkie_capture_callback ble_client_walkie_talkie_capture_cb;
    ble_client_walkie_talkie_playback_stream_start_callback ble_client_walkie_talkie_playback_stream_start_cb;
    ble_client_walkie_talkie_playback_stream_stop_callback ble_client_walkie_talkie_playback_stream_stop_cb;
    ble_client_walkie_talkie_capture_stream_start_callback ble_client_walkie_talkie_capture_stream_start_cb;
    ble_client_walkie_talkie_capture_stream_stop_callback ble_client_walkie_talkie_capture_stream_stop_cb;
}ble_client_walkie_talkie_callback_t;


#ifdef __cplusplus
}
#endif
#endif // end of __BTH_IF_EVT_DEFINITION_H__
