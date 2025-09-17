/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "bt_sco_chain_reframe.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "stream_mcps.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "audio_dump.h"
#include "speech_cfg.h"
#include "kfifo.h"
#include "cmsis_os.h"
#if defined(SCO_CP_ACCEL)
#include "hal_location.h"
#include "bt_sco_chain_cp_thirdparty.h"
#endif
#include "bt_sco_chain_thirdparty_def.h"
// TODO: #include "thirdparty_algo.h"

// #define _REFRAME_DEBUG
// #define BT_SCO_CHAIN_REFRAME_AUDIO_DUMP

#define BT_SCO_CHAIN_LOG_I(str, ...)        TR_INFO(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
#if defined(_REFRAME_DEBUG)
#define BT_SCO_CHAIN_LOG_D(str, ...)        TR_DEBUG(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
static uint32_t g_debug_index = 0;
#else
#define BT_SCO_CHAIN_LOG_D(str, ...)
#endif


#if defined(SPEECH_TX_24BIT)
typedef int     _PCM_T;
#define SAMPLE_BYTES                     (4)
#else
typedef short   _PCM_T;
#define SAMPLE_BYTES                     (2)
#endif

#define PROCESS_FRAME_LEN                (THIRDPARTY_FRAME_LEN)
#define PCM_TX_CHANNEL_NUM               (SPEECH_CODEC_CAPTURE_CHANNEL_NUM)
#define PCM_FIFO_FRAME_NUM               (3)

#if defined(LOW_DELAY_SCO)
#define IO_FRAME_LEN                     (120)   // 7.5ms
#else
#define IO_FRAME_LEN                     (240)   // 15ms
#endif

#define PCM_OUT_DELAY                    (THIRDPARTY_FRAME_LEN * (PCM_FIFO_FRAME_NUM - 1))

#define CALC_MIPS_CNT_THD_MS             (3000)
#define GET_OUT_FIFO_DATA_WAIT_TIME_US   (100)
#define GET_OUT_FIFO_DATA_WAIT_CNT       (2)

// Thread
#define REFRAME_THREAD_STACK_SIZE        (1024 * 2)
static osThreadId reframe_thread_tid;
static void _reframe_thread(void const *argument);
osThreadDef(_reframe_thread, osPriorityAboveNormal, 1, REFRAME_THREAD_STACK_SIZE, "speech_reframe");

// Signal
#define REFRAME_SIGNAL_WRITE             (1)

static int32_t _nextpow2(int32_t N)
{
    int32_t i = 1;
    while (1) {
        i <<= 1;
        if (i >= N)
            return i;
    }
}

typedef struct {
    struct kfifo pcm_fifo;
    uint8_t *pcm_fifo_mem;
    struct kfifo ref_fifo;
    uint8_t *ref_fifo_mem;
    struct kfifo vpu_fifo;
    uint8_t *vpu_fifo_mem;
    struct kfifo out_fifo;
    uint8_t *out_fifo_mem;

} reframe_tx_fifo_t;

static reframe_tx_fifo_t g_tx_fifo   = {0};
static uint8_t *process_pcm_buf      = NULL;
static uint8_t *process_ref_buf      = NULL;
static uint8_t *deinterlaved_pcm_buf = NULL;
#ifdef SPEECH_BONE_SENSOR
static uint8_t *process_vpu_buf      = NULL;
#endif

static reframe_tx_fifo_t *get_reframe_tx_fifo(void)
{
    return &g_tx_fifo;
}

#if defined(SCO_CP_ACCEL)
CP_TEXT_SRAM_LOC
int32_t sco_cp_algo(_PCM_T *pcm_buf[], _PCM_T *out_buf, int frame_len)
{
    ASSERT(frame_len == PROCESS_FRAME_LEN, "[%s] frame_len(%d) != PROCESS_FRAME_LEN", __func__, frame_len);

    // calculate_mips_run_start(true);

#if 1
    // TODO: Call thirdparty algo process function which run in CP Core
#else
    for (uint32_t i=0; i<frame_len; i++) {
        out_buf[i] = pcm_buf[REFRAME_CHANNEL_FB][i];
    }
#endif

    return 0;
}
#endif

static void reframe_algo_process(_PCM_T *pcm_buf[REFRAME_CHANNEL_QTY], _PCM_T *out_buf)
{
    //---------- Process Start ----------
#if 0
    // TODO: Call thirdparty algo process function which run in Main Core

#if defined(SCO_CP_ACCEL)
    sco_cp_process(pcm_buf, out_buf, PROCESS_FRAME_LEN);
#else
    // TODO: Call thirdparty algo process function which run in CP Core
#endif
#else
    for (uint32_t i = 0; i < PROCESS_FRAME_LEN; i++) {
        out_buf[i] = pcm_buf[REFRAME_CHANNEL_FB][i];
    }
#endif
    //---------- Process End ----------
}

static bool pcm_fifo_is_ready(void)
{
    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;
    if (kfifo_len(&tx_fifo->pcm_fifo) >= pcm_data_size &&
        kfifo_get_free_space(&tx_fifo->out_fifo) >= out_data_size) {
        return true;
    } else {
        return false;
    }
}

static void reframe_process(void)
{
    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;

    if (!(kfifo_len(&tx_fifo->pcm_fifo) >= pcm_data_size &&
        kfifo_get_free_space(&tx_fifo->out_fifo) >= out_data_size)) {
        return;
    }

    kfifo_get(&tx_fifo->pcm_fifo, process_pcm_buf, pcm_data_size);
    kfifo_get(&tx_fifo->ref_fifo, process_ref_buf, pcm_data_size / PCM_TX_CHANNEL_NUM);

#ifdef SPEECH_BONE_SENSOR
    kfifo_get(&tx_fifo->vpu_fifo, process_vpu_buf, pcm_data_size / PCM_TX_CHANNEL_NUM);
#endif

    _PCM_T *_process_pcm_buf = (_PCM_T *)process_pcm_buf;
    _PCM_T *_deinterlaved_pcm_buf = (_PCM_T *)deinterlaved_pcm_buf;
    for (uint32_t ch = 0; ch < PCM_TX_CHANNEL_NUM; ch++) {
        for (uint32_t i = 0; i < PROCESS_FRAME_LEN; i++) {
            _deinterlaved_pcm_buf[ch * PROCESS_FRAME_LEN + i] = _process_pcm_buf[PCM_TX_CHANNEL_NUM * i + ch];
        }
    }

    _PCM_T *pcm_buf[REFRAME_CHANNEL_QTY] = {NULL};

    for (uint32_t i = 0; i < PCM_TX_CHANNEL_NUM; i++) {
        pcm_buf[i] = (_PCM_T *)&deinterlaved_pcm_buf[i * PROCESS_FRAME_LEN * SAMPLE_BYTES];
    }

    pcm_buf[REFRAME_CHANNEL_REF] = (_PCM_T *)process_ref_buf;

#ifdef SPEECH_BONE_SENSOR
    pcm_buf[REFRAME_CHANNEL_BS] = (_PCM_T *)process_vpu_buf;
#endif

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
    uint32_t dump_ch = 0;
    audio_dump_clear_up();

#ifdef SPEECH_TX_24BIT
    for (uint32_t i = 0; i < PCM_TX_CHANNEL_NUM; i++) {
        audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, pcm_buf[i], PROCESS_FRAME_LEN, 1, 0, 8);
    }
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, pcm_buf[REFRAME_CHANNEL_REF], PROCESS_FRAME_LEN, 1, 0, 8);
#ifdef SPEECH_BONE_SENSOR
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, pcm_buf[REFRAME_CHANNEL_BS], PROCESS_FRAME_LEN, 1, 0, 8);
#endif
#else
    for (uint32_t i = 0; i < PCM_TX_CHANNEL_NUM; i++) {
        audio_dump_add_channel_data(dump_ch++, pcm_buf[i], PROCESS_FRAME_LEN);
    }
    audio_dump_add_channel_data(dump_ch++, pcm_buf[REFRAME_CHANNEL_REF], PROCESS_FRAME_LEN);
#ifdef SPEECH_BONE_SENSOR
    audio_dump_add_channel_data(dump_ch++, pcm_buf[REFRAME_CHANNEL_BS], PROCESS_FRAME_LEN);
#endif
#endif
#endif

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_pre("REFRAME_TX");
#endif

    reframe_algo_process(pcm_buf, (_PCM_T *)process_pcm_buf);

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_post("REFRAME_TX");
#endif

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
#ifdef SPEECH_TX_24BIT
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(dump_ch++, process_pcm_buf, PROCESS_FRAME_LEN, 1, 0, 8);
#else
    audio_dump_add_channel_data(dump_ch++, process_pcm_buf, PROCESS_FRAME_LEN);
#endif
    audio_dump_run();
#endif

    kfifo_put(&tx_fifo->out_fifo, process_pcm_buf, out_data_size);
}

static void _reframe_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        
        // BT_SCO_CHAIN_LOG_D("[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if (evt.status == osEventSignal) {
            // BT_SCO_CHAIN_LOG_D("signal = %ld", signals);
            if (signals & (1 << REFRAME_SIGNAL_WRITE)) {
                while (1) {
                    if (pcm_fifo_is_ready()) {
                        BT_SCO_CHAIN_LOG_D("RUN Start ...");
                        reframe_process();
                    } else {
                        break;
                    }
                }
                BT_SCO_CHAIN_LOG_D("RUN End ...");
            }
        } else {
            ASSERT(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

int32_t bt_sco_chain_reframe_open(uint32_t tx_sample_rate, uint32_t tx_frame_len)
{
    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);
    ASSERT(tx_frame_len != PROCESS_FRAME_LEN, "[%s] tx_frame_len(%d) == PROCESS_FRAME_LEN", __func__, tx_frame_len);

#if defined(SCO_CP_ACCEL)
    sco_cp_init(PROCESS_FRAME_LEN, REFRAME_CHANNEL_QTY);
#endif

    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    uint32_t in_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_TX_CHANNEL_NUM * PCM_FIFO_FRAME_NUM;
    uint32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_FIFO_FRAME_NUM;

    uint32_t pcm_fifo_len = _nextpow2(in_data_size);
    uint32_t ref_fifo_len = _nextpow2(in_data_size / PCM_TX_CHANNEL_NUM);
    uint32_t out_fifo_len = _nextpow2(out_data_size);

    tx_fifo->pcm_fifo_mem = speech_malloc(pcm_fifo_len);
    tx_fifo->ref_fifo_mem = speech_malloc(ref_fifo_len);
    tx_fifo->out_fifo_mem = speech_malloc(out_fifo_len);

    kfifo_init(&tx_fifo->pcm_fifo, tx_fifo->pcm_fifo_mem, pcm_fifo_len);
    kfifo_init(&tx_fifo->ref_fifo, tx_fifo->ref_fifo_mem, ref_fifo_len);
    kfifo_init(&tx_fifo->out_fifo, tx_fifo->out_fifo_mem, out_fifo_len);

    uint32_t process_data_len = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM;
    process_pcm_buf = speech_calloc(process_data_len, SAMPLE_BYTES);
    deinterlaved_pcm_buf = speech_calloc(process_data_len, SAMPLE_BYTES);
    process_ref_buf = speech_calloc(PROCESS_FRAME_LEN, SAMPLE_BYTES);

#ifdef SPEECH_BONE_SENSOR
    uint32_t vpu_fifo_len = _nextpow2(in_data_size / PCM_TX_CHANNEL_NUM);
    tx_fifo->vpu_fifo_mem = speech_malloc(vpu_fifo_len);
    kfifo_init(&tx_fifo->vpu_fifo, tx_fifo->vpu_fifo_mem, vpu_fifo_len);
    process_vpu_buf = speech_calloc(PROCESS_FRAME_LEN, SAMPLE_BYTES);
#endif

    BT_SCO_CHAIN_LOG_I("[%s] Pust %d pcm into out FIFO", __func__, PCM_OUT_DELAY);
    kfifo_put(&tx_fifo->out_fifo, (uint8_t *)tx_fifo->pcm_fifo_mem, PCM_OUT_DELAY * SAMPLE_BYTES);

    reframe_thread_tid = osThreadCreate(osThread(_reframe_thread), NULL);
    osSignalSet(reframe_thread_tid, 0x0);

    // TODO: Call thridpart algo init function

#if defined(_REFRAME_DEBUG)
    g_debug_index = 0;
#endif

#if defined(CALC_MIPS_CNT_THD_MS)
    float frame_ms = (PROCESS_FRAME_LEN * 1000) / tx_sample_rate;
    stream_mcps_start("REFRAME_TX", 96, frame_ms, CALC_MIPS_CNT_THD_MS);
#endif

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
    audio_dump_init(PROCESS_FRAME_LEN, sizeof(int16_t), 6);
#endif

    return 0;
}

int32_t bt_sco_chain_reframe_close(void)
{
    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);

#ifdef BT_SCO_CHAIN_REFRAME_AUDIO_DUMP
    audio_dump_deinit();
#endif

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_stop("REFRAME_TX");
#endif

    // TODO: Call thridpart algo deinit function

    osThreadTerminate(reframe_thread_tid);

    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();

#ifdef SPEECH_BONE_SENSOR
    speech_free(tx_fifo->vpu_fifo_mem);
    speech_free(process_vpu_buf);
#endif

    speech_free(process_ref_buf);
    speech_free(deinterlaved_pcm_buf);
    speech_free(process_pcm_buf);

    speech_free(tx_fifo->out_fifo_mem);
    speech_free(tx_fifo->ref_fifo_mem);
    speech_free(tx_fifo->pcm_fifo_mem);

#if defined(SCO_CP_ACCEL)
    sco_cp_deinit();
#endif

    return 0;
}

int32_t bt_sco_chain_reframe_tx_process(REFRAME_TX_PCM_T *pcm_buf)
{
    ASSERT(pcm_buf->frame_len == IO_FRAME_LEN, "[%s] frame_len(%d) != IO_FRAME_LEN", __func__, pcm_buf->frame_len);
    ASSERT(pcm_buf->out != NULL, "[%s] out_buf == NULL", __func__);

#if defined(_REFRAME_DEBUG)
    BT_SCO_CHAIN_LOG_D("[REFRAME] %d: frame_len = %d", g_debug_index++, pcm_buf->frame_len);
#endif

    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = pcm_buf->frame_len * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t data_size = pcm_buf->frame_len * SAMPLE_BYTES;

    if (kfifo_get_free_space(&tx_fifo->pcm_fifo) >= pcm_data_size) {
        kfifo_put(&tx_fifo->pcm_fifo, (uint8_t *)pcm_buf->in, pcm_data_size);
        if (pcm_buf->ref){
            kfifo_put(&tx_fifo->ref_fifo, (uint8_t *)pcm_buf->ref, data_size);
        }
#ifdef SPEECH_BONE_SENSOR
        if (pcm_buf->vpu){
            kfifo_put(&tx_fifo->vpu_fifo_mem, (uint8_t *)pcm_buf->vpu, data_size);
        }
#endif
    } else {
        TRACE(0, "[%s] Input buffer is overflow", __func__);
    }
    osSignalSet(reframe_thread_tid, (1 << REFRAME_SIGNAL_WRITE));

    int32_t cnt = 0;
    int32_t out_size = pcm_buf->frame_len * SAMPLE_BYTES;
    while (true) {
        if (kfifo_len(&tx_fifo->out_fifo) >= out_size) {
            kfifo_get(&tx_fifo->out_fifo, (uint8_t *)pcm_buf->out, out_size);
            break;
        } else {
            if (cnt <= GET_OUT_FIFO_DATA_WAIT_CNT) {
                TRACE(0, "[%s] Output buffer is underflow, wait cnt:%d.", __func__, cnt);
                hal_sys_timer_delay_us(GET_OUT_FIFO_DATA_WAIT_TIME_US);
                cnt ++;
            } else {
                TRACE(0, "[%s] Output buffer is underflow, out set to zero!", __func__);
                memset(pcm_buf->out, 0, out_size);
                break;
            }
        }
    }

    return 0;
}