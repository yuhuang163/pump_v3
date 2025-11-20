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
#include "plat_types.h"
#include "cmsis.h"
#include "cmsis_os2.h"
#include "SEGGER_SYSVIEW.h"
#include "los_task.h"
#include <string.h>
#include "hal_trace.h"

enum SystemViewEventId {
    Preempt = 33,
    KernelSuspend,
    KernelResume,

    TaskError,
    TaskPrioritySet,
    TaskYield,
    TaskSuspend,
    TaskResume,
    TaskBlocked,

    MessageQueueNew = 50,
    MessageQueueGet,
    MessageQueuePut,
    MessageQueueError,

    TimerError = 60,
    TimerNew,
    TimerStart,
    TimerStop,
    TimerDelete,
    TimerCallback,

    TaskFlagsError = 70,
    TaskFlagsSet,
    TaskFlagsClear,
    TaskFlagsGet,
    TaskFlagsWait,

    MutexError = 80,
    MutexNew,
    MutexAcquire,
    MutexRelease,
    MutexDelete,

    SemaphoreError = 90,
    SemaphoreNew,
    SemaphoreAcquire,
    SemaphoreRelease,
    SemaphoreDelete,
};

static uint64_t _cbGetTime() {
    switch (osKernelGetState()) {
    case osKernelRunning: {
        uint64_t uptime_ticks = osKernelGetTickCount();
        return (uptime_ticks * 1000000) / osKernelGetTickFreq();
    }

    case osKernelInactive:
        return 0;

    case osKernelReady:
    case osKernelLocked:
    case osKernelSuspended:
    case osKernelError:
    case osKernelReserved:
    default:
        return g_sysClock;
    }
}

static void SendTaskInfo(const LosTaskCB *task) {

    SEGGER_SYSVIEW_TASKINFO info;

    memset(&info, 0, sizeof(info));
    /* info.TaskID    = id(Task); */
    info.TaskID    = (uint32_t)task->taskID;
    info.sName     = task->taskName;
    info.Prio      = task->priority;
    info.StackBase = (uint32_t)task->topOfStack;
    info.StackSize = task->stackSize;

    SEGGER_SYSVIEW_SendTaskInfo(&info);
}

static void _cbSendTaskList() {
    const LosTaskCB *taskCB;
    int   loopNum = 0;
    for (loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        taskCB = (((LosTaskCB *)g_taskCBArray) + loopNum);
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }
        SendTaskInfo(taskCB);
    }
}

#if !defined(EVR_LOS_DISABLE)

void EvrLosKernelInitialize (void)
{
}

void EvrLosTaskPreempted(uint32_t task_id) {
     SEGGER_SYSVIEW_RecordU32(Preempt, SEGGER_SYSVIEW_ShrinkId(task_id));
}

void EvrLosTaskCreated(LosTaskCB *task_cb) {

    SendTaskInfo(task_cb);
}

void EvrLosTaskExit(uint32_t task_id) {
    SEGGER_SYSVIEW_OnTaskTerminate(task_id);
}

void EvrLosTaskUnblocked(uint32_t task_id, uint32_t retval) {
    SEGGER_SYSVIEW_OnTaskStartReady(task_id);
}

void EvrLosTaskBlocked(uint32_t task_id, uint32_t timeout) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)task_id);
    SEGGER_SYSVIEW_RecordU32x2(TaskBlocked, TaskID, timeout);
}

void EvrLosKernelSuspended(uint32_t ticks) {
    SEGGER_SYSVIEW_RecordU32(KernelSuspend, ticks);
    SEGGER_SYSVIEW_OnIdle();
}
void EvrLosKernelResume(uint32_t sleep_ticks) {
    SEGGER_SYSVIEW_RecordU32(KernelResume, sleep_ticks);
}

void EvrLosTaskSwitched(uint32_t task_id) {
    SEGGER_SYSVIEW_OnTaskStartExec(task_id);
}

void EvrLosTaskError (uint32_t task_id, int32_t status) {
    U32 TaskID;

    if (task_id) {
        TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    } else {
        TaskID = 0;
    }

    SEGGER_SYSVIEW_RecordU32x2(TaskError, TaskID, status);
}

void EvrLosTaskSetPriority (uint32_t task_id, osPriority_t priority) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    SEGGER_SYSVIEW_RecordU32x2(TaskPrioritySet, TaskID, (U32)priority);

    ((LosTaskCB *)task_id)->priority = priority;
    SendTaskInfo(task_id);
}

void EvrLosTaskYield (void) {
    SEGGER_SYSVIEW_RecordVoid(TaskYield);
}

void EvrLosTaskSuspend (uint32_t task_id) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    SEGGER_SYSVIEW_RecordU32(TaskSuspend, TaskID);
}

void EvrLosTaskResume (uint32_t task_id) {
    U32 TaskID;
    TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    SEGGER_SYSVIEW_RecordU32(TaskResume, TaskID);
}

void EvrLosISREnter () {
    SEGGER_SYSVIEW_RecordEnterISR();
}

void EvrLosISRExit () {
    SEGGER_SYSVIEW_RecordExitISR();
}

/*message queue*/
void EvrLosMessageQueueNew (uint32_t msg_count, uint32_t msg_size, uint32_t msg_id) {

    SEGGER_SYSVIEW_RecordU32x3(MessageQueueNew, msg_count, msg_size, msg_id);
}

void EvrLosMessageQueueGet (uint32_t mq_id, void *msg_ptr,
                                uint8_t *msg_prio, uint32_t timeout) {

    SEGGER_SYSVIEW_RecordU32x4(MessageQueueGet, (U32)mq_id, (U32)msg_ptr,
                                            (U32)msg_prio, timeout);
}

void EvrLosMessageQueuePut (uint32_t mq_id, void *msg_ptr,
                                uint8_t *msg_prio, uint32_t timeout) {

    SEGGER_SYSVIEW_RecordU32x4(MessageQueuePut, (U32)mq_id, (U32)msg_ptr,
                                            (U32)msg_prio, timeout);
}

void EvrLosMessageQueueError (osMessageQueueId_t mq_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(MessageQueueError, (U32)mq_id, status);
}


/*timer event*/
void EvrLosTimerError (osTimerId_t timer_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(TimerError, (U32)timer_id, status);
}

void EvrLosTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument,
                                                const osTimerAttr_t *attr) {
    SEGGER_SYSVIEW_RecordU32x4(TimerNew, (U32)func, (U32)type,
                                        (U32)argument, (U32)attr);
}

void EvrLosTimerStart (osTimerId_t timer_id, uint32_t ticks) {
    SEGGER_SYSVIEW_RecordU32x2(TimerStart, (U32)timer_id, ticks);
}

void EvrLosTimerStop (osTimerId_t timer_id) {
    SEGGER_SYSVIEW_RecordU32(TimerStop, (U32)timer_id);
}

void EvrLosTimerDelete (osTimerId_t timer_id) {
    SEGGER_SYSVIEW_RecordU32(TimerDelete, (U32)timer_id);
}

void EvrLosTimerCallback (osTimerFunc_t func, void *argument) {
    SEGGER_SYSVIEW_RecordU32x2(TimerCallback, (U32)func, (U32)argument);
}

/*Task flags event*/
void EvrLosTaskFlagsError (uint32_t task_id, int32_t status) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    SEGGER_SYSVIEW_RecordU32x2(TaskFlagsError, TaskID, status);
}

void EvrLosTaskFlagsSet (uint32_t task_id, uint32_t flags) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId(task_id);
    SEGGER_SYSVIEW_RecordU32x2(TaskFlagsSet, TaskID, flags);
}

void EvrLosTaskFlagsClear (uint32_t flags) {
    SEGGER_SYSVIEW_RecordU32(TaskFlagsClear, flags);
}

void EvrLosTaskFlagsGet (uint32_t Task_flags) {
    SEGGER_SYSVIEW_RecordU32(TaskFlagsGet, Task_flags);
}

void EvrLosTaskFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x3(TaskFlagsWait, flags, options, timeout);
}

void EvrLosTaskDelay (uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32(TaskSuspend, timeout);
}

/*Task flags event*/
void EvrLosMutexError (uint32_t mutex_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(MutexError, mutex_id, status);
}

void EvrLosMutexNew (uint32_t mutex_id) {
    SEGGER_SYSVIEW_RecordU32(MutexNew, mutex_id);
}

void EvrLosMutexAcquire (uint32_t mutex_id, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x2(MutexAcquire, mutex_id, timeout);
}

void EvrLosMutexRelease (uint32_t mutex_id) {
    SEGGER_SYSVIEW_RecordU32(MutexRelease, mutex_id);
}

void EvrLosMutexDelete (uint32_t mutex_id) {
    SEGGER_SYSVIEW_RecordU32(MutexDelete, mutex_id);
}

/*semaphore events*/
void EvrLosSemaphoreError (uint32_t semaphore_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(SemaphoreError, semaphore_id, status);
}

void EvrLosSemaphoreNew (uint32_t max_count, uint32_t initial_count,
                                                 uint32_t semaphore_id) {
    SEGGER_SYSVIEW_RecordU32x3(SemaphoreNew, max_count, initial_count, semaphore_id);
}

void EvrLosSemaphoreAcquire (uint32_t semaphore_id, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x2(SemaphoreAcquire, semaphore_id, timeout);
}

void EvrLosSemaphoreRelease (uint32_t semaphore_id) {
    SEGGER_SYSVIEW_RecordU32(SemaphoreRelease, semaphore_id);
}

void EvrLosSemaphoreDelete (uint32_t semaphore_id) {
    SEGGER_SYSVIEW_RecordU32(SemaphoreDelete, semaphore_id);
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/
// Callbacks provided to SYSTEMVIEW by Loteos
const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI = {
    &_cbGetTime,
    &_cbSendTaskList,
};

#endif
