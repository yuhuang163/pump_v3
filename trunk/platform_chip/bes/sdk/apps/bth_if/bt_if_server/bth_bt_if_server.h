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

#ifndef __BTH_BT_IF_SERVER_H__
#define __BTH_BT_IF_SERVER_H__
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_bth_bt_if_init(void);
void app_bth_a2dp_src_if_init(void);
void app_bth_hfp_cli_if_init(void);
void app_bth_ble_if_init(void);
void app_bth_gatt_if_init(void);
void app_bth_a2dp_sink_if_init(void);

#ifdef __cplusplus
}
#endif
#endif // end of __BTH_BT_IF_SERVER_H__