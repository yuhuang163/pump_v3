/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __BT_SERVICE_BTS_H__
#define __BT_SERVICE_BTS_H__

#include "bluetooth_bt_api.h"
#include "bts_defines.h"
#include "bts_utils.h"
#include "bts_status.h"
#include "bts_device.h"
#include "bts_a2dp.h"
#include "bts_a2dp_source.h"
#include "bts_a2dp_sink.h"
#include "bts_hfp.h"
#include "bts_hfp_ag.h"
#include "bts_hfp_hf.h"
#include "bts_avrcp.h"
#include "bts_avrcp_tg.h"
#include "bts_avrcp_ct.h"
#include "bts_spp.h"
#include "bts_spp_client.h"
#include "bts_spp_server.h"

#include "transport_if.h"

#if defined(__cplusplus)
extern "C" {
#endif

bts_status_t bt_service_init(transport_if_t *transport);

#if defined(__cplusplus)
}
#endif

#endif /* __BT_SERVICE_BTS_H__ */