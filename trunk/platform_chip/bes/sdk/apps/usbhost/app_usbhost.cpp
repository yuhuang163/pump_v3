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
#include "app_usbhost.h"
#include "usb_host.h"
#include "usb_utils.h"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "mbed_fs.h"

#ifdef RTOS
#include "cmsis_os2.h"
// --- thread & msg
#define USB_HOST_THREAD_STACK_SIZE (2048)
static osThreadId_t usbhost_thread_id;
static void usbhost_thread(void const *argument);
static uint64_t os_thread_def_stack_usb_host_thread [ROUND_UP(USB_HOST_THREAD_STACK_SIZE, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_usb_host = {
    .name = "usbhost_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_usb_host_thread,
    .stack_size = ROUND_UP(USB_HOST_THREAD_STACK_SIZE, 8),
    .priority = osPriorityNormal,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

#define USB_DISK_THREAD_STACK_SIZE (1024 * 8)
static osThreadId_t usbdisk_thread_id;
static void usbdisk_thread(void const *argument);
static uint64_t os_thread_def_stack_usb_disk_thread [ROUND_UP(USB_DISK_THREAD_STACK_SIZE, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_usb_disk = {
    .name = "usbdisk_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_usb_disk_thread,
    .stack_size = ROUND_UP(USB_DISK_THREAD_STACK_SIZE, 8),
    .priority = osPriorityNormal,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};
#define USB_DISK_THREAD_MSG_SIZE (10)
static const osMessageQueueAttr_t MessageQueueAttr_usbdisk = {
    .name = "usbdisk_msg_q",
    .attr_bits = 0U,
    .cb_mem = NULL,
    .cb_size = 0U,
    .mq_mem = NULL,
    .mq_size = 0U,
};
static osMessageQueueId_t usbdisk_msg_q_id;
#endif


enum USBH_CLASS_EVENT {
    USBH_IDLE = 0,
    USBH_INSET,
    USBH_REMOVE
};

typedef struct {
    enum USBH_CLASS_EVENT event;
}usbhost_disk_msg_t;

#define TEST_READ
#define TEST_WRITE
#define TEST_COPY

void DumpFS(int depth, int count);

int OnDiskInsert(int device)
{
    usbhost_disk_msg_t usb_disk_msg;
    myprintf(0, "%s: %d\r\n", __FUNCTION__, device);
    usb_disk_msg.event = USBH_INSET;
    osMessageQueuePut(usbdisk_msg_q_id, &usb_disk_msg, 0U, 0);
    return 0;
}

int onDiskRemove(int device)
{
    usbhost_disk_msg_t usb_disk_msg;
    myprintf(0, "%s: %d\r\n", __FUNCTION__, device);
    usb_disk_msg.event = USBH_REMOVE;
    osMessageQueuePut(usbdisk_msg_q_id, &usb_disk_msg, 0U, 0);
    myprintf(0, "%s: fs removed OK\r\n", __FUNCTION__);

    return 0;
}

void USB_disk_remove()
{
    mbed_fs_umount("usb");
}

//Serial pc(USBTX, USBRX);
void DumpFS(int depth, int count)
{
    DIR *d = opendir("/usb");
    if (!d) {
        myprintf(0, "USB file system borked");
        return;
    }

    myprintf(0, "Dumping root dir");
    struct dirent *p;
    while((p = readdir(d))) {
        int len = sizeof( dirent);
        myprintf(2, "%s %d\n", p->d_name, len);
    }
    closedir(d);
}

#if defined(TEST_READ) || defined(TEST_WRITE) || defined(TEST_COPY)

unsigned char file_buf[512];
unsigned int total_copy_size;

int test_file_copy(void)
{
    int ret = 0;
    size_t rcnt, wcnt;
    int tmperr;
    uint32_t stime, etime;
    uint32_t file_size;

    rcnt = 0;
    wcnt = 0;
    file_size = 0;

    const char *rfile = "/usb/usb_test.txt";
    const char *wfile = "/usb/file_copy.txt";

    FILE *rfp = NULL;
    FILE *wfp = NULL;

    rfp = fopen(rfile, "rb");
    if (rfp == NULL) {
        myprintf(0, "Failed to open file: %s", rfile);
        ret = 1;
        goto _exit;
    }
    wfp = fopen(wfile, "wb");
    if (wfp == NULL) {
        myprintf(0, "Failed to open file: %s", wfile);
        ret = 2;
        goto _exit;
    }

#ifdef TEST_READ
    // Test read
    myprintf(0, "Start to read file ...");

    stime = hal_fast_sys_timer_get();
    total_copy_size = 0;

    while ((rcnt = fread(file_buf, 1, sizeof(file_buf), rfp)) > 0) {
        total_copy_size += rcnt;
        if ((total_copy_size & ((1 << 20) - 1)) == 0) {
            myprintf(0, "Read 0x%x bytes", total_copy_size);
        }
    }
    tmperr = errno;
    if (ret == 0) {
        if (feof(rfp) == 0) {
            myprintf(0, "Failed to read file: %d", tmperr);
            ret = 1;
        }
    }
    etime = hal_fast_sys_timer_get();
    myprintf(0, "Elapsed time: %08d s", TICKS_TO_MS(etime - stime) / 1000);
    myprintf(0, "Read rate: 0x%x byte/s for 0x%x bytes", (total_copy_size/TICKS_TO_MS(etime - stime))*1000, total_copy_size);

    if (ret) {
        goto _exit;
    }
#endif

#ifdef TEST_WRITE
    // Test write
    myprintf(0, "Start to write file ...");

    file_size = 4000000;
    stime = hal_fast_sys_timer_get();
    total_copy_size = 0;
    rcnt = sizeof(file_buf);

    while (total_copy_size < file_size) {
        wcnt = fwrite(file_buf, 1, rcnt, wfp);
        if (wcnt != rcnt) {
            myprintf(0, "Failed to write %d bytes of data at offset %d: %d", rcnt, total_copy_size, errno);
            ret = 1;
            break;
        }
        total_copy_size += rcnt;
    }
    tmperr = errno;
    if (ret) {
        myprintf(0, "Failed to write file: %d", tmperr);
    }
    etime = hal_fast_sys_timer_get();
    myprintf(0, "Elapsed time: %08d s", TICKS_TO_MS(etime - stime) / 1000);
    myprintf(0, "Write rate: 0x%x byte/s for 0x%x bytes", (total_copy_size / TICKS_TO_MS(etime - stime))*1000, total_copy_size);

    if (ret) {
        goto _exit;
    }
#endif

#ifdef TEST_COPY
    if (rfp) {
        fclose(rfp);
        rfp = NULL;
    }
    rfp = fopen(rfile, "rb");
    if (rfp == NULL) {
        myprintf(0, "Failed to open file: %s", rfile);
        ret = 1;
        goto _exit;
    }
    // Test copy
    myprintf(0, "Start to copy file ...");

    stime = hal_fast_sys_timer_get();
    total_copy_size = 0;

    while ((rcnt = fread(file_buf, 1, sizeof(file_buf), rfp)) > 0) {
        wcnt = fwrite(file_buf, 1, rcnt, wfp);
        if (wcnt != rcnt) {
            myprintf(0, "Failed to write %d bytes of data at offset %d: %d", rcnt, total_copy_size, errno);
            ret = 1;
            break;
        }
        total_copy_size += rcnt;
        if ((total_copy_size & ((1 << 20) - 1)) == 0) {
            myprintf(0, "Copied %u bytes", total_copy_size);
        }
    }
    tmperr = errno;
    if (ret == 0) {
        if (feof(rfp) == 0) {
            myprintf(0, "Failed to read file: %d", tmperr);
            ret = 1;
        }
    }
    etime = hal_fast_sys_timer_get();
    myprintf(0, "Elapsed time: %08d s", TICKS_TO_MS(etime - stime) / 1000);
    myprintf(0, "Copy rate: 0x%x byte/s for %u bytes", (total_copy_size / TICKS_TO_MS(etime - stime))*1000, total_copy_size);
#endif

_exit:
    if (rfp) {
        fclose(rfp);
    }
    if (wfp) {
        fclose(wfp);
    }

    return ret;
}
#endif

void USB_disk_inset(void)
{
    int ret;
    mbed_fs_mount("usb");
    osDelay(200);
    DumpFS(0, 0);
#if defined(TEST_READ) || defined(TEST_WRITE) || defined(TEST_COPY)
    //osDelay(200);
    ret = test_file_copy();
#endif
    myprintf(0, "%d\r\n", ret);
}
/*
    Simple test shell to exercise mouse,keyboard,mass storage and hubs.
    Add 2 15k pulldown resistors between D+/D- and ground, attach a usb socket and have at it.
*/
#ifndef RTOS
int main(void)
{
    int ret;

    hal_audma_open();
    hal_gpdma_open();
#if (DEBUG_PORT == 2)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#else
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
    hwtimer_init();
    ret = hal_analogif_open();
    if (ret)
        myprintf(0,"hal_analogif_open failed \n");

    ret = pmu_open();
    ASSERT(ret == 0, "Failed to open pmu");

    myprintf(0, "USBShell:Now get a bunch of usb things and plug them in...");

    USBInit();

    uint32_t start = hal_fast_sys_timer_get();
    uint32_t cur;

    for (;;) {
        cur = hal_fast_sys_timer_get();
        if (cur - start > MS_TO_TICKS(2000)) {
            start = cur;
            myprintf(0, "In main loop");
        }
        USBLoop();
    }
}
#else
static void usbhost_thread(void const *argument)
{
    uint32_t count = 0;
    TRACE_IMM(0, "usbhost_thread\r\n");
    USBInit();
    while (1) {
        osDelay(20);
        if (count++ > 100) {
            count = 0;
            TRACE_IMM(0, "usb host loop\r\n");
        }
        USBLoop();
    }
}

static void usbdisk_thread(void const *argument)
{
    TRACE_IMM(0, "usbdisk_thread\r\n");
    osStatus_t status;
    usbhost_disk_msg_t usb_disk_msg;
    usbdisk_msg_q_id = osMessageQueueNew(USB_DISK_THREAD_MSG_SIZE, sizeof(usbhost_disk_msg_t), &MessageQueueAttr_usbdisk);
    if (usbdisk_msg_q_id == NULL) {
        TRACE(1, "%s: create msgq err\n", __func__);
        return;
    }
    while (1) {
        status = osMessageQueueGet(usbdisk_msg_q_id, &usb_disk_msg, NULL, osWaitForever);
        if (status == osOK) {
            if (usb_disk_msg.event == USBH_INSET) {
                USB_disk_inset();
            } else if (usb_disk_msg.event == USBH_REMOVE) {
                USB_disk_remove();
            }
        }
    }
}

extern "C" void app_usb_host_disk_test(void)
{
    TRACE_IMM(0, "app_usb_host_disk_test\r\n");

    usbhost_thread_id = osThreadNew((osThreadFunc_t)usbhost_thread, NULL, &ThreadAttr_usb_host);
    if (usbhost_thread_id == NULL) {
        TRACE(1, "%s: create thread err\n", __func__);
        return;
    }

    usbdisk_thread_id = osThreadNew((osThreadFunc_t)usbdisk_thread, NULL, &ThreadAttr_usb_disk);
    if (usbdisk_thread_id == NULL) {
        TRACE(1, "%s: create thread err\n", __func__);
        return;
    }
}
#endif
