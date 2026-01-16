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

#include "nuttx/semaphore.h"
#include "nuttx/clock.h"

int gra_sem_init(sem_t *sem, int pshared, unsigned int value)
{
    return nxsem_init(sem, pshared, value);
}

int gra_sem_wait(sem_t *sem)
{
    return nxsem_wait(sem);
}

int gra_sem_timedwait(sem_t *sem, unsigned int ms)
{
    return nxsem_tickwait(sem, USEC2TICK(ms * 1000));
}

int gra_sem_wait_uninterruptible(sem_t *sem)
{
    return nxsem_wait_uninterruptible(sem);
}

int gra_sem_post(sem_t *sem)
{
    return nxsem_post(sem);
}

int gra_sem_get_value(sem_t *sem, int *value)
{
    return nxsem_get_value(sem, value);
}

int gra_sem_destroy (sem_t *sem)
{
    return nxsem_destroy(sem);
}

int gra_sem_trywait(sem_t *sem)
{
    return nxsem_trywait(sem);
}

int gra_sem_reset(sem_t *sem, unsigned int value)
{
    return nxsem_reset(sem, value);
}
