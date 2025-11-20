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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_aud.h"

#include "tgt_hardware.h"
#include "hal_sysfreq.h"
#include "audioflinger.h"
#include "vow_dev.h"
#include "vow_stream.h"
#include "wsink.h"
#include "wusb.h"
#include "wui.h"
//#include "bwifi_interface.h"
#include "hal_wifitsf.h"
#include "vowlan_main.h"

#ifdef APP_UART_MODULE
#include "app_uart_dma_thread.h"
#endif

#ifdef APP_UART_BRIDGE_MODULE
#include "app_uart_bridge.h"
#endif

VOW_DEVICE *vow_dev = NULL;
uint8_t g_vow_status_now = VOW_STATUS_NULL;
uint8_t g_vow_status_old = VOW_STATUS_NULL;

void vow_set_status(uint8_t status)
{
    g_vow_status_old = g_vow_status_now;
    g_vow_status_now = status;
    vow_status_printf(g_vow_status_old, g_vow_status_now);
}

uint8_t vow_get_status(void)
{
    return g_vow_status_now;
}

uint8_t vow_get_old_status(void)
{
    return g_vow_status_old;
}

void vow_status_printf(uint8_t old_status, uint8_t status)
{
    printf("[VoWLAN]");
    switch (old_status) {
    case VOW_STATUS_NULL :
        printf("VOW_STATUS_NULL ->");
        break;
    case VOW_STATUS_PAIRING :
        printf("VOW_STATUS_PAIRING ->");

        break;
    case VOW_STATUS_CONNECTED :
        printf("VOW_STATUS_CONNECTED ->");
        break;
    case VOW_STATUS_STREAM_IDLE :
        printf("VOW_STATUS_STREAM_IDLE ->");
        break;
    case VOW_STATUS_STREAM_TRANSFE :
        printf("VOW_STATUS_STREAM_TRANSFE ->");
        break;
    default:
        printf("error status ## ");
        /* Unhandled intermediate states */
        break;
    }

    switch (status) {
    case VOW_STATUS_NULL :
        printf(" VOW_STATUS_NULL\n");
        break;
    case VOW_STATUS_PAIRING :
        printf(" VOW_STATUS_PAIRING\n");

        break;
    case VOW_STATUS_CONNECTED :
        printf(" VOW_STATUS_CONNECTED\n");
        break;
    case VOW_STATUS_STREAM_IDLE :
        printf(" VOW_STATUS_STREAM_IDLE\n");
        break;
    case VOW_STATUS_STREAM_TRANSFE :
        printf(" VOW_STATUS_STREAM_TRANSFE\n");
        break;
    default:
        printf("error status ##\n");
        /* Unhandled intermediate states */
        break;
    }
}

void vow_open(void)
{
    vow_set_status(VOW_STATUS_NULL);

    return;
}

#ifdef VOWLAN_MASTER /* VOWLAN master device */
void vowlan_start_softap(char *ssid)
{
    int ret = 0;

    /* master unit connected */
    if (bwifi_get_softap_status() == BWIFI_SOFTAP_STATUS_OFF) {
        /* The AP is created and the LAN is established */
        ret = vow_dev_ap_start(vow_dev, ssid);
        if (ret < 0) {
            if (vow_get_status() <= VOW_STATUS_CONNECTED)
                vow_set_status(VOW_STATUS_CONNECTED);
        }
    } else {
        if (vow_get_status() <= VOW_STATUS_PAIRING)
            vow_set_status(VOW_STATUS_PAIRING);
    }
}
#endif

#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
#define GET_MAX_BSS_INFO_NUM    10

int vowlan_find_target_ssid(char *ssid, struct bwifi_bss_info* bssinfo, int count)
{
    int i = 0;
    int len = 0;
    for (i = 0; i < count; i++)
    {
        len = strlen(bssinfo[i].ssid);
        bssinfo[i].ssid[len] = '\0';
        TRACE(0,"ref:%s, %s", ssid, bssinfo[i].ssid);
        if(strcmp(ssid, bssinfo[i].ssid) == 0) {
            return 1;
        }
    }
    return 0;
}

void vowlan_start_station(char *ssid)
{
    struct bwifi_ssid scan_ssid;
    struct bwifi_scan_config scan_config = {0};
    struct bwifi_bss_info ssid_result[GET_MAX_BSS_INFO_NUM];
    int ret = 0, find_ssid_flgs = 0;

    memset(ssid_result, 0, GET_MAX_BSS_INFO_NUM * sizeof(bwifi_bss_info));

    strcpy((char *)scan_ssid.ssid, ssid);
    scan_ssid.next = NULL;
    scan_config.ssids = &scan_ssid;
    scan_config.channels = NULL;

    if (bwifi_get_current_status () < BWIFI_STATUS_GOT_IP) {
        //if (bwifi_get_current_status () < BWIFI_STATUS_GOT_IP)
        {
            bwifi_flush_scan_result();
            ret = bwifi_config_scan(&scan_config);
            osDelay(100);
            bwifi_get_scan_result(ssid_result, GET_MAX_BSS_INFO_NUM);
            find_ssid_flgs = vowlan_find_target_ssid(ssid, ssid_result, GET_MAX_BSS_INFO_NUM);
            if (ret > 0 && find_ssid_flgs)
            {
                TRACE(0,"###############find ssid");
                bwifi_connect_to_ssid((char *)ssid, NULL, 0, 1, NULL);
            }
        }
    } else {
        if (vow_get_status() <= VOW_STATUS_CONNECTED)
            vow_set_status(VOW_STATUS_CONNECTED);

        if (vow_get_status() <= VOW_STATUS_PAIRING)
            vow_set_status(VOW_STATUS_PAIRING);
    }
}
#endif

void vowlan_connected_handle(char *ssid)
{
#ifdef VOWLAN_MASTER /* VOWLAN master device */
    vowlan_start_softap(ssid);
#endif

#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
    /* slave unit connected */
    vowlan_start_station(ssid);
#endif
    return;
}

void vowlan_pairing_handle(void)
{
#ifdef VOWLAN_MASTER /* VOWLAN master device */
    /* master unit pairing */
    vow_cmd_input(VOW_CMD_TYPE_PAIRING, NULL, VOW_FRAME_PAIRING_CMD_LEN);
    osDelay(500);

    //TODO:
#endif

#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
    /* slave unit pairing */

#endif

    return;
}

uint64_t g_heartbeat_tsftime[VOW_SESSION_ID_MAX] = {0};
uint8_t no_heartbeat_cnt[VOW_SESSION_ID_MAX] = {0};
bool sync_vol_flag = false;
void vowlan_on_line_handle(void)
{
#ifdef VOWLAN_MASTER /* VOWLAN master device */
    /* master unit on_line */
    uint8_t i = 0;
    uint8_t on_line_dev_cnt = 0;
    SceneParam_t *sceneparam = vow_get_scene_param();

    /* sync vol cfg */
    if (sync_vol_flag == false) {
        uint8_t sync_vol = TGT_VOLUME_LEVEL_MAX;
        VOW_SESSION *session = NULL;
        for (i = 0; i < sceneparam->rf_tx_number; i++) {
            session = vow_get_session(i);
            if (session == NULL) {
                TRACE(0,"%s, session[%d] is NULL", __func__, i);
                break;
            }
            sync_vol = session->audio_cfg.vol;
        }

        uint8_t cmd_data[VOW_FRAME_MAX_CMD_LEN] = {0};
        cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 7);    /* DIR : M to S. */
        cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 2);    /* TYPE : set vol CMD */

        cmd_data[VOW_CMD_SYNC_CFG_CONT2_SEQ] = sync_vol;

        vow_cmd_input(VOW_CMD_TYPE_SYNC_CFG, cmd_data, VOW_FRAME_SYNC_CFG_CMD_LEN);
        sync_vol_flag = true;
        osDelay(1000);
    }

    /* Check on line status */
    for (i = 0; i < VOW_SESSION_ID_MAX ; i++) {
        if (vow_peer_dev_addr[i][6] == VOW_DEV_ADDR_USE_UP) {
            on_line_dev_cnt++;
            uint64_t last_heartbeat_tsftime = g_heartbeat_tsftime[i];
            g_heartbeat_tsftime[i] = vow_get_heartbeat_tsftime(i);
            int32_t heartbeat_tsfdiff = g_heartbeat_tsftime[i] - last_heartbeat_tsftime;
            TRACE(0,"[%d] - heartbeat_tsfdiff %d.", i, heartbeat_tsfdiff);

            if (heartbeat_tsfdiff == 0) {
                no_heartbeat_cnt[i]++;
                if (no_heartbeat_cnt[i] > 5) {
                    vow_session_free(i);    /* dev off line */
                    no_heartbeat_cnt[i] = 0;
                    on_line_dev_cnt--;
                }
            } else {
                no_heartbeat_cnt[i] = 0;
            }
        }
    }

    if (on_line_dev_cnt == 0) {
        vow_set_status(VOW_STATUS_NULL);
        vow_mac_init();
        bwifi_set_probe_req_filter(0);
        bwifi_softap_stop();
        sync_vol_flag = false;
    }

    uint8_t cmd_data[VOW_FRAME_MAX_CMD_LEN] = {0};
    cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 7);    /* DIR : M to S. */
    cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 3);    /* TYPE : Heartbeat */

    vow_cmd_input(VOW_CMD_TYPE_SYNC_CFG, cmd_data, VOW_FRAME_SYNC_CFG_CMD_LEN);

    osDelay(1000);

    //TODO:
#endif

#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
    /* slave unit on_line */

#endif

    return;
}

void vowlan_control_handle(void)
{
    uint8_t current_status;
    POSSIBLY_UNUSED uint8_t old_status;
    char ssid[32 + 1] = "vow_2005";
    //char ssid[33] = "TP-LINK_E3C9";

    while (1) {
        current_status = vow_get_status();
        old_status = vow_get_old_status();
        switch (current_status) {
        case VOW_STATUS_NULL :
#ifdef VOWLAN_MASTER
            /* set work channel */
            ///TOD:
#endif
            vow_set_status(VOW_STATUS_CONNECTED);
            break;
        case VOW_STATUS_CONNECTED :
            vowlan_connected_handle(ssid);
            break;
        case VOW_STATUS_PAIRING :
            vowlan_pairing_handle();
            break;
        case VOW_STATUS_STREAM_IDLE :
        case VOW_STATUS_STREAM_TRANSFE :
            vowlan_on_line_handle();
            break;
        default:
            /* Unhandled intermediate states */
            vow_set_status(VOW_STATUS_NULL);
            break;
        }

        osDelay(100);
    }
}

void vowlan_main(void const *pArg)
{
    SceneParam_t SceneInitParam;
    memset(&SceneInitParam, 0, sizeof(SceneParam_t));

    vow_open();

    /* SceneInitParam init */
    SceneInitParam.play_ch_num = 2; /* 2.0 or 5.1 audio default local play is 2 channel */

#if defined(VOWLAN_6_CHANNEL_MODE) && (VOWLAN_SELAVE_NUM==1)
    SceneInitParam.play_ch_num = 8; /* 5.1 audio default local play is 8 channel */
#endif

#if defined(VOWLAN_8_CHANNEL_MODE) && (VOWLAN_SELAVE_NUM==1)
    SceneInitParam.play_ch_num = 8; /* 7.1 audio default local play is 8 channel */
#endif

    SceneInitParam.master_ch_num = VOWLAN_OUTPUT_CH_NUM_MASTER;
    SceneInitParam.slave_ch_num = VOWLAN_OUTPUT_CH_NUM_SLAVE;

#ifdef VOWLAN_MASTER /* VOWLAN master device */
    SceneInitParam.type = VOW_DEVICE_MASTER;
    TRACE(0,"type = master ,play_ch_num = %d ,master_ch_num = %d ,slave_ch_num = %d", \
        SceneInitParam.play_ch_num, SceneInitParam.master_ch_num, SceneInitParam.slave_ch_num);

    /* input and output device init */
#if defined(VOWLAN_SUPPORT_INPUT_BY_USB)
    SceneInitParam.input_device |= VOW_INPUT_USB;
    TRACE(0,"input_device : VOW_INPUT_USB");
    SceneInitParam.usb_input_ch_num = USB_AUDIO_RECV_CHAN;
    TRACE(0,"usb_input_ch_num = %d", SceneInitParam.usb_input_ch_num);
#endif

#if defined(VOWLAN_SUPPORT_INPUT_BY_ADC)
    SceneInitParam.input_device |= VOW_INPUT_ADC;
    TRACE(0,"input_device : VOW_INPUT_ADC");
#endif

#if defined(VOWLAN_SUPPORT_INPUT_BY_I2S)
    SceneInitParam.input_device |= VOW_INPUT_IIS;
    TRACE(0,"input_device : VOW_INPUT_IIS");
    SceneInitParam.i2s_input_ch_num = 2;    /* i2s input default paly is 2 channel */
    TRACE(0,"i2s_input_ch_num = %d", SceneInitParam.i2s_input_ch_num);
#endif

#if defined(VOWLAN_SUPPORT_INPUT_BY_BT)
    SceneInitParam.input_device |= VOW_INPUT_BT;
    TRACE(0,"input_device : VOW_INPUT_BT");
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_DAC
    SceneInitParam.output_device |= VOW_OUTPUT_DAC;
    TRACE(0,"output_device : VOW_OUTPUT_DAC");
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_I2S
    SceneInitParam.output_device |= VOW_OUTPUT_IIS;
    TRACE(0,"output_device : VOW_OUTPUT_IIS");
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_RF
    SceneInitParam.output_device |= VOW_OUTPUT_WIFI;
    TRACE(0,"output_device : VOW_OUTPUT_WIFI");
    SceneInitParam.rf_tx_number = VOWLAN_RF_OUTPUT_NUM;
    TRACE(0,"rf_tx_number = %d", SceneInitParam.rf_tx_number);
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_USB
    SceneInitParam.output_device |= VOW_OUTPUT_USB;
    TRACE(0,"output_device : VOW_OUTPUT_USB");
#endif

#endif /* #ifdef VOWLAN_MASTER */

#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
    SceneInitParam.type = VOW_DEVICE_SLAVE;
    TRACE(0,"type = slave , play_ch_num = %d ,master_ch_num = %d ,slave_ch_num = %d", \
        SceneInitParam.play_ch_num, SceneInitParam.master_ch_num, SceneInitParam.slave_ch_num);

    /* input and output device init */
#if defined(VOWLAN_SUPPORT_INPUT_BY_RF_SINK)
    SceneInitParam.input_device |= VOW_INPUT_WIFI;
    TRACE(0,"input_device : VOW_INPUT_WIFI");
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_DAC
    SceneInitParam.output_device |= VOW_OUTPUT_DAC;
    TRACE(0,"output_device : VOW_OUTPUT_DAC");
#endif

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_I2S
    SceneInitParam.output_device |= VOW_OUTPUT_IIS;
    TRACE(0,"output_device : VOW_OUTPUT_IIS");
#endif

#endif /* #ifdef VOWLAN_SLAVE */

#ifdef VOW_AUDIO_192000_24
    SceneInitParam.sample_depth = AUD_BITS_24;
    SceneInitParam.sample_freq = AUD_SAMPRATE_192000;
#endif

#ifdef VOW_AUDIO_96000_24
    SceneInitParam.sample_depth = AUD_BITS_24;
    SceneInitParam.sample_freq = AUD_SAMPRATE_96000;
#endif

#ifdef VOW_AUDIO_48000_24
    SceneInitParam.sample_depth = AUD_BITS_24;
    SceneInitParam.sample_freq = AUD_SAMPRATE_48000;
#endif

#ifdef VOW_AUDIO_192000_16
    SceneInitParam.sample_depth = AUD_BITS_16;
    SceneInitParam.sample_freq = AUD_SAMPRATE_192000;
#endif

#ifdef VOW_AUDIO_96000_16
    SceneInitParam.sample_depth = AUD_BITS_16;
    SceneInitParam.sample_freq = AUD_SAMPRATE_96000;
#endif

#ifdef VOW_AUDIO_48000_16
    SceneInitParam.sample_depth = AUD_BITS_16;
    SceneInitParam.sample_freq = AUD_SAMPRATE_48000;
#endif

#ifdef VOW_AUDIO_16000_16
    SceneInitParam.sample_depth = AUD_BITS_16;
    SceneInitParam.sample_freq = AUD_SAMPRATE_16000;
#endif

    TRACE(0,"sample_depth = %d bit,sample_freq = %d Hz", SceneInitParam.sample_depth, SceneInitParam.sample_freq);

    SceneInitParam.play_interval = VOW_PLAY_INTERVAL * 1000;
    SceneInitParam.palyoad_len = VOW_FRAME_MAX_PAYLOAD_LEN;
    SceneInitParam.vow_dma_buffer_size = SceneInitParam.palyoad_len * 2;

    TRACE(0,"play_interval = %d us,palyoad_len = %d size ,vow_dma_buffer_size = %d size", \
        SceneInitParam.play_interval, SceneInitParam.palyoad_len, SceneInitParam.vow_dma_buffer_size);

    /* bes tbd :tx_repeat_num */
    SceneInitParam.tx_repeat_num = 0;

    /* Connection callback */
#ifdef VOWLAN_SLAVE /* VOWLAN slave device */
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_CONN_INTER_STATE, vow_wifi_connect_callback);
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_CONNECTED, vow_wifi_connect_callback);
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_GOT_IP, vow_wifi_connect_callback);
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_DISCONNECTED, vow_wifi_disconnect_callback);
#endif

#ifdef VOWLAN_MASTER /* VOWLAN master device */
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_AP_STA_CONNECTED, vow_wifi_connect_callback);
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_AP_STA_DISCONNECTED, vow_wifi_disconnect_callback);
#endif

    bwifi_reg_user_evt_handler(WIFI_USER_EVT_MAC_RESET_DONE, vow_error_handle_callback);

    TR_INFO(TR_MOD(MAIN), "_________________ CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    vow_mac_init(); /* set mac */

    vow_dev = vow_dev_init(&SceneInitParam);
    if (vow_dev == NULL) {
        TRACE(0,"%s:%d,vow_dev_init fail", __func__, __LINE__);
    }

    /* vowlan_control_handle is while(1) code */
    vowlan_control_handle();

    osThreadExit();
}

#ifndef VOWLAN_MAIN_THREAD_STACK_SIZE
#define VOWLAN_MAIN_THREAD_STACK_SIZE               (8 * 1024)
#endif
static osThreadId vowlan_main_tid = NULL;
osThreadDef(vowlan_main, osPriorityAboveNormal, 1, VOWLAN_MAIN_THREAD_STACK_SIZE, "vowlan_main");

extern "C" void vowlan_app_init(void)
{
    TRACE(0,"enter %s %d", __func__, __LINE__);

#ifdef APP_UART_MODULE
    app_uart_init();
#endif

#ifdef APP_UART_BRIDGE_MODULE
    app_uart_bridge_init();
#endif

    if (vowlan_main_tid == NULL)
        vowlan_main_tid = osThreadCreate(osThread(vowlan_main), NULL);

    ASSERT(vowlan_main_tid, "%s %d fail to create vowlan_main task", __func__, __LINE__);

    TRACE(0,"create vowlan_main task = 0x%X", (uint32_t)vowlan_main_tid);

    return;
}
