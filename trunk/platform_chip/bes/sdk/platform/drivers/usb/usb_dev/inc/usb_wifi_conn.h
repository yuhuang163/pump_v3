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

#ifndef __USB_WIFI_CONN_H__
#define __USB_WIFI_CONN_H__
#include "hal_usb.h"

#define EPINT_IN                            1
#define EP2BULK_IN                          2   // wlan RX
#define EP3BULK_IN                          3   // bluetooth RX

#define EP2BULK_OUT                         2   // wlan TX
#define EP3BULK_OUT                         3   // bluetooth TX

void usb_wifi_conn_open(void);
#endif /*__USB_WIFI_CONN_H__*/
