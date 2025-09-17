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
#ifndef __CSI_SENS_SC132GS_H__
#define __CSI_SENS_SC132GS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "video_public.h"

struct VP_DEVICE_OPS_T *sc132gs_sensor_ops_get(void);

uint32_t sc132gs_sensor_calc_mipi_clock(uint32_t width, uint32_t height, uint32_t bits, uint32_t fps);

#ifdef __cplusplus
}
#endif

#endif
