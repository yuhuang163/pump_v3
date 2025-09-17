/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prtransportr written
 * permisstransportn of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary informattransportn of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "los_task.h"
#include "los_sem.h"
#include "los_mux.h"
#include "los_queue.h"
#include "los_event.h"
#include "los_swtmr.h"
#include "los_hook.h"
#include "los_sysview_cnv.h"
#include "los_sysview_evr.h"
#include "hal_trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

STATIC VOID CnvLosMemInit(VOID *pool, UINT32 size)
{
    EvrLosMemoryInit(pool, size, 1);
}

STATIC VOID CnvLosMemAlloc(VOID *pool, VOID *ptr, UINT32 size)
{
    EvrLosMemoryAlloc(pool, size, 0, ptr);
}

STATIC VOID CnvLosMemFree(VOID *pool, VOID *ptr)
{
    EvrLosMemoryFree(pool, ptr, 1);
}

STATIC VOID CnvLosMemRealloc(VOID *pool, VOID *ptr, UINT32 size)
{
    EvrLosMemoryFree(pool, ptr, 1);
    EvrLosMemoryAlloc(pool, size, 0, ptr);
}

STATIC VOID CnvLosMemAllocAlign(VOID *pool, VOID *ptr, UINT32 size, UINT32 boundary)
{
}

STATIC VOID CnvLosEventInit(PEVENT_CB_S eventCB)
{
    EvrLosEventFlagsNew(eventCB->uwEventID);
}

STATIC VOID CnvLosEventRead(PEVENT_CB_S eventCB, UINT32 eventMask, UINT32 mode, UINT32 timeout)
{
    EvrLosEventFlagsGet(eventCB->uwEventID, eventMask);
}

STATIC VOID CnvLosEventWrite(PEVENT_CB_S eventCB, UINT32 events)
{
    EvrLosEventFlagsSet(eventCB->uwEventID, events);
}

STATIC VOID CnvLosEventClear(PEVENT_CB_S eventCB, UINT32 events)
{
    EvrLosEventFlagsClear(eventCB->uwEventID, events);
}

STATIC VOID CnvLosEventDestroy(PEVENT_CB_S eventCB)
{
    EvrLosEventFlagsDelete(eventCB->uwEventID);
}

STATIC VOID CnvLosQueueCreate(const LosQueueCB *queueCB)
{
    EvrLosMessageQueueNew( queueCB->queueLen, queueCB->queueSize - sizeof(UINT32), queueCB->queueID);
}

STATIC VOID CnvLosQueueRead(const LosQueueCB *queueCB, UINT32 operateType,
                    UINT32 bufferSize, UINT32 timeout)
{
    EvrLosMessageQueueGet(queueCB->queueID, &bufferSize, NULL, timeout);
}

STATIC VOID CnvLosQueueWrite(const LosQueueCB *queueCB, UINT32 operateType,
                    UINT32 bufferSize, UINT32 timeout)
{
    EvrLosMessageQueuePut(queueCB->queueID, &bufferSize, NULL, timeout);
}

STATIC VOID CnvLosQueueDelete(const LosQueueCB *queueCB)
{
    EvrLosMessageQueueDelete(queueCB->queueID);
}

STATIC VOID CnvLosSemCreate(const LosSemCB *semCB)
{
    EvrLosSemaphoreNew(semCB->maxSemCount, semCB->semCount, semCB->semID);
}

STATIC VOID CnvLosSemPost(const LosSemCB *semCB, const LosTaskCB *resumedTask)
{
    (VOID)resumedTask;
    EvrLosSemaphoreRelease(semCB->semID);
}

STATIC VOID CnvLosSemPend(const LosSemCB *semCB, const LosTaskCB *runningTask, UINT32 timeout)
{
    (VOID)runningTask;
    EvrLosSemaphoreAcquire(semCB->semID, timeout);
}

STATIC VOID CnvLosSemDelete(const LosSemCB *semCB)
{
    EvrLosSemaphoreDelete(semCB->semID);
}

STATIC VOID CnvLosMuxCreate(const LosMuxCB *muxCB)
{
    EvrLosMutexNew(muxCB->muxID);
}

STATIC VOID CnvLosMuxPost(const LosMuxCB *muxCB)
{
    EvrLosMutexRelease(muxCB->muxID);
}

STATIC VOID CnvLosMuxPend(const LosMuxCB *muxCB, UINT32 timeout)
{
    EvrLosMutexAcquire(muxCB->muxID, timeout);
}

STATIC VOID CnvLosMuxDelete(const LosMuxCB *muxCB)
{
    EvrLosMutexDelete(muxCB->muxID);
}

STATIC VOID CnvLosTaskCreated(const LosTaskCB *taskCB)
{
    EvrLosTaskCreated(taskCB);
}

STATIC VOID CnvLosTaskPriModify(const LosTaskCB *taskCB, UINT32 prio)
{
    EvrLosTaskPriorityUpdated(taskCB->taskID, prio);
}

STATIC VOID CnvLosTaskDelete(const LosTaskCB *taskCB)
{
    EvrLosTaskDestroyed(taskCB->taskID);
}

STATIC VOID CnvLosTaskSwitchedIn(VOID)
{
    LosTaskCB *newTask = g_losTask.newTask;
    LosTaskCB *runTask = g_losTask.runTask;
    EvrLosTaskSwitched(newTask->taskID);
}

STATIC VOID CnvLosTaskResume(const LosTaskCB *taskCB)
{
    EvrLosTaskResume(taskCB->taskID);
}

STATIC VOID CnvLosTaskSuspend(const LosTaskCB *taskCB)
{
    EvrLosTaskSuspend(taskCB->taskID);
}

STATIC VOID CnvLosIsrEnter(UINT32 hwiNum)
{
    //EvrLosISREnter();
}

STATIC VOID CnvLosIsrExit(UINT32 hwiNum)
{
    //EvrLosISRExit();
}

STATIC VOID CnvLosSwtmrCreate(const SWTMR_CTRL_S *swtmr)
{
    //EvrLosTimerNew(swtmr->usTimerID);
}

STATIC VOID CnvLosSwtmrDelete(const SWTMR_CTRL_S *swtmr)
{
    EvrLosTimerDelete(swtmr->usTimerID);
}

STATIC VOID CnvLosSwtmrExpired(const SWTMR_CTRL_S *swtmr)
{
    EvrLosTimerCallback(swtmr->pfnHandler, swtmr->uwArg);
}

STATIC VOID CnvLosSwtmrStart(const SWTMR_CTRL_S *swtmr)
{
    EvrLosTimerStart(swtmr->usTimerID, swtmr->uwInterval);
}

STATIC VOID CnvLosSwtmrStop(const SWTMR_CTRL_S *swtmr)
{
    EvrLosTimerStop(swtmr->usTimerID);
}

VOID CnvLosInit(VOID)
{
    LOS_HookReg(LOS_HOOK_TYPE_MEM_ALLOC, CnvLosMemAlloc);
    LOS_HookReg(LOS_HOOK_TYPE_MEM_FREE, CnvLosMemFree);
    LOS_HookReg(LOS_HOOK_TYPE_MEM_INIT, CnvLosMemInit);
    LOS_HookReg(LOS_HOOK_TYPE_MEM_REALLOC, CnvLosMemRealloc);
    LOS_HookReg(LOS_HOOK_TYPE_MEM_ALLOCALIGN, CnvLosMemAllocAlign);
    LOS_HookReg(LOS_HOOK_TYPE_EVENT_INIT, CnvLosEventInit);
    LOS_HookReg(LOS_HOOK_TYPE_EVENT_READ, CnvLosEventRead);
    LOS_HookReg(LOS_HOOK_TYPE_EVENT_WRITE, CnvLosEventWrite);
    LOS_HookReg(LOS_HOOK_TYPE_EVENT_CLEAR, CnvLosEventClear);
    LOS_HookReg(LOS_HOOK_TYPE_EVENT_DESTROY, CnvLosEventDestroy);
    LOS_HookReg(LOS_HOOK_TYPE_QUEUE_CREATE, CnvLosQueueCreate);
    LOS_HookReg(LOS_HOOK_TYPE_QUEUE_DELETE, CnvLosQueueDelete);
    LOS_HookReg(LOS_HOOK_TYPE_QUEUE_READ, CnvLosQueueRead);
    LOS_HookReg(LOS_HOOK_TYPE_QUEUE_WRITE, CnvLosQueueWrite);
    LOS_HookReg(LOS_HOOK_TYPE_SEM_CREATE, CnvLosSemCreate);
    LOS_HookReg(LOS_HOOK_TYPE_SEM_DELETE, CnvLosSemDelete);
    LOS_HookReg(LOS_HOOK_TYPE_SEM_POST, CnvLosSemPost);
    LOS_HookReg(LOS_HOOK_TYPE_SEM_PEND, CnvLosSemPend);
    LOS_HookReg(LOS_HOOK_TYPE_MUX_CREATE, CnvLosMuxCreate);
    LOS_HookReg(LOS_HOOK_TYPE_MUX_POST, CnvLosMuxPost);
    LOS_HookReg(LOS_HOOK_TYPE_MUX_PEND, CnvLosMuxPend);
    LOS_HookReg(LOS_HOOK_TYPE_MUX_DELETE, CnvLosMuxDelete);
    LOS_HookReg(LOS_HOOK_TYPE_TASK_PRIMODIFY, CnvLosTaskPriModify);
    LOS_HookReg(LOS_HOOK_TYPE_TASK_DELETE, CnvLosTaskDelete);
    LOS_HookReg(LOS_HOOK_TYPE_TASK_CREATE, CnvLosTaskCreated);
    LOS_HookReg(LOS_HOOK_TYPE_TASK_SWITCHEDIN, CnvLosTaskSwitchedIn);
    LOS_HookReg(LOS_HOOK_TYPE_ISR_ENTER, CnvLosIsrEnter);
    LOS_HookReg(LOS_HOOK_TYPE_ISR_EXIT, CnvLosIsrExit);
    LOS_HookReg(LOS_HOOK_TYPE_SWTMR_CREATE, CnvLosSwtmrCreate);
    LOS_HookReg(LOS_HOOK_TYPE_SWTMR_DELETE, CnvLosSwtmrDelete);
    LOS_HookReg(LOS_HOOK_TYPE_SWTMR_EXPIRED, CnvLosSwtmrExpired);
    LOS_HookReg(LOS_HOOK_TYPE_SWTMR_START, CnvLosSwtmrStart);
    LOS_HookReg(LOS_HOOK_TYPE_SWTMR_STOP, CnvLosSwtmrStop);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
