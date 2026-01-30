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
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "spi_flash_trans.h"
#include "spi_norflash_drv.h"
#include "spi_nandflash_drv.h"
#include "spi_flash.h"

#ifdef SPI_FLASH_DEBUG
    #define SPI_FLASH_TRACE(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
    #define SPI_FLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#else
    #define SPI_FLASH_TRACE(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SPI_FLASH_ERROR(n, s, ...)    TRACE(n, s, ##__VA_ARGS__)
#endif

static const struct HAL_SPI_CFG_T _spi_cfg_default = {
    .rate = 24 * 1000000,
    .clk_delay_half = true,
    .clk_polarity = true,
    .slave = false,
    .dma_rx = true,
    .dma_tx = true,
    .rx_sep_line = true,
    .cs = 0,
    .tx_bits = 32,
    .rx_bits = 32,
    .rx_frame_bits = 0,
    .samp_delay = 0,
};

struct SPI_FLASH_CONTEXT {
    bool opened;
    uint8_t device_id[SPI_FLASH_DEVICE_ID_LEN];
#ifdef FLASH_UNIQUE_ID
    uint8_t unique_id[SPI_FLASH_UNIQUE_ID_LEN + SPI_FLASH_CP_ID_LEN];
#endif
    uint32_t total_size;
    uint32_t block_size;
    uint32_t sector_size;
    uint32_t page_size;
    enum SPI_FLASH_RET_T open_state;
};

struct HAL_SPI_NANDFLASH_CTX_T {
    bool opened;
    uint8_t device_id[SPI_FLASH_DEVICE_ID_LEN];
#ifdef FLASH_UNIQUE_ID
    uint8_t unique_id[SPI_FLASH_UNIQUE_ID_LEN + SPI_FLASH_CP_ID_LEN];
#endif
    uint32_t total_size;
    uint32_t block_size;
    uint32_t page_size;
    uint32_t page_oob_size;
    uint32_t blocks_per_die;
    uint32_t cur_block;
    uint32_t cur_die;
    enum SPI_FLASH_RET_T open_state;
};

enum SPI_FLASH_TYPE_T spi_flash_type;
static struct SPI_FLASH_CONTEXT spi_flash_ctx[SPI_FLASH_ID_NUM];
static struct HAL_SPI_NANDFLASH_CTX_T spi_nandflash_ctx[SPI_FLASH_ID_NUM];
static const struct SPI_FLASH_CONFIG_T spi_flash_cfg = {
    .mode = SPI_FLASH_OP_MODE_STAND_SPI
            | SPI_FLASH_OP_MODE_FAST_SPI
            | HAL_NANDFLASH_OP_MODE_INTERNAL_ECC,
};

static enum SPI_FLASH_RET_T _spi_init(uint32_t spi_rate, uint32_t cs_pin)
{
    int ret;
    struct HAL_SPI_CFG_T spi_cfg;

    spi_cfg = _spi_cfg_default;
    spi_cfg.rate = spi_rate;
    // spi_cfg.samp_delay = 1;

    ret = spi_flash_spi_init(&spi_cfg, cs_pin);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_flash_spi_init failed! result = %d", __func__, ret);
        return SPI_FLASH_SPI_INIT_ERROR;
    }
    return SPI_FLASH_OK;
}

static enum SPI_FLASH_RET_T _spi_norflash_open(enum SPI_FLASH_ID_T id, const struct SPI_FLASH_CONFIG_T *cfg)
{
    enum SPI_FLASH_RET_T ret = SPI_FLASH_OK;
    int result;
    bool matched;
    uint8_t flash_id[SPI_FLASH_DEVICE_ID_LEN];
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(!spi_flash_ctx[id].opened, "%s: spi flash is opened!", __func__);

    spi_norflash_drv_reset();
    spi_norflash_drv_set_mode(cfg->mode);

    result = spi_norflash_drv_get_id(flash_id, sizeof(flash_id));
    if (result) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_get_id failed! ret = %d", __func__, result);
        return SPI_FLASH_GET_ID_FAIL;
    }
    SPI_FLASH_TRACE(1, "%s norflash id: %02x-%02x-%02x", __func__, flash_id[0], flash_id[1], flash_id[2]);

    matched = spi_norflash_drv_match_chip(flash_id, SPI_FLASH_DEVICE_ID_LEN);
    if (matched) {
        spi_norflash_drv_get_size(&total_size, &block_size, &sector_size, &page_size);
        memcpy(spi_flash_ctx[id].device_id, flash_id, SPI_FLASH_DEVICE_ID_LEN);
        spi_flash_ctx[id].total_size = total_size;
        spi_flash_ctx[id].block_size = block_size;
        spi_flash_ctx[id].sector_size = sector_size;
        spi_flash_ctx[id].page_size = page_size;
        spi_flash_ctx[id].opened = true;
        spi_flash_ctx[id].open_state = 0;
    } else {
        SPI_FLASH_ERROR(1, "%s: spi_flash_match_chip false! ", __func__);
        ret = SPI_FLASH_BAD_ID;
    }
#ifdef FLASH_UNIQUE_ID
    spi_norflash_drv_get_unique_id(spi_flash_ctx[id].unique_id, sizeof(spi_flash_ctx[id].unique_id));
#endif
    spi_flash_ctx[id].open_state = ret;
    return ret;
}

static enum SPI_FLASH_RET_T _spi_nandflash_open(enum SPI_FLASH_ID_T id, const struct SPI_FLASH_CONFIG_T *cfg)
{
    enum SPI_FLASH_RET_T ret = SPI_FLASH_OK;
    int32_t result;
    bool matched;
    uint8_t flash_id[SPI_FLASH_DEVICE_ID_LEN];
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;
    uint32_t page_oob_size = 0;
    uint32_t die_num = 0;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(!spi_nandflash_ctx[id].opened, "%s: spi flash is opened!", __func__);

    spi_nandflash_drv_reset();
    spi_nandflash_drv_set_mode(cfg->mode);

    result = spi_nandflash_drv_get_id(flash_id, sizeof(flash_id));
    if (result) {
        SPI_FLASH_ERROR(1, "%s: spi_nandflash_drv_get_id failed! ret = %d", __func__, result);
        return SPI_FLASH_GET_ID_FAIL;
    }

    SPI_FLASH_TRACE(1, "%s nandflash id: %02x-%02x-%02x", __func__, flash_id[0], flash_id[1], flash_id[2]);

    matched = spi_nandflash_drv_match_chip(flash_id, SPI_FLASH_DEVICE_ID_LEN);
    if (matched) {
        spi_nandflash_drv_get_size(&total_size, &block_size, &page_size, &page_oob_size, &die_num);
        memcpy(spi_nandflash_ctx[id].device_id, flash_id, SPI_FLASH_DEVICE_ID_LEN);
        spi_nandflash_ctx[id].total_size = total_size;
        spi_nandflash_ctx[id].block_size = block_size;
        spi_nandflash_ctx[id].page_size = page_size;
        spi_nandflash_ctx[id].page_oob_size = page_oob_size;
        if (die_num > 0) {
            spi_nandflash_ctx[id].blocks_per_die = (total_size / die_num) / block_size;
        } else {
            spi_nandflash_ctx[id].blocks_per_die = total_size / block_size;
        }
        spi_nandflash_ctx[id].opened = true;
        spi_nandflash_ctx[id].cur_block = 0;
        spi_nandflash_ctx[id].cur_die = 0;
        // SPI_FLASH_TRACE(1, "%s: spi_flash_match_chip ok ", __func__);

    } else {
        SPI_FLASH_ERROR(1, "%s: spi_flash_match_chip false! ", __func__);
        ret = SPI_FLASH_BAD_ID;
    }

#ifdef FLASH_UNIQUE_ID
    spi_nandflash_drv_get_unique_id(spi_nandflash_ctx[id].unique_id, SPI_FLASH_UNIQUE_ID_LEN, false);
#endif

    spi_nandflash_drv_protection_disable();
    spi_nandflash_ctx[id].open_state = ret;

    return ret;
}

//---------------------------------------------------------------------
// spi flash apis
//---------------------------------------------------------------------
enum SPI_FLASH_RET_T spi_flash_open(enum SPI_FLASH_ID_T id, uint32_t spi_rate, uint32_t cs_pin)
{
    int ret;
    uint8_t flash_id[SPI_FLASH_DEVICE_ID_LEN];
    const struct SPI_FLASH_CONFIG_T *cfg = &spi_flash_cfg;

    ret = _spi_init(spi_rate, cs_pin);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: _spi_init failed! ret = %d", __func__, ret);
        return SPI_FLASH_SPI_INIT_ERROR;
    }

    ret = spi_norflash_drv_get_id(flash_id, sizeof(flash_id));
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_get_id failed! ret = %d", __func__, ret);
        return SPI_FLASH_GET_ID_FAIL;
    }
    // SPI_FLASH_TRACE(1, "1st reade flash id: %02x-%02x-%02x", flash_id[0], flash_id[1], flash_id[2]);

    if (flash_id[0] == 0 && flash_id[1] != 0 && flash_id[2] != 0) {
        ret = spi_nandflash_drv_get_id(flash_id, sizeof(flash_id));
        if (ret) {
            SPI_FLASH_ERROR(1, "%s: spi_nandflash_drv_get_id failed! ret = %d", __func__, ret);
            return SPI_FLASH_GET_ID_FAIL;
        }
        // SPI_FLASH_TRACE(1, "2nd read flash id: %02x-%02x-%02x", flash_id[0], flash_id[1], flash_id[2]);
    }

    if ((flash_id[0] == 0 && flash_id[1] == 0 && flash_id[1] == 0) ||
        (flash_id[0] == 0xFF && flash_id[1] == 0xFF && flash_id[1] == 0xFF)) {
        SPI_FLASH_ERROR(1, "%s: spi_flash_get_id failed! ret = %d", __func__, ret);
        return SPI_FLASH_GET_ID_FAIL;
    }

    if (flash_id[0] == flash_id[2]) {
        spi_flash_type = SPI_FLASH_TYPE_NANDFLASH;
        SPI_FLASH_TRACE(1, "%s  flash type: nandflash", __func__);
    } else {
        spi_flash_type = SPI_FLASH_TYPE_NORFLASH;
        SPI_FLASH_TRACE(1, "%s  flash type: norflash", __func__);
    }

    if (spi_flash_type == SPI_FLASH_TYPE_NORFLASH) {
        ret = _spi_norflash_open(id, cfg);
        if (ret) {
            SPI_FLASH_ERROR(1, "%s: _spi_norflash_open failed! ret = %d", __func__, ret);
            return SPI_FLASH_GET_ID_FAIL;
        }
    } else {
        ret = _spi_nandflash_open(id, cfg);
        if (ret) {
            SPI_FLASH_ERROR(1, "%s: _spi_nandflash_open failed! ret = %d", __func__, ret);
            return SPI_FLASH_GET_ID_FAIL;
        }
    }

    return SPI_FLASH_OK;
}

bool spi_flash_opened(enum SPI_FLASH_ID_T id)
{
    if (spi_flash_ctx[id].opened || spi_nandflash_ctx[id].opened) {
        return true;
    }

    return false;
}

enum SPI_FLASH_RET_T spi_flash_close(enum SPI_FLASH_ID_T id)
{
    int ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    if (!spi_flash_ctx[id].opened && !spi_nandflash_ctx[id].opened) {
        return SPI_FLASH_NOT_OPENED;
    }

    if (spi_flash_type == SPI_FLASH_TYPE_NORFLASH) {
        memset(&spi_flash_ctx[id], 0, sizeof(spi_flash_ctx[id]));
    } else {
        memset(&spi_flash_ctx[id], 0, sizeof(spi_flash_ctx[id]));
    }
    ret = spi_flash_spi_deinit();
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_flash_spi_deinit failed! ret = %d.", __func__, ret);
        return SPI_FLASH_ERR;
    }

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_flash_get_type(enum SPI_FLASH_ID_T id, enum SPI_FLASH_TYPE_T *type)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);

    if (!spi_flash_ctx[id].opened && !spi_nandflash_ctx[id].opened) {
        return SPI_FLASH_NOT_OPENED;
    }

    *type = spi_flash_type;

    return SPI_FLASH_OK;
}

//---------------------------------------------------------------------
// spi norflash apis
//---------------------------------------------------------------------
enum SPI_FLASH_RET_T spi_norflash_get_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    int32_t ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened!", __func__);

    ret = spi_norflash_drv_get_id(value, len);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_get_id failed! ret = %d.", __func__, ret);
        return SPI_FLASH_ERR;
    }

    return SPI_FLASH_OK;
}

#ifdef FLASH_UNIQUE_ID
enum SPI_FLASH_RET_T spi_norflash_get_unique_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    len = MIN(len, sizeof(spi_flash_ctx[id].unique_id));

    memcpy(value, spi_flash_ctx[id].unique_id, len);

    return SPI_FLASH_OK;
}
#endif

enum SPI_FLASH_RET_T spi_norflash_get_size(enum SPI_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size, uint32_t *sector_size, uint32_t *page_size)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    *total_size = spi_flash_ctx[id].total_size;
    *block_size = spi_flash_ctx[id].block_size;
    *sector_size = spi_flash_ctx[id].sector_size;
    *page_size = spi_flash_ctx[id].page_size;

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_norflash_read(enum SPI_FLASH_ID_T id, uint32_t addr, uint8_t *buffer, uint32_t len)
{
    int32_t ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    if (addr + len > spi_flash_ctx[id].total_size) {
        SPI_FLASH_ERROR(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return SPI_FLASH_BAD_ADDR;
    }

    if (buffer == NULL) {
        SPI_FLASH_ERROR(1, "%s: buffer is null!", __func__);
        return SPI_FLASH_BUFF_IS_NULL;
    }

    if (len == 0) {
        SPI_FLASH_ERROR(1, "%s: len = 0!", __func__);
        return SPI_FLASH_BAD_LEN;
    }

    ret = spi_norflash_drv_read(addr, buffer, len);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_read failed! ret = %d", __func__, ret);
        return SPI_FLASH_READ_ERROR;
    }

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_norflash_write(enum SPI_FLASH_ID_T id, uint32_t addr, uint8_t *buffer, uint32_t len)
{
    int32_t ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    if (addr + len > spi_flash_ctx[id].total_size) {
        SPI_FLASH_ERROR(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return SPI_FLASH_BAD_ADDR;
    }

    if (buffer == NULL) {
        SPI_FLASH_ERROR(1, "%s: buffer is null!", __func__);
        return SPI_FLASH_BUFF_IS_NULL;
    }

    if (len == 0) {
        SPI_FLASH_ERROR(1, "%s: len = 0!", __func__);
        return SPI_FLASH_BAD_LEN;
    }

    ret = spi_norflash_drv_write(addr, buffer, len);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_write failed! ret = %d", __func__, ret);
        return SPI_FLASH_WRITE_ERROR;
    }

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_norflash_erase(enum SPI_FLASH_ID_T id, uint32_t addr, uint32_t len)
{
    int32_t ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    if (addr + len > spi_flash_ctx[id].total_size) {
        SPI_FLASH_ERROR(1, "%s: addr error! add = 0x%x,len = 0x%x.", __func__, addr, len);
        return SPI_FLASH_BAD_ADDR;
    }

    if (len == 0) {
        SPI_FLASH_ERROR(1, "%s: len = 0!", __func__);
        return SPI_FLASH_BAD_LEN;
    }
#ifdef FLASH_ERASE_PAGE_ENABLE
    if (addr & (spi_flash_ctx[id].page_size - 1)) {
        SPI_FLASH_ERROR(1, "%s: addr no alignment.addr = 0x%x!", __func__, addr);
        return SPI_FLASH_BAD_ADDR;
    }

    if (len & (spi_flash_ctx[id].page_size - 1)) {
        SPI_FLASH_ERROR(1, "%s: len no alignment.addr = 0x%x!", __func__, len);
        return SPI_FLASH_BAD_ADDR;
    }
#else
    if (addr & (spi_flash_ctx[id].sector_size - 1)) {
        SPI_FLASH_ERROR(1, "%s: addr no alignment.addr = 0x%x!", __func__, addr);
        return SPI_FLASH_BAD_ADDR;
    }

    if (len & (spi_flash_ctx[id].sector_size - 1)) {
        SPI_FLASH_ERROR(1, "%s: len no alignment.addr = 0x%x!", __func__, len);
        return SPI_FLASH_BAD_ADDR;
    }
#endif

    ret = spi_norflash_drv_erase(addr, len);
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_erase failed! ret = %d", __func__, ret);
        return SPI_FLASH_ERASE_ERROR;
    }

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_norflash_reset(enum SPI_FLASH_ID_T id)
{
    int ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_flash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = spi_norflash_drv_reset();
    if (ret) {
        SPI_FLASH_ERROR(1, "%s: spi_norflash_drv_reset failed! ret = %d", __func__, ret);
        return SPI_FLASH_RESET_ERROR;
    }

    return SPI_FLASH_OK;
}

//---------------------------------------------------------------------
// nandflash
//---------------------------------------------------------------------
static enum SPI_FLASH_RET_T _check_block(enum SPI_FLASH_ID_T id, uint32_t block)
{
    if ((block * spi_nandflash_ctx[id].block_size) >= spi_nandflash_ctx[id].total_size) {
        SPI_FLASH_ERROR(1, "%s block %d error", __func__, block);
        return SPI_FLASH_BAD_BLOCK;
    }

    return SPI_FLASH_OK;
}

static enum SPI_FLASH_RET_T _check_page(enum SPI_FLASH_ID_T id, uint32_t page)
{
    if ((page * spi_nandflash_ctx[id].page_size) >= spi_nandflash_ctx[id].block_size) {
        SPI_FLASH_ERROR(1, "%s page %d error", __func__, page);
        return SPI_FLASH_BAD_PAGE;
    }

    return SPI_FLASH_OK;
}

static enum SPI_FLASH_RET_T _check_page_offset_len(enum SPI_FLASH_ID_T id, uint32_t page_offset, uint32_t len)
{
    uint32_t page_oob_size = spi_nandflash_drv_get_page_oob_size();

    if (page_offset >= spi_nandflash_ctx[id].page_size + page_oob_size) {
        SPI_FLASH_ERROR(1, "%s offset %d error", __func__, page_offset);
        return SPI_FLASH_BAD_OFFSET;
    }
    if (len > spi_nandflash_ctx[id].page_size + page_oob_size) {
        SPI_FLASH_ERROR(1, "%s len %d error", __func__, len);
        return SPI_FLASH_BAD_LEN;
    }
    if (page_offset + len > spi_nandflash_ctx[id].page_size + page_oob_size) {
        SPI_FLASH_ERROR(1, "%s len %d error", __func__, len);
        return SPI_FLASH_BAD_LEN;
    }

    return SPI_FLASH_OK;
}

static enum SPI_FLASH_RET_T _check_oob_offset_len(uint32_t oob_offset, uint32_t len)
{
    uint32_t page_oob_size = spi_nandflash_drv_get_page_oob_size();

    if (oob_offset >= page_oob_size) {
        SPI_FLASH_ERROR(1, "%s oob_offset %d error", __func__, oob_offset);
        return SPI_FLASH_BAD_OFFSET;
    }
    if (len > page_oob_size) {
        SPI_FLASH_ERROR(1, "%s len %d error", __func__, len);
        return SPI_FLASH_BAD_LEN;
    }
    if (oob_offset + len > page_oob_size) {
        SPI_FLASH_ERROR(1, "%s len %d error", __func__, len);
        return SPI_FLASH_BAD_LEN;
    }

    return SPI_FLASH_OK;
}

static void _select_die(enum SPI_FLASH_ID_T id, uint32_t block)
{
    uint32_t die;

    if (spi_nandflash_ctx[id].blocks_per_die == 0) {
        return;
    }
    if (spi_nandflash_ctx[id].cur_block == block) {
        return;
    }
    spi_nandflash_ctx[id].cur_block = block;
    die = block / spi_nandflash_ctx[id].blocks_per_die;
    if (spi_nandflash_ctx[id].cur_die == die) {
        return;
    }
    spi_nandflash_ctx[id].cur_die = die;

    spi_nandflash_die_select(die);
}

//---------------------------------------------------------------------
// spi nandFlash apis
//---------------------------------------------------------------------
enum SPI_FLASH_RET_T spi_nandflash_get_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    len = MIN(len, sizeof(spi_nandflash_ctx[id].device_id));
    memcpy(value, spi_nandflash_ctx[id].device_id, len);

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_get_size(enum SPI_FLASH_ID_T id, uint32_t *total_size, uint32_t *block_size,
                                        uint32_t *page_size, uint32_t *page_oob_size, uint32_t *die_num)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    if (!spi_nandflash_ctx[id].opened) {
        return SPI_FLASH_NOT_OPENED;
    }

    if (total_size) {
        *total_size  = spi_nandflash_ctx[id].total_size;
    }
    if (block_size) {
        *block_size  = spi_nandflash_ctx[id].block_size;
    }
    if (page_size) {
        *page_size   = spi_nandflash_ctx[id].page_size;
    }
    if (page_oob_size) {
        *page_oob_size = spi_nandflash_drv_get_page_oob_size();
    }
    if (die_num) {
        *die_num = (spi_nandflash_ctx[id].total_size / spi_nandflash_ctx[id].block_size) / spi_nandflash_ctx[id].blocks_per_die;
    }

    return SPI_FLASH_OK;
}

#ifdef FLASH_UNIQUE_ID
enum SPI_FLASH_RET_T spi_nandflash_get_unique_id(enum SPI_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    len = MIN(len, sizeof(spi_nandflash_ctx[id].unique_id));
    memcpy(value, spi_nandflash_ctx[id].unique_id, len);

    return SPI_FLASH_OK;
}
#endif

bool spi_nandflash_block_is_bad(enum SPI_FLASH_ID_T id, uint32_t block)
{
    enum SPI_FLASH_RET_T ret;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    ASSERT(ret == 0, "%s block %d error", __func__, block);

    _select_die(id, block);

    return spi_nandflash_drv_raw_block_is_bad(block);
}

enum SPI_FLASH_RET_T spi_nandflash_read_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page_offset_len(id, offset, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }

    if (len == 0) {
        return 0;
    }

    _select_die(id, block);

    result = spi_nandflash_drv_read_raw_page(block, page, offset, buffer, len);

    return result ? SPI_FLASH_READ_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_read_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_oob_offset_len(offset, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }

    if (len == 0) {
        return SPI_FLASH_OK;
    }

    _select_die(id, block);

    offset += spi_nandflash_ctx[id].page_size;
    result = spi_nandflash_drv_read_raw_page(block, page, offset, buffer, len);

    return result ? SPI_FLASH_READ_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_mark_bad_block(enum SPI_FLASH_ID_T id, uint32_t block)
{
    enum SPI_FLASH_RET_T ret;
    uint32_t offset;
    uint8_t mark;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }

    _select_die(id, block);

    offset = spi_nandflash_ctx[id].page_size;
    mark = 0;
    /*
    spi_nandflash_drv_erase_raw_block(block);
    */
    spi_nandflash_drv_update_page_offset(block, 0, offset, &mark, 1);
    spi_nandflash_drv_update_page_offset(block, 1, offset, &mark, 1);

    return SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_erase_block(enum SPI_FLASH_ID_T id, uint32_t block)
{
    enum SPI_FLASH_RET_T ret;
    int result;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }

    _select_die(id, block);

    result = spi_nandflash_drv_erase_raw_block(block);

    return result ? SPI_FLASH_ERASE_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_write_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, const uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;
    uint32_t offset;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page_offset_len(id, 0, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }

    _select_die(id, block);

    offset = 0;
    result = spi_nandflash_drv_write_page_offset(block, page, offset, buffer, len);

    return result ? SPI_FLASH_WRITE_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_write_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, const uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;
    uint32_t offset;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret) {
        return ret;
    }
    ret = _check_oob_offset_len(0, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    if (len == 0) {
        return SPI_FLASH_OK;
    }

    _select_die(id, block);

    offset = spi_nandflash_ctx[id].page_size;
    result = spi_nandflash_drv_write_page_offset(block, page, offset, buffer, len);

    return result ? SPI_FLASH_WRITE_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_update_page(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page_offset_len(id, offset, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    if (len == 0) {
        return SPI_FLASH_OK;
    }

    _select_die(id, block);

    result = spi_nandflash_drv_update_page_offset(block, page, offset, buffer, len);

    return result ? SPI_FLASH_WRITE_ERROR : SPI_FLASH_OK;
}

enum SPI_FLASH_RET_T spi_nandflash_update_oob(enum SPI_FLASH_ID_T id, uint32_t block, uint32_t page, uint32_t offset, const uint8_t *buffer, uint32_t len)
{
    enum SPI_FLASH_RET_T ret;
    int result;

    ASSERT(id < SPI_FLASH_ID_NUM, "%s: id error! id = %d", __func__, id);
    ASSERT(spi_nandflash_ctx[id].opened, "%s: spi flash not opened", __func__);

    ret = _check_block(id, block);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_page(id, page);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    ret = _check_oob_offset_len(offset, len);
    if (ret != SPI_FLASH_OK) {
        return ret;
    }
    if (len == 0) {
        return SPI_FLASH_OK;
    }

    _select_die(id, block);

    offset += spi_nandflash_ctx[id].page_size;
    result = spi_nandflash_drv_update_page_offset(block, page, offset, buffer, len);

    return result ? SPI_FLASH_WRITE_ERROR : SPI_FLASH_OK;
}

#endif
