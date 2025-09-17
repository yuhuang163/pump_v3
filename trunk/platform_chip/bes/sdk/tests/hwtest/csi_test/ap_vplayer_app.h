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
#ifndef __AP_VPLAYER_APP_H__
#define __AP_VPLAYER_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vplayer_app_common.h"

int ap_vplayer_app_init(enum VPLAYER_APP_MODE_T mode);

void ap_vplayer_app_open(enum VPLAYER_APP_MODE_T mode);

void ap_vplayer_app_close(enum VPLAYER_APP_MODE_T mode);

void ap_vplayer_app_on(bool on);

void ap_vplayer_app_trigger(bool on);

typedef void (*AP_VPLAYER_STREAM_READY_NOTIFY_T)(uint32_t id, uint32_t stream, uint32_t addr, uint32_t len, uint32_t seq);

void ap_vplayer_app_register_stream_ready(AP_VPLAYER_STREAM_READY_NOTIFY_T fn);

#ifdef __cplusplus
}
#endif

#endif /* __AP_VPLAYER_APP_H__ */
