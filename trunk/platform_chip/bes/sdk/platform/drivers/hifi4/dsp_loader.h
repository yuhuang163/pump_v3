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
#ifndef __DSP_LOADER_H__
#define __DSP_LOADER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dsp_hifi4_ipc.h"

int dsp_open(void);

int dsp_close(void);

uint32_t dsp_get_load_base(void);

#ifdef __cplusplus
}
#endif

#endif
