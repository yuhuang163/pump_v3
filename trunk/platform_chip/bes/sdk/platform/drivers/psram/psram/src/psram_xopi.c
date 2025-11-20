/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifdef PSRAM_CTRL_ENABLE

#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_timer.h"
#include "pmu.h"
#if (CHIP_PSRAM_CTRL_VER == 10)
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#elif (CHIP_PSRAM_CTRL_VER == 11)
#include "reg_psram_mc_v11.h"
#include "reg_psram_phy_v11.h"
#endif
#include "string.h"

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"
#include "psram_drv.h"

///:NOTE: add psram timing here
const extern struct PSRAM_DEVICE_TIMING_T xopi_device_timing;
const extern struct PSRAM_DEVICE_CFG_T xopi_device_cfg;
const extern struct PSRAM_DEVICE_CMD_TABLE_T xopi_cmd_table;
const extern struct PSRAM_DEVICE_FUNC_CFG_T xopi_device_func_cfg;


void SRAM_TEXT_LOC psram_xopi_struct_init(struct PSRAM_CFG_T *psram_cfg)
{
    psram_cfg->device_timing = &xopi_device_timing;
    psram_cfg->cmd = &xopi_cmd_table;
    psram_cfg->device_func_cfg = &xopi_device_func_cfg;
    psram_cfg->device_cfg = &xopi_device_cfg;
}

/// TODO: Use macro to define MR value
void psram_xopi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->device_func_cfg->psram_set_mr == NULL) {
        PSRAM_TRACE(0,"psram set mr fail...");
        return;
    }

    psram_cfg->device_func_cfg->psram_set_mr(id, psram_cfg);
}

void psram_xopi_mr_show(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->device_func_cfg->psram_show_mr == NULL) {
        PSRAM_TRACE(0,"psram show mr fail...");
        return;
    }

    psram_cfg->device_func_cfg->psram_show_mr(id);
}

void SRAM_TEXT_LOC psram_xopi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    if (psram_cfg->device_func_cfg->psram_get_mr == NULL) {
        PSRAM_TRACE(0,"psram get mr fail...");
        return;
    }

    psram_cfg->device_func_cfg->psram_get_mr(psram_cfg, reg, val, clk);
}

#endif
