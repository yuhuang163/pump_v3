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
#ifndef __PSRAM_DRV_H__
#define __PSRAM_DRV_H__

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_debug.h"

#define PSRAM_SWAP8BITS(val)            (((val & 0xff00) >> 8) | ((val & 0xff) << 8))

struct PSRAM_T {
    uint32_t cfg_freq_mhz;
    uint32_t run_freq_mhz[HAL_PSRAM_ID_NUM];
    uint8_t  psram_type[HAL_PSRAM_ID_NUM];
    uint8_t  psram_data_bus[HAL_PSRAM_ID_NUM];
    uint8_t  psram_die_name[HAL_PSRAM_ID_NUM];
    enum PSRAM_PHY_BYPASS_CFG_T phy_bypass_cfg[HAL_PSRAM_ID_NUM];
    // char*    info[];
};

void psram_anaphy_calib(const struct PSRAM_T *psram);

void psram_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk);

void psram_mr_setting(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_mr_show(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_calib(const struct PSRAM_T *psram);

int psram_type_bus_init(struct PSRAM_T *psram);

void psram_struct_init(struct PSRAM_CFG_T *psram_cfg);

int psram_die_check(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_xopi_struct_init (struct PSRAM_CFG_T *psram_cfg);

void psram_xopi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_xopi_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk);

void psram_xopi_set_dly(enum HAL_PSRAMPHY_ID_T phy_id, enum HAL_PSRAM_PHY_CHANNEL_T c, struct HLA_PSRAM_DLL_DLY_VAL_T *v);

void psram_xopi_mr_show(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_opi_struct_init(struct PSRAM_CFG_T *psram_cfg);

void psram_opi_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_opi_mr_show(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_hyper_struct_init(struct PSRAM_CFG_T *psram_cfg);

void psram_hyper_mr_set(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

void psram_hyper_sleep(enum HAL_PSRAM_ID_T id);

void psram_device_sleep(struct PSRAM_CFG_T *psram_cfg);

uint8_t psram_check_wr_valid(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c);

int find_the_largest_window_of_1d_array(bool *array, uint8_t volume);

int find_the_largest_window_of_2d_array(void *array, uint8_t row_column_num, int byte_pos, struct HAL_PSRAM_RECTANGLE_T *rectangle);

void psram_reset(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg);

#endif /* __PSRAM_DRV_H__ */
