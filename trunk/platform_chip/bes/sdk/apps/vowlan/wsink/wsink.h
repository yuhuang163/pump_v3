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
#ifndef __WSINK_H__
#define __WSINK_H__

#ifdef __cplusplus
extern "C" {
#endif

void wsink_init(VOW_DEVICE *vodev, SceneParam_t *sceneparam);

void wsink_playback_vol_changed(uint32_t level);

#ifdef __cplusplus
}
#endif

#endif  /* __WSINK_H__ */