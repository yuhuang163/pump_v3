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
#include <string.h>
#include <stdlib.h>
#include "hal_sec_eng.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_timer.h"
#include "hal_dma.h"
#include "sec_lock.h"

#ifdef RTOS
#include "cmsis_os2.h"

#define CHECK_OS_STATE                              \
    do {                                            \
        if (osKernelGetState() < osKernelRunning)   \
            return;                                 \
    } while (0)

static osSemaphoreId_t sec_sem = NULL;
#endif

SEC_LOCK_DEC(hal_lock);
static volatile int work_done_flag;

static void sec_sem_init(void)
{
#ifdef RTOS
    CHECK_OS_STATE;
    if (sec_sem == NULL) {
        sec_sem = osSemaphoreNew(1, 0, NULL);
    }
#endif
}

static void sec_sem_wait(void)
{
    uint32_t timeout = MS_TO_TICKS(2000), time;

#ifdef RTOS
    if (osKernelGetState() == osKernelRunning && sec_sem) {
        osSemaphoreAcquire(sec_sem, osWaitForever);
    }
#endif

    /*
        1. Reason for adding lock: to prevent BCM interrupt from being generated
           after wakeup and sleeping in its callback, causing busy() to get
           stuck later
        2. Reason for wakeup first and sleep later: if wakeup is not added, busy()
           may get stuck; because wakeup is added in the front to save power and
           sleep is needed later
    */
    uint32_t lock = int_lock();
    if (work_done_flag) {
        hal_se_wakeup();
    }

    time = hal_sys_timer_get();
    while (hal_se_aes_busy() || hal_se_hash_busy()) {
        if (hal_sys_timer_get() - time >= timeout) {
            int_unlock(lock);
            ASSERT(false, "%s, Waiting for BCM idle timeout", __func__);
            break;
        }
    }

    if (work_done_flag) {
        hal_se_sleep();
    }
    int_unlock(lock);
}

static void sec_sem_release(void)
{
#ifdef RTOS
    CHECK_OS_STATE;
    if (sec_sem) {
        osSemaphoreRelease(sec_sem);
    }
#endif
}

static void sec_work_done(void *buf, uint32_t len, enum HAL_SE_DONE_ERR_T err)
{
    work_done_flag = 1;
    sec_sem_release();
    hal_se_sleep();
}

enum HAL_SE_RET_T sec_alt_open(void)
{
    enum HAL_SE_RET_T ret;

    sec_sem_init();

    if (!hal_lock) {
        sec_alt_lock_init(&hal_lock);
    }

    sec_alt_lock(hal_lock);
    ret = hal_se_open();
    hal_se_sleep();
    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_close(void)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    hal_se_wakeup();
    ret = hal_se_close();
    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_aes_encrypt(struct HAL_SE_AES_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    cfg->done_hdlr = sec_work_done;
    hal_se_wakeup();
    work_done_flag = 0;

    ret = hal_se_aes_encrypt(cfg);
    if (ret == HAL_SE_OK) {
        sec_sem_wait();
    }

    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_aes_decrypt(struct HAL_SE_AES_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    cfg->done_hdlr = sec_work_done;
    hal_se_wakeup();
    work_done_flag = 0;

    ret = hal_se_aes_decrypt(cfg);
    if (ret == HAL_SE_OK) {
        sec_sem_wait();
    }

    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_hash_init(enum HAL_SE_HASH_MODE_T mode)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    hal_se_wakeup();

    ret = hal_se_hash_init(mode);

    hal_se_sleep();
    sec_alt_unlock(hal_lock);
    return ret;
}

int sec_alt_hash_busy(void)
{
    int ret;
    sec_alt_lock(hal_lock);
    hal_se_wakeup();

    ret = hal_se_hash_busy();

    hal_se_sleep();
    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_hash_update(struct HAL_SE_HASH_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    cfg->done_hdlr = sec_work_done;
    hal_dma_sync_for_device((const void *)cfg->in, cfg->in_len);

    hal_se_wakeup();
    work_done_flag = 0;

    ret = hal_se_hash_update(cfg);
    if (ret == HAL_SE_OK) {
        sec_sem_wait();
    }

    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_hash_final(struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len)
{
    enum HAL_SE_RET_T ret;

    sec_alt_lock(hal_lock);
    cfg->done_hdlr = sec_work_done;
    hal_dma_sync_for_device((const void *)cfg->in, cfg->in_len);

    hal_se_wakeup();
    work_done_flag = 0;

    ret = hal_se_hash_final(cfg, total_in_len);
    if (ret == HAL_SE_OK) {
        sec_sem_wait();
    }

    sec_alt_unlock(hal_lock);
    return ret;
}

enum HAL_SE_RET_T sec_alt_hash_get_digest(void *out, uint32_t out_len, uint32_t *real_len)
{
    enum HAL_SE_RET_T ret;
    sec_alt_lock(hal_lock);
    hal_se_wakeup();

    ret = hal_se_hash_get_digest(out, out_len, real_len);

    uint32_t lock = int_lock();
    /* Prevent MCU from being stuck in interrupt by closing clk before
       bcm interrupt is generated */
    if (work_done_flag) {
        hal_se_sleep();
    }
    int_unlock(lock);
    sec_alt_unlock(hal_lock);
    return ret;
}
