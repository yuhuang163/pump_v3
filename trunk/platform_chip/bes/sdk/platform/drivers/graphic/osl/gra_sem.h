/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifndef __GRA_SEM_H__
#define __GRA_SEM_H__

#ifdef GRA_SEM_CMSIS
#include "cmsis_os.h"

typedef struct {
    osSemaphoreId     id;
} sem_t;
#elif defined(GRA_SEM_NO_OS)
#define sem_t volatile int
#endif

/**
 * gra_sem_init - initializes the semaphore.
 *
 * @sem : Semaphore to be initialized
 * @pshared : Process sharing (not used)
 * @value : Semaphore initialization value
 *
 * Returns:zero (OK) if successful.  Otherwise, -1 (ERROR)
 */
int gra_sem_init(sem_t *sem, int pshared, unsigned int value);

/**
 * gra_sem_wait_uninterruptible - wrapped version of nxsem_wait(), which is
 *   uninterruptible and convenient for use.
 *
 * @sem : Semaphore descriptor.
 *
 * Returns:
 *   Zero(OK)  : On success
 *   EINVAL    : Invalid attempt to get the semaphore
 *   ECANCELED : May be returned if the thread is canceled while waiting.
 *
 */
int gra_sem_wait_uninterruptible(sem_t *sem);

/**
 * gra_sem_wait - attempts to lock the semaphore.
 *
 * @sem : Semaphore descriptor.
 *
 * Returns:
 *   zero (OK) if successful.  Otherwise, -1 (ERROR) is returned and
 *   the errno value is set appropriately.  Possible errno values include:
 */
int gra_sem_wait(sem_t *sem);

/**
 * gra_sem_post - release the semaphore.
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: zero (OK) if successful.  Otherwise, -1 (ERROR)
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 */
int gra_sem_post(sem_t *sem);

/**
 * gra_sem_destroy - destroy the semaphore.
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: zero (OK),  -1 (ERROR)
 */
int gra_sem_destroy(sem_t *sem);

/**
 * gra_sem_trywait - lighter weight version of sem_timedwait()
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: Zero (OK) is returned on success. A negated errno value is
 * returned on failure:
 *     -ETIMEDOUT is returned on the timeout condition.
 *     -ECANCELED may be returned if the thread is canceled while waiting.
 */
int gra_sem_trywait(sem_t *sem);

/**
 * gra_sem_reset - reset semaphore.
 *
 * @sem : Semaphore descriptor.
 */
int gra_sem_reset(sem_t *sem, unsigned int value);

/**
 * gra_sem_timedwait - attempts to lock the semaphore with timeout.
 *
 * @sem : Semaphore descriptor.
 * @ms  : Timeout in miliseconds.
 *
 * Returns: Zero (OK) , A negated errno value is returned on failure.
 */
int gra_sem_timedwait(sem_t *sem, unsigned int ms);

/**
 * gra_sem_getvalue - get the counter value of semaphore.
 */
int gra_sem_get_value(sem_t *sem, int *value);

#endif /* __GRA_SEM_H__ */

