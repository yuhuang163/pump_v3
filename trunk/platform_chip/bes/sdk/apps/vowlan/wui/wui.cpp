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
#include "wui.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "hal_wifitsf.h"

#include "vow_dev.h"
#include "vowlan_main.h"
#include "vow_session.h"
#include "vow_stream.h"
#include "wusb.h"
#include "wsink.h"
#include "bwifi_interface.h"

uint8_t conn_sta_num = 0;
#define AP_STATUS_ONLINE     1
#define AP_STATUS_OFFLINE    0
uint8_t  ap_status= AP_STATUS_OFFLINE;

int vow_wifi_set_ap_status(uint8_t status)
{
    ap_status = status;
    return 0;
}

int vow_wifi_get_ap_status(void)
{
    return ap_status;
}

void vow_wifi_connect_callback(WIFI_USER_EVT_ID event_id, void *arg)
{
    printf("%s event_id:%d\n", __func__, event_id);

    SceneParam_t *sceneparam = vow_get_scene_param();
    uint8_t mac[6];

    if (event_id == WIFI_USER_EVT_CONN_INTER_STATE) {
        BWIFI_CONNECT_INTER_STATE state = *(BWIFI_CONNECT_INTER_STATE *)arg;

        switch (state) {
        case INTER_STATE_AUTHENTICATING:
        case INTER_STATE_ASSOCIATING:
        case INTER_STATE_ASSOCIATED:
        case INTER_STATE_4WAY_HANDSHAKE:
        case INTER_STATE_GROUP_HANDSHAKE:
            break;
        case INTER_STATE_AUTH_REJECT:
        case INTER_STATE_AUTH_TIMEOUT:
        case INTER_STATE_ASSOC_REJECT:
        case INTER_STATE_ASSOC_TIMEOUT:
        case INTER_STATE_HANDSHAKE_FAIL:
            break;
        default:
            /* Unhandled intermediate states */
            return;
        }

    } else if (event_id == WIFI_USER_EVT_CONNECTED) {

        TRACE(0,"connect ...");
#if LWIP_SUPPORT
    } else if (event_id == WIFI_USER_EVT_GOT_IP) {
#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
        if (sceneparam->type == VOW_DEVICE_SLAVE) {
            vow_update_wifitsf();   /* sync wifitsf */

            bwifi_get_dev_mac(WIFI_IF_STATION, mac);
            TRACE(0,"slave mac : %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            //TODO
        }
#endif
#endif
    }else if (event_id == WIFI_USER_EVT_AP_STA_CONNECTED) {
#ifdef VOWLAN_MASTER /* VOWLAN master device */
        if (sceneparam->type == VOW_DEVICE_MASTER) {
            bwifi_get_dev_mac(WIFI_IF_STATION, mac);
            mac[5] ^= WIFI_IF_SOFTAP;
            TRACE(0,"master mac : %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            //TODO
            vow_wifi_set_ap_status(AP_STATUS_ONLINE);
        }
#endif
    }
}

void vow_wifi_disconnect_callback(WIFI_USER_EVT_ID event_id, void *arg)
{
    printf("%s event_id:%d\n", __func__, event_id);

    if (event_id == WIFI_USER_EVT_DISCONNECTED) {
        //TODO
#if defined(VOWLAN_SUPPORT_OUTPUT_BY_I2S) && defined(VOWLAN_SLAVE)
        vow_i2s_stream_stop();
        vow_i2s_stream_close();
        vow_slink_init_flag_clear();
#endif
    } else if (event_id == WIFI_USER_EVT_AP_STA_DISCONNECTED) {
        //TODO
        vow_wifi_set_ap_status(AP_STATUS_OFFLINE);
        /* Soft AP Filter */
        bwifi_set_probe_req_filter(0);
    }

    if (vow_get_status() >= VOW_STATUS_CONNECTED)
        vow_set_status(VOW_STATUS_CONNECTED);
}

void vow_error_handle_callback(WIFI_USER_EVT_ID event_id, void *arg)
{
    switch (event_id)
    {
        //TODO
        default:
            /* Unhandled intermediate states */
            return;
    }
}
