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
#ifndef SPI_FLASH_TRANS_H
#define SPI_FLASH_TRANS_H

#include "plat_types.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define SPI_CMD_SIZE            32
#define SPI_BUFFER_SIZE         4096
#define SPI_NORFLASH_PAGE_SIZE  4092
//---------------------------------------------------------------------------
// API
//---------------------------------------------------------------------------
int spi_flash_send_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len);
int spi_flash_recv_word(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len);
int spi_flash_send_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len);
int spi_flash_recv_byte(uint8_t *cmd, uint32_t cmd_len, uint8_t *buff, uint32_t buff_len);
int spi_flash_spi_init(const struct HAL_SPI_CFG_T *spi_cfg, uint32_t cs_pin);
int spi_flash_spi_deinit(void);

// Error code.
#define SPI_FLASH_TRANS_OK   0
#define SPI_FLASH_TRANS_ERROR 1

#ifdef __cplusplus
}
#endif

#endif /*SPI_FLASH_TRANS_H*/

