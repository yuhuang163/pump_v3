/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __PSRAMPHY_H__
#define __PSRAMPHY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(psramphy)
#include "hal_analogif.h"
#include "hal_phyif.h"

#ifdef SPIPHY_PSRAM0_REG
#define SPI_PSRAMPHY0_REG(reg)              SPIPHY_PSRAM0_REG(reg)
#elif defined(ISPI_PSRAMPHY0_REG)
#define SPI_PSRAMPHY0_REG(reg)              ISPI_PSRAMPHY0_REG(reg)
#else
#define SPI_PSRAMPHY0_REG(reg)              (reg)
#endif

#ifdef SPIPHY_PSRAM1_REG
#define SPI_PSRAMPHY1_REG(reg)              SPIPHY_PSRAM1_REG(reg)
#elif defined(ISPI_PSRAMPHY1_REG)
#define SPI_PSRAMPHY1_REG(reg)              ISPI_PSRAMPHY1_REG(reg)
#else
#define SPI_PSRAMPHY1_REG(reg)              (reg)
#endif

#ifdef SPIPHY_BASE
#define psramphy0_read(reg, val)            hal_phyif_reg_read(SPI_PSRAMPHY0_REG(reg), val)
#define psramphy0_write(reg, val)           hal_phyif_reg_write(SPI_PSRAMPHY0_REG(reg), val)

#define psramphy1_read(reg, val)            hal_phyif_reg_read(SPI_PSRAMPHY1_REG(reg), val)
#define psramphy1_write(reg, val)           hal_phyif_reg_write(SPI_PSRAMPHY1_REG(reg), val)
#else
#define psramphy0_read(reg, val)            hal_analogif_reg_read(SPI_PSRAMPHY0_REG(reg), val)
#define psramphy0_write(reg, val)           hal_analogif_reg_write(SPI_PSRAMPHY0_REG(reg), val)

#define psramphy1_read(reg, val)            hal_analogif_reg_read(SPI_PSRAMPHY1_REG(reg), val)
#define psramphy1_write(reg, val)           hal_analogif_reg_write(SPI_PSRAMPHY1_REG(reg), val)
#endif

void psramphy_open(uint32_t clk);

void psramphy_init_calib(void);

void psramphy_calib(uint32_t clk);

void psramphy_sleep(void);

void psramphy_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif
