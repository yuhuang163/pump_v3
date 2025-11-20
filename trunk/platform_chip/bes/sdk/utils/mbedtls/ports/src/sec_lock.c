/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "sec_lock.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif

int sec_alt_lock_init(sec_lock_t *lock)
{
#ifdef RTOS
    if (osKernelGetState() != osKernelRunning) {
        return -1;
    }
    if (!lock) {
        return -1;
    }

    osMutexAttr_t attr = { NULL,
                           osMutexRecursive | osMutexPrioInherit | osMutexRobust,
                           NULL, 0U
                         };
    osMutexId_t id = osMutexNew(&attr);
    if (id) {
        *lock = (void *)id;
        return 0;
    }
#endif

    return -1;
}

void sec_alt_lock_free(sec_lock_t *lock)
{
#ifdef RTOS
    if (osKernelGetState() != osKernelRunning) {
        return;
    }
    if (lock && *lock) {
        osMutexDelete((osMutexId_t)*lock);
        *lock = NULL;
    }
#endif
}

int sec_alt_lock(sec_lock_t lock)
{
#ifdef RTOS
    if (osKernelGetState() != osKernelRunning) {
        return -1;
    }
    if (!lock) {
        return -1;
    }
    return osMutexAcquire((osMutexId_t)lock, osWaitForever);
#else
    return 0;
#endif
}

int sec_alt_unlock(sec_lock_t lock)
{
#ifdef RTOS
    if (osKernelGetState() != osKernelRunning) {
        return -1;
    }
    if (!lock) {
        return -1;
    }
    return osMutexRelease((osMutexId_t)lock);
#else
    return 0;
#endif
}
