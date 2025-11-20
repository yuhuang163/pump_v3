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
#ifndef __PSRAM_DRV_H__
#define __PSRAM_DRV_H__

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"

struct PSRAM_T{
    uint32_t cfg_freq_mhz;
    uint32_t run_freq_mhz[HAL_PSRAM_ID_NUM];
    uint8_t  psram_type[HAL_PSRAM_ID_NUM];
    uint8_t  psram_data_bus[HAL_PSRAM_ID_NUM];
    enum PSRAM_PHY_BYPASS_CFG_T phy_bypass_cfg[HAL_PSRAM_ID_NUM];
    // char*    info[];
};

void psram_init(void);
void psram_open(enum HAL_PSRAM_ID_T id, const struct PSRAM_T *psram);
void psram_wakeup_init(void);
void psram_dll_dly_init(const struct PSRAM_T *psram);
void psram_anaphy_calib(const struct PSRAM_T *psram);
void psram_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk);
bool psram_window_2d_cb_exec(void * const array, uint8_t volume, uint8_t dly_inc, int byte_pos, const struct HAL_PSRAM_RECTANGLE_T *rectangle);

void psram_auto_lowpower_init(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);
void psram_struct_init(struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg);

void psram_opi_struct_init (struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg);
void psram_opi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);
void psram_opi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk);
void psram_opi_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, struct HLA_PSRAM_DLL_DLY_VAL_T *v);
void psram_opi_calib(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk);

void psram_xopi_struct_init (struct PSRAM_MC_CFG_T *mc_cfg, struct PSRAM_PHY_CFG_T *phy_cfg, struct PSRAM_CFG_T *psram_cfg);
void psram_xopi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);
void psram_xopi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk);
void psram_xopi_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, struct HLA_PSRAM_DLL_DLY_VAL_T *v);
void psram_xopi_calib(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk);

#endif /* __PSRAM_DRV_H__ */
