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
#ifndef __PCIEPHY_H__
#define __PCIEPHY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(pciephy)
#include "hal_analogif.h"
#include "hal_phyif.h"

#ifdef SPIPHY_PCIE_REG
#define SPI_PCIEPHY_REG(reg)             SPIPHY_PCIE_REG(reg)
#elif defined(ISPI_PCIEPHY_REG)
#define SPI_PCIEPHY_REG(reg)             ISPI_PCIEPHY_REG(reg)
#else
#define SPI_PCIEPHY_REG(reg)             (reg)
#endif

#ifdef SPIPHY_BASE
#define pciephy_read(reg, val)           hal_phyif_reg_read(SPI_PCIEPHY_REG(reg), val)
#define pciephy_write(reg, val)          hal_phyif_reg_write(SPI_PCIEPHY_REG(reg), val)
#else
#define pciephy_read(reg, val)           hal_analogif_reg_read(SPI_PCIEPHY_REG(reg), val)
#define pciephy_write(reg, val)          hal_analogif_reg_write(SPI_PCIEPHY_REG(reg), val)
#endif

void pciephy_open(void);

void pciephy_close(void);

void pciephy_sleep(void);

void pciephy_wakeup(void);

void pciephy_open_state_intr(void);

#ifdef __cplusplus
}
#endif

#endif

