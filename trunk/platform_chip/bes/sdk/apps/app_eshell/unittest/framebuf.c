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

#if defined(UTILS_ESHELL_EN) && defined(PSRAM_ENABLE)
#include "cmsis.h"
#include "plat_types.h"
#include "plat_addr_map.h"
#include "hal_location.h"

PSRAM_BSS_LOC ALIGNED(64) uint8_t psram_big_buffer0[1024*1024];
PSRAM_BSS_LOC ALIGNED(64) uint8_t psram_big_buffer1[1024*1024];
void *frame_test_buf0 = psram_big_buffer0;
void *frame_test_buf1 = psram_big_buffer1;

#endif
