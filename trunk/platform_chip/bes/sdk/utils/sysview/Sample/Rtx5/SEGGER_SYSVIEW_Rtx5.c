/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "cmsis_os.h"
#include "SEGGER_SYSVIEW.h"
#include <string.h>
#include "rtx_os.h"

enum SystemViewEventId {
    Preempt = 33,
    KernelSuspend,
    KernelResume,

    ThreadError,
    ThreadPrioritySet,
    ThreadYield,
    ThreadSuspend,
    ThreadResume,
    ThreadBlocked,

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

    ThreadFlagsError = 70,
    ThreadFlagsSet,
    ThreadFlagsClear,
    ThreadFlagsGet,
    ThreadFlagsWait,

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
        return osRtxInfo.kernel.tick;
    }
}

static void SendThreadInfo(const osRtxThread_t *thread) {

    SEGGER_SYSVIEW_TASKINFO info;

    memset(&info, 0, sizeof(info));
    /* info.TaskID    = id(thread); */
    info.TaskID    = (uint32_t)thread;
    info.sName     = thread->name;
    info.Prio      = thread->priority;
    info.StackBase = (uint32_t)thread->stack_mem;
    info.StackSize = thread->stack_size;

    SEGGER_SYSVIEW_SendTaskInfo(&info);
}

static void _cbSendTaskList() {
  const osRtxThread_t *thread;

  thread = osRtxInfo.thread.run.curr;
  SendThreadInfo(thread);

  for (thread = osRtxInfo.thread.ready.thread_list;
       thread != NULL; thread = thread->thread_next) {
                SendThreadInfo(thread);
  }

  // Delay List
  for (thread = osRtxInfo.thread.delay_list;
       thread != NULL; thread = thread->delay_next) {
                SendThreadInfo(thread);
  }

  // Wait List
  for (thread = osRtxInfo.thread.wait_list;
       thread != NULL; thread = thread->delay_next) {
                SendThreadInfo(thread);
  }
}

#if !defined(EVR_RTX_DISABLE)

void EvrRtxKernelInitialize (void)
{
}

void EvrRtxThreadPreempted(osThreadId_t thread_id) {
     SEGGER_SYSVIEW_RecordU32(Preempt, SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id));
}

void EvrRtxThreadCreated(osThreadId_t thread_id) {

    SendThreadInfo(thread_id);
}

void EvrRtxThreadExit(osThreadId_t thread_id) {
    SEGGER_SYSVIEW_OnTaskTerminate((uint32_t)thread_id);
}


void EvrRtxThreadUnblocked(osThreadId_t thread_id, uint32_t retval) {
    SEGGER_SYSVIEW_OnTaskStartReady((uint32_t)thread_id);
}

void EvrRtxThreadBlocked(osThreadId_t thread_id, uint32_t timeout) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32x2(ThreadBlocked, TaskID, timeout);
}

void EvrRtxKernelSuspended(uint32_t ticks) {
    SEGGER_SYSVIEW_RecordU32(KernelSuspend, ticks);
    SEGGER_SYSVIEW_OnIdle();
}
void EvrRtxKernelResume(uint32_t sleep_ticks) {
    SEGGER_SYSVIEW_RecordU32(KernelResume, sleep_ticks);
}

void EvrRtxThreadSwitched(osThreadId_t thread_id) {
    SEGGER_SYSVIEW_OnTaskStartExec((uint32_t)thread_id);
}

void EvrRtxThreadError (osThreadId_t thread_id, int32_t status) {
    U32 TaskID;

    if (thread_id) {
        TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    } else {
        TaskID = 0;
    }

    SEGGER_SYSVIEW_RecordU32x2(ThreadError, TaskID, status);
}

void EvrRtxThreadSetPriority (osThreadId_t thread_id, osPriority_t priority) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32x2(ThreadPrioritySet, TaskID, (U32)priority);

    ((osRtxThread_t *)thread_id)->priority = priority;
    SendThreadInfo(thread_id);
}

void EvrRtxThreadYield (void) {
    SEGGER_SYSVIEW_RecordVoid(ThreadYield);
}

void EvrRtxThreadSuspend (osThreadId_t thread_id) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32(ThreadSuspend, TaskID);
}

void EvrRtxThreadResume (osThreadId_t thread_id) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32(ThreadResume, TaskID);
}

void EvrRtxMessageQueueNew (uint32_t msg_count, uint32_t msg_size,
                                            const osMessageQueueAttr_t *attr) {

    SEGGER_SYSVIEW_RecordU32x3(MessageQueueNew, msg_count, msg_size, (U32)attr);
}

void EvrRtxMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr,
                                    uint8_t *msg_prio, uint32_t timeout) {

    SEGGER_SYSVIEW_RecordU32x4(MessageQueueGet, (U32)mq_id, (U32)msg_ptr,
                                            (U32)msg_prio, timeout);
}

void EvrRtxMessageQueuePut (osMessageQueueId_t mq_id, const void *msg_ptr,
                                        uint8_t msg_prio, uint32_t timeout) {

    SEGGER_SYSVIEW_RecordU32x4(MessageQueuePut, (U32)mq_id, (U32)msg_ptr,
                                            (U32)msg_prio, timeout);
}

void EvrRtxMessageQueueError (osMessageQueueId_t mq_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(MessageQueueError, (U32)mq_id, status);
}


/*timer event*/
void EvrRtxTimerError (osTimerId_t timer_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(TimerError, (U32)timer_id, status);
}

void EvrRtxTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument,
                                                const osTimerAttr_t *attr) {
    SEGGER_SYSVIEW_RecordU32x4(TimerNew, (U32)func, (U32)type,
                                        (U32)argument, (U32)attr);
}

void EvrRtxTimerStart (osTimerId_t timer_id, uint32_t ticks) {
    SEGGER_SYSVIEW_RecordU32x2(TimerStart, (U32)timer_id, ticks);
}

void EvrRtxTimerStop (osTimerId_t timer_id) {
    SEGGER_SYSVIEW_RecordU32(TimerStop, (U32)timer_id);
}

void EvrRtxTimerDelete (osTimerId_t timer_id) {
    SEGGER_SYSVIEW_RecordU32(TimerDelete, (U32)timer_id);
}

void EvrRtxTimerCallback (osTimerFunc_t func, void *argument) {
    SEGGER_SYSVIEW_RecordU32x2(TimerCallback, (U32)func, (U32)argument);
}

/*thread flags event*/
void EvrRtxThreadFlagsError (osThreadId_t thread_id, int32_t status) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32x2(ThreadFlagsError, TaskID, status);
}

void EvrRtxThreadFlagsSet (osThreadId_t thread_id, uint32_t flags) {
    U32 TaskID;

    TaskID = SEGGER_SYSVIEW_ShrinkId((uint32_t)thread_id);
    SEGGER_SYSVIEW_RecordU32x2(ThreadFlagsSet, TaskID, flags);
}

void EvrRtxThreadFlagsClear (uint32_t flags) {
    SEGGER_SYSVIEW_RecordU32(ThreadFlagsClear, flags);
}

void EvrRtxThreadFlagsGet (uint32_t thread_flags) {
    SEGGER_SYSVIEW_RecordU32(ThreadFlagsGet, thread_flags);
}

void EvrRtxThreadFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x3(ThreadFlagsWait, flags, options, timeout);
}


/*thread flags event*/
void EvrRtxMutexError (osMutexId_t mutex_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(MutexError, (U32)mutex_id, status);
}

void EvrRtxMutexNew (const osMutexAttr_t *attr) {
    if (attr)
        SEGGER_SYSVIEW_RecordString(MutexNew, attr->name);
}

void EvrRtxMutexAcquire (osMutexId_t mutex_id, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x2(MutexAcquire, (U32)mutex_id, timeout);
}

void EvrRtxMutexRelease (osMutexId_t mutex_id) {
    SEGGER_SYSVIEW_RecordU32(MutexRelease, (U32)mutex_id);
}

void EvrRtxMutexDelete (osMutexId_t mutex_id) {
    SEGGER_SYSVIEW_RecordU32(MutexDelete, (U32)mutex_id);
}

/*semaphore events*/
void EvrRtxSemaphoreError (osSemaphoreId_t semaphore_id, int32_t status) {
    SEGGER_SYSVIEW_RecordU32x2(SemaphoreError, (U32)semaphore_id, status);
}

void EvrRtxSemaphoreNew (uint32_t max_count, uint32_t initial_count,
                                                const osSemaphoreAttr_t *attr) {
    SEGGER_SYSVIEW_RecordU32x3(SemaphoreNew, max_count, initial_count, (U32)attr);
}

void EvrRtxSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout) {
    SEGGER_SYSVIEW_RecordU32x2(SemaphoreAcquire, (U32)semaphore_id, timeout);
}

void EvrRtxSemaphoreRelease (osSemaphoreId_t semaphore_id) {
    SEGGER_SYSVIEW_RecordU32(SemaphoreRelease, (U32)semaphore_id);
}

void EvrRtxSemaphoreDelete (osSemaphoreId_t semaphore_id) {
    SEGGER_SYSVIEW_RecordU32(SemaphoreDelete, (U32)semaphore_id);
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/
// Callbacks provided to SYSTEMVIEW by Rtx5
const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI = {
    &_cbGetTime,
    &_cbSendTaskList,
};

#endif
