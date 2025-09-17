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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "hal_wifitsf.h"
#include "audioflinger.h"
#include "usb_audio_app.h"

#include "vow_dev.h"
#include "vow_session.h"
#include "vow_stream.h"
#include "vowlan_main.h"

#include "wusb.h"
#include "usb_audio.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "wui.h"
#include "usbaudio_thread.h"


#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
#ifdef BLE_WIFI_DUAL_CHIP_MODE
#include "app_ble_wifi_communication.h"
#endif
#ifdef BLE_WIFI_DUAL_MODE_DONGLE
extern "C" void * app_os_tid_get(void);
extern osThreadId app_get_bt_thread_id(void);
extern osThreadId vow_session_thread_id(void);
static USB_AUDIO_SOURCE_EVENT_CALLBACK_T lea_stream_func_list;
#endif
// static bool ble_audio_focused = false;
static bool lea_playback_stream_started = false;
static bool lea_capture_stream_started = false;
static bool usb_playback_stream_started = false;
static bool usb_capture_stream_started = false;
#endif

#define WUSB_DEBUG                      (1)
#define ALIGNED4                        ALIGNED(4)
#if defined(VOWLAN_8_CHANNEL_MODE) || defined(VOWLAN_6_CHANNEL_MODE)
#define VOW_USB_CHANNEL_SIZE            (VOW_FRAME_MAX_PAYLOAD_LEN / 8)
#else
#define VOW_USB_CHANNEL_SIZE            (VOW_FRAME_MAX_PAYLOAD_LEN / 2)
#endif
#define VOW_USB_RECV_MAX_PAYLOAD_LEN    (VOW_USB_CHANNEL_SIZE * (USB_AUDIO_RECV_CHAN))

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_USB
#define WUSB_DATA_SEND_BUF_NUM   (3)
static VOW_DATA_UPDATA_BUF wusb_data_send_buf[WUSB_DATA_SEND_BUF_NUM] ALIGNED4;
#if (USB_AUDIO_RECV_CHAN == 2)
static SRAM_BSS_LOC uint8_t temporary_buf[VOW_USB_RECV_MAX_PAYLOAD_LEN / 4] ALIGNED4;
#endif
#endif

static VOW_AUDIO_CFG rf_out_audio_cfg;
static VOW_AUDIO_CFG local_out_audio_cfg;

static int8_t wusb_tran_vsess_id[VOWLAN_RF_OUTPUT_NUM] = {0};
static int8_t wusb_main_vsess_id = -1;
static uint8_t g_audio_channel = 2;
static uint8_t *vow_usb_recv_buffer_new = NULL;
static uint32_t usb_recv_payload_len = 0;
static uint64_t usb_timestamp = 0;

#ifndef WUSB_AUDIO_INPUT_THREAD_STACK_SIZE
#define WUSB_AUDIO_INPUT_THREAD_STACK_SIZE          (8 * 1024)
#endif
static void wusb_audio_input_data_codec(void const *pArg);
osThreadDef(wusb_audio_input_data_codec, osPriorityAboveNormal, 1, WUSB_AUDIO_INPUT_THREAD_STACK_SIZE, "wusb_audio_input_data_codec");
static osThreadId usb_codec_tid = NULL;

osSemaphoreDef(usb_irq_sem);
static osSemaphoreId usb_irq_sem_id = NULL;

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)
int32_t accumula_usb_recv_len = 0;
bool usb_recv_finish_flag = false;

#if ((USB_AUDIO_RECV_CHAN) == 2)
SRAM_BSS_LOC uint8_t wusb_ch0_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wusb_ch1_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */

SRAM_BSS_LOC uint8_t wusb_ch0_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wusb_ch1_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */
#endif

#if ((USB_AUDIO_RECV_CHAN) == 6)
SRAM_BSS_LOC uint8_t wusb_ch0_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wusb_ch1_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */
SRAM_BSS_LOC uint8_t wusb_ch2_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FC */
SRAM_BSS_LOC uint8_t wusb_ch3_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_LFE */
SRAM_BSS_LOC uint8_t wusb_ch4_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_BL */
SRAM_BSS_LOC uint8_t wusb_ch5_buf0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_BR */

SRAM_BSS_LOC uint8_t wusb_ch0_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wusb_ch1_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */
SRAM_BSS_LOC uint8_t wusb_ch2_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FC */
SRAM_BSS_LOC uint8_t wusb_ch3_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_LFE */
SRAM_BSS_LOC uint8_t wusb_ch4_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_BL */
SRAM_BSS_LOC uint8_t wusb_ch5_buf1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_BR */

#if (VOWLAN_SUPPORT_OUTPUT_BY_I2S == 1) || (VOWLAN_SUPPORT_OUTPUT_BY_DAC == 1)
/* wusb_main_cha contain 2ch data */
//SRAM_BSS_LOC uint8_t wusb_main_cha_buf0[VOW_USB_CHANNEL_SIZE * 2] ALIGNED4;
//SRAM_BSS_LOC uint8_t wusb_main_cha_buf1[VOW_USB_CHANNEL_SIZE * 2] ALIGNED4;
#endif

#endif

#if ((USB_AUDIO_RECV_CHAN) == 8)
/* default 1 to 1 */
#endif
#else
#if (USB_AUDIO_RECV_CHAN == 2) && defined(__VOW_SPLIT_L_R_CHANNEL__)
SRAM_BSS_LOC uint8_t wusb_ch0[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FL */
SRAM_BSS_LOC uint8_t wusb_ch1[VOW_USB_CHANNEL_SIZE] ALIGNED4;          /* CHANNEL_FR */
#else
SRAM_BSS_LOC uint8_t vow_usb_recv_buffer_old[VOW_USB_RECV_MAX_PAYLOAD_LEN] ALIGNED4;
#endif
#endif

extern "C" int wifiusbaudio_entry(void);
extern "C" void wifiusbaudio_exit(void);

static void wusb_audio_input_ch71_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 8)
    VOW_SESSION *session = NULL;
    uint32 i = 0;

    SceneParam_t *sceneparam = vow_get_scene_param();
#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    //TODO
#else   /* Single slave or No slave */
    /* if 7.1 ch mode slave = 1, Default : Master no play, The slave plays the 7.1 channels separately */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == VOW_CHANNEL_ALL) {
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 8, &usb_timestamp);
        }
    }

#endif

#endif  /* USB_AUDIO_RECV_CHAN = 8 */
}

static void wusb_audio_input_ch71_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 8)
    VOW_SESSION *session = NULL;
    uint32 i = 0;
//    TRACE(0,"%s %d", __func__, Len);

    SceneParam_t *sceneparam = vow_get_scene_param();
#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    //TODO
#else   /* Single slave or No slave */
    /* if 7.1 ch mode slave = 1, Default : Master no play, The slave plays the 7.1 channels separately */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == VOW_CHANNEL_ALL) {
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 8, &usb_timestamp);
        }
    }
#endif

#endif  /* USB_AUDIO_RECV_CHAN = 8 */
}

static void wusb_audio_input_ch51_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 6)
    VOW_SESSION *session = NULL;
    uint32 i = 0;

    SceneParam_t *sceneparam = vow_get_scene_param();
#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    //TODO
#else   /* Single slave or No slave */
    /* if 5.1 ch mode slave = 1, Default : Master no play, The slave plays the 5.1 channels separately */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

//        TRACE(0,"session is [%d]. smpcnt %d. Len %d.", i, smpcnt, Len);

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR | VOW_CHANNEL_FC | VOW_CHANNEL_LFE | \
                VOW_CHANNEL_BL | VOW_CHANNEL_BR)) {
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 6, &usb_timestamp);
        }
    }
#endif

#endif  /* USB_AUDIO_RECV_CHAN = 6 */
}

static void wusb_audio_input_ch51_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 6)
    VOW_SESSION *session = NULL;
    uint32_t i = 0;
    SceneParam_t *sceneparam = vow_get_scene_param();
#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    /* if 7.1 ch mode slave > 2, Default : Master no play, The slave plays the FL FR FC LFE BL and BR channels separately */
    static uint8_t *curr_buf0 = wusb_ch0_buf0;
    static uint8_t *curr_buf1 = wusb_ch1_buf0;
    static uint8_t *curr_buf2 = wusb_ch2_buf0;
    static uint8_t *curr_buf3 = wusb_ch3_buf0;
    static uint8_t *curr_buf4 = wusb_ch4_buf0;
    static uint8_t *curr_buf5 = wusb_ch5_buf0;
    static uint8_t *next_buf0 = wusb_ch0_buf1;
    static uint8_t *next_buf1 = wusb_ch1_buf1;
    static uint8_t *next_buf2 = wusb_ch2_buf1;
    static uint8_t *next_buf3 = wusb_ch3_buf1;
    static uint8_t *next_buf4 = wusb_ch4_buf1;
    static uint8_t *next_buf5 = wusb_ch5_buf1;
    static uint64_t curr_usb_timestamp = usb_timestamp;

    short *ptr0 = (short *)next_buf0;    /* CHANNEL_FL */
    short *ptr1 = (short *)next_buf1;    /* CHANNEL_FR */
    short *ptr2 = (short *)next_buf2;    /* CHANNEL_FC */
    short *ptr3 = (short *)next_buf3;    /* CHANNEL_LFE */
    short *ptr4 = (short *)next_buf4;    /* CHANNEL_BL */
    short *ptr5 = (short *)next_buf5;    /* CHANNEL_BR */

    short *data = (short*)pData;

    /* --- Data Register --- */
    /* transfer format(byte) : |dataFL0|dataFL1|dataFR0|dataFR1|dataFC0|dataFC1|dataLFE0|dataLFE1|dataBL0|dataBL1|dataBR0|dataBR1|...| =>
     * wusb_ch0 : |dataFL0|dataFL1|dataFL2|dataFL3|...|
     * wusb_ch1 : |dataFR0|dataFR1|dataFR2|dataFR3|...|
     * wusb_ch2 : |dataFC0|dataFC1|dataFC2|dataFC3|...|
     * wusb_ch3 : |dataLFE0|dataLFE1|dataLFE2|dataLFE3|...|
     * wusb_ch4 : |dataBL0|dataBL1|dataBL2|dataBL3|...|
     * wusb_ch5 : |dataBR0|dataBR1|dataBR2|dataBR3|...|
     */

    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *data++;
        *ptr1++ = *data++;

        *ptr2++ = *data++;
        *ptr3++ = *data++;

        *ptr4++ = *data++;
        *ptr5++ = *data++;
    }

    /* --- Buf addr Updata --- */
    if (usb_recv_finish_flag == true) {
        usb_recv_finish_flag = false;
        curr_usb_timestamp = usb_timestamp;
        if (curr_buf0 == wusb_ch0_buf0) {
            curr_buf0 = wusb_ch0_buf1;
            curr_buf1 = wusb_ch1_buf1;
            curr_buf2 = wusb_ch2_buf1;
            curr_buf3 = wusb_ch3_buf1;
            curr_buf4 = wusb_ch4_buf1;
            curr_buf5 = wusb_ch5_buf1;
            next_buf0 = wusb_ch0_buf0;
            next_buf1 = wusb_ch1_buf0;
            next_buf2 = wusb_ch2_buf0;
            next_buf3 = wusb_ch3_buf0;
            next_buf4 = wusb_ch4_buf0;
            next_buf5 = wusb_ch5_buf0;
        } else {
            curr_buf0 = wusb_ch0_buf0;
            curr_buf1 = wusb_ch1_buf0;
            curr_buf2 = wusb_ch2_buf0;
            curr_buf3 = wusb_ch3_buf0;
            curr_buf4 = wusb_ch4_buf0;
            curr_buf5 = wusb_ch5_buf0;
            next_buf0 = wusb_ch0_buf1;
            next_buf1 = wusb_ch1_buf1;
            next_buf2 = wusb_ch2_buf1;
            next_buf3 = wusb_ch3_buf1;
            next_buf4 = wusb_ch4_buf1;
            next_buf5 = wusb_ch5_buf1;
        }
    } else {
        next_buf0 += Len;
        next_buf1 += Len;
        next_buf2 += Len;
        next_buf3 += Len;
        next_buf4 += Len;
        next_buf5 += Len;
    }

    /* --- Data Transmition --- */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
//            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }
        if (i == (accumula_usb_recv_len / (usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM))) {
            if (session->audio_channel == VOW_CHANNEL_FL) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf0, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_FR) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf1, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_FC) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf2, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_LFE) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf3, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_BL) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf4, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_BR) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf5, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
        }
    }

#else   /* Single slave or No slave */
    /* if 5.1 ch mode slave = 1, Default : Master no play, The slave plays the 5.1 channels separately */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR | VOW_CHANNEL_FC | VOW_CHANNEL_LFE | \
                VOW_CHANNEL_BL | VOW_CHANNEL_BR)) {
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 6, &usb_timestamp);
        }
    }

#endif

#endif  /* USB_AUDIO_RECV_CHAN = 6 */
}

static void wusb_audio_input_ch2_24bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 2)
    VOW_SESSION *session = NULL;
    uint32 i = 0;
    SceneParam_t *sceneparam = vow_get_scene_param();

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    /* if 2.0 ch mode slave > 2, Default : Master no play, The slave plays the left and right channels separately */
    static uint8_t *curr_buf0 = wusb_ch0_buf0;
    static uint8_t *curr_buf1 = wusb_ch1_buf0;
    static uint8_t *next_buf0 = wusb_ch0_buf1;
    static uint8_t *next_buf1 = wusb_ch1_buf1;
    static uint64_t curr_usb_timestamp = usb_timestamp;

    unsigned char *ptr0 = NULL;
    unsigned char *ptr1 = NULL;
    unsigned char *ptr = (uint8_t*)pData;

    /* --- Data Register --- */
    /* transfer format(byte) : |dataL0|dataL1|dataR0|dataR1|dataL2|dataL3|...| =>
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|...|
     */
    ptr0 = (uint8_t *)next_buf0;    /* FL */
    ptr1 = (uint8_t *)next_buf1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
    }


    /* --- Buf addr Updata --- */
    if (usb_recv_finish_flag == true) {
        usb_recv_finish_flag = false;
        curr_usb_timestamp = usb_timestamp;
        if (curr_buf0 == wusb_ch0_buf0) {
            curr_buf0 = wusb_ch0_buf1;
            curr_buf1 = wusb_ch1_buf1;
            next_buf0 = wusb_ch0_buf0;
            next_buf1 = wusb_ch1_buf0;
        } else {
            curr_buf0 = wusb_ch0_buf0;
            curr_buf1 = wusb_ch1_buf0;
            next_buf0 = wusb_ch0_buf1;
            next_buf1 = wusb_ch1_buf1;
        }
    } else {
        next_buf0 += Len;
        next_buf1 += Len;
    }

    /* --- Data Transmition --- */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }
        if (i == (accumula_usb_recv_len / (usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM))) {
            if (session->audio_channel == VOW_CHANNEL_FL) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf0, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_FR) {
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf1, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
        }
    }

#else   /* Single slave or No slave */
#ifdef __VOW_SPLIT_L_R_CHANNEL__
    unsigned char *ptr0 = NULL;
    unsigned char *ptr1 = NULL;
    unsigned char *ptr = (uint8_t*)pData;

    /* transfer format(byte) : |dataL0|dataL1|dataL2|dataR0|dataR1|dataR2|dataL3|dataL4|dataL5|dataR3|...| => 
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|dataL4|dataL5|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|dataR4|dataR5|...|
     */

    ptr0 = (uint8_t *)wusb_ch0;    /* FL */
    ptr1 = (uint8_t *)wusb_ch1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
        *ptr1++ = *ptr++;
    }

    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)wusb_ch0, Len, &usb_timestamp);
        if (session->audio_channel == VOW_CHANNEL_FR)
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)wusb_ch1, Len, &usb_timestamp);
    }

    if (wusb_main_vsess_id < 0)
        return;
    else {
        session = vow_get_session(wusb_main_vsess_id);
        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wusb_main_vsess_id, (uint8_t *)wusb_ch0, Len, &usb_timestamp);
        if (session->audio_channel == VOW_CHANNEL_FR)
            vow_session_input(wusb_main_vsess_id, (uint8_t *)wusb_ch1, Len, &usb_timestamp);
    }
    return;
#else
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
//            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR))
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 2, &usb_timestamp);
    }

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
        if (wusb_main_vsess_id < 0)
            return;
        else
            vow_session_input(wusb_main_vsess_id, (uint8_t *)pData, Len * 2, &usb_timestamp);
#else
        if (wusb_main_vsess_id >= 0) {
            /* 96k/24bit/2ch >> 48k/16bit/2ch */
            /* transfer format(byte) : |data0|data1|data2|data3|data4|data5|...| => |data4|data5|...| */
//            TRACE(0,"session is %d. %d %d", wusb_main_vsess_id, smpcnt, Len);
            uint32_t obj_len = Len * 2 / 3;
            smpcnt = smpcnt / 2;    /* 96k -> 48k */
            uint8_t *temporary_ptr = (uint8_t*)temporary_buf;
            for (i = 0; i < smpcnt; i++) {
                *temporary_ptr++ = *(pData + (i * 12) + 7);
                *temporary_ptr++ = *(pData + (i * 12) + 8);
                *temporary_ptr++ = *(pData + (i * 12) + 10);
                *temporary_ptr++ = *(pData + (i * 12) + 11);
            }

            vow_session_input(wusb_main_vsess_id, (uint8_t *)temporary_buf, obj_len, &usb_timestamp);
        }
#endif

    return;
#endif
#endif

#endif  /* USB_AUDIO_RECV_CHAN = 2 */
}

static void wusb_audio_input_ch2_16bit_data_handle(uint8_t *pData, uint32 smpcnt, uint32 Len)
{
#if (USB_AUDIO_RECV_CHAN == 2)
    VOW_SESSION *session = NULL;
    uint32 i = 0;
    SceneParam_t *sceneparam = vow_get_scene_param();

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    /* if 2.0 ch mode slave > 2, Default : Master no play, The slave plays the left and right channels separately */
    static uint8_t *curr_buf0 = wusb_ch0_buf0;
    static uint8_t *curr_buf1 = wusb_ch1_buf0;
    static uint8_t *next_buf0 = wusb_ch0_buf1;
    static uint8_t *next_buf1 = wusb_ch1_buf1;
    static uint64_t curr_usb_timestamp = usb_timestamp;

    unsigned short *ptr0 = NULL;
    unsigned short *ptr1 = NULL;
    unsigned short *ptr = (uint16_t*)pData;

    /* --- Data Register --- */
    /* transfer format(byte) : |dataL0|dataL1|dataR0|dataR1|dataL2|dataL3|...| =>
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|...|
     */
    ptr0 = (uint16_t *)next_buf0;    /* FL */
    ptr1 = (uint16_t *)next_buf1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
    }

    /* --- Buf addr Updata --- */
    if (usb_recv_finish_flag == true) {
        usb_recv_finish_flag = false;
        curr_usb_timestamp = usb_timestamp;
        if (curr_buf0 == wusb_ch0_buf0) {
            curr_buf0 = wusb_ch0_buf1;
            curr_buf1 = wusb_ch1_buf1;
            next_buf0 = wusb_ch0_buf0;
            next_buf1 = wusb_ch1_buf0;
        } else {
            curr_buf0 = wusb_ch0_buf0;
            curr_buf1 = wusb_ch1_buf0;
            next_buf0 = wusb_ch0_buf1;
            next_buf1 = wusb_ch1_buf1;
        }
    } else {
        next_buf0 += Len;
        next_buf1 += Len;
    }

    /* --- Data Transmition --- */
    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }
        if (i == (accumula_usb_recv_len / (usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM))) {
            if (session->audio_channel == VOW_CHANNEL_FL) {
//                TRACE(0, "--------------- FL");
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf0, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
            if (session->audio_channel == VOW_CHANNEL_FR) {
//                TRACE(0, "--------------- FR");
                vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)curr_buf1, Len * VOWLAN_RF_OUTPUT_NUM, &usb_timestamp);
            }
        }
    }

#else   /* Single slave or No slave */
#ifdef __VOW_SPLIT_L_R_CHANNEL__
    unsigned short *ptr0 = NULL;
    unsigned short *ptr1 = NULL;
    unsigned short *ptr = (uint16_t*)pData;

    /* transfer format(byte) : |dataL0|dataL1|dataR0|dataR1|dataL2|dataL3|...| =>
     * wusb_ch0 : |dataL0|dataL1|dataL2|dataL3|...|
     * wusb_ch1 : |dataR0|dataR1|dataR2|dataR3|...|
     */
    ptr0 = (uint16_t *)wusb_ch0;    /* FL */
    ptr1 = (uint16_t *)wusb_ch1;    /* FR */
    for (i = 0; i < smpcnt; i++) {
        *ptr0++ = *ptr++;
        *ptr1++ = *ptr++;
    }

    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)wusb_ch0, Len, &usb_timestamp);
        if (session->audio_channel == VOW_CHANNEL_FR)
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)wusb_ch1, Len, &usb_timestamp);
    }

    if (wusb_main_vsess_id < 0)
        return;
    else {
        session = vow_get_session(wusb_main_vsess_id);
        if (session->audio_channel == VOW_CHANNEL_FL)
            vow_session_input(wusb_main_vsess_id, (uint8_t *)wusb_ch0, Len, &usb_timestamp);
        if (session->audio_channel == VOW_CHANNEL_FR)
            vow_session_input(wusb_main_vsess_id, (uint8_t *)wusb_ch1, Len, &usb_timestamp);
    }

    return;
#else

    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if (session == NULL) {
            TRACE(0,"%s, session[%d] is NULL", __func__, i);
            break;
        }

        if (session->audio_channel == (VOW_CHANNEL_FL | VOW_CHANNEL_FR))
            vow_session_input(wusb_tran_vsess_id[i], (uint8_t *)pData, Len * 2, &usb_timestamp);
    }

    if (wusb_main_vsess_id < 0)
        return;
    else
        vow_session_input(wusb_main_vsess_id, (uint8_t *)pData, Len * 2, &usb_timestamp);

    return;
#endif
#endif

#endif  /* USB_AUDIO_RECV_CHAN = 2 */
}

#ifdef VOWLAN_SUPPORT_OUTPUT_BY_USB
static uint32_t wusb_data_send_buf_queue_lock(void)
{
    return int_lock();
}

static void wusb_data_send_buf_queue_unlock(uint32_t lock)
{
    int_unlock(lock);
}

int wusb_data_send_buf_enqueue(uint8_t *indata, uint32_t len, uint64_t play_time)
{
    /* data enqueue */
    uint32_t lock = wusb_data_send_buf_queue_lock();

    uint8_t i;

    for (i = 0; i < WUSB_DATA_SEND_BUF_NUM; i++) {
        if (wusb_data_send_buf[i].used_flag == VOW_DATA_UPDATA_BUF_UNUSED_FLAG) {
            wusb_data_send_buf[i].used_flag = VOW_DATA_UPDATA_BUF_USED_FLAG;
            wusb_data_send_buf[i].play_time = play_time;
            vow_memcpy(wusb_data_send_buf[i].data, indata, len);
            wusb_data_send_buf_queue_unlock(lock);
            return 0;
        }
    }

    uint64_t oldest_time = 0xffffffffffffffff;
    uint64_t oldest_idx = 0;
    for (i = 0; i < WUSB_DATA_SEND_BUF_NUM; i++) {
        if (wusb_data_send_buf[i].play_time < oldest_time) {
            oldest_time = wusb_data_send_buf[i].play_time;
            oldest_idx = i;
        }
    }

    wusb_data_send_buf[oldest_idx].used_flag = VOW_DATA_UPDATA_BUF_USED_FLAG;
    wusb_data_send_buf[oldest_idx].play_time = play_time;
    vow_memcpy(wusb_data_send_buf[oldest_idx].data, indata, len);

    wusb_data_send_buf_queue_unlock(lock);

    return 0;
}

int wusb_data_send_buf_dequeue(uint8_t *outdata, uint32_t len)
{
    int ret = -1;

    /* data dequeue */
    uint32_t lock = wusb_data_send_buf_queue_lock();

    uint8_t i;
    uint64_t oldest_time = 0xffffffffffffffff;
    uint64_t oldest_idx = 0;
    for (i = 0; i < WUSB_DATA_SEND_BUF_NUM; i++) {
        if (wusb_data_send_buf[i].used_flag == VOW_DATA_UPDATA_BUF_USED_FLAG) {
            if (wusb_data_send_buf[i].play_time < oldest_time) {
                oldest_time = wusb_data_send_buf[i].play_time;
                oldest_idx = i;
            }
        }
    }

    if(oldest_time != 0xffffffffffffffff) {
        wusb_data_send_buf[oldest_idx].used_flag = VOW_DATA_UPDATA_BUF_UNUSED_FLAG;
        wusb_data_send_buf[oldest_idx].play_time = 0;
        vow_memcpy(outdata, wusb_data_send_buf[oldest_idx].data, len);
        wusb_data_send_buf_queue_unlock(lock);
        return 0;
    }

    wusb_data_send_buf_queue_unlock(lock);

    return ret;
}

int wusb_stream_send(uint8_t *data, uint32_t len, uint64_t play_time)
{
    return wusb_data_send_buf_enqueue(data, len, play_time);
}
#endif

static void wusb_audio_input_data_codec(void const *pArg)
{
    uint8_t *pData = NULL ;
    uint32_t smpcnt = 0;
    uint32_t Len = 0;
    TRACE(0, "%s start %d", __func__, __LINE__);

    while (1) {
        osSemaphoreWait(usb_irq_sem_id, osWaitForever);

        /* update tsf */
        vow_update_wifitsf();

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
        Len = usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM;
        pData = vow_usb_recv_buffer_new;
#else   /* Single slave or No slave */
        Len = usb_recv_payload_len;
#if defined(__VOW_SPLIT_L_R_CHANNEL__) && (USB_AUDIO_RECV_CHAN == 2)
        /* no audio data move */
        pData = vow_usb_recv_buffer_new;
#else
        /* audio data move */
        vow_memset(vow_usb_recv_buffer_old, 0, usb_recv_payload_len);
        vow_memcpy(vow_usb_recv_buffer_old, vow_usb_recv_buffer_new, usb_recv_payload_len);
        pData = vow_usb_recv_buffer_old;
#endif
#endif
        if (vow_wifi_get_ap_status() == 0) {
            //todo
            TRACE(0,"usb: softap <==> sta link is not online...");
            return;
        }

        AUD_BITS_T sample_depth = rf_out_audio_cfg.sample_depth;
        smpcnt = (((sample_depth == AUD_BITS_16) ? (VOW_USB_RECV_MAX_PAYLOAD_LEN >> 1) : (VOW_USB_RECV_MAX_PAYLOAD_LEN >> 2)) / g_audio_channel); /* every ch sample count */
#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)
        smpcnt /= VOWLAN_RF_OUTPUT_NUM;
#endif
        Len /= g_audio_channel;
        switch (sample_depth) {
        case AUD_BITS_16:
            if (g_audio_channel == 8) {
                wusb_audio_input_ch71_16bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 6) {
                wusb_audio_input_ch51_16bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 2) {
                wusb_audio_input_ch2_16bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0, "%s sample_depth 16bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
            break;
        case AUD_BITS_24:
            if (g_audio_channel == 8) {
                wusb_audio_input_ch71_24bit_data_handle(pData, smpcnt, Len);
            } else if(g_audio_channel == 6) {
                wusb_audio_input_ch51_24bit_data_handle(pData, smpcnt, Len);
            } else if (g_audio_channel == 2) {
                wusb_audio_input_ch2_24bit_data_handle(pData, smpcnt, Len);
            } else {
                TRACE(0, "%s sample_depth 24bit. Not supported invalid g_audio_channel=%d for now.", __func__, g_audio_channel);
            }
            break;
        default:
            TRACE(0, "%s invalid depth=%d", __func__, sample_depth);
            break;
        }
    }
}

extern "C" void usb_codec_task_init(void)
{
    TRACE(0, "enter %s %d", __func__, __LINE__);

    if (usb_irq_sem_id == NULL)
        usb_irq_sem_id = osSemaphoreCreate(osSemaphore(usb_irq_sem), 0);

    ASSERT(usb_irq_sem_id, "%s failed to create usb irq sem", __func__);

    if (usb_codec_tid == NULL)
        usb_codec_tid = osThreadCreate(osThread(wusb_audio_input_data_codec), NULL);

    ASSERT(usb_codec_tid, "%s failed to create usb codec task", __func__);

    usb_os_init();

    return;
}

osThreadId wusb_audio_codec_thread_id(void)
{
    return usb_codec_tid;
}

static uint32_t wusb_audio_input(uint8_t* ptrBuf, uint32_t length)
{
//    TRACE(0, "%s %d", __func__, length);
#ifdef BLE_WIFI_DUAL_MODE_DONGLE
    if (vow_get_status() != VOW_STATUS_STREAM_TRANSFE)
        return 0;
#endif

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)    /* Multiple slave */
    if ((usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM) == length) {
        accumula_usb_recv_len += length;
        vow_usb_recv_buffer_new = ptrBuf;

        if (accumula_usb_recv_len == usb_recv_payload_len) {
            accumula_usb_recv_len = 0;
            usb_recv_finish_flag = true;
            usb_timestamp = hal_wifitsf_timer_get() + rf_out_audio_cfg.delay;
        }

        osSemaphoreRelease(usb_irq_sem_id);
    } else
        TRACE(0, "USB input error, len=%d, match len=%d", length, (usb_recv_payload_len / VOWLAN_RF_OUTPUT_NUM));
#else   /* Single slave or No slave */
    if (usb_recv_payload_len == length) {
        vow_usb_recv_buffer_new = ptrBuf;

#if (WUSB_DEBUG == 1)
        SceneParam_t * param = vow_get_scene_param();
        uint64_t last_usb_timestamp = usb_timestamp;
#endif

        usb_timestamp = hal_wifitsf_timer_get() + rf_out_audio_cfg.delay;

#if (WUSB_DEBUG == 1)
        int32_t usb_input_diff = usb_timestamp - last_usb_timestamp;
        if (usb_input_diff <= (param->play_interval / 2) || \
                usb_input_diff >= (param->play_interval * 2)) {
            TRACE(0,"[%s] usb interval error. usb_input_diff = %d", __func__, usb_input_diff);
        }
#endif

        osSemaphoreRelease(usb_irq_sem_id);
    } else
        TRACE(0, "USB input error, len=%d, match len=%d", length, usb_recv_payload_len);
#endif

    return length;
}

static void wusb_audio_output(uint8_t* ptrBuf, uint32_t length)
{
#ifdef VOWLAN_SUPPORT_OUTPUT_BY_USB
    int ret;
    ret = wusb_data_send_buf_dequeue(ptrBuf, length);
    if (ret < 0) {
        TRACE(0, "%s error. wusb_data_send queue is empty.", __func__);
    }
#endif
}

void vow_cmd_input_playback_stream_onoff(bool on)
{
    uint8_t cmd_data[VOW_FRAME_MAX_CMD_LEN] = {0};
    cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 7);    /* DIR : M to S. */
    cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 0);    /* TYPE : start or stop CMD */
    cmd_data[VOW_CMD_SYNC_CFG_CONT2_SEQ] = on;
    TRACE(0, "vow_cmd_input %s", on ? "start" : "stop");
    vow_cmd_input(VOW_CMD_TYPE_SYNC_CFG, cmd_data, VOW_FRAME_SYNC_CFG_CMD_LEN);
}

#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
#define BLE_WIFI_AUDIO_MANAGER_DEBOUCE
#ifdef BLE_WIFI_AUDIO_MANAGER_DEBOUCE
static void app_ble_wifi_audio_manager_debounce_timer_handler(void const *param);
osTimerDef(APP_BLE_WIFI_AUDIO_MANAGER_DEBOUNCE_TIMER, app_ble_wifi_audio_manager_debounce_timer_handler);
static osTimerId app_ble_wifi_audio_manager_debounce_timer = NULL;
#define APP_BLE_WIFI_AUDIO_MANAGER_DEBOUNCE_TIME_IN_MS  150
static void app_ble_wifi_audio_manager_debounce_timer_handler(void const *param)
{
    static bool cap = false, play = false;
    POSSIBLY_UNUSED static bool low_latency_mode = false;

    TRACE(1, "curr audio status: cap(%d) play(%d)", cap, play);
    TRACE(1, "new audio status: cap(%d) play(%d)", usb_capture_stream_started, usb_playback_stream_started);

#if defined(BLE_WIFI_DUAL_MODE_DONGLE)
    if (cap == false && usb_capture_stream_started == true) {
        wusb_lea_stream_process_init(AUD_STREAM_CAPTURE);
        if (lea_stream_func_list.capture_start_cb) {
            lea_stream_func_list.capture_start_cb();
        }
        cap = true;
    }

    if (cap == true && usb_capture_stream_started == false) {
        if (lea_stream_func_list.capture_stop_cb) {
            lea_stream_func_list.capture_stop_cb();
        }
        cap = false;
    }
    return;
#elif defined(BLE_WIFI_DUAL_CHIP_MODE)
    if (cap == false && usb_capture_stream_started == true) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_CAPTURE_STREAM_START, 0, 0);
        cap = true;

        if (low_latency_mode == true) {
            // app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_STOP, low_latency_mode, 0);
            low_latency_mode = false;
            app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_START, low_latency_mode, 0);
        }
    }

    if (cap == true && usb_capture_stream_started == false) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_CAPTURE_STREAM_STOP, 0, 0);
        cap = false;

        if (play == true && low_latency_mode == false) {
            app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_STOP, low_latency_mode, 0);
            low_latency_mode = true;
            // app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_START, low_latency_mode, 0);
        }
    }

    if (play == false && usb_playback_stream_started == true) {
        low_latency_mode = (cap == true) ? false : true;
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_START, low_latency_mode, 0);
        play = true;
    }

    if (play == true && usb_playback_stream_started == false) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_STOP, low_latency_mode, 0);
        play = false;
    }
#endif
}

void app_ble_wifi_audio_manager_debounce_timer_start(void)
{
    if (NULL == app_ble_wifi_audio_manager_debounce_timer) {
        app_ble_wifi_audio_manager_debounce_timer = osTimerCreate(osTimer(APP_BLE_WIFI_AUDIO_MANAGER_DEBOUNCE_TIMER), osTimerOnce, NULL);
    }

    ASSERT(app_ble_wifi_audio_manager_debounce_timer, "%s create os timer failed.", __func__);

    osTimerStop(app_ble_wifi_audio_manager_debounce_timer);
    osTimerStart(app_ble_wifi_audio_manager_debounce_timer, APP_BLE_WIFI_AUDIO_MANAGER_DEBOUNCE_TIME_IN_MS);
}
#endif
#endif

static void wusb_playback_start(void)
{
    TRACE(0, "%s", __func__);
#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
    TRACE(1, "capture:%d, playback:%d", lea_capture_stream_started, lea_playback_stream_started);
    usb_playback_stream_started = true;
#ifndef BLE_WIFI_AUDIO_MANAGER_DEBOUCE
    if ((lea_capture_stream_started == true) && (lea_playback_stream_started == false)) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_START, 0, 0);
        lea_playback_stream_started = true;
    }
#else
    app_ble_wifi_audio_manager_debounce_timer_start();
#endif
#if defined(BLE_WIFI_DUAL_MODE_DONGLE)
    osThreadSetPriority(app_os_tid_get(), osPriorityAboveNormal);
    osThreadSetPriority(app_get_bt_thread_id(), osPriorityAboveNormal);
    osThreadSetPriority(vow_session_thread_id(), osPriorityHigh);
    osThreadSetPriority(wusb_audio_codec_thread_id(), osPriorityHigh);
#endif
#endif
}

static void wusb_playback_stop(void)
{
    TRACE(0, "%s", __func__);
#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
    TRACE(1, "capture:%d, playback:%d", lea_capture_stream_started, lea_playback_stream_started);
    usb_playback_stream_started = false;
#ifndef BLE_WIFI_AUDIO_MANAGER_DEBOUCE
    if (lea_playback_stream_started == true) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_STOP, 0, 0);
        lea_playback_stream_started = false;
    }
#else
    app_ble_wifi_audio_manager_debounce_timer_start();
#endif
#if defined(BLE_WIFI_DUAL_MODE_DONGLE)
    osThreadSetPriority(app_os_tid_get(), osPriorityHigh);
    osThreadSetPriority(app_get_bt_thread_id(), osPriorityHigh);
    osThreadSetPriority(vow_session_thread_id(), osPriorityAboveNormal);
    osThreadSetPriority(wusb_audio_codec_thread_id(), osPriorityAboveNormal);
#endif
#endif
}

static void wusb_capture_start(void)
{
    TRACE(0, "%s", __func__);
#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
    usb_capture_stream_started = true;
#ifndef BLE_WIFI_AUDIO_MANAGER_DEBOUCE
    if (lea_capture_stream_started == false) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_CAPTURE_STREAM_START, 0, 0);
        lea_capture_stream_started = true;
    }

    if (usb_playback_stream_started == true) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_START, 0, 0);
        lea_playback_stream_started = true;
    }
#else
    app_ble_wifi_audio_manager_debounce_timer_start();
#endif
#endif
}

static void wusb_capture_stop(void)
{
    TRACE(0, "%s", __func__);
#if defined(VOWLAN_MASTER) && (defined(BLE_WIFI_DUAL_CHIP_MODE) || defined(BLE_WIFI_DUAL_MODE_DONGLE))
    usb_capture_stream_started = false;
#ifndef BLE_WIFI_AUDIO_MANAGER_DEBOUCE
    if (lea_capture_stream_started == true) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_CAPTURE_STREAM_STOP, 0, 0);
        lea_capture_stream_started = false;
    }

    if (lea_playback_stream_started == true) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_STREAM_STOP, 0, 0);
        lea_playback_stream_started = false;
    }
#else
    app_ble_wifi_audio_manager_debounce_timer_start();
#endif
#endif
}

static uint8_t updata_vol;
static uint8_t wusb_playback_vol = TGT_VOLUME_LEVEL_MAX;
static void wusb_playback_vol_changed(uint32_t level)
{
    TRACE(0, "%s %d", __func__, level);
    SceneParam_t * sceneparam = vow_get_scene_param();
    VOW_SESSION *session = NULL;
    uint8_t i;

    wusb_playback_vol = level;

    if (rf_out_audio_cfg.vol != level) {
        for (i = 0; i < sceneparam->rf_tx_number; i++) {
            session = vow_get_session(i);
            if (session == NULL) {
                TRACE(0,"%s, session[%d] is NULL", __func__, i);
                break;
            }

            session->audio_cfg.vol = level;
        }

        uint8_t cmd_data[VOW_FRAME_MAX_CMD_LEN] = {0};
        cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 7);    /* DIR : M to S. */
        cmd_data[VOW_CMD_SYNC_CFG_TYPE_SEQ] |= (1 << 2);    /* TYPE : set vol CMD */

        cmd_data[VOW_CMD_SYNC_CFG_CONT2_SEQ] = level;

        vow_cmd_input(VOW_CMD_TYPE_SYNC_CFG, cmd_data, VOW_FRAME_SYNC_CFG_CMD_LEN);

        rf_out_audio_cfg.vol = level;
    }

#if defined(BLE_WIFI_DUAL_CHIP_MODE) && defined(VOWLAN_MASTER)
    static uint32_t playback_vol_level = 0;
    if (playback_vol_level != level) {
        app_ble_wifi_dual_mode_audio_manager_send_request_remotely(APP_BLE_WIFI_COMM_PLAYBACK_VOLUME_CTRL, level, 0);
        playback_vol_level = level;
    }
#endif

    if ((updata_vol == rf_out_audio_cfg.vol) || (level == 0) || (level == 16)) {
        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_UP, 0);
        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_DOWN, 0);
    }

    if (local_out_audio_cfg.vol != level) {
        if (wusb_main_vsess_id < 0)
            return;
        else {
            ///TODO:
        }
    }
}

void wusb_playback_vol_updata(uint32_t level)
{
    TRACE(0, "%s %d", __func__, level);

    if (rf_out_audio_cfg.vol < level) {
        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_UP, 1);
//        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_UP, 0);
    } else if (rf_out_audio_cfg.vol > level) {
        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_DOWN, 1);
//        usb_audio_hid_set_event(USB_AUDIO_HID_VOL_DOWN, 0);
    }

    updata_vol = level;

    if (local_out_audio_cfg.vol != level) {
        if (wusb_main_vsess_id < 0)
            return;
        else {
            ///TODO:
        }
    }
}

static uint32_t wusb_data_playback(uint8_t * pcm_buf, uint32_t len)
{
#ifdef BLE_WIFI_DUAL_MODE_DONGLE
    wusb_lea_pcmbuff_put(AUD_STREAM_PLAYBACK, pcm_buf, len);
    wusb_lea_shot_playback_stream_process();
#endif
    return 0;
}

static void wusb_data_capture(uint8_t * pcm_buf, uint32_t len)
{
#ifdef BLE_WIFI_DUAL_MODE_DONGLE
    wusb_lea_pcmbuff_get(AUD_STREAM_CAPTURE, pcm_buf, len);
    wusb_lea_shot_capture_stream_process();
#endif
    return;
}

static const struct USB_AUDIO_SOURCE_EVENT_CALLBACK_T wifi_usb_audio_stream_func_cb_list =
{
#ifdef BLE_WIFI_DUAL_MODE_DONGLE
    .src_type = USB_AUDIO_SOURCE_BW,
#else
    .src_type = USB_AUDIO_SOURCE_WIFI,
#endif
    .data_playback_cb = wusb_data_playback,
    .data_recv_cb = wusb_audio_input,
    .data_capture_cb = wusb_data_capture,
    .data_send_cb = wusb_audio_output,
    .data_prep_cb = NULL,

    .playback_start_cb = wusb_playback_start,
    .playback_stop_cb = wusb_playback_stop,

    .capture_start_cb = wusb_capture_start,
    .capture_stop_cb = wusb_capture_stop,

    .playback_vol_change_cb = wusb_playback_vol_changed,
};

extern "C" void wusb_set_wifi_audio_source(void)
{
    usb_audio_source_config_init(&wifi_usb_audio_stream_func_cb_list);
    TRACE(0, "register wifi audio as the usb audio success!");
}

void wusb_session_updata(VOW_DEVICE *vodev, SceneParam_t *sceneparam)
{
    uint8_t i = 0;
    VOW_SESSION *session = NULL;

    for (i = 0; i < sceneparam->rf_tx_number; i++) {
        session = vow_get_session(i);
        if ((wusb_tran_vsess_id[i] != -1) && (session == NULL)) {   /* dev off line -> reconnection */
            wusb_tran_vsess_id[i] = vow_session_alloc(vodev, &rf_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_WIFI);
            if (wusb_tran_vsess_id[i] < 0) {
                TRACE(0, "%s:%d, vow_session_alloc fail, wusb_tran_vsess_id[%d]=%d", __func__, __LINE__,i, wusb_tran_vsess_id[i]);
                break;
            }
            TRACE(0, "wusb_tran_vsess_id[%d]:%d", i, wusb_tran_vsess_id[i]);
        }
    }
}

void wusb_init(VOW_DEVICE *vodev, SceneParam_t *sceneparam)
{
    uint32_t submit_size = 0;

    if (!vodev) {
        TRACE(0, "wusb_init fail, vodev error");
        return;
    }

    /* net audio session alloc */
    if (sceneparam->output_device & VOW_OUTPUT_WIFI) {
        rf_out_audio_cfg.sample_rate = sceneparam->sample_freq;
        rf_out_audio_cfg.sample_depth = sceneparam->sample_depth;
        rf_out_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;     /* paly retardation */

        rf_out_audio_cfg.mute_state = 0;
        rf_out_audio_cfg.channel_num = sceneparam->play_ch_num;;
        rf_out_audio_cfg.vol = wusb_playback_vol;  //TGT_VOLUME_LEVEL_MAX as default.
        rf_out_audio_cfg.burst_size = 1;

        for (uint8_t i = 0; i < sceneparam->rf_tx_number; i++) {
            wusb_tran_vsess_id[i] = -1;
            wusb_tran_vsess_id[i] = vow_session_alloc(vodev, &rf_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_WIFI);
            if (wusb_tran_vsess_id[i] < 0) {
                TRACE(0, "%s:%d, vow_session_alloc fail, wusb_tran_vsess_id[%d]=%d", __func__, __LINE__,i, wusb_tran_vsess_id[i]);
                goto FAIL;
            }
            TRACE(0, "wusb_tran_vsess_id[%d]:%d", i, wusb_tran_vsess_id[i]);
        }
    }

#ifndef VOWLAN_SUPPORT_OUTPUT_BY_USB
    /* local audio session alloc */
    if ((sceneparam->output_device & VOW_OUTPUT_IIS) || (sceneparam->output_device & VOW_OUTPUT_DAC)) {
        local_out_audio_cfg.sample_rate = sceneparam->sample_freq;
        local_out_audio_cfg.sample_depth = sceneparam->sample_depth;
        local_out_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;      /* paly retardation */

        local_out_audio_cfg.mute_state = 0;
        local_out_audio_cfg.channel_num = sceneparam->play_ch_num;
        local_out_audio_cfg.vol = 15;
        local_out_audio_cfg.burst_size = 1;

        if (sceneparam->output_device & VOW_OUTPUT_IIS)
            wusb_main_vsess_id = vow_session_alloc(vodev, &local_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_IIS);
        else if (sceneparam->output_device & VOW_OUTPUT_DAC)
            wusb_main_vsess_id = vow_session_alloc(vodev, &local_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_DAC);

        if (wusb_main_vsess_id < 0) {
            TRACE(0, "%s:%d, vow_session_alloc fail", __func__, __LINE__);
            goto FAIL;
        }
        TRACE(0, "wusb_main_vsess_id:%d", wusb_main_vsess_id);
    }
#else
    /* local audio session alloc */
    if ((sceneparam->output_device & VOW_OUTPUT_IIS) || (sceneparam->output_device & VOW_OUTPUT_DAC)) {
        local_out_audio_cfg.sample_rate = AUD_SAMPRATE_48000;
        local_out_audio_cfg.sample_depth = AUD_BITS_16;
        local_out_audio_cfg.delay = sceneparam->play_interval * VOW_BEAT_TIME_NUM;      /* paly retardation */

        local_out_audio_cfg.mute_state = 0;
        local_out_audio_cfg.channel_num = 2;
        local_out_audio_cfg.vol = 15;
        local_out_audio_cfg.burst_size = 1;

        if (sceneparam->output_device & VOW_OUTPUT_IIS)
            wusb_main_vsess_id = vow_session_alloc(vodev, &local_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_IIS);
        else if (sceneparam->output_device & VOW_OUTPUT_DAC)
            wusb_main_vsess_id = vow_session_alloc(vodev, &local_out_audio_cfg, VOW_SOURCE_USB, VOW_DEST_DAC);

        if (wusb_main_vsess_id < 0) {
            TRACE(0, "%s:%d, vow_session_alloc fail", __func__, __LINE__);
            goto FAIL;
        }
        TRACE(0, "wusb_main_vsess_id:%d", wusb_main_vsess_id);
    }
#endif

    /* submit_size config */
    submit_size = ((sceneparam->sample_freq / 1000) * (sceneparam->sample_depth / 8) * 2 * \
                sceneparam->play_interval) / 1000;

    /* calculate submit_size */
    submit_size *= (sceneparam->usb_input_ch_num / 2); /* such as : 6ch = 2ch * 6 / 2 */
    g_audio_channel = sceneparam->usb_input_ch_num;

    TRACE(0, "%s submit_size=%d, g_audio_channel=%d", __func__, submit_size, g_audio_channel);

    usb_recv_payload_len = submit_size;

#if (defined(VOWLAN_SUPPORT_OUTPUT_BY_RF)) && (VOWLAN_RF_OUTPUT_NUM > 1)
    submit_size /= VOWLAN_RF_OUTPUT_NUM;
    TRACE(0,"alter submit_size = %d, VOWLAN_RF_OUTPUT_NUM = %d", submit_size, VOWLAN_RF_OUTPUT_NUM);
    usb_recv_finish_flag = false;
#endif

#if !defined(BLE_WIFI_DUAL_MODE_DONGLE)
    /* usb audio input init */
    usb_audio_source_config_init(&wifi_usb_audio_stream_func_cb_list);
    wifiusbaudio_entry();
    usb_codec_task_init();
#endif

    return;

FAIL:
    if (sceneparam->output_device & VOW_OUTPUT_WIFI) {
        for (uint8_t i = 0; i < sceneparam->rf_tx_number; i++) {
            if (wusb_tran_vsess_id[i] >= 0) {
                vow_session_free(wusb_tran_vsess_id[i]);
                wusb_tran_vsess_id[i] = -1;
            }
        }
    }

    if (wusb_main_vsess_id >= 0) {
        vow_session_free(wusb_main_vsess_id);
        wusb_main_vsess_id = -1;
    }

    return;
}

#ifdef BLE_WIFI_DUAL_MODE_DONGLE
#include "cqueue.h"
static uint8_t* pcm_queue_buff[AUD_STREAM_NUM] = {NULL};
static CQueue pcm_queue[AUD_STREAM_NUM];
static osMutexId pcmbuff_mutex_id[AUD_STREAM_NUM] = {NULL};
osMutexDef(pcmbuff_playback_mutex);
osMutexDef(pcmbuff_capture_mutex);
static uint8_t playback_pcm_buff[LEA_ENCODE_PCM_FRAME_LEN * 2];
static uint8_t capture_pcm_buff[LEA_DECODE_PCM_FRAME_LEN * 2];

int wusb_lea_pcmbuff_init(enum AUD_STREAM_T stream)
{
    uint32_t pcmbuff_size = 0;
    if (stream == AUD_STREAM_PLAYBACK) {
        if (pcmbuff_mutex_id[stream] == NULL)
            pcmbuff_mutex_id[stream] = osMutexCreate((osMutex(pcmbuff_playback_mutex)));
        pcmbuff_size = LEA_ENCODE_PCM_FRAME_LEN;
    } else if (stream == AUD_STREAM_CAPTURE) {
        if (pcmbuff_mutex_id[stream] == NULL)
            pcmbuff_mutex_id[stream] = osMutexCreate((osMutex(pcmbuff_capture_mutex)));
        pcmbuff_size = LEA_DECODE_PCM_FRAME_LEN;
    } else {
        ASSERT(0, "invalid aud stream: %d", stream);
        return -1;
    }

    ASSERT(pcmbuff_mutex_id[stream], "%s create os mutex failed.", __func__);

    if (stream == AUD_STREAM_PLAYBACK) {
        if (pcm_queue_buff[stream] == NULL) {
            pcm_queue_buff[stream] = (uint8_t*)playback_pcm_buff;
        }

    } else if(stream == AUD_STREAM_CAPTURE) {
        if (pcm_queue_buff[stream] == NULL) {
            pcm_queue_buff[stream] = (uint8_t*)capture_pcm_buff;
        }
    }

    osMutexWait(pcmbuff_mutex_id[stream], osWaitForever);
    InitCQueue(&pcm_queue[stream], pcmbuff_size * 2, pcm_queue_buff[stream]);
    memset(pcm_queue_buff[stream], 0x00, pcmbuff_size * 2);
    osMutexRelease(pcmbuff_mutex_id[stream]);

    return 0;
}

int wusb_lea_pcmbuff_space(enum AUD_STREAM_T stream)
{
    int len;

    osMutexWait(pcmbuff_mutex_id[stream], osWaitForever);
    len = AvailableOfCQueue(&pcm_queue[stream]);
    osMutexRelease(pcmbuff_mutex_id[stream]);

    return len;
}

int wusb_lea_pcmbuff_length(enum AUD_STREAM_T stream)
{
    int len;

    osMutexWait(pcmbuff_mutex_id[stream], osWaitForever);
    len = LengthOfCQueue(&pcm_queue[stream]);
    osMutexRelease(pcmbuff_mutex_id[stream]);

    return len;
}

int wusb_lea_pcmbuff_put(enum AUD_STREAM_T stream, uint8_t *pcm, uint16_t pcm_len)
{
    int status;

    osMutexWait(pcmbuff_mutex_id[stream], osWaitForever);
    status = EnCQueue(&pcm_queue[stream], pcm, pcm_len);
    osMutexRelease(pcmbuff_mutex_id[stream]);

    return status;
}

int wusb_lea_pcmbuff_get(enum AUD_STREAM_T stream, uint8_t *pcm, uint16_t pcm_len)
{
    unsigned char *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    int status;

    osMutexWait(pcmbuff_mutex_id[stream], osWaitForever);
    status = PeekCQueue(&pcm_queue[stream], pcm_len, &e1, &len1, &e2, &len2);
    if (pcm_len == (len1 + len2)) {
        memcpy(pcm, e1, len1);
        memcpy(pcm + len1, e2, len2);
        DeCQueue(&pcm_queue[stream], 0, len1);
        DeCQueue(&pcm_queue[stream], 0, len2);
    }else {
        memset(pcm, 0x00, pcm_len);
        status = -1;
    }
    osMutexRelease(pcmbuff_mutex_id[stream]);

    return status;
}

extern USB_AUDIO_SOURCE_EVENT_CALLBACK_T app_ble_usb_get_leaudio_source(void);
static osThreadId lea_stream_process_tid = NULL;
void wusb_lea_stream_process(void const *argument);
osThreadDef(wusb_lea_stream_process, osPriorityAboveNormal, 1, (4096), "wusb_lea_stream_process");

osThreadId get_lea_stream_process_tid(void)
{
    return lea_stream_process_tid;
}

void wusb_lea_shot_playback_stream_process(void)
{
    if (wusb_lea_pcmbuff_length(AUD_STREAM_PLAYBACK) >= LEA_ENCODE_PCM_FRAME_LEN) {
        osSignalSet(get_lea_stream_process_tid(), BLE_WIFI_DUAL_MODE_SINGAL_PCM_ENCODE);
    }
}

void wusb_lea_shot_capture_stream_process(void)
{
    osSignalSet(get_lea_stream_process_tid(), BLE_WIFI_DUAL_MODE_SINGAL_PCM_DECODE);
}

void wusb_lea_stream_process(void const *argument)
{
    osEvent evt;
    uint32_t signals;

    while (1) 
    {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;

        if (evt.status == osEventSignal) {
            if (signals & BLE_WIFI_DUAL_MODE_SINGAL_PCM_ENCODE) {
                uint8_t tmp_buff[LEA_ENCODE_PCM_FRAME_LEN] = {0};
                if (wusb_lea_pcmbuff_length(AUD_STREAM_PLAYBACK) >= LEA_ENCODE_PCM_FRAME_LEN) {
                    wusb_lea_pcmbuff_get(AUD_STREAM_PLAYBACK, tmp_buff, LEA_ENCODE_PCM_FRAME_LEN);
                    if (lea_stream_func_list.data_playback_cb) {
                        lea_stream_func_list.data_playback_cb(tmp_buff, LEA_ENCODE_PCM_FRAME_LEN);
                    }
                }
            } else if (signals & BLE_WIFI_DUAL_MODE_SINGAL_PCM_DECODE) {
                uint8_t tmp_buff[LEA_DECODE_PCM_FRAME_LEN] = {0};
                if (lea_stream_func_list.data_capture_cb) {
                    lea_stream_func_list.data_capture_cb(tmp_buff, LEA_DECODE_PCM_FRAME_LEN);
                    wusb_lea_pcmbuff_put(AUD_STREAM_CAPTURE, tmp_buff, LEA_DECODE_PCM_FRAME_LEN);
                }
            } else {
                TRACE(1, "warning: recv invalid signals<%d>.", signals);
            }
        }
    }
}

void wusb_lea_stream_process_init(AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_PLAYBACK) {
        lea_stream_func_list.playback_start_cb = app_ble_usb_get_leaudio_source().playback_start_cb;
        lea_stream_func_list.playback_stop_cb  = app_ble_usb_get_leaudio_source().playback_stop_cb;
        lea_stream_func_list.data_playback_cb  = app_ble_usb_get_leaudio_source().data_playback_cb;
        wusb_lea_pcmbuff_init(AUD_STREAM_PLAYBACK);
    } else if (stream == AUD_STREAM_CAPTURE) {
        lea_stream_func_list.capture_start_cb = app_ble_usb_get_leaudio_source().capture_start_cb;
        lea_stream_func_list.capture_stop_cb  = app_ble_usb_get_leaudio_source().capture_stop_cb;
        lea_stream_func_list.data_capture_cb  = app_ble_usb_get_leaudio_source().data_capture_cb;
        wusb_lea_pcmbuff_init(AUD_STREAM_CAPTURE);
    }

    if (lea_stream_process_tid == NULL)
        lea_stream_process_tid = osThreadCreate(osThread(wusb_lea_stream_process), NULL);

    ASSERT(lea_stream_process_tid, "create wusb lea stream thread failed.");

    TRACE(1, "wusb lea %s stream process init done.", (stream == AUD_STREAM_CAPTURE) ? "capture" : "playback");
}
#endif