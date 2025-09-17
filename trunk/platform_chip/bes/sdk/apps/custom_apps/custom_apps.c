/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#include "stdio.h"
#include "stdlib.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_wdt.h"
#include "hal_trng.h"
#include "hal_uart.h"
#include "hwtimer_list.h"
#include "mpu_cfg.h"
#include "mpu.h"
#include "pmu.h"
#include "analog.h"
#include "norflash_api.h"
#include "app_utils.h"
#include "watchdog/watchdog.h"
#include "audioflinger.h"
#include "aiot_generic.h"
#include "heap_api.h"
#include "custom_apps.h"


// custom message handler management

extern uint32_t __custom_apps_message_handler_table_start[];
extern uint32_t __custom_apps_message_handler_table_end[];

#define CUSTOM_APPS_MESSAGE_HANDLER_PTR_FROM_ENTRY_INDEX(index)    \
    ((CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T *)((uint32_t)__custom_apps_message_handler_table_start + (index)*sizeof(CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T)))

static CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T* custom_apps_get_message_handler_entry_pointer_from_message_id(uint16_t message_id)
{
    for (uint32_t index = 0;
        index < ((uint32_t)__custom_apps_message_handler_table_end-
        (uint32_t)__custom_apps_message_handler_table_start)/
        sizeof(CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T);index++)
    {
        if (CUSTOM_APPS_MESSAGE_HANDLER_PTR_FROM_ENTRY_INDEX(index)->message_id == message_id)
        {
            return CUSTOM_APPS_MESSAGE_HANDLER_PTR_FROM_ENTRY_INDEX(index);
        }
    }

    return NULL;
}

// custom application heap management

// can be coordinated accordingly
#ifndef CUSTOM_APPS_HEAP_SIZE
#define CUSTOM_APPS_HEAP_SIZE   (32*1024)
#endif

static uint8_t custom_apps_heap[CUSTOM_APPS_HEAP_SIZE];
static heap_handle_t custom_apps_heap_handle = NULL;

static void custom_apps_heap_init(void)
{
    custom_apps_heap_handle = heap_register(custom_apps_heap, sizeof(custom_apps_heap));
}

void* custom_apps_heap_allocate(uint32_t length)
{
    void* ret_ptr = NULL;
    if (custom_apps_heap_handle)
    {
        ret_ptr = heap_malloc(custom_apps_heap_handle, length);
        ASSERT(ret_ptr != NULL, "custom apps heap used out!");
    }

    return ret_ptr;
}

void custom_apps_heap_free(void* ptr)
{
    if (custom_apps_heap_handle)
    {
        heap_free(custom_apps_heap_handle, ptr);
    }
}

uint32_t custom_apps_heap_get_free_size(void)
{
    if (custom_apps_heap_handle)
    {
        return heap_free_size(custom_apps_heap_handle);
    }
    else
    {
        return 0;
    }
}

// custom application mailbox management
typedef struct {
    uint16_t    message_id;
    uint32_t    msg_param_len;
    uint8_t*    msg_param_ptr;
} CUSTOM_APPS_MESSAGE_T;


#ifndef CUSTOM_APPS_MAILBOX_MAX_CNT
#define CUSTOM_APPS_MAILBOX_MAX_CNT 32
#endif

osMailQDef (custom_apps_mailbox, CUSTOM_APPS_MAILBOX_MAX_CNT, CUSTOM_APPS_MESSAGE_T);
static osMailQId custom_apps_mailbox_id = NULL;

static int8_t custom_apps_mailbox_init(void)
{
    custom_apps_mailbox_id = osMailCreate(osMailQ(custom_apps_mailbox), NULL);
    if (NULL == custom_apps_mailbox_id)
    {
        ASSERT(0, "Failed to Create custom_apps_mailbox\n");
        return -1;
    }
    return 0;
}

int32_t custom_apps_mailbox_put(uint16_t       message_id, uint8_t* msg_param_ptr, uint32_t msg_param_len)
{
    osStatus status;

    CUSTOM_APPS_MESSAGE_T* msg_p = (CUSTOM_APPS_MESSAGE_T*)osMailAlloc(custom_apps_mailbox_id, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(0, "osMailAlloc error dump");
        for (uint8_t i = 0;i < CUSTOM_APPS_MAILBOX_MAX_CNT;i++)
        {
            evt = osMailGet(custom_apps_mailbox_id, 0);
            if (evt.status == osEventMail)
            {
                TRACE_IMM(9, "index:%d msg id:%d param len:%08x param ptr:0x%x",
                            i,
                            (uint32_t)((CUSTOM_APPS_MESSAGE_T *)(evt.value.p))->message_id,
                            (uint32_t)((CUSTOM_APPS_MESSAGE_T *)(evt.value.p))->msg_param_len,
                            (uint32_t)((CUSTOM_APPS_MESSAGE_T *)(evt.value.p))->msg_param_ptr);
            }
            else
            {
                TRACE_IMM(2, "index:%d %d", i, evt.status);
                break;
            }
        }
        TRACE_IMM(0,"osMailAlloc error dump end");
    }

    ASSERT(msg_p, "osMailAlloc error");

    msg_p->message_id = message_id;
    msg_p->msg_param_len = msg_param_len;
    if (msg_param_len > 0)
    {
        msg_p->msg_param_ptr = (uint8_t *)custom_apps_heap_allocate(msg_param_len);
        memcpy(msg_p->msg_param_ptr, msg_param_ptr, msg_param_len);
    }
    else
    {
        msg_p->msg_param_ptr = NULL;
    }
    status = osMailPut(custom_apps_mailbox_id, msg_p);
    return (int32_t)status;
}

static int8_t custom_apps_mailbox_free(CUSTOM_APPS_MESSAGE_T* msg_p)
{
    if (msg_p->msg_param_len > 0)
    {
        custom_apps_heap_free(msg_p->msg_param_ptr);
    }

    osStatus status;

    status = osMailFree(custom_apps_mailbox_id, msg_p);

    return (int)status;
}

static int8_t custom_apps_mailbox_get(CUSTOM_APPS_MESSAGE_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(custom_apps_mailbox_id, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (CUSTOM_APPS_MESSAGE_T *)evt.value.p;
        return 0;
    }
    return -1;
}

// custom application thread
// can be coordinated accordingly
#ifndef CUSTOM_APPS_THREAD_STACK_SIZE
#define CUSTOM_APPS_THREAD_STACK_SIZE 8192
#endif

static void custom_apps_thread(void const *argument);
osThreadDef(custom_apps_thread, osPriorityHigh, 1, CUSTOM_APPS_THREAD_STACK_SIZE, "custom_apps_thread");
static osThreadId custom_apps_thread_id = NULL;

static void custom_apps_thread(void const *argument)
{
    while(1){
        CUSTOM_APPS_MESSAGE_T *msg_p = NULL;

        if (!custom_apps_mailbox_get(&msg_p)) {
            CUSTOM_APPS_MESSAGE_HANDLER_INSTANCE_T* pInstance =
                custom_apps_get_message_handler_entry_pointer_from_message_id(msg_p->message_id);
            if (pInstance)
            {
                pInstance->messageHandler(pInstance->message_id, msg_p->msg_param_ptr, msg_p->msg_param_len);
            }
            custom_apps_mailbox_free(msg_p);
        }
    }
}

static int8_t custom_apps_thread_init(void)
{
    custom_apps_thread_id = osThreadCreate(osThread(custom_apps_thread), NULL);
    if (NULL == custom_apps_thread_id)
    {
        ASSERT(0, "Failed to Create custom_apps_thread\n");
        return -1;
    }
    return 0;
}

void aiot_platform_pre_initializing_callback(void)
{
    TRACE(0, "pre SDK initialization started.");
}

// for test purpose
#define CUSTOM_APPS_MESSAGE_DEMO_ENABLED

#ifdef CUSTOM_APPS_MESSAGE_DEMO_ENABLED

static void customAppsDemoMessageHandler(uint16_t message_id, uint8_t* ptrParam, uint32_t paramLen)
{
    TRACE(0, "Recevie msg id 0x%x, param len %d content:", message_id, paramLen);
    DUMP8("%02x ", ptrParam, paramLen);
    TRACE(0, "Current time %d ms", *(uint32_t *)ptrParam);
}
CUSTOM_APPS_MESSAGE_HANDLER_TO_ADD(CUSTOM_APPS_MSG_DEMO, customAppsDemoMessageHandler);

#define CUSTOM_APPS_MESSAGE_DEMO_PERIOD_MS  2000
static void custom_apps_demo_timer_handler(void const *param);
osTimerDef(custom_apps_demo_timer, custom_apps_demo_timer_handler);
static osTimerId custom_apps_demo_timer_id = NULL;
static void custom_apps_demo_timer_handler(void const *param)
{
    uint32_t current_time = GET_CURRENT_MS();
    custom_apps_mailbox_put(CUSTOM_APPS_MSG_DEMO, (uint8_t *)&current_time, sizeof(current_time));
}

static void custom_apps_start_timer_demo(void)
{
    custom_apps_demo_timer_id = osTimerCreate(osTimer(custom_apps_demo_timer), osTimerPeriodic, NULL);
    if (custom_apps_demo_timer_id != NULL)
    {
        osTimerStart(custom_apps_demo_timer_id, CUSTOM_APPS_MESSAGE_DEMO_PERIOD_MS);
    }
}
#endif

void aiot_platform_initialized_callback(void)
{
    TRACE(0, "SDK initialization completed, ready to start custom application.");

    // create applcation heap
    custom_apps_heap_init();

    // create application thread used mailbox
    custom_apps_mailbox_init();

    // create application thread handler
    custom_apps_thread_init();

#ifdef CUSTOM_APPS_MESSAGE_DEMO_ENABLED
    custom_apps_start_timer_demo();
#endif
}

