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
#ifdef NANDFLASH_TEST
#include <stdio.h>
#include <stdlib.h>
#include "hwtest.h"
#include "nandflash_drv.h"
#include "string.h"
#include "hexdump.h"

//#define NAND_FLASH_TEST_OOB_PROGRAM
//#define READ_SPECIAL_BLOCK_PAGE
//#define UNALIGNED_DMA_ACCESS
#define PAGE_SIZE_MAX   4096
#define OOB_SIZE_MAX    (PAGE_SIZE_MAX/16)
#define BAD_BLOCK_MAX   100

static enum HAL_FLASH_ID_T flash_id = HAL_FLASH_ID_1;

#define MAX_TEST_BLOCK_CNT              0
#define ALIGN4                          ALIGNED(4)

ALIGN4
static uint8_t data_buf[PAGE_SIZE_MAX];

#ifdef NAND_FLASH_TEST_OOB_PROGRAM
ALIGN4
static uint8_t oob_buf[PAGE_SIZE_MAX];
#endif

POSSIBLY_UNUSED
ALIGN4
static uint8_t oob_read_buf[PAGE_SIZE_MAX];
static uint16_t bad_block_list[BAD_BLOCK_MAX];

static uint16_t bad_block_cnt;

POSSIBLY_UNUSED
static void append_bad_block_list(uint32_t block)
{
    if (bad_block_cnt >= ARRAY_SIZE(bad_block_list)) {
        ASSERT(false, "Too many bad blocks: %u (should < %u)", (bad_block_cnt + 1), ARRAY_SIZE(bad_block_list));
    }
    bad_block_list[bad_block_cnt++] = block;
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

#if 0

void nandflash_test(void)
{
    int init_ret;
    int ret;
    uint32_t block_cnt;
    uint32_t block;
    uint32_t page;
    uint32_t total_cnt;
    uint32_t err_cnt;
    bool bad;
    uint32_t i;
    uint8_t data;
    uint8_t cur_die;
    uint32_t src;
    uint32_t dst;
    uint32_t total_size, block_size, page_size, spare_size, die_num;
    uint32_t blocks_per_die;
    uint8_t device_id[NANDFLASH_ID_LEN];

    TRACE(0,"%s: start", __func__);
    init_ret = nandflash_init(flash_id);
    nandflash_get_id(flash_id, device_id, sizeof(device_id));
    TRACE(1, "NAND ID: 0x%02x-0x%02x-0x%02x",
                device_id[0],
                device_id[1],
                device_id[2]);
    TRACE_FLUSH();
    ASSERT(init_ret == NANDFLASH_RET_OK, "Failed to init nandflash: %d", init_ret);

    nandflash_get_size(flash_id, &total_size, &block_size, &page_size, &spare_size, &die_num);
    TRACE(0,"total_size=%u block_size=%u", total_size, block_size);
    TRACE(0,"page_size=%u spare_size=%u die_num=%u", page_size, spare_size, die_num);

    if (die_num) {
        blocks_per_die = total_size / block_size / die_num;
    } else {
        blocks_per_die = 0;
    }

    nandflash_show_calib_result(flash_id);

    nandflash_dump_features(flash_id);

    block_cnt = total_size / block_size;

    TRACE(0,"Bad blocks: ");
    bad_block_cnt = 0;
    for (block = 0; block < block_cnt; block++) {
        bad = nandflash_block_is_bad(flash_id, block);
        if (bad) {
            TRACE(0,"  %u", block);
            TRACE_FLUSH();
            append_bad_block_list(block);
        }
    }
    TRACE(0,"Scan done for %u blocks (%u bad)", block, bad_block_cnt);
    TRACE_FLUSH();
    if (MAX_TEST_BLOCK_CNT && block_cnt > MAX_TEST_BLOCK_CNT) {
        block_cnt = MAX_TEST_BLOCK_CNT;
    }

    // Get block for single block test
    for (block = 10; block < block_cnt; block++) {
        bad = in_bad_block_list(block);
        if (!bad) {
            break;
        }
    }
    src = block;
    for (block = src + 2; block < block_cnt; block++) {
        bad = in_bad_block_list(block);
        if (!bad) {
            break;
        }
    }
    dst = block;
    if (dst >= block_cnt) {
        ASSERT(false, "Cannot find 2 good blocks: %u %u", src, dst);
    }

#ifdef READ_SPECIAL_BLOCK_PAGE
    TRACE(0,"");
    TRACE_IMM("Read special block page");
    block = 1024;
    page = 0;
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read special block page failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"Normal content for block=%u page=%u:", block, page);
    dump_buffer_imm(data_buf, page_size);
    ret = nandflash_read_oob(flash_id, block, page, 0, data_buf, spare_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read special block page oob failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"Oob content for block=%u page=%u:", block, page);
    dump_buffer_imm(data_buf, spare_size);
    page = 1;
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read special block page failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"Normal content for block=%u page=%u:", block, page);
    dump_buffer_imm(data_buf, page_size);
    ret = nandflash_read_oob(flash_id, block, page, 0, data_buf, spare_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read special block page oob failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"Oob content for block=%u page=%u:", block, page);
    dump_buffer_imm(data_buf, spare_size);
    while (1);
#endif

#ifdef UNALIGNED_DMA_ACCESS
    TRACE(0,"");
    TRACE(0,"Unaligned DMA access");
    block = dst;
    page = 0;
    TRACE(0,"  UnalignedDMA %u-%u", block, page);
    ret = nandflash_erase_block(flash_id, block);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Erase failed for block=%u: %d", block, ret);
    }
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    for (i = 0; i < page_size; i++) {
        if (data_buf[i] != 0xFF) {
            ASSERT(false, "  Erase verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], 0xFF);
        }
    }
    // Unaligned DMA program
    uint32_t write_len;
    uint32_t extra_len = 8;
    write_len = 99;
    for (i = 0; i < page_size; i++) {
        data_buf[i] = 0xAA + i + block + page;
    }
    TRACE(0,"UnalignedDMA source (write_len=%u):", write_len);
    dump_buffer_imm(data_buf, write_len);
    TRACE(0,"UnalignedDMA extra (extra_len=%u):", extra_len);
    dump_buffer_imm(data_buf + write_len, extra_len);
    ret = nandflash_write_page(flash_id, block, page, data_buf, write_len);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Write page failed: %d", ret);
    }
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"UnalignedDMA after write (write_len=%u):", write_len);
    dump_buffer_imm(data_buf, write_len);
    TRACE(0,"UnalignedDMA after write (extra_len=%u):", extra_len);
    dump_buffer_imm(data_buf + write_len, extra_len);
    for (i = 0; i < write_len; i++) {
        data = 0xAA + i + block + page;
        if (data_buf[i] != data) {
            ASSERT(false, "  UnalignedDMA1-1 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    data = 0xFF;
    for (i = write_len; i < page_size; i++) {
        if (data_buf[i] != data) {
            ASSERT(false, "  UnalignedDMA1-2 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    // Unaligned DMA read
    for (i = 0; i < page_size; i++) {
        data_buf[i] = 0xFF;
    }
    uint32_t read_len;
    read_len = write_len - 8;
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, read_len);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    TRACE(0,"UnalignedDMA after unaligned read (read_len=%u):", read_len);
    dump_buffer_imm(data_buf, read_len);
    TRACE(0,"UnalignedDMA after unaligned read (extra_len=%u):", extra_len);
    dump_buffer_imm(data_buf + read_len, extra_len);
    for (i = 0; i < read_len; i++) {
        data = 0xAA + i + block + page;
        if (data_buf[i] != data) {
            ASSERT(false, "  Update2-1 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    data = 0xFF;
    for (i = read_len; i < page_size; i++) {
        if (data_buf[i] != data) {
            ASSERT(false, "  Update2-2 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    TRACE(0,"UnalignedDMA done for %u-%u", block, page);
    while (1);
#endif

    TRACE(0,"Erase blocks");
    total_cnt = 0;
    err_cnt = 0;
    for (block = 0; block < block_cnt; block++) {
        bad = in_bad_block_list(block);
        if (bad) {
            TRACE(0,"  Skip bad block: %u", block);
            continue;
        }
        ret = nandflash_erase_block(flash_id, block);
        if (ret != NANDFLASH_RET_OK) {
            TRACE(0,"  Erase failed for block=%u: %d (add to bad block list)", block, ret);
            append_bad_block_list(block);
            err_cnt++;
        }
        total_cnt++;
    }
    TRACE(0,"Erase done for %u blocks (%u failed)", total_cnt, err_cnt);

    TRACE(0,"Program block pages");
    total_cnt = 0;
    err_cnt = 0;
    for (block = 0; block < block_cnt; block++) {
        bad = in_bad_block_list(block);
        if (bad) {
            TRACE(0,"  Skip bad block: %u", block);
            continue;
        }
        if (blocks_per_die) {
            cur_die = block / blocks_per_die;
        } else {
            cur_die = 0;
        }
        for (page = 0; page < block_size / page_size; page++) {
            for (i = 0; i < page_size; i++) {
                data_buf[i] = i + block + page + cur_die;
            }
            ret = nandflash_write_page(flash_id, block, page, data_buf, page_size);
            if (ret != NANDFLASH_RET_OK) {
                TRACE(0,"  Program failed for block=%u page=%u: %d (add to bad block list)", block, page, ret);
                append_bad_block_list(block);
                err_cnt++;
                break;
            }
        }
        total_cnt++;
    }
    TRACE(0,"Program done for %u blocks (%u failed)", total_cnt, err_cnt);

    total_cnt = 0;
    TRACE(0,"Read block pages");
    err_cnt = 0;
    for (block = 0; block < block_cnt; block++) {
        bad = in_bad_block_list(block);
        if (bad) {
            TRACE(0,"  Skip bad block: %u", block);
            continue;
        }
        if (blocks_per_die) {
            cur_die = block / blocks_per_die;
        } else {
            cur_die = 0;
        }
        for (page = 0; page < block_size / page_size; page++) {
            ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
            if (ret != NANDFLASH_RET_OK) {
                if (ret == NANDFLASH_RET_READ_CORRECTED) {
                    TRACE(0,"  Read corrected: block=%u page=%u", block, page);
                } else {
                    TRACE(0,"  Read failed for block=%u page=%u: %d (add to bad block list)", block, page, ret);
                    append_bad_block_list(block);
                    err_cnt++;
                    break;
                }
            }
            for (i = 0; i < page_size; i++) {
                data = i + block + page + cur_die;
                if (data_buf[i] != data) {
                    //ASSERT(false, "  Read verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
                    TRACE(1, "  Read verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);

                }
            }
        }
        total_cnt++;
    }
    TRACE(0,"Read done for %u blocks (%u failed)", total_cnt, err_cnt);

    TRACE(0,"Copy block page");
    page = 0;
    TRACE(0,"  Copy from %u-%u to %u-%u", src, page, dst, page);
    ret = nandflash_erase_block(flash_id, dst);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Erase failed for block=%u: %d", dst, ret);
    }
    ret = nandflash_read_page(flash_id, dst, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", dst, page, ret);
    }
    data = 0xFF;
    for (i = 0; i < page_size; i++) {
        if (data_buf[i] != data) {
            ASSERT(false, "  Erase verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", dst, page, i, data_buf[i], data);
        }
    }
    ret = nandflash_copy_page(flash_id, dst, page, src, page);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Copy page failed: %d", ret);
    }
    ret = nandflash_read_page(flash_id, dst, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", dst, page, ret);
    }
    if (blocks_per_die) {
        cur_die = src / blocks_per_die;
    } else {
        cur_die = 0;
    }
    for (i = 0; i < page_size; i++) {
        data = i + src + page + cur_die;
        if (data_buf[i] != data) {
            ASSERT(false, "  Copy verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", dst, page, i, data_buf[i], data);
        }
    }
    TRACE(0,"Copy done for %u-%u ==> %u-%u", src, page, dst, page);

    TRACE(0,"Update block page");
    block = dst;
    TRACE(0,"  Update %u-%u", block, page);
    ret = nandflash_erase_block(flash_id, block);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Erase failed for block=%u: %d", block, ret);
    }
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    data = 0xFF;
    for (i = 0; i < page_size; i++) {
        if (data_buf[i] != data) {
            ASSERT(false, "  Erase verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    for (i = 0; i < page_size / 2; i++) {
        data_buf[i] = i + block + page;
    }
    ret = nandflash_update_page(flash_id, block, page, page_size / 2, data_buf, page_size / 2);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Update page failed: %d", ret);
    }
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    data = 0xFF;
    for (i = 0; i < page_size / 2; i++) {
        if (data_buf[i] != data) {
            ASSERT(false, "  Update1-1 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    for (i = page_size / 2; i < page_size / 2; i++) {
        data = i - page_size / 2 + src + page;
        if (data_buf[i] != data) {
            ASSERT(false, "  Update1-2 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    for (i = 0; i < page_size / 2; i++) {
        data_buf[i] = ~(i + block + page);
    }
    ret = nandflash_update_page(flash_id, block, page, 0, data_buf, page_size / 2);
    if (ret != NANDFLASH_RET_OK) {
        ASSERT(false, "  Update page failed: %d", ret);
    }
    ret = nandflash_read_page(flash_id, block, page, 0, data_buf, page_size);
    if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED) {
        ASSERT(false, "  Read failed for block=%u page=%u: %d", block, page, ret);
    }
    for (i = 0; i < page_size / 2; i++) {
        data = ~(i + block + page);
        if (data_buf[i] != data) {
            ASSERT(false, "  Update2-1 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    for (i = page_size / 2; i < page_size / 2; i++) {
        data = i - page_size / 2 + src + page;
        if (data_buf[i] != data) {
            ASSERT(false, "  Update2-2 verify failed for block=%u page=%u offset=%u: 0x%02X (0x%02X expected)", block, page, i, data_buf[i], data);
        }
    }
    TRACE(0,"Update done for %u-%u", block, page);

    TRACE(0,"%s: DONE", __func__);
}

#else

#define NANDFLASH_BAD_BLOCK_MAX     64


/*--------------Test Case Switch-----------------*/
#define NANDFLASH_TEST_BASE_FUCTION
#define NANDFLASH_TEST_PERFORMANCE
#define NANDFLASH_TEST_LIFE


#define NANDFLASH_TEST_BASE_FUCTION_ON     (0x1 << 0)
#define NANDFLASH_TEST_PERFORMANCE_ON      (0x1 << 4)
#define NANDFLASH_TEST_LIFE_ON             (0x1 << 5)
/*----------------------------------------*/

#define BLOCK_BAD_TYPE_OLD_STR     "OLD BAD"
#define BLOCK_BAD_TYPE_ERASE_STR   "ERASE BAD"
#define BLOCK_BAD_TYPE_PROGRAM_STR "PROGRAM BAD"


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

ALIGN4
static uint8_t data_read_buf[PAGE_SIZE_MAX];

static NANDFLASH_TEST_RESULT nandflash_test_result[HAL_FLASH_ID_NUM];

static void  _test_result_init(enum HAL_FLASH_ID_T id)
{
    memset((uint8_t*)&nandflash_test_result[id], 0, sizeof(nandflash_test_result[id]));
}

static void _test_result_print(enum HAL_FLASH_ID_T id)
{
    uint32_t i;
    uint8_t device_id[NANDFLASH_ID_LEN];
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;
    uint32_t page_spare_size = 0;
    uint32_t die_num = 0;

    TRACE(1, "\n--------NANDFLASH %d TEST ENVIRONMENT----------\n", id);

    nandflash_get_id(id, device_id, NANDFLASH_ID_LEN);
    TRACE(1, "FLASH ID:        0x%02x-0x%02x-0x%02x", device_id[0],device_id[1],device_id[2]);
    nandflash_get_size(id, &total_size, &block_size, &page_size, &page_spare_size, &die_num);
    TRACE(1, "TOTAL SIZE:      0x%08x(byte)", total_size);
    TRACE(1, "BLOCK SIZE:      0x%08x(byte)", block_size);
    TRACE(1, "PAGE SIZE:       0x%08x(byte)", page_size);
    TRACE(1, "PAGE_SPARE SIZE: 0x%08x(byte)", page_spare_size);
    TRACE(1, "DIE SIZE:        0x%08x(byte)", die_num);

    TRACE(1, "\n--------NANDFLASH %d TEST RESULT----------\n", id);

        if(nandflash_test_result[id].test_case_bmp & NANDFLASH_TEST_BASE_FUCTION_ON)
        {
            if(nandflash_test_result[id].test_case_result_bmp & NANDFLASH_TEST_BASE_FUCTION_ON)
            {
                TRACE(1, "NANDFLASH TEST BASE FUNCTION PASS");
            }
            else
            {
                TRACE(1, "NANDFLASH TEST BASE FUNCTION NG");
            }
        }

        TRACE(1, "BAD BLODK COUNT:  %d", nandflash_test_result[id].bad_block_count);

        TRACE(1, "ERASE SIZE:       0x%x(KB)  ERASE TIME:   0x%08x(ms)",
              nandflash_test_result[id].performance.erase_size/1024,nandflash_test_result[id].performance.erase_time/1000);
        TRACE(1, "PROGRAM SIZE:     0x%x(KB)  PROGRAM TIME: 0x%08x(ms)",
              nandflash_test_result[id].performance.program_size/1024,nandflash_test_result[id].performance.program_time/1000);

        TRACE(1, "READ SIZE:        0x%x(KB)  READ TIME:    0x%08x(ms)",
              nandflash_test_result[id].performance.read_size/1024,nandflash_test_result[id].performance.read_time/1000);

        TRACE(1, "BLOCK ERASE SPEED:      %08d (KB/s)",
            ((nandflash_test_result[id].performance.erase_size/1024)*1000)/(nandflash_test_result[id].performance.erase_time/1000));
        TRACE(1, "BLOCK PROGRAM SPEED:    %08d (KB/s)",
            ((nandflash_test_result[id].performance.program_size/1024)*1000)/(nandflash_test_result[id].performance.program_time/1000));
        TRACE(1, "BLOCK READ    SPEED:    %08d (KB/s)",
            ((nandflash_test_result[id].performance.read_size/1024)*1000)/(nandflash_test_result[id].performance.read_time/1000));

        TRACE(1, "\n-------------------BAD BLOCK------------------------------\n");
        for(i = 0; i < nandflash_test_result[id].bad_block_count; i++)
        {
            TRACE(1, "BAD BLOCK NUMBER:  %d", nandflash_test_result[id].bad_block_info[i].bad_block_num);
            TRACE(1, "BAD PAGE COUNT :   %d", nandflash_test_result[id].bad_block_info[i].bad_page_count);
            if(nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_OLD)
            {
                TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_OLD_STR);
            }
            else if(nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_ERASE)
            {
                TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_ERASE_STR);
            }
            else if(nandflash_test_result[id].bad_block_info[i].bad_type == BLOCK_BAD_TYPE_PROGRAM)
            {
                TRACE(1, "BAD TYPE:          %s", BLOCK_BAD_TYPE_PROGRAM_STR);
            }
            TRACE(1, "BAD PAGE BMP:0x%08x,0x%08x,0x%08x,0x%08x",
                     nandflash_test_result[id].bad_block_info[i].bad_page_bmp[3],
                     nandflash_test_result[id].bad_block_info[i].bad_page_bmp[2],
                     nandflash_test_result[id].bad_block_info[i].bad_page_bmp[1],
                     nandflash_test_result[id].bad_block_info[i].bad_page_bmp[0]);
        }


    TRACE(1, "\n--------------------------------------------------\n");
}

void nandflash_test_set_test_case_bmp(enum HAL_FLASH_ID_T id, uint32_t case_id)
{
    nandflash_test_result[id].test_case_bmp |= case_id;
}

void nandflash_test_set_test_case_pass(enum HAL_FLASH_ID_T id, uint32_t case_id)
{
    nandflash_test_result[id].test_case_result_bmp |= case_id;
}

static int erase_check(uint32_t block)
{
    uint32_t page;
    uint32_t block_size, page_size, spare_size;
    uint32_t page_cnt, i, offs;
    int ret = 0;
    uint32_t start_time, end_time;
    uint32_t ind = nandflash_test_result[flash_id].bad_block_count;
    bool is_bad = false;

    if(block % 100 == 0)
    {
        TRACE(1,"%s: block: %d", __func__, block);
    }
    nandflash_get_size(flash_id, NULL, &block_size, &page_size, &spare_size, NULL);
    page_cnt = block_size/page_size;

    start_time = hal_fast_sys_timer_get();
    ret = nandflash_erase_block(flash_id, block);
    end_time = hal_fast_sys_timer_get();

    if(ret)
    {
        TRACE(1,"ERASE FAIL:  %u, %d", block, ret);
        TRACE_FLUSH();
    }
    else
    {
        nandflash_test_result[flash_id].performance.erase_time += FAST_TICKS_TO_US(end_time - start_time);
        nandflash_test_result[flash_id].performance.erase_size += block_size;
        for(page = 0; page < page_cnt; page ++)
        {
            offs = 0;
            start_time = hal_fast_sys_timer_get();
            ret = nandflash_read_page(flash_id, block, page, offs, data_buf, page_size);
            end_time = hal_fast_sys_timer_get();

            if(ret)
            {
                TRACE(1," nandflash_read_page FAIL:  %u, %d", block, ret);
                TRACE_FLUSH();
                is_bad = true;
                ret = -1;
                goto __PAGE_FAIL;
            }
            else
            {
                nandflash_test_result[flash_id].performance.read_time += FAST_TICKS_TO_US(end_time - start_time);
                nandflash_test_result[flash_id].performance.read_size += page_size;
            }

            for(i = 0; i < page_size; i++)
            {
                if(data_buf[i] != 0xff)
                {
                    TRACE(1,"%s data error block:%d, page:%d, offs: %d, data[%d](0xff): 0x%x",
                          __func__, block, page, offs, i, data_buf[i]);
                    DUMP8("0x%x,", (uint8_t*)&data_buf[i],16);
                    is_bad = true;
                    ret = -2;
                    goto __PAGE_FAIL;
                }
            }
            offs = 0;
            ret = nandflash_read_oob(flash_id, block, page, offs, data_buf, spare_size);
            if(ret)
            {
                TRACE(1, "%s: nandflash_read_oob FAIL:  %u, %d", __func__, block, ret);
                TRACE_FLUSH();
                is_bad = true;
                ret = -3;
                goto __PAGE_FAIL;
            }
            for(i = 0; i < spare_size; i++)
            {
                if(data_buf[i] != 0xff)
                {
                    TRACE(1,"%s oob error block:%d, page:%d, offse: %d, data[%d](0xff): 0x%x",
                          __func__, block, page, offs, i, data_buf[i]);
                    DUMP8("0x%x,", (uint8_t*)&data_buf[i],16);
                    is_bad = true;
                    ret = -4;
                    goto __PAGE_FAIL;
                }
            }

            continue;
__PAGE_FAIL:
            nandflash_test_result[flash_id].bad_block_info[ind].bad_page_bmp[page/32] |= (0x1 << (page - (page/32)*32));
            nandflash_test_result[flash_id].bad_block_info[ind].bad_page_count ++;
        }
    }

    if(is_bad)
    {
        ret = -1;
        goto FUNC_ERROR;
    }

    if(block % 100 == 0)
    {
        TRACE(1,"%s block: %d DONE", __func__, block);
    }
    return 0;

FUNC_ERROR:
    nandflash_test_result[flash_id].bad_block_info[ind].bad_block_num = block;
    nandflash_test_result[flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_ERASE;
    TRACE(1,"%s block %d FAIL, ret = %d", __func__, block, ret);
    return ret;
}

static int program_check(uint32_t block)
{
    uint32_t page;
    uint32_t block_size, page_size, spare_size;
    uint32_t page_cnt, i, offs;
    int ret = 0;
    uint16_t *p, *p1;
    uint32_t start_time, end_time;
    uint32_t ind = nandflash_test_result[flash_id].bad_block_count;
    bool is_bad = false;

    if(block % 100 == 0)
    {
        TRACE(1,"%s: block: %d", __func__, block);
    }
    nandflash_get_size(flash_id, NULL, &block_size, &page_size, &spare_size, NULL);
    // TRACE(1,"%s: %u, %d, %d", __func__, block_size, page_size, spare_size);
    page_cnt = block_size/page_size;

    p = (uint16_t*)data_buf;
    for(i = 0; i < page_size/sizeof(*p); i++)
    {
        p[i] = block + i;
    }

    for(page = 0; page < page_cnt; page ++)
    {
        start_time = hal_fast_sys_timer_get();
        ret = nandflash_write_page(flash_id, block, page, data_buf, page_size);
        end_time = hal_fast_sys_timer_get();
        if (ret)
        {
            TRACE(1,"%s write page FAIL:  %u, %d", __func__, block, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -1;
            goto __PAGE_FAIL;
        }

        offs = 0;
        ret = nandflash_read_page(flash_id, block, page, offs, data_read_buf, page_size);
        if (ret != NANDFLASH_RET_OK && ret != NANDFLASH_RET_READ_CORRECTED)
        {
            TRACE(1,"%s read page FAIL:  %u, %d, %d", __func__, block, page, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -2;
            goto __PAGE_FAIL;
        }

        p = (uint16_t*)data_buf;
        p1 = (uint16_t*)data_read_buf;
        for(i = 0; i < page_size/sizeof(*p); i++)
        {
            if(p[i] != p1[i])
            {
                TRACE(1,"%s data error block:%d, page:%d, offs: %d, data[%d](0x%x): 0x%x",
                      __func__, block, page, offs, i, p[i], p1[i]);
                DUMP16("0x%x,", (uint8_t*)&data_read_buf[i],16);
                is_bad = true;
                ret = -3;
                goto __PAGE_FAIL;
            }
        }
        nandflash_test_result[flash_id].performance.program_time += FAST_TICKS_TO_US(end_time - start_time);
        nandflash_test_result[flash_id].performance.program_size += page_size;
#ifdef NAND_FLASH_TEST_OOB_PROGRAM
        ASSERT(sizeof(oob_buf) >= spare_size, "%s: oob_buf too small, %d, %d",
                  __func__, sizeof(oob_buf), spare_size);
        for(i = 0; i < spare_size; i++)
        {
            oob_buf[i] = i;
        }
        oob_buf[0] = 0xff;

        ret = nandflash_write_oob(flash_id, block, page, oob_buf, spare_size);
        if (ret)
        {
            TRACE(1, "%s: write oob FAIL:  %u, %d, %d", __func__, block, page, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -4;
            goto __PAGE_FAIL;
        }

        offs = 0;
        ret = nandflash_read_oob(flash_id, block, page, offs, oob_read_buf, spare_size);
        if (ret)
        {
            TRACE(1, "%s: read oob FAIL:  %u, %d, %d", __func__, block, page, ret);
            TRACE_FLUSH();
            is_bad = true;
            ret = -5;
            goto __PAGE_FAIL;
        }

        p = (uint16_t*)oob_buf;
        p1 = (uint16_t*)oob_read_buf;
        for(i = 0; i < spare_size; i++)
        {
            if(p[i] != p1[i])
            {
                TRACE(1,"%s oob error block:%d, page:%d, offse: %d, oob_buf[%d](0x%x): 0x%x",
                      __func__, block, page, offs, i, p[i], p1[i]);
                DUMP8("0x%x,", (uint8_t*)&p1[i],16);
                is_bad = true;
                ret = -6;
                goto __PAGE_FAIL;
            }
        }
#endif
        continue;

__PAGE_FAIL:
        nandflash_test_result[flash_id].bad_block_info[ind].bad_page_bmp[page/32] |= (0x1 << (page - (page/32)*32));
        nandflash_test_result[flash_id].bad_block_info[ind].bad_page_count ++;
    }

    if(is_bad)
    {
        ret = -1;
        goto FUNC_ERROR;
    }

    if(block % 100 == 0)
    {
        TRACE(1,"%s block: %d DONE", __func__, block);
    }
    return 0;

FUNC_ERROR:
    nandflash_test_result[flash_id].bad_block_info[ind].bad_block_num = block;
    nandflash_test_result[flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_PROGRAM;
    TRACE(1,"%s block %d FAIL, ret = %d", __func__, block, ret);
    return ret;
}

void nandflash_test(void)
{
    int ret;
    uint32_t block;
    uint8_t cur_die;
    uint32_t total_size, block_size, page_size, spare_size, die_num;
    uint32_t blocks_per_die;
    uint32_t bad_block_count;
    uint8_t device_id[NANDFLASH_ID_LEN];
    uint32_t ind;

    TRACE(0,"%s: start", __func__);

    _test_result_init(flash_id);

    ret = nandflash_init(flash_id);
    nandflash_get_id(flash_id, device_id, sizeof(device_id));
    if(ret)
    {
        TRACE(1, "Failed to init nandflash: %d", ret);
        goto FUNC_FAIL;
    }
    TRACE(0,"%s: get id: init_ret=%d id=%02X-%02X-%02X",__func__, ret, device_id[0], device_id[1], device_id[2]);

    nandflash_get_size(flash_id, &total_size, &block_size, &page_size, &spare_size, &die_num);
    TRACE(0,"total_size=%u block_size=%u", total_size, block_size);
    TRACE(0,"page_size=%u spare_size=%u die_num=%u", page_size, spare_size, die_num);

    nandflash_show_calib_result(flash_id);

    nandflash_dump_features(flash_id);

    die_num += 1;
    blocks_per_die = total_size / block_size / die_num;

    // base function test
    nandflash_test_set_test_case_bmp(flash_id, NANDFLASH_TEST_BASE_FUCTION_ON);
    for(cur_die = 0; cur_die < die_num; cur_die ++)
    {
        TRACE(0,"%s: DIE(%d) BAD BLOCK CHECK, block cnt: %d", __func__, cur_die, blocks_per_die);
        for(block = 0; block < blocks_per_die; block++)
        {
            ret = nandflash_block_is_bad(flash_id, block);
            if(ret)
            {
                TRACE(1, "%s: DIE(%d) BAD BLOCK:,BLOCK(%d).", __func__, cur_die, block + cur_die*blocks_per_die);
                append_bad_block_list(block);
                ind = nandflash_test_result[flash_id].bad_block_count;
                nandflash_test_result[flash_id].bad_block_info[ind].bad_block_num = block;
                nandflash_test_result[flash_id].bad_block_info[ind].bad_type = BLOCK_BAD_TYPE_OLD;
                nandflash_test_result[flash_id].bad_block_count ++;

            }
        }
        TRACE(1, "%s: DIE(%d) BAD BLOCK CHECK DONE. BAD BLOCK COUNT: %d", __func__, cur_die, get_bad_block_count());
        TRACE(0,"%s: DIE(%d) ERASE BLOCK CHECK, block cnt: %d", __func__, cur_die, blocks_per_die);

        for(block = 0; block < blocks_per_die; block++)
        {
            if(in_bad_block_list(block))
            {
                TRACE(0,"%s: DIE(%d) skip bad block(%d) ", __func__, cur_die, block);
                continue;
            }
            ret = erase_check(block + cur_die*blocks_per_die);
            if(ret)
            {
                TRACE(1, "%s: DIE(%d) erase_check(%d) FAIL, ret = %d", __func__, cur_die, block + cur_die*blocks_per_die, ret);
                append_bad_block_list(block);
                nandflash_test_result[flash_id].bad_block_count ++;
                continue;
            }
            ret = program_check(block + cur_die*blocks_per_die);
            if(ret)
            {
                TRACE(1, "%s: DIE(%d) program_check(%d) FAIL, ret = %d", __func__, cur_die, block + cur_die*blocks_per_die, ret);
                append_bad_block_list(block);
                nandflash_test_result[flash_id].bad_block_count ++;
                continue;
            }
        }
        bad_block_count = get_bad_block_count();
        TRACE(0,"%s: total block: %d, bad block: %d", __func__, blocks_per_die, bad_block_count);
        TRACE(0,"%s: DIE(%d) CHECK DONE", __func__, cur_die);
    }
    nandflash_test_set_test_case_pass(flash_id, NANDFLASH_TEST_BASE_FUCTION_ON);
    _test_result_print(flash_id);
    TRACE(0,"NANDFLASH TEST DONE");
    return;

FUNC_FAIL:
    TRACE(0,"NANDFLASH TEST NG!");
    return;
}

void nandflash_simple_test(void)
{
    int ret;
    static uint32_t block = 0;
    uint8_t device_id[NANDFLASH_ID_LEN];
    static bool is_fst = true;

    TRACE_IMM(0,"%s block = %d", __func__, block);
    if(is_fst)
    {
        is_fst = false;
        ret = nandflash_init(flash_id);
        nandflash_get_id(flash_id, device_id, sizeof(device_id));
        if(ret)
        {
            TRACE(1, "Failed to init nandflash: %d", ret);
            return;
        }
        TRACE(0,"%s: get id: init_ret=%d id=%02X-%02X-%02X",__func__, ret, device_id[0], device_id[1], device_id[2]);
    }

    do
    {
        if(in_bad_block_list(block))
        {
            TRACE(0,"%s: skip bad block(%d) ", __func__, block);
            break;
        }
        ret = erase_check(block);
        if(ret)
        {
            TRACE(1, "%s: erase_check(%d) FAIL, ret = %d", __func__, block, ret);
            break;
        }
        ret = program_check(block);
        if(ret)
        {
            TRACE(1, "%s: program_check(%d) FAIL, ret = %d", __func__, block, ret);
            break;
        }
    }while(0);
    TRACE_IMM(0,"%s block = %d done", __func__, block);
    block ++;
}

#endif
#endif // NANDFLASH_TEST
