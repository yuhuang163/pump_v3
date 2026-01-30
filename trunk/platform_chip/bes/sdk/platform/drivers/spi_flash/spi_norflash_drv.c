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
#include "plat_addr_map.h"

#ifdef SPI_BASE

#include "plat_types.h"
#include "hal_spi.h"
#include "hal_cmu.h"
#include "hal_norflaship.h"
#include "norflash_gd25q32c.h"
#include "norflash_cfg.h"
#include "norflash_drv.h"
#include "spi_flash_trans.h"
#include "spi_norflash_drv.h"
#include "spi_flash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#ifdef SPI_FLASH_DEBUG
    #define SPIFLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define SPIFLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

#ifdef FLASH_ERASE_PAGE_ENABLE
    #define PUYA_FLASH_CMD_PAGE_ERASE   0x81
#endif

static uint8_t spi_norflash_idx = INVALID_SPIFLASH_IDX;
static const struct NORFLASH_CFG_T  **spi_norflash_list = NULL;
static uint32_t spi_list_len = 0;
static uint32_t spi_norflash_op_mode = 0;
// extened address
// VAL  ADDR BIT  ADDR RANGE
// 0x0: 0-A23     0x0       - 0xFFFFFF
// 0x1: A24       0x1  0x1000000 - 0x1FFFFFF
// 0x2: A25       0x2  0x2000000 - 0x3FFFFFF
static uint8_t _spi_extened_addr_mask = 0;

static void spi_flash_delay(uint32_t us)
{
#ifdef CHIP_BEST1000
    hal_sys_timer_delay(US_TO_TICKS(us));
#else
    hal_sys_timer_delay_us(us);
#endif
}

static int _norflash_busy_wait(void)
{
    int ret;
    uint8_t cmd = GD25Q32C_CMD_READ_STATUS_S0_S7;
    uint8_t status = 0;

    while (1) {
        ret = spi_flash_recv_byte(&cmd, sizeof(cmd), &status, sizeof(status));
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
            break;
        }
        if (!(status & GD25Q32C_WIP_BIT_MASK)) {
            return 0;
        }
    }
    return -1;
}

static int _norflash_write_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_WRITE_ENABLE;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    return ret;
}

static int _norflash_reset_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_ENABLE_RESET;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    _norflash_busy_wait();
    return ret;
}

static int _norflash_reset(void)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_RESET;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    _norflash_busy_wait();
    return ret;
}

static int _norflash_write_extened(uint32_t addr)
{
    int32_t ret;
    uint8_t cmd[2];

    cmd[0] = GD25Q32C_CMD_EXTENDED_ADDR_REGISTER_WRITE;

    if (addr >= (2 * NORFLASH_4BYTE_ADDR_SIZE(0))) {
        cmd[1] = EXTENED_ADDR_A25_BIT_MASK;
    } else if (addr >= NORFLASH_4BYTE_ADDR_SIZE(0)) {
        cmd[1] = EXTENED_ADDR_A24_BIT_MASK;
    } else {
        cmd[1] = 0;
    }

    if (cmd[1] == _spi_extened_addr_mask) {
        return 0;
    }
    _spi_extened_addr_mask = cmd[1];

    _norflash_write_enable();

    ret = spi_flash_send_byte(cmd, sizeof(cmd), NULL, 0);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
    }
    _norflash_busy_wait();
    return ret;
}

//------------------------------------------------------------------
// spi_norflash_drv functions
//------------------------------------------------------------------

void spi_norflash_drv_set_mode(uint32_t mode)
{
    spi_norflash_op_mode = mode;
}

bool spi_norflash_drv_match_chip(const uint8_t *id, uint32_t len)
{
    const uint8_t *cmp_id;
    const struct NORFLASH_CFG_T  *tmp_list;

    norflash_get_flash_list(&tmp_list, &spi_list_len);
    spi_norflash_list = (const struct NORFLASH_CFG_T **)tmp_list;

    if (len == NORFLASH_ID_LEN) {
        for (spi_norflash_idx = 0; spi_norflash_idx < spi_list_len; spi_norflash_idx++) {
            cmp_id = spi_norflash_list[spi_norflash_idx]->id;
            if (id[0] == cmp_id[0] && id[1] == cmp_id[1] && id[2] == cmp_id[2]) {
                return true;
            }
        }
    }

    return false;
}

int spi_norflash_drv_get_size(uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size)
{
    if (total_size) {
        *total_size  = spi_norflash_list[spi_norflash_idx]->total_size;
    }
    if (block_size) {
        *block_size  = NORFLASH_BLOCK_SIZE_32K;
    }
    if (sector_size) {
        *sector_size = NORFLASH_SECTOR_SIZE;
    }
    if (page_size) {
        *page_size   = NORFLASH_PAGE_SIZE;
    }

    return SPI_NORFLASH_DRV_OK;
}

int spi_norflash_drv_get_id(uint8_t *id, uint32_t len)
{
    int32_t ret;
    uint8_t cmd = GD25Q32C_CMD_ID;

    _norflash_busy_wait();
    ret = spi_flash_recv_byte(&cmd, sizeof(cmd), id, len);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
        return SPI_NORFLASH_DRV_ERROR;
    }

    return SPI_NORFLASH_DRV_OK;
}

#ifdef FLASH_UNIQUE_ID
int spi_norflash_drv_get_unique_id(uint8_t *flash_id, uint32_t len)
{
    int32_t ret;
    uint8_t cmd[5] = {GD25Q32C_CMD_UNIQUE_ID, 0,};

    _norflash_busy_wait();
    ret = spi_flash_recv_byte(cmd, sizeof(cmd), flash_id, len);
    if (ret != 0) {
        SPIFLASH_TRACE(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
        return SPI_NORFLASH_DRV_ERROR;
    }

    return SPI_NORFLASH_DRV_OK;
}
#endif

int spi_norflash_drv_read(uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[5];
    uint8_t idx = 0;
    uint32_t dummy_cnt = 0;

    _norflash_busy_wait();
    _norflash_write_extened(addr);

    cur_addr = addr;
    pbuff = buff;
    remain_len = len;

    if (spi_norflash_op_mode & HAL_NORFLASH_OP_MODE_FAST_SPI){
        /* fast */
        cmd[idx] = GD25Q32C_CMD_STANDARD_FAST_READ;
        dummy_cnt = 1;
    } else {
        /* normal */
        cmd[idx] = GD25Q32C_CMD_STANDARD_READ;
    }

    do {
        idx = 1;
        cmd[idx++] = ((cur_addr >> 16) & 0xff);
        cmd[idx++] = ((cur_addr >> 8) & 0xff);
        cmd[idx++] = (cur_addr & 0xff);
        if (dummy_cnt > 0) {
            cmd[idx++] = 0;
        }
        tmp_len = remain_len <= SPI_NORFLASH_PAGE_SIZE ? remain_len : SPI_NORFLASH_PAGE_SIZE;

        if (idx == 4 && (len / 4) * 4 == len) {
            ret = spi_flash_recv_word(cmd, idx, pbuff, tmp_len);
        } else {
            ret = spi_flash_recv_byte(cmd, idx, pbuff, tmp_len);
        }
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_recv_word failed!ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_ERROR;
        }
        cur_addr += tmp_len;
        remain_len -= tmp_len;
        pbuff += tmp_len;
    } while (remain_len);

    return SPI_NORFLASH_DRV_OK;
}

int spi_norflash_drv_write(uint32_t addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[4];

    _norflash_busy_wait();
    _norflash_write_extened(addr);

    cur_addr = addr;
    pbuff = buff;
    remain_len = len;

    do {
        _norflash_write_enable();

        cmd[0] = GD25Q32C_CMD_PAGE_PROGRAM;
        cmd[1] = ((cur_addr >> 16) & 0xff);
        cmd[2] = ((cur_addr >> 8) & 0xff);
        cmd[3] = (cur_addr & 0xff);

        tmp_len = remain_len <= SPI_NORFLASH_PAGE_SIZE ? remain_len : SPI_NORFLASH_PAGE_SIZE;

        ret = spi_flash_send_word(cmd, sizeof(cmd), pbuff, tmp_len);
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_send_word failed!ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_ERROR;
        }

        remain_len -= tmp_len;
        pbuff += tmp_len;
        cur_addr += tmp_len;

        _norflash_busy_wait();
    } while (remain_len);

    return SPI_NORFLASH_DRV_OK;
}

int spi_norflash_drv_erase(uint32_t addr, uint32_t len)
{
    int32_t ret;
    uint32_t erase_len;
    uint8_t cmd[4];
    uint8_t erase_type;
    uint32_t cur_addr;
    uint32_t remain_len;

    _norflash_busy_wait();
    _norflash_write_extened(addr);

    cur_addr = addr;
    remain_len = len;

    do {
        erase_len = 0;
#ifdef FLASH_ERASE_PAGE_ENABLE
        if (cur_addr & (NORFLASH_SECTOR_SIZE - 1)) {
            if (remain_len >= NORFLASH_PAGE_SIZE) {
                erase_len = NORFLASH_PAGE_SIZE;
            }
        } else
#endif
            if (cur_addr & (NORFLASH_BLOCK_SIZE_32K - 1)) {
                if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#else
                    erase_len = 0;
#endif
                }
            } else if (cur_addr & ((NORFLASH_BLOCK_SIZE_32K * 2) - 1)) {
                if (remain_len >= NORFLASH_BLOCK_SIZE_32K) {
                    erase_len = NORFLASH_BLOCK_SIZE_32K;
                } else if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#endif
                }
            } else {
                if (remain_len >= (NORFLASH_BLOCK_SIZE_32K * 2)) {
                    erase_len = (NORFLASH_BLOCK_SIZE_32K * 2);
                } else if (remain_len >= NORFLASH_BLOCK_SIZE_32K) {
                    erase_len = NORFLASH_BLOCK_SIZE_32K;
                } else if (remain_len >= NORFLASH_SECTOR_SIZE) {
                    erase_len = NORFLASH_SECTOR_SIZE;
                } else {
#ifdef FLASH_ERASE_PAGE_ENABLE
                    if (remain_len >= NORFLASH_PAGE_SIZE) {
                        erase_len = NORFLASH_PAGE_SIZE;
                    }
#endif
                }
            }

        if (erase_len == (NORFLASH_BLOCK_SIZE_64K)) {
            erase_type = GD25Q32C_CMD_BLOCK_ERASE_64K;
        } else if (erase_len == NORFLASH_BLOCK_SIZE_32K) {
            erase_type = GD25Q32C_CMD_BLOCK_ERASE_32K;
        } else if (erase_len == NORFLASH_SECTOR_SIZE) {
            erase_type = GD25Q32C_CMD_SECTOR_ERASE;
        }
#ifdef FLASH_ERASE_PAGE_ENABLE
        else if (erase_len == NORFLASH_PAGE_SIZE) {
            erase_type = PUYA_FLASH_CMD_PAGE_ERASE;
        }
#endif
        else {
            ASSERT(0, "%s: erase_len = 0", __func__);
        }

        _norflash_write_enable();

        cmd[0] = erase_type;
        cmd[1] = ((cur_addr >> 16) & 0xff);
        cmd[2] = ((cur_addr >> 8) & 0xff);
        cmd[3] = (cur_addr & 0xff);

        ret = spi_flash_send_word(cmd, sizeof(cmd), NULL, 0);
        if (ret) {
            SPIFLASH_TRACE(1, "%s: _spi_send_word failed!.ret = %d", __func__, ret);
            return SPI_NORFLASH_DRV_ERROR;
        }

        cur_addr += erase_len;
        remain_len -= erase_len;

        _norflash_busy_wait();
    } while (remain_len);

    _norflash_busy_wait();
    return SPI_NORFLASH_DRV_OK;
}

int spi_norflash_drv_reset(void)
{
    int32_t ret;

    _norflash_busy_wait();

    ret = _norflash_reset_enable();
    if (ret) {
        return SPI_NORFLASH_DRV_ERROR;
    }
    spi_flash_delay(40);
    ret = _norflash_reset();
    if (ret) {
        return SPI_NORFLASH_DRV_ERROR;
    }
    spi_flash_delay(500);
    spi_norflash_idx = 0;
    return SPI_NORFLASH_DRV_OK;
}

#endif
