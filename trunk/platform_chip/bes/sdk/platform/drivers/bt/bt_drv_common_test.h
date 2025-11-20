/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef __BT_DRV_COMMON_TEST_H__
#define __BT_DRV_COMMON_TEST_H__

#include "bt_drv_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void btdrv_common_pscan_standard_enable_test(uint16_t interval,uint16_t window);
void btdrv_common_pscan_interlaced_enable_test(uint16_t interval,uint16_t window);
void btdrv_common_bothscan_enable_test(uint16_t interval,uint16_t window);
void btdrv_common_bt_create_connect_test(void);
void btdrv_common_bt_enter_sniff_test(uint16_t conhandle,uint16_t sniff_interval,uint16_t attempt,uint16_t timeout);
void btdrv_common_ble_adv_enable_test(uint16_t adv_interval);
void btdrv_common_ble_create_connect_test(uint16_t conn_interval);
void btdrv_common_ble_adv_enable_with_data_test(uint16_t adv_interval);
void btdrv_bqbtest_loop(void);

#ifdef __cplusplus
}
#endif
#endif
