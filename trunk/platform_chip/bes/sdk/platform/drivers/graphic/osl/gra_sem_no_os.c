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

#include "hal_timer.h"
#include "hal_trace.h"
#include "gra_sem.h"

#ifndef EINVAL
#define EINVAL          1
#endif
#define OK              0

int gra_sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (!sem)
        return -EINVAL;

    *sem = value;
    return OK;
}

int gra_sem_wait(sem_t *sem)
{
    if (!sem)
        return -EINVAL;

    while (*(volatile sem_t *)sem <= 0) {
    }
    *sem -= 1;
    return OK;
}

int gra_sem_timedwait(sem_t *sem, unsigned int ms)
{
    unsigned int start = hal_sys_timer_get();

    if (!sem)
        return -EINVAL;

    while (*(volatile sem_t *)sem <= 0 && TICKS_TO_MS(hal_sys_timer_get() - start) < ms) {
    }
    if (*sem > 0) {
        *sem -= 1;
        return OK;
    }
    return -EINVAL;
}

int gra_sem_wait_uninterruptible(sem_t *sem)
{
    return gra_sem_wait(sem);
}

int gra_sem_post(sem_t *sem)
{
    if (!sem)
        return -EINVAL;

    *sem += 1;
    return OK;
}

int gra_sem_get_value(sem_t *sem, int *value)
{
    if (!sem || !value)
        return -EINVAL;

    *value = *sem;
    return OK;
}

int gra_sem_destroy(sem_t *sem)
{
    *sem = 0;
    return OK;
}

int gra_sem_trywait(sem_t *sem)
{
    return gra_sem_wait(sem);
}

int gra_sem_reset(sem_t *sem, unsigned int value)
{
    *sem = value;
    return OK;
}
