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
#ifndef __USB_GENERAL_H__
#define __USB_GENERAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_usb.h"
#include "stdint.h"
#define EPINT_IN                             1
#define EP2BULK_IN                           2
#define EP3BULK_IN                           3

#define EP2BULK_OUT                          2
#define EP3BULK_OUT                          3

#define WCID_VENDOR_CODE                     0x17
/* Vendor descriptor types*/
#define WCID_DESCRIPTOR                      0x0
/* Vendor descriptor lengths*/
#define BULK_WCID_DESC_SET_SIZE              0xA2

enum USB_GENERAL_RET_VALUE {
    USB__GENERALERR_OK                      = 0,
    USB_GENERAL_ERR_RXTX_DATA               = -1,
    USB_GENERAL_ERR_NOT_DONE                = -2,
    USB_GENERAL_ERR_NOT_IDLE                = -3,
    USB_GENERAL_ERR_NOT_LOCK                = -9,
    USB_GENERAL_ERR_NOT_CONNECT             = -10,
    USB_GENERAL_ERR_INV_PARAM               = -11,
    USB_GENERAL_ERR_UNALIGNED               = -12,
    USB_GENERAL_ERR_RX_SIZE                 = -13,
    USB_GENERAL_ERR_RXTX_CANCEL             = -14,
};

typedef void (*USB_GENERAL_DEVICE_STATUS_CALLBACK)(uint32_t status, uint32_t param);

struct USB_GENERAL_CFG_T {
    USB_GENERAL_DEVICE_STATUS_CALLBACK status_callback;
};

void usb_general_init_xfer(void);

int usb_general_flush_recv_buffer(void);

int usb_general_open(const struct USB_GENERAL_CFG_T *cfg);

void usb_general_close(void);

int usb_general_ready(void);

int usb_general_connected(void);

void usb_general_cancel_recv(void);

int usb_general_recv(uint8_t *buf, uint32_t size);

void usb_general_cancel_send(void);

int usb_general_send(const uint8_t *buf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /*__USB_GENERAL_H__*/
