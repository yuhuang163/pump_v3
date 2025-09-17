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
#ifndef __BLUETOOTH_BT_API_H__
#define __BLUETOOTH_BT_API_H__
#include "tgt_hardware.h"
#include "bt_common_define.h"
#include "source_common_define.h"
#include "adapter_service.h"
#include "bes_source_api.h"
#include "bes_me_api.h"
#include "bes_l2cap_api.h"
#include "bes_a2dp_api.h"
#include "bes_avrcp_api.h"
#include "bes_hfp_api.h"
#include "bes_ibrt_api.h"
#include "bes_dip_api.h"
#include "bes_hid_api.h"
#include "hci_api.h"
#include "spp_api.h"
#include "app_bt_hid.h"
#include "app_map.h"
#include "besbt.h"
#ifdef BLE_STACK_NEW_DESIGN
#include "gap_service.h"
#include "gatt_service.h"
#endif

#if defined(__GATT_OVER_BR_EDR__)
#include "app_btgatt.h"
#endif

#endif /* __BLUETOOTH_BT_API_H__ */

