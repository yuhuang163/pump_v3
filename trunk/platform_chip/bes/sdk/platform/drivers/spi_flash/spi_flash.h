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
#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_cmu.h"

#define SPI_FLASH_DEVICE_ID_LEN          3
#define SPI_FLASH_UNIQUE_ID_LEN          16
#define SPI_FLASH_CP_ID_LEN              2
#define INVALID_SPIFLASH_IDX             0xFF

enum SPI_FLASH_ID_T {
    SPI_FLASH_ID_0 = 0,
    SPI_FLASH_ID_NUM,
};

enum SPI_FLASH_TYPE_T {
    SPI_FLASH_TYPE_NORFLASH,
    SPI_FLASH_TYPE_NANDFLASH,
};

enum SPI_FLASH_RET_T {
    SPI_FLASH_OK,
    SPI_FLASH_ERR,
    SPI_FLASH_GET_ID_FAIL,
    SPI_FLASH_BAD_ID,
    SPI_FLASH_BAD_IDX,
    SPI_FLASH_BAD_ADDR,
    SPI_FLASH_BAD_LEN,
    SPI_FLASH_BUFF_IS_NULL,
    SPI_FLASH_NOT_OPENED,
    SPI_FLASH_CFG_NULL,
    SPI_FLASH_SPI_INIT_ERROR,
    SPI_FLASH_SPI_SEND_ERROR,
    SPI_FLASH_SPI_RECV_ERROR,
    SPI_FLASH_SEMAPHORE_ERROR,
    SPI_FLASH_CHECK_ID_ERROR,
    SPI_FLASH_READ_ERROR,
    SPI_FLASH_WRITE_ERROR,
    SPI_FLASH_ERASE_ERROR,
    SPI_FLASH_RESET_ERROR,
    SPI_FLASH_STATUS_ERROR,
    SPI_FLASH_BAD_BLOCK,
    SPI_FLASH_BAD_PAGE,
    SPI_FLASH_BAD_OFFSET,
    SPI_FLASH_CFG_ERROR,
    SPI_FLASH_DATA_VERIFY_ERROR,
    SPI_FLASH_READ_CORRECTED,
};

enum SPI_FLASH_OP_MODE {
    // (1) basic read mode
    // standard spi mode
    SPI_FLASH_OP_MODE_STAND_SPI      = (1 << 0),
    // fast spi mode.
    SPI_FLASH_OP_MODE_FAST_SPI       = (1 << 1),

    // (2)nandflash
    HAL_NANDFLASH_OP_MODE_INTERNAL_ECC   = (1 << 16),

    SPI_FLASH_OP_MODE_RESERVED       = 0xFFFFFFFF,
};

struct SPI_FLASH_CONFIG_T {
    enum SPI_FLASH_OP_MODE mode;
};

/* spi flash apis */
enum SPI_FLASH_RET_T spi_flash_open(enum SPI_FLASH_ID_T id, uint32_t spi_rate, uint32_t cs_pin);
enum SPI_FLASH_RET_T spi_flash_close(enum SPI_FLASH_ID_T id);
enum SPI_FLASH_RET_T spi_flash_get_type(enum SPI_FLASH_ID_T id, enum SPI_FLASH_TYPE_T *type);
bool spi_flash_opened(enum SPI_FLASH_ID_T id);

/* spi norflash apis */
enum SPI_FLASH_RET_T spi_norflash_get_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum SPI_FLASH_RET_T spi_norflash_get_unique_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum SPI_FLASH_RET_T spi_norflash_get_size(enum SPI_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size,
                                         uint32_t *sector_size, uint32_t *page_size);
enum SPI_FLASH_RET_T spi_norflash_read(enum SPI_FLASH_ID_T id, uint32_t addr, uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_norflash_write(enum SPI_FLASH_ID_T id, uint32_t addr, uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_norflash_erase(enum SPI_FLASH_ID_T id, uint32_t addr, uint32_t len);
enum SPI_FLASH_RET_T spi_norflash_reset(enum SPI_FLASH_ID_T id);

/* spi nandflash apis */
bool spi_nandflash_block_is_bad(enum SPI_FLASH_ID_T id, uint32_t block);
enum SPI_FLASH_RET_T spi_nandflash_get_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_get_size(enum SPI_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size,
                                         uint32_t *page_size, uint32_t *page_oob_size, uint32_t *die_num);
enum SPI_FLASH_RET_T spi_nandflash_get_unique_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_mark_bad_block(enum SPI_FLASH_ID_T id, uint32_t block);
enum SPI_FLASH_RET_T spi_nandflash_read_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         uint32_t offset, uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_read_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         uint32_t offset, uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_erase_block(enum SPI_FLASH_ID_T id, uint32_t block);
enum SPI_FLASH_RET_T spi_nandflash_write_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         const uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_write_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         const uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_update_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         uint32_t offset, const uint8_t *buffer, uint32_t len);
enum SPI_FLASH_RET_T spi_nandflash_update_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page,
                                         uint32_t offset, const uint8_t *buffer, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* SPI_FLASH_H */

