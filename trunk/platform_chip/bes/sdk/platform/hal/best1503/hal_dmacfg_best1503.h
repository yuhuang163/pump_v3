
/***************************************************************************
 *
 * Copyright 2020-2022 BES.
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
#ifndef __HAL_DMACFG_BEST1503_H__
#define __HAL_DMACFG_BEST1503_H__
#include "plat_addr_map.h"

#if defined(CHIP_DMA_CFG_IDX) && (CHIP_DMA_CFG_IDX == 1)
#include CHIP_SPECIFIC_HDR(hal_dmacfg1)
#else
#include CHIP_SPECIFIC_HDR(hal_dmacfg0)
#endif

#endif
