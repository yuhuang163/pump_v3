/*
 * Copyright (c) 2013-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS LOS
 * Title:       LOS Event Recorder
 *
 * -----------------------------------------------------------------------------
 */

#include <string.h>
#include "cmsis_compiler.h"
#include "los_sysview_evr.h"                    // LOS Event Recorder definitions
#include "hal_trace.h"

#ifdef  RTE_Compiler_EventRecorder

//lint -e923 -e9074 -e9078 [MISRA Note 13]

/// Event IDs for "LOS Memory Management"
#define EvtLosMemoryInit                    EventID(EventLevelOp,     EvtLosMemoryNo, 0x00U)
#define EvtLosMemoryAlloc                   EventID(EventLevelOp,     EvtLosMemoryNo, 0x01U)
#define EvtLosMemoryFree                    EventID(EventLevelOp,     EvtLosMemoryNo, 0x02U)
#define EvtLosMemoryBlockInit               EventID(EventLevelOp,     EvtLosMemoryNo, 0x03U)
#define EvtLosMemoryBlockAlloc              EventID(EventLevelOp,     EvtLosMemoryNo, 0x04U)
#define EvtLosMemoryBlockFree               EventID(EventLevelOp,     EvtLosMemoryNo, 0x05U)

/// Event IDs for "LOS Kernel"
#define EvtLosKernelError                   EventID(EventLevelError,  EvtLosKernelNo, 0x00U)
#define EvtLosKernelInitialize              EventID(EventLevelAPI,    EvtLosKernelNo, 0x01U)
#define EvtLosKernelInitialized             EventID(EventLevelOp,     EvtLosKernelNo, 0x02U)
#define EvtLosKernelGetInfo                 EventID(EventLevelAPI,    EvtLosKernelNo, 0x03U)
#define EvtLosKernelInfoRetrieved           EventID(EventLevelOp,     EvtLosKernelNo, 0x04U)
#define EvtLosKernelInfoRetrieved_Detail    EventID(EventLevelDetail, EvtLosKernelNo, 0x05U)
#define EvtLosKernelGetState                EventID(EventLevelAPI,    EvtLosKernelNo, 0x06U)
#define EvtLosKernelStart                   EventID(EventLevelAPI,    EvtLosKernelNo, 0x07U)
#define EvtLosKernelStarted                 EventID(EventLevelOp,     EvtLosKernelNo, 0x08U)
#define EvtLosKernelLock                    EventID(EventLevelAPI,    EvtLosKernelNo, 0x09U)
#define EvtLosKernelLocked                  EventID(EventLevelOp,     EvtLosKernelNo, 0x0AU)
#define EvtLosKernelUnlock                  EventID(EventLevelAPI,    EvtLosKernelNo, 0x0BU)
#define EvtLosKernelUnlocked                EventID(EventLevelOp,     EvtLosKernelNo, 0x0CU)
#define EvtLosKernelRestoreLock             EventID(EventLevelAPI,    EvtLosKernelNo, 0x0DU)
#define EvtLosKernelLockRestored            EventID(EventLevelOp,     EvtLosKernelNo, 0x0EU)
#define EvtLosKernelSuspend                 EventID(EventLevelAPI,    EvtLosKernelNo, 0x0FU)
#define EvtLosKernelSuspended               EventID(EventLevelOp,     EvtLosKernelNo, 0x10U)
#define EvtLosKernelResume                  EventID(EventLevelAPI,    EvtLosKernelNo, 0x11U)
#define EvtLosKernelResumed                 EventID(EventLevelOp,     EvtLosKernelNo, 0x12U)
#define EvtLosKernelGetTickCount            EventID(EventLevelAPI,    EvtLosKernelNo, 0x13U)
#define EvtLosKernelGetTickFreq             EventID(EventLevelAPI,    EvtLosKernelNo, 0x14U)
#define EvtLosKernelGetSysTimerCount        EventID(EventLevelAPI,    EvtLosKernelNo, 0x15U)
#define EvtLosKernelGetSysTimerFreq         EventID(EventLevelAPI,    EvtLosKernelNo, 0x16U)

/// Event IDs for "LOS Thread"
#define EvtLosThreadError                   EventID(EventLevelError,  EvtLosThreadNo, 0x00U)
#define EvtLosThreadNew                     EventID(EventLevelAPI,    EvtLosThreadNo, 0x01U)
#define EvtLosThreadCreated_Addr            EventID(EventLevelOp,     EvtLosThreadNo, 0x03U)
#define EvtLosThreadCreated_Name            EventID(EventLevelOp,     EvtLosThreadNo, 0x2CU)
#define EvtLosThreadGetName                 EventID(EventLevelAPI,    EvtLosThreadNo, 0x04U)
#define EvtLosThreadGetId                   EventID(EventLevelAPI,    EvtLosThreadNo, 0x06U)
#define EvtLosThreadGetState                EventID(EventLevelAPI,    EvtLosThreadNo, 0x07U)
#define EvtLosThreadGetStackSize            EventID(EventLevelAPI,    EvtLosThreadNo, 0x08U)
#define EvtLosThreadGetStackSpace           EventID(EventLevelAPI,    EvtLosThreadNo, 0x09U)
#define EvtLosThreadSetPriority             EventID(EventLevelAPI,    EvtLosThreadNo, 0x0AU)
#define EvtLosThreadPriorityUpdated         EventID(EventLevelOp,     EvtLosThreadNo, 0x2DU)
#define EvtLosThreadGetPriority             EventID(EventLevelAPI,    EvtLosThreadNo, 0x0BU)
#define EvtLosThreadYield                   EventID(EventLevelAPI,    EvtLosThreadNo, 0x0CU)
#define EvtLosThreadSuspend                 EventID(EventLevelAPI,    EvtLosThreadNo, 0x0DU)
#define EvtLosThreadSuspended               EventID(EventLevelOp,     EvtLosThreadNo, 0x0EU)
#define EvtLosThreadResume                  EventID(EventLevelAPI,    EvtLosThreadNo, 0x0FU)
#define EvtLosThreadResumed                 EventID(EventLevelOp,     EvtLosThreadNo, 0x10U)
#define EvtLosThreadDetach                  EventID(EventLevelAPI,    EvtLosThreadNo, 0x11U)
#define EvtLosThreadDetached                EventID(EventLevelOp,     EvtLosThreadNo, 0x12U)
#define EvtLosThreadJoin                    EventID(EventLevelAPI,    EvtLosThreadNo, 0x13U)
#define EvtLosThreadJoinPending             EventID(EventLevelOp,     EvtLosThreadNo, 0x14U)
#define EvtLosThreadJoined                  EventID(EventLevelOp,     EvtLosThreadNo, 0x15U)
#define EvtLosThreadBlocked                 EventID(EventLevelDetail, EvtLosThreadNo, 0x16U)
#define EvtLosThreadUnblocked               EventID(EventLevelDetail, EvtLosThreadNo, 0x17U)
#define EvtLosThreadPreempted               EventID(EventLevelDetail, EvtLosThreadNo, 0x18U)
#define EvtLosThreadSwitched                EventID(EventLevelOp,     EvtLosThreadNo, 0x19U)
#define EvtLosThreadExit                    EventID(EventLevelAPI,    EvtLosThreadNo, 0x1AU)
#define EvtLosThreadTerminate               EventID(EventLevelAPI,    EvtLosThreadNo, 0x1BU)
#define EvtLosThreadDestroyed               EventID(EventLevelOp,     EvtLosThreadNo, 0x1CU)
#define EvtLosThreadGetCount                EventID(EventLevelAPI,    EvtLosThreadNo, 0x1DU)
#define EvtLosThreadEnumerate               EventID(EventLevelAPI,    EvtLosThreadNo, 0x1EU)

/// Event IDs for "LOS Thread Flags"
#define EvtLosThreadFlagsError              EventID(EventLevelError,  EvtLosThreadFlagsNo, 0x00U)
#define EvtLosThreadFlagsSet                EventID(EventLevelAPI,    EvtLosThreadFlagsNo, 0x01U)
#define EvtLosThreadFlagsSetDone            EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x02U)
#define EvtLosThreadFlagsClear              EventID(EventLevelAPI,    EvtLosThreadFlagsNo, 0x03U)
#define EvtLosThreadFlagsClearDone          EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x04U)
#define EvtLosThreadFlagsGet                EventID(EventLevelAPI,    EvtLosThreadFlagsNo, 0x05U)
#define EvtLosThreadFlagsWait               EventID(EventLevelAPI,    EvtLosThreadFlagsNo, 0x06U)
#define EvtLosThreadFlagsWaitPending        EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x07U)
#define EvtLosThreadFlagsWaitTimeout        EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x08U)
#define EvtLosThreadFlagsWaitCompleted      EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x09U)
#define EvtLosThreadFlagsWaitNotCompleted   EventID(EventLevelOp,     EvtLosThreadFlagsNo, 0x0AU)

/// Event IDs for "LOS Generic Wait"
#define EvtLosDelayError                    EventID(EventLevelError,  EvtLosWaitNo, 0x00U)
#define EvtLosDelay                         EventID(EventLevelAPI,    EvtLosWaitNo, 0x01U)
#define EvtLosDelayUntil                    EventID(EventLevelAPI,    EvtLosWaitNo, 0x02U)
#define EvtLosDelayStarted                  EventID(EventLevelOp,     EvtLosWaitNo, 0x03U)
#define EvtLosDelayUntilStarted             EventID(EventLevelOp,     EvtLosWaitNo, 0x04U)
#define EvtLosDelayCompleted                EventID(EventLevelOp,     EvtLosWaitNo, 0x05U)

/// Event IDs for "LOS Timer"
#define EvtLosTimerError                    EventID(EventLevelError,  EvtLosTimerNo, 0x00U)
#define EvtLosTimerCallback                 EventID(EventLevelOp,     EvtLosTimerNo, 0x01U)
#define EvtLosTimerNew                      EventID(EventLevelAPI,    EvtLosTimerNo, 0x02U)
#define EvtLosTimerCreated                  EventID(EventLevelOp,     EvtLosTimerNo, 0x04U)
#define EvtLosTimerGetName                  EventID(EventLevelAPI,    EvtLosTimerNo, 0x05U)
#define EvtLosTimerStart                    EventID(EventLevelAPI,    EvtLosTimerNo, 0x07U)
#define EvtLosTimerStarted                  EventID(EventLevelOp,     EvtLosTimerNo, 0x08U)
#define EvtLosTimerStop                     EventID(EventLevelAPI,    EvtLosTimerNo, 0x09U)
#define EvtLosTimerStopped                  EventID(EventLevelOp,     EvtLosTimerNo, 0x0AU)
#define EvtLosTimerIsRunning                EventID(EventLevelAPI,    EvtLosTimerNo, 0x0BU)
#define EvtLosTimerDelete                   EventID(EventLevelAPI,    EvtLosTimerNo, 0x0CU)
#define EvtLosTimerDestroyed                EventID(EventLevelOp,     EvtLosTimerNo, 0x0DU)

/// Event IDs for "LOS Event Flags"
#define EvtLosEventFlagsError               EventID(EventLevelError,  EvtLosEventFlagsNo, 0x00U)
#define EvtLosEventFlagsNew                 EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x01U)
#define EvtLosEventFlagsCreated             EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x03U)
#define EvtLosEventFlagsGetName             EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x04U)
#define EvtLosEventFlagsSet                 EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x06U)
#define EvtLosEventFlagsSetDone             EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x07U)
#define EvtLosEventFlagsClear               EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x08U)
#define EvtLosEventFlagsClearDone           EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x09U)
#define EvtLosEventFlagsGet                 EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x0AU)
#define EvtLosEventFlagsWait                EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x0BU)
#define EvtLosEventFlagsWaitPending         EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x0CU)
#define EvtLosEventFlagsWaitTimeout         EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x0DU)
#define EvtLosEventFlagsWaitCompleted       EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x0EU)
#define EvtLosEventFlagsWaitNotCompleted    EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x0FU)
#define EvtLosEventFlagsDelete              EventID(EventLevelAPI,    EvtLosEventFlagsNo, 0x10U)
#define EvtLosEventFlagsDestroyed           EventID(EventLevelOp,     EvtLosEventFlagsNo, 0x11U)

/// Event IDs for "LOS Mutex"
#define EvtLosMutexError                    EventID(EventLevelError,  EvtLosMutexNo, 0x00U)
#define EvtLosMutexNew                      EventID(EventLevelAPI,    EvtLosMutexNo, 0x01U)
#define EvtLosMutexCreated                  EventID(EventLevelOp,     EvtLosMutexNo, 0x03U)
#define EvtLosMutexGetName                  EventID(EventLevelAPI,    EvtLosMutexNo, 0x04U)
#define EvtLosMutexAcquire                  EventID(EventLevelAPI,    EvtLosMutexNo, 0x06U)
#define EvtLosMutexAcquirePending           EventID(EventLevelOp,     EvtLosMutexNo, 0x07U)
#define EvtLosMutexAcquireTimeout           EventID(EventLevelOp,     EvtLosMutexNo, 0x08U)
#define EvtLosMutexAcquired                 EventID(EventLevelOp,     EvtLosMutexNo, 0x09U)
#define EvtLosMutexNotAcquired              EventID(EventLevelOp,     EvtLosMutexNo, 0x0AU)
#define EvtLosMutexRelease                  EventID(EventLevelAPI,    EvtLosMutexNo, 0x0BU)
#define EvtLosMutexReleased                 EventID(EventLevelOp,     EvtLosMutexNo, 0x0CU)
#define EvtLosMutexGetOwner                 EventID(EventLevelAPI,    EvtLosMutexNo, 0x0DU)
#define EvtLosMutexDelete                   EventID(EventLevelAPI,    EvtLosMutexNo, 0x0EU)
#define EvtLosMutexDestroyed                EventID(EventLevelOp,     EvtLosMutexNo, 0x0FU)

/// Event IDs for "LOS Semaphore"
#define EvtLosSemaphoreError                EventID(EventLevelError,  EvtLosSemaphoreNo, 0x00U)
#define EvtLosSemaphoreNew                  EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x01U)
#define EvtLosSemaphoreCreated              EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x03U)
#define EvtLosSemaphoreGetName              EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x04U)
#define EvtLosSemaphoreAcquire              EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x06U)
#define EvtLosSemaphoreAcquirePending       EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x07U)
#define EvtLosSemaphoreAcquireTimeout       EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x08U)
#define EvtLosSemaphoreAcquired             EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x09U)
#define EvtLosSemaphoreNotAcquired          EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x0AU)
#define EvtLosSemaphoreRelease              EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x0BU)
#define EvtLosSemaphoreReleased             EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x0CU)
#define EvtLosSemaphoreGetCount             EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x0DU)
#define EvtLosSemaphoreDelete               EventID(EventLevelAPI,    EvtLosSemaphoreNo, 0x0EU)
#define EvtLosSemaphoreDestroyed            EventID(EventLevelOp,     EvtLosSemaphoreNo, 0x0FU)

/// Event IDs for "LOS Memory Pool"
#define EvtLosMemoryPoolError               EventID(EventLevelError,  EvtLosMemoryPoolNo, 0x00U)
#define EvtLosMemoryPoolNew                 EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x01U)
#define EvtLosMemoryPoolCreated             EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x03U)
#define EvtLosMemoryPoolGetName             EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x04U)
#define EvtLosMemoryPoolAlloc               EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x06U)
#define EvtLosMemoryPoolAllocPending        EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x07U)
#define EvtLosMemoryPoolAllocTimeout        EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x08U)
#define EvtLosMemoryPoolAllocated           EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x09U)
#define EvtLosMemoryPoolAllocFailed         EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x0AU)
#define EvtLosMemoryPoolFree                EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x0BU)
#define EvtLosMemoryPoolDeallocated         EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x0CU)
#define EvtLosMemoryPoolFreeFailed          EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x0DU)
#define EvtLosMemoryPoolGetCapacity         EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x0EU)
#define EvtLosMemoryPoolGetBlockSize        EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x0FU)
#define EvtLosMemoryPoolGetCount            EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x10U)
#define EvtLosMemoryPoolGetSpace            EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x11U)
#define EvtLosMemoryPoolDelete              EventID(EventLevelAPI,    EvtLosMemoryPoolNo, 0x12U)
#define EvtLosMemoryPoolDestroyed           EventID(EventLevelOp,     EvtLosMemoryPoolNo, 0x13U)

/// Event IDs for "LOS Message Queue"
#define EvtLosMessageQueueError             EventID(EventLevelError,  EvtLosMessageQueueNo, 0x00U)
#define EvtLosMessageQueueNew               EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x01U)
#define EvtLosMessageQueueCreated           EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x03U)
#define EvtLosMessageQueueGetName           EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x04U)
#define EvtLosMessageQueuePut               EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x06U)
#define EvtLosMessageQueuePutPending        EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x07U)
#define EvtLosMessageQueuePutTimeout        EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x08U)
#define EvtLosMessageQueueInsertPending     EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x09U)
#define EvtLosMessageQueueInserted          EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x0AU)
#define EvtLosMessageQueueNotInserted       EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x0BU)
#define EvtLosMessageQueueGet               EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x0CU)
#define EvtLosMessageQueueGetPending        EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x0DU)
#define EvtLosMessageQueueGetTimeout        EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x0EU)
#define EvtLosMessageQueueRetrieved         EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x0FU)
#define EvtLosMessageQueueNotRetrieved      EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x10U)
#define EvtLosMessageQueueGetCapacity       EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x11U)
#define EvtLosMessageQueueGetMsgSize        EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x12U)
#define EvtLosMessageQueueGetCount          EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x13U)
#define EvtLosMessageQueueGetSpace          EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x14U)
#define EvtLosMessageQueueReset             EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x15U)
#define EvtLosMessageQueueResetDone         EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x16U)
#define EvtLosMessageQueueDelete            EventID(EventLevelAPI,    EvtLosMessageQueueNo, 0x17U)
#define EvtLosMessageQueueDestroyed         EventID(EventLevelOp,     EvtLosMessageQueueNo, 0x18U)

#endif  // RTE_Compiler_EventRecorder

//lint -esym(522, EvrLos*) "Functions 'EvrLos*' can be overridden (do not lack side-effects)"


//  ==== Memory Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_INIT_DISABLE))
__WEAK void EvrLosMemoryInit (void *mem, uint32_t size, uint32_t result) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryInit, (uint32_t)mem, size, result, 0U);
#else
  (void)mem;
  (void)size;
  (void)result;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_ALLOC_DISABLE))
__WEAK void EvrLosMemoryAlloc (void *mem, uint32_t size, uint32_t type, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryAlloc, (uint32_t)mem, size, type, (uint32_t)block);
#else
  (void)mem;
  (void)size;
  (void)type;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_FREE_DISABLE))
__WEAK void EvrLosMemoryFree (void *mem, void *block, uint32_t result) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryFree, (uint32_t)mem, (uint32_t)block, result, 0U);
#else
  (void)mem;
  (void)block;
  (void)result;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_INIT_DISABLE))
__WEAK void EvrLosMemoryBlockInit (void *mp_info, uint32_t block_count, uint32_t block_size, void *block_mem) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryBlockInit, (uint32_t)mp_info, block_count, block_size, (uint32_t)block_mem);
#else
  (void)mp_info;
  (void)block_count;
  (void)block_size;
  (void)block_mem;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_ALLOC_DISABLE))
__WEAK void EvrLosMemoryBlockAlloc (void *mp_info, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryBlockAlloc, (uint32_t)mp_info, (uint32_t)block);
#else
  (void)mp_info;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_FREE_DISABLE))
__WEAK void EvrLosMemoryBlockFree (void *mp_info, void *block, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryBlockFree, (uint32_t)mp_info, (uint32_t)block, (uint32_t)status, 0U);
#else
  (void)mp_info;
  (void)block;
  (void)status;
#endif
}
#endif


//  ==== Kernel Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_ERROR_DISABLE))
__WEAK void EvrLosKernelError (int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelError, (uint32_t)status, 0U);
#else
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INITIALIZE_DISABLE))
__WEAK void EvrLosKernelInitialize (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelInitialize, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INITIALIZED_DISABLE))
__WEAK void EvrLosKernelInitialized (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelInitialized, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_INFO_DISABLE))
__WEAK void EvrLosKernelGetInfo (osVersion_t *version, char *id_buf, uint32_t id_size) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosKernelGetInfo, (uint32_t)version, (uint32_t)id_buf, id_size, 0U);
#else
  (void)version;
  (void)id_buf;
  (void)id_size;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INFO_RETRIEVED_DISABLE))
__WEAK void EvrLosKernelInfoRetrieved (const osVersion_t *version, const char *id_buf, uint32_t id_size) {
#if defined(RTE_Compiler_EventRecorder)
  if (version != NULL) {
    (void)EventRecord2(EvtLosKernelInfoRetrieved, version->api, version->kernel);
  }
  if (id_buf != NULL) {
    (void)EventRecordData(EvtLosKernelInfoRetrieved_Detail, id_buf, id_size);
  }
#else
  (void)version;
  (void)id_buf;
  (void)id_size;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_STATE_DISABLE))
__WEAK void EvrLosKernelGetState (osKernelState_t state) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelGetState, (uint32_t)state, 0U);
#else
  (void)state;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_START_DISABLE))
__WEAK void EvrLosKernelStart (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelStart, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_STARTED_DISABLE))
__WEAK void EvrLosKernelStarted (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelStarted, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCK_DISABLE))
__WEAK void EvrLosKernelLock (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelLock, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCKED_DISABLE))
__WEAK void EvrLosKernelLocked (int32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelLocked, (uint32_t)lock, 0U);
#else
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_UNLOCK_DISABLE))
__WEAK void EvrLosKernelUnlock (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelUnlock, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_UNLOCKED_DISABLE))
__WEAK void EvrLosKernelUnlocked (int32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelUnlocked, (uint32_t)lock, 0U);
#else
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESTORE_LOCK_DISABLE))
__WEAK void EvrLosKernelRestoreLock (int32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelRestoreLock, (uint32_t)lock, 0U);
#else
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCK_RESTORED_DISABLE))
__WEAK void EvrLosKernelLockRestored (int32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelLockRestored, (uint32_t)lock, 0U);
#else
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_SUSPEND_DISABLE))
__WEAK void EvrLosKernelSuspend (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelSuspend, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_SUSPENDED_DISABLE))
__WEAK void EvrLosKernelSuspended (uint32_t sleep_ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelSuspended, sleep_ticks, 0U);
#else
  (void)sleep_ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESUME_DISABLE))
__WEAK void EvrLosKernelResume (uint32_t sleep_ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelResume, sleep_ticks, 0U);
#else
  (void)sleep_ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESUMED_DISABLE))
__WEAK void EvrLosKernelResumed (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelResumed, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_TICK_COUNT_DISABLE))
__WEAK void EvrLosKernelGetTickCount (uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelGetTickCount, count, 0U);
#else
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_TICK_FREQ_DISABLE))
__WEAK void EvrLosKernelGetTickFreq (uint32_t freq) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelGetTickFreq, freq, 0U);
#else
  (void)freq;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_SYS_TIMER_COUNT_DISABLE))
__WEAK void EvrLosKernelGetSysTimerCount (uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelGetSysTimerCount, count, 0U);
#else
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_SYS_TIMER_FREQ_DISABLE))
__WEAK void EvrLosKernelGetSysTimerFreq (uint32_t freq) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosKernelGetSysTimerFreq, freq, 0U);
#else
  (void)freq;
#endif
}
#endif

//  ==== Thread Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_ERROR_DISABLE))
__WEAK void EvrLosTaskError (uint32_t task_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadError, (uint32_t)task_id, (uint32_t)status);
#else
  LosTaskCB *task = OS_TCB_FROM_TID(task_id);
  TRACE(2,"EvrLosTaskError, task id:%s, status:%d", task->taskName, status);
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_NEW_DISABLE))
__WEAK void EvrLosTaskNew (TSK_ENTRY_FUNC func, void *argument, uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosThreadNew, (uint32_t)func, (uint32_t)argument, (uint32_t)task_id, 0U);
#else
  (void)func;
  (void)argument;
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_CREATED_DISABLE))
__WEAK void EvrLosTaskCreated (LosTaskCB *taskCB) {
#if defined(RTE_Compiler_EventRecorder)
  if (taskCB->taskName != NULL) {
    (void)EventRecord2(EvtLosThreadCreated_Name, (uint32_t)task_id, (uint32_t)(taskCB->taskName));
  } else {
    (void)EventRecord2(EvtLosThreadCreated_Addr, (uint32_t)task_id, (uint32_t)(taskCB->taskEntry));
  }
#else
  (void)taskCB;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_NAME_DISABLE))
__WEAK void EvrLosTaskGetName (uint32_t task_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetName, (uint32_t)task_id, (uint32_t)name);
#else
  (void)task_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_ID_DISABLE))
__WEAK void EvrLosTaskGetId (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetId, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_STATE_DISABLE))
__WEAK void EvrLosTaskGetState (uint32_t task_id, uint32_t state) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetState, (uint32_t)task_id, (uint32_t)state);
#else
  (void)task_id;
  (void)state;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_STACK_SIZE_DISABLE))
__WEAK void EvrLosTaskGetStackSize (uint32_t task_id, uint32_t stack_size) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetStackSize, (uint32_t)task_id, stack_size);
#else
  (void)task_id;
  (void)stack_size;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_STACK_SPACE_DISABLE))
__WEAK void EvrLosTaskGetStackSpace (uint32_t task_id, uint32_t stack_space) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetStackSpace, (uint32_t)task_id, stack_space);
#else
  (void)task_id;
  (void)stack_space;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_SET_PRIORITY_DISABLE))
__WEAK void EvrLosTaskSetPriority (uint32_t task_id, osPriority_t priority) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadSetPriority, (uint32_t)task_id, (uint32_t)priority);
#else
  (void)task_id;
  (void)priority;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_PRIORITY_UPDATED_DISABLE))
__WEAK void EvrLosTaskPriorityUpdated (uint32_t task_id, osPriority_t priority) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadPriorityUpdated, (uint32_t)task_id, (uint32_t)priority);
#else
  (void)task_id;
  (void)priority;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_PRIORITY_DISABLE))
__WEAK void EvrLosTaskGetPriority (uint32_t task_id, osPriority_t priority) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetPriority, (uint32_t)task_id, (uint32_t)priority);
#else
  (void)task_id;
  (void)priority;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_YIELD_DISABLE))
__WEAK void EvrLosTaskYield (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadYield, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_SUSPEND_DISABLE))
__WEAK void EvrLosTaskSuspend (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadSuspend, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_SUSPENDED_DISABLE))
__WEAK void EvrLosTaskSuspended (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadSuspended, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_RESUME_DISABLE))
__WEAK void EvrLosTaskResume (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadResume, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_RESUMED_DISABLE))
__WEAK void EvrLosTaskResumed (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadResumed, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_DETACH_DISABLE))
__WEAK void EvrLosTaskDetach (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadDetach, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_DETACHED_DISABLE))
__WEAK void EvrLosTaskDetached (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadDetached, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_JOIN_DISABLE))
__WEAK void EvrLosTaskJoin (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadJoin, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_JOIN_PENDING_DISABLE))
__WEAK void EvrLosTaskJoinPending (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadJoinPending, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_JOINED_DISABLE))
__WEAK void EvrLosTaskJoined (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadJoined, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_BLOCKED_DISABLE))
__WEAK void EvrLosTaskBlocked (uint32_t task_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadBlocked, (uint32_t)task_id, timeout);
#else
  (void)task_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_UNBLOCKED_DISABLE))
__WEAK void EvrLosTaskUnblocked (uint32_t task_id, uint32_t ret_val) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadUnblocked, (uint32_t)task_id, ret_val);
#else
  (void)task_id;
  (void)ret_val;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_PREEMPTED_DISABLE))
__WEAK void EvrLosTaskPreempted (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadPreempted, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_SWITCHED_DISABLE))
__WEAK void EvrLosTaskSwitched (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadSwitched, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_EXIT_DISABLE))
__WEAK void EvrLosTaskExit (void) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadExit, 0U, 0U);
#else
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_TERMINATE_DISABLE))
__WEAK void EvrLosTaskTerminate (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadTerminate, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_DESTROYED_DISABLE))
__WEAK void EvrLosTaskDestroyed (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadDestroyed, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_GET_COUNT_DISABLE))
__WEAK void EvrLosTaskGetCount (uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadGetCount, count, 0U);
#else
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_THREAD_ENUMERATE_DISABLE))
__WEAK void EvrLosTaskEnumerate (uint32_t *TASK_array, uint32_t array_items, uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosThreadEnumerate, (uint32_t)TASK_array, array_items, count, 0U);
#else
  (void)TASK_array;
  (void)array_items;
  (void)count;
#endif
}
#endif


//  ==== Thread Flags Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_ERROR_DISABLE))
__WEAK void EvrLosTaskFlagsError (uint32_t task_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsError, (uint32_t)task_id, (uint32_t)status);
#else
  (void)task_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_SET_DISABLE))
__WEAK void EvrLosTaskFlagsSet (uint32_t task_id, uint32_t flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsSet, (uint32_t)task_id, flags);
#else
  (void)task_id;
  (void)flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_SET_DONE_DISABLE))
__WEAK void EvrLosTaskFlagsSetDone (uint32_t task_id, uint32_t task_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsSetDone, (uint32_t)task_id, task_flags);
#else
  (void)task_id;
  (void)task_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_CLEAR_DISABLE))
__WEAK void EvrLosTaskFlagsClear (uint32_t flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsClear, flags, 0U);
#else
  (void)flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_CLEAR_DONE_DISABLE))
__WEAK void EvrLosTaskFlagsClearDone (uint32_t task_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsClearDone, task_flags, 0U);
#else
  (void)task_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_GET_DISABLE))
__WEAK void EvrLosTaskFlagsGet (uint32_t task_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsGet, task_flags, 0U);
#else
  (void)task_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_WAIT_DISABLE))
__WEAK void EvrLosTaskFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosThreadFlagsWait, flags, options, timeout, 0U);
#else
  (void)flags;
  (void)options;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_WAIT_PENDING_DISABLE))
__WEAK void EvrLosTaskFlagsWaitPending (uint32_t flags, uint32_t options, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosThreadFlagsWaitPending, flags, options, timeout, 0U);
#else
  (void)flags;
  (void)options;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_WAIT_TIMEOUT_DISABLE))
__WEAK void EvrLosTaskFlagsWaitTimeout (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsWaitTimeout, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_WAIT_COMPLETED_DISABLE))
__WEAK void EvrLosTaskFlagsWaitCompleted (uint32_t flags, uint32_t options, uint32_t thread_flags, uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosThreadFlagsWaitCompleted, flags, options, thread_flags, (uint32_t)task_id);
#else
  (void)flags;
  (void)options;
  (void)thread_flags;
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_THREAD_FLAGS_WAIT_NOT_COMPLETED_DISABLE))
__WEAK void EvrLosTaskFlagsWaitNotCompleted (uint32_t flags, uint32_t options) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosThreadFlagsWaitNotCompleted, flags, options);
#else
  (void)flags;
  (void)options;
#endif
}
#endif


//  ==== Generic Wait Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_ERROR_DISABLE))
__WEAK void EvrLosDelayError (int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelayError, (uint32_t)status, 0U);
#else
  (void)status;
#endif
}
#endif


#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_DISABLE))
__WEAK void EvrLosDelay (uint32_t ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelay, ticks, 0U);
#else
  (void)ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_UNTIL_DISABLE))
__WEAK void EvrLosDelayUntil (uint32_t ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelayUntil, ticks, 0U);
#else
  (void)ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_STARTED_DISABLE))
__WEAK void EvrLosDelayStarted (uint32_t ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelayStarted, ticks, 0U);
#else
  (void)ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_UNTIL_STARTED_DISABLE))
__WEAK void EvrLosDelayUntilStarted (uint32_t ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelayUntilStarted, ticks, 0U);
#else
  (void)ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_COMPLETED_DISABLE))
__WEAK void EvrLosDelayCompleted (uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosDelayCompleted, (uint32_t)task_id, 0U);
#else
  (void)task_id;
#endif
}
#endif


//  ==== Timer Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_ERROR_DISABLE))
__WEAK void EvrLosTimerError (osTimerId_t timer_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerError, (uint32_t)timer_id, (uint32_t)status);
#else
  (void)timer_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_CALLBACK_DISABLE))
__WEAK void EvrLosTimerCallback (osTimerFunc_t func, void *argument) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerCallback, (uint32_t)func, (uint32_t)argument);
#else
  (void)func;
  (void)argument;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_NEW_DISABLE))
__WEAK void EvrLosTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosTimerNew, (uint32_t)func, (uint32_t)type, (uint32_t)argument, (uint32_t)attr);
#else
  (void)func;
  (void)type;
  (void)argument;
  (void)attr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_CREATED_DISABLE))
__WEAK void EvrLosTimerCreated (uint32_t timer_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerCreated, (uint32_t)timer_id, (uint32_t)name);
#else
  (void)timer_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_GET_NAME_DISABLE))
__WEAK void EvrLosTimerGetName (uint32_t timer_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerGetName, (uint32_t)timer_id, (uint32_t)name);
#else
  (void)timer_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_START_DISABLE))
__WEAK void EvrLosTimerStart (uint32_t timer_id, uint32_t ticks) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerStart, (uint32_t)timer_id, ticks);
#else
  (void)timer_id;
  (void)ticks;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STARTED_DISABLE))
__WEAK void EvrLosTimerStarted (uint32_t timer_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerStarted, (uint32_t)timer_id, 0U);
#else
  (void)timer_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STOP_DISABLE))
__WEAK void EvrLosTimerStop (uint32_t timer_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerStop, (uint32_t)timer_id, 0U);
#else
  (void)timer_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STOPPED_DISABLE))
__WEAK void EvrLosTimerStopped (uint32_t timer_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerStopped, (uint32_t)timer_id, 0U);
#else
  (void)timer_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_IS_RUNNING_DISABLE))
__WEAK void EvrLosTimerIsRunning (uint32_t timer_id, uint32_t running) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerIsRunning, (uint32_t)timer_id, running);
#else
  (void)timer_id;
  (void)running;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_DELETE_DISABLE))
__WEAK void EvrLosTimerDelete (uint32_t timer_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerDelete, (uint32_t)timer_id, 0U);
#else
  (void)timer_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_DESTROYED_DISABLE))
__WEAK void EvrLosTimerDestroyed (uint32_t timer_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosTimerDestroyed, (uint32_t)timer_id, 0U);
#else
  (void)timer_id;
#endif
}
#endif


//  ==== Event Flags Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_ERROR_DISABLE))
__WEAK void EvrLosEventFlagsError (uint32_t ef_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsError, (uint32_t)ef_id, (uint32_t)status);
#else
  (void)ef_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_NEW_DISABLE))
__WEAK void EvrLosEventFlagsNew (uint32_t ef_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsNew, (uint32_t)ef_id, 0U);
#else
  (void)ef_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CREATED_DISABLE))
__WEAK void EvrLosEventFlagsCreated (uint32_t ef_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsCreated, (uint32_t)ef_id, (uint32_t)name);
#else
  (void)ef_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_GET_NAME_DISABLE))
__WEAK void EvrLosEventFlagsGetName (uint32_t ef_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsGetName, (uint32_t)ef_id, (uint32_t)name);
#else
  (void)ef_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_SET_DISABLE))
__WEAK void EvrLosEventFlagsSet (uint32_t ef_id, uint32_t flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsSet, (uint32_t)ef_id, flags);
#else
  (void)ef_id;
  (void)flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_SET_DONE_DISABLE))
__WEAK void EvrLosEventFlagsSetDone (uint32_t ef_id, uint32_t event_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsSetDone, (uint32_t)ef_id, event_flags);
#else
  (void)ef_id;
  (void)event_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CLEAR_DISABLE))
__WEAK void EvrLosEventFlagsClear (uint32_t ef_id, uint32_t flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsClear, (uint32_t)ef_id, flags);
#else
  (void)ef_id;
  (void)flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CLEAR_DONE_DISABLE))
__WEAK void EvrLosEventFlagsClearDone (uint32_t ef_id, uint32_t event_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsClearDone, (uint32_t)ef_id, event_flags);
#else
  (void)ef_id;
  (void)event_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_GET_DISABLE))
__WEAK void EvrLosEventFlagsGet (uint32_t ef_id, uint32_t event_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsGet, (uint32_t)ef_id, event_flags);
#else
  (void)ef_id;
  (void)event_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_DISABLE))
__WEAK void EvrLosEventFlagsWait (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosEventFlagsWait, (uint32_t)ef_id, flags, options, timeout);
#else
  (void)ef_id;
  (void)flags;
  (void)options;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_PENDING_DISABLE))
__WEAK void EvrLosEventFlagsWaitPending (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosEventFlagsWaitPending, (uint32_t)ef_id, flags, options, timeout);
#else
  (void)ef_id;
  (void)flags;
  (void)options;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_TIMEOUT_DISABLE))
__WEAK void EvrLosEventFlagsWaitTimeout (uint32_t ef_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsWaitTimeout, (uint32_t)ef_id, 0U);
#else
  (void)ef_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_COMPLETED_DISABLE))
__WEAK void EvrLosEventFlagsWaitCompleted (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t event_flags) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosEventFlagsWaitCompleted, (uint32_t)ef_id, flags, options, event_flags);
#else
  (void)ef_id;
  (void)flags;
  (void)options;
  (void)event_flags;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_NOT_COMPLETED_DISABLE))
__WEAK void EvrLosEventFlagsWaitNotCompleted (uint32_t ef_id, uint32_t flags, uint32_t options) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosEventFlagsWaitNotCompleted, (uint32_t)ef_id, flags, options, 0U);
#else
  (void)ef_id;
  (void)flags;
  (void)options;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_DELETE_DISABLE))
__WEAK void EvrLosEventFlagsDelete (uint32_t ef_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsDelete, (uint32_t)ef_id, 0U);
#else
  (void)ef_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_DESTROYED_DISABLE))
__WEAK void EvrLosEventFlagsDestroyed (uint32_t ef_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosEventFlagsDestroyed, (uint32_t)ef_id, 0U);
#else
  (void)ef_id;
#endif
}
#endif


//  ==== Mutex Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ERROR_DISABLE))
__WEAK void EvrLosMutexError (uint32_t mutex_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexError, (uint32_t)mutex_id, (uint32_t)status);
#else
  (void)mutex_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_NEW_DISABLE))
__WEAK void EvrLosMutexNew (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexNew, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_CREATED_DISABLE))
__WEAK void EvrLosMutexCreated (uint32_t mutex_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexCreated, (uint32_t)mutex_id, (uint32_t)name);
#else
  (void)mutex_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_GET_NAME_DISABLE))
__WEAK void EvrLosMutexGetName (uint32_t mutex_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexGetName, (uint32_t)mutex_id, (uint32_t)name);
#else
  (void)mutex_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_DISABLE))
__WEAK void EvrLosMutexAcquire (uint32_t mutex_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexAcquire, (uint32_t)mutex_id, timeout);
#else
  (void)mutex_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_PENDING_DISABLE))
__WEAK void EvrLosMutexAcquirePending (uint32_t mutex_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexAcquirePending, (uint32_t)mutex_id, timeout);
#else
  (void)mutex_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_TIMEOUT_DISABLE))
__WEAK void EvrLosMutexAcquireTimeout (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexAcquireTimeout, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRED_DISABLE))
__WEAK void EvrLosMutexAcquired (uint32_t mutex_id, uint32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexAcquired, (uint32_t)mutex_id, lock);
#else
  (void)mutex_id;
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_NOT_ACQUIRED_DISABLE))
__WEAK void EvrLosMutexNotAcquired (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexNotAcquired, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_RELEASE_DISABLE))
__WEAK void EvrLosMutexRelease (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexRelease, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_RELEASED_DISABLE))
__WEAK void EvrLosMutexReleased (uint32_t mutex_id, uint32_t lock) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexReleased, (uint32_t)mutex_id, lock);
#else
  (void)mutex_id;
  (void)lock;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_GET_OWNER_DISABLE))
__WEAK void EvrLosMutexGetOwner (uint32_t mutex_id, uint32_t task_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexGetOwner, (uint32_t)mutex_id, (uint32_t)task_id);
#else
  (void)mutex_id;
  (void)task_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_DELETE_DISABLE))
__WEAK void EvrLosMutexDelete (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexDelete, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_DESTROYED_DISABLE))
__WEAK void EvrLosMutexDestroyed (uint32_t mutex_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMutexDestroyed, (uint32_t)mutex_id, 0U);
#else
  (void)mutex_id;
#endif
}
#endif


//  ==== Semaphore Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ERROR_DISABLE))
__WEAK void EvrLosSemaphoreError (uint32_t semaphore_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreError, (uint32_t)semaphore_id, (uint32_t)status);
#else
  (void)semaphore_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_NEW_DISABLE))
__WEAK void EvrLosSemaphoreNew (uint32_t max_count, uint32_t initial_count, uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosSemaphoreNew, max_count, initial_count, (uint32_t)semaphore_id, 0U);
#else
  (void)max_count;
  (void)initial_count;
  (void)semaphore_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_CREATED_DISABLE))
__WEAK void EvrLosSemaphoreCreated (uint32_t semaphore_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreCreated, (uint32_t)semaphore_id, (uint32_t)name);
#else
  (void)semaphore_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_GET_NAME_DISABLE))
__WEAK void EvrLosSemaphoreGetName (uint32_t semaphore_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreGetName, (uint32_t)semaphore_id, (uint32_t)name);
#else
#endif
  (void)semaphore_id;
  (void)name;
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_DISABLE))
__WEAK void EvrLosSemaphoreAcquire (uint32_t semaphore_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreAcquire, (uint32_t)semaphore_id, timeout);
#else
  (void)semaphore_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_PENDING_DISABLE))
__WEAK void EvrLosSemaphoreAcquirePending (uint32_t semaphore_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreAcquirePending, (uint32_t)semaphore_id, (uint32_t)timeout);
#else
  (void)semaphore_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_TIMEOUT_DISABLE))
__WEAK void EvrLosSemaphoreAcquireTimeout (uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreAcquireTimeout, (uint32_t)semaphore_id, 0U);
#else
  (void)semaphore_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRED_DISABLE))
__WEAK void EvrLosSemaphoreAcquired (uint32_t semaphore_id, uint32_t tokens) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreAcquired, (uint32_t)semaphore_id, tokens);
#else
  (void)semaphore_id;
  (void)tokens;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_NOT_ACQUIRED_DISABLE))
__WEAK void EvrLosSemaphoreNotAcquired (uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreNotAcquired, (uint32_t)semaphore_id, 0U);
#else
  (void)semaphore_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_RELEASE_DISABLE))
__WEAK void EvrLosSemaphoreRelease (uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreRelease, (uint32_t)semaphore_id, 0U);
#else
  (void)semaphore_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_RELEASED_DISABLE))
__WEAK void EvrLosSemaphoreReleased (uint32_t semaphore_id, uint32_t tokens) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreReleased, (uint32_t)semaphore_id, tokens);
#else
  (void)semaphore_id;
  (void)tokens;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_GET_COUNT_DISABLE))
__WEAK void EvrLosSemaphoreGetCount (uint32_t semaphore_id, uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreGetCount, (uint32_t)semaphore_id, count);
#else
  (void)semaphore_id;
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_DELETE_DISABLE))
__WEAK void EvrLosSemaphoreDelete (uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreDelete, (uint32_t)semaphore_id, 0U);
#else
  (void)semaphore_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_DESTROYED_DISABLE))
__WEAK void EvrLosSemaphoreDestroyed (uint32_t semaphore_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosSemaphoreDestroyed, (uint32_t)semaphore_id, 0U);
#else
  (void)semaphore_id;
#endif
}
#endif


//  ==== Memory Pool Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ERROR_DISABLE))
__WEAK void EvrLosMemoryPoolError (uint32_t mp_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolError, (uint32_t)mp_id, (uint32_t)status);
#else
  (void)mp_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_NEW_DISABLE))
__WEAK void EvrLosMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMemoryPoolNew, block_count, block_size, (uint32_t)attr, 0U);
#else
  (void)block_count;
  (void)block_size;
  (void)attr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_CREATED_DISABLE))
__WEAK void EvrLosMemoryPoolCreated (uint32_t mp_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolCreated, (uint32_t)mp_id, (uint32_t)name);
#else
  (void)mp_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_NAME_DISABLE))
__WEAK void EvrLosMemoryPoolGetName (uint32_t mp_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolGetName, (uint32_t)mp_id, (uint32_t)name);
#else
  (void)mp_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_DISABLE))
__WEAK void EvrLosMemoryPoolAlloc (uint32_t mp_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolAlloc, (uint32_t)mp_id, timeout);
#else
  (void)mp_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_PENDING_DISABLE))
__WEAK void EvrLosMemoryPoolAllocPending (uint32_t mp_id, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolAllocPending, (uint32_t)mp_id, timeout);
#else
  (void)mp_id;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_TIMEOUT_DISABLE))
__WEAK void EvrLosMemoryPoolAllocTimeout (uint32_t mp_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolAllocTimeout, (uint32_t)mp_id, 0U);
#else
  (void)mp_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOCATED_DISABLE))
__WEAK void EvrLosMemoryPoolAllocated (uint32_t mp_id, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolAllocated, (uint32_t)mp_id, (uint32_t)block);
#else
  (void)mp_id;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_FAILED_DISABLE))
__WEAK void EvrLosMemoryPoolAllocFailed (uint32_t mp_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolAllocFailed, (uint32_t)mp_id, 0U);
#else
  (void)mp_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_FREE_DISABLE))
__WEAK void EvrLosMemoryPoolFree (uint32_t mp_id, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolFree, (uint32_t)mp_id, (uint32_t)block);
#else
  (void)mp_id;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DEALLOCATED_DISABLE))
__WEAK void EvrLosMemoryPoolDeallocated (uint32_t mp_id, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolDeallocated, (uint32_t)mp_id, (uint32_t)block);
#else
  (void)mp_id;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_FREE_FAILED_DISABLE))
__WEAK void EvrLosMemoryPoolFreeFailed (uint32_t mp_id, void *block) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolFreeFailed, (uint32_t)mp_id, (uint32_t)block);
#else
  (void)mp_id;
  (void)block;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_CAPACITY_DISABLE))
__WEAK void EvrLosMemoryPoolGetCapacity (uint32_t mp_id, uint32_t capacity) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolGetCapacity, (uint32_t)mp_id, capacity);
#else
  (void)mp_id;
  (void)capacity;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_BLOCK_SZIE_DISABLE))
__WEAK void EvrLosMemoryPoolGetBlockSize (uint32_t mp_id, uint32_t block_size) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolGetBlockSize, (uint32_t)mp_id, block_size);
#else
  (void)mp_id;
  (void)block_size;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_COUNT_DISABLE))
__WEAK void EvrLosMemoryPoolGetCount (uint32_t mp_id, uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolGetCount, (uint32_t)mp_id, count);
#else
  (void)mp_id;
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_SPACE_DISABLE))
__WEAK void EvrLosMemoryPoolGetSpace (uint32_t mp_id, uint32_t space) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolGetSpace, (uint32_t)mp_id, space);
#else
  (void)mp_id;
  (void)space;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DELETE_DISABLE))
__WEAK void EvrLosMemoryPoolDelete (uint32_t mp_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolDelete, (uint32_t)mp_id, 0U);
#else
  (void)mp_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DESTROYED_DISABLE))
__WEAK void EvrLosMemoryPoolDestroyed (uint32_t mp_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMemoryPoolDestroyed, (uint32_t)mp_id, 0U);
#else
  (void)mp_id;
#endif
}
#endif


//  ==== Message Queue Events ====

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_ERROR_DISABLE))
__WEAK void EvrLosMessageQueueError (uint32_t mq_id, int32_t status) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2 (EvtLosMessageQueueError, (uint32_t)mq_id, (uint32_t)status);
#else
  (void)mq_id;
  (void)status;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NEW_DISABLE))
__WEAK void EvrLosMessageQueueNew (uint32_t msg_count, uint32_t msg_size, uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMessageQueueNew, msg_count, msg_size, (uint32_t)mq_id, 0U);
#else
  (void)msg_count;
  (void)msg_size;
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_CREATED_DISABLE))
__WEAK void EvrLosMessageQueueCreated (uint32_t mq_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueCreated, (uint32_t)mq_id, (uint32_t)name);
#else
  (void)mq_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_NAME_DISABLE))
__WEAK void EvrLosMessageQueueGetName (uint32_t mq_id, const char *name) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetName, (uint32_t)mq_id, (uint32_t)name);
#else
  (void)mq_id;
  (void)name;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_DISABLE))
__WEAK void EvrLosMessageQueuePut (uint32_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMessageQueuePut, (uint32_t)mq_id, (uint32_t)msg_ptr, (uint32_t)msg_prio, timeout);
#else
  (void)mq_id;
  (void)msg_ptr;
  (void)msg_prio;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_PENDING_DISABLE))
__WEAK void EvrLosMessageQueuePutPending (uint32_t mq_id, const void *msg_ptr, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMessageQueuePutPending, (uint32_t)mq_id, (uint32_t)msg_ptr, timeout, 0U);
#else
  (void)mq_id;
  (void)msg_ptr;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_TIMEOUT_DISABLE))
__WEAK void EvrLosMessageQueuePutTimeout (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueuePutTimeout, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_INSERT_PENDING_DISABLE))
__WEAK void EvrLosMessageQueueInsertPending (uint32_t mq_id, const void *msg_ptr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueInsertPending, (uint32_t)mq_id, (uint32_t)msg_ptr);
#else
  (void)mq_id;
  (void)msg_ptr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_INSERTED_DISABLE))
__WEAK void EvrLosMessageQueueInserted (uint32_t mq_id, const void *msg_ptr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueInserted, (uint32_t)mq_id, (uint32_t)msg_ptr);
#else
  (void)mq_id;
  (void)msg_ptr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NOT_INSERTED_DISABLE))
__WEAK void EvrLosMessageQueueNotInserted (uint32_t mq_id, const void *msg_ptr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueNotInserted, (uint32_t)mq_id, (uint32_t)msg_ptr);
#else
  (void)mq_id;
  (void)msg_ptr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_DISABLE))
__WEAK void EvrLosMessageQueueGet (uint32_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMessageQueueGet, (uint32_t)mq_id, (uint32_t)msg_ptr, (uint32_t)msg_prio, timeout);
#else
  (void)mq_id;
  (void)msg_ptr;
  (void)msg_prio;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_PENDING_DISABLE))
__WEAK void EvrLosMessageQueueGetPending (uint32_t mq_id, void *msg_ptr, uint32_t timeout) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord4(EvtLosMessageQueueGetPending, (uint32_t)mq_id, (uint32_t)msg_ptr, timeout, 0U);
#else
  (void)mq_id;
  (void)msg_ptr;
  (void)timeout;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_TIMEOUT_DISABLE))
__WEAK void EvrLosMessageQueueGetTimeout (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetTimeout, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RETRIEVED_DISABLE))
__WEAK void EvrLosMessageQueueRetrieved (uint32_t mq_id, void *msg_ptr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueRetrieved, (uint32_t)mq_id, (uint32_t)msg_ptr);
#else
  (void)mq_id;
  (void)msg_ptr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NOT_RETRIEVED_DISABLE))
__WEAK void EvrLosMessageQueueNotRetrieved (uint32_t mq_id, void *msg_ptr) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueNotRetrieved, (uint32_t)mq_id, (uint32_t)msg_ptr);
#else
  (void)mq_id;
  (void)msg_ptr;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_CAPACITY_DISABLE))
__WEAK void EvrLosMessageQueueGetCapacity (uint32_t mq_id, uint32_t capacity) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetCapacity, (uint32_t)mq_id, capacity);
#else
  (void)mq_id;
  (void)capacity;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_MSG_SIZE_DISABLE))
__WEAK void EvrLosMessageQueueGetMsgSize (uint32_t mq_id, uint32_t msg_size) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetMsgSize, (uint32_t)mq_id, msg_size);
#else
  (void)mq_id;
  (void)msg_size;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_COUNT_DISABLE))
__WEAK void EvrLosMessageQueueGetCount (uint32_t mq_id, uint32_t count) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetCount, (uint32_t)mq_id, count);
#else
  (void)mq_id;
  (void)count;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_SPACE_DISABLE))
__WEAK void EvrLosMessageQueueGetSpace (uint32_t mq_id, uint32_t space) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueGetSpace, (uint32_t)mq_id, space);
#else
  (void)mq_id;
  (void)space;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RESET_DISABLE))
__WEAK void EvrLosMessageQueueReset (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueReset, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RESET_DONE_DISABLE))
__WEAK void EvrLosMessageQueueResetDone (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueResetDone, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_DELETE_DISABLE))
__WEAK void EvrLosMessageQueueDelete (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueDelete, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif

#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_DESTROYED_DISABLE))
__WEAK void EvrLosMessageQueueDestroyed (uint32_t mq_id) {
#if defined(RTE_Compiler_EventRecorder)
  (void)EventRecord2(EvtLosMessageQueueDestroyed, (uint32_t)mq_id, 0U);
#else
  (void)mq_id;
#endif
}
#endif
