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

#include "cmsis.h"
#include "cmsis_os.h"

#include "gra_sem.h"

#define SEM_VALUE_MAX   128
#ifndef EINTR
#define EINTR           4
#endif
#ifndef EINVAL
#define EINVAL          1
#endif
#define OK              0

int gra_sem_init(sem_t *sem, int pshared, unsigned int value)
{
#ifdef KERNEL_RTX5
    if (in_isr())
        return -EINVAL;
#endif
    /* Verify that a semaphore was provided and the count is within the valid
     * range.
     */
    if (sem && value <= SEM_VALUE_MAX) {
        sem->id = osSemaphoreNew(SEM_VALUE_MAX, value, NULL);
        if (sem->id)
            return OK;
    }

    return -EINVAL;
}

int gra_sem_wait(sem_t *sem)
{
    if (sem && sem->id)
        return osSemaphoreWait(sem->id, osWaitForever);
    else
        return -EINVAL;
}

int gra_sem_timedwait(sem_t *sem, unsigned int ms)
{
    if (sem && sem->id)
        return osSemaphoreWait(sem->id, ms) ? 0 : -1;
    else
        return -EINVAL;
}

int gra_sem_wait_uninterruptible(sem_t *sem)
{
    int ret;

    do {
        /* Take the semaphore (perhaps waiting) */
        ret = gra_sem_wait(sem);
    } while (ret == -EINTR);

    return ret;
}

int gra_sem_post(sem_t *sem)
{
    if (sem && sem->id)
        return osSemaphoreRelease(sem->id);
    else
        return -EINVAL;
}

int gra_sem_get_value(sem_t *sem, int *value)
{
    if (!sem || !value)
        return -EINVAL;
    if (!sem->id)
        return -EINVAL;

    *value = (int)osSemaphoreGetCount(sem->id);
    return OK;
}

int gra_sem_destroy(sem_t *sem)
{
    /* Assure a valid semaphore is specified */
    if (sem && sem->id) {
        osSemaphoreDelete(sem->id);
        return OK;
    }
    return -EINVAL;
}

int gra_sem_trywait(sem_t *sem)
{
    if (sem && sem->id)
        return osSemaphoreWait(sem->id, 0);

    return -EINVAL;
}

int gra_sem_reset(sem_t *sem, unsigned int value)
{
    gra_sem_destroy(sem);
    return gra_sem_init(sem, 0, value);
}
