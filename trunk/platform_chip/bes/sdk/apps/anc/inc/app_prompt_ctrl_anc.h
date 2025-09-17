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
#ifndef __APP_PROMPT_CTRL_ANC_H__
#define __APP_PROMPT_CTRL_ANC_H__

#include "app_anc.h"

#define ANC_VOLUME_QTY       (3)

#if ANC_VOLUME_QTY > 4
#pragma message ("Invalid ANC_VOLUME_QTY define!")
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint32_t app_prompt_ctrl_anc_init(uint32_t aud_id);
int32_t app_prompt_ctrl_anc_process(void);

bool is_prompt_ctrl_anc(uint32_t aud_id);
uint32_t app_prompt_ctrl_anc(app_anc_mode_t mode);
uint32_t app_prompt_ctrl_anc_volume(uint16_t need_update_mode, uint16_t vol_l, uint16_t vol_r);

int32_t app_prompt_ctrl_music_process(void);
int32_t app_prompt_ctrl_music_check_stop(void);

#ifdef __cplusplus
}
#endif

#endif
