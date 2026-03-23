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
#include "vg_lite_kernel.h"
#include "vg_lite_platform.h"
#include "vg_lite_hw.h"
#include "vg_lite_hal.h"
#ifdef RTOS
#include "gra_sem.h"
#endif

#if (GPU_TASK_COUNT == 0)
#error "GPU_TASK_COUNT can not be 0."
#endif
#if (GPU_TASK_COUNT > 1) && !defined(RTOS)
#error "GPU multi-task support needs RTOS=1"
#endif

#define GPU_TIMEOUT                             (500) /* ms */

#define GPU_CONTIGUOUS_SIZE                     (GPU_TSBUF_SIZE + (GPU_CMDBUF_SIZE * 2 * GPU_TASK_COUNT))

#if defined(DDR_ENABLE)
#define GPU_MEM_LOC                             DDR_BSS_LOC
#elif defined(PSRAM_ENABLE)
#define GPU_MEM_LOC                             PSRAM_NC_BSS_LOC
#else
#define GPU_MEM_LOC
#endif

static uint8_t gpu_contiguous_mem[VG_SYSTEM_RESERVE_COUNT][GPU_CONTIGUOUS_SIZE] ALIGNED(64) GPU_MEM_LOC;
#if (GPU_HEAP_SIZE > 0x10000)
static uint8_t gpu_heap_mem[GPU_HEAP_SIZE] GPU_MEM_LOC;
#else
static uint8_t gpu_heap_mem[GPU_HEAP_SIZE] PSRAM_NC_BSS_LOC;
#endif
static heap_handle_t gpu_heap = NULL;

struct gpu_task_t {
    uint32_t tid;
    void *task;
};
static struct gpu_task_t gpu_tasks[GPU_TASK_COUNT];

#ifdef RTOS
static sem_t gpu_done_sem;
static sem_t gpu_lock;
#else
static bool gpu_done = false;
#endif
#ifdef GPU_MHZ
static uint32_t gpu_freq = GPU_MHZ;
#else
static uint32_t gpu_freq = 200; /* default 200Mhz */
#endif
static bool gpu_hal_inited = false;

#define IS_AXI_BUS_ERR(x)                       ((x) & (1 << 31))
#define IS_AXI_WR_ERR(x)                        ((x) & (3 << 8))
static uint32_t gpu_intr_status = 0;

static void gpu_irq_handler(void)
{
#ifdef GPU_IRQ_TEST
    set_soft_break_point();
#endif
    gpu_intr_status = vg_lite_hal_peek(VG_LITE_INTR_STATUS);
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
    return heap_malloc(gpu_heap, size);
}

void *gpu_hal_realloc(void *buf, uint32_t size)
{
    if (!gpu_heap) {
        gpu_heap = heap_register(gpu_heap_mem, GPU_HEAP_SIZE);
        ASSERT(gpu_heap != NULL, "%s: gpu heap register failed.", __func__);
    }
    return heap_realloc(gpu_heap, buf, size);
}

void gpu_hal_free(void *mem)
{
    if (!gpu_heap || !mem)
        return;
    heap_free(gpu_heap, mem);
}

int gpu_hal_os_lock(void)
{
#ifdef RTOS
    static bool gpu_lock_init = false;
    if (!gpu_lock_init) {
        gra_sem_init(&gpu_lock, 0, 1);
        gpu_lock_init = true;
    }
    return gra_sem_wait(&gpu_lock);
#else
    return 0;
#endif
}

int gpu_hal_os_unlock(void)
{
#ifdef RTOS
    return gra_sem_post(&gpu_lock);
#else
    return 0;
#endif
}

int gpu_hal_os_set_tls(void *task)
{
#if defined(RTOS) && (GPU_TASK_COUNT > 1)
    uint32_t tid = (uint32_t)osThreadGetId();
    bool set = false;
    for (int i = 0; i < GPU_TASK_COUNT; i++) {
        if (tid == gpu_tasks[i].tid) {
            gpu_tasks[i].task = task;
            if (task == NULL)
                gpu_tasks[i].tid = 0;
            set = true;
            break;
        }
    }
    gpu_hal_os_lock();
    if (!set && task) {
        for (int i = 0; i < GPU_TASK_COUNT; i++) {
            if (gpu_tasks[i].tid == 0) {
                gpu_tasks[i].tid = tid;
                gpu_tasks[i].task = task;
                set = true;
                break;
            }
        }
    }
    gpu_hal_os_unlock();
    return set ? 0 : -1 ;
#else
    gpu_tasks[0].task = task;
    return 0;
#endif
}

void *gpu_hal_os_get_tls(void)
{
#if defined(RTOS) && (GPU_TASK_COUNT > 1)
    uint32_t tid = (uint32_t)osThreadGetId();
    for (int i = 0; i < GPU_TASK_COUNT; i++) {
        if (tid == gpu_tasks[i].tid) {
            return gpu_tasks[i].task;
        }
    }
    return NULL;
#else
    return gpu_tasks[0].task;
#endif
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
    int ret = 0;
    bool timeout = false;
    uint32_t axi_status = 0;
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
    axi_status = vg_lite_hal_peek(VG_LITE_AXI_STATUS);
    if (IS_AXI_BUS_ERR(gpu_intr_status) || IS_AXI_WR_ERR(axi_status)) {
        TRACE(0, "GPU BUS ERROR! intrStatus=%08x, axiStatus=%08x",
                 gpu_intr_status, axi_status);
        ret = -1;
    }
    if (timeout) {
        TRACE(0, "GPU IRQ TIMEOUT!");
        ret = -1;
    }
    gpu_intr_status = 0;
    return ret;
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

void gpu_hal_cache_sync_all(void)
{
    hal_cache_sync_invalidate_all(HAL_CACHE_ID_D_CACHE);
}

uint32_t gpu_hal_buf_addr(uint32_t addr)
{
#if defined(PSRAM_BASE) && defined(PSRAM_SIZE) && defined(PSRAM_NC_BASE)
    if (addr >= PSRAM_BASE && addr < (PSRAM_BASE+PSRAM_SIZE))
        return (addr - PSRAM_BASE + PSRAM_NC_BASE);
#endif
#ifdef ADDR_CPU_TO_DEV
    return ADDR_CPU_TO_DEV(addr);
#else
    return addr;
#endif
}

