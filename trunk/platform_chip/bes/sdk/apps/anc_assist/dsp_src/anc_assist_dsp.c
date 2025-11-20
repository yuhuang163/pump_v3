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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "heap_api.h"
#include "app_utils.h"
#include "speech_utils.h"
#include "anc_assist_dsp.h"
#include "anc_assist_mic.h"
#include "anc_assist_comm.h"
#include "app_rpc_api.h"

#if defined(_ASSIST_THREAD_DEBUG)
#define ASSIST_THREAD_LOG_D(str, ...)       TR_DEBUG(TR_MOD(AUD), "[ASSIST_THREAD]" str, ##__VA_ARGS__)
#else
#define ASSIST_THREAD_LOG_D(str, ...)
#endif

#if defined(ASSIST_LOW_RAM_MOD)
#define ALGO_FRAME_LEN      (60)
#else
#define ALGO_FRAME_LEN      (120)
#endif

#define ANC_ASSIST_ALGO_FRAME_LEN   (480)   // input 4x7.5ms data, output 30ms data
#define ANC_ASSIST_ALGO_FRAME_BYTES (ANC_ASSIST_ALGO_FRAME_LEN * sizeof(float))
#define ASSIST_QUEUE_FRAME_NUM      (8)
#define ASSIST_QUEUE_BUF_PCM_NUM    (128 * ASSIST_QUEUE_FRAME_NUM)  // 2^N

typedef struct {
    anc_assist_user_t user;
    app_anc_assist_core_cmd_t cmd;
    uint32_t ctrl;
    uint32_t ctrl_buf_len;
    uint8_t *ctrl_buf;
} ANC_ASSIST_MESSAGE_T;

static float g_in_queue_buf[MIC_INDEX_QTY][ASSIST_QUEUE_BUF_PCM_NUM];
static float g_process_buf[MIC_INDEX_QTY][ANC_ASSIST_ALGO_FRAME_LEN];
static float *assist_in_queue_buf[MIC_INDEX_QTY] = {NULL};
static float *assist_process_buf[MIC_INDEX_QTY] = {NULL};
static uint32_t g_in_queue_len = 0;
static uint8_t g_ff_ch_num = MAX_FF_CHANNEL_NUM;
static uint8_t g_fb_ch_num = MAX_FB_CHANNEL_NUM;
static uint32_t g_user_status = 0;
static app_voice_assist_dsp_t *g_voice_assist_dsp[ANC_ASSIST_USER_QTY] = {NULL};
static anc_assist_fifo_t g_fifo;

// Thread
#define ANC_ASSIST_THREAD_STACK_SIZE    (1024 * 2)
static osThreadId anc_assist_thread_tid;
static void anc_assist_thread(void const *argument);
osThreadDef(anc_assist_thread, osPriorityNormal, 1, ANC_ASSIST_THREAD_STACK_SIZE, "anc_assist");

// mailbox
#define ANC_ASSIST_MAIL_MAX                 (30)
#ifdef ANC_ASSIST_TRIGGER_DSP_PROCESS
#define ANC_ASSIST_PROCESS_MAIL_THD         (3)
#else
#define ANC_ASSIST_PROCESS_MAIL_THD         (0)
#endif
static osMailQId anc_assist_mailbox = NULL;
osMailQDef (anc_assist_mailbox, ANC_ASSIST_MAIL_MAX, ANC_ASSIST_MESSAGE_T);

// Mailbox Heap
static heap_handle_t anc_assist_mailbox_heap;
static uint8_t anc_assist_mailbox_heap_pool[1024];

static uint32_t anc_assist_mailbox_cnt(void);

int32_t anc_assist_dsp_register(anc_assist_user_t user, app_voice_assist_dsp_t* anc_assist_dsp)
{
    TRACE(0, "[%s] user: %d", __func__, user);
    g_voice_assist_dsp[user] = anc_assist_dsp;

    return 0;
}

int32_t anc_assist_dsp_send_result_to_bth(anc_assist_user_t user, uint8_t *buf, uint32_t len,  uint32_t cmd)
{
    app_anc_assist_core_to_bth_data_t tx_cmd;
    uint32_t send_len = sizeof(app_anc_assist_core_to_bth_data_t) - sizeof(tx_cmd.buf) + len;

    memset(&tx_cmd, 0, sizeof(tx_cmd));
    tx_cmd.user = user;
    tx_cmd.sub_cmd = cmd;
    tx_cmd.len = len;
    if (len > 0) {
        ASSERT(len < sizeof(tx_cmd.buf), "buf is too long");
        memcpy(tx_cmd.buf, buf, len);
    }

    app_rpc_send_cmd(ANC_ASSIST_CORE, TASK_ANC_ASSIST_CMD_RSP, (uint8_t *)&tx_cmd, send_len);

    TRACE(3, "[%s] user: %d, cmd: %d OK", __func__, user, cmd);
    return 0;
}

static void anc_assist_frequency_update(void)
{
    uint8_t i = 0;
    uint32_t freq = 0;
    uint32_t status = 0;
    enum APP_SYSFREQ_FREQ_T total_freq;

    status = g_user_status;

    while(status) {
        if ((g_voice_assist_dsp[i] != NULL) && ((g_user_status & (0x1 << i)) != 0)) {
            if (g_voice_assist_dsp[i]->get_freq != NULL) {
                freq += g_voice_assist_dsp[i]->get_freq();
            } else {
                TRACE(2, "[%s] WARNING: user(%d) don't register get_freq func", __func__, i);
            }
        }
        status &= ~(0x1 << i) ;
        i++;
    }

    if (freq >= 168) {
        total_freq = HAL_CMU_FREQ_208M; // 192
    } else if (freq >= 144) {
        total_freq = HAL_CMU_FREQ_208M; // 168
    } else if (freq >= 120) {
        total_freq = HAL_CMU_FREQ_208M; //144
    } else if (freq >= 96) {
        total_freq = HAL_CMU_FREQ_208M; //120
    } else if(freq >= 72) {
        total_freq = APP_SYSFREQ_104M; //96
    } else if (freq >= 48) {
        total_freq = APP_SYSFREQ_78M; //72
    } else if (freq >= 24) {
        total_freq = APP_SYSFREQ_52M; //48
    } else if (freq >= 15) {
        total_freq = APP_SYSFREQ_26M; //24
    } else if (freq >= 6) {
        total_freq = APP_SYSFREQ_15M; //15
    } else {
        total_freq = HAL_CMU_FREQ_32K;
    }

    TRACE(3, "[%s] Need freq: %d; Set freq: %d", __func__, freq, total_freq);

    app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, total_freq);
}

static int32_t _process_frame(float **ff_mic, uint8_t ff_ch_num,
                       float **fb_mic, uint8_t fb_ch_num,
                       float *talk_mic,
                       float *ref,
#if defined(ANC_ASSIST_VPU)
                       float *vpu_mic,
#endif
                       uint32_t frame_len)
{
    int ret = 0;
    uint32_t status = 0;
    int i = 0;
    process_frame_data_t process_frame_date;

    process_frame_date.ff_mic = ff_mic;
    process_frame_date.ff_ch_num = ff_ch_num;
    process_frame_date.fb_mic = fb_mic;
    process_frame_date.fb_ch_num = fb_ch_num;
    process_frame_date.talk_mic = talk_mic;
#if defined(ANC_ASSIST_VPU)
    process_frame_date.vpu_mic = vpu_mic;
#endif
    process_frame_date.ref = ref;
    process_frame_date.frame_len = frame_len;
    status = g_user_status;

    while(status) {
        if ((g_voice_assist_dsp[i] != NULL) && ((g_user_status & (0x1 << i)) != 0)) {
            if (g_voice_assist_dsp[i]->process != NULL) {
                ret = g_voice_assist_dsp[i]->process(&process_frame_date);
            } else {
                TRACE(2, "[%s] WARNING: user(%d) don't register process func", __func__, i);
            }
        }
        status &= ~(0x1 << i) ;
        i++;
    }

    return ret;
}

static int32_t user_reset(anc_assist_user_t user)
{
    if ((g_voice_assist_dsp[user] != NULL) && ((g_user_status & (0x1 << user)) != 0)) {
        if (g_voice_assist_dsp[user]->reset != NULL) {
            g_voice_assist_dsp[user]->reset();
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register reset func", __func__, user);
        }
    }

    return 0;
}

static int32_t user_ctrl(anc_assist_user_t user, uint32_t ctrl, uint8_t *ptr, uint32_t ptr_len)
{
    if ((g_voice_assist_dsp[user] != NULL) && ((g_user_status & (0x1 << user)) != 0)) {
        if (g_voice_assist_dsp[user]->ctrl != NULL) {
            g_voice_assist_dsp[user]->ctrl(ctrl, ptr, ptr_len);
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register ctrl func", __func__, user);
        }
        anc_assist_frequency_update();
    }

    return 0;
}

static int32_t all_users_set_mode(uint32_t mode)
{
    for (uint32_t user=0; user<ANC_ASSIST_USER_QTY; user++) {
        if (g_voice_assist_dsp[user] != NULL) {
            if (g_voice_assist_dsp[user]->set_mode != NULL) {
                g_voice_assist_dsp[user]->set_mode(mode);
            }
        }
    }
    anc_assist_frequency_update();

    return 0;
}

static int32_t user_open(anc_assist_user_t user)
{
    if ((g_voice_assist_dsp[user] != NULL) && ((g_user_status & (0x1 << user)) == 0)) {
        if (g_voice_assist_dsp[user]->open != NULL) {
            g_voice_assist_dsp[user]->open();
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register open func", __func__, user);
        }
        g_user_status = g_user_status | (0x1 << user);
        anc_assist_frequency_update();
    }

    return 0;
}

static int32_t user_close(anc_assist_user_t user)
{
    if ((g_voice_assist_dsp[user] != NULL) && ((g_user_status & (0x1 << user)) != 0)) {
        if (g_voice_assist_dsp[user]->close != NULL) {
            g_voice_assist_dsp[user]->close();
        } else {
            TRACE(2, "[%s] WARNING: user(%d) don't register close func", __func__, user);
        }
        g_user_status = g_user_status & ~(0x1 << user);
        anc_assist_frequency_update();
    }

    return 0;
}

static void anc_assist_mailbox_heap_init(void)
{
    anc_assist_mailbox_heap =
        heap_register(anc_assist_mailbox_heap_pool,
        sizeof(anc_assist_mailbox_heap_pool));
}

static void *anc_assist_mailbox_heap_malloc(uint32_t size)
{
    void *ptr = NULL;
    if (size){
        ptr = heap_malloc(anc_assist_mailbox_heap, size);
        ASSERT(ptr, "%s size:%d, mailboxCnt:%d", __func__, size, anc_assist_mailbox_cnt());
    }
    return ptr;
}

static void anc_assist_mailbox_heap_free(void *rmem)
{
    if (rmem){
        heap_free(anc_assist_mailbox_heap, rmem);
    }
}

static uint32_t anc_assist_mailbox_cnt(void)
{
    return osMailGetCount(anc_assist_mailbox);
}

static int anc_assist_mailbox_put(ANC_ASSIST_MESSAGE_T *msg_src)
{
    osStatus status = osOK;

    ANC_ASSIST_MESSAGE_T *msg = NULL;
    msg = (ANC_ASSIST_MESSAGE_T *)osMailAlloc(anc_assist_mailbox, 0);

    if (!msg) {
        osEvent evt;
        TRACE(0, "[ANC_ASSIST] osMailAlloc error dump");
        for (uint8_t i = 0; i < ANC_ASSIST_MAIL_MAX; i++)
        {
            evt = osMailGet(anc_assist_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE(4, "cnt:%d user:%d param:%d/%p",
                      i,
                      ((ANC_ASSIST_MESSAGE_T *)(evt.value.p))->user,
                      ((ANC_ASSIST_MESSAGE_T *)(evt.value.p))->cmd,
                      ((ANC_ASSIST_MESSAGE_T *)(evt.value.p))->ctrl_buf);
            } else {
                TRACE(2, "cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE(0, "osMailAlloc error dump end");
        ASSERT(0, "[ANC_ASSIST] osMailAlloc error");
    } else {
        msg->user = msg_src->user;
        msg->cmd = msg_src->cmd;
        msg->ctrl_buf = msg_src->ctrl_buf;
        msg->ctrl = msg_src->ctrl;
        msg->ctrl_buf_len = msg_src->ctrl_buf_len;

        status = osMailPut(anc_assist_mailbox, msg);
        if (osOK != status) {
            TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
        }
    }

    return (int)status;
}

static int anc_assist_mailbox_get(ANC_ASSIST_MESSAGE_T **msg)
{
    int ret = 0;
    osEvent evt;
    evt = osMailGet(anc_assist_mailbox, osWaitForever);

    if (evt.status == osEventMail) {
        *msg = (ANC_ASSIST_MESSAGE_T *)evt.value.p;
    } else {
        ret = -1;
    }

    return ret;
}

static int anc_assist_mailbox_free(ANC_ASSIST_MESSAGE_T *msg)
{
    osStatus status;

    anc_assist_mailbox_heap_free(msg->ctrl_buf);
    status = osMailFree(anc_assist_mailbox, msg);
    if (osOK != status) {
        TRACE(2, "[%s] WARNING: failed: %d", __func__, status);
    }

    return (int)status;
}

static bool anc_assist_data_is_ready(void)
{
    if (kfifo_len(&(g_fifo.talk_fifo)) >= ANC_ASSIST_ALGO_FRAME_BYTES) {
        return true;
    } else {
        return false;
    }
}

static void anc_assist_data_clear(void)
{
    uint32_t len = kfifo_len(&(g_fifo.ff_mic_fifo));    // Must check ff_mic_fifo
    uint32_t loop_cnt = len / ANC_ASSIST_ALGO_FRAME_BYTES;

    for (uint32_t i=0; i<loop_cnt; i++) {
        kfifo_get(&(g_fifo.ff_mic_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_FF], ANC_ASSIST_ALGO_FRAME_BYTES);
        kfifo_get(&(g_fifo.fb_mic_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_FB], ANC_ASSIST_ALGO_FRAME_BYTES);
        kfifo_get(&(g_fifo.ref_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_REF], ANC_ASSIST_ALGO_FRAME_BYTES);
#if defined(ANC_ASSIST_VPU)
        kfifo_get(&(g_fifo.vpu_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_VPU], ANC_ASSIST_ALGO_FRAME_BYTES);
#endif 
        kfifo_get(&(g_fifo.talk_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_TALK], ANC_ASSIST_ALGO_FRAME_BYTES);
    }

    if (len % ANC_ASSIST_ALGO_FRAME_BYTES > 0) {
        kfifo_get(&(g_fifo.ff_mic_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_FF], ANC_ASSIST_ALGO_FRAME_BYTES);
        kfifo_get(&(g_fifo.fb_mic_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_FB], ANC_ASSIST_ALGO_FRAME_BYTES);
        kfifo_get(&(g_fifo.ref_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_REF], ANC_ASSIST_ALGO_FRAME_BYTES);
#if defined(ANC_ASSIST_VPU)
        kfifo_get(&(g_fifo.vpu_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_VPU], ANC_ASSIST_ALGO_FRAME_BYTES);
#endif 
        kfifo_get(&(g_fifo.talk_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_TALK], ANC_ASSIST_ALGO_FRAME_BYTES);
    }
}

static void anc_assist_kfifo_init(anc_assist_fifo_t *fifo)
{
    fifo->ff_mic_fifo_mem = (uint8_t *)assist_in_queue_buf[MIC_INDEX_FF];
    fifo->fb_mic_fifo_mem = (uint8_t *)assist_in_queue_buf[MIC_INDEX_FB];
    fifo->talk_fifo_mem = (uint8_t *)assist_in_queue_buf[MIC_INDEX_TALK];
    fifo->ref_fifo_mem = (uint8_t *)assist_in_queue_buf[MIC_INDEX_REF];
#if defined(ANC_ASSIST_VPU)
    fifo->vpu_fifo_mem = (uint8_t *)assist_in_queue_buf[MIC_INDEX_VPU];
#endif

    kfifo_init(&fifo->ff_mic_fifo, fifo->ff_mic_fifo_mem, ASSIST_QUEUE_BUF_PCM_NUM * sizeof(float));
    kfifo_init(&fifo->fb_mic_fifo, fifo->fb_mic_fifo_mem, ASSIST_QUEUE_BUF_PCM_NUM * sizeof(float));
    kfifo_init(&fifo->talk_fifo, fifo->talk_fifo_mem, ASSIST_QUEUE_BUF_PCM_NUM * sizeof(float));
    kfifo_init(&fifo->ref_fifo, fifo->ref_fifo_mem, ASSIST_QUEUE_BUF_PCM_NUM * sizeof(float));
#if defined(ANC_ASSIST_VPU)
    kfifo_init(&fifo->vpu_fifo, fifo->vpu_fifo_mem, ASSIST_QUEUE_BUF_PCM_NUM * sizeof(float));
#endif

    TRACE(3, "[%s] fifo addr: %p, size: %d", __func__,  fifo, g_fifo.talk_fifo.size);
}

static void anc_assist_thread(void const *argument)
{
    ANC_ASSIST_MESSAGE_T *msg_p = NULL;
    float *ff_mic_buf[MAX_FF_CHANNEL_NUM];
    float *fb_mic_buf[MAX_FB_CHANNEL_NUM];

    while(1) {
        if (!anc_assist_mailbox_get(&msg_p)) {
            if (msg_p->cmd == APP_ANC_ASSIST_CORE_OPEN) {
                user_open(msg_p->user);
                app_anc_assist_core_to_bth_rsp_t rsp;
                memset(&rsp, 0, sizeof(rsp));
                rsp.cmd = APP_ANC_ASSIST_CORE_OPEN;
                rsp.user = msg_p->user;
                rsp.fifo_ptr = &g_fifo;
                app_rpc_send_cmd_rsp(ANC_ASSIST_CORE, TASK_ANC_ASSIST_CMD_RSP, (uint8_t *)&rsp, sizeof(rsp));
            } else if (msg_p->cmd == APP_ANC_ASSIST_CORE_CLOSE) {
                user_close(msg_p->user);
                if (g_user_status == 0) {
                    anc_assist_data_clear();
                }
                app_rpc_send_cmd_rsp(ANC_ASSIST_CORE, TASK_ANC_ASSIST_CMD_RSP, NULL, 0);
            } else if (msg_p->cmd == APP_ANC_ASSIST_CORE_RESET) {
                user_reset(msg_p->user);
                app_rpc_send_cmd_rsp(ANC_ASSIST_CORE, TASK_ANC_ASSIST_CMD_RSP, NULL, 0);
            } else if (msg_p->cmd == APP_ANC_ASSIST_CORE_CTRL) {
                user_ctrl(msg_p->user, msg_p->ctrl,msg_p->ctrl_buf, msg_p->ctrl_buf_len);
                app_rpc_send_cmd_rsp(ANC_ASSIST_CORE, TASK_ANC_ASSIST_CMD_RSP, NULL, 0);
            } else if (msg_p->cmd == APP_ANC_ASSIST_CORE_SET_MODE) {
                all_users_set_mode(msg_p->ctrl);
            }
            anc_assist_mailbox_free(msg_p);

            while (g_user_status != 0) {
                if (anc_assist_data_is_ready()) {
                    ASSIST_THREAD_LOG_D("Process run start ...");

                    for (uint8_t i = 0; i < g_ff_ch_num; i++) {
                        ff_mic_buf[i] = assist_process_buf[i];
                    }

                    for (uint8_t i = 0; i < g_fb_ch_num; i++) {
                        fb_mic_buf[i] = assist_process_buf[i + MAX_FF_CHANNEL_NUM];
                    }

                    kfifo_get(&(g_fifo.ff_mic_fifo), (uint8_t *)ff_mic_buf[0], ANC_ASSIST_ALGO_FRAME_BYTES);
                    kfifo_get(&(g_fifo.fb_mic_fifo), (uint8_t *)fb_mic_buf[0], ANC_ASSIST_ALGO_FRAME_BYTES);
                    kfifo_get(&(g_fifo.ref_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_REF], ANC_ASSIST_ALGO_FRAME_BYTES);
#if defined(ANC_ASSIST_VPU)
                    kfifo_get(&(g_fifo.vpu_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_VPU], ANC_ASSIST_ALGO_FRAME_BYTES);
#endif 
                    kfifo_get(&(g_fifo.talk_fifo), (uint8_t *)assist_process_buf[MIC_INDEX_TALK], ANC_ASSIST_ALGO_FRAME_BYTES);

                    _process_frame(ff_mic_buf, g_ff_ch_num,
                                    fb_mic_buf, g_fb_ch_num,
                                    assist_process_buf[MIC_INDEX_TALK],
                                    assist_process_buf[MIC_INDEX_REF],
#if defined(ANC_ASSIST_VPU)
                                    assist_process_buf[MIC_INDEX_VPU],
#endif
                                    ANC_ASSIST_ALGO_FRAME_LEN);

                                    ASSIST_THREAD_LOG_D("Process run end ...");
                    if (anc_assist_mailbox_cnt() > ANC_ASSIST_PROCESS_MAIL_THD) {
                        break;
                    }
                } else {
#ifdef ANC_ASSIST_TRIGGER_DSP_PROCESS
                    break;
#else
                    osDelay(3);
#endif
                }
            }
        }
    }
}

int32_t anc_assist_dsp_open(void)
{
    TRACE(1, "[%s] START", __func__);

    g_user_status = 0;
    g_in_queue_len = 0;

    for(uint32_t ch = 0; ch < MIC_INDEX_QTY; ch++) {
        assist_in_queue_buf[ch] = g_in_queue_buf[ch];
        assist_process_buf[ch] = g_process_buf[ch];
    }

    anc_assist_kfifo_init(&g_fifo);

    anc_assist_mailbox_heap_init();

    anc_assist_mailbox = osMailCreate(osMailQ(anc_assist_mailbox), NULL);
    ASSERT(anc_assist_mailbox != NULL, "[%s] Can not create mailbox", __func__);

    anc_assist_thread_tid = osThreadCreate(osThread(anc_assist_thread), NULL);
    ASSERT(anc_assist_thread_tid != NULL, "[%s] Can not create thread", __func__);

    osSignalSet(anc_assist_thread_tid, 0x0);

    return 0;
}

int32_t anc_assist_dsp_close(void)
{
    TRACE(1, "[%s] ...", __func__);

    return 0;
}

int32_t anc_assist_dsp_capture_process(float *in_buf[], uint32_t frame_len)
{
    ANC_ASSIST_MESSAGE_T msg;

    if (anc_assist_mailbox_cnt() < ANC_ASSIST_PROCESS_MAIL_THD) {
        memset(&msg, 0, sizeof(msg));
        msg.user = ANC_ASSIST_USER_QTY;
        msg.cmd = APP_ANC_ASSIST_CORE_PROCESS;
        anc_assist_mailbox_put(&msg);
    }

    return 0;
}

int32_t anc_assist_dsp_capture_cmd(uint8_t *buf, uint32_t len)
{
    ANC_ASSIST_MESSAGE_T msg;
    app_anc_assist_core_to_m55_data_t *rx_cmd = (app_anc_assist_core_to_m55_data_t *)buf;

    TRACE(4, "[%s] user: %d, cmd: %d, status: 0x%x", __func__, rx_cmd->user, rx_cmd->cmd, g_user_status);

    memset(&msg, 0, sizeof(msg));
    msg.user = rx_cmd->user;
    msg.cmd = rx_cmd->cmd;
    msg.ctrl = rx_cmd->ctrl;
    msg.ctrl_buf_len = rx_cmd->ctrl_buf_len;
    if (msg.ctrl_buf_len > 0) {
        msg.ctrl_buf = (uint8_t *)anc_assist_mailbox_heap_malloc(msg.ctrl_buf_len);
        memcpy(msg.ctrl_buf, rx_cmd->ctrl_buf, msg.ctrl_buf_len);
    } else {
        msg.ctrl_buf = NULL;
    }
    anc_assist_mailbox_put(&msg);

    return 0;
}

int32_t anc_assist_dsp_capture_set_mode(uint32_t mode)
{
    app_anc_assist_core_to_m55_data_t rx_cmd;
    memset(&rx_cmd, 0, sizeof(rx_cmd));
    rx_cmd.cmd = APP_ANC_ASSIST_CORE_SET_MODE;
    rx_cmd.ctrl = mode;
    anc_assist_dsp_capture_cmd((uint8_t *)&rx_cmd, sizeof(rx_cmd));

    return 0;
}

int32_t anc_assist_dsp_thread_loop_test(void)
{
    uint32_t cnt = 0;

    while (1) {
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_26M);
        anc_assist_dsp_open();
        hal_sys_timer_delay_us(13000);

        extern void rtx_show_memory_stats(void);
        rtx_show_memory_stats();

        TRACE(2, "[%s] cnt: %d", __func__, cnt++);
    }

    return 0;
}