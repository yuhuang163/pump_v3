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

#ifndef __USB_BT_WIN_H__
#define __USB_BT_WIN_H__
#include "hal_usb.h"

#define USBBT_MAX_PACKET_SIZE_EPINT         64
#define USBBT_MAX_PACKET_SIZE_BULK          64
#define USBBT_MAX_PACKET_SIZE_ISO           256

#define EPINT_IN                            1   // bt event rx
#define EP2BULK_IN                          2   // bt acl RX
#define EP3ISO_IN                           3   // bluetooth sco RX

#define EP2BULK_OUT                         2   // bt acl TX
#define EP3ISO_OUT                          3   // bluetooth sco TX

void usb_bt_win_open(void);
void usb_bt_win_close(void);
#endif /*__USB_BT_WIN_H__*/
