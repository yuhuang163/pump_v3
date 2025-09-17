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
#ifndef __EMMC_CONFIG_H__
#define __EMMC_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "host.h"

/* The heap's buffer size(in bytes) for emmc driver using */
#define EMMC_HEAP_BUFF_SIZE (128 * 1024)

struct emmc_config {
    /*pmu*/
    unsigned int vmin_begin;

    /*test*/
    unsigned int controller;
    unsigned int test_id;
    unsigned int subtest_id;
    unsigned int real_card;

    /*platform*/
    unsigned int src_region;
    unsigned int dst_region;
    unsigned int aligned;
    unsigned int clk_drv;
    unsigned int cmd_drv;
    unsigned int dat_drv;
    unsigned int vcc_main;
    unsigned int rst_pin;
    unsigned int pwr_3p3_pin;
    unsigned int pwr_1p8_pin;
    unsigned int pwr_on_level;

    /*data transfer*/
    unsigned int blksz;
    unsigned int blks;
    unsigned int card_offset;
    unsigned int seed;
    unsigned int pattern;

    /*host mode*/
    unsigned int dma_mode;
    unsigned int dma_bdry;
    unsigned int desc_type;
    unsigned int desc_sz;
    unsigned int desc_nr;
    unsigned int auto_cmd_type;
    unsigned char clk_gating_en;
    unsigned char resp_polling_mode;//0: interrupt mode, 1:polling mode

    /*bus io*/
    unsigned int timing;
    unsigned int khz;//Only valid in DS mode
    unsigned int width;
    unsigned int voltage;
    unsigned int strobe;
    unsigned int quirks;

    /*delay*/
    void (*delay)(unsigned int);

    /*callback*/
    void (*emmc_host_error)(enum EMMC_HOST_ERR error);
    void (*emmc_host_dma_done)(void);
};

#ifdef __cplusplus
}
#endif

#endif
