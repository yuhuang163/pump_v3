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
#ifndef __USB_ADB_H__
#define __USB_ADB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum USB_ADB_API_MODE {
    USB_ADB_API_NONBLOCKING,
    USB_ADB_API_BLOCKING,
};

enum USB_ADB_STATE_EVENT_T {
    USB_ADB_STATE_RESET, // RESET event should be processed as quickly as possible
    USB_ADB_STATE_DISCONNECT,
    USB_ADB_STATE_SLEEP,
    USB_ADB_STATE_WAKEUP,
    USB_ADB_STATE_CONFIG,
    USB_ADB_STATE_STALL_RECV,
    USB_ADB_STATE_STALL_SEND,
    USB_ADB_STATE_STALL_INTR,
    USB_ADB_STATE_UNSTALL_RECV,
    USB_ADB_STATE_UNSTALL_SEND,
    USB_ADB_STATE_UNSTALL_INTR,
    USB_ADB_STATE_TERMINAL_CONNECTED,
    USB_ADB_STATE_TERMINAL_DISCONNECTED,
};

enum USB_ADB_RET_VALUE {
    USB_ERR_OK                          = 0,
    USB_ERR_RXTX_DATA                   = -1,
    USB_ERR_NOT_DONE                    = -2,
    USB_ERR_NOT_IDLE                    = -3,
    USB_ERR_TIMEOUT                     = -4,
    USB_ERR_NOT_LOCK                    = -9,
    USB_ERR_NOT_CONNECT                 = -10,
    USB_ERR_INV_PARAM                   = -11,
    USB_ERR_UNALIGNED                   = -12,
    USB_ERR_RX_SIZE                     = -13,
    USB_ERR_RXTX_CANCEL                 = -14,
};

enum USB_ADB_XFER_TYPE_TYPE_T {
    USB_ADB_XFER_TYPE_ALL,
    USB_ADB_XFER_TYPE_RX,
    USB_ADB_XFER_TYPE_TX,
    USB_ADB_XFER_TYPE_NOTIF,
};

typedef void (*USB_ADB_STATE_CALLBACK)(enum USB_ADB_STATE_EVENT_T event);
typedef void (*USB_ADB_XFER_CALLBACK)(const void *data, uint32_t size, int error);

struct USB_ADB_CFG_T {
    USB_ADB_STATE_CALLBACK state_callback;
    enum USB_ADB_API_MODE mode;
};

int usb_adb_open(const struct USB_ADB_CFG_T *cfg);

void usb_adb_close(void);

void usb_adb_init_xfer(enum USB_ADB_XFER_TYPE_TYPE_T type);

int usb_adb_irq_run(void);

void usb_adb_stop_recv(void);

void usb_adb_cancel_recv(void);

int usb_adb_flush_recv_buffer(void);

int usb_adb_recv(void *buf, uint32_t size, int timeout);

int usb_adb_direct_recv(void *dma_buf, uint32_t size, uint32_t expect, uint32_t *recv,
                           USB_ADB_XFER_CALLBACK callback);

void usb_adb_cancel_send(void);

int usb_adb_send(const void *buf, uint32_t size);

int usb_adb_direct_send(const void *dma_buf, uint32_t size, USB_ADB_XFER_CALLBACK callback);

void usb_adb_set_recv_cb(USB_ADB_XFER_CALLBACK recv_cb, USB_ADB_XFER_CALLBACK send_cb);

#ifdef __cplusplus
}
#endif

#endif

