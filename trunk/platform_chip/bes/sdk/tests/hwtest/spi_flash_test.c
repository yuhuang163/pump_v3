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
#ifdef SPI_FLASH_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_iomux.h"
#include "spi_flash.h"

#if 1
    #define SPI_FLASH_TEST_TRACE TRACE
#else
    #define SPI_FLASH_TEST_TRACE(str,...)
#endif

#define SPI_FLASH_BUFFER_SIZE           0x1000
#define SET_BUFFER_MODE_0               0
#define SET_BUFFER_MODE_1               1
#define SPI_RATE                        (24 * 1000000)//

/*--------------------NORFLASH ---------------*/
#define SPI_NORFLASH_SECTOR_SIZE        0x1000
//#define TEST_NORFLASH_ADDR_START        0
//#define TEST_NORFLASH_ADDR_END          0x2000000

/*--------------------NANDFLASH ---------------*/
 #define NANDFLASH_TEST_OOB_PROGRAM
 #define NANDFLASH_TEST_SKIP_BAD_BLOCK_CHECK
 // #define READ_SPECIAL_BLOCK_PAGE
 // #define UNALIGNED_DMA_ACCESS
 // #define TEST_BLOCK_START                 0
 // #define TEST_BLOCK_END                   100

/*--------------Test Case Switch-----------------*/
#define NANDFLASH_TEST_BASE_FUCTION
#define NANDFLASH_TEST_PERFORMANCE
#define NANDFLASH_TEST_LIFE


#define NANDFLASH_TEST_BASE_FUCTION_ON     (0x1 << 0)
#define NANDFLASH_TEST_PERFORMANCE_ON      (0x1 << 4)
#define NANDFLASH_TEST_LIFE_ON             (0x1 << 5)
/*----------------------------------------*/
static uint8_t test_buff_r[SPI_FLASH_BUFFER_SIZE];
static uint8_t test_buff_w[SPI_FLASH_BUFFER_SIZE];
enum SPI_FLASH_ID_T spi_flash_id = SPI_FLASH_ID_0;

static void _set_buffer(uint8_t *buff, uint32_t len, uint8_t mode, uint8_t val)
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

static uint8_t _cmp_buffer(uint8_t * buff1, uint8_t * buff2, uint32_t len, uint32_t *diff_offs)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
        if (buff1[i] != buff2[i]) {
            SPI_FLASH_TEST_TRACE(1, "buff1[%d] = 0x%x, buff2[%d] = 0x%x", i, buff1[i], i, buff2[i]);
            *diff_offs = i;
            return 1;
        }
    }

    return 0;
}

static uint8_t _cmp_pattern(uint8_t * buff, uint8_t pattern, uint32_t len, uint32_t *diff_offs)
{
    uint32_t i;

    if (pattern == 0xFF || pattern == 0x00) {
        for (i = 0; i < len; i++) {
            if (buff[i] != pattern) {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }

    if (pattern == 0x01) {
        for (i = 0; i < len; i++) {
            if (buff[i] != (i & 0xff)) {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }

    if (pattern == 0x10) {
        for (i = len; i > 0; i--) {
            if (buff[i - 1] != ((i - 1) & 0xff)) {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

#ifdef SPI_FLASH_CS_PIN_INDEX
static uint32_t get_spi_cs_pin(void)
{
    int decimal = 0;
    int i = 0;
    int digit;
    int octal = SPI_FLASH_CS_PIN_INDEX;
    int multi = 1;

    while (octal != 0) {
        digit = octal % 10;
        decimal += digit * multi;
        multi *= 8;
        octal /= 10;
        i++;
    }
    return (uint32_t)decimal;
}
#else
static uint32_t get_spi_cs_pin(void)
{
    //err: please select spi_csi_pin.
    uint32_t cs_pin = (uint32_t)HAL_IOMUX_PIN_P2_5;
    return cs_pin;
}

#endif

//-------------------------------------------------------------------
// nandflash
//-------------------------------------------------------------------
static int _spi_norflash_test(void)
{
    enum SPI_FLASH_RET_T ret;
    uint32_t i;
    uint32_t start_addr = 0;
    uint32_t len;
    uint32_t sec_cnt;
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t addr;
    uint32_t diff_offs;
    uint8_t chip_id[SPI_FLASH_DEVICE_ID_LEN];

    SPI_FLASH_TEST_TRACE(1, "%s", __func__);
    ret = spi_norflash_get_id(spi_flash_id, chip_id, sizeof(chip_id));
    if (ret) {
        SPI_FLASH_TEST_TRACE(1, "flash get chip id failed, ret = %d", ret);
        goto _FUNC_FAIL;
    } else {
        SPI_FLASH_TEST_TRACE(1, "get chip id ok, id:0x%X-0x%X-0x%X", chip_id[0], chip_id[1], chip_id[2]);
    }
#if defined(FLASH_UNIQUE_ID)
    {
        uint8_t unique_id[SPI_FLASH_UNIQUE_ID_LEN + SPI_FLASH_CP_ID_LEN];

        ret = spi_norflash_get_unique_id(spi_flash_id, unique_id, sizeof(unique_id));
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "flash get unique id failed, ret = %d", ret);
            goto _FUNC_FAIL;
        } else {
            SPI_FLASH_TEST_TRACE(0, "unique id:");
            DUMP8("0x%x ", unique_id, SPI_FLASH_UNIQUE_ID_LEN);
        }
    }
#endif

    ret = spi_norflash_get_size(spi_flash_id, &total_size, &block_size, &sector_size, &page_size);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1, "flash get size failed, ret = %d", ret);
        goto _FUNC_FAIL;
    } else {
        SPI_FLASH_TEST_TRACE(1, "total size :%d", total_size);
        SPI_FLASH_TEST_TRACE(1, "block size :%d", block_size);
        SPI_FLASH_TEST_TRACE(1, "sector size:%d", sector_size);
        SPI_FLASH_TEST_TRACE(1, "page size  :%d", page_size);
    }
#if defined(TEST_NORFLASH_ADDR_START) && defined(TEST_NORFLASH_ADDR_END)
    len = TEST_NORFLASH_ADDR_END - TEST_NORFLASH_ADDR_START;
    start_addr = TEST_NORFLASH_ADDR_START;
#else
    len = total_size;
#endif
    SPI_FLASH_TEST_TRACE(1, "spi sector erase/write/read check range, 0x%08X--0x%08X", 0, len);
    sec_cnt = len / SPI_NORFLASH_SECTOR_SIZE;
    for (i = start_addr; i < sec_cnt; i++) {
        SPI_FLASH_TEST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
        SPI_FLASH_TEST_TRACE(0, "******sector: %08u/%08u", i + 1, sec_cnt);

        addr = i * SPI_NORFLASH_SECTOR_SIZE;
        ret = spi_norflash_erase(spi_flash_id, addr, SPI_NORFLASH_SECTOR_SIZE);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: flash erase failed,addr:0x%X, ret = %d", __func__, addr, ret);
            goto _FUNC_FAIL;
        }
        SPI_FLASH_TEST_TRACE(1, "---flash erase addr:0x%X done", addr);

        ret = spi_norflash_read(spi_flash_id, addr, test_buff_r, SPI_NORFLASH_SECTOR_SIZE);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: flash read failed,addr:0x%X, ret = %d", __func__, addr, ret);
            goto _FUNC_FAIL;
        }

        if (_cmp_pattern(test_buff_r, 0xFF, SPI_NORFLASH_SECTOR_SIZE, &diff_offs)== 0) {
            SPI_FLASH_TEST_TRACE(1, "---erase check ok,addr:0x%X", addr);
        } else {
            SPI_FLASH_TEST_TRACE(1, "%s: erase check error, diff_offs = 0x%x", __func__, diff_offs);
            DUMP8("0x%02X ", &test_buff_r[diff_offs], 16);
            goto _FUNC_FAIL;
        }

        _set_buffer(test_buff_w, SPI_NORFLASH_SECTOR_SIZE, SET_BUFFER_MODE_1, 0);

        ret = spi_norflash_write(spi_flash_id, addr, test_buff_w, SPI_NORFLASH_SECTOR_SIZE);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: flash write failed, ret = %d", __func__, ret);
            goto _FUNC_FAIL;
        }
        SPI_FLASH_TEST_TRACE(1, "---flash write addr:0x%X done", addr);

        ret = spi_norflash_read(spi_flash_id, addr, test_buff_r, SPI_NORFLASH_SECTOR_SIZE);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: flash read failed, ret = %d", __func__, ret);
            goto _FUNC_FAIL;
        }
        SPI_FLASH_TEST_TRACE(1, "---flash read addr:0x%X done", addr);

        if (_cmp_buffer(test_buff_r, test_buff_w, SPI_NORFLASH_SECTOR_SIZE, &diff_offs) == 0) {
            //DUMP8("0x%02X ", &test_buff_r[diff_offs], 16);
            //DUMP8("0x%02X ", &test_buff_w[diff_offs], 16);
            SPI_FLASH_TEST_TRACE(1, "---write check ok,addr:0x%X", addr);
        } else {
            SPI_FLASH_TEST_TRACE(1,"%s SECTOR DATA ERROR: addr = 0x%x, data[%u] (0x%x) != 0x%x",
              __func__, addr, diff_offs, test_buff_r[diff_offs], test_buff_w[diff_offs]);

            DUMP8("0x%02X ", &test_buff_r[diff_offs], 16);
            DUMP8("0x%02X ", &test_buff_w[diff_offs], 16);
            goto _FUNC_FAIL;
        }
    }

    SPI_FLASH_TEST_TRACE(1, "spi sector erase/write/read check done, 0x%08X--0x%08X", start_addr, sec_cnt * SPI_NORFLASH_SECTOR_SIZE);
    SPI_FLASH_TEST_TRACE(1, "%s: done", __func__);
    SPI_FLASH_TEST_TRACE(0, "SPI NORFLASH TEST PASS");
    return 0;

_FUNC_FAIL:
    SPI_FLASH_TEST_TRACE(0, "SPI NORFLASH TEST NG");
    return ret;
}

//-------------------------------------------------------------------
// nandflash
//-------------------------------------------------------------------
#define PAGE_SIZE_MAX                 4096
#define OOB_SIZE_MAX                  (PAGE_SIZE_MAX/16)
#define BAD_BLOCK_MAX                 10
#define NANDFLASH_BAD_BLOCK_MAX       64

#define BLOCK_BAD_TYPE_OLD_STR     "OLD BAD"
#define BLOCK_BAD_TYPE_ERASE_STR   "ERASE BAD"
#define BLOCK_BAD_TYPE_PROGRAM_STR "PROGRAM BAD"

#define MAX_TEST_BLOCK_CNT              0
#define ALIGN4                          ALIGNED(4)

enum BLOCK_BAD_TYPE {
    BLOCK_BAD_TYPE_OLD,
    BLOCK_BAD_TYPE_ERASE,
    BLOCK_BAD_TYPE_PROGRAM,
    };

typedef struct {
    uint32_t bad_block_num;
    uint32_t bad_page_count;
    uint32_t bad_page_bmp[4];
    enum BLOCK_BAD_TYPE bad_type;
}BAD_BLOCK_INFO;

typedef struct {
    uint32_t erase_time;
    uint32_t erase_size;
    uint32_t program_time;
    uint32_t program_size;
    uint32_t read_time;
    uint32_t read_size;
}FLASH_PERFORMANCE;


typedef struct {
    uint32_t bad_block_count;
    BAD_BLOCK_INFO bad_block_info[NANDFLASH_BAD_BLOCK_MAX];
    FLASH_PERFORMANCE performance;
    uint32_t test_case_bmp;
    uint32_t test_case_result_bmp;
}NANDFLASH_TEST_RESULT;

static uint16_t bad_block_list[BAD_BLOCK_MAX];

static uint16_t bad_block_cnt;

static uint32_t nandflash_total_size;
static uint32_t nandflash_block_size;
static uint32_t nandflash_page_size;
static uint32_t nandflash_page_oob_size;
static uint32_t nandflash_die_num;

static NANDFLASH_TEST_RESULT spi_nandflash_test_result[HAL_FLASH_ID_NUM];

POSSIBLY_UNUSED
static int append_bad_block_list(uint32_t block)
{
    if (bad_block_cnt >= ARRAY_SIZE(bad_block_list)) {
        SPI_FLASH_TEST_TRACE(1, "Too many bad blocks: %u (should < %u)", (bad_block_cnt + 1), ARRAY_SIZE(bad_block_list));
        return 1;
    }
    bad_block_list[bad_block_cnt++] = block;
    return 0;
}

POSSIBLY_UNUSED
static int in_bad_block_list(uint32_t block)
{
    for (uint32_t i = 0; i < bad_block_cnt; i++) {
        if (bad_block_list[i] == block) {
            return true;
        }
    }

    return false;
}

POSSIBLY_UNUSED
static int get_bad_block_count(void)
{
    return bad_block_cnt;
}

static void  _test_result_init(enum HAL_FLASH_ID_T id)
{
    memset((uint8_t*)&spi_nandflash_test_result[id], 0, sizeof(spi_nandflash_test_result[id]));
}

static void _test_result_print(enum HAL_FLASH_ID_T id)
{
    uint32_t i;
    uint8_t device_id[SPI_FLASH_DEVICE_ID_LEN];

    SPI_FLASH_TEST_TRACE(1, "--------NANDFLASH %d DEVICE INFORMATION--------------\n", id);

    spi_nandflash_get_id(id, device_id, SPI_FLASH_DEVICE_ID_LEN);
    SPI_FLASH_TEST_TRACE(1, "FLASH ID:             0x%02x-0x%02x-0x%02x", device_id[0],device_id[1],device_id[2]);
    SPI_FLASH_TEST_TRACE(1, "TOTAL SIZE:           0x%08x(byte)", nandflash_total_size);
    SPI_FLASH_TEST_TRACE(1, "BLOCK SIZE:           0x%08x(byte)", nandflash_block_size);
    SPI_FLASH_TEST_TRACE(1, "PAGE SIZE:            0x%08x(byte)", nandflash_page_size);
    SPI_FLASH_TEST_TRACE(1, "PAGE OOB SIZE:        0x%08x(byte)", nandflash_page_oob_size);
    SPI_FLASH_TEST_TRACE(1, "DIE SIZE:             0x%08x(byte)", nandflash_die_num);

    SPI_FLASH_TEST_TRACE(1, "--------NANDFLASH %d TEST RESULT---------------------\n", id);

        if (spi_nandflash_test_result[id].test_case_bmp & NANDFLASH_TEST_BASE_FUCTION_ON) {
            if (spi_nandflash_test_result[id].test_case_result_bmp & NANDFLASH_TEST_BASE_FUCTION_ON) {
                SPI_FLASH_TEST_TRACE(1, "NANDFLASH TEST BASE FUNCTION PASS");
            } else {
                SPI_FLASH_TEST_TRACE(1, "NANDFLASH TEST BASE FUNCTION NG");
            }
        }

        SPI_FLASH_TEST_TRACE(1, "BAD BLODK COUNT:  %d", spi_nandflash_test_result[id].bad_block_count);

        SPI_FLASH_TEST_TRACE(1, "ERASE SIZE:       0x%x(KB)  ERASE TIME:   0x%08x(ms)",
              spi_nandflash_test_result[id].performance.erase_size/1024,spi_nandflash_test_result[id].performance.erase_time/1000);
        SPI_FLASH_TEST_TRACE(1, "PROGRAM SIZE:     0x%x(KB)  PROGRAM TIME: 0x%08x(ms)",
              spi_nandflash_test_result[id].performance.program_size/1024,spi_nandflash_test_result[id].performance.program_time/1000);

        SPI_FLASH_TEST_TRACE(1, "READ SIZE:        0x%x(KB)  READ TIME:    0x%08x(ms)",
              spi_nandflash_test_result[id].performance.read_size/1024,spi_nandflash_test_result[id].performance.read_time/1000);

        SPI_FLASH_TEST_TRACE(1, "BLOCK ERASE SPEED:      %08d (KB/s)",
            ((spi_nandflash_test_result[id].performance.erase_size/1024)*1000)/(spi_nandflash_test_result[id].performance.erase_time/1000));
        SPI_FLASH_TEST_TRACE(1, "BLOCK PROGRAM SPEED:    %08d (KB/s)",
            ((spi_nandflash_test_result[id].performance.program_size/1024)*1000)/(spi_nandflash_test_result[id].performance.program_time/1000));
        SPI_FLASH_TEST_TRACE(1, "BLOCK READ    SPEED:    %08d (KB/s)",
            ((spi_nandflash_test_result[id].performance.read_size/1024)*1000)/(spi_nandflash_test_result[id].performance.read_time/1000));

        SPI_FLASH_TEST_TRACE(1, "-------------------BAD BLOCK------------------------------\n");
        SPI_FLASH_TEST_TRACE(1, "Bad block number: %d", spi_nandflash_test_result[id].bad_block_count);
        for (i = 0; i < spi_nandflash_test_result[id].bad_block_count; i++) {
            SPI_FLASH_TEST_TRACE(1, "BAD BLOCK NUMBER:  %d", spi_nandflash_test_result[id].bad_block_info[i].bad_block_num);
            SPI_FLASH_TEST_TRACE(1, "BAD PAGE COUNT :   %d", spi_nandflash_test_result[id].bad_block_info[i].bad_page_count);
            if (spi_nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_OLD) {
                SPI_FLASH_TEST_TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_OLD_STR);
            } else if (spi_nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_ERASE) {
                SPI_FLASH_TEST_TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_ERASE_STR);
            } else if (spi_nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_PROGRAM) {
                SPI_FLASH_TEST_TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_PROGRAM_STR);
            }
            SPI_FLASH_TEST_TRACE(1, "BAD PAGE BMP:0x%08x,0x%08x,0x%08x,0x%08x",
                     spi_nandflash_test_result[id].bad_block_info[i].bad_page_bmp[3],
                     spi_nandflash_test_result[id].bad_block_info[i].bad_page_bmp[2],
                     spi_nandflash_test_result[id].bad_block_info[i].bad_page_bmp[1],
                     spi_nandflash_test_result[id].bad_block_info[i].bad_page_bmp[0]);
        }

        SPI_FLASH_TEST_TRACE(1, "-----------------------------------------------------------\n");
}

void spi_nandflash_test_set_test_case_bmp(enum HAL_FLASH_ID_T id, uint32_t case_id)
{
    spi_nandflash_test_result[id].test_case_bmp |= case_id;
}

void spi_nandflash_test_set_test_case_pass(enum HAL_FLASH_ID_T id, uint32_t case_id)
{
    spi_nandflash_test_result[id].test_case_result_bmp |= case_id;
}

static int base_functions_test(uint32_t block)
{
    int ret;
    uint32_t offs, len;
    uint32_t page;
    uint32_t programing_size;
    uint32_t diff_offs;
    uint16_t *p, *p1;
    uint32_t i;
    bool is_bad;

    SPI_FLASH_TEST_TRACE(1, "\n");
    SPI_FLASH_TEST_TRACE(1, "%s", __func__);

    ret = spi_nandflash_erase_block(spi_flash_id, block);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s ERASE FAIL:  %u, %d", __func__, block, ret);
        TRACE_FLUSH();
        ret = -1;
        goto _FUNC_FAIL;
    }

    page = 0;
    offs = 0;
    ret = spi_nandflash_read_page(spi_flash_id, block, page, offs, test_buff_r, nandflash_page_size);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1," spi_nandflash_read_page FAIL:  %u, %d", block, ret);
        TRACE_FLUSH();
        ret = -2;
        goto _FUNC_FAIL;
    }

    ret = _cmp_pattern(test_buff_r, 0xFF, nandflash_page_size, &diff_offs);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s data error block:%d, page:%d, offs: %d, data[%d](0xff): 0x%x",
              __func__, block, page, offs, diff_offs, test_buff_r[diff_offs]);
        DUMP8("0x%x,", (uint8_t*)&test_buff_r[diff_offs],16);
        ret = -3;
        goto _FUNC_FAIL;
    }
    SPI_FLASH_TEST_TRACE(1, "erase block %u done", block);

#ifdef NANDFLASH_TEST_OOB_PROGRAM
    programing_size = nandflash_page_size + nandflash_page_oob_size;
#else
    programing_size = nandflash_page_size;
#endif

    // set page buffer
    p = (uint16_t*)test_buff_w;
    for (i = 0; i < nandflash_page_size/sizeof(p[0]); i++) {
        p[i] = i;
    }
#ifdef NANDFLASH_TEST_OOB_PROGRAM
    // set page oob buffer.
    p = (uint16_t*)(test_buff_w + nandflash_page_size);
    for (i = 0; i < nandflash_page_oob_size/sizeof(p[0]); i++) {
        if (i < 1) {
            p[i] = 0xFFFF;
        } else {
            p[i] = 0x9000 | i;
        }
    }
#endif

    // page 0: write main + oob.
    page = 0;
    len = programing_size;
    p = (uint16_t*)test_buff_w;
    p1 = (uint16_t*)test_buff_r;

    ret = spi_nandflash_write_page(spi_flash_id, block, page, (uint8_t*)p, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE WRITE FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -4;
        goto _FUNC_FAIL;
    }

    ret = spi_nandflash_read_page(spi_flash_id, block, page, offs, (uint8_t*)p1, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE READ FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -5;
        goto _FUNC_FAIL;
    }

    ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, len, &diff_offs);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE DATA ERROR: %u, %u, %u, data[%d] (0x%x) != 0x%x",
              __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
        DUMP8("0x%x,", (uint16_t*)&p[diff_offs], 16);
        DUMP8("0x%x,", (uint16_t*)&p1[diff_offs], 16);
        ret = -6;
        goto _FUNC_FAIL;
    }

    SPI_FLASH_TEST_TRACE(1, "page 0: write main + oob test done");

#ifdef NANDFLASH_TEST_OOB_PROGRAM
    // page 1: write oobs.
    page = 1;
    offs = 0;
    len = nandflash_page_oob_size - offs;

    p = (uint16_t*)(test_buff_w + nandflash_page_size + offs);
    p1 = (uint16_t*)(test_buff_r + nandflash_page_size + offs);
    ret = spi_nandflash_write_oob(spi_flash_id, block, page, (uint8_t*)p, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s OOB WRITE FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -7;
        goto _FUNC_FAIL;
    }

    ret = spi_nandflash_read_oob(spi_flash_id, block, page, offs, (uint8_t*)p1, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s OOB READ FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -8;
        goto _FUNC_FAIL;
    }

    ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, len, &diff_offs);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s OOB DATA ERROR: %u, %u, %u, data[%d] (0x%x) != 0x%x",
              __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
        DUMP8("0x%x,", (uint8_t*)&p[diff_offs], 16);
        DUMP8("0x%x,", (uint8_t*)&p1[diff_offs], 16);
        ret = -9;
        goto _FUNC_FAIL;
    }
    SPI_FLASH_TEST_TRACE(1, "page 1: write oob test done");

#endif

    // page 2: update main.
    page = 2;
    offs = 0x100;
    len = nandflash_page_size - offs;
    p = (uint16_t*)(test_buff_w +  offs);
    p1 = (uint16_t*)(test_buff_r + offs);

    ret = spi_nandflash_update_page(spi_flash_id, block, page, offs, (uint8_t*)p, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE UPDATE FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -10;
        goto _FUNC_FAIL;
    }

    ret = spi_nandflash_read_page(spi_flash_id, block, page, offs, (uint8_t*)p1, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE READ FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -11;
        goto _FUNC_FAIL;
    }

    ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, len, &diff_offs);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE UPDATA DATA ERROR: %u, %u, %u, data[%d] (0x%x) != 0x%x",
              __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
        DUMP8("0x%x,", (uint8_t*)&p[diff_offs], 16);
        DUMP8("0x%x,", (uint8_t*)&p1[diff_offs], 16);
        ret = -12;
        goto _FUNC_FAIL;
    }
    SPI_FLASH_TEST_TRACE(1, "page 2: update main test done");

#ifdef NANDFLASH_TEST_OOB_PROGRAM
    // page 3: update oob.
    page = 3;
    offs = 0x10;
    len = nandflash_page_oob_size - offs;
    p = (uint16_t*)(test_buff_w + nandflash_page_size + offs);
    p1 = (uint16_t*)(test_buff_r + nandflash_page_size + offs);

    ret = spi_nandflash_update_oob(spi_flash_id, block, page, offs, (uint8_t*)p, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s PAGE UPDATE FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -13;
        goto _FUNC_FAIL;
    }

    ret = spi_nandflash_read_oob(spi_flash_id, block, page, offs, (uint8_t*)p1, len);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s OOB READ FAIL:  %u, %u, %d", __func__, block, page, ret);
        TRACE_FLUSH();
        ret = -14;
        goto _FUNC_FAIL;
    }

    ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, nandflash_page_oob_size - offs, &diff_offs);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"%s OOB UPDATA DATA ERROR: %u, %u, %u, data[%d] (0x%x) != 0x%x",
              __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
        DUMP8("0x%x,", (uint8_t*)&p[diff_offs], 16);
        DUMP8("0x%x,", (uint8_t*)&p1[diff_offs], 16);
        ret = -15;
        goto _FUNC_FAIL;
    }
    SPI_FLASH_TEST_TRACE(1, "page 3: update oob test done");
#endif
    // bad block mark test
    is_bad = spi_nandflash_block_is_bad(spi_flash_id, block);
    if (is_bad) {
        SPI_FLASH_TEST_TRACE(1, "BLOCK IS BAD: %u", block);
        ret = -16;
        goto _FUNC_FAIL;
    } else {
        SPI_FLASH_TEST_TRACE(1, "mark a bad block: %u", block);
        ret = spi_nandflash_mark_bad_block(spi_flash_id, block);
        if(ret) {
            SPI_FLASH_TEST_TRACE(1, "%s BAD BLOCK MARK FAIL: %u, %d", __func__, block, ret);
            ret = -17;
            goto _FUNC_FAIL;
        }

        SPI_FLASH_TEST_TRACE(1, "check if it is a bad block: %u", block);
        is_bad = spi_nandflash_block_is_bad(spi_flash_id, block);
        if (!is_bad) {
            SPI_FLASH_TEST_TRACE(1, "still a good block, BAD BLOCK MARK FAIL: %u", block);
            ret = -18;
            goto _FUNC_FAIL;
        }

        SPI_FLASH_TEST_TRACE(1, "clear bad block mard(erase block): %u", block);
        ret = spi_nandflash_erase_block(spi_flash_id, block);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1,"%s ERASE FAIL:  %u, %d", __func__, block, ret);
            TRACE_FLUSH();
            ret = -19;
            goto _FUNC_FAIL;
        }
        SPI_FLASH_TEST_TRACE(1, "bad block mard test done");
    }

    SPI_FLASH_TEST_TRACE(1, "%s done", __func__);
    return 0;

_FUNC_FAIL:
    SPI_FLASH_TEST_TRACE(1, "%s fail, error code: %d", __func__, ret);
    return ret;
}

static int erase_check(uint32_t block)
{
    uint32_t page;
    uint32_t page_cnt, offs;
    int ret = 0;
    uint32_t start_time, end_time;
    uint32_t ind = spi_nandflash_test_result[spi_flash_id].bad_block_count;
    bool is_bad = false;
    uint32_t diff_offs;

    if (block % 100 == 0) {
        SPI_FLASH_TEST_TRACE(1,"%s: block: %d", __func__, block);
    }

    page_cnt = nandflash_block_size/nandflash_page_size;

    start_time = hal_fast_sys_timer_get();
    ret = spi_nandflash_erase_block(spi_flash_id, block);
    end_time = hal_fast_sys_timer_get();

    if (ret) {
        SPI_FLASH_TEST_TRACE(1,"ERASE FAIL:  %u, %d", block, ret);
        TRACE_FLUSH();
    } else {
        spi_nandflash_test_result[spi_flash_id].performance.erase_time += FAST_TICKS_TO_US(end_time - start_time);
        spi_nandflash_test_result[spi_flash_id].performance.erase_size += nandflash_block_size;
        for (page = 0; page < page_cnt; page ++) {
            offs = 0;
            start_time = hal_fast_sys_timer_get();
            ret = spi_nandflash_read_page(spi_flash_id, block, page, offs, test_buff_r, nandflash_page_size);
            end_time = hal_fast_sys_timer_get();

            if (ret) {
                SPI_FLASH_TEST_TRACE(1," spi_nandflash_read_page FAIL:  %u, %d", block, ret);
                TRACE_FLUSH();
                is_bad = true;
                ret = -1;
                goto __PAGE_FAIL;
            } else {
                spi_nandflash_test_result[spi_flash_id].performance.read_time += FAST_TICKS_TO_US(end_time - start_time);
                spi_nandflash_test_result[spi_flash_id].performance.read_size += nandflash_page_size;
            }
            ret = _cmp_pattern(test_buff_r, 0xFF, nandflash_page_size, &diff_offs);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1,"%s data error block:%d, page:%d, offs: %d, data[%d](0xff): 0x%x",
                      __func__, block, page, offs, diff_offs, test_buff_r[diff_offs]);
                DUMP8("0x%x,", (uint8_t*)&test_buff_r[diff_offs],16);
                is_bad = true;
                ret = -2;
                goto __PAGE_FAIL;
            }

            offs = 0;
            ret = spi_nandflash_read_oob(spi_flash_id, block, page, offs, test_buff_r, nandflash_page_oob_size);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1, "%s: spi_nandflash_read_oob FAIL:  %u, %d", __func__, block, ret);
                TRACE_FLUSH();
                is_bad = true;
                ret = -3;
                goto __PAGE_FAIL;
            }

            ret = _cmp_pattern(test_buff_r, 0xFF, nandflash_page_size, &diff_offs);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1,"%s oob error block:%d, page:%d, offse: %d, data[%d](0xff): 0x%x",
                      __func__, block, page, offs, diff_offs, test_buff_r[diff_offs]);
                DUMP8("0x%x,", (uint8_t*)&test_buff_r[diff_offs],16);
                is_bad = true;
                ret = -4;
                goto __PAGE_FAIL;
            }

            continue;
__PAGE_FAIL:
            spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_page_bmp[page/32] |= (0x1 << (page - (page/32)*32));
            spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_page_count ++;
        }
    }

    if (is_bad) {
        ret = -1;
        goto _FUNC_FAIL;
    }

    if (block % 100 == 0) {
        SPI_FLASH_TEST_TRACE(1,"%s block: %d DONE", __func__, block);
    }
    return 0;

_FUNC_FAIL:
    spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_block_num = block;
    spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_ERASE;
    SPI_FLASH_TEST_TRACE(1,"%s block %d FAIL, ret = %d", __func__, block, ret);
    return ret;
}

static int program_check(uint32_t block)
{
    uint32_t page;
    uint32_t programing_size;
    uint32_t page_cnt, i, offs;
    int ret = 0;
    uint16_t *p, *p1;
    uint32_t start_time, end_time;
    uint32_t ind = spi_nandflash_test_result[spi_flash_id].bad_block_count;
    bool is_bad = false;
    uint32_t diff_offs;

    if (block % 100 == 0) {
        SPI_FLASH_TEST_TRACE(1,"%s: block: %d", __func__, block);
    }

    page_cnt = nandflash_block_size/nandflash_page_size;
    // SPI_FLASH_TEST_TRACE(1, "page_oob_size = %d", page_oob_size);

#ifdef NANDFLASH_TEST_OOB_PROGRAM
    programing_size = nandflash_page_size + nandflash_page_oob_size;
#else
    programing_size = nandflash_page_size;
#endif

    // set page buffer
    p = (uint16_t*)test_buff_w;
    for (i = 0; i < nandflash_page_size/sizeof(p[0]); i++) {
        p[i] = i;
    }
#ifdef NANDFLASH_TEST_OOB_PROGRAM
    // set page oob buffer.
    p = (uint16_t*)(test_buff_w + nandflash_page_size);
    for (i = 0; i < nandflash_page_oob_size/sizeof(p[0]); i++) {
        if (i < 1) {
            // Bad Block Mark
            p[i] = 0xFFFF;
        } else {
            p[i] = 0x9000 | i;
        }
    }
#endif

    for (page = 0; page < page_cnt; page ++) {
        start_time = hal_fast_sys_timer_get();
        ret = spi_nandflash_write_page(spi_flash_id, block, page, test_buff_w, programing_size);
        end_time = hal_fast_sys_timer_get();
        if (ret) {
            SPI_FLASH_TEST_TRACE(1,"%s write page FAIL:  %u, %d", __func__, block, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -1;
            goto __PAGE_FAIL;
        }

        offs = 0;
        ret = spi_nandflash_read_page(spi_flash_id, block, page, offs, test_buff_r, programing_size);
        if (ret != SPI_FLASH_OK && ret != SPI_FLASH_READ_CORRECTED) {
            SPI_FLASH_TEST_TRACE(1,"%s read page FAIL:  %u, %d, %d", __func__, block, page, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -2;
            goto __PAGE_FAIL;
        }

        p = (uint16_t*)test_buff_w;
        p1 = (uint16_t*)test_buff_r;
        ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, nandflash_page_size, &diff_offs);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1,"%s data error block:%d, page:%d, offs: %d, data[%d](0x%x): 0x%x",
                  __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
            DUMP16("0x%04x,", (uint8_t*)&p[diff_offs],16);
            DUMP16("0x%04x,", (uint8_t*)&p1[diff_offs],16);
            is_bad = true;
            ret = -3;
            goto __PAGE_FAIL;
        }

#ifdef NANDFLASH_TEST_OOB_PROGRAM
        offs = 0;
        ret = spi_nandflash_read_oob(spi_flash_id, block, page, offs, test_buff_r + nandflash_page_size, nandflash_page_oob_size);
        if (ret != SPI_FLASH_OK && ret != SPI_FLASH_READ_CORRECTED) {
            SPI_FLASH_TEST_TRACE(1,"%s read page FAIL:  %u, %d, %d", __func__, block, page, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -2;
            goto __PAGE_FAIL;
        }

        p = (uint16_t*)(test_buff_w + nandflash_page_size);
        p1 = (uint16_t*)(test_buff_r + nandflash_page_size);
        ret = _cmp_buffer((uint8_t*)p, (uint8_t*)p1, nandflash_page_size, &diff_offs);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1,"%s oob data error block:%d, page:%d, offs: %d, data[%d](0x%x): 0x%x",
                  __func__, block, page, offs, diff_offs, p[diff_offs], p1[diff_offs]);
            DUMP16("0x%04x,", (uint8_t*)&p[diff_offs],16);
            DUMP16("0x%04x,", (uint8_t*)&p1[diff_offs],16);
            is_bad = true;
            ret = -4;
            goto __PAGE_FAIL;
        }

        //DUMP16("%04x ", p, 16);
        //DUMP16("%04x ", p1, 16);
#endif
        spi_nandflash_test_result[spi_flash_id].performance.program_time += FAST_TICKS_TO_US(end_time - start_time);
        spi_nandflash_test_result[spi_flash_id].performance.program_size += nandflash_page_size;
        continue;
__PAGE_FAIL:
        spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_page_bmp[page/32] |= (0x1 << (page - (page/32)*32));
        spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_page_count ++;
        break;
    }

    if (is_bad) {
        ret = -1;
        goto _FUNC_FAIL;
    }

    if (block % 100 == 0) {
        SPI_FLASH_TEST_TRACE(1,"%s block: %d DONE", __func__, block);
    }
    return 0;

_FUNC_FAIL:
    spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_block_num = block;
    spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_PROGRAM;
    SPI_FLASH_TEST_TRACE(1,"%s block %d FAIL, ret = %d", __func__, block, ret);
    return ret;
}

void _spi_nandflash_test(void)
{
    int ret;
    uint32_t block;
    uint32_t start_block = 0;
    uint32_t test_blocks;
    uint8_t cur_die;
    uint32_t blocks_per_die;
    uint32_t pages_per_block;
    uint32_t bad_block_count;
    uint8_t device_id[SPI_FLASH_DEVICE_ID_LEN];
#ifndef NANDFLASH_TEST_SKIP_BAD_BLOCK_CHECK
    uint32_t ind;
#endif

    SPI_FLASH_TEST_TRACE(0, " ");
    SPI_FLASH_TEST_TRACE(0, "SPI NANDFLASH TEST");
    SPI_FLASH_TEST_TRACE(1, " ");
#if defined(FLASH_UNIQUE_ID)
    {
        uint8_t unique_id[SPI_FLASH_UNIQUE_ID_LEN + SPI_FLASH_CP_ID_LEN];

        ret = spi_nandflash_get_unique_id(spi_flash_id, unique_id, sizeof(unique_id));
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "flash get unique id failed, ret = %d", ret);
            goto _FUNC_FAIL;
        } else {
            SPI_FLASH_TEST_TRACE(0, "unique id:");
            DUMP8("%02x ", unique_id, SPI_FLASH_UNIQUE_ID_LEN);
        }
    }
#endif

    _test_result_init(spi_flash_id);

    spi_nandflash_get_id(spi_flash_id, device_id, sizeof(device_id));
    SPI_FLASH_TEST_TRACE(0,"FLASH ID: %02X-%02X-%02X", device_id[0], device_id[1], device_id[2]);

    spi_nandflash_get_size(spi_flash_id, &nandflash_total_size, &nandflash_block_size,
                               &nandflash_page_size, &nandflash_page_oob_size, &nandflash_die_num);
    SPI_FLASH_TEST_TRACE(0,"total_size = 0x%x, block_size = 0x%x, page_size = 0x%x", nandflash_total_size, nandflash_block_size, nandflash_page_size);
    SPI_FLASH_TEST_TRACE(0,"page_oob_size = 0x%x, die_num = 0x%x", nandflash_page_oob_size, nandflash_die_num);

    nandflash_die_num = nandflash_die_num == 0 ? 1 : nandflash_die_num;

    blocks_per_die = nandflash_total_size / nandflash_block_size / nandflash_die_num;
    pages_per_block = nandflash_block_size/nandflash_page_size;
    SPI_FLASH_TEST_TRACE(0,"blocks_per_die = 0x%x, pages_per_block = 0x%x", blocks_per_die, pages_per_block);

    // base function test
    spi_nandflash_test_set_test_case_bmp(spi_flash_id, NANDFLASH_TEST_BASE_FUCTION_ON);
 #ifdef TEST_BLOCK_START
    start_block = TEST_BLOCK_START;
 #else
    start_block = 0;
 #endif
 #ifdef TEST_BLOCK_END
    test_blocks = TEST_BLOCK_END;
 #else
    test_blocks = blocks_per_die;
 #endif

    block = 1;
    ret = base_functions_test(block);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1, "%s: base_functions_test(%d) FAIL, ret = %d", __func__, block, ret);
        goto _FUNC_FAIL;
    }

    SPI_FLASH_TEST_TRACE(1, "\n");
    SPI_FLASH_TEST_TRACE(1, "nandflash full scan");
    for (cur_die = 0; cur_die < nandflash_die_num; cur_die ++) {
#ifndef NANDFLASH_TEST_SKIP_BAD_BLOCK_CHECK
        SPI_FLASH_TEST_TRACE(0,"%s: DIE(%d) BAD BLOCK CHECK, test block cnt: %d", __func__, cur_die, test_blocks);
        for (block = start_block; block < test_blocks; block++) {
            ret = spi_nandflash_block_is_bad(spi_flash_id, block);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1, "%s: DIE(%d) BAD BLOCK:,BLOCK(%d).", __func__, cur_die, block + cur_die*blocks_per_die);
                ret = append_bad_block_list(block);
                if (ret) {
                    goto _FUNC_FAIL;
                }
                ind = spi_nandflash_test_result[spi_flash_id].bad_block_count;
                spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_block_num = block;
                spi_nandflash_test_result[spi_flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_OLD;
                spi_nandflash_test_result[spi_flash_id].bad_block_count ++;

            }
#ifdef NANDFLASH_TEST_PD_SLEEP
            break;
#endif
        }
        SPI_FLASH_TEST_TRACE(1, "%s: DIE(%d) BAD BLOCK CHECK DONE. BAD BLOCK COUNT: %d", __func__, cur_die, get_bad_block_count());
#endif // !NANDFLASH_TEST_SKIP_BAD_BLOCK_CHECK
        SPI_FLASH_TEST_TRACE(1, "%s: DIE(%d) ERASE BLOCK CHECK, block cnt: %d", __func__, cur_die, test_blocks);
        for (block = start_block; block < test_blocks; block++) {
            if (in_bad_block_list(block)) {
                SPI_FLASH_TEST_TRACE(0,"%s: DIE(%d) skip bad block(%d) ", __func__, cur_die, block);
                continue;
            }
            ret = erase_check(block + cur_die*blocks_per_die);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1, "%s: DIE(%d) erase_check(%d) FAIL, ret = %d", __func__, cur_die, block + cur_die*blocks_per_die, ret);
                ret = append_bad_block_list(block);
                if (ret) {
                    goto _FUNC_FAIL;
                }
                spi_nandflash_test_result[spi_flash_id].bad_block_count ++;
                continue;
            }
            ret = program_check(block + cur_die*blocks_per_die);
            if (ret) {
                SPI_FLASH_TEST_TRACE(1, "%s: DIE(%d) program_check(%d) FAIL, ret = %d", __func__, cur_die, block + cur_die*blocks_per_die, ret);
                ret = append_bad_block_list(block);
                if (ret) {
                    goto _FUNC_FAIL;
                }
                spi_nandflash_test_result[spi_flash_id].bad_block_count ++;
                continue;
            }
#ifdef NANDFLASH_TEST_PD_SLEEP
            start_block = block + 1;
            break;
#endif
        }
        bad_block_count = get_bad_block_count();
        SPI_FLASH_TEST_TRACE(0,"%s: total block: %d, bad block: %d", __func__, blocks_per_die, bad_block_count);
        SPI_FLASH_TEST_TRACE(0,"%s: DIE(%d) CHECK DONE", __func__, cur_die);
    }
    SPI_FLASH_TEST_TRACE(1, "nandflash full scan done");
    SPI_FLASH_TEST_TRACE(1, "\n");
    spi_nandflash_test_set_test_case_pass(spi_flash_id, NANDFLASH_TEST_BASE_FUCTION_ON);
    _test_result_print(spi_flash_id);
    SPI_FLASH_TEST_TRACE(0,"NANDFLASH TEST PASS");
    SPI_FLASH_TEST_TRACE(1, "\n");
    TRACE_FLUSH();
    // osDelay(1000);
    return;

_FUNC_FAIL:
    SPI_FLASH_TEST_TRACE(0,"NANDFLASH TEST NG");
    SPI_FLASH_TEST_TRACE(1, "\n");
    TRACE_FLUSH();
    return;
}

void _spi_nandflash_simple_test(void)
{
    int ret;
    uint32_t block = 1;
    uint32_t blocks_per_die;
    uint32_t pages_per_block;
    uint8_t device_id[SPI_FLASH_DEVICE_ID_LEN];

    SPI_FLASH_TEST_TRACE(1, " ");
    SPI_FLASH_TEST_TRACE(1, "SPI NANDFLASH SIMPLE TEST");
    SPI_FLASH_TEST_TRACE(1, " ");
    spi_nandflash_get_id(spi_flash_id, device_id, sizeof(device_id));
    SPI_FLASH_TEST_TRACE(0,"FLASH ID: %02X-%02X-%02X", device_id[0], device_id[1], device_id[2]);

    spi_nandflash_get_size(spi_flash_id, &nandflash_total_size, &nandflash_block_size,
                               &nandflash_page_size, &nandflash_page_oob_size, &nandflash_die_num);
    SPI_FLASH_TEST_TRACE(0,"total_size = 0x%x, block_size = 0x%x, page_size = 0x%x", nandflash_total_size, nandflash_block_size, nandflash_page_size);
    SPI_FLASH_TEST_TRACE(0,"page_oob_size = 0x%x, die_num = 0x%x", nandflash_page_oob_size, nandflash_die_num);

    nandflash_die_num = nandflash_die_num == 0 ? 1 : nandflash_die_num;

    blocks_per_die = nandflash_total_size / nandflash_block_size / nandflash_die_num;
    pages_per_block = nandflash_block_size/nandflash_page_size;
    SPI_FLASH_TEST_TRACE(0,"blocks_per_die = 0x%x, pages_per_block = 0x%x", blocks_per_die, pages_per_block);

    do {
        if (in_bad_block_list(block)) {
            SPI_FLASH_TEST_TRACE(0,"%s: skip bad block(%u) ", __func__, block);
            block ++;
            continue;
        }

        ret = spi_nandflash_block_is_bad(spi_flash_id, block);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: skip bad block(%u)", __func__, block);
            ret = append_bad_block_list(block);
            if (ret) {
                goto _FUNC_FAIL;
            }
            block ++;
            continue;
        }

        SPI_FLASH_TEST_TRACE(1, "%s block = %u", __func__, block);

        ret = base_functions_test(block);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: base_functions_test(%d) FAIL, ret = %d", __func__, block, ret);
            goto _FUNC_FAIL;
        }

        SPI_FLASH_TEST_TRACE(1, "\n");
        ret = erase_check(block);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: erase_check(%d) FAIL, ret = %d", __func__, block, ret);
            goto _FUNC_FAIL;
        }

        SPI_FLASH_TEST_TRACE(1, "\n");
        ret = program_check(block);
        if (ret) {
            SPI_FLASH_TEST_TRACE(1, "%s: program_check(%d) FAIL, ret = %d", __func__, block, ret);
            goto _FUNC_FAIL;
        }
        //TRACE(1, "program check done");

    } while(0);

    SPI_FLASH_TEST_TRACE(1, "\n");
    SPI_FLASH_TEST_TRACE(0, "SPI NANDFLASH SIMPLE TEST PASS");
    return;

_FUNC_FAIL:
    SPI_FLASH_TEST_TRACE(0, "SPI NANDFLASH SIMPLE TEST NG");
    return;
}

void spi_flash_test(void)
{
    enum SPI_FLASH_RET_T ret;
    enum SPI_FLASH_TYPE_T flash_type;
    uint32_t cs_pin;

    SPI_FLASH_TEST_TRACE(1, " ");
    SPI_FLASH_TEST_TRACE(1, "%s", __func__);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    SPI_FLASH_TEST_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

    cs_pin = get_spi_cs_pin();
    ret = spi_flash_open(spi_flash_id, SPI_RATE, cs_pin);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1, "flash open failed, ret = %d", ret);
        goto _FUNC_FAIL;
    }
    // SPI_FLASH_TEST_TRACE(1, "flash(%d) init done", spi_flash_id);

    ret = spi_flash_get_type(spi_flash_id, &flash_type);
    if (ret) {
        SPI_FLASH_TEST_TRACE(1, "get flash type failed, ret = %d", ret);
        goto _FUNC_FAIL;
    }

    if (flash_type == SPI_FLASH_TYPE_NORFLASH) {
        SPI_FLASH_TEST_TRACE(1, "flash type: NORFLASH");
        _spi_norflash_test();
         //SPI_FLASH_TEST_TRACE(1, "spi sector erase/write/read check done, 0x%08X--0x%08X", TEST_NORFLASH_ADDR_START, TEST_NORFLASH_ADDR_END);
    } else {
        SPI_FLASH_TEST_TRACE(1, "flash type: NANDFLASH");
        _spi_nandflash_test();
        // _spi_nandflash_simple_test();
    }

    // spi_flash_close(spi_flash_id);
    SPI_FLASH_TEST_TRACE(1, "%s done", __func__);
    SPI_FLASH_TEST_TRACE(1, "\n");
    return;
_FUNC_FAIL:
    SPI_FLASH_TEST_TRACE(1, "%s failed", __func__);
    SPI_FLASH_TEST_TRACE(1, "\n");
    return;
}

#endif //SPI_FLASH_TEST
