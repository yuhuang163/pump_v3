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
#ifndef __WUI_H__
#define __WUI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bwifi_interface.h"
#include "bwifi_event.h"

void vow_wifi_connect_callback(WIFI_USER_EVT_ID event_id, void *arg);
void vow_wifi_disconnect_callback(WIFI_USER_EVT_ID event_id, void *arg);
void vow_error_handle_callback(WIFI_USER_EVT_ID event_id, void *arg);
int vow_wifi_get_ap_status(void);
#ifdef __cplusplus
}
#endif

#endif  /* __WUI_H__ */
