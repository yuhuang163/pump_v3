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
#ifdef TEST_SYSVIEW
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "test_hal_api.h"
#ifdef EVR_TRACING
#include "SEGGER_SYSVIEW.h"
#endif
#ifdef UTILS_ESHELL_EN
#include "eshell.h"
#endif
#ifdef USER_GPS_TO_FLASH
#include "user_gps_section.h"
#endif

#ifdef USER_GPS_TO_FLASH
typedef struct {
    uint8_t    *data;
    uint32_t    len;
    uint8_t     status;
} __attribute__((packed)) USER_GPS_MESSAGE_T;

#define USER_GPS_MAILBOX_MAX (3)
osMailQDef (user_gps_mailbox, USER_GPS_MAILBOX_MAX, USER_GPS_MESSAGE_T);
static osMailQId user_gps_mailbox = NULL;
static uint8_t user_gps_mailbox_cnt = 0;

static void user_gps_mailbox_init(void)
{
    TRACE(0, "%s", __func__);
    user_gps_mailbox = osMailCreate(osMailQ(user_gps_mailbox), NULL);
    ASSERT(user_gps_mailbox, "Failed to create app_user_gps mailbox");

    user_gps_mailbox_cnt = 0;
}

static int32_t user_gps_mailbox_free(USER_GPS_MESSAGE_T *msg_p)
{
    osStatus status;

    status = osMailFree(user_gps_mailbox, msg_p);
    if (status == osOK) {
        user_gps_mailbox_cnt--;
    }

    return (int32_t)status;
}

static int32_t user_gps_mailbox_get(USER_GPS_MESSAGE_T **msg_p)
{
    osEvent evt;
    evt = osMailGet(user_gps_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (USER_GPS_MESSAGE_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t user_gps_mailbox_free_cnt(uint32_t cnt)
{
    USER_GPS_MESSAGE_T *msg_p = NULL;
    int32_t POSSIBLY_UNUSED status = osOK;
    osEvent evt;

    for (uint8_t i=0; i<cnt; i++) {
        evt = osMailGet(user_gps_mailbox, 0);
        if (evt.status == osEventMail) {
            msg_p = (USER_GPS_MESSAGE_T *)evt.value.p;
            status = user_gps_mailbox_free(msg_p);
        } else {
            TRACE(2, "[%s]: get mailbox failed: %d", __func__, evt.status);
        }
   }

   return 0;
}

static int32_t user_gps_mailbox_put(USER_GPS_MESSAGE_T *msg)
{
    osStatus status;

    USER_GPS_MESSAGE_T *msg_new = NULL;

    msg_new = (USER_GPS_MESSAGE_T *)osMailAlloc(user_gps_mailbox, 0);

    if (msg_new == NULL) {
        TRACE(0, "[%s] cnt: %d, Free some message...", __func__, user_gps_mailbox_cnt);
        user_gps_mailbox_free_cnt(user_gps_mailbox_cnt/2);
        msg_new = (USER_GPS_MESSAGE_T *)osMailAlloc(user_gps_mailbox, 0);
        ASSERT(msg_new, "app_user_gps: osMailCAlloc fail");
    }

    memcpy(msg_new, msg, sizeof(USER_GPS_MESSAGE_T));

    status = osMailPut(user_gps_mailbox, msg_new);
    if (osOK == status) {
        user_gps_mailbox_cnt++;
    } else {
        TRACE(0, "app_user_gps: osMailPut fail, status: 0x%x", status);
    }

    return (int32_t)status;
}

void app_user_gps_post_msg(uint8_t *buf, uint32_t len, uint8_t status)
{
    USER_GPS_MESSAGE_T msg = {0};
    msg.data    = buf;
    msg.len     = len;
    msg.status  = status;
    user_gps_mailbox_put(&msg);
}

static uint8_t data_buf[4096] = {0};

#define DATA_END    0x01

static void dump_data_to_flash(uint8_t *buff, uint32_t len, uint8_t status)
{
    static uint32_t cnt = 0;
    static uint32_t length = 0;
    static uint8_t *ptr = data_buf;
    uint32_t _len;

    if (status == DATA_END) {
        user_gps_write(data_buf, length, cnt*4096);
        return ;
    }

    while (length + len >= 4096) {
        _len = 4096 - length;
        memcpy(ptr, buff, _len);
        user_gps_write(data_buf, 4096, cnt*4096);

        ptr = data_buf;
        buff += _len;
        len -= _len;
        length = 0;
        cnt++;
    }

    memcpy(ptr, buff, len);
    ptr += len;
    length += len;
}
#endif

typedef struct _thread_def {
  osThreadId                 thread_id;
  osThreadDef_t              thread_def;
} ThreadDef_t;

#define TASK_NUM    3

ThreadDef_t task_array[TASK_NUM] = {0};

#define TASK_STACK_SIZE (6*1024)
#ifdef SYSTEM_USE_FILE
__attribute__((aligned(8))) uint8_t test1_task_stack[TASK_STACK_SIZE];
__attribute__((aligned(8))) uint8_t test2_task_stack[TASK_STACK_SIZE];
#endif
#ifdef USER_GPS_TO_FLASH
__attribute__((aligned(8))) uint8_t user_gps_task_stack[TASK_STACK_SIZE];
#endif

#ifdef SYSTEM_USE_FILE
static void test1_task_entry(void const *argument)
{
    uint8_t cnt = 0;
    bool flag = false;
#if defined(SYSTEMVIEW_TO_FLASH) || defined (SYSTEMVIEW_TO_SPI)
    TRACE(0, "sysview start");
    segger_systemview_start();
#endif

    while (1) {
        osDelay(1000);
#if defined(SYSTEMVIEW_TO_FLASH) || defined (SYSTEMVIEW_TO_SPI)
        if (cnt < 100) {
            cnt++;
        } else {
            if (flag == false) {
                segger_systemview_stop();
                TRACE_IMM(0, "sysview stop");
                flag = true;
            }
        }
#endif
        TRACE(0, "== [%s] %d", __func__, __LINE__);
        osThreadFlagsSet(task_array[1].thread_id, 0x2);
    }
}

static void test2_task_entry(void const *argument)
{
    static uint32_t cnt = 0;
    uint8_t buf[100] = {0};
    while (1) {
        osThreadFlagsWait(0x2, osFlagsWaitAny, osWaitForever);
        cnt++;
        snprintf(buf, sizeof(buf), "test2_task_entry recv flag 0x1 cnt=%d", cnt);
    #ifdef EVR_TRACING
        SEGGER_SYSVIEW_Print(buf);
    #endif
        memset(buf, 0, sizeof(buf));
        TRACE(0, "== [%s] recv flag 0x1 cnt=%d", __func__, cnt);
    }
}
#endif

#ifdef USER_GPS_TO_FLASH
static void user_gps_task_entry(void const *argument)
{
    TRACE(0, "== [%s] %d", __func__, __LINE__);

    while (1) {
        USER_GPS_MESSAGE_T *msg = NULL;
        if (!user_gps_mailbox_get(&msg)) {
            TRACE(0, "msg->data=0x%08X, msg->len=%d, msg->status=%d", msg->data, msg->len, msg->status);
            if (msg->status == DATA_END) {
                dump_data_to_flash(msg->data, msg->len, 0);
                dump_data_to_flash(NULL, NULL, DATA_END);
            } else {
                dump_data_to_flash(msg->data, msg->len, 0);
            }
            user_gps_mailbox_free(msg);
        }
    }
}
#endif

static void task_init(void)
{
    uint32_t task_num = 0;
#ifdef SYSTEM_USE_FILE
    task_array[task_num].thread_def.attr.name           = "test1_task";
    task_array[task_num].thread_def.pthread             = (os_pthread)test1_task_entry;
    task_array[task_num].thread_def.attr.priority       = 40;
    task_array[task_num].thread_def.attr.stack_mem      = test1_task_stack;
    task_array[task_num].thread_def.attr.stack_size     = sizeof(test1_task_stack);
    task_num++;

    task_array[task_num].thread_def.attr.name           = "test2_task";
    task_array[task_num].thread_def.pthread             = (os_pthread)test2_task_entry;
    task_array[task_num].thread_def.attr.priority       = 40;
    task_array[task_num].thread_def.attr.stack_mem      = test2_task_stack;
    task_array[task_num].thread_def.attr.stack_size     = sizeof(test2_task_stack);
    task_num++;
#endif
#ifdef USER_GPS_TO_FLASH
    task_array[task_num].thread_def.attr.name           = "user_gps_host";
    task_array[task_num].thread_def.pthread             = (os_pthread)user_gps_task_entry;
    task_array[task_num].thread_def.attr.priority       = 40;
    task_array[task_num].thread_def.attr.stack_mem	    = user_gps_task_stack;
    task_array[task_num].thread_def.attr.stack_size     = sizeof(user_gps_task_stack);
    task_num++;

    user_gps_mailbox_init();
#endif

    for (uint8_t i=0; i<task_num; i++) {
        TRACE(0, "[%s] %d stack:%p size:%dkB", __func__, __LINE__, \
                    task_array[i].thread_def.attr.stack_mem, (task_array[i].thread_def.attr.stack_size/1024));
        memset(task_array[i].thread_def.attr.stack_mem, 0x55, task_array[i].thread_def.attr.stack_size);
        task_array[i].thread_id = osThreadNew(task_array[i].thread_def.pthread, NULL, &task_array[i].thread_def.attr);
        if (task_array[i].thread_id == NULL) {
            ASSERT(0, "osThreadNew %s fail", task_array[i].thread_def.attr.name);
        }
        TRACE(0, "osThreadNew %s success", task_array[i].thread_def.attr.name);
    }
}

void test_sysview(void)
{
#ifdef SYSTEM_USE_FILE

#ifdef SYSTEMVIEW_TO_UART
    #if defined(UTILS_ESHELL_EN)
    #ifdef APP_RPC_ENABLE
    app_eshell_rpc_init();
    eshell_rpc_run_cmd_func_cb_register(app_eshell_rpc_send_op_cmd_func, app_eshell_rpc_send_op_result_func);
    #endif
    eshell_open(HAL_UART_ID_0, true);
    TRACE_IMM(0,"IC initial success \r\n");
    eshell_putstring("IC initial success \r\n");
    TRACE_IMM(
        0, "\r\n eshell-cli start \r\n send ascii str <help\\r\\n> by log port for more info \r\n");
    #endif
#elif defined (SYSTEMVIEW_TO_FLASH)
    user_gps_to_flash_init();
    user_gps_erase_section();
#elif defined (SYSTEMVIEW_TO_SPI)
    spi_slave_test();
#endif

#else
    user_gps_to_flash_init();
    user_gps_erase_section();

    test_spi();
#endif

    task_init();
}

#endif
