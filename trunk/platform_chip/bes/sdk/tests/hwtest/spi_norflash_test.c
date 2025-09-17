/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_spi_norflash.h"

#if 1
    #define SPI_NORFLASH_TEST_TRACE TRACE
#else
    #define SPI_NORFLASH_TEST_TRACE(str,...)
#endif

#define TEST_ADDR_START             0
#define TEST_ADDR_END               0x100000
#define SPI_NORFLASH_SECTOR_SIZE    0x1000
#define SET_BUFFER_MODE_0           0
#define SET_BUFFER_MODE_1           1

static uint8_t test_buff_r[SPI_NORFLASH_SECTOR_SIZE];
static uint8_t test_buff_w[SPI_NORFLASH_SECTOR_SIZE];

static void set_buffer(uint8_t *buff, uint32_t len, uint8_t mode, uint8_t val)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
        if (mode == SET_BUFFER_MODE_0) {
            buff[i] = val;
        } else {
            buff[i] = val++;
        }
    }
}

static int _spi_norflash_test(void)
{
    enum HAL_SPI_NORFLASH_RET_T ret;
    uint8_t chip_id[HAL_SPI_NORFLASH_DEVICE_ID_LEN];
    uint32_t i;
    uint32_t len;
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t addr;

    ret = hal_spi_norflash_init();
    if (ret) {
        SPI_NORFLASH_TEST_TRACE(1, "norflash init failed, ret = %d", ret);
        return ret;
    }

    ret = hal_spi_norflash_get_id(HAL_SPI_NORFLASH_ID_0, chip_id, sizeof(chip_id));
    if (ret) {
        SPI_NORFLASH_TEST_TRACE(1, "norflash get chip id failed, ret = %d", ret);
        return ret;
    } else {
        SPI_NORFLASH_TEST_TRACE(3, "get chip id ok, id:0x%X-0x%X-0x%X", chip_id[0], chip_id[1], chip_id[2]);
    }

#if defined(FLASH_UNIQUE_ID)
    {
        uint8_t unique_id[HAL_SPI_NORFLASH_UNIQUE_ID_LEN + HAL_SPI_NORFLASH_CP_ID_LEN];

        ret = hal_spi_norflash_get_unique_id(HAL_SPI_NORFLASH_ID_0, unique_id, sizeof(unique_id));
        if (ret) {
            SPI_NORFLASH_TEST_TRACE(1, "norflash get unique id failed, ret = %d", ret);
            return ret;
        } else {
            SPI_NORFLASH_TEST_TRACE(0, "unique id:");
            DUMP8("0x%x ", unique_id, sizeof(unique_id));
        }
    }
#endif

    ret = hal_spi_norflash_get_size(HAL_SPI_NORFLASH_ID_0, &total_size, &block_size, &sector_size, &page_size);
    if (ret) {
        SPI_NORFLASH_TEST_TRACE(1, "norflash get size failed, ret = %d", ret);
        return ret;
    } else {
        SPI_NORFLASH_TEST_TRACE(1, "total size :%d", total_size);
        SPI_NORFLASH_TEST_TRACE(1, "block size :%d", block_size);
        SPI_NORFLASH_TEST_TRACE(1, "sector size:%d", sector_size);
        SPI_NORFLASH_TEST_TRACE(1, "page size  :%d", page_size);
    }

    len = TEST_ADDR_END - TEST_ADDR_START;
    SPI_NORFLASH_TEST_TRACE(1, "spi sector erase/write/read check range, 0x%08X--0x%08X", TEST_ADDR_START, TEST_ADDR_END);
    len = len / SPI_NORFLASH_SECTOR_SIZE;
    for (i = TEST_ADDR_START; i < len; i++) {
        SPI_NORFLASH_TEST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
        SPI_NORFLASH_TEST_TRACE(0, "******cnt:%d/%d", i + 1, len);

        addr = i * SPI_NORFLASH_SECTOR_SIZE;
        ret = hal_spi_norflash_erase(HAL_SPI_NORFLASH_ID_0, addr, SPI_NORFLASH_SECTOR_SIZE);
        ASSERT(ret == HAL_SPI_NORFLASH_OK, "%s:%d,norflash erase failed,addr:0x%X, ret = %d", __func__, __LINE__, addr, ret);
        SPI_NORFLASH_TEST_TRACE(1, "---norflash erase addr:0x%X done", addr);

        ret = hal_spi_norflash_read(HAL_SPI_NORFLASH_ID_0, addr, test_buff_r, sizeof(test_buff_r));
        ASSERT(ret == HAL_SPI_NORFLASH_OK, "%s:%d,norflash read failed, ret = %d", __func__, __LINE__, ret);

        memset(test_buff_w, 0xff, sizeof(test_buff_w));
        if (memcmp(test_buff_r, test_buff_w, sizeof(test_buff_r)) == 0) {
            SPI_NORFLASH_TEST_TRACE(1, "---erase check ok,addr:0x%X", addr);
        } else {
            DUMP8("0x%X ", test_buff_r, 16);
            ASSERT(0, "%s:%d,erase check error", __func__, __LINE__);
        }
        set_buffer(test_buff_w, sizeof(test_buff_w), SET_BUFFER_MODE_1, 0);

        ret = hal_spi_norflash_write(HAL_SPI_NORFLASH_ID_0, addr, test_buff_w, sizeof(test_buff_w));
        ASSERT(ret == HAL_SPI_NORFLASH_OK, "%s:%d,norflash write failed, ret = %d", __func__, __LINE__, ret);
        SPI_NORFLASH_TEST_TRACE(1, "---norflash write addr:0x%X done", addr);

        ret = hal_spi_norflash_read(HAL_SPI_NORFLASH_ID_0, addr, test_buff_r, sizeof(test_buff_r));
        ASSERT(ret == HAL_SPI_NORFLASH_OK, "%s:%d,norflash read failed, ret = %d", __func__, __LINE__, ret);

        if (memcmp(test_buff_r, test_buff_w, sizeof(test_buff_r)) == 0) {
            DUMP8("0x%02X ", test_buff_r, 16);
            DUMP8("0x%02X ", test_buff_r + 16, 16);
            SPI_NORFLASH_TEST_TRACE(1, "---write check ok,addr:0x%X", addr);
        } else {
            uint32_t offs;
            uint32_t n;

            for (offs = 0; offs < sizeof(test_buff_r); offs ++) {
                if (test_buff_w[offs] != test_buff_r[offs]) {
                    break;
                }
            }

            SPI_NORFLASH_TEST_TRACE(1, "offset = 0x%x, write check error", offs);

            for (n = 0; n < sizeof(test_buff_r) / 32; n ++) {
                DUMP8("0x%x,", test_buff_r + n * 32, 32);
                hal_sys_timer_delay(32);
            }
            ASSERT(0, "%s:%d, write check error", __func__, __LINE__);
        }
    }
    hal_spi_norflash_deinit();
    SPI_NORFLASH_TEST_TRACE(1, "spi sector erase/write/read check done, 0x%08X--0x%08X", TEST_ADDR_START, TEST_ADDR_END);

    return ret;
}

void spi_norflash_test(void)
{
    enum HAL_SPI_NORFLASH_RET_T ret;

    SPI_NORFLASH_TEST_TRACE(0, "  ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SPI_NORFLASH_TEST_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SPI_NORFLASH_TEST_TRACE(1, "%s: begin", __func__);
    ret = _spi_norflash_test();
    if (ret) {
        SPI_NORFLASH_TEST_TRACE(1, "%s: failed! ret = %d", __func__, ret);
        return;
    }

    SPI_NORFLASH_TEST_TRACE(1, "%s: done", __func__);
    SPI_NORFLASH_TEST_TRACE(0, "======spi norflash test success,2021-07-20======");
}

