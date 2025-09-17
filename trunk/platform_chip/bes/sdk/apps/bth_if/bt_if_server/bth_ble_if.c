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
#ifdef BLE_HOST_SUPPORT
#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"
#include "kfifo.h"
#ifdef BLE_WALKIE_TALKIE
#include "walkie_talkie.h"
#include "app_walkie_talkie.h"
#endif

static bool btif_module_enable = false;
static uint8_t  bth_if_comm_type = BTH_IF_COMM_TYPE_RPC;
static bt_callbacks_t *bth_ble_if_cb = NULL;

#ifdef BLE_WALKIE_TALKIE
#define BLE_WALKIE_TALKIE_AUDIO_KFIFO_SIZE  1024
#define BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM 4
#define BLE_WALKIE_TALKIE_AUDIO_RECV_PACKET_NUMBER 5


typedef struct {
    bool used_flag;
    uint8_t device_id;
    uint8_t index;
    bool stream_state;
    uint8_t packet_num;
    struct kfifo playback_kfifo;
    uint8_t kfifo_buffer[BLE_WALKIE_TALKIE_AUDIO_KFIFO_SIZE];
}walkie_talkie_link_t;

typedef enum {
    WALKIE_TALKIE_CAPTURE_NONE,
    WALKIE_TALKIE_CAPTURE_INIT_STATE,
    WALKIE_TALKIE_CAPTURE_START_STATE,
    WALKIE_TALKIE_CAPTURE_STOP_STATE,
}WALKIE_TALKIE_CAPTURE_STATE_E;


static bool walkie_talkie_init_flag = false;
static bool walkie_talkie_listen_flag = false;
static uint8_t walkie_talkie_recv_link_num = 0;
static uint8_t walkie_talkie_nabor_count = 0;
walkie_talkie_link_t  walkie_talkie_link[BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM];

static struct kfifo *capture_kfifo;
static uint8_t walkie_talkie_capture_buf[BLE_WALKIE_TALKIE_AUDIO_KFIFO_SIZE];
static WALKIE_TALKIE_CAPTURE_STATE_E walkie_talkie_capture_state = WALKIE_TALKIE_CAPTURE_NONE;

static osSemaphoreId_t walkie_talkie_send_sema_id = NULL;
static osThreadId walkie_talkie_thread_tid = NULL;
static void walkie_talkie_send_thread(const void *arg);
osThreadDef(walkie_talkie_send_thread, osPriorityAboveNormal, 1, 1024, "walkie_talkie_send");

walkie_talkie_link_t *btif_ble_find_walkie_talkie_link(uint8_t device_id)
{
    walkie_talkie_link_t *link = NULL;

    for(uint8_t i = 0; i < BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM; i++)
    {
        if(walkie_talkie_link[i].used_flag && (walkie_talkie_link[i].device_id == device_id))
        {
            link = &walkie_talkie_link[i];
            break;
        }
    }
    return link;
}

walkie_talkie_link_t *btif_ble_get_walkie_talkie_link(void)
{
    walkie_talkie_link_t *link = NULL;

    for(uint8_t i = 0; i < BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM; i++)
    {
        if(!walkie_talkie_link[i].used_flag)
        {
            walkie_talkie_link[i].used_flag = true;
            walkie_talkie_link[i].index = i;
            link = &walkie_talkie_link[i];
            break;
        }
    }

    return link;
}

void btif_ble_free_walkie_talkie_link(walkie_talkie_link_t *link)
{
    link->used_flag = 0;
    link->device_id = 0;
    link->stream_state = false;
    link->packet_num = 0;
}

void btif_ble_stop_audio_player(void)
{
    app_bth_if_payload_param_info_t info;

    for(uint8_t i = 0; i < BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM; i++)
    {
        if(walkie_talkie_link[i].used_flag && walkie_talkie_link[i].stream_state)
        {
            info.ble_walkie_talkie_recv_stream_evt_info.state = 0;
            info.ble_walkie_talkie_recv_stream_evt_info.device_id = walkie_talkie_link[i].index;
            if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
                                sizeof(app_ble_walkie_talkie_recv_stream_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_STREAM_EVT);
            }
            walkie_talkie_link[i].used_flag = false;
            walkie_talkie_link[i].stream_state = false;
            walkie_talkie_link[i].device_id = 0;
            walkie_talkie_link[i].packet_num = 0;
        }
    }
    walkie_talkie_recv_link_num = 0;
}


static void btif_ble_walkie_talkie_ready_to_send_data_cb()
{
    TRACE(1, "W-T:ready_to_send_data_cb");
    walkie_talkie_init_flag = true;
    walkie_talkie_capture_state = WALKIE_TALKIE_CAPTURE_INIT_STATE;
}

static void btif_ble_walkie_talkie_find_device_cb(uint8_t device_id, uint8_t* device_addrss)
{
    app_bth_if_payload_param_info_t info;
    TRACE(1, "W-T:find_device(%d),address:",device_id);

    uint8_t nebor_count = walkie_talkie_get_neighbor_count();
    if(walkie_talkie_nabor_count != nebor_count)
    {
        walkie_talkie_nabor_count = nebor_count;
        info.ble_walkie_talkie_nabor_changed_evt_info.nebor_count = nebor_count;
        if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
            bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
                            sizeof(app_ble_walkie_talkie_nabor_changed_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_NABOR_CHANGED_EVT);
        }
    }
}

static void btif_ble_walkie_talkie_device_loss_cb(uint8_t device_id,uint8_t* device_addrss)
{
    app_bth_if_payload_param_info_t info;
    TRACE(1, "W-T:device(%d),address loss",device_id);

    uint8_t nebor_count = walkie_talkie_get_neighbor_count();
    if(walkie_talkie_nabor_count != nebor_count)
    {
        walkie_talkie_nabor_count = nebor_count;
        info.ble_walkie_talkie_nabor_changed_evt_info.nebor_count = nebor_count;
        if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
            bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
                            sizeof(app_ble_walkie_talkie_nabor_changed_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_NABOR_CHANGED_EVT);
        }
    }
}

static void btif_ble_walkie_talkie_device_is_stalking(uint8_t device_id,uint8_t* device_addrss)
{
    walkie_talkie_link_t *link = NULL;
    app_bth_if_payload_param_info_t info;
    TRACE(1, "W-T:device(%d) is talking",device_id);

    if(walkie_talkie_recv_link_num > BLE_WALKIE_TALKIE_AUDIO_RECV_LINK_MAX_NUM)
    {
        TRACE(3, "%s audio link num is already max, device id %d link num %d", __func__, device_id, walkie_talkie_recv_link_num);
        return;
    }
    link = btif_ble_find_walkie_talkie_link(device_id);
    if(link)
    {
        TRACE(2, "%s device id %d is already started", __func__, device_id);
        return;
    }
    link = btif_ble_get_walkie_talkie_link();
    if(!link)
    {
        TRACE(1, "%s get walkie_talkie_link error", __func__);
        return;
    }
    walkie_talkie_recv_link_num ++;
    link->stream_state = true;
    link->device_id = device_id;
    link->packet_num = 0;
    kfifo_init(&link->playback_kfifo, link->kfifo_buffer, BLE_WALKIE_TALKIE_AUDIO_KFIFO_SIZE * sizeof(uint8_t));
    info.ble_walkie_talkie_recv_stream_evt_info.state = 1;
    info.ble_walkie_talkie_recv_stream_evt_info.device_id = link->index;
    info.ble_walkie_talkie_recv_stream_evt_info.kfifo = (void *)&link->playback_kfifo;
    //TRACE(3, "%s device id %d kfifo %p", __func__, device_id, info.ble_walkie_talkie_recv_stream_evt_info.kfifo);
    if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
        bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
                        sizeof(app_ble_walkie_talkie_recv_stream_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_STREAM_EVT);
    }
}

static void btif_ble_wakie_talkie_device_end_stalking(uint8_t device_id,uint8_t* device_addrss)
{
    walkie_talkie_link_t *link = NULL;
    //app_bth_if_payload_param_info_t info;
    TRACE(1, "W-T:device(%d) talking end",device_id);

    link = btif_ble_find_walkie_talkie_link(device_id);
    if(!link)
    {
        //TRACE(2, "%s get walkie_talkie_link error, device id %d", __func__, device_id);
        return;
    }
    if(link->stream_state)
    {
        // link->stream_state = false;
        // walkie_talkie_recv_link_num = walkie_talkie_recv_link_num > 0 ? walkie_talkie_recv_link_num - 1 : 0;
        // info.ble_walkie_talkie_recv_stream_evt_info.state = 0;
        // info.ble_walkie_talkie_recv_stream_evt_info.device_id = link->index;
        // if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
        //     bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
        //                     sizeof(app_ble_walkie_talkie_recv_stream_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_STREAM_EVT);
        // }
    }
}

static const app_walkie_talkie_full_dup_event_cb btif_ble_walkie_talkie_full_dup_event_cbs = {
    .wt_ready_to_send_data = btif_ble_walkie_talkie_ready_to_send_data_cb,
    .wt_find_device = btif_ble_walkie_talkie_find_device_cb,
    .wt_device_loss = btif_ble_walkie_talkie_device_loss_cb,
    .wt_device_is_stalking = btif_ble_walkie_talkie_device_is_stalking,
    .wt_device_end_stalking = btif_ble_wakie_talkie_device_end_stalking,
};

static void btif_ble_walkie_talkie_data_callback(uint8_t device_id, uint8_t *data, uint16_t dataLen)
{
    walkie_talkie_link_t *link = NULL;
    app_bth_if_payload_param_info_t info;

    if((dataLen == 0) || (walkie_talkie_listen_flag == false))
    {
        return;
    }

    link = btif_ble_find_walkie_talkie_link(device_id);
    if(!link || !link->stream_state)
    {
        TRACE(2, "%s devicd id %d not start, started and discard msg!", __func__, device_id);
        btif_ble_walkie_talkie_device_is_stalking(device_id, NULL);
        return;
    }

    TRACE(2, "%s recv data device id %d", __func__, device_id);
    //push data to kfifo
    uint32_t avil_size = kfifo_get_free_space(&link->playback_kfifo);
    if(avil_size < dataLen)
    {
        TRACE(2, "%s recv bis audio kfifo full, avil size %d", __func__, avil_size);
        link->packet_num = BLE_WALKIE_TALKIE_AUDIO_RECV_PACKET_NUMBER;
    }
    else
    {
        kfifo_put(&link->playback_kfifo, data, dataLen);
        link->packet_num++;
    }
    if(link->packet_num >= BLE_WALKIE_TALKIE_AUDIO_RECV_PACKET_NUMBER)
    {
        info.ble_walkie_talkie_recv_data_evt_info.device_id = link->index;
        if (BTH_IF_COMM_TYPE_RPC == bth_if_comm_type) {
            bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&info.context,
                            sizeof(app_ble_walkie_talkie_recv_data_evt_t), BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_RECV_DATA_EVT);
        }
        link->packet_num = 0;
    }
}

static void walkie_talkie_send_thread(const void *arg)
{
    uint32_t send_len = 0;
    uint32_t available_len = 0;

    TRACE(0, "walkie_talkie_send_thread entry");
     while(1) {
        if((walkie_talkie_capture_state == WALKIE_TALKIE_CAPTURE_INIT_STATE) || (walkie_talkie_capture_state == WALKIE_TALKIE_CAPTURE_NONE))
        {
            osSemaphoreAcquire(walkie_talkie_send_sema_id, osWaitForever);
        }
        else if(walkie_talkie_capture_state == WALKIE_TALKIE_CAPTURE_START_STATE)
        {
            osSemaphoreAcquire(walkie_talkie_send_sema_id, 50);
        }
        else if(walkie_talkie_capture_state == WALKIE_TALKIE_CAPTURE_STOP_STATE)
        {
            walkie_talkie_capture_state = WALKIE_TALKIE_CAPTURE_INIT_STATE;
            bes_ble_walkie_send_data(BES_IF_WALKIE_GAP_DATA_END, NULL, 0);
        }

        if(walkie_talkie_capture_state == WALKIE_TALKIE_CAPTURE_START_STATE)
        {
            available_len =  kfifo_len(capture_kfifo);
            if(available_len >= 83)
            {
                send_len = kfifo_get(capture_kfifo, (uint8_t *)&walkie_talkie_capture_buf[0], 83);
                bes_ble_walkie_send_data(BES_IF_WALKIE_GAP_DATA_AUDIO, walkie_talkie_capture_buf,send_len);
                TRACE(3, "%s send data len %d available len %d", __func__, send_len, available_len);
            }
        }

    }
    TRACE(0, "walkie_talkie_send_thread exit");
}

void btif_ble_walkie_talkie_init(void)
{
    walkie_talkie_send_sema_id = osSemaphoreNew(65535u, 0, NULL);
    if (!walkie_talkie_send_sema_id) {
        ASSERT(0, "cannot create walkie_talkie_send_sema_id");
    }
    walkie_talkie_thread_tid = osThreadCreate(osThread(walkie_talkie_send_thread), NULL);
    wakie_talke_register_data_ind_call_back(btif_ble_walkie_talkie_data_callback);
    app_walkie_talkie_reg_full_dup_state_changed_callback(&btif_ble_walkie_talkie_full_dup_event_cbs);
}

#endif


static void btif_ble_evt_customif_cb(ble_event_t *event, void *param)
{
    TRACE(3, "[%s] event=%p btif_enable=%d", __func__, event, btif_module_enable);
}

uint32_t app_bth_ble_if_enable(void *callback_func_if, uint8_t comm_type)
{
    bes_ble_gap_core_register_global_handler(btif_ble_evt_customif_cb);

    bth_ble_if_cb = (bt_callbacks_t *)callback_func_if;
    bth_if_comm_type = comm_type;
    btif_module_enable = true;

    //return the bluetooth interface ptr to client side
    app_bt_init_cmp_evt_t report_ret;
    report_ret.module_interface_addr = (uint32_t)NULL;
    bth_if_send_result_to_app(APP_BTH_IF_USER_BLE, (uint8_t*)&report_ret,
                                sizeof(app_bt_init_cmp_evt_t), BTH_IF_CORE_COMM_RESULT_INIT_CMP_EVT);

    return 0;
}

uint32_t app_bth_ble_if_disable(void)
{
    btif_module_enable = false;
    bth_ble_if_cb = NULL;
    return 0;
}

uint32_t app_bth_ble_if_restart(void)
{
    return 0;
}

uint32_t app_bth_ble_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{
    TRACE(2, "%s sub_cmd %d", __func__, sub_cmd);
    switch(sub_cmd)
    {
#ifdef BLE_WALKIE_TALKIE
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_AUDIO_INIT_DONE:
        //bes_ble_walkie_bis_register_recv_iso_data_callback();
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_INIT:
            app_walkie_talkie_init();
            bes_ble_walkie_bis_register_recv_iso_data_callback();
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_DEINIT:
            btif_ble_stop_audio_player();
            bes_ble_walkie_bis_unregister_recv_iso_data_callback();
            app_walkie_talkie_deinit();
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_OPEN_SCAN:
            walkie_talkie_listen_flag = true;
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_STOP_SCAN:
            walkie_talkie_listen_flag = false;
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_START_CAPTURE:
        {
            app_ble_walkie_talkie_start_capture_cmd_t *capture = (app_ble_walkie_talkie_start_capture_cmd_t*)ptr;
            capture_kfifo = (struct kfifo *)capture->kfifo;
            //walkie_talkie_start_capture();
            //app_walkie_talkie_handle_event(WT_TALK_BTN_DOWN);
            walkie_talkie_capture_state = WALKIE_TALKIE_CAPTURE_START_STATE;
            osSemaphoreRelease(walkie_talkie_send_sema_id);
            TRACE(2, "%s start capture, kfifo %p", __func__, capture_kfifo);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_WALKIE_TALKIE_STOP_CAPTURE:
            walkie_talkie_capture_state = WALKIE_TALKIE_CAPTURE_STOP_STATE;
            //walkie_talkie_stop_capture();
            //app_walkie_talkie_handle_event(WT_TALK_BTN_UP);
            TRACE(1, "%s stop capture", __func__);
        break;
#endif
        case BTH_IF_CORE_COMM_SUBCMD_BLE_ADV_ENABLE: {
            bool adv_enable = *ptr;
            if (adv_enable) {
                bes_ble_gap_custom_adv_start(BLE_ADV_ACTIVITY_USER_2);
            } else {
                bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_2);
            }
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_BLE_CON_UPDATE: {
            uint32_t con_interval = *(uint32_t *)ptr;
            TRACE(1, "%s interval %d", __func__, con_interval);
            bes_ble_gap_conn_update_param(0, con_interval, con_interval, 20000, 0);
        }
        break;
        default:
        break;
    }

    return 0;
}


static const app_bth_if_ipc_msg_t bth_bleif_ipc_handle_table = {
    .open           = app_bth_ble_if_enable,
    .close          = app_bth_ble_if_disable,
    .restart        = app_bth_ble_if_restart,
    .ctrl           = app_bth_ble_if_ctrl,
};

void app_bth_ble_if_init(void)
{
    bth_if_thread_server_register(APP_BTH_IF_USER_BLE, &bth_bleif_ipc_handle_table);
#ifdef BLE_WALKIE_TALKIE
    btif_ble_walkie_talkie_init();
#endif
}

#endif

