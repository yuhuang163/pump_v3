/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

/**
 * GPU hardware interface for vglite driver.
 */
#include <stdio.h>
#include <string.h>
#include "cmsis_nvic.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_psc.h"
#include "hal_cmu.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_location.h"
#include "heap_api.h"
#include "gpu_port.h"
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "vg_lite_platform.h"
#include "vg_lite_hw.h"
#include "vg_lite_hal.h"
#ifdef RTOS
#include "gra_sem.h"
#endif

//#define GPU_L2CC_ENABLE

#define GPU_TIMEOUT                             (200) /* ms */

#define GPU_HEAP_SIZE                           (16*1024)

#ifndef GPU_CONTIGUOUS_SIZE
/* 64KB for Command buffer, 64~176KB for TS buffer */
#define GPU_CONTIGUOUS_SIZE                     (240*1024)
#endif

#if defined(DDR_ENABLE)
#define GPU_MEM_LOC                             DDR_BSS_LOC
#elif defined(PSRAM_ENABLE)
#define GPU_MEM_LOC                             PSRAM_NC_BSS_LOC
#else
#define GPU_MEM_LOC
#endif

static uint8_t gpu_contiguous_mem[VG_SYSTEM_RESERVE_COUNT][GPU_CONTIGUOUS_SIZE] ALIGNED(64) GPU_MEM_LOC;
static uint8_t gpu_heap_mem[GPU_HEAP_SIZE];
static heap_handle_t gpu_heap = NULL;

#ifdef RTOS
static sem_t gpu_done_sem;
#else
static bool gpu_done = false;
#endif
#ifdef GPU_MHZ
static uint32_t gpu_freq = GPU_MHZ;
#else
static uint32_t gpu_freq = 200; /* default 200Mhz */
#endif
static bool gpu_hal_inited = false;

#ifdef GPU_L2CC_ENABLE
#ifndef SYS_GPU_L2CC_BASE
#error "GPU do not support l2cache on this chip!"
#endif
/* reg_100 */
#define L2CC_CACHE_ENACR                        0x100
#define L2CC_CACHE_ENA                          (1 << 0)
/* reg_730 */
#define L2CC_CACHE_SYNCCR                       0x730
/* reg_f60 */
#define L2CC_CACHE_PREFETCHCR                   0xf60
#define L2CC_CACHE_PREFETCH_DBLF_ENA            (1 << 30)
#define L2CC_CACHE_PREFETCH_INCDBLF_ENA         (1 << 23)
/* reg_77c */
#define L2CC_CACHE_INVALIDCR                    0x77c

#define L2CC_WRITE32(reg, val)  \
        (*(volatile uint32_t *)(SYS_GPU_L2CC_BASE + reg) = val)
#define L2CC_READ32(reg) \
        (*(volatile uint32_t *)(SYS_GPU_L2CC_BASE + reg))

POSSIBLY_UNUSED
static void gpu_l2cc_cache_config(void)
{
    uint32_t val = 0;

    L2CC_WRITE32(L2CC_CACHE_INVALIDCR, 0xf);
    val = L2CC_READ32(L2CC_CACHE_INVALIDCR);

    while (val & 0xf){
        val = L2CC_READ32(L2CC_CACHE_INVALIDCR);
    }

    /* Enable L2CC for GPU */
    L2CC_WRITE32(L2CC_CACHE_ENACR, L2CC_CACHE_ENA);

    /* Set prefetch contorl register, double linefill and incr double linefill enable */
    val = L2CC_READ32(L2CC_CACHE_PREFETCHCR);
    val |= L2CC_CACHE_PREFETCH_DBLF_ENA;
    val |= L2CC_CACHE_PREFETCH_INCDBLF_ENA;
    L2CC_WRITE32(L2CC_CACHE_PREFETCHCR, val);
}

POSSIBLY_UNUSED
static void gpu_l2cc_invlid_all(void)
{
    uint32_t val = 0;

    /* invalid all entries in L2CC */
    L2CC_WRITE32(L2CC_CACHE_INVALIDCR, 0xffff);

    val = L2CC_READ32(L2CC_CACHE_INVALIDCR);
    while(val & 0xf){
        val = L2CC_READ32(L2CC_CACHE_INVALIDCR);
    }
}
#endif

static void gpu_irq_handler(void)
{
#ifdef GPU_IRQ_TEST
    set_soft_break_point();
#endif
    vg_lite_hal_peek(VG_LITE_INTR_STATUS);
#ifdef RTOS
    gra_sem_post(&gpu_done_sem);
#else
    gpu_done = true;
#endif
}

void gpu_hal_init(void)
{
    vg_module_parameters_t param;

    if (gpu_hal_inited)
        return;

    /* Enable GPU clock */
    hal_cmu_gpu_clock_enable(gpu_freq);

#ifdef GPU_L2CC_ENABLE
    gpu_l2cc_cache_config();
#endif

#ifdef RTOS
    gra_sem_init(&gpu_done_sem, 0, 0);
#endif

    param.register_mem_base = GPU_BASE;
    for (int i = 0; i < VG_SYSTEM_RESERVE_COUNT; i++) {
        param.gpu_mem_base[i] = 0;
        param.contiguous_mem_base[i] = gpu_contiguous_mem[i];
        param.contiguous_mem_size[i] = GPU_CONTIGUOUS_SIZE;
    }
    vg_lite_init_mem(&param);

    NVIC_SetVector(GPU_IRQn, (uint32_t)gpu_irq_handler);
    NVIC_SetPriority(GPU_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(GPU_IRQn);
    NVIC_EnableIRQ(GPU_IRQn);

    gpu_hal_inited = true;
}

void gpu_hal_exit(void)
{
    if (!gpu_hal_inited)
        return;

    gpu_hal_inited = false;

    NVIC_ClearPendingIRQ(GPU_IRQn);
    NVIC_DisableIRQ(GPU_IRQn);

    hal_cmu_gpu_clock_disable();

#ifdef RTOS
    gra_sem_destroy(&gpu_done_sem);
#endif
}

void gpu_hal_reset(void)
{
    gpu_hal_inited = false;

    NVIC_ClearPendingIRQ(GPU_IRQn);
    NVIC_DisableIRQ(GPU_IRQn);

    hal_cmu_gpu_clock_disable();

    hal_sys_timer_delay(10);

    hal_cmu_gpu_clock_enable(gpu_freq);

    NVIC_SetVector(GPU_IRQn, (uint32_t)gpu_irq_handler);
    NVIC_SetPriority(GPU_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(GPU_IRQn);
    NVIC_EnableIRQ(GPU_IRQn);

    gpu_hal_inited = true;
}

void *gpu_hal_malloc(uint32_t size)
{
    if (!gpu_heap) {
        gpu_heap = heap_register(gpu_heap_mem, GPU_HEAP_SIZE);
        ASSERT(gpu_heap != NULL, "%s: gpu heap register failed.", __func__);
    }
    return heap_malloc(gpu_heap, size );
}

void gpu_hal_free(void *mem)
{
    if (!gpu_heap || !mem)
        return;
    heap_free(gpu_heap, mem);
}

uint32_t gpu_hal_get_freqmhz(void)
{
    return gpu_freq;
}

void gpu_hal_set_freqmhz(uint32_t freqmhz)
{
    hal_cmu_gpu_set_mhz(freqmhz);
    gpu_freq = freqmhz;
}

void gpu_hal_enter_lpwr(void)
{
    hal_cmu_gpu_set_mhz(24);
    hal_cmu_gpu_sleep();
}

void gpu_hal_exit_lpwr(void)
{
    hal_cmu_gpu_wakeup();
    hal_cmu_gpu_set_mhz(gpu_freq);
}

int gpu_hal_wait_done(void)
{
    bool timeout = false;
#ifdef RTOS
    if (gra_sem_timedwait(&gpu_done_sem, GPU_TIMEOUT))
        timeout = true;
#else
    uint32_t start = hal_sys_timer_get();
    while (!gpu_done && TICKS_TO_MS(hal_sys_timer_get() - start) < GPU_TIMEOUT)
        hal_sys_timer_delay(1);
    if (TICKS_TO_MS(hal_sys_timer_get() - start) >= GPU_TIMEOUT)
        timeout = true;
    gpu_done = false;
#endif
    if (timeout) {
        TRACE(0, "GPU IRQ TIMEOUT!");
        return -1;
    }
    return 0;
}

/* rewrite for vglite */
void vg_lite_bus_error_handler()
{
    ASSERT(0, "GPU ERROR!");
}

void gpu_hal_cache_sync(void)
{
#ifdef PSRAM_ENABLE
    /* Only PSRAM cached buffer need cache sync */
    for (int i = 0; i < VG_SYSTEM_RESERVE_COUNT; i++) {
        if ((uint32_t)gpu_contiguous_mem[i] >= PSRAM_BASE &&
            (uint32_t)gpu_contiguous_mem[i] <= (PSRAM_BASE + PSRAM_SIZE)) {
            hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
            return;
        }
    }
#endif
}

