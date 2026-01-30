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
#include "string.h"
#include "hal_spi.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_norflaship.h"
#include "norflash_gd25q32c.h"
#include "nandflash_w25m02gv.h"
#include "nandflash_gd5f4gq4.h"
#include "norflash_cfg.h"
#include "norflash_drv.h"
#include "nandflash_drv.h"
#include "spi_flash_trans.h"
#include "spi_nandflash_drv.h"
#include "spi_flash.h"

#ifdef SPI_FLASH_DEBUG
    #define SPI_NANDFLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
    #define SPI_NANDFLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define SPI_NANDFLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SPI_NANDFLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#endif

#ifdef FLASH_ERASE_PAGE_ENABLE
    #define PUYA_FLASH_CMD_PAGE_ERASE   0x81
#endif


static uint8_t spi_nandflash_idx = INVALID_SPIFLASH_IDX;
static const struct NANDFLASH_CFG_T  **spi_nandflash_list = NULL;
static uint32_t spi_list_len = 0;
static uint32_t spi_nandflash_op_mode = 0;

void _nanflash_delay_ns(uint32_t ns)
{
    hal_sys_timer_delay_ns(ns);
}

static inline uint32_t _block_to_row_addr(uint32_t block)
{
    return (block << 6);
}

static uint32_t _block_page_offset_to_col_addr(uint32_t block, uint32_t offset)
{
    uint8_t plane_num = (spi_nandflash_list[spi_nandflash_idx]->plane_cfg >> 4) & 0xF;
    uint8_t plane_offset = (spi_nandflash_list[spi_nandflash_idx]->plane_cfg) & 0xF;

    if (plane_num > 1)
        return ((block&(plane_num-1)) << plane_offset) | offset;
    else
        return offset;
}

static inline uint32_t _block_page_to_row_addr(uint32_t block, uint32_t page)
{
    return (block << 6) | (page & 0x3F);
}

#ifdef SPI_FLASH_DEBUG
static int _dump_feature(uint8_t param)
{
    int ret;
    uint8_t cmd[2];
    uint8_t val;

    cmd[0] = GD5F4GQ4_CMD_GET_FEATURE;
    cmd[1] = param;

    ret = spi_flash_recv_byte(cmd, sizeof(cmd), &val, 1);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: spi_flash_recv_byte failed,ret = %d", __func__, ret);
        return 1;
    }
    TRACE_IMM(1, "REG[0x%02x] = 0x%02x", param, val);
    return 0;
}
#endif

void spi_nandflash_drv_set_mode(uint32_t mode)
{
    spi_nandflash_op_mode = mode;
}

static int _read_feature(uint8_t param, uint8_t *val)
{
    int ret;
    uint8_t cmd[2];

    cmd[0] = GD5F4GQ4_CMD_GET_FEATURE;
    cmd[1] = param;

    ret = spi_flash_recv_byte(cmd, sizeof(cmd), val, 1);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: spi_flash_recv_byte failed,ret = %d", __func__, ret);
        return 1;
    }
    return 0;
}

POSSIBLY_UNUSED static int _write_feature(uint8_t param, uint8_t val)
{
    int ret;
    uint8_t cmd[2];

    cmd[0] = GD5F4GQ4_CMD_SET_FEATURE;
    cmd[1] = param;

    ret = spi_flash_send_byte(cmd, sizeof(cmd), &val, 1);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: spi_flash_send_byte failed, ret = %d", __func__, ret);
        return 1;
    }
    return 0;
}

static int _nandflash_busy_wait(void)
{
    int ret;
    uint8_t cmd[2] = {GD5F4GQ4_CMD_GET_FEATURE, GD5F4GQ4_REG_STATUS};
    uint8_t status = 0;

    while (1) {
        ret = spi_flash_recv_byte(cmd, sizeof(cmd), &status, sizeof(status));
        if (ret) {
            SPI_NANDFLASH_ERROR(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
            break;
        }
        if (!(status & GD5F4GQ4_ST_OIP)) {
            return 0;
        }
    }
    return 1;
}

static int _nandflash_write_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD5F4GQ4_CMD_WRITE_ENABLE;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }
    _nandflash_busy_wait();

    return 0;
}

static int _nandflash_reset_enable(void)
{
    int32_t ret;
    uint8_t cmd = GD5F4GQ4_CMD_POWERON_RESET_ENABLE;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }
    _nandflash_busy_wait();

    return 0;
}

static int _nandflash_reset(void)
{
    int32_t ret;
    uint8_t cmd = GD5F4GQ4_CMD_POWERON_RESET;

    ret = spi_flash_send_byte(&cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }
    _nandflash_busy_wait();

    return 0;
}

static int _nandflash_erase_block(uint32_t row_addr)
{
    int32_t ret;
    uint8_t cmd[4];

    cmd[0] = GD5F4GQ4_CMD_BLOCK_ERASE;
    cmd[1] = (row_addr >> 16) & 0xFF;
    cmd[2] = (row_addr >> 8) & 0xFF;
    cmd[3] = (row_addr) & 0xFF;

    ret = spi_flash_send_word(cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }

    _nanflash_delay_ns(25);
    _nandflash_busy_wait();

    return 0;
}

static int _nandflash_read_to_cache(uint32_t row_addr, bool check_ecc)
{
    uint8_t cmd[4];
    enum NANDFLASH_ECC_ST_TYPE_T st_type;
    uint8_t ecc10_reg = 0, ecc32_reg = 0;
    uint8_t status10 = 0, status32 = 0;
    bool no_err;
    int ret;

    cmd[0] = GD5F4GQ4_CMD_PAGE_READ;
    cmd[1] = (row_addr >> 16) & 0xFF;
    cmd[2] = (row_addr >> 8) & 0xFF;
    cmd[3] = (row_addr) & 0xFF;

    ret = spi_flash_send_word(cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }

    _nanflash_delay_ns(25);

    _nandflash_busy_wait();

    if (!check_ecc) {
        return 0;
    }

    st_type = spi_nandflash_list[spi_nandflash_idx]->ecc_cfg.st_type;

    nandflash_ecc_get_eccs(st_type, &ecc10_reg, &ecc32_reg);

    ret = _read_feature(ecc10_reg, &status10);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature failed,ret = %d", __func__, ret);
        return ret;
    }

    if (ecc10_reg != ecc32_reg && ecc32_reg != 0) {
        ret = _read_feature(ecc32_reg, &status32);
        if (ret) {
            SPI_NANDFLASH_TRACE(1, "%s: _read_feature failed,ret = %d", __func__, ret);
            return ret;
        }
    }

    no_err = nandflash_ecc_check(st_type, status10, status32, spi_nandflash_list[spi_nandflash_idx]->ecc_cfg.st_bad);
    if (!no_err) {
        SPI_NANDFLASH_ERROR(1, "%s: nandflash_ecc_check failed,ret = %d", __func__, no_err);
        return 2;
    }

    return 0;

}

static int _nandflash_read_from_cache(uint32_t col_addr, uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[4];
    uint32_t total_page_size;

    _nandflash_busy_wait();

    total_page_size = spi_nandflash_list[spi_nandflash_idx]->page_size +
                spi_nandflash_drv_get_page_oob_size();
    cur_addr = col_addr;
    pbuff = buff;
    remain_len = len;

    if (spi_nandflash_op_mode & HAL_NORFLASH_OP_MODE_FAST_SPI){
        /* fast */
        cmd[0] = GD5F4GQ4_CMD_STANDARD_FAST_READ;
    } else {
        /* normal */
        cmd[0] = GD5F4GQ4_CMD_STANDARD_READ;
    }

    do {
        cmd[1] = ((cur_addr >> 8) & 0xff);
        cmd[2] = (cur_addr & 0xff);
        cmd[3] = 0;

        tmp_len = remain_len <=  total_page_size? \
                  remain_len : total_page_size;

        if ((tmp_len / 4) * 4 == tmp_len) {
            ret = spi_flash_recv_word(cmd, sizeof(cmd), pbuff, tmp_len);
        } else {
            ret = spi_flash_recv_byte(cmd, sizeof(cmd), pbuff, tmp_len);
        }
        if (ret) {
            SPI_NANDFLASH_ERROR(1, "%s: _spi_recv_byte failed!ret = %d", __func__, ret);
            return 1;
        }

        cur_addr += tmp_len;
        remain_len -= tmp_len;
        pbuff += tmp_len;
    } while (remain_len);

    return 0;
}

static int _nandflash_program_load(uint32_t col_addr, const uint8_t *buff, uint32_t len)
{
    int32_t ret;
    uint32_t tmp_len;
    uint32_t cur_addr;
    uint32_t remain_len;
    uint8_t *pbuff;
    uint8_t cmd[3];
    uint32_t total_page_size;

    total_page_size = spi_nandflash_list[spi_nandflash_idx]->page_size +
                spi_nandflash_drv_get_page_oob_size();
    cur_addr = col_addr;
    pbuff = (uint8_t*)buff;
    remain_len = len;
    do {
        cmd[0] = GD5F4GQ4_CMD_PROGRAM_LOAD;
        cmd[1] = ((cur_addr >> 8) & 0xff);
        cmd[2] = (cur_addr & 0xff);

        tmp_len = remain_len <= total_page_size ? \
                  remain_len : total_page_size;
        ret = spi_flash_send_byte(cmd, sizeof(cmd), pbuff, tmp_len);
        if (ret) {
            SPI_NANDFLASH_ERROR(1, "%s: spi_flash_send_word failed!ret = %d", __func__, ret);
            return 1;
        }

        cur_addr += tmp_len;
        remain_len -= tmp_len;
        pbuff += tmp_len;
    } while (remain_len);

     _nandflash_busy_wait();

    return 0;
}

static int _nandflash_program_execute(uint32_t row_addr)
{
    uint8_t cmd[4];
    uint8_t status;
    int ret;

    cmd[0] = GD5F4GQ4_CMD_PROGRAM_EXECUTRE;
    cmd[1] = (row_addr >> 16) & 0xFF;
    cmd[2] = (row_addr >> 8) & 0xFF;
    cmd[3] = (row_addr) & 0xFF;

    ret = spi_flash_send_word(cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return 1;
    }

    _nanflash_delay_ns(25);
    _nandflash_busy_wait();

    ret = _read_feature(GD5F4GQ4_REG_STATUS, &status);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature(0xC0) failed,ret = %d", __func__, ret);
        return ret;
    }

    if (status & GD5F4GQ4_ST_P_FAIL) {
        return 3;
    }

    return 0;
}

//------------------------------------------------------------------
// spi_nandflash_drv functions
//------------------------------------------------------------------
bool spi_nandflash_drv_match_chip(const uint8_t *flash_id, uint32_t len)
{
    const uint8_t *cmp_id;
    const struct NANDFLASH_CFG_T  *tmp_list;
    uint8_t idx;

    nandflash_get_flash_list(&tmp_list, &spi_list_len);
    spi_nandflash_list = (const struct NANDFLASH_CFG_T **)tmp_list;

    if (len == NORFLASH_ID_LEN) {
        for (idx = 0; idx < spi_list_len; idx++) {
            cmp_id = spi_nandflash_list[idx]->id;
            if (flash_id[0] == cmp_id[0] && flash_id[1] == cmp_id[1] && flash_id[2] == cmp_id[2]) {
                spi_nandflash_idx = idx;
                return true;
            }
        }
    }

    return false;
}

int spi_nandflash_drv_get_size(uint32_t *total_size, uint32_t *block_size, uint32_t *page_size, uint32_t *page_oob_size, uint32_t *die_num)
{
    if (total_size) {
        *total_size  = spi_nandflash_list[spi_nandflash_idx]->total_size;
    }
    if (block_size) {
        *block_size  = spi_nandflash_list[spi_nandflash_idx]->block_size;
    }
    if (page_size) {
        *page_size   = spi_nandflash_list[spi_nandflash_idx]->page_size;
    }
    if (page_oob_size) {
        *page_oob_size = spi_nandflash_drv_get_page_oob_size();
    }
    if (die_num) {
        *die_num = spi_nandflash_list[spi_nandflash_idx]->die_num;
    }

    return SPI_NANDFLASH_DRV_OK;
}

uint32_t spi_nandflash_drv_get_page_oob_size(void)
{
    if (spi_nandflash_op_mode & HAL_NANDFLASH_OP_MODE_INTERNAL_ECC) {
        return spi_nandflash_list[spi_nandflash_idx]->page_spare_size / 2;
    } else {
        return spi_nandflash_list[spi_nandflash_idx]->page_spare_size;
    }
}

int spi_nandflash_die_select(uint8_t die_id)
{
    int32_t ret;
    uint8_t cmd[2];

    cmd[0] = W25M02GV_CMD_DIE_SEL;
    cmd[1] = die_id;

    ret = spi_flash_send_byte(cmd, sizeof(cmd), NULL, 0);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_send_byte failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }
    _nandflash_busy_wait();
    return SPI_NANDFLASH_DRV_OK;
}

int  spi_nandflash_drv_reset(void)
{
    int32_t ret;

    _nandflash_busy_wait();

    ret = _nandflash_reset_enable();
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_reset_enable failed!.ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    _nanflash_delay_ns(40*1000);
    ret = _nandflash_reset();
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_reset failed!.ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    _nanflash_delay_ns(500*1000);
    spi_nandflash_idx = 0;

    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_protection_enable(void)
{
    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_protection_disable(void)
{
    int ret;
    uint8_t val = 0;

    ret = _read_feature(GD5F4GQ4_REG_PROTECTION, &val);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature failed!.ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }
    val &= ~(GD5F4GQ4_PRT_CMP | GD5F4GQ4_PRT_INV | GD5F4GQ4_PRT_BP0_2_MASK);
    ret = _write_feature(GD5F4GQ4_REG_PROTECTION, val);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _write_feature failed!.ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_get_id(uint8_t *flash_id, uint32_t len)
{
    int32_t ret;
    uint8_t cmd[2] = {GD5F4GQ4_CMD_ID, 0};

    _nandflash_busy_wait();
    ret = spi_flash_recv_byte(cmd, sizeof(cmd), flash_id, len);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _spi_recv_byte failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    return SPI_NANDFLASH_DRV_OK;
}

#ifdef FLASH_UNIQUE_ID
static bool uinque_id_verify(int8_t *original, int8_t *complement, uint32_t len)
{
    uint32_t i;
    int8_t comp;

    for (i = 0; i < len; i++) {
        comp = original[i] >= 0 ? original[i] : (~(-original[i]) + 1);
        if (comp != complement[i]) {
            return false;
        }
    }

    return true;
}

int spi_nandflash_drv_get_unique_id(uint8_t *unique_id, uint32_t len, bool verify)
{
    int ret = 0;
    int result;
    uint8_t val = 0;
    uint32_t row_addr = 0;
    int8_t buff[32];
    bool verify_result;

    // 1. Use Set Feature command to set OTP_EN.
    result = _read_feature(GD5F4GQ4_REG_CONFIGURATION, &val);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature failed!ret = %d", __func__, result);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    val |= GD5F4GQ4_CFG_OTP_EN;
    result = _write_feature(GD5F4GQ4_REG_CONFIGURATION, val);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _write_feature failed!ret = %d", __func__, result);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    // 2. Use Get Feature command to get data from B0 register and check if the OTP_EN is enable.
    result = _read_feature(GD5F4GQ4_REG_CONFIGURATION, &val);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature failed!ret = %d", __func__, result);
        ret = SPI_NANDFLASH_DRV_ERROR;
        goto _func_end;
    }

    if (!(val & GD5F4GQ4_CFG_OTP_EN)) {
        SPI_NANDFLASH_ERROR(1, "%s: bit OTP_EN = 0, val = 0x%02x", __func__, val);
        ret = SPI_NANDFLASH_DRV_ERROR;
        goto _func_end;
    }

    // 3. Use page read to cache (13h) command with address 24’h000000h, read data from array to cache.
    result = _nandflash_read_to_cache(row_addr, false);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_read_to_cache failed!ret = %d", __func__, result);
        ret = SPI_NANDFLASH_DRV_ERROR;
        goto _func_end;
    }

    // 5. User can use Read from cache command (03H/0BH), read 16 bytes UID from cache.
    result = _nandflash_read_from_cache(0, (uint8_t*)buff, 32);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_read_from_cache failed!ret = %d", __func__, result);
        ret = SPI_NANDFLASH_DRV_ERROR;
        goto _func_end;
    }

    if (verify) {
        verify_result = uinque_id_verify((int8_t*)buff, (int8_t*)buff + 16, 16);
        if (!verify_result) {
            SPI_NANDFLASH_ERROR(1, "%s: uid verify fail", __func__);
            DUMP8("0x%02x", (uint8_t*)buff, 16);
            DUMP8("0x%02x", (uint8_t*)buff + 16, 16);
            ret = SPI_NANDFLASH_DRV_ERROR;
            goto _func_end;
        }
    }

    memcpy(unique_id, buff, len);

_func_end:
    // 6. Use Set Feature command to clr OTP_EN.
    result = _read_feature(GD5F4GQ4_REG_CONFIGURATION, &val);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature failed!ret = %d", __func__, result);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    val &= ~GD5F4GQ4_CFG_OTP_EN;
    result = _write_feature(GD5F4GQ4_REG_CONFIGURATION, val);
    if (result) {
        SPI_NANDFLASH_ERROR(1, "%s: _write_feature failed!ret = %d", __func__, result);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    return ret;
}
#endif

bool spi_nandflash_drv_raw_block_is_bad(uint32_t block)
{
    int ret;
    uint32_t page;
    uint8_t mark[2];

    for (page = 0; page < 2; page++) {
        ret = spi_nandflash_drv_read_raw_page(block, page, spi_nandflash_list[spi_nandflash_idx]->page_size, mark+page, 1);
        if (ret) {
            SPI_NANDFLASH_ERROR(1, "%s spi_nandflash_read_raw_page faile, ret = %d", __func__, ret);
            return true;
        }
    }

    if (mark[0] != 0xFF || mark[1] != 0xFF) {
        SPI_NANDFLASH_ERROR(1, "block[%d] is bad mark[0] = 0x%x, mark[1] = 0x%x", block, mark[0], mark[1]);
        return true;
    }

    return false;
}

int spi_nandflash_drv_read_raw_page(uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    int ret;
    uint32_t row_addr, col_addr;

    row_addr = _block_page_to_row_addr(block, page);
    ret = _nandflash_read_to_cache(row_addr, true);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s _nandflash_read_to_cache faile, ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    if (len) {
        col_addr = _block_page_offset_to_col_addr(block, offset);
        ret = _nandflash_read_from_cache(col_addr, buffer, len);
        if (ret) {
            SPI_NANDFLASH_ERROR(1, "%s _nandflash_read_from_cache faile, ret = %d", __func__, ret);
            return SPI_NANDFLASH_DRV_ERROR;
        }
    }

    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_erase_raw_block(uint32_t block)
{
    uint32_t row_addr;
    uint8_t status;
    int ret;

    row_addr = _block_to_row_addr(block);

    _nandflash_write_enable();

    _nandflash_erase_block(row_addr);

    ret = _read_feature(GD5F4GQ4_REG_STATUS, &status);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _read_feature(0xC0) failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    if (status & GD5F4GQ4_ST_E_FAIL) {
        SPI_NANDFLASH_ERROR(1, "%s: ST_E_FAIL ERROR, st: 0x%02x", __func__, status);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_write_page_offset(uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    int ret;
    uint32_t col_addr, row_addr;

    _nandflash_write_enable();

    col_addr = _block_page_offset_to_col_addr(block, offset);
    ret = _nandflash_program_load(col_addr, buffer, len);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s _nandflash_program_load faile, ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    row_addr = _block_page_to_row_addr(block, page);
    ret = _nandflash_program_execute(row_addr);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s _nandflash_program_execute faile, ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    //_nandflash_busy_wait();

    return SPI_NANDFLASH_DRV_OK;
}

int spi_nandflash_drv_update_page_offset(uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    int ret;
    uint32_t row_addr, col_addr;

    row_addr = _block_page_to_row_addr(block, page);

    ret = _nandflash_read_to_cache(row_addr, true);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_read_to_cache() failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    row_addr = _block_page_to_row_addr(block, page);

    _nandflash_write_enable();

    col_addr = _block_page_offset_to_col_addr(block, offset);
    ret = _nandflash_program_load(col_addr, buffer, len);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_program_load() failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    ret = _nandflash_program_execute(row_addr);
    if (ret) {
        SPI_NANDFLASH_ERROR(1, "%s: _nandflash_program_execute() failed,ret = %d", __func__, ret);
        return SPI_NANDFLASH_DRV_ERROR;
    }

    return SPI_NANDFLASH_DRV_OK;
}

#ifdef SPI_FLASH_DEBUG
void spi_nandflash_drv_dump_reg(void)
{
    _dump_feature(GD5F4GQ4_REG_PROTECTION);
    _dump_feature(GD5F4GQ4_REG_CONFIGURATION);
    _dump_feature(GD5F4GQ4_REG_STATUS);
    _dump_feature(GD5F4GQ4_REG_DRIVER);
    _dump_feature(GD5F4GQ4_REG_STATUS2);
}
#endif

#endif
