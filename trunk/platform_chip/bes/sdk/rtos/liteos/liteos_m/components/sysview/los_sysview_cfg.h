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
 * $Revision:   V5.5.0
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Configuration definitions
 *
 * -----------------------------------------------------------------------------
 */

#ifndef LOS_SYSVIEW_CONFIG_H_
#define LOS_SYSVIEW_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

//     <q>Memory Management
//     <i> Enables Memory Management event generation.
#ifndef OS_EVR_MEMORY
#define OS_EVR_MEMORY               1
#endif

//     <q>Kernel
//     <i> Enables Kernel event generation.
#ifndef OS_EVR_KERNEL
#define OS_EVR_KERNEL               1
#endif

//     <q>Thread
//     <i> Enables Thread event generation.
#ifndef OS_EVR_TASK
#define OS_EVR_TASK               1
#endif

//     <q>Generic Wait
//     <i> Enables Generic Wait event generation.
#ifndef OS_EVR_WAIT
#define OS_EVR_WAIT                 1
#endif

//     <q>Thread Flags
//     <i> Enables Thread Flags event generation.
#ifndef OS_EVR_THFLAGS
#define OS_EVR_THFLAGS              1
#endif

//     <q>Event Flags
//     <i> Enables Event Flags event generation.
#ifndef OS_EVR_EVFLAGS
#define OS_EVR_EVFLAGS              1
#endif

//     <q>Timer
//     <i> Enables Timer event generation.
#ifndef OS_EVR_TIMER
#define OS_EVR_TIMER                1
#endif

//     <q>Mutex
//     <i> Enables Mutex event generation.
#ifndef OS_EVR_MUTEX
#define OS_EVR_MUTEX                1
#endif

//     <q>Semaphore
//     <i> Enables Semaphore event generation.
#ifndef OS_EVR_SEMAPHORE
#define OS_EVR_SEMAPHORE            1
#endif

//     <q>Memory Pool
//     <i> Enables Memory Pool event generation.
#ifndef OS_EVR_MEMPOOL
#define OS_EVR_MEMPOOL              1
#endif

//     <q>Message Queue
//     <i> Enables Message Queue event generation.
#ifndef OS_EVR_MSGQUEUE
#define OS_EVR_MSGQUEUE             1
#endif

//------------- <<< end of configuration section >>> ---------------------------

#endif  // LOS_SYSVIEW_CONFIG_H_
