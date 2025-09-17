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
#include "common_debug.h"

#define COMMON_CMD(n)           {TO_STRING(n), CONCAT(common_debug_, n)},
#define rf_read(reg, val)       hal_analogif_reg_read (RF_REG(reg), val)
#define rf_write(reg, val)      hal_analogif_reg_write(RF_REG(reg), val)
#ifdef CHIP_BEST1600
#define rc_read(reg, val)       hal_analogif_reg_read (RC_REG(reg), val)
#define rc_write(reg, val)      hal_analogif_reg_write(RC_REG(reg), val)
#endif

static size_t reg, val;

static void common_debug_ana_w(void) {
    CMD_TRACE("[0x%04X][W]:\t0x%04X", reg, val);
    analog_write(reg, val);
}

static void common_debug_ana_r(void)
{
    analog_read(reg, (uint16_t *)&val);
    CMD_TRACE("[0x%04X][R]:\t0x%04X", reg, val);
}

static void common_debug_w4(void)
{
    CMD_TRACE("[0x%08X][W]:\t0x%08X", reg, val);
    *(volatile uint32_t *)(reg) = val;
}

static void common_debug_r4(void)
{
    val = *(volatile uint32_t *)(reg);
    CMD_TRACE("[0x%08X][R]:\t0x%08X", reg, val);
}

static void common_debug_rf_w(void)
{
    CMD_TRACE("[0x%04X][W]:\t\t0x%04X", reg, val);
    rf_write(reg, val);
}

static void common_debug_rf_r(void)
{
    rf_read(reg, (uint16_t *)&val);
    CMD_TRACE("[0x%04X][R]:\t\t0x%04X", reg, val);
}

static void common_debug_pmu_w(void)
{
    CMD_TRACE("[0x%04X][W]:\t0x%04X", reg, val);
    pmu_write(reg, val);
}

static void common_debug_pmu_r(void)
{
    pmu_read(reg, (uint16_t *)&val);
    CMD_TRACE("[0x%04X][R]:\t0x%04X", reg, val);
}

#ifdef CHIP_BEST1600
static void common_debug_rc_w(void)
{
    CMD_TRACE("[0x%04X][W]:\t\t0x%04X", reg, val);
    rc_write(reg, val);
}

static void common_debug_rc_r(void)
{
    rc_read(reg, (uint16_t *)&val);
    CMD_TRACE("[0x%04X][R]:\t\t0x%04X", reg, val);
}

static void common_debug_pmui_w(void)
{
    CMD_TRACE("[0x%04X][W]:\t0x%04X", reg, val);
    pmu_write(ITNPMU_REG(reg), val);
}

static void common_debug_pmui_r(void)
{
    pmu_read(ITNPMU_REG(reg), (uint16_t *)&val);
    CMD_TRACE("[0x%04X][R]:\t0x%04X", reg, val);
}
#endif

static const common_debug_handler_t common_debug_handler[] = {
    COMMON_CMD(ana_w)
    COMMON_CMD(ana_r)
    COMMON_CMD(w4)
    COMMON_CMD(r4)
    COMMON_CMD(rf_w)
    COMMON_CMD(rf_r)
    COMMON_CMD(pmu_w)
    COMMON_CMD(pmu_r)
#ifdef CHIP_BEST1600
    COMMON_CMD(rc_w)
    COMMON_CMD(rc_r)
    COMMON_CMD(pmui_w)
    COMMON_CMD(pmui_r)
#endif
};

void common_debug(const char **params)
{
    STR_TO_HEX(reg, params[1]);
    STR_TO_HEX(val, params[2]);
#if 0
    PARAM_CHK(REG_CHK(reg));
#endif
    CMD_HANDLER(common_debug, params[0]);
}