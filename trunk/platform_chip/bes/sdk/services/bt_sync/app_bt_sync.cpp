/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "plat_types.h"
#include "bt_drv_interface.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "app_bt_sync.h"
#include "hal_timer.h"
#include <stdarg.h>
#ifdef IBRT
#include "app_tws_ibrt_cmd_handler.h"
#include "audio_policy.h"
#include "earbud_ux_api.h"
#include "app_tws_ctrl_thread.h"
#endif
#ifdef USE_BASIC_THREADS
#include "app_thread.h"
#endif

#if defined(__BT_SYNC__) && defined(IBRT)
// #define BT_SYNC_DEBUG_LEVEL
#define BT_SYNC_INFO_LEVEL
#ifdef BT_SYNC_DEBUG_LEVEL
#define BT_SYNC_DBG                     TRACE
#else
#define BT_SYNC_DBG(...)                do{} while(0)
#endif

#ifdef BT_SYNC_INFO_LEVEL
#define BT_SYNC_INFO                    TRACE
#else
#define BT_SYNC_INFO(...)               do{} while(0)
#endif
#define BT_SYNC_WARN                    TRACE
#define BT_SYNC_ERROR                   TRACE


static bool bt_sync_initilized = false;
static BT_SYNC_ENV_T bt_sync_env;

#ifndef USE_BASIC_THREADS
static osThreadId bt_sync_thread_tid = NULL;
static void app_bt_sync_thread(void const *argument);
osThreadDef(app_bt_sync_thread, osPriorityRealtime, 1, APP_BT_SYNC_THREAD_STACK_SIZE, "BT_SYNC");
osMailQDef(bt_sync_mailbox, 5, BT_SYNC_MSG_BLOCK_T);
static osMailQId bt_sync_mailbox = NULL;
#endif // USE_BASIC_THREADS

static APP_BT_SYNC_INFO_REPORT_T bt_sync_share_info_report_func = NULL;

static void app_bt_sync_supervision_timeout_handler(void const *param);
osTimerDef(BT_SYNC_SUPERVISION_TIMER, app_bt_sync_supervision_timeout_handler);
static osTimerId bt_sync_supervision_timer_id = NULL;
osMutexDef(bt_sync_mutex);
static osMutexId bt_sync_mutex_id = NULL;

// internal API
static BT_SYNC_JOB_LIST_T *app_bt_sync_seek_job_handler(uint32_t opCode)
{
    uint32_t total_job_cnt = ((uint32_t)__app_bt_sync_command_handler_table_end-
        (uint32_t)__app_bt_sync_command_handler_table_start) /
        sizeof(BT_SYNC_JOB_LIST_T);

    for (uint32_t i = 0; i < total_job_cnt; i++)
    {
        BT_SYNC_JOB_LIST_T *p_job = APP_BT_SYNC_COMMAND_PTR_FROM_ENTRY_INDEX(i);
        if (p_job->opCode == opCode)
        {
            return p_job;
        }
    }

    return NULL;
}

static BT_SYNC_INSTANCE_T *app_bt_sync_seek_work_instance(uint32_t opCode)
{
    BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[0]);
    uint32_t total_work_cnt = bt_sync_env.supervisorCnt;

    for (uint32_t i = 0; i < total_work_cnt; i++) {
        if (opCode == p_work->triInfo.opCode) {
            return p_work;
        }
        p_work++;
    }

    return NULL;
}

// supervision list
#ifdef BT_SYNC_DEBUG_LEVEL
static void app_bt_sync_dump_supervison(void)
{
    BT_SYNC_INSTANCE_T *p_work= &bt_sync_env.workInstance[0];

    for (uint32_t i = 0; i < bt_sync_env.supervisorCnt; i++) {
        BT_SYNC_DBG(1, "(d%d)bt_sync_dump, opCode:0x%x, chl:%d, msRemain:%u", i,
            p_work->triInfo.opCode, p_work->triInfo.triChl, p_work->msTillTimeout);
    }
}
#endif

static void app_bt_sync_refresh_supervison_env(void)
{
    uint32_t totalCnt = bt_sync_env.supervisorCnt;
    // do nothing if no supervisor was added
    if (totalCnt > 0)
    {
        uint32_t currentTicks = GET_CURRENT_TICKS();
        uint32_t passedTicks = 0;

        if (currentTicks >= bt_sync_env.lastSysTicks)
        {
            passedTicks = (currentTicks - bt_sync_env.lastSysTicks);
        }
        else
        {
            passedTicks = (hal_sys_timer_get_max() - bt_sync_env.lastSysTicks + 1) + currentTicks;
        }

        uint32_t deltaMs = TICKS_TO_MS(passedTicks);

        BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[0]);
        for (uint32_t i = 0; i < totalCnt; i++)
        {
            ASSERT(p_work->msTillTimeout >= deltaMs,
                "(d%d)Supervisor timer missed,%d ms passed but the timeout is %d",
                i, deltaMs, p_work->msTillTimeout);

            BT_SYNC_INFO(2, "(d%d)refresh_supervisor:%u-%u, cnt:%d", i,
                p_work->msTillTimeout, deltaMs, totalCnt);

            p_work->msTillTimeout -= deltaMs;
            p_work++;
        }
    }

    bt_sync_env.lastSysTicks = GET_CURRENT_TICKS();
}

static void app_bt_sync_remove_out_supervision(uint32_t opCode)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);
    uint32_t total_cnt = bt_sync_env.supervisorCnt;
    ASSERT(total_cnt > 0, "bt_sync_remove_supervision empty");
    uint32_t index = 0;
    BT_SYNC_INSTANCE_T *p_work= &bt_sync_env.workInstance[0];

    BT_SYNC_INFO(1, "bt_sync_remove_supervision, opCode:%d, cnt:%d", opCode, total_cnt);
    for (index = 0; index < total_cnt; index++)
    {
        if (p_work->triInfo.opCode == opCode)
        {
            if ((index + 1) != total_cnt) {
                // not last element
                memmove(p_work, p_work + 1, (total_cnt - index - 1) * sizeof(BT_SYNC_INSTANCE_T));
            }
            // already find
            break;
        }
        p_work++;
    }

    // cannot find it, directly return
    if (index == total_cnt)
    {
        BT_SYNC_WARN(1, "bt_sync_remove_supervision, not found opCode:%d", opCode);
        goto exit;
    }

    bt_sync_env.supervisorCnt--;

    if (bt_sync_env.supervisorCnt > 0)
    {
        // refresh supervisor environment firstly
        app_bt_sync_refresh_supervison_env();

        // start timer, the first entry is the most close one
        osTimerStart(bt_sync_supervision_timer_id, bt_sync_env.workInstance[0].msTillTimeout);
    }
    else
    {
        // no supervisor, directly stop the timer
        osTimerStop(bt_sync_supervision_timer_id);
    }

exit:
#ifdef BT_SYNC_DEBUG_LEVEL
    app_bt_sync_dump_supervison();
#endif
    osMutexRelease(bt_sync_mutex_id);
}

static void app_bt_sync_add_into_supervision_list(uint32_t opCode, uint8_t index)
{
    ASSERT(bt_sync_env.supervisorCnt < APP_BT_SYNC_CHANNEL_MAX,
        "%s supervisor full:%d", __func__, opCode);

    osMutexWait(bt_sync_mutex_id, osWaitForever);

    // refresh supervisor environment firstly
    app_bt_sync_refresh_supervison_env();

    bt_sync_env.workInstance[index].msTillTimeout = APP_BT_SYNC_SUPERVISON_TIMEOUT;
    bt_sync_env.supervisorCnt++;

#ifdef BT_SYNC_DEBUG_LEVEL
    app_bt_sync_dump_supervison();
#endif
    // start timer, the first entry is the most close one
    osTimerStart(bt_sync_supervision_timer_id, bt_sync_env.workInstance[0].msTillTimeout);
    osMutexRelease(bt_sync_mutex_id);
}

#ifdef USE_BASIC_THREADS
static int app_bt_sync_handle(APP_MESSAGE_BODY *msg_body)
{
    uint32_t index = msg_body->message_Param1;
    bool triResult = !!msg_body->message_Param0;
    uint32_t opCode = 0xFFFF;
    BT_SYNC_JOB_LIST_T *p_job_handler = NULL;
    BT_SYNC_INSTANCE_T *p_work = NULL;
    bool bShareStatus = false;

    p_work = &(bt_sync_env.workInstance[index]);
    opCode = p_work->triInfo.opCode;
    p_job_handler = app_bt_sync_seek_job_handler(opCode);

    BT_SYNC_INFO(4, "bt_sync_thread, job:%p-%p Code:0x%x, idx:%d, result:%d",
        p_work, p_job_handler, opCode, index, triResult);

    if (p_work) {
        p_work->used = 0;
        app_bt_sync_remove_out_supervision(opCode);
        bShareStatus = !!p_work->twsShareDone;
        if (p_job_handler && p_job_handler->statusNotify)
        {
            p_job_handler->statusNotify(opCode, triResult, bShareStatus);
        }
    }

    if ((triResult) && (p_job_handler) &&
        (p_job_handler->syncCallback))
    {
        p_job_handler->syncCallback();
    }

    return 0;
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    uint32_t opCode = bt_sync_env.workInstance[0].triInfo.opCode;

    TRACE(2, "bt_sync_timeout, opCode:%d", opCode);

    bt_sync_env.workInstance[0].used = false;

    bt_drv_i2v_enable_sleep_for_bt_access();

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODUAL_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_FAILED;
    msg.msg_body.message_Param1 = 0;
    app_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index > APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_ERROR(1, "bt_sync_irq_handler, index:%d error", index);
        return;
    }
    BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_INFO(3, "bt_sync_irq_handler, src:%d opCode:0x%x",
        src, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    bt_drv_i2v_enable_sleep_for_bt_access();

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODUAL_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_PASS;
    msg.msg_body.message_Param1 = index;
    app_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    if (!bt_sync_initilized) {
        app_set_threadhandle(APP_MODUAL_BTSYNC, app_bt_sync_handle);

        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
        if (!bt_sync_supervision_timer_id)
        {
            return -1;
        }

        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
        if (!bt_sync_mutex_id)
        {
            return -2;
        }

        memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));
        bt_sync_initilized = true;
     }

    return 0;
}

#else // USE_BASIC_THREADS

static int app_bt_sync_mailbox_free(BT_SYNC_MSG_BLOCK_T *msg_p)
{
    osStatus status;

    status = osMailFree(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        BT_SYNC_ERROR(1, "%s failed to free", __func__);
    }
    return (int)status;
}

static int app_bt_sync_mailbox_get(BT_SYNC_MSG_BLOCK_T **msg_p)
{
    osEvent evt;
    evt = osMailGet(bt_sync_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (BT_SYNC_MSG_BLOCK_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int app_bt_sync_mailbox_put(BT_SYNC_MSG_BLOCK_T *msg_src)
{
    osStatus status;
    BT_SYNC_MSG_BLOCK_T *msg_p = NULL;

    msg_p = (BT_SYNC_MSG_BLOCK_T *)osMailAlloc(bt_sync_mailbox, 0);

    if (!msg_p) {
        BT_SYNC_ERROR(1, "%s failed to alloc", __func__);
        return -1;
    }

    *msg_p = *msg_src;
    status = osMailPut(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        TRACE(1, "%s failed to put", __func__);
    }

    return (int)status;
}

static void app_bt_sync_thread(void const *argument)
{
    while(1)
    {
        BT_SYNC_MSG_BLOCK_T *msg_p = NULL;
        uint32_t index = APP_BT_SYNC_INVALID_CHANNEL;
        BT_SYNC_JOB_LIST_T *p_job_handler = NULL;
        BT_SYNC_INSTANCE_T *p_work = NULL;
        bool bShareStatus = false;
        bool triResult = false;
        uint32_t opCode = 0xFFFF;

        if (!app_bt_sync_mailbox_get(&msg_p)) {
            index = msg_p->workIdx;
            triResult = !!msg_p->triResult;

            app_bt_sync_mailbox_free(msg_p);

            p_work = &(bt_sync_env.workInstance[index]);
            opCode = p_work->triInfo.opCode;
            p_job_handler = app_bt_sync_seek_job_handler(opCode);

            BT_SYNC_INFO(4, "bt_sync_thread, job:%p-%p code:0x%x, idx:0x%x",
                p_work, p_job_handler, opCode, index);

            if (p_work) {
                p_work->used = 0;
                app_bt_sync_remove_out_supervision(opCode);
                bShareStatus = !!p_work->twsShareDone;
                if (p_job_handler && p_job_handler->statusNotify)
                {
                    p_job_handler->statusNotify(opCode, triResult, bShareStatus);
                }
            }

            if ((triResult) && (p_job_handler) &&
                (p_job_handler->syncCallback))
            {
                p_job_handler->syncCallback();
            }
        }
    }
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    BT_SYNC_WARN(0, "bt_sync_timeout");
    bt_drv_i2v_enable_sleep_for_bt_access();
    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_FAILED;
    msg.workIdx = 0;
    app_bt_sync_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index > APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_ERROR(1, "bt_sync_irq_handler, index:%d error", index);
        return;
    }

    BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_INFO(3, "bt_sync_irq_handler, src:%d opCode:0x%x",
        src, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    bt_drv_i2v_enable_sleep_for_bt_access();

    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_PASS;
    msg.workIdx = index;
    app_bt_sync_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    if (!bt_sync_initilized) {
        bt_sync_mailbox = osMailCreate(osMailQ(bt_sync_mailbox), NULL);
        if (!bt_sync_mailbox) {
            return -1;
        }

        bt_sync_thread_tid = osThreadCreate(osThread(app_bt_sync_thread), NULL);
        if (!bt_sync_thread_tid) {
            return -2;
        }

        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
        if (!bt_sync_supervision_timer_id)
        {
            return -3;
        }

        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
        if (!bt_sync_mutex_id)
        {
            return -4;
        }

        memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));
        bt_sync_initilized = true;
     }

    return 0;
}
#endif // USE_BASIC_THREADS

// external API
void app_bt_sync_get_master_time_from_slave_time(uint32_t SlaveBtTicksUs,
    uint32_t* p_master_clk_cnt, uint16_t* p_master_bit_cnt)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(p_ibrt_ctrl->tws_conhandle);
    uint32_t slaveClockCnt;
    uint16_t finecnt;
    btdrv_reg_op_bts_to_bt_time(SlaveBtTicksUs, &slaveClockCnt, &finecnt);

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

    *p_master_clk_cnt = (slaveClockCnt + offset) & 0x0fffffff;
    *p_master_bit_cnt = finecnt;
}

uint32_t app_bt_sync_get_slave_time_from_master_time(uint32_t master_clk_cnt, uint16_t master_bit_cnt)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(p_ibrt_ctrl->tws_conhandle);

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }
    uint32_t slaveClockCnt = (master_clk_cnt - offset) & 0x0fffffff;

    uint32_t SlaveBtTicksUs = btdrv_reg_op_bt_time_to_bts(slaveClockCnt, master_bit_cnt);
    return SlaveBtTicksUs;
}

void app_bt_sync_get_current_tws_bt_ticks_in_us_on_slave_side(uint32_t* pMasterBtClockCnt,
    uint32_t* pSlaveBtTicksUs)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    int32_t offset;

    uint32_t slaveBtClkCnt = btdrv_syn_get_curr_ticks();

    uint8_t link_id = btdrv_conhdl_to_linkid(p_ibrt_ctrl->tws_conhandle);
    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

    *pMasterBtClockCnt = (slaveBtClkCnt + offset) & 0x0fffffff;

    *pSlaveBtTicksUs = btdrv_reg_op_bt_time_to_bts(slaveBtClkCnt, 0);
}

uint8_t app_bt_sync_get_avaliable_trigger_channel(uint32_t opcode = APP_BT_SYNC_OP_MAX,
    uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    // Need to initilize first.
    // Otherwise the variable locTriggerInfo will be cleared by sync module
    int ret = app_bt_sync_module_init();
    if (ret) {
        BT_SYNC_WARN(2, "bt_sync_get_ava_chl, init failed:%d", ret);
        return APP_BT_SYNC_INVALID_CHANNEL;
    }

    uint32_t index = 0;
    uint32_t chl = APP_BT_SYNC_INVALID_CHANNEL;
    BT_SYNC_INSTANCE_T *p_instance = &bt_sync_env.workInstance[0];

    for (; index < APP_BT_SYNC_CHANNEL_MAX; index++)
    {
        if (p_instance->used
            && (opcode == p_instance->triInfo.opCode)
            && (APP_BT_SYNC_POLICY_UPDATE == policy))
        {
            btdrv_syn_clr_trigger(p_instance->triInfo.triChl);
            hal_cmu_bt_trigger_disable((HAL_CMU_BT_TRIGGER_SRC_T)p_instance->triInfo.triChl);
            BT_SYNC_INFO(1,"bt_sync_get_ava_chl, reuse chl:%d", p_instance->triInfo.triChl);
            goto FIND_CHL;
        }

        if (!p_instance->used)
        {
            break;
        }
        p_instance++;
    }

FIND_CHL:
    if (index < APP_BT_SYNC_CHANNEL_MAX)
    {
        chl = index + APP_BT_SYNC_CHANNEL_OFFSET;
        p_instance = &bt_sync_env.workInstance[index];
        p_instance->used = 1;
        p_instance->triInfo.opCode = opcode;
        // channel0 is reserved for A2DP/HFP, channel1 is reserved for prompt
        p_instance->triInfo.triChl = chl;
    }

    BT_SYNC_INFO(2, "bt_sync_get_ava_chl, chl:%d, opcode:%d", chl, opcode);
    return chl;
}

void app_bt_sync_register_report_info_callback(APP_BT_SYNC_INFO_REPORT_T cb)
{
    bt_sync_share_info_report_func = cb;
}

void app_bt_sync_send_tws_cmd_done(uint8_t *ptrParam, uint16_t paramLen)
{
    if (!ptrParam || (paramLen < sizeof(BT_SYNC_TRIGGER_INFO_T))) {
        BT_SYNC_ERROR(1, "%s, len error:%d", __func__, paramLen);
        return;
    }

    BT_SYNC_TRIGGER_INFO_T triInfo;
    memcpy((uint8_t*)&triInfo, ptrParam, sizeof(BT_SYNC_TRIGGER_INFO_T));

    BT_SYNC_INSTANCE_T *p_work = NULL;

    p_work = app_bt_sync_seek_work_instance(triInfo.opCode);
    if (p_work) {
        p_work->twsShareDone = 1;
    }
}

void app_bt_sync_tws_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    uint32_t len_tri_info = sizeof(BT_SYNC_TRIGGER_INFO_T);
    if (!p_buff || (length < len_tri_info)) {
        BT_SYNC_ERROR(1, "%s, len error:%d", __func__, length);
        return;
    }

    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint32_t curr_ticks = 0;
    int ret = 0;
    uint32_t chnl = APP_BT_SYNC_INVALID_CHANNEL;
    uint32_t index = 0;
    uint8_t btRole = BTIF_BCR_MASTER;
    BT_SYNC_TRIGGER_INFO_T triInfo;
    memcpy((uint8_t*)&triInfo, p_buff, len_tri_info);

    if (bt_sync_share_info_report_func && (length > len_tri_info)) {
        bt_sync_share_info_report_func(triInfo.opCode, p_buff + len_tri_info, length - len_tri_info);
    }

    btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);
    BT_SYNC_INFO(4, "bt_sync_tws_cmd_handler, opCode:%d role:%d tick:%u, chl:%d",
        triInfo.opCode, btRole, triInfo.triTick, triInfo.triChl);

    if (BTIF_BCR_SLAVE == btRole)
    {
        /**
        * Step1: Do sanity check here
        * 1. Module init failed
        * 2. TWS not connected
        * 3. The maximum limit has been reached.
        * 4. Check trigger ticks
        *
        */
        ret = app_bt_sync_module_init();
        if (ret) {
            BT_SYNC_WARN(2, "bt_sync_tws_cmd_handler, init failed:%d", ret);
            ret = -1;
            goto exit;
        }

        if (BT_INVALID_CONN_HANDLE == p_ibrt_ctrl->tws_conhandle) {
            ret = -2;
            goto exit;

        }

        chnl = app_bt_sync_get_avaliable_trigger_channel(triInfo.opCode, triInfo.policy);
        if (APP_BT_SYNC_INVALID_CHANNEL == chnl) {
            ret = -3;
            goto exit;

        }

        index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
        curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->tws_conhandle);
        BT_SYNC_WARN(2, "bt_sync_tws_cmd_handler, curr tick:%u", curr_ticks);
        if (curr_ticks >= triInfo.triTick){
            ret = -4;
            goto exit;
        }

        bt_drv_i2v_disable_sleep_for_bt_access();
        bt_syn_set_tg_ticks(triInfo.triTick, p_ibrt_ctrl->tws_conhandle,
            BT_TRIG_SLAVE_ROLE, chnl, false);
        hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
            app_bt_sync_irq_handler);
        hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);
        app_bt_sync_add_into_supervision_list(triInfo.opCode, index);
    }
    else if (BTIF_BCR_MASTER == btRole)
    {
        ret = app_bt_sync_enable(triInfo.opCode, 0, NULL, triInfo.policy);
    }
    else
    {
        ret = -5;
    }

exit:
    BT_SYNC_INFO(1, "bt_sync_tws_cmd_handler, ret:%d", ret);
    return;
}

void app_bt_sync_release_trigger_channel(uint8_t chnl)
{
    uint8_t index = 0;
    BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[0];
    for (; index < APP_BT_SYNC_CHANNEL_MAX; index++) {
        if (chnl == p_work->triInfo.triChl) {
            break;
        }
        p_work++;
    }

    if (APP_BT_SYNC_CHANNEL_MAX > index)
    {
        if (!p_work->used) {
            BT_SYNC_WARN(1, "bt_sync_release_chl, chl:%d not used", chnl);
        } else {
            p_work->used = 0;
            bt_drv_i2v_enable_sleep_for_bt_access();
            BT_SYNC_INFO(1, "bt_sync_release_chl, chl:%d opcode:%d", chnl, p_work->triInfo.opCode);
        }
    }
    else
    {
        BT_SYNC_WARN(1, "bt_sync_release_chl, chl:%d not found", chnl);
    }
}

bool app_bt_sync_enable(uint32_t opCode, uint8_t length,
    uint8_t *p_buff, uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    BT_SYNC_TRIGGER_INFO_T tri_info = {0};
    uint32_t tg_tick = 0;
    uint32_t curr_ticks = 0;
    uint8_t index = 0xff;
    uint8_t btRole = BTIF_BCR_MASTER;
    uint32_t delayUs = 0;
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t *mobile_addr = NULL;
    bool mobileInSniff = false;
    uint8_t extra_share_buf[128] = {0};
    int ret = 0;
    rx_agc_t link_agc_info = {0};
    btif_cmgr_handler_t *cmgrHandler = NULL;
    uint32_t len_struct = 0;
    struct BT_DEVICE_T *device = NULL;
    uint32_t device_id = BT_DEVICE_INVALID_ID;
    BT_SYNC_INSTANCE_T *p_work = NULL;

    /**
    * Step1: Do sanity check here
    * 1. TWS not connected or manager handler is null
    * 2. The maximum limit has been reached.
    * 3. Check to see if there is the same. If yes. then check policy
    * 4. (Support share info to peer bud) The length of share info exceeds the max allowed length
    *    Max size: ARRAY_SIZE(shareBuf) - sizeof(BT_SYNC_TRIGGER_INFO_T)
    * 5. (Support share info to peer bud) Length is not zero, but the pointer is empty.
    * 6. Check start iBRT on process or not (???)
    *
    */
    if (p_ibrt_ctrl->tws_conhandle == BT_INVALID_CONN_HANDLE) {
        ret = -1;
        goto exit;
    }

    cmgrHandler = btif_lock_free_cmgr_get_acl_handler(p_ibrt_ctrl->p_tws_remote_dev);
    if (!cmgrHandler) {
        ret = -2;
        goto exit;
    }

    p_work = app_bt_sync_seek_work_instance(opCode);
    if (p_work) {
        if (APP_BT_SYNC_POLICY_MULTIPLEX == policy) {
            goto exit;
        } else if (APP_BT_SYNC_POLICY_UPDATE == policy) {
            goto CHECK_DONE;
        } else  {
            ret = -3;
            goto exit;
        }
    }

    if (bt_sync_initilized) {
        osMutexWait(bt_sync_mutex_id, osWaitForever);
        if (bt_sync_env.supervisorCnt >= APP_BT_SYNC_CHANNEL_MAX) {
            ret = -4;
            osMutexRelease(bt_sync_mutex_id);
            goto exit;
        }
        osMutexRelease(bt_sync_mutex_id);
    }

    len_struct = sizeof(BT_SYNC_TRIGGER_INFO_T);
    if ((length > (ARRAY_SIZE(extra_share_buf) - len_struct)) ||
        ((length > 0) && (!p_buff))) {
        ret = -5;
        goto exit;
    }

    device_id = app_bt_audio_get_curr_a2dp_device();
    device = app_bt_get_device(device_id);
    if (app_ibrt_if_start_ibrt_onporcess(&device->remote))
    {
        ret = -6;
        goto exit;
    }

    /**
    * Step2: Initialize the module
    *
    */
    ret = app_bt_sync_module_init();
    if (ret) {
        BT_SYNC_WARN(2, "bt_sync_enable, init failed:%d", ret);
        ret = -7;
        goto exit;
    }

CHECK_DONE:
    /**
    * Step3: Calculate trigger time if it is BT central
    *
    */
    btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);
    if (BTIF_BCR_SLAVE == btRole) {
        goto cal_done;
    }

    if (app_ibrt_conn_any_mobile_connected()) {
        for (uint32_t i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            mobile_addr = &curr_device->remote;
            p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(mobile_addr);

            if ((NULL != p_mobile_info) && (p_mobile_info->mobile_mode == IBRT_SNIFF_MODE)) {
                mobileInSniff = true;
                app_tws_ibrt_exit_sniff_with_mobile(mobile_addr);
                break;
            }
        }
    }

    if ((mobileInSniff) || (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE)) {
        app_tws_ibrt_exit_sniff_with_tws();
        uint16_t twsSnifferInterval = btif_cmgr_get_cmgrhandler_sniff_interval(cmgrHandler);
        BT_SYNC_INFO(2, "bt_sync_enable, in sniff mode,itv:%d-%d", twsSnifferInterval, mobileInSniff);
        delayUs = twsSnifferInterval * 1000 * 2;
    } else {
        if (p_ibrt_ctrl->acl_interval < IBRT_UI_DEFAULT_POLL_INTERVAL) {
            delayUs = IBRT_UI_DEFAULT_POLL_INTERVAL * 625 * 4;
        } else {
            delayUs = p_ibrt_ctrl->acl_interval * 625 * 4;
        }
    }

    if (bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(p_ibrt_ctrl->p_tws_remote_dev),
        &link_agc_info))
    {
        BT_SYNC_DBG(1, "bt_sync_enable, TWS-RSSI:%d", link_agc_info.rssi);
        if (link_agc_info.rssi < -90) {
            delayUs *= 4;
        } else if (link_agc_info.rssi < -85) {
            delayUs *= 2;
        }
    }

    curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->tws_conhandle);
    tg_tick = curr_ticks + US_TO_BTCLKS(delayUs);
    tri_info.triTick = tg_tick;

cal_done:
    /**
    * Step4: Share with peer bud if it is BT central
    *
    */
    tri_info.opCode = opCode;
    tri_info.policy = policy;
    memcpy(extra_share_buf, (uint8_t *)&tri_info, len_struct);
    if (length)
    {
        memcpy(extra_share_buf + len_struct, p_buff, length);
    }

    if (!tws_ctrl_send_cmd_high_priority(APP_TWS_CMD_SET_SYNC_TIME, extra_share_buf,
        len_struct + length))
    {
        if (btRole == BTIF_BCR_MASTER)
        {
             /// get the trigger channel
            uint32_t chnl = app_bt_sync_get_avaliable_trigger_channel(opCode);

            if (chnl < APP_BT_SYNC_CHANNEL_TOTAL) {
                /// get the index of trigger management module(
                /// minus 2 for reserved channel0 for A2DP/HFP, channel 1 for prompt)
                index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
            }  else {
                ret = -8;
                goto exit;
            }

            BT_SYNC_INSTANCE_T *p_instance = &bt_sync_env.workInstance[index];
            p_instance->triInfo.triTick = tg_tick;
            BT_SYNC_INFO(4, "bt_sync_enable, triSrc:%d, curr tick:%u+delayUs:%d > tg_tick:%u",
                p_instance->triInfo.triChl, curr_ticks, delayUs, tg_tick);

            /**
            * Step5: enable BT STAMP
            *
            */
            bt_drv_i2v_disable_sleep_for_bt_access();
            bt_syn_set_tg_ticks(tg_tick, p_ibrt_ctrl->tws_conhandle,
                BT_TRIG_MASTER_ROLE, chnl, false);

            hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
                app_bt_sync_irq_handler);
            hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);

            app_bt_sync_add_into_supervision_list(opCode, index);
        }
    }
    else
    {
        ret = -9;
    }

exit:
    BT_SYNC_INFO(1, "bt_sync_enable, ret:%d", ret);
    return ret ? false : true;
}
#endif

