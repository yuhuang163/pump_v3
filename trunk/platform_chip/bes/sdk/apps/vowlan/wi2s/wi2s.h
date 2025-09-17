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
#ifndef __WI2S_H__
#define __WI2S_H__

#ifdef __cplusplus
extern "C" {
#endif

void wi2s_init(VOW_DEVICE *vodev, SceneParam_t *sceneparam);
void wi2s_session_updata(VOW_DEVICE *vodev, SceneParam_t *sceneparam);
void vow_i2s_capture_start(void);
void vow_i2s_capture_stop(void);
#ifdef __cplusplus
}
#endif

#endif  /* __WI2S_H__ */