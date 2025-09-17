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

#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "bth_if_core.h"
#include "bth_if_thread_server.h"
#include "bluetooth_bt_api.h"
#include "bth_if_cmd_def.h"
#include "bth_if_evt_def.h"
#include "eaudio_a2dp_timer.h"
#include "kfifo.h"
#include "hal_timer.h"
#include "bt_drv_interface.h"

#ifdef BT_SOURCE
typedef struct _a2dp_fifo_t {
    struct kfifo    audio_fifo;
    uint8_t         *audio_fifo_mem;
    uint32_t        audio_fifo_max_size;
} a2dp_audio_stream_data_fifo_t;

typedef struct _a2dp_app_env_t{
    uint8_t         connect_state;
    uint8_t         stream_state;
    uint8_t         remote_addr[6];
    codec_info_t    codec_info;
} a2dp_app_env_t;

typedef struct _a2dp_source_send_info_t
{
    uint16_t    total_len;
    uint16_t    frame_send_time;
    uint16_t    frame_len;
    uint8_t     frame_num;
    uint8_t     codec_type;
    bool        packet_type;
} a2dp_source_send_info_t;

enum packet_type_e
{
    PCM_DATA    =0,
    ENCODE_DATA =1,
};

static btav_source_callbacks_t *bth_a2dp_if_cb = NULL;
static uint32_t stream_buf_size = 0;
static a2dp_audio_stream_data_fifo_t *audio_stream_fifo = NULL;

static a2dp_app_env_t a2dp_app_env = {0};
static a2dp_source_send_info_t a2dp_send_info ={0};

#define A2DP_SOURCE_STREAM_SEND_PERIOD_MS           (23220)

static osSemaphoreId_t a2dp_media_send_sema_id = NULL;

static osThreadId a2dp_media_thread_tid = NULL;
static void a2dp_src_media_send_thread(const void *arg);
osThreadDef(a2dp_src_media_send_thread, osPriorityAboveNormal, 1, 1024*10, "a2dp_media_send");
#define SEND_DATA_BUF_MAX_SIZE          (1024*8)
#define SEND_DATA_HEADER_SIZE           (sizeof(a2dp_source_send_info_t))

static uint8_t  a2dp_if_comm_type = BTH_IF_COMM_TYPE_RPC;

extern uint32_t btdrv_syn_get_curr_ticks(void);
static uint32_t get_frame_encode_min_samples(uint8_t codec_type)
{
    uint32_t samples_min = 128;
    if (A2DP_IF_STREAM_CODEC_TYPE_SBC == codec_type) {
        samples_min = 128*8;
    } else if (A2DP_IF_STREAM_CODEC_TYPE_AAC == codec_type) {
        samples_min = 1024;
    } else if (A2DP_IF_STREAM_CODEC_TYPE_LC3 == codec_type) {
        samples_min = 60;
    } else if (A2DP_IF_STREAM_CODEC_TYPE_PCM == codec_type) {
        samples_min = 1024;
    }
    return samples_min;
}

POSSIBLY_UNUSED static uint32_t get_stream_send_min_size(void)
{
    uint32_t size = 0;
    size = (get_frame_encode_min_samples(a2dp_app_env.codec_info.codec_type)
                * a2dp_app_env.codec_info.channel_num * a2dp_app_env.codec_info.sample_bits)/8;
    return size;
}

static void a2dp_source_consume_data_to_send(void *param)
{
    osSemaphoreRelease(a2dp_media_send_sema_id);
}

static void  a2dp_src_send_info_read(struct kfifo* audio_fifo)
{
    memcpy((unsigned char*)(&a2dp_send_info),
            audio_fifo->buffer + (audio_fifo->out & (audio_fifo->size - 1)), SEND_DATA_HEADER_SIZE);
}

static void a2dp_src_media_send_thread(const void *arg)
{
    uint8_t data_buf[SEND_DATA_BUF_MAX_SIZE + SEND_DATA_HEADER_SIZE] = {0};
    uint32_t send_len = 0;
    uint32_t real_len = 0;
    uint32_t available_len = 0;
    static uint32_t send_time_ms = A2DP_SOURCE_STREAM_SEND_PERIOD_MS;
    uint32_t cur_bt_tick = 0;

    TRACE(0, "a2dp_src_media_send_thread entry");
    while(1) {
        osSemaphoreAcquire(a2dp_media_send_sema_id, osWaitForever);
        if (a2dp_app_env.stream_state == A2DP_STREAM_CTRL_PLAY) {
            /*************************************************************
            * 1. check whether the data is enough to process.
            * 2. encode the stream buff data based on the negotiate codec.
            * 3. send the buff data according to bt interface.
            *************************************************************/
            available_len =  kfifo_len(&(audio_stream_fifo->audio_fifo));
            if ( available_len >= SEND_DATA_HEADER_SIZE) {
                a2dp_src_send_info_read(&(audio_stream_fifo->audio_fifo));
                send_len = a2dp_send_info.total_len;
                send_len = (send_len > SEND_DATA_BUF_MAX_SIZE)?SEND_DATA_BUF_MAX_SIZE:send_len;
                send_len += SEND_DATA_HEADER_SIZE;

                available_len =  kfifo_len(&(audio_stream_fifo->audio_fifo));
                if (available_len >= send_len) {
                    real_len = kfifo_get(&(audio_stream_fifo->audio_fifo), (uint8_t *)&data_buf[0], send_len);
                    if (real_len == send_len) {
                        bes_bt_source_write_a2dp_data(a2dp_send_info.packet_type, a2dp_send_info.codec_type,
                                data_buf + SEND_DATA_HEADER_SIZE, a2dp_send_info.total_len, a2dp_send_info.frame_len);
                    } else {
                        TRACE(0, "kickout a2dp data lost!!!");
                    }
                }

                if (a2dp_send_info.packet_type == PCM_DATA) {
                    send_time_ms = A2DP_SOURCE_STREAM_SEND_PERIOD_MS;
                } else {
                    send_time_ms = a2dp_send_info.frame_send_time;
                }
            }
            cur_bt_tick = btdrv_syn_get_curr_ticks();
            eaudio_a2dp_timer_consumer_tune(send_time_ms, cur_bt_tick, NULL);
        }
    }
    TRACE(0, "a2dp_src_media_send_thread exit");
}

static const char* a2dp_evt2str(uint16_t event)
{
    switch (event) {
        case BT_EVENT_A2DP_SOURCE_OPENED:
            return "A2DP_SOURCE_OPENED";
        break;
        case BT_EVENT_A2DP_SOURCE_CLOSED:
            return "A2DP_SOURCE_CLOSED";
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_START:
            return "A2DP_SOURCE_STREAM_START";
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_RECONFIG:
            return "A2DP_SOURCE_STREAM_RECONFIG";
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_SUSPEND:
            return "A2DP_SOURCE_STREAM_SUSPEND";
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_CLOSE:
            return "A2DP_SOURCE_STREAM_CLOSE";
        break;
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static const char* a2dp_cmd2str(uint32_t sub_cmd)
{
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CONN_INIT:
            return "A2DP_SRC_INIT";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_STREAM_CTRL:
            return "A2DP_SRC_STREAM_CTRL";
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CODEC_CFG:
            return "A2DP_SRC_CODEC_CFG";
        break;
        default:
            return "UNKNOW";
        break;
    }
    return "UNKNOW";
}

static int btif_a2dp_evt_customif_cb(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    if (memcmp(a2dp_evt2str(event), "UNKNOW", strlen("UNKNOW"))) {
        TRACE(2, "[%s] event=%s", __func__, a2dp_evt2str(event));
    }

    app_bth_if_payload_param_info_t a2dp_param;
    switch (event) {
        case BT_EVENT_A2DP_SOURCE_OPENED: {
            bt_a2dp_opened_param_t* a2dp_conn = param.av.opened;
            a2dp_app_env.connect_state = 1;
            memcpy(a2dp_app_env.remote_addr, bd_addr->address, 6);
            a2dp_app_env.codec_info.codec_type = a2dp_conn->codec_type;

            TRACE(2, "codec_type=%d, codec_info_len=%d", a2dp_conn->codec_type, a2dp_conn->codec_info_len);
            a2dp_param.a2dp_conn_state_evt_info.error_code = a2dp_conn->error_code;
            a2dp_param.a2dp_conn_state_evt_info.codec_type = a2dp_conn->codec_type;
            memcpy(a2dp_param.a2dp_conn_state_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (a2dp_if_comm_type == BTH_IF_COMM_TYPE_RPC) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP, (uint8_t*)&a2dp_param.context,
                        sizeof(app_bt_a2dp_conn_state_evt_t), BTH_IF_CORE_COMM_RESULT_A2DP_CONNECT_EVT);
            }
        }
        break;
        case BT_EVENT_A2DP_SOURCE_CLOSED: {
            bt_a2dp_closed_param_t* a2dp_disc = param.av.closed;
            a2dp_app_env.connect_state = 0;
            a2dp_app_env.stream_state = A2DP_STREAM_CTRL_STOP;

            a2dp_param.a2dp_disc_state_evt_info.error_code = a2dp_disc->error_code;
            memcpy(a2dp_param.a2dp_disc_state_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (a2dp_if_comm_type == BTH_IF_COMM_TYPE_RPC) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP, (uint8_t*)&a2dp_param.context,
                        sizeof(app_bt_a2dp_disc_state_evt_t), BTH_IF_CORE_COMM_RESULT_A2DP_DISCONN_EVT);
            }
        }
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_START: {
            a2dp_param.a2dp_stream_state_evt_info.onoff = true;
            a2dp_param.a2dp_stream_state_evt_info.codec_type = a2dp_app_env.codec_info.codec_type;
            memcpy(a2dp_param.a2dp_stream_state_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (a2dp_if_comm_type == BTH_IF_COMM_TYPE_RPC) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP, (uint8_t*)&a2dp_param.context,
                        sizeof(app_bt_a2dp_stream_state_evt_t), BTH_IF_CORE_COMM_RESULT_A2DP_STREAM_STATUS_CHANGE_EVT);
            }
            a2dp_app_env.stream_state = A2DP_STREAM_CTRL_PLAY;
            if (audio_stream_fifo != NULL) {
                eaudio_a2dp_timer_consumer_init(a2dp_source_consume_data_to_send, audio_stream_fifo);
                eaudio_a2dp_timer_consumer_start(A2DP_SOURCE_STREAM_SEND_PERIOD_MS);
            } else {
                uint8_t device_id = bt_adapter_get_device_id_byaddr(bd_addr);
                bes_bt_source_suspend_a2dp_stream(device_id);
            }
        }
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_RECONFIG: {
            bt_a2dp_stream_reconfig_param_t* a2dp_codec = param.av.stream_reconfig;
            a2dp_app_env.codec_info.codec_type = a2dp_codec->codec_type;
            TRACE(2, "codec_type=%d, codec_info_len=%d", a2dp_codec->codec_type, a2dp_codec->codec_info_len);
        }
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_SUSPEND: {
            a2dp_param.a2dp_stream_state_evt_info.onoff = false;
            a2dp_param.a2dp_stream_state_evt_info.codec_type = a2dp_app_env.codec_info.codec_type;
            memcpy(a2dp_param.a2dp_stream_state_evt_info.bt_addr.addr, bd_addr->address, 6);
            if (a2dp_if_comm_type == BTH_IF_COMM_TYPE_RPC) {
                bth_if_send_result_to_app(APP_BTH_IF_USER_A2DP, (uint8_t*)&a2dp_param.context,
                        sizeof(app_bt_a2dp_stream_state_evt_t), BTH_IF_CORE_COMM_RESULT_A2DP_STREAM_STATUS_CHANGE_EVT);
            }
            a2dp_app_env.stream_state = A2DP_STREAM_CTRL_PAUSE;
            eaudio_a2dp_timer_consumer_stop();
        }
        break;
        case BT_EVENT_A2DP_SOURCE_STREAM_CLOSE: {
            a2dp_app_env.stream_state = A2DP_STREAM_CTRL_STOP;
            eaudio_a2dp_timer_consumer_deinit();
        }
        break;
        default:
        break;
    }

    return 0;
}

uint32_t app_bth_a2dp_src_if_enable(void *callback_func_if, uint8_t comm_type)
{
    bt_add_event_callback(btif_a2dp_evt_customif_cb, BT_EVENT_MASK_A2DP_SRC_GROUP);
    bth_a2dp_if_cb = (btav_source_callbacks_t*)callback_func_if;
    a2dp_media_send_sema_id = osSemaphoreNew(65535u, 0, NULL);
    if (!a2dp_media_send_sema_id) {
        ASSERT(0, "cannot create a2dp_media_send_sema_id");
    }
    a2dp_media_thread_tid = osThreadCreate(osThread(a2dp_src_media_send_thread), NULL);
    return 0;
}

uint32_t app_bth_a2dp_src_if_disable(void)
{
    return 0;
}

uint32_t app_bth_a2dp_src_if_restart(void)
{
    return 0;
}

uint32_t app_bth_a2dp_src_if_ctrl(uint32_t sub_cmd, uint8_t *ptr, uint32_t ptr_len)
{
    TRACE(3, "[%s] sub_cmd=%s payload_len=%d", __func__, a2dp_cmd2str(sub_cmd), ptr_len);
    switch (sub_cmd) {
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CONN_INIT: {
            app_a2dp_src_connect_init_cmd_t* conn_init =
                                    (app_a2dp_src_connect_init_cmd_t*)ptr;
            audio_stream_fifo = (a2dp_audio_stream_data_fifo_t*)conn_init->buff_ptr;
            stream_buf_size = conn_init->buff_size;
            TRACE(2, "stream_kfifo=%p buf_size=%d", audio_stream_fifo, stream_buf_size);
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_STREAM_CTRL: {
            app_a2dp_src_stream_ctrl_cmd_t* stream_ctrl =
                                    (app_a2dp_src_stream_ctrl_cmd_t*)ptr;

            if (a2dp_app_env.connect_state) {
                bt_bdaddr_t bd_addr;
                memcpy(bd_addr.address, a2dp_app_env.remote_addr, 6);
                uint8_t device_id = bt_adapter_get_device_id_byaddr(&bd_addr);
                if (stream_ctrl->ctrl_key == A2DP_STREAM_CTRL_PLAY) {
                    bes_bt_source_a2dp_stream_buffer_init();
                    bes_bt_source_start_a2dp_stream(device_id);
                } else if (stream_ctrl->ctrl_key == A2DP_STREAM_CTRL_PAUSE) {
                    bes_bt_source_suspend_a2dp_stream(device_id);
                } else if (stream_ctrl->ctrl_key == A2DP_STREAM_CTRL_STOP) {
                    bes_bt_source_suspend_a2dp_stream(device_id);
                    bes_bt_source_a2dp_stream_buffer_deinit();
                }
            }
        }
        break;
        case BTH_IF_CORE_COMM_SUBCMD_A2DP_SRC_CODEC_CFG: {
            POSSIBLY_UNUSED app_a2dp_src_stream_codec_cmd_t* codec_cfg =
                                    (app_a2dp_src_stream_codec_cmd_t*)ptr;
        }
        break;
        default:
        break;
    }
    return 0;
}

static const app_bth_if_ipc_msg_t bth_a2dp_src_btif_ipc_handle_table = {
    .open           = app_bth_a2dp_src_if_enable,
    .close          = app_bth_a2dp_src_if_disable,
    .restart        = app_bth_a2dp_src_if_restart,
    .ctrl           = app_bth_a2dp_src_if_ctrl,
};
#endif

void app_bth_a2dp_src_if_init(void)
{
#ifdef BT_SOURCE
    bth_if_thread_server_register(APP_BTH_IF_USER_A2DP, &bth_a2dp_src_btif_ipc_handle_table);
    a2dp_app_env.codec_info.codec_type = A2DP_IF_STREAM_CODEC_TYPE_SBC;
    a2dp_app_env.codec_info.sample_bits = 16;
    a2dp_app_env.codec_info.channel_num = 2;
    a2dp_app_env.codec_info.sample_rate = 44100;
#endif
}
