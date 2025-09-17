
/***************************************************************************
 *
 * Copyright 2020-2023 BES.
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
#ifndef __HAL_CMU_PRI_BEST1306P_H__
#define __HAL_CMU_PRI_BEST1306P_H__

#define HAL_CMU_RC_STABLE_TIME          550
#define HAL_CMU_OSC_STABLE_TIME         2000

#ifdef BT_LOG_POWEROFF
#define BT_CMU_OSC_READY_TIMEOUT_US     (HAL_CMU_OSC_STABLE_TIME)
#define BT_RESERVED_LPO_CNT             19
#define BT_ADDITIONAL_LPO_CNT           8
#else
#define BT_CMU_26M_READY_TIMEOUT_US    (HAL_CMU_OSC_STABLE_TIME)
#define BT_CMU_OSC_READY_TIMEOUT_US    (HAL_CMU_OSC_STABLE_TIME + 100)
#define BT_CMU_WEXT_READY_TIMEOUT_US   (HAL_CMU_OSC_STABLE_TIME)
#endif

#define HAL_CMU_PLL_LOCKED_TIMEOUT_US   200
#define HAL_CMU_X4_LOCKED_TIMEOUT_US    140 /* >=(2.5 lpo cycles + 10)us */
#define HAL_CMU_LPU_EXTRA_TIMEOUT_US    0
#define HAL_CMU_26M_READY_TIMEOUT_US    HAL_CMU_OSC_STABLE_TIME

#define HAL_CMU_PLL_LOCKED_TIMEOUT      US_TO_TICKS(HAL_CMU_PLL_LOCKED_TIMEOUT_US)
#define HAL_CMU_26M_READY_TIMEOUT       US_TO_TICKS(HAL_CMU_26M_READY_TIMEOUT_US)
#define HAL_CMU_LPU_EXTRA_TIMEOUT       US_TO_TICKS(HAL_CMU_LPU_EXTRA_TIMEOUT_US)

#endif
