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
 * Title:       LOS Event Recorder definitions
 *
 * -----------------------------------------------------------------------------
 */

#ifndef _LOS_SYSVIEW_EVR_H_
#define _LOS_SYSVIEW_EVR_H_

#include "cmsis_os2.h"                  // CMSIS RTOS API
#include "los_config.h"                 // Liteos Configuration
#include "los_task.h"                     // Liteos definitions
#include "los_sysview_cfg.h"

//  ==== Memory Events ====

/**
  \brief  Event on memory initialization (Op)
  \param[in]  mem           pointer to memory pool.
  \param[in]  size          size of a memory pool in bytes.
  \param[in]  result        execution status: 1 - success, 0 - failure.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_INIT_DISABLE))
extern void EvrLosMemoryInit (void *mem, uint32_t size, uint32_t result);
#else
#define EvrLosMemoryInit(mem, size, result)
#endif

/**
  \brief  Event on memory allocate (Op)
  \param[in]  mem           pointer to memory pool.
  \param[in]  size          size of a memory block in bytes.
  \param[in]  type          memory block type: 0 - generic, 1 - control block.
  \param[in]  block         pointer to allocated memory block or NULL in case of no memory is available.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_ALLOC_DISABLE))
extern void EvrLosMemoryAlloc (void *mem, uint32_t size, uint32_t type, void *block);
#else
#define EvrLosMemoryAlloc(mem, size, type, block)
#endif

/**
  \brief  Event on memory free (Op)
  \param[in]  mem           pointer to memory pool.
  \param[in]  block         memory block to be returned to the memory pool.
  \param[in]  result        execution status: 1 - success, 0 - failure.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_FREE_DISABLE))
extern void EvrLosMemoryFree (void *mem, void *block, uint32_t result);
#else
#define EvrLosMemoryFree(mem, block, result)
#endif

/**
  \brief  Event on memory block initialization (Op)
  \param[in]  mp_info       memory pool info.
  \param[in]  block_count   maximum number of memory blocks in memory pool.
  \param[in]  block_size    size of a memory block in bytes.
  \param[in]  block_mem     pointer to memory for block storage.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_INIT_DISABLE))
extern void EvrLosMemoryBlockInit (void *mp_info, uint32_t block_count, uint32_t block_size, void *block_mem);
#else
#define EvrLosMemoryBlockInit(mp_info, block_count, block_size, block_mem)
#endif

/**
  \brief  Event on memory block alloc (Op)
  \param[in]  mp_info       memory pool info.
  \param[in]  block         address of the allocated memory block or NULL in case of no memory is available.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_ALLOC_DISABLE))
extern void EvrLosMemoryBlockAlloc (void *mp_info, void *block);
#else
#define EvrLosMemoryBlockAlloc(mp_info, block)
#endif

/**
  \brief  Event on memory block free (Op)
  \param[in]  mp_info       memory pool info.
  \param[in]  block         address of the allocated memory block to be returned to the memory pool.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMORY != 0) && !defined(EVR_LOS_MEMORY_BLOCK_FREE_DISABLE))
extern void EvrLosMemoryBlockFree (void *mp_info, void *block, int32_t status);
#else
#define EvrLosMemoryBlockFree(mp_info, block, status)
#endif


//  ==== Kernel Events ====

/**
  \brief  Event on RTOS kernel error (Error)
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_ERROR_DISABLE))
extern void EvrLosKernelError (int32_t status);
#else
#define EvrLosKernelError(status)
#endif

/**
  \brief  Event on RTOS kernel initialize (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INITIALIZE_DISABLE))
extern void EvrLosKernelInitialize (void);
#else
#define EvrLosKernelInitialize()
#endif

/**
  \brief  Event on successful RTOS kernel initialize (Op)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INITIALIZED_DISABLE))
extern void EvrLosKernelInitialized (void);
#else
#define EvrLosKernelInitialized()
#endif

/**
  \brief  Event on RTOS kernel information retrieve (API)
  \param[in]  version       pointer to buffer for retrieving version information.
  \param[in]  id_buf        pointer to buffer for retrieving kernel identification string.
  \param[in]  id_size       size of buffer for kernel identification string.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_INFO_DISABLE))
extern void EvrLosKernelGetInfo (osVersion_t *version, char *id_buf, uint32_t id_size);
#else
#define EvrLosKernelGetInfo(version, id_buf, id_size)
#endif

/**
  \brief  Event on successful RTOS kernel information retrieve (Op)
  \param[in]  version       pointer to buffer for retrieving version information.
  \param[in]  id_buf        pointer to buffer for retrieving kernel identification string.
  \param[in]  id_size       size of buffer for kernel identification string.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_INFO_RETRIEVED_DISABLE))
extern void EvrLosKernelInfoRetrieved (const osVersion_t *version, const char *id_buf, uint32_t id_size);
#else
#define EvrLosKernelInfoRetrieved(version, id_buf, id_size)
#endif

/**
  \brief  Event on current RTOS Kernel state retrieve (API)
  \param[in]  state         current RTOS Kernel state.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_STATE_DISABLE))
extern void EvrLosKernelGetState (osKernelState_t state);
#else
#define EvrLosKernelGetState(state)
#endif

/**
  \brief  Event on RTOS Kernel scheduler start (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_START_DISABLE))
extern void EvrLosKernelStart (void);
#else
#define EvrLosKernelStart()
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler start (Op)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_STARTED_DISABLE))
extern void EvrLosKernelStarted (void);
#else
#define EvrLosKernelStarted()
#endif

/**
  \brief  Event on RTOS Kernel scheduler lock (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCK_DISABLE))
extern void EvrLosKernelLock (void);
#else
#define EvrLosKernelLock()
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler lock (Op)
  \param[in]  lock          previous lock state (1 - locked, 0 - not locked).
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCKED_DISABLE))
extern void EvrLosKernelLocked (int32_t lock);
#else
#define EvrLosKernelLocked(lock)
#endif

/**
  \brief  Event on RTOS Kernel scheduler unlock (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_UNLOCK_DISABLE))
extern void EvrLosKernelUnlock (void);
#else
#define EvrLosKernelUnlock()
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler unlock (Op)
  \param[in]  lock          previous lock state (1 - locked, 0 - not locked).
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_UNLOCKED_DISABLE))
extern void EvrLosKernelUnlocked (int32_t lock);
#else
#define EvrLosKernelUnlocked(lock)
#endif

/**
  \brief  Event on RTOS Kernel scheduler lock state restore (API)
  \param[in]  lock          lock state obtained by \ref osKernelLock or \ref osKernelUnlock.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESTORE_LOCK_DISABLE))
extern void EvrLosKernelRestoreLock (int32_t lock);
#else
#define EvrLosKernelRestoreLock(lock)
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler lock state restore (Op)
  \param[in]  lock          new lock state (1 - locked, 0 - not locked).
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_LOCK_RESTORED_DISABLE))
extern void EvrLosKernelLockRestored (int32_t lock);
#else
#define EvrLosKernelLockRestored(lock)
#endif

/**
  \brief  Event on RTOS Kernel scheduler suspend (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_SUSPEND_DISABLE))
extern void EvrLosKernelSuspend (void);
#else
#define EvrLosKernelSuspend()
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler suspend (Op)
  \param[in]  sleep_ticks   time in ticks, for how long the system can sleep or power-down.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_SUSPENDED_DISABLE))
extern void EvrLosKernelSuspended (uint32_t sleep_ticks);
#else
#define EvrLosKernelSuspended(sleep_ticks)
#endif

/**
  \brief  Event on RTOS Kernel scheduler resume (API)
  \param[in]  sleep_ticks   time in ticks, for how long the system was in sleep or power-down mode.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESUME_DISABLE))
extern void EvrLosKernelResume (uint32_t sleep_ticks);
#else
#define EvrLosKernelResume(sleep_ticks)
#endif

/**
  \brief  Event on successful RTOS Kernel scheduler resume (Op)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_RESUMED_DISABLE))
extern void EvrLosKernelResumed (void);
#else
#define EvrLosKernelResumed()
#endif

/**
  \brief  Event on RTOS kernel tick count retrieve (API)
  \param[in]  count         RTOS kernel current tick count.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_TICK_COUNT_DISABLE))
extern void EvrLosKernelGetTickCount (uint32_t count);
#else
#define EvrLosKernelGetTickCount(count)
#endif

/**
  \brief  Event on RTOS kernel tick frequency retrieve (API)
  \param[in]  freq          frequency of the kernel tick.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_TICK_FREQ_DISABLE))
extern void EvrLosKernelGetTickFreq (uint32_t freq);
#else
#define EvrLosKernelGetTickFreq(freq)
#endif

/**
  \brief  Event on RTOS kernel system timer count retrieve (API)
  \param[in]  count         RTOS kernel current system timer count as 32-bit value.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_SYS_TIMER_COUNT_DISABLE))
extern void EvrLosKernelGetSysTimerCount (uint32_t count);
#else
#define EvrLosKernelGetSysTimerCount(count)
#endif

/**
  \brief  Event on RTOS kernel system timer frequency retrieve (API)
  \param[in]  freq          frequency of the system timer.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_KERNEL != 0) && !defined(EVR_LOS_KERNEL_GET_SYS_TIMER_FREQ_DISABLE))
extern void EvrLosKernelGetSysTimerFreq (uint32_t freq);
#else
#define EvrLosKernelGetSysTimerFreq(freq)
#endif


//  ==== TASK Events ====

/**
  \brief  Event on TASK error (Error)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_ERROR_DISABLE))
extern void EvrLosTaskError (uint32_t TASK_id, int32_t status);
#else
#define EvrLosTaskError(TASK_id, status)
#endif

/**
  \brief  Event on TASK create and intialize (API)
  \param[in]  func          TASK function.
  \param[in]  argument      pointer that is passed to the TASK function as start argument.
  \param[in]  attr          TASK attributes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_NEW_DISABLE))
extern void EvrLosTaskNew (TSK_ENTRY_FUNC func, void *argument, uint32_t task_id);
#else
#define EvrLosTaskNew(func, argument, task_id)
#endif

/**
  \brief  Event on successful TASK create (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  TASK_addr   TASK entry address.
  \param[in]  name          pointer to TASK object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_CREATED_DISABLE))
extern void EvrLosTaskCreated (LosTaskCB *taskCB);
#else
#define EvrLosTaskCreated(taskCB)
#endif

/**
  \brief  Event on TASK name retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  name          pointer to TASK object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_NAME_DISABLE))
extern void EvrLosTaskGetName (uint32_t TASK_id, const char *name);
#else
#define EvrLosTaskGetName(TASK_id, name)
#endif

/**
  \brief  Event on current running TASK ID retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_ID_DISABLE))
extern void EvrLosTaskGetId (uint32_t TASK_id);
#else
#define EvrLosTaskGetId(TASK_id)
#endif

/**
  \brief  Event on TASK state retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  state         current TASK state of the specified TASK.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_STATE_DISABLE))
extern void EvrLosTaskGetState (uint32_t TASK_id, uint32_t state);
#else
#define EvrLosTaskGetState(TASK_id, state)
#endif

/**
  \brief  Event on TASK stack size retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  stack_size    stack size in bytes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_STACK_SIZE_DISABLE))
extern void EvrLosTaskGetStackSize (uint32_t TASK_id, uint32_t stack_size);
#else
#define EvrLosTaskGetStackSize(TASK_id, stack_size)
#endif

/**
  \brief  Event on available stack space retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  stack_space   remaining stack space in bytes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_STACK_SPACE_DISABLE))
extern void EvrLosTaskGetStackSpace (uint32_t TASK_id, uint32_t stack_space);
#else
#define EvrLosTaskGetStackSpace(TASK_id, stack_space)
#endif

/**
  \brief  Event on TASK priority set (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  priority      new priority value for the TASK function.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_SET_PRIORITY_DISABLE))
extern void EvrLosTaskSetPriority (uint32_t TASK_id, osPriority_t priority);
#else
#define EvrLosTaskSetPriority(TASK_id, priority)
#endif

/**
  \brief  Event on TASK priority updated (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  priority      new priority value for the TASK function.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_PRIORITY_UPDATED_DISABLE))
extern void EvrLosTaskPriorityUpdated (uint32_t TASK_id, osPriority_t priority);
#else
#define EvrLosTaskPriorityUpdated(TASK_id, priority)
#endif

/**
  \brief  Event on TASK priority retrieve (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  priority      current priority value of the specified TASK.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_PRIORITY_DISABLE))
extern void EvrLosTaskGetPriority (uint32_t TASK_id, osPriority_t priority);
#else
#define EvrLosTaskGetPriority(TASK_id, priority)
#endif

/**
  \brief  Event on TASK yield (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_YIELD_DISABLE))
extern void EvrLosTaskYield (void);
#else
#define EvrLosTaskYield()
#endif

/**
  \brief  Event on TASK suspend (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_SUSPEND_DISABLE))
extern void EvrLosTaskSuspend (uint32_t TASK_id);
#else
#define EvrLosTaskSuspend(TASK_id)
#endif

/**
  \brief  Event on successful TASK suspend (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_SUSPENDED_DISABLE))
extern void EvrLosTaskSuspended (uint32_t TASK_id);
#else
#define EvrLosTaskSuspended(TASK_id)
#endif

/**
  \brief  Event on TASK resume (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_RESUME_DISABLE))
extern void EvrLosTaskResume (uint32_t TASK_id);
#else
#define EvrLosTaskResume(TASK_id)
#endif

/**
  \brief  Event on successful TASK resume (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_RESUMED_DISABLE))
extern void EvrLosTaskResumed (uint32_t TASK_id);
#else
#define EvrLosTaskResumed(TASK_id)
#endif

/**
  \brief  Event on TASK detach (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_DETACH_DISABLE))
extern void EvrLosTaskDetach (uint32_t TASK_id);
#else
#define EvrLosTaskDetach(TASK_id)
#endif

/**
  \brief  Event on successful TASK detach (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_DETACHED_DISABLE))
extern void EvrLosTaskDetached (uint32_t TASK_id);
#else
#define EvrLosTaskDetached(TASK_id)
#endif

/**
  \brief  Event on TASK join (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_JOIN_DISABLE))
extern void EvrLosTaskJoin (uint32_t TASK_id);
#else
#define EvrLosTaskJoin(TASK_id)
#endif

/**
  \brief  Event on pending TASK join (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_JOIN_PENDING_DISABLE))
extern void EvrLosTaskJoinPending (uint32_t TASK_id);
#else
#define EvrLosTaskJoinPending(TASK_id)
#endif

/**
  \brief  Event on successful TASK join (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_JOINED_DISABLE))
extern void EvrLosTaskJoined (uint32_t TASK_id);
#else
#define EvrLosTaskJoined(TASK_id)
#endif

/**
  \brief  Event on TASK execution block (Detail)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_BLOCKED_DISABLE))
extern void EvrLosTaskBlocked (uint32_t TASK_id, uint32_t timeout);
#else
#define EvrLosTaskBlocked(TASK_id, timeout)
#endif

/**
  \brief  Event on TASK execution unblock (Detail)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  ret_val       extended execution status of the TASK.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_UNBLOCKED_DISABLE))
extern void EvrLosTaskUnblocked (uint32_t TASK_id, uint32_t ret_val);
#else
#define EvrLosTaskUnblocked(TASK_id, ret_val)
#endif

/**
  \brief  Event on running TASK pre-emption (Detail)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_PREEMPTED_DISABLE))
extern void EvrLosTaskPreempted (uint32_t TASK_id);
#else
#define EvrLosTaskPreempted(TASK_id)
#endif

/**
  \brief  Event on running TASK switch (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_SWITCHED_DISABLE))
extern void EvrLosTaskSwitched (uint32_t TASK_id);
#else
#define EvrLosTaskSwitched(TASK_id)
#endif

/**
  \brief  Event on TASK exit (API)
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_EXIT_DISABLE))
extern void EvrLosTaskExit (void);
#else
#define EvrLosTaskExit()
#endif

/**
  \brief  Event on TASK terminate (API)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_TERMINATE_DISABLE))
extern void EvrLosTaskTerminate (uint32_t TASK_id);
#else
#define EvrLosTaskTerminate(TASK_id)
#endif

/**
  \brief  Event on successful TASK terminate (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_DESTROYED_DISABLE))
extern void EvrLosTaskDestroyed (uint32_t TASK_id);
#else
#define EvrLosTaskDestroyed(TASK_id)
#endif

/**
  \brief  Event on active TASK count retrieve (API)
  \param[in]  count         number of active TASKs.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_GET_COUNT_DISABLE))
extern void EvrLosTaskGetCount (uint32_t count);
#else
#define EvrLosTaskGetCount(count)
#endif

/**
  \brief  Event on active TASKs enumerate (API)
  \param[in]  TASK_array  pointer to array for retrieving TASK IDs.
  \param[in]  array_items   maximum number of items in array for retrieving TASK IDs.
  \param[in]  count         number of enumerated TASKs.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TASK != 0) && !defined(EVR_LOS_TASK_ENUMERATE_DISABLE))
extern void EvrLosTaskEnumerate (uint32_t *TASK_array, uint32_t array_items, uint32_t count);
#else
#define EvrLosTaskEnumerate(TASK_array, array_items, count)
#endif


//  ==== TASK Flags Events ====

/**
  \brief  Event on TASK flags error (Error)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_ERROR_DISABLE))
extern void EvrLosTaskFlagsError (uint32_t TASK_id, int32_t status);
#else
#define EvrLosTaskFlagsError(TASK_id, status)
#endif

/**
  \brief  Event on TASK flags set (API)
  \param[in]   TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]   flags         flags of the TASK that shall be set.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_SET_DISABLE))
extern void EvrLosTaskFlagsSet (uint32_t TASK_id, uint32_t flags);
#else
#define EvrLosTaskFlagsSet(TASK_id, flags)
#endif

/**
  \brief  Event on successful TASK flags set (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
  \param[in]  TASK_flags  TASK flags after setting.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_SET_DONE_DISABLE))
extern void EvrLosTaskFlagsSetDone (uint32_t TASK_id, uint32_t TASK_flags);
#else
#define EvrLosTaskFlagsSetDone(TASK_id, TASK_flags)
#endif

/**
  \brief  Event on TASK flags clear (API)
  \param[in]  flags         flags of the TASK that shall be cleared.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_CLEAR_DISABLE))
extern void EvrLosTaskFlagsClear (uint32_t flags);
#else
#define EvrLosTaskFlagsClear(flags)
#endif

/**
  \brief  Event on successful TASK flags clear (Op)
  \param[in]  TASK_flags  TASK flags before clearing.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_CLEAR_DONE_DISABLE))
extern void EvrLosTaskFlagsClearDone (uint32_t TASK_flags);
#else
#define EvrLosTaskFlagsClearDone(TASK_flags)
#endif

/**
  \brief  Event on TASK flags retrieve (API)
  \param[in]  TASK_flags  current TASK flags.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_GET_DISABLE))
extern void EvrLosTaskFlagsGet (uint32_t TASK_flags);
#else
#define EvrLosTaskFlagsGet(TASK_flags)
#endif

/**
  \brief  Event on wait for TASK flags (API)
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_WAIT_DISABLE))
extern void EvrLosTaskFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout);
#else
#define EvrLosTaskFlagsWait(flags, options, timeout)
#endif

/**
  \brief  Event on pending wait for TASK flags (Op)
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_WAIT_PENDING_DISABLE))
extern void EvrLosTaskFlagsWaitPending (uint32_t flags, uint32_t options, uint32_t timeout);
#else
#define EvrLosTaskFlagsWaitPending(flags, options, timeout)
#endif

/**
  \brief  Event on wait timeout for TASK flags (Op)
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_WAIT_TIMEOUT_DISABLE))
extern void EvrLosTaskFlagsWaitTimeout (uint32_t TASK_id);
#else
#define EvrLosTaskFlagsWaitTimeout(TASK_id)
#endif

/**
  \brief  Event on successful wait for TASK flags (Op)
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  TASK_flags  TASK flags before clearing.
  \param[in]  TASK_id     TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_WAIT_COMPLETED_DISABLE))
extern void EvrLosTaskFlagsWaitCompleted (uint32_t flags, uint32_t options, uint32_t TASK_flags, uint32_t TASK_id);
#else
#define EvrLosTaskFlagsWaitCompleted(flags, options, TASK_flags, TASK_id)
#endif

/**
  \brief  Event on unsuccessful wait for TASK flags (Op)
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_THFLAGS != 0) && !defined(EVR_LOS_TASK_FLAGS_WAIT_NOT_COMPLETED_DISABLE))
extern void EvrLosTaskFlagsWaitNotCompleted (uint32_t flags, uint32_t options);
#else
#define EvrLosTaskFlagsWaitNotCompleted(flags, options)
#endif


//  ==== Generic Wait Events ====

/**
  \brief  Event on delay error (Error)
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_ERROR_DISABLE))
extern void EvrLosDelayError (int32_t status);
#else
#define EvrLosDelayError(status)
#endif

/**
  \brief  Event on delay for specified time (API)
  \param[in]  ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_DISABLE))
extern void EvrLosDelay (uint32_t ticks);
#else
#define EvrLosDelay(ticks)
#endif

/**
  \brief  Event on delay until specified time (API)
  \param[in]  ticks         absolute time in ticks.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_UNTIL_DISABLE))
extern void EvrLosDelayUntil (uint32_t ticks);
#else
#define EvrLosDelayUntil(ticks)
#endif

/**
  \brief  Event on delay started (Op)
  \param[in]  ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_STARTED_DISABLE))
extern void EvrLosDelayStarted (uint32_t ticks);
#else
#define EvrLosDelayStarted(ticks)
#endif

/**
  \brief  Event on delay until specified time started (Op)
  \param[in]  ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_UNTIL_STARTED_DISABLE))
extern void EvrLosDelayUntilStarted (uint32_t ticks);
#else
#define EvrLosDelayUntilStarted(ticks)
#endif

/**
  \brief  Event on delay completed (Op)
  \param[in]  TASK_id     TASK ID obtained.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_WAIT != 0) && !defined(EVR_LOS_DELAY_COMPLETED_DISABLE))
extern void EvrLosDelayCompleted (uint32_t task_id);
#else
#define EvrLosDelayCompleted(task_id)
#endif


//  ==== Timer Events ====

/**
  \brief  Event on timer error (Error)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_ERROR_DISABLE))
extern void EvrLosTimerError (osTimerId_t timer_id, int32_t status);
#else
#define EvrLosTimerError(timer_id, status)
#endif

/**
  \brief  Event on timer callback call (Op)
  \param[in]  func          start address of a timer call back function.
  \param[in]  argument      argument to the timer call back function.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_CALLBACK_DISABLE))
extern void EvrLosTimerCallback (osTimerFunc_t func, void *argument);
#else
#define EvrLosTimerCallback(func, argument)
#endif

/**
  \brief  Event on timer create and initialize (API)
  \param[in]  func          start address of a timer call back function.
  \param[in]  type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
  \param[in]  argument      argument to the timer call back function.
  \param[in]  attr          timer attributes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_NEW_DISABLE))
extern void EvrLosTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr);
#else
#define EvrLosTimerNew(func, type, argument, attr)
#endif

/**
  \brief  Event on successful timer create (Op)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
  \param[in]  name          pointer to timer object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_CREATED_DISABLE))
extern void EvrLosTimerCreated (uint32_t timer_id, const char *name);
#else
#define EvrLosTimerCreated(timer_id, name)
#endif

/**
  \brief  Event on timer name retrieve (API)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
  \param[in]  name          pointer to timer object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_GET_NAME_DISABLE))
extern void EvrLosTimerGetName (uint32_t timer_id, const char *name);
#else
#define EvrLosTimerGetName(timer_id, name)
#endif

/**
  \brief  Event on timer start (API)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
  \param[in]  ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value of the timer.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_START_DISABLE))
extern void EvrLosTimerStart (uint32_t timer_id, uint32_t ticks);
#else
#define EvrLosTimerStart(timer_id, ticks)
#endif

/**
  \brief  Event on successful timer start (Op)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STARTED_DISABLE))
extern void EvrLosTimerStarted (uint32_t timer_id);
#else
#define EvrLosTimerStarted(timer_id)
#endif

/**
  \brief  Event on timer stop (API)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STOP_DISABLE))
extern void EvrLosTimerStop (uint32_t timer_id);
#else
#define EvrLosTimerStop(timer_id)
#endif

/**
  \brief  Event on successful timer stop (Op)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_STOPPED_DISABLE))
extern void EvrLosTimerStopped (uint32_t timer_id);
#else
#define EvrLosTimerStopped(timer_id)
#endif

/**
  \brief  Event on timer running state check (API)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
  \param[in]  running       running state: 0 not running, 1 running.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_IS_RUNNING_DISABLE))
extern void EvrLosTimerIsRunning (uint32_t timer_id, uint32_t running);
#else
#define EvrLosTimerIsRunning(timer_id, running)
#endif

/**
  \brief  Event on timer delete (API)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_DELETE_DISABLE))
extern void EvrLosTimerDelete (uint32_t timer_id);
#else
#define EvrLosTimerDelete(timer_id)
#endif

/**
  \brief  Event on successful timer delete (Op)
  \param[in]  timer_id      timer ID obtained by \ref LOS_SwtmrCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_TIMER != 0) && !defined(EVR_LOS_TIMER_DESTROYED_DISABLE))
extern void EvrLosTimerDestroyed (uint32_t timer_id);
#else
#define EvrLosTimerDestroyed(timer_id)
#endif


//  ==== Event Flags Events ====

/**
  \brief  Event on event flags error (Error)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_ERROR_DISABLE))
extern void EvrLosEventFlagsError (uint32_t ef_id, int32_t status);
#else
#define EvrLosEventFlagsError(ef_id, status)
#endif

/**
  \brief  Event on event flags create and initialize (API)
  \param[in]  attr          event flags attributes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_NEW_DISABLE))
extern void EvrLosEventFlagsNew (uint32_t ef_id);
#else
#define EvrLosEventFlagsNew(ef_id)
#endif

/**
  \brief  Event on successful event flags create (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  name          pointer to event flags object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CREATED_DISABLE))
extern void EvrLosEventFlagsCreated (uint32_t ef_id, const char *name);
#else
#define EvrLosEventFlagsCreated(ef_id, name)
#endif

/**
  \brief  Event on event flags name retrieve (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  name          pointer to event flags object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_GET_NAME_DISABLE))
extern void EvrLosEventFlagsGetName (uint32_t ef_id, const char *name);
#else
#define EvrLosEventFlagsGetName(ef_id, name)
#endif

/**
  \brief  Event on event flags set (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags that shall be set.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_SET_DISABLE))
extern void EvrLosEventFlagsSet (uint32_t ef_id, uint32_t flags);
#else
#define EvrLosEventFlagsSet(ef_id, flags)
#endif

/**
  \brief  Event on successful event flags set (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  event_flags   event flags after setting.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_SET_DONE_DISABLE))
extern void EvrLosEventFlagsSetDone (uint32_t ef_id, uint32_t event_flags);
#else
#define EvrLosEventFlagsSetDone(ef_id, event_flags)
#endif

/**
  \brief  Event on event flags clear (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags that shall be cleared.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CLEAR_DISABLE))
extern void EvrLosEventFlagsClear (uint32_t ef_id, uint32_t flags);
#else
#define EvrLosEventFlagsClear(ef_id, flags)
#endif

/**
  \brief  Event on successful event flags clear (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  event_flags   event flags before clearing.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_CLEAR_DONE_DISABLE))
extern void EvrLosEventFlagsClearDone (uint32_t ef_id, uint32_t event_flags);
#else
#define EvrLosEventFlagsClearDone(ef_id, event_flags)
#endif

/**
  \brief  Event on event flags retrieve (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  event_flags   current event flags.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_GET_DISABLE))
extern void EvrLosEventFlagsGet (uint32_t ef_id, uint32_t event_flags);
#else
#define EvrLosEventFlagsGet(ef_id, event_flags)
#endif

/**
  \brief  Event on wait for event flags (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_DISABLE))
extern void EvrLosEventFlagsWait (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);
#else
#define EvrLosEventFlagsWait(ef_id, flags, options, timeout)
#endif

/**
  \brief  Event on pending wait for event flags (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_PENDING_DISABLE))
extern void EvrLosEventFlagsWaitPending (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);
#else
#define EvrLosEventFlagsWaitPending(ef_id, flags, options, timeout)
#endif

/**
  \brief  Event on wait timeout for event flags (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_TIMEOUT_DISABLE))
extern void EvrLosEventFlagsWaitTimeout (uint32_t ef_id);
#else
#define EvrLosEventFlagsWaitTimeout(ef_id)
#endif

/**
  \brief  Event on successful wait for event flags (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
  \param[in]  event_flags   event flags before clearing or 0 if specified flags have not been set.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_COMPLETED_DISABLE))
extern void EvrLosEventFlagsWaitCompleted (uint32_t ef_id, uint32_t flags, uint32_t options, uint32_t event_flags);
#else
#define EvrLosEventFlagsWaitCompleted(ef_id, flags, options, event_flags)
#endif

/**
  \brief  Event on unsuccessful wait for event flags (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
  \param[in]  flags         flags to wait for.
  \param[in]  options       flags options (osFlagsXxxx).
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_WAIT_NOT_COMPLETED_DISABLE))
extern void EvrLosEventFlagsWaitNotCompleted (uint32_t ef_id, uint32_t flags, uint32_t options);
#else
#define EvrLosEventFlagsWaitNotCompleted(ef_id, flags, options)
#endif

/**
  \brief  Event on event flags delete (API)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_DELETE_DISABLE))
extern void EvrLosEventFlagsDelete (uint32_t ef_id);
#else
#define EvrLosEventFlagsDelete(ef_id)
#endif

/**
  \brief  Event on successful event flags delete (Op)
  \param[in]  ef_id         event flags ID obtained by \ref LOS_EventInit.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_EVFLAGS != 0) && !defined(EVR_LOS_EVENT_FLAGS_DESTROYED_DISABLE))
extern void EvrLosEventFlagsDestroyed (uint32_t ef_id);
#else
#define EvrLosEventFlagsDestroyed(ef_id)
#endif


//  ==== Mutex Events ====

/**
  \brief  Event on mutex error (Error)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate or NULL when ID is unknown.
  \param[in]  status    extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ERROR_DISABLE))
extern void EvrLosMutexError (uint32_t mutex_id, int32_t status);
#else
#define EvrLosMutexError(mutex_id, status)
#endif

/**
  \brief  Event on mutex create and initialize (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate or NULL when ID is unknown.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_NEW_DISABLE))
extern void EvrLosMutexNew (uint32_t mutex_id);
#else
#define EvrLosMutexNew(mutex_id)
#endif

/**
  \brief  Event on successful mutex create (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  name      pointer to mutex object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_CREATED_DISABLE))
extern void EvrLosMutexCreated (uint32_t mutex_id, const char *name);
#else
#define EvrLosMutexCreated(mutex_id, name)
#endif

/**
  \brief  Event on mutex name retrieve (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  name      pointer to mutex object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_GET_NAME_DISABLE))
extern void EvrLosMutexGetName (uint32_t mutex_id, const char *name);
#else
#define EvrLosMutexGetName(mutex_id, name)
#endif

/**
  \brief  Event on mutex acquire (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  timeout   \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_DISABLE))
extern void EvrLosMutexAcquire (uint32_t mutex_id, uint32_t timeout);
#else
#define EvrLosMutexAcquire(mutex_id, timeout)
#endif

/**
  \brief  Event on pending mutex acquire (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  timeout   \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_PENDING_DISABLE))
extern void EvrLosMutexAcquirePending (uint32_t mutex_id, uint32_t timeout);
#else
#define EvrLosMutexAcquirePending(mutex_id, timeout)
#endif

/**
  \brief  Event on mutex acquire timeout (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRE_TIMEOUT_DISABLE))
extern void EvrLosMutexAcquireTimeout (uint32_t mutex_id);
#else
#define EvrLosMutexAcquireTimeout(mutex_id)
#endif

/**
  \brief  Event on successful mutex acquire (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  lock      current number of times mutex object is locked.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_ACQUIRED_DISABLE))
extern void EvrLosMutexAcquired (uint32_t mutex_id, uint32_t lock);
#else
#define EvrLosMutexAcquired(mutex_id, lock)
#endif

/**
  \brief  Event on unsuccessful mutex acquire (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_NOT_ACQUIRED_DISABLE))
extern void EvrLosMutexNotAcquired (uint32_t mutex_id);
#else
#define EvrLosMutexNotAcquired(mutex_id)
#endif

/**
  \brief  Event on mutex release (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_RELEASE_DISABLE))
extern void EvrLosMutexRelease (uint32_t mutex_id);
#else
#define EvrLosMutexRelease(mutex_id)
#endif

/**
  \brief  Event on successful mutex release (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  lock      current number of times mutex object is locked.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_RELEASED_DISABLE))
extern void EvrLosMutexReleased (uint32_t mutex_id, uint32_t lock);
#else
#define EvrLosMutexReleased(mutex_id, lock)
#endif

/**
  \brief  Event on mutex owner retrieve (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
  \param[in]  TASK_id TASK ID obtained by  \ref LOS_TaskCreate or \ref LOS_CurTaskIDGet.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_GET_OWNER_DISABLE))
extern void EvrLosMutexGetOwner (uint32_t mutex_id, uint32_t TASK_id);
#else
#define EvrLosMutexGetOwner(mutex_id, TASK_id)
#endif

/**
  \brief  Event on mutex delete (API)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_DELETE_DISABLE))
extern void EvrLosMutexDelete (uint32_t mutex_id);
#else
#define EvrLosMutexDelete(mutex_id)
#endif

/**
  \brief  Event on successful mutex delete (Op)
  \param[in]  mutex_id  mutex ID obtained by \ref LOS_MuxCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MUTEX != 0) && !defined(EVR_LOS_MUTEX_DESTROYED_DISABLE))
extern void EvrLosMutexDestroyed (uint32_t mutex_id);
#else
#define EvrLosMutexDestroyed(mutex_id)
#endif


//  ==== Semaphore Events ====

/**
  \brief  Event on semaphore error (Error)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ERROR_DISABLE))
extern void EvrLosSemaphoreError (uint32_t semaphore_id, int32_t status);
#else
#define EvrLosSemaphoreError(semaphore_id, status)
#endif

/**
  \brief  Event on semaphore create and initialize (API)
  \param[in]  max_count     maximum number of available tokens.
  \param[in]  initial_count initial number of available tokens.
  \param[in]  semaphore_id  semphore id.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_NEW_DISABLE))
extern void EvrLosSemaphoreNew (uint32_t max_count, uint32_t initial_count, uint32_t semaphore_id);
#else
#define EvrLosSemaphoreNew(max_count, initial_count, semaphore_id)
#endif

/**
  \brief  Event on successful semaphore create (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  name          pointer to semaphore object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_CREATED_DISABLE))
extern void EvrLosSemaphoreCreated (uint32_t semaphore_id, const char *name);
#else
#define EvrLosSemaphoreCreated(semaphore_id, name)
#endif

/**
  \brief  Event on semaphore name retrieve (API)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  name          pointer to semaphore object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_GET_NAME_DISABLE))
extern void EvrLosSemaphoreGetName (uint32_t semaphore_id, const char *name);
#else
#define EvrLosSemaphoreGetName(semaphore_id, name)
#endif

/**
  \brief  Event on semaphore acquire (API)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_DISABLE))
extern void EvrLosSemaphoreAcquire (uint32_t semaphore_id, uint32_t timeout);
#else
#define EvrLosSemaphoreAcquire(semaphore_id, timeout)
#endif

/**
  \brief  Event on pending semaphore acquire (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_PENDING_DISABLE))
extern void EvrLosSemaphoreAcquirePending (uint32_t semaphore_id, uint32_t timeout);
#else
#define EvrLosSemaphoreAcquirePending(semaphore_id, timeout)
#endif

/**
  \brief  Event on semaphore acquire timeout (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRE_TIMEOUT_DISABLE))
extern void EvrLosSemaphoreAcquireTimeout (uint32_t semaphore_id);
#else
#define EvrLosSemaphoreAcquireTimeout(semaphore_id)
#endif

/**
  \brief  Event on successful semaphore acquire (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  tokens        number of available tokens.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_ACQUIRED_DISABLE))
extern void EvrLosSemaphoreAcquired (uint32_t semaphore_id, uint32_t tokens);
#else
#define EvrLosSemaphoreAcquired(semaphore_id, tokens)
#endif

/**
  \brief  Event on unsuccessful semaphore acquire (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_NOT_ACQUIRED_DISABLE))
extern void EvrLosSemaphoreNotAcquired (uint32_t semaphore_id);
#else
#define EvrLosSemaphoreNotAcquired(semaphore_id)
#endif

/**
  \brief  Event on semaphore release (API)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_RELEASE_DISABLE))
extern void EvrLosSemaphoreRelease (uint32_t semaphore_id);
#else
#define EvrLosSemaphoreRelease(semaphore_id)
#endif

/**
  \brief  Event on successful semaphore release (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  tokens        number of available tokens.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_RELEASED_DISABLE))
extern void EvrLosSemaphoreReleased (uint32_t semaphore_id, uint32_t tokens);
#else
#define EvrLosSemaphoreReleased(semaphore_id, tokens)
#endif

/**
  \brief  Event on semaphore token count retrieval (API)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
  \param[in]  count         current number of available tokens.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_GET_COUNT_DISABLE))
extern void EvrLosSemaphoreGetCount (uint32_t semaphore_id, uint32_t count);
#else
#define EvrLosSemaphoreGetCount(semaphore_id, count)
#endif

/**
  \brief  Event on semaphore delete (API)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_DELETE_DISABLE))
extern void EvrLosSemaphoreDelete (uint32_t semaphore_id);
#else
#define EvrLosSemaphoreDelete(semaphore_id)
#endif

/**
  \brief  Event on successful semaphore delete (Op)
  \param[in]  semaphore_id  semaphore ID obtained by \ref OsSemCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_SEMAPHORE != 0) && !defined(EVR_LOS_SEMAPHORE_DESTROYED_DISABLE))
extern void EvrLosSemaphoreDestroyed (uint32_t semaphore_id);
#else
#define EvrLosSemaphoreDestroyed(semaphore_id)
#endif


//  ==== Memory Pool Events ====

/**
  \brief  Event on memory pool error (Error)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ERROR_DISABLE))
extern void EvrLosMemoryPoolError (uint32_t mp_id, int32_t status);
#else
#define EvrLosMemoryPoolError(mp_id, status)
#endif

/**
  \brief  Event on memory pool create and initialize (API)
  \param[in]  block_count   maximum number of memory blocks in memory pool.
  \param[in]  block_size    memory block size in bytes.
  \param[in]  attr          memory pool attributes; NULL: default values.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_NEW_DISABLE))
extern void EvrLosMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr);
#else
#define EvrLosMemoryPoolNew(block_count, block_size, attr)
#endif

/**
  \brief  Event on successful memory pool create (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  name          pointer to memory pool object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_CREATED_DISABLE))
extern void EvrLosMemoryPoolCreated (uint32_t mp_id, const char *name);
#else
#define EvrLosMemoryPoolCreated(mp_id, name)
#endif

/**
  \brief  Event on memory pool name retrieve (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  name          pointer to memory pool object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_NAME_DISABLE))
extern void EvrLosMemoryPoolGetName (uint32_t mp_id, const char *name);
#else
#define EvrLosMemoryPoolGetName(mp_id, name)
#endif

/**
  \brief  Event on memory pool allocation (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_DISABLE))
extern void EvrLosMemoryPoolAlloc (uint32_t mp_id, uint32_t timeout);
#else
#define EvrLosMemoryPoolAlloc(mp_id, timeout)
#endif

/**
  \brief  Event on pending memory pool allocation (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_PENDING_DISABLE))
extern void EvrLosMemoryPoolAllocPending (uint32_t mp_id, uint32_t timeout);
#else
#define EvrLosMemoryPoolAllocPending(mp_id, timeout)
#endif

/**
  \brief  Event on memory pool allocation timeout (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_TIMEOUT_DISABLE))
extern void EvrLosMemoryPoolAllocTimeout (uint32_t mp_id);
#else
#define EvrLosMemoryPoolAllocTimeout(mp_id)
#endif

/**
  \brief  Event on successful memory pool allocation (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  block         address of the allocated memory block.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOCATED_DISABLE))
extern void EvrLosMemoryPoolAllocated (uint32_t mp_id, void *block);
#else
#define EvrLosMemoryPoolAllocated(mp_id, block)
#endif

/**
  \brief  Event on unsuccessful memory pool allocation (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_ALLOC_FAILED_DISABLE))
extern void EvrLosMemoryPoolAllocFailed (uint32_t mp_id);
#else
#define EvrLosMemoryPoolAllocFailed(mp_id)
#endif

/**
  \brief  Event on memory pool free (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  block         address of the allocated memory block to be returned to the memory pool.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_FREE_DISABLE))
extern void EvrLosMemoryPoolFree (uint32_t mp_id, void *block);
#else
#define EvrLosMemoryPoolFree(mp_id, block)
#endif

/**
  \brief  Event on successful memory pool free (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  block         address of the allocated memory block to be returned to the memory pool.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DEALLOCATED_DISABLE))
extern void EvrLosMemoryPoolDeallocated (uint32_t mp_id, void *block);
#else
#define EvrLosMemoryPoolDeallocated(mp_id, block)
#endif

/**
  \brief  Event on unsuccessful memory pool free (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  block         address of the allocated memory block to be returned to the memory pool.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_FREE_FAILED_DISABLE))
extern void EvrLosMemoryPoolFreeFailed (uint32_t mp_id, void *block);
#else
#define EvrLosMemoryPoolFreeFailed(mp_id, block)
#endif

/**
  \brief  Event on memory pool capacity retrieve (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  capacity      maximum number of memory blocks.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_CAPACITY_DISABLE))
extern void EvrLosMemoryPoolGetCapacity (uint32_t mp_id, uint32_t capacity);
#else
#define EvrLosMemoryPoolGetCapacity(mp_id, capacity)
#endif

/**
  \brief  Event on memory pool block size retrieve (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  block_size    memory block size in bytes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_BLOCK_SZIE_DISABLE))
extern void EvrLosMemoryPoolGetBlockSize (uint32_t mp_id, uint32_t block_size);
#else
#define EvrLosMemoryPoolGetBlockSize(mp_id, block_size)
#endif

/**
  \brief  Event on used memory pool blocks retrieve (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  count         number of memory blocks used.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_COUNT_DISABLE))
extern void EvrLosMemoryPoolGetCount (uint32_t mp_id, uint32_t count);
#else
#define EvrLosMemoryPoolGetCount(mp_id, count)
#endif

/**
  \brief  Event on available memory pool blocks retrieve (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
  \param[in]  space         number of memory blocks available.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_GET_SPACE_DISABLE))
extern void EvrLosMemoryPoolGetSpace (uint32_t mp_id, uint32_t space);
#else
#define EvrLosMemoryPoolGetSpace(mp_id, space)
#endif

/**
  \brief  Event on memory pool delete (API)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DELETE_DISABLE))
extern void EvrLosMemoryPoolDelete (uint32_t mp_id);
#else
#define EvrLosMemoryPoolDelete(mp_id)
#endif

/**
  \brief  Event on successful memory pool delete (Op)
  \param[in]  mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MEMPOOL != 0) && !defined(EVR_LOS_MEMORY_POOL_DESTROYED_DISABLE))
extern void EvrLosMemoryPoolDestroyed (uint32_t mp_id);
#else
#define EvrLosMemoryPoolDestroyed(mp_id)
#endif


//  ==== Message Queue Events ====

/**
  \brief  Event on message queue error (Error)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate or NULL when ID is unknown.
  \param[in]  status        extended execution status.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_ERROR_DISABLE))
extern void EvrLosMessageQueueError (uint32_t mq_id, int32_t status);
#else
#define EvrLosMessageQueueError(mq_id, status)
#endif

/**
  \brief  Event on message queue create and initialization (API)
  \param[in]  msg_count     maximum number of messages in queue.
  \param[in]  msg_size      maximum message size in bytes.
  \param[in]  mq_id         message queue ID obtained; NULL: default values.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NEW_DISABLE))
extern void EvrLosMessageQueueNew (uint32_t msg_count, uint32_t msg_size, uint32_t mq_id);
#else
#define EvrLosMessageQueueNew(msg_count, msg_size, mq_id)
#endif

/**
  \brief  Event on successful message queue create (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  name          pointer to message queue object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_CREATED_DISABLE))
extern void EvrLosMessageQueueCreated (uint32_t mq_id, const char *name);
#else
#define EvrLosMessageQueueCreated(mq_id, name)
#endif

/**
  \brief  Event on message queue name retrieve(API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  name          pointer to message queue object name.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_NAME_DISABLE))
extern void EvrLosMessageQueueGetName (uint32_t mq_id, const char *name);
#else
#define EvrLosMessageQueueGetName(mq_id, name)
#endif

/**
  \brief  Event on message put (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer with message to put into a queue.
  \param[in]  msg_prio      message priority.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_DISABLE))
extern void EvrLosMessageQueuePut (uint32_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);
#else
#define EvrLosMessageQueuePut(mq_id, msg_ptr, msg_prio, timeout)
#endif

/**
  \brief  Event on pending message put (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer with message to put into a queue.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_PENDING_DISABLE))
extern void EvrLosMessageQueuePutPending (uint32_t mq_id, const void *msg_ptr, uint32_t timeout);
#else
#define EvrLosMessageQueuePutPending(mq_id, msg_ptr, timeout)
#endif

/**
  \brief  Event on message put timeout (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_PUT_TIMEOUT_DISABLE))
extern void EvrLosMessageQueuePutTimeout (uint32_t mq_id);
#else
#define EvrLosMessageQueuePutTimeout(mq_id)
#endif

/**
  \brief  Event on pending message insert (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer with message to put into a queue.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_INSERT_PENDING_DISABLE))
extern void EvrLosMessageQueueInsertPending (uint32_t mq_id, const void *msg_ptr);
#else
#define EvrLosMessageQueueInsertPending(mq_id, msg_ptr)
#endif

/**
  \brief  Event on successful message insert (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer with message to put into a queue.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_INSERTED_DISABLE))
extern void EvrLosMessageQueueInserted (uint32_t mq_id, const void *msg_ptr);
#else
#define EvrLosMessageQueueInserted(mq_id, msg_ptr)
#endif

/**
  \brief  Event on unsuccessful message insert (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer with message to put into a queue.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NOT_INSERTED_DISABLE))
extern void EvrLosMessageQueueNotInserted (uint32_t mq_id, const void *msg_ptr);
#else
#define EvrLosMessageQueueNotInserted(mq_id, msg_ptr)
#endif

/**
  \brief  Event on message get (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer for message to get from a queue.
  \param[in]  msg_prio      message priority.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_DISABLE))
extern void EvrLosMessageQueueGet (uint32_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);
#else
#define EvrLosMessageQueueGet(mq_id, msg_ptr, msg_prio, timeout)
#endif

/**
  \brief  Event on pending message get (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer for message to get from a queue.
  \param[in]  timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_PENDING_DISABLE))
extern void EvrLosMessageQueueGetPending (uint32_t mq_id, void *msg_ptr, uint32_t timeout);
#else
#define EvrLosMessageQueueGetPending(mq_id, msg_ptr, timeout)
#endif

/**
  \brief  Event on message get timeout (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_TIMEOUT_DISABLE))
extern void EvrLosMessageQueueGetTimeout (uint32_t mq_id);
#else
#define EvrLosMessageQueueGetTimeout(mq_id)
#endif

/**
  \brief  Event on successful message get (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer for message to get from a queue.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RETRIEVED_DISABLE))
extern void EvrLosMessageQueueRetrieved (uint32_t mq_id, void *msg_ptr);
#else
#define EvrLosMessageQueueRetrieved(mq_id, msg_ptr)
#endif

/**
  \brief  Event on unsuccessful message get (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_ptr       pointer to buffer for message to get from a queue.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_NOT_RETRIEVED_DISABLE))
extern void EvrLosMessageQueueNotRetrieved (uint32_t mq_id, void *msg_ptr);
#else
#define EvrLosMessageQueueNotRetrieved(mq_id, msg_ptr)
#endif

/**
  \brief  Event on message queue capacity retrieve (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  capacity      maximum number of messages.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_CAPACITY_DISABLE))
extern void EvrLosMessageQueueGetCapacity (uint32_t mq_id, uint32_t capacity);
#else
#define EvrLosMessageQueueGetCapacity(mq_id, capacity)
#endif

/**
  \brief  Event on message queue message size retrieve (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  msg_size      maximum message size in bytes.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_MSG_SIZE_DISABLE))
extern void EvrLosMessageQueueGetMsgSize (uint32_t mq_id, uint32_t msg_size);
#else
#define EvrLosMessageQueueGetMsgSize(mq_id, msg_size)
#endif

/**
  \brief  Event on message queue message count retrieve (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  count         number of queued messages.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_COUNT_DISABLE))
extern void EvrLosMessageQueueGetCount (uint32_t mq_id, uint32_t count);
#else
#define EvrLosMessageQueueGetCount(mq_id, count)
#endif

/**
  \brief  Event on message queue message slots retrieve (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
  \param[in]  space         number of available slots for messages.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_GET_SPACE_DISABLE))
extern void EvrLosMessageQueueGetSpace (uint32_t mq_id, uint32_t space);
#else
#define EvrLosMessageQueueGetSpace(mq_id, space)
#endif

/**
  \brief  Event on message queue reset (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RESET_DISABLE))
extern void EvrLosMessageQueueReset (uint32_t mq_id);
#else
#define EvrLosMessageQueueReset(mq_id)
#endif

/**
  \brief  Event on successful message queue reset (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_RESET_DONE_DISABLE))
extern void EvrLosMessageQueueResetDone (uint32_t mq_id);
#else
#define EvrLosMessageQueueResetDone(mq_id)
#endif

/**
  \brief  Event on message queue delete (API)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_DELETE_DISABLE))
extern void EvrLosMessageQueueDelete (uint32_t mq_id);
#else
#define EvrLosMessageQueueDelete(mq_id)
#endif

/**
  \brief  Event on successful message queue delete (Op)
  \param[in]  mq_id         message queue ID obtained by \ref LOS_QueueCreate.
*/
#if (!defined(EVR_LOS_DISABLE) && (OS_EVR_MSGQUEUE != 0) && !defined(EVR_LOS_MESSAGE_QUEUE_DESTROYED_DISABLE))
extern void EvrLosMessageQueueDestroyed (uint32_t mq_id);
#else
#define EvrLosMessageQueueDestroyed(mq_id)
#endif


#endif  // _LOS_SYSVIEW_EVR_H_
