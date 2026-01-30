/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef SPI_NANDFLASH_DRV_H
#define SPI_NANDFLASH_DRV_H

#include "plat_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------------------------
// API
//---------------------------------------------------------------------------
void spi_nandflash_drv_set_mode(uint32_t mode);
bool spi_nandflash_drv_match_chip(const uint8_t *flash_id, uint32_t len);
bool spi_nandflash_drv_raw_block_is_bad(uint32_t block);
uint32_t spi_nandflash_drv_get_page_oob_size(void);
int spi_nandflash_drv_get_size(uint32_t *total_size, uint32_t *block_size, uint32_t *page_size, uint32_t *page_oob_size, uint32_t *die_num);
int spi_nandflash_drv_get_id(uint8_t *flash_id, uint32_t len);
int spi_nandflash_drv_get_unique_id(uint8_t *unique_id, uint32_t len, bool verify);
int spi_nandflash_die_select(uint8_t die_id);
int spi_nandflash_drv_reset(void);
int spi_nandflash_drv_protection_enable(void);
int spi_nandflash_drv_protection_disable(void);
int spi_nandflash_drv_read_raw_page(uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len);
int spi_nandflash_drv_erase_raw_block(uint32_t block);
int spi_nandflash_drv_write_page_offset(uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len);
int spi_nandflash_drv_update_page_offset(uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len);
void spi_nandflash_drv_dump_reg(void);

// Error code.
#define SPI_NANDFLASH_DRV_OK   0
#define SPI_NANDFLASH_DRV_ERROR 1

#ifdef __cplusplus
}
#endif

#endif /*SPI_NANDFLASH_DRV_H*/

