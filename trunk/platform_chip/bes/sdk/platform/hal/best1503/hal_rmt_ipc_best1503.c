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
#include "hal_rmt_ipc_pri.h"
#include "hal_trace.h"

extern const struct HAL_RMT_IPC_CFG_T mcu2cp_cfg;

const struct HAL_RMT_IPC_CFG_T *hal_rmt_ipc_get_cfg(enum HAL_RMT_IPC_CORE_T core)
{
    if (0) {
#ifdef CHIP_ROLE_CP
    } else if (core == HAL_RMT_IPC_CORE_MCU_C0) {
        return &mcu2cp_cfg;
#elif defined(CP_AS_SUBSYS)
    } else if (core == HAL_RMT_IPC_CORE_MCU_C1) {
        return &mcu2cp_cfg;
#endif
    }
    return NULL;
}
