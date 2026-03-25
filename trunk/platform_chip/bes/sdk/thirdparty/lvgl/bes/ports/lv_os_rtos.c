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

/*********************
 *      INCLUDES
 *********************/
#include "src/osal/lv_os_private.h"

#if LV_USE_OS == LV_OS_CUSTOM

#include <stdio.h>
#include <string.h>
#include "heap_api.h"
#include "hal_trace.h"

#define LV_RTOS_DEBUG
#ifdef LV_RTOS_DEBUG
#define RTOS_DBG(...)       TR_INFO( TR_MOD(HAL), "//RTOS: "  __VA_ARGS__)
#else
#define RTOS_DBG(...)
#endif
#define RTOS_LOG(...)       TR_INFO( TR_MOD(HAL), "//RTOS: "  __VA_ARGS__)
#define RTOS_ERR(...)       TR_INFO( TR_MOD(HAL), "//RTOS: "  __VA_ARGS__)

#ifndef LVGL_STACK_SIZE
#define LVGL_STACK_SIZE     (256*1024)
#endif
#if (LVGL_STACK_SIZE > (64*1024))
// #define BUF_LOC            PSRAM_BSS_LOC
#define BUF_LOC            PSRAM_NC_BSS_LOC
#else
#define BUF_LOC            PSRAM_NC_BSS_LOC
#endif

#define MAX_THREAD_COUNT    10

struct thread_info_t {
    uint32_t id;
    void *mem;
};

static struct thread_info_t thread_list[MAX_THREAD_COUNT];

static ALIGNED(64) uint8_t stack_buffer[LVGL_STACK_SIZE] BUF_LOC;
static heap_handle_t stack_pool = NULL;

static void *lv_stack_alloc(int bytes)
{
    if (!stack_pool) {
        stack_pool = heap_register(stack_buffer, LVGL_STACK_SIZE);
        ASSERT(stack_pool != NULL, "%s: LVGL stack pool register failed.", __func__);
    }
    return heap_malloc_align(stack_pool, bytes, 64);
}

static void lv_stack_free(void *mem)
{
    if (!stack_pool || !mem)
        return;
    heap_free_align(stack_pool, mem);
}

lv_result_t lv_thread_init(lv_thread_t *thread, const char * const name,
                           lv_thread_prio_t prio, void (*cb)(void *),
                           size_t stack_size, void *userdata)
{
    osThreadAttr_t attr;
    osPriority_t pri;
    void *stack_mem;
    int index = -1;

    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
        if (thread_list[i].id == 0) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        ASSERT(0, "Failed to find thread seat in list.");
        return LV_RESULT_INVALID;
    }

    stack_mem = lv_stack_alloc(stack_size);
    if (!stack_mem) {
        ASSERT(0, "Failed to alloc thread mem.");
        return LV_RESULT_INVALID;
    }

    switch (prio) {
        case LV_THREAD_PRIO_LOWEST:
            pri = osPriorityLow;
            break;
        case LV_THREAD_PRIO_HIGH:
            pri = osPriorityHigh;
            break;
        case LV_THREAD_PRIO_HIGHEST:
            pri = osPriorityRealtime;
            break;
        default:
            pri = osPriorityNormal;
            break;
    }

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = name;
    attr.attr_bits = osThreadDetached;
    attr.stack_mem = stack_mem;
    attr.stack_size = stack_size;
    attr.priority = pri;

    *thread = osThreadNew(cb, userdata, &attr);
    if (NULL == *thread) {
        RTOS_ERR("%s: Failed to create thread, stack_mem %p", __func__, stack_mem);
        lv_stack_free(stack_mem);
        return LV_RESULT_INVALID;
    }

    thread_list[index].id = (uint32_t)*thread;
    thread_list[index].mem = stack_mem;
    RTOS_DBG("%s: [%s] created, priority %d, stack_size 0x%x, cb %p.", __func__,
             name, pri, stack_size, cb);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_delete(lv_thread_t *thread)
{
    uint32_t id = (uint32_t)(*thread);
    int index = -1;
    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
        if (thread_list[i].id == id) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        RTOS_ERR("%s: Failed to find thread %08x in list.", __func__, id);
        return LV_RESULT_INVALID;
    }

    osStatus_t status = osThreadTerminate(*thread);

    lv_stack_free(thread_list[index].mem);

    thread_list[index].id = 0;
    thread_list[index].mem = NULL;

    RTOS_DBG("%s: thread%d delete.", __func__, index);
    return (status == osOK) ? LV_RESULT_OK : LV_RESULT_INVALID;
}

#ifdef KERNEL_FREERTOS

static void prvMutexInit(lv_mutex_t * pxMutex)
{
    pxMutex->xMutex = xSemaphoreCreateRecursiveMutex();

    if(pxMutex->xMutex == NULL) {
        RTOS_ERR("xSemaphoreCreateMutex failed!");
        return;
    }
    pxMutex->xIsInitialized = pdTRUE;
}

static void prvCheckMutexInit(lv_mutex_t * pxMutex)
{
    if(pxMutex->xIsInitialized == pdFALSE) {
        taskENTER_CRITICAL();
        if(pxMutex->xIsInitialized == pdFALSE) {
            prvMutexInit(pxMutex);
        }
        taskEXIT_CRITICAL();
    }
}

lv_result_t lv_mutex_init(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xMutexTakeStatus = xSemaphoreTake(pxMutex->xMutex, portMAX_DELAY);
    if(xMutexTakeStatus != pdTRUE) {
        RTOS_ERR("xSemaphoreTake failed!");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    BaseType_t xMutexTakeStatus = xSemaphoreTakeFromISR(pxMutex->xMutex, &xHigherPriorityTaskWoken);
    if(xMutexTakeStatus != pdTRUE) {
        RTOS_ERR("xSemaphoreTake failed!");
        return LV_RESULT_INVALID;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_unlock(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xMutexGiveStatus = xSemaphoreGive(pxMutex->xMutex);
    if(xMutexGiveStatus != pdTRUE) {
        RTOS_ERR("xSemaphoreGive failed!");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_delete(lv_mutex_t * pxMutex)
{
    vSemaphoreDelete(pxMutex->xMutex);
    pxMutex->xIsInitialized = pdFALSE;
    return LV_RESULT_OK;
}

#else /* KERNEL_RTOS */

lv_result_t lv_mutex_init(lv_mutex_t *mutex)
{
    osMutexAttr_t attr = { NULL,
                           osMutexRecursive | osMutexPrioInherit | osMutexRobust,
                           NULL, 0U
                         };
    *mutex = osMutexNew(&attr);
    if (*mutex == NULL)  {
        RTOS_ERR("Failed to create mutex.");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t *mutex)
{
    if (*mutex == NULL)
        return LV_RESULT_INVALID;
    osStatus_t status = osMutexAcquire(*mutex, ~0U);
    if (status != osOK)  {
        RTOS_ERR("Failed to lock mutex %p, ret=%d", *mutex, (int)status);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t *mutex)
{
    if (*mutex == NULL)
        return LV_RESULT_INVALID;
    osStatus_t status = osMutexAcquire(*mutex, ~0U);
    if (status != osOK)  {
        RTOS_ERR("Failed to lock mutex %p in an ISR, ret=%d", *mutex, (int)status);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_unlock(lv_mutex_t *mutex)
{
    if (*mutex == NULL)
        return LV_RESULT_INVALID;
    osStatus_t status = osMutexRelease(*mutex);
    if (status != osOK)  {
        RTOS_ERR("Failed to release mutex %p, ret=%d", *mutex, (int)status);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_delete(lv_mutex_t *mutex)
{
    if (*mutex == NULL)
        return LV_RESULT_INVALID;
    osStatus_t status = osMutexDelete(*mutex);
    if (status != osOK)  {
        RTOS_ERR("Failed to delete mutex %p, ret=%d", *mutex, (int)status);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}
#endif

lv_result_t lv_thread_sync_init(lv_thread_sync_t *sync)
{
    *sync = osEventFlagsNew(NULL);
    if (NULL == *sync) {
        RTOS_ERR("Failed to create sync event");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_wait(lv_thread_sync_t *sync)
{
    uint32_t ret = osEventFlagsWait(*sync, 0x01, osFlagsWaitAny, osWaitForever);
    if (ret & (1 << 31)) {
        RTOS_ERR("Failed to wait sync event, ret=%d", ret);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal(lv_thread_sync_t * sync)
{
    uint32_t ret = osEventFlagsSet(*sync, 0x01);
    if (ret & (1 << 31)) {
        RTOS_ERR("Failed to set sync event, ret=%d", ret);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_delete(lv_thread_sync_t * sync)
{
    osStatus_t status = osEventFlagsDelete(*sync);
    if (status != osOK)  {
        RTOS_ERR("Failed to delete sync event, ret=%d", (int)status);
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

uint32_t lv_os_get_idle_percent(void)
{
    return 100;
}



#endif /*LV_USE_OS == LV_OS_CMSIS_RTOS2*/
