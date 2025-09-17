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
#if defined(CHIP_HAS_APSRAM) && (CHIP_APSRAM_CTRL_VER == 1)
#include "plat_types.h"
#include "plat_addr_map.h"

#include "cmsis.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_apsram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "pmu.h"
#include "string.h"
#include "reg_apsram_v1.h"
#include "hal_cmu.h"
#include "plat_addr_map.h"
#include "hal_trace.h"

#ifdef APSRAM_DEBUG
#define APSRAM_TRACE(a, s, ...)      TRACE_IMM(a, "APSRAM_DEBUG: " s , ##__VA_ARGS__)
#else
#define APSRAM_TRACE(a, s, ...)
#endif

#define T_NCK_TO_NS(clk, n)          ((uint32_t)(((n) * 1000) / ((clk) / 1000000)))
#define T_NCK_TO_US(clk, n)          ((n < 100) ? 1 : ((uint32_t)(((n) * 1000) / ((clk) / 1000)) + 1))

#define APSRAM_CLK_100MHZ            (100*1000*1000)
#define APSRAM_CLK_500MHZ            (500*1000*1000)

// #define APSRAM_CLOCK_GATE_ENABLE

enum APSRAM_MR_CFG_T {
    APSRAM_MR_0,
    APSRAM_MR_1,
    APSRAM_MR_2,
    APSRAM_MR_3,
};

struct APSRAM_MR_T {
    uint8_t mr0;
    uint8_t mr1;
    uint8_t mr2;
    uint8_t mr3;
};

struct APSRAM_CONFIG_T {
    uint32_t repair;
    uint32_t crc_val;
    uint32_t turn_on_soft;
    struct APSRAM_MR_T *mr_cfg;
};

static struct APSRAM_MR_T apsram_mr = {
    .mr0 = 0x12,
    .mr1 = 0x0,
    .mr2 = 0xff,
    .mr3 = 0x0,
};

static struct APSRAM_CONFIG_T apsram_mc_cfg = {
    .repair              = 0,
    .crc_val             = 0x1410000,
    .turn_on_soft        = 0x400,
};

static struct APSRAM_CTRL_T *const apsram = (struct APSRAM_CTRL_T *)SYS_APSRAM_CTRL_BASE;
// static struct APSRAM_MC_T *const apsram_mc = (struct APSRAM_MC_T *)SYS_APSRAM_MC_BASE;
static struct APSRAM_SCAN_T *const apsram_scan = (struct APSRAM_SCAN_T *)SYS_APSRAM_SCAN_BASE;

/********************************* hal **************************************/
void hal_apsram_open(struct APSRAM_CONFIG_T *apsram_cfg)
{
    //close repair and mapping
    // apsram_mc->REG_020 = apsram_cfg->repair;

    hal_sys_timer_delay_us(1);

    apsram_scan->REG_210 = apsram_cfg->crc_val;
    hal_sys_timer_delay_us(1);

    hal_sys_timer_delay_us(200);
    apsram_scan->REG_220 = apsram_cfg->turn_on_soft;
    hal_sys_timer_delay_us(200);

    hal_sys_timer_delay_us(1);
    APSRAM_TRACE(0, "REG_220 : val = 0x%x", apsram_scan->REG_220);
    hal_sys_timer_delay_us(200);
}

void hal_apsram_write_fifo(uint32_t val)
{
    apsram_scan->REG_230 = val;
    hal_sys_timer_delay_us(12);
}

void hal_apsram_read_fifo(uint32_t *val)
{
    *val = apsram_scan->REG_230;
    hal_sys_timer_delay_us(12);
}

void hal_apsram_mr_write(enum APSRAM_MR_CFG_T mr, struct APSRAM_MR_T * mr_val)
{
    uint32_t val;

    switch (mr)
    {
    case APSRAM_MR_0:
        val = mr_val->mr0 << 24;
        break;
    case APSRAM_MR_1:
        val = (mr_val->mr0 << 24 | mr_val->mr1 << 16);
        break;
    case APSRAM_MR_2:
        val = (mr_val->mr0 << 24 | mr_val->mr1 << 16 | mr_val->mr2 << 8);
        break;

    default:
        val = (mr_val->mr0 << 24 | mr_val->mr1 << 16 | mr_val->mr2 << 8 | mr_val->mr3);
        break;
    }

    APSRAM_TRACE(0, "get mr:%d = 0x%x", mr, val);

    hal_apsram_write_fifo(val);
}

void hal_apsram_mr_read(uint32_t *val)
{
    hal_sys_timer_delay_us(1);
    apsram_scan->REG_220 = 0x430;
    hal_sys_timer_delay_us(12);

    hal_apsram_read_fifo(val);
}

void hal_apsram_clk_gate()
{
    apsram->REG_084 &= ~APSRAM_CTRL_CACTIVE_OUT_M_MAN;
}

/********************************* driver **************************************/
void hal_apsram_cfg_init(struct APSRAM_CONFIG_T *apsram_cfg)
{
    apsram_cfg->repair = apsram_mc_cfg.repair;
    apsram_cfg->crc_val = apsram_mc_cfg.crc_val;
    apsram_cfg->turn_on_soft = apsram_mc_cfg.turn_on_soft;

    apsram_cfg->mr_cfg = &apsram_mr;
}

void hal_apsram_mr_set(struct APSRAM_CONFIG_T *apsram_cfg)
{
    uint32_t val;

    hal_apsram_mr_read(&val);
    APSRAM_TRACE(0, "get mr = 0x%x", val);

    //set mr0
    hal_apsram_mr_write(APSRAM_MR_0, apsram_cfg->mr_cfg);
    //set mr2
    hal_apsram_mr_write(APSRAM_MR_2, apsram_cfg->mr_cfg);

    hal_sys_timer_delay_us(12);
    hal_apsram_mr_read(&val);
    APSRAM_TRACE(0, "get mr = 0x%x", val);
}

void hal_apsram_mc_set_freq(uint32_t clk)
{
    enum HAL_CMU_FREQ_T freq;

    if (clk <= 26000000) {
        freq = HAL_CMU_FREQ_26M;
    } else if (clk <= 52000000) {
        freq = HAL_CMU_FREQ_52M;
    } else if (clk <= 104000000) {
        freq = HAL_CMU_FREQ_104M;
    } else if (clk <= 208000000) {
        freq = HAL_CMU_FREQ_208M;
    } else {
        freq = HAL_CMU_FREQ_208M;
    }

    hal_cmu_mem_set_freq(freq);
}

void hal_apsram_init(void)
{
    struct APSRAM_CONFIG_T apsram_cfg;

    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);

    pmu_apsram_freq_config(APSRAM_SPEED * 1000000);

    hal_cmu_apsram_clock_enable();
#ifdef APSRAM_CLOCK_GATE_ENABLE
    hal_cmu_apsram_clock_gate_enable();
#endif

    hal_apsram_mc_set_freq(APSRAM_SPEED * 1000000);

    APSRAM_TRACE(1,"APSRAM Init, APSRAM speed = %dMHz", APSRAM_SPEED * 1000000);

    hal_apsram_cfg_init(&apsram_cfg);

    hal_apsram_open(&apsram_cfg);

    // hal_apsram_mr_set(&apsram_cfg);
#ifdef APSRAM_CLOCK_GATE_ENABLE
    hal_apsram_clk_gate();
#endif
}

#endif

