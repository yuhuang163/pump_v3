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
#ifndef __PSRAM_TIMING_H__
#define __PSRAM_TIMING_H__

#include "plat_types.h"

#define PSRAM_CLK_24MHZ   (24*1000*1000)
#define PSRAM_CLK_48MHZ   (48*1000*1000)
#define PSRAM_CLK_100MHZ  (100*1000*1000)
#define PSRAM_CLK_133MHZ  (133*1000*1000)
#define PSRAM_CLK_200MHZ  (200*1000*1000)
#define PSRAM_CLK_225MHZ  (225*1000*1000)

struct PSRAM_DEVICE_CMD_TABLE_T {
    uint8_t  cmd_array_rd;
    uint8_t  cmd_array_wr;
    uint8_t  cmd_reg_rd;
    uint8_t  cmd_reg_wr;
    uint8_t  cmd_auto_refr;
    uint8_t  cmd_self_refr;
    uint8_t  cmd_hslp_entry;
    uint8_t  cmd_glbrst;
    uint8_t  cmd_nop;
};

struct PSRAM_AC_TIMING_T {
    uint32_t freq_mhz;
    uint8_t  wl;
    uint8_t  rl_min;
    uint8_t  rl_max;
    uint8_t  t_rc;
    uint8_t  t_cphr;
    uint8_t  t_cphw;
    uint8_t  t_mrr;
    uint8_t  t_mrs;
    uint16_t t_cem;
    uint16_t t_rst;
    uint32_t t_hs;
    uint8_t  t_xphs;
    uint32_t t_xhs;
    uint8_t t_dqsck;
    //useless in xccela
    uint8_t  t_cke;
    uint8_t  t_cscke;
    uint8_t  t_csckeh;
    // useless for low speeed psram
    uint8_t  t_srf;  /// ?useless
    uint8_t  t_xsr;  /// ?useless
    uint16_t t_wpst;
    uint16_t t_xckd;
    uint16_t t_eckd;
    uint16_t t_zacal;
    uint16_t t_zqcrst;
    uint16_t t_refi;
    uint8_t  t_rfc;
    uint16_t t_precharge;
    uint16_t t_dvdd;
    uint16_t t_avddq;
    uint16_t t_lock;
};

struct PSRAM_DEVICE_TIMING_T {
    uint8_t  wl_def;
    uint8_t  rl_max_def;
    uint8_t  rl_min_def;
    uint8_t  bl_mrs;
    uint32_t t_xphs_min_ns;
    float    t_dqsck_max_ns;     // ? t_dqsck impact the rising eadeg if CE (delay)?
    bool     is_mrs_wl_fixed;
    uint8_t  fixed_wl_mrs;
    bool     mrs_as_pd_en;
    uint8_t  mrs_as_pd_mr;
    uint8_t  mrs_as_pd_md;

    const struct PSRAM_AC_TIMING_T *ac_timing;
    uint32_t ac_timing_count;
};


///:NOTE: add psram timing here
const extern struct PSRAM_DEVICE_TIMING_T opi_device_timing;
const extern struct PSRAM_DEVICE_CMD_TABLE_T opi_cmd_table;
const extern struct PSRAM_DEVICE_TIMING_T xopi_device_timing;
const extern struct PSRAM_DEVICE_CMD_TABLE_T xopi_cmd_table;

#endif /* __PSRAM_TIMING_H__ */
