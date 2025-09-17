/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef SENSOR_HUB_TEST
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_key.h"
#include "hal_location.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtest.h"
#include "hwtimer_list.h"
#include "pmu.h"
#ifdef MCU2SENS_MSG_TEST
#include "rmt_msg_test.h"
#endif
#include "sensor_hub.h"
#include "string.h"
#if defined(VAD_CODEC_TEST) || defined(VAD_APP_TEST)
#include "analog.h"
#include "audioflinger.h"
#include "sens_msg.h"
#endif
#ifdef VAD_APP_TEST
#include "app_sensor_hub.h"
#include "mcu_sensor_hub_app.h"
#ifdef VAD_IF_TEST
#include "mcu_sensor_hub_app_ai.h"
#endif
#endif
#ifdef MINIMA_TEST
#include "hal_psc.h"
#endif

#ifdef SENSOR_BT_TEST
#include "bt_drv_interface.h"
#endif

#ifdef DMA_AUDIO_APP
#include "dma_audio_host.h"
#endif

#ifdef CPU_MAX_POWER_TEST
#include CHIP_SPECIFIC_HDR(pmu_itn)
#endif

#ifdef OPENAMP_ENABLE
#include <rptun/chips/rptun_chips.h>
#endif

#define HANDSHAKE_TIMEOUT               6

static const uint8_t handshake_msg[] = "Hi";
static bool handshake_done;

static uint32_t pwr_key_click_cnt;

#ifdef SLEEP_TEST
#ifndef NO_TIMER
static void timer_handler(void *param)
{
    static uint32_t wakeup_cnt = 0;

    sensor_hub_send((uint8_t *)&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
}
#endif

static void sensor_hub_sleep_test(void)
{
#ifndef NO_TIMER
    sleep_test_register_timer_callback(timer_handler);
#endif

    // Jump to next test case and the last one is SLEEP_TEST
}
#endif /* SLEEP_TEST */

#ifdef VAD_CODEC_TEST
#define VAD_CODEC_WAKE_USER             HAL_CPU_WAKE_LOCK_USER_3
#define CMD_LIST_SIZE                   10
static uint32_t cmd_list[CMD_LIST_SIZE][4];
static uint8_t cmd_len[CMD_LIST_SIZE];
static uint8_t cmd_list_rpos;
static uint8_t cmd_list_wpos;
static bool cmd_list_full;

static struct ANA_RPC_REPLY_MSG_T ana_reply;

#define ALIGNED4                        ALIGNED(4)

#ifdef VAD_CODEC_LOOP
#define PLAYBACK_SIZE                   (48 * 2 * 16)
static uint8_t ALIGNED4 playback_buf[PLAYBACK_SIZE];
static uint8_t ALIGNED4 loop_data_buf[PLAYBACK_SIZE * 2];
static uint32_t loop_data_rpos;
static uint32_t loop_data_wpos;
static bool loop_data_full;
#else
#define CAP_BUF_SIZE                    (128 * 1024)
static uint8_t ALIGNED4 cap_data_buf[CAP_BUF_SIZE];
static uint32_t cap_data_size;
static uint32_t prev_click_cnt;
static bool cap_on;
#endif

static int save_cmd(const void *data, unsigned int len)
{
    uint32_t lock;

    ASSERT(len <= sizeof(cmd_list[0]), "%s: len too large: %u (should <= %u)", __func__, len, sizeof(cmd_list[0]));
    if (cmd_list_full) {
        return 1;
    }
    memcpy(cmd_list[cmd_list_wpos], data, len);
    cmd_len[cmd_list_wpos] = len;
    cmd_list_wpos++;
    if (cmd_list_wpos >= CMD_LIST_SIZE) {
        cmd_list_wpos = 0;
    }
    lock = int_lock();
    if (cmd_list_wpos == cmd_list_rpos) {
        cmd_list_full = true;
    }
    int_unlock(lock);
    return 0;
}

static int get_cmd(void *data, unsigned int len, unsigned int *rlen)
{
    uint32_t lock;

    ASSERT(len <= sizeof(cmd_list[0]), "%s: len too large: %u (should <= %u)", __func__, len, sizeof(cmd_list[0]));
    if (cmd_list_wpos == cmd_list_rpos && !cmd_list_full) {
        return 1;
    }
    memcpy(data, cmd_list[cmd_list_rpos], len);
    if (rlen) {
        *rlen = cmd_len[cmd_list_rpos];
    }
    lock = int_lock();
    cmd_list_rpos++;
    if (cmd_list_rpos >= CMD_LIST_SIZE) {
        cmd_list_rpos = 0;
    }
    cmd_list_full = false;
    int_unlock(lock);
    return 0;
}

static unsigned int codec_test_rx_handler(const void *data, unsigned int len)
{
    int ret;

    //TR_INFO(0, "%s: data=%p len=%u", __func__, data, len);

    ASSERT(len >= 2, "%s: Bad len=%u", __func__, len);

    ret = save_cmd(data, len);
    ASSERT(ret == 0, "%s: Failed to save cmd: %d", __func__, ret);

    hal_cpu_wake_lock(VAD_CODEC_WAKE_USER);

    return len;
}

static void codec_test_tx_handler(const void *data, unsigned int len)
{
    //TR_INFO(0, "%s: data=%p len=%u", __func__, data, len);
}

#ifdef VAD_CODEC_LOOP

static uint32_t get_loop_data_free_size(void)
{
    uint32_t rpos;
    uint32_t wpos;
    bool full;
    uint32_t lock;

    lock = int_lock();
    rpos = loop_data_rpos;
    wpos = loop_data_wpos;
    full = loop_data_full;
    int_unlock(lock);

    if (rpos == wpos) {
        if(full) {
            return 0;
        } else {
            return ARRAY_SIZE(loop_data_buf);
        }
    } else if (wpos > rpos) {
        return (ARRAY_SIZE(loop_data_buf) - wpos) + rpos;
    }
    return rpos - wpos;
}

static uint32_t get_loop_data_occupied_size(void)
{
    return ARRAY_SIZE(loop_data_buf) - get_loop_data_free_size();
}

static uint32_t get_loop_data(uint8_t *buf, uint32_t len)
{
    uint32_t avail;
    uint32_t rpos;
    uint32_t lock;

    avail = get_loop_data_occupied_size();
    if (len > avail) {
        len = avail;
    }
    if (len == 0) {
        return 0;
    }

    rpos = loop_data_rpos;

    if (rpos + len <= ARRAY_SIZE(loop_data_buf)) {
        memcpy(buf, &loop_data_buf[rpos], len);
        rpos += len;
        if (rpos >= ARRAY_SIZE(loop_data_buf)) {
            rpos = 0;
        }
    } else {
        uint32_t len1;

        len1 = ARRAY_SIZE(loop_data_buf) - rpos;
        memcpy(buf, &loop_data_buf[rpos], len1);
        memcpy(buf + len1, &loop_data_buf[0], len - len1);
        rpos = len - len1;
    }

    lock = int_lock();
    loop_data_rpos = rpos;
    loop_data_full = false;
    int_unlock(lock);

    return len;
}

static uint32_t put_loop_data(const uint8_t *buf, uint32_t len)
{
    uint32_t free_bytes;
    uint32_t wpos;
    uint32_t lock;

    free_bytes = get_loop_data_free_size();
    if (len > free_bytes) {
        len = free_bytes;
    }
    if (len == 0) {
        return 0;
    }

    wpos = loop_data_wpos;

    if (wpos + len <= ARRAY_SIZE(loop_data_buf)) {
        memcpy(&loop_data_buf[wpos], buf, len);
        wpos += len;
        if (wpos >= ARRAY_SIZE(loop_data_buf)) {
            wpos = 0;
        }
    } else {
        uint32_t len1;

        len1 = ARRAY_SIZE(loop_data_buf) - wpos;
        memcpy(&loop_data_buf[wpos], buf, len1);
        memcpy(&loop_data_buf[0], buf + len1, len - len1);
        wpos = len - len1;
    }

    lock = int_lock();
    loop_data_wpos = wpos;
    if (loop_data_rpos == loop_data_wpos) {
        loop_data_full = true;
    }
    int_unlock(lock);

    return len;
}

static uint32_t af_playback(uint8_t *buf, uint32_t len)
{
    uint32_t copy_len;

    copy_len = get_loop_data(buf, len);
    TR_VERBOSE(0, "len=%u copy_len=%u", len, copy_len);

    return 0;
}

#else /* !defined(VAD_CODEC_LOOP) */

SRAM_TEXT_LOC
static uint32_t vad_cap_rpc_svr(const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if (!cap_on) {
        if (prev_click_cnt == pwr_key_click_cnt) {
            return 0;
        }
        cap_on = true;
        cap_data_size = 0;
    }

    if (cap_data_size + len > sizeof(cap_data_buf)) {
        if (cap_data_size < sizeof(cap_data_buf)) {
            len = sizeof(cap_data_buf) - cap_data_size;
        } else {
            len = 0;
            cap_data_size = sizeof(cap_data_buf);
        }
    }
    memcpy(cap_data_buf + cap_data_size, buf, len);
    cap_data_size += len;

    if (cap_data_size >= sizeof(cap_data_buf)) {
        uint32_t lock;

        lock = int_lock();

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_104M);

        for (i = 0; i + 16 < cap_data_size; i += 8) {
            DUMP16("%6hd ", cap_data_buf + i, 16);
            TRACE_FLUSH();
        }
        if (i < cap_data_size) {
            DUMP16("%6hd ", cap_data_buf + i, cap_data_size - i);
        }
        TRACE_FLUSH();

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);

        int_unlock(lock);

        cap_on = false;
        prev_click_cnt = pwr_key_click_cnt;
    }

    return len;
}

#endif /* VAD_CODEC_LOOP */

static POSSIBLY_UNUSED void codec_test_cmd_loop(void)
{
    int ret;
    uint32_t data[4];
    uint32_t rlen;
    struct SENS_MSG_HDR_T *hdr;

    ret = get_cmd(data, sizeof(data), &rlen);
    if (ret) {
        hal_cpu_wake_unlock(VAD_CODEC_WAKE_USER);
        return;
    }

    hdr = (struct SENS_MSG_HDR_T *)data;
    if (hdr->reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, hdr->id);
    } else {
        if (hdr->id == SENS_MSG_ID_ANA_RPC) {
            struct ANA_RPC_REQ_MSG_T *ana_req;
            enum ANALOG_RPC_ID_T ana_id;

            ana_req = (struct ANA_RPC_REQ_MSG_T *)data;
            ana_id = ana_req->rpc_id;

            TRACE(0, "%s: Rx ana rpc msg: %d", __func__, ana_id);

#ifdef VAD_CODEC_LOOP
            af_lock_thread();
#endif

            switch (ana_id) {
            case ANALOG_RPC_FREQ_PLL_CFG:
                analog_rpcsvr_freq_pll_config(ana_req->param[0], ana_req->param[1]);
                break;
            case ANALOG_RPC_OSC_CLK_ENABLE:
                analog_rpcsvr_osc_clk_enable(ana_req->param[0]);
                break;
            case ANALOG_RPC_CODEC_OPEN:
                analog_rpcsvr_codec_open(ana_req->param[0]);
                break;
            case ANALOG_RPC_CODEC_ADC_ENABLE:
                analog_rpcsvr_codec_adc_enable(ana_req->param[0], ana_req->param[1], ana_req->param[2]);
                break;
            case ANALOG_RPC_VAD_ENABLE:
                analog_rpcsvr_vad_enable(ana_req->param[0], ana_req->param[1]);
                break;
            case ANALOG_RPC_VAD_ADC_ENABLE:
                analog_rpcsvr_vad_adc_enable(ana_req->param[0]);
                break;
            default:
                ASSERT(false, "%s: Bad ana rpc id: %d", __func__, ana_id);
                break;
            };

#ifdef VAD_CODEC_LOOP
            af_unlock_thread();
#endif

            ana_reply.hdr.id = SENS_MSG_ID_ANA_RPC;
            ana_reply.hdr.reply = 1;
            ana_reply.rpc_id = ana_id;
            ana_reply.ret = 0;
            ret = sensor_hub_send(&ana_reply, sizeof(ana_reply));
            ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);
        } else if (hdr->id == SENS_MSG_ID_AF_BUF) {
            struct AF_RPC_REQ_MSG_T *af_req;

            af_req = (struct AF_RPC_REQ_MSG_T *)data;
            //TR_INFO(0, "%s: Rx af buf msg: stream=%d", __func__, af_req->stream_id);
#ifdef VAD_CODEC_LOOP
            uint32_t save_len;
            save_len = put_loop_data(af_req->buf, af_req->len);
            TR_VERBOSE(0, "len=%u save_len=%u", af_req->len, save_len);
#else
            vad_cap_rpc_svr(af_req->buf, af_req->len);
#endif
        } else {
            ASSERT(false, "%s: Bad msg: id=%d", __func__, hdr->id);
        }
    }
}

static void sensor_hub_vad_codec_test(void)
{
#ifdef VAD_CODEC_LOOP
    int ret;
    struct AF_STREAM_CONFIG_T stream_cfg;

    af_open();

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
    stream_cfg.sample_rate = AUD_SAMPRATE_16927; //AUD_SAMPRATE_50781;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    stream_cfg.handler = af_playback;
    stream_cfg.data_ptr = playback_buf;
    stream_cfg.data_size = PLAYBACK_SIZE;

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
#endif

    while (1) {
        codec_test_cmd_loop();
#ifdef RTOS
        osDelay(2);
#else
#ifdef VAD_CODEC_LOOP
        extern void af_thread(void const *argument);
        af_thread(NULL);
#endif
        hal_sleep_enter_sleep();
#endif
    }
}

#endif /* VAD_CODEC_TEST */

#ifdef VAD_APP_TEST

#ifdef VAD_KEY_EVT
static int vad_key_event_handler(uint32_t key_code, uint8_t key_event)
{
    static int ms_flag = 0, cs_flag = 0;

    TRACE(3,"%s: code=0x%X, event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            ms_flag ^= 1;
            //TRACE(0, "====> PWR KEY clicked, mem scan %s", ms_flag ? "start" : "stop");
            //app_mcu_sensor_hub_send_test_msg(SENS_VAD_TEST_SCMD_MEM_TEST, ms_flag, 0);
            //app_mcu_sensor_hub_send_test_msg(SENS_VAD_TEST_SCMD_KWS_TEST, ms_flag, 0);
        } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {
            cs_flag ^= 1;
            //TRACE(0, "====> PWR KEY long pressed, cpu test %s", cs_flag ? "start" : "stop");
            //app_mcu_sensor_hub_send_test_msg(SENS_VAD_TEST_SCMD_MIC_DATA, cs_flag, 0);
        }
    }
    return 0;
}
#endif /* VAD_KEY_EVT */

static void vad_evt_handler(enum APP_VAD_EVENT_T event, uint8_t *param)
{
    TRACE(0, "vad event=%d, param:%d/%d/%d", event, param[0], param[1], param[2]);
    if (event == APP_VAD_EVT_VOICE_CMD) {
        TRACE(0, "====> MCU recevied VOICE_CMD");
    }
}

static POSSIBLY_UNUSED void __delay_ms(uint32_t ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(ms));
}

static void sensor_hub_vad_app_init(void)
{
    int ret = 0;

    TRACE(0, "%s: start", __func__);

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);

    app_core_bridge_init();
    TRACE(0, "core bridge init done");

    ret = sensor_hub_open(app_core_bridge_data_received, app_core_bridge_data_tx_done);
    ASSERT(ret==0, "%s: sensor_hub_open failed: %d", __func__, ret);
    TRACE(0, "sensor hub open done");

    hal_sys_timer_delay(MS_TO_TICKS(200));
}

#ifdef VAD_IF_TEST
static void app_mcu_vad_ss_ai_test(void)
{
    TRACE(0, "%s: init ss lib!", __func__);
    app_mcu_sensor_hub_start_vad(APP_VAD_ADPT_ID_AI_KWS);
    app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_SS,1);
}
#endif

static void sensor_hub_vad_app_test(void)
{
    TRACE(0, "%s: start", __func__);

    // transmit msg to start vad
    app_mcu_sensor_hub_setup_vad_event_handler(vad_evt_handler);
#ifdef VAD_IF_TEST
    app_mcu_vad_ss_ai_test();
#else
    app_mcu_sensor_hub_start_vad(APP_VAD_ADPT_ID_DEMO);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    hal_sysfreq_print_user_freq();
    TRACE(0, "MCU CPU freq=%d", hal_sys_timer_calc_cpu_freq(5,0));
    TRACE(0, "waiting sensor hub cmd");

    while(1) {
#ifdef RTOS
        osSignalWait(0,osWaitForever);
#else
        osDelay(500);
#endif
    };
}

#endif /* VAD_APP_TEST */

static POSSIBLY_UNUSED unsigned int rx_handler(const void *data, unsigned int len)
{
    if (!handshake_done) {
        if (data && len == (sizeof(handshake_msg) - 1) &&
                memcmp(data, handshake_msg, len) == 0) {
            TR_INFO(0, "sensor_hub alive!");
            handshake_done = true;

            return len;
        }
    }

#ifdef DMA_AUDIO_APP
    unsigned int rlen = dma_audio_rx_data_handler(data, len);
    if (rlen == len) {
        return len;
    }
#endif

#ifdef OPENAMP_ENABLE
    bes_bth2sens_rx_handler(data, len);
#endif

#ifdef MCU2SENS_MSG_TEST
    return rmt_msg_test_rx_handler(data, len);
#endif

#ifdef VAD_CODEC_TEST
    return codec_test_rx_handler(data, len);
#endif

    return len;
}

static POSSIBLY_UNUSED void tx_handler(const void *data, unsigned int len)
{
#ifdef DMA_AUDIO_APP
    dma_audio_tx_data_handler(data, len);
#endif

#ifdef MCU2SENS_MSG_TEST
    rmt_msg_test_tx_handler(data, len);
#endif

#ifdef VAD_CODEC_TEST
    codec_test_tx_handler(data, len);
#endif

#ifdef OPENAMP_ENABLE
    bes_bth2sens_tx_handler(data, len);
#endif
}

#ifdef MCU2SENS_MSG_TEST
static void mcu2sens_msg_test_server(void)
{
    rmt_msg_test(false, sensor_hub_send, NULL);
}
#endif

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    TRACE(3,"%s: code=0x%X, event=%u", __FUNCTION__, key_code, key_event);

    if (key_event == HAL_KEY_EVENT_CLICK) {
        if (key_code == HAL_KEY_CODE_PWR) {
            pwr_key_click_cnt++;
        }
    }
#ifdef VAD_KEY_EVT
    vad_key_event_handler(key_code, key_event);
#endif
#ifdef DMA_AUDIO_APP
    dma_audio_app_test_key_handler(key_code, key_event);
#endif
    return 0;
}

void sensor_hub_test(void)
{
    int POSSIBLY_UNUSED ret = 0;

    TRACE(0, "%s", __func__);

    hwtest_register_key_callback(key_event_process);

#ifdef SLEEP_TEST
    sensor_hub_sleep_test();
#endif

#ifdef MCU2SENS_MSG_TEST
    mcu2sens_msg_test_server();
#endif

#ifdef CPU_MAX_POWER_TEST
    pmu_itn_sens_dig_set_volt(PMU_ITN_VDIG_0_875V, PMU_ITN_VDIG_0_875V);
#endif

#ifdef SENSOR_BT_TEST
    btdrv_start_bt();
#endif

#ifdef VAD_APP_TEST
    sensor_hub_vad_app_init();
#else
    ret = sensor_hub_open(rx_handler, tx_handler);
    ASSERT(ret == 0, "sensor_hub_open failed: %d", ret);
#endif

#ifdef VAD_APP_TEST
    sensor_hub_vad_app_test();
#endif

#ifdef VAD_CODEC_TEST
    sensor_hub_vad_codec_test();
#endif

#if defined(MINIMA_TEST)
    pmu_vcorev_dig_enable();
#if 0
    hal_psc_jtag_select_sens();
    hal_iomux_set_clock_out();
    hal_cmu_clock_out_enable(HAL_CMU_CLOCK_OUT_SENS_SYS);
#endif
#endif

#if defined(OPENAMP_ENABLE)
    uint32_t wait_cnt = 0;
    while (!handshake_done && wait_cnt < HANDSHAKE_TIMEOUT) {
        osDelay(1);
        wait_cnt ++;
    };
    if (wait_cnt > HANDSHAKE_TIMEOUT) {
        ASSERT(true, "sensor hub handshake too long %d!!!", wait_cnt);
    }
#if defined(SENS_LOAD_FILE_NAME)
    rptun_init_bth2sens_fs();
#else
    rptun_init_bth2sens();
#endif

#if defined(OPENAMP_ECHO_EXAMPLE)
    int rpmsg_echo_bth2sens_master(int argc, char *argv[]);
    rpmsg_echo_bth2sens_master(0, NULL);
#endif
#endif

#ifdef DMA_AUDIO_APP
    dma_audio_app_test(DMA_AUD_TRIG_MODE_PWRKEY, true);
#endif

}

#endif

