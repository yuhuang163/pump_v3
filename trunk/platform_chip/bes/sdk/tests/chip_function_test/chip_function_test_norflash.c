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
#ifdef CHIP_FUNCTION_TEST_NORFLASH
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "norflash_drv.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "norflash_api.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"
#define DEBUG_FLASH

#ifdef DEBUG_FLASH
#define FLASH_DEBUG(...) TRACE(0, ##__VA_ARGS__)
#else
#define FLASH_DEBUG(...) do{}while(1)
#endif

/*---------------Test Case-------------------------*/
#ifdef FLASH_SIMPLE_TEST
//#define FLASH_BLOCK_ERASE_TEST
//#define FLASH_SUSPEND_TEST
#define FLASH_BASE_FUNCTION_TEST
#define FLASH_READ_SPEED
#else
//#define FLASH_CACHE_TEST
#define FLASH_CHIP_ERASE_TEST
#define FLASH_BLOCK_ERASE_TEST
#define FLASH_SUSPEND_TEST
#define FLASH_BASE_FUNCTION_TEST
#define FLASH_NORFLASH_API_TEST
#define FLASH_READ_SPEED
#define FLASH_LIFE_TEST
#endif
/*--------------Test Case Switch-----------------*/
#define FLASH_CACHE_TEST_ON                  (0x1 << 0)
#define FLASH_PROTECTION_TEST_ON             (0x1 << 4)
#define FLASH_SUSPEND_TEST_ON                (0x1 << 5)
#define FLASH_BASE_FUNCTION_TEST_ON          (0x1 << 6)
#define FLASH_NORFLASH_API_TEST_ON           (0x1 << 7)
#define FLASH_READ_SPEED_ON                  (0x1 << 8)
#define FLASH_LIFE_TEST_ON                   (0x1 << 9)
#define FLASH_SECURITY_REG_TEST_ON           (0x1 << 10)
#define FLASH_SET_DUAL_CHIP_TEST_ON          (0x1 << 11)
#define FLASH_RANDOM_WRITE_TEST_ON           (0x1 << 12)
/*----------------------------------------*/
// #define CHECK_SET_PROTECTION_TIME
/*----------------------------------------*/

#define PMU_FLASH_WRITE_ENABLE()  pmu_flash_write_config()
#define PMU_FLASH_READ_ENABLE()   pmu_flash_read_config()
#define ALIGN_N(a, n) (((a + n-1)/n)*n)

typedef struct {
    bool protection_pass;
    bool suspend_pass;
    bool norflash_api_pass;
    bool security_register_pass;
    bool base_func_pass;
    bool erase_pass;
    bool write_pass;
    bool read_pass;
    bool set_dual_chip_pass;
    bool random_write_pass;
    uint32_t read_speed_cache;
    uint32_t read_speed_no_cache;
    uint32_t chip_erase_time;
    uint32_t erase_speed_block;
    uint32_t erase_speed_sector;
    uint32_t erase_speed_page;
    uint32_t write_speed;
}TEST_RESULT;

typedef struct {
    TEST_RESULT test_result[HAL_FLASH_ID_NUM];
    uint32_t    test_case_switch[HAL_FLASH_ID_NUM];
}TEST_LIST;

uint8_t buff_r[NORFLASH_API_BLOCK_MAX_SIZE];
uint8_t buff_w[NORFLASH_API_SECTOR_MAX_SIZE];
#ifdef FLASH_CHIP_ERASE_TEST
uint8_t buff_b[NORFLASH_API_BLOCK_MAX_SIZE*4];
#else
uint8_t buff_b[NORFLASH_API_BLOCK_MAX_SIZE];
#endif

extern uint32_t __flash_end[];
extern uint32_t __factory_start[];
extern uint32_t __factory_end[];

uint32_t test_erase_time = 0;
uint32_t test_write_time = 0;
uint32_t test_read_time = 0;
bool test_print_details = false;

static TEST_RESULT flash_test_result[HAL_FLASH_ID_NUM];
static uint32_t flash_test_case_switch[HAL_FLASH_ID_NUM];

static SRAM_TEXT_LOC uint32_t _get_base_addr(enum HAL_FLASH_ID_T id, bool is_cache)
{
    uint32_t base_addr = 0;
    if(id == HAL_FLASH_ID_0)
    {
        if(is_cache)
        {
            base_addr = FLASH_BASE;
        }
        else
        {
            base_addr = FLASH_NC_BASE;
        }
    }
 #ifdef FLASH1_CTRL_BASE
    else if(id == HAL_FLASH_ID_1)
    {
        if(is_cache)
        {
            base_addr = FLASH1_BASE;
        }
        else
        {
            base_addr = FLASH1_NC_BASE;
        }
    }
 #endif
    else
    {
        ASSERT(0, "%s: ERROR ID: %d", __func__, id);
    }
    return base_addr;
}

static SRAM_TEXT_LOC void _print_flash_test_result(enum HAL_FLASH_ID_T id,uint32_t test_case_switch, TEST_RESULT *test_result)
{
    TRACE(1, "\n--------FLASH %d TEST RESULT----------\n", id);

    if(test_case_switch & FLASH_CACHE_TEST_ON)
    {
        TRACE(1, "FLASH CACHE TEST");
    }
    else
    {
        TRACE(1, "FLASH NOCACHE TEST");
    }

    if(test_case_switch & FLASH_PROTECTION_TEST_ON)
    {
        if(test_result->protection_pass)
        {
            TRACE(1, "FLASH PROTECTION TEST:        PASS");
        }
        else
        {
            TRACE(1, "FLASH PROTECTION TEST:        NG!");
        }
    }

    if(test_case_switch & FLASH_SUSPEND_TEST_ON)
    {
        if(test_result->suspend_pass)
        {
            TRACE(1, "FLASH SUSPEND TEST:           PASS");
        }
        else
        {
            TRACE(1, "FLASH SUSPEND TEST:           NG!");
        }
    }

    if(test_case_switch & FLASH_NORFLASH_API_TEST_ON)
    {
        if(test_result->norflash_api_pass)
        {
            TRACE(1, "NOFLASH API TEST:             PASS");
        }
        else
        {
            TRACE(1, "NOFLASH API TEST:             NG!");
        }
    }

    if(test_case_switch & FLASH_SECURITY_REG_TEST_ON)
    {
        if(test_result->security_register_pass)
        {
            TRACE(1, "FLASH SECURITY REGISTER TEST: PASS");
        }
        else
        {
            TRACE(1, "FLASH SECURITY REGISTER TEST:  NG!");
        }
    }

    if(id == HAL_FLASH_ID_0 && (test_case_switch & FLASH_SET_DUAL_CHIP_TEST_ON))
    {
        if(test_result->set_dual_chip_pass)
        {
            TRACE(1, "FLASH SET DUAL CHIP TEST:     PASS");
        }
        else
        {
            TRACE(1, "FLASH SET DUAL CHIP TEST:      NG!");
        }
    }
    if(id == HAL_FLASH_ID_0 && (test_case_switch & FLASH_RANDOM_WRITE_TEST_ON))
    {
        if(test_result->random_write_pass)
        {
            TRACE(1, "FLASH RANDOM WRITE TEST:      PASS");
        }
        else
        {
            TRACE(1, "FLASH RANDOM WRITE TEST:      NG!");
        }
    }


    if(test_case_switch & FLASH_BASE_FUNCTION_TEST_ON)
    {
        if(test_result->base_func_pass)
        {
            TRACE(1, "FLASH ERASE  TEST:            PASS");
            TRACE(1, "FLASH PROGRAM TEST:           PASS");
            TRACE(1, "FLASH READ   TEST:            PASS\n\n");
            TRACE(1, "\n------------------------------\n");

            TRACE(1, "FLASH CHIP  ERASE TIME:     %08d(ms)", FAST_TICKS_TO_MS(test_result->chip_erase_time));

            TRACE(1, "FLASH BLOCK  ERASE SPEED:   %08d(KB/s)", test_result->erase_speed_block);
            TRACE(1, "FLASH SECTOR ERASE SPEED:   %08d(KB/s)", test_result->erase_speed_sector);
#ifdef PUYA_FLASH_ERASE_PAGE_ENABLE
            TRACE(1, "FLASH PAGE   ERASE SPEED:   %08d(KB/s)", test_result->erase_speed_page);
#endif
            TRACE(1, "FLASH PROGRAM SPEED:        %08d(KB/s)", test_result->write_speed);
        }
        else
        {
            TRACE(1, "FLASH ERASE   TEST:     NG!");
            TRACE(1, "FLASH PROGRAM TEST:     NG!");
            TRACE(1, "FLASH READ    TEST:     NG!\n\n");
        }
    }

    if(test_case_switch & FLASH_READ_SPEED_ON)
    {
        TRACE(1, "\n------------------------------\n");
        TRACE(1, "FLASH CACHE READ SPEED:     %08d(KB/s)", test_result->read_speed_cache);
        TRACE(1, "FLASH NO CACHE READ SPEED:  %08d(KB/s)", test_result->read_speed_no_cache);
    }
    TRACE(1, "\n------------------------------\n");
}

static SRAM_TEXT_LOC uint8_t _memcmp_ext(uint8_t * buff1, uint8_t * buff2, uint32_t len, uint32_t *diff_offs)
{
    uint32_t i;
    for(i = 0; i < len; i++)
    {
        if(buff1[i] != buff2[i])
        {
            *diff_offs = i;
            return 1;
        }
    }

    return 0;
}

static SRAM_TEXT_LOC uint8_t _memcmp_pattern(uint8_t * buff, uint8_t pattern, uint32_t len, uint32_t *diff_offs)
{
    uint32_t i;

    if(pattern == 0x55 || pattern == 0xAA || pattern == 0xFF || pattern == 0x00)
    {
        for(i = 0; i < len; i++)
        {
            if(buff[i] != pattern)
            {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }

    if(pattern == 0x01)
    {
        for(i = 0; i < len; i++)
        {
            if(buff[i] != (i & 0xff))
            {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }

    if(pattern == 0x10)
    {
        for(i = len; i > 0; i--)
        {
            if(buff[i - 1] != ((i - 1) & 0xff))
            {
                *diff_offs = i;
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static SRAM_TEXT_LOC int32_t _flash_ewr(enum HAL_FLASH_ID_T id,
                                                  uint32_t addr,
                                                  uint32_t erase_size,
                                                  uint8_t is_restore,
                                                  uint32_t expect_start_addr,
                                                  uint32_t expect_end_addr,
                                                  uint8_t is_bp_map_check)
{
    uint32_t start_addr;
    uint32_t i,j;
    uint8_t value = 0;
    int32_t ret = 0;
    uint8_t* pstart;
    enum HAL_NORFLASH_RET_T result = HAL_NORFLASH_OK;
    uint32_t lock;
    uint32_t flash_total_size = 0;
    uint32_t flash_block_size = 0;
    uint32_t flash_sector_size = 0;
    uint32_t len;
    uint32_t offs;
    uint8_t *p;
    uint8_t cmp_ret;
    uint32_t diff_offs;
    uint32_t start_time;

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, NULL);

    // Set buff_w.

    for(j = 0; j < sizeof(buff_w); j++)
    {
        buff_w[j] = value ++;
    }

    lock = int_lock_global();
    start_addr = addr;
    pstart = (uint8_t*)start_addr;
    if(is_restore)
    {
        start_time = hal_fast_sys_timer_get();
#ifdef NO_FLASH1_ADDR_ACCESS
        if(id > HAL_FLASH_ID_0)
        {
            hal_norflash_read(id, (uint8_t*)pstart, buff_b, erase_size);
        }
        else
#endif
        {
            memcpy(buff_b, pstart, erase_size);
        }
        test_read_time += (hal_fast_sys_timer_get() - start_time);
    }

    result = hal_norflash_erase(id, start_addr, erase_size);
    if(result != HAL_NORFLASH_OK)
    {
        TRACE(1,"%s: erase fail! start_addr = 0x%x, erase_size = 0x%x, ret = %d.",
            __func__,start_addr, erase_size, ret);
        ret = -1;
        goto _func_fail;
    }
#ifdef FLASH_CACHE_TEST
    hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, start_addr, erase_size);
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, start_addr, erase_size);
#endif
    len = erase_size < flash_sector_size ? erase_size : flash_sector_size;

    for(i = 0; i < erase_size/len; i++)
    {
        offs = i*len;
#ifdef NO_FLASH1_ADDR_ACCESS
        if(id > HAL_FLASH_ID_0)
        {
            hal_norflash_read(id, (uint8_t*)start_addr + offs, buff_r, len);
            p = buff_r;
        }
        else
#endif
        {
            p = (uint8_t*)start_addr + offs;
        }

        cmp_ret = _memcmp_pattern(p, 0xFF, len, &diff_offs);
        if(cmp_ret)
        {
            if(
#ifdef FLASH_PROTECTION_TEST
                !IN_EXPEC_PROTECTION_AREA(start_addr, len, expect_start_addr,expect_end_addr) ||
#endif
                is_bp_map_check == 0)
            {
                TRACE(1,"%s: expect_start_addr = 0x%x, expect_end_addr = 0x%x",
                        __func__,expect_start_addr, expect_end_addr);
                TRACE(1,"%s: erase verify error! start_addr = 0x%x, leftover = %d, value = 0x%x ",
                        __func__,start_addr + offs + diff_offs, erase_size - (offs + diff_offs), p[diff_offs]);
                DUMP8("0x%x", (uint8_t*)start_addr + j, 32);
                DUMP8("0x%x", (uint8_t*)((start_addr & ~FLASH_BASE) | FLASH_NC_BASE) + j, 32);
            }
            ret = -3;
            goto _func_fail;
        }
    }

    for(i = 0; i < erase_size / len; i++)
    {
        offs = i*len;
        result = hal_norflash_write(id, start_addr + offs, buff_w, len);
        if(result != HAL_NORFLASH_OK)
        {
            ret = -5;
            goto _func_fail;
        }
#ifdef FLASH_CACHE_TEST
        hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, start_addr + offs, len);
        hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, start_addr + offs, len);
#endif

#ifdef NO_FLASH1_ADDR_ACCESS
        if(id > HAL_FLASH_ID_0)
        {
            hal_norflash_read(id, (uint8_t*)start_addr + offs, buff_r, len);
            p = buff_r;
        }
        else
#endif
        {
            p = (uint8_t*)start_addr + offs;
        }
        cmp_ret = _memcmp_ext(p, buff_w, len, &diff_offs);
        if(cmp_ret)
        {
            if(
#ifdef FLASH_PROTECTION_TEST
                !IN_EXPEC_PROTECTION_AREA(start_addr, len, expect_start_addr,expect_end_addr) ||
#endif
                is_bp_map_check == 0)
            {
                TRACE(1,"%s: expect_start_addr = 0x%x, expect_end_addr = 0x%x",
                       __func__,expect_start_addr, expect_end_addr);
                TRACE(1,"%s: write verify error! @0x%x 0x%02x != 0x%02x",
                    __func__,start_addr + offs + j, p[diff_offs], buff_w[diff_offs]);
                DUMP8("0x%x", p + j, 32);
                DUMP8("0x%x", (uint8_t*)buff_w + j, 32);
            }
            ret = -6;
            goto _func_fail;
        }
    }

    if(is_restore)
    {
        start_time = hal_fast_sys_timer_get();
        result = hal_norflash_erase(id,start_addr,erase_size);
        test_erase_time += (hal_fast_sys_timer_get() - start_time);
        if(result != HAL_NORFLASH_OK)
        {
            ret = -7;
            goto _func_fail;
        }

        start_time = hal_fast_sys_timer_get();
        result = hal_norflash_write(id,start_addr,buff_b, erase_size);
        test_write_time += (hal_fast_sys_timer_get() - start_time);
        if(result != HAL_NORFLASH_OK)
        {
            ret = -8;
            goto _func_fail;
        }

        // restore checking
#ifdef FLASH_CACHE_TEST
        hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, start_addr, erase_size);
        hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, start_addr, erase_size);
#endif
        for(i = 0; i < erase_size / len; i++)
        {
            offs = i*len;
#ifdef NO_FLASH1_ADDR_ACCESS
            if(id > HAL_FLASH_ID_0)
            {
                hal_norflash_read(id, (uint8_t*)start_addr + offs, buff_r, len);
                p = buff_r;
            }
            else
#endif
            {
                p = (uint8_t*)start_addr + offs;
            }

            cmp_ret = _memcmp_ext(p, buff_b + offs, len, &diff_offs);
            if(cmp_ret)
            {

                TRACE(1,"%s: restore write verify error! @0x%x, value = 0x%x(0x%x)",
                   __func__,start_addr + offs + j, buff_r[diff_offs], buff_b[offs + diff_offs]);

                ret = -10;
                goto _func_fail;
            }
        }
    }

    int_unlock_global(lock);
    if(is_bp_map_check)
    {
        TRACE(1,"addr(0x%x) NOT PROTECTION.",
               start_addr);
    }
    else if(test_print_details) {
        TRACE(1,"%s: e/w addr(0x%x),erase_size(0x%x) ok.",
              __func__,start_addr,erase_size);
    }
    TRACE_FLUSH();
    return 0;

_func_fail:
    int_unlock_global(lock);
    if(is_bp_map_check)
    {
        TRACE(1,"addr(0x%x) PROTECTION(%d).",
                 addr, ret);
    }
    else
    {
        TRACE(1,"%s: e/w (0x%x),(0x%x) fail. is_restore(%d),ret(%d).",
            __func__,addr,erase_size,is_restore,ret);
    }
    TRACE_FLUSH();
    return ret;
}

#ifdef FLASH_PROTECTION_TEST

#ifdef FLASH_PROTECTION
#define FLASH_PROTECTION_MAP_TEST
#define FLASH_PROTECTION_CHECK

#define FLASH_SINGLE_CHIP_PROTECTION_MIN_SIZE  0x10000
#define FLASH_SINGLE_CHIP_PROTECTION_MAX_SIZE  0x2000000

#define FLASH_DUAL_CHIP_PROTECTION_MIN_SIZE  0x20000
#define FLASH_DAUL_CHIP_PROTECTION_MAX_SIZE  0x2000000


typedef struct
{
    uint32_t bp;
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t real_start_addr;
    uint32_t real_end_addr;
    uint32_t is_abnormal;
}FLASH_PROTECTION_TEST_INFO;

FLASH_PROTECTION_TEST_INFO protection_test_info[FLASH_BP_MAP_LEN_MAX];
#define IN_EXPEC_PROTECTION_AREA(addr, len, start, end) (((addr) & HAL_NORFLASH_ADDR_MASK) >= start && ((addr) & HAL_NORFLASH_ADDR_MASK) + len <= ((end) & HAL_NORFLASH_ADDR_MASK) + 1)

POSSIBLY_UNUSED
static SRAM_TEXT_LOC void _flash_protection_check(enum HAL_FLASH_ID_T id)
{
    FLASH_BP_MAP_T *bp_map = NULL;
    uint32_t       bp_count = 0;
    uint32_t i,j;
    int ret;
    uint32_t start_addr;
    uint32_t addr;
    uint32_t protect_start;
    uint32_t protect_end;
    enum HAL_NORFLASH_RET_T r;
    bool is_started;
    bool is_ended;
    bool is_abnormal;
    uint32_t pass_count = 0;
    uint32_t start_time = 0;
    uint32_t end_time = 0;
    uint32_t flash_total_size = 0;
    uint32_t flash_block_size = 0;
    uint32_t flash_sector_size = 0;
    uint32_t flash_page_size = 0;

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, &flash_page_size);

#ifdef FLASH_CACHE_TEST
    start_addr = _get_base_addr(id, true);
#else
    start_addr = _get_base_addr(id, false);
#endif
    norflash_api_get_pb_map(id, &bp_map, &bp_count);
    if(bp_count == 0)
    {
        goto _func_end;
    }

    for(i = 0; i < bp_count; i++)
    {
        is_started = false;
        is_ended = false;
        is_abnormal = false;
        protect_start = 0;
        protect_end = 0;

        TRACE(1, "%s: Check PB:%04xh(0x%02x)", __func__, bp_map[i].bp, (bp_map[i].bp & 0xff)>>2);
        start_time = hal_fast_sys_timer_get();
        r = hal_norflash_set_protection(id, bp_map[i].bp);
        end_time = hal_fast_sys_timer_get();
        ASSERT(r == HAL_NORFLASH_OK,
            "%s: set protection fail! r = %d",
            __func__, r);
#ifdef CHECK_SET_PROTECTION_TIME
        TRACE(1, "%s: Set protection time: %08d(us)", __func__, FAST_TICKS_TO_US(end_time-start_time));
        continue;
#else
        if(0)
        {
            TRACE(1, "%s: Set protection time: %08d(us)", __func__, FAST_TICKS_TO_US(end_time-start_time));
        }
#endif
        for(j = 0; j < flash_total_size / flash_sector_size; j++)
        {
            addr= start_addr + j*flash_sector_size;
            ret = _flash_ewr(id, addr, flash_sector_size, 1, bp_map[i].start_addr, bp_map[i].end_addr, 1);
            if(ret)
            {
                if(is_started == false)
                {
                    protect_start =  addr;
                    protect_end =  flash_total_size;
                    is_started = true;
                }

                if(is_ended)
                {
                    TRACE(1, "%s: protection is ended!!! bp:0x%x, start:0x%x, end:0x%x,addr:0x%x",
                          __func__,
                          protection_test_info[i].bp,
                          protect_start,
                          protect_end,
                          addr
                         );
                    is_abnormal = true;
                    break;
                }
            }
            else
            {
                if(is_started == true && is_ended == false)
                {
                    is_ended = true;
                    protect_end = addr;
                }
            }
        }

        protection_test_info[i].bp = bp_map[i].bp;
        protection_test_info[i].start_addr = bp_map[i].start_addr;
        protection_test_info[i].end_addr = bp_map[i].end_addr;
        protection_test_info[i].real_start_addr = protect_start & HAL_NORFLASH_ADDR_MASK;
        if(protect_end > 0)
        {
            if(((protect_end - 1) & HAL_NORFLASH_ADDR_MASK) > 0)
            {
                protection_test_info[i].real_end_addr = ((protect_end-1) & HAL_NORFLASH_ADDR_MASK);
            }
            else
            {
                protection_test_info[i].real_end_addr = 0;
            }
        }
        else
        {
            protection_test_info[i].real_end_addr = 0;
        }
        //protection_test_info[i].real_end_addr = (protect_end & HAL_NORFLASH_ADDR_MASK) > 0 ? (protect_end & HAL_NORFLASH_ADDR_MASK) - 1: 0;
        protection_test_info[i].is_abnormal = is_abnormal;
        if(protection_test_info[i].start_addr == protection_test_info[i].real_start_addr &&
           protection_test_info[i].end_addr == protection_test_info[i].real_end_addr &&
           !protection_test_info[i].is_abnormal)
        {
            pass_count ++;
            TRACE(1, "BP(0x%04x): (0x%08x - 0x%08x) CHECK: (0x%08x - 0x%08x) OK",
                protection_test_info[i].bp,
                protection_test_info[i].start_addr,
                protection_test_info[i].end_addr,
                protection_test_info[i].real_start_addr,
                protection_test_info[i].real_end_addr);
        }
        else
        {
            TRACE(1, "BP(0x%04x): (0x%08x - 0x%08x) CHECK: (0x%08x - 0x%08x) FALIL",
                protection_test_info[i].bp,
                protection_test_info[i].start_addr,
                protection_test_info[i].end_addr,
                protection_test_info[i].real_start_addr,
                protection_test_info[i].real_end_addr);
            // goto _func_print;
        }
        TRACE(1, "%s: Check PB:%04xh(0x%02x) Done", __func__, bp_map[i].bp, (bp_map[i].bp & 0xff)>>2);
    }

    // SET FLASH_PROTECTION TO BP 0
    // BP 0 is none protection mode
    r = hal_norflash_set_protection(id, bp_map[0].bp);
            ASSERT(r == HAL_NORFLASH_OK,
                "%s: set protection fail! r = %d",
                __func__, r);
    goto _func_print;
_func_print:
    for(i = 0; i < bp_count; i++)
    {
        if(protection_test_info[i].start_addr == protection_test_info[i].real_start_addr &&
           protection_test_info[i].end_addr == protection_test_info[i].real_end_addr &&
           !protection_test_info[i].is_abnormal)
        {
            TRACE(1, "BP(0x%04x): (0x%08x - 0x%08x) CHECK: (0x%08x - 0x%08x) OK",
                protection_test_info[i].bp,
                protection_test_info[i].start_addr,
                protection_test_info[i].end_addr,
                protection_test_info[i].real_start_addr,
                protection_test_info[i].real_end_addr);
        }
        else
        {
            TRACE(1, "BP(0x%04x): (0x%08x - 0x%08x) CHECK: (0x%08x - 0x%08x) FALIL",
                protection_test_info[i].bp,
                protection_test_info[i].start_addr,
                protection_test_info[i].end_addr,
                protection_test_info[i].real_start_addr,
                protection_test_info[i].real_end_addr);
        }
    }
_func_end:
    if(bp_count > 0 && pass_count == bp_count)
    {
        flash_test_result[id].protection_pass = true;
    }
    else
    {
        flash_test_result[id].protection_pass = false;
    }
}

POSSIBLY_UNUSED
static SRAM_TEXT_LOC void _flash_protection_map_dump(enum HAL_FLASH_ID_T id)
{
    uint32_t i;
    FLASH_BP_MAP_T *pb_map = NULL;
    uint32_t       pb_count = 0;
    TRACE(1, "PB   | Adddress  Density | Portion");
    norflash_api_get_pb_map(id, &pb_map, &pb_count);
    for(i = 0; i < pb_count; i++)
    {
        osDelay(4);
        TRACE(1, "[%02d] %04xh(0x%02x): |  %08xh - %08xh  |  %08dKB  | ",
            i,
            pb_map[i].bp,
            (pb_map[i].bp & 0xff) >> 2,
            pb_map[i].start_addr,
            pb_map[i].end_addr,
            (pb_map[i].end_addr + 1 - pb_map[i].start_addr)/1024);
    }
    TRACE(1, "PB_MAP DUMP END");
}

POSSIBLY_UNUSED
static SRAM_TEXT_LOC void _flash_protection_map_test(enum HAL_FLASH_ID_T id)
{
    uint32_t total_size;
    int chip_dual_en;
    uint32_t min_size, max_size;

    TRACE(1, "%s", __func__);
    osDelay(10);
    hal_norflash_get_dual_chip_mode(id, &chip_dual_en, NULL);
    if(chip_dual_en)
    {
        min_size = FLASH_DUAL_CHIP_PROTECTION_MIN_SIZE;
        max_size = FLASH_DAUL_CHIP_PROTECTION_MAX_SIZE;
    }
    else
    {
        min_size = FLASH_SINGLE_CHIP_PROTECTION_MIN_SIZE;
        max_size = FLASH_SINGLE_CHIP_PROTECTION_MAX_SIZE;
    }
    total_size = min_size;
    while(1)
    {
        osDelay(4);
        if(total_size > max_size)
        {
            break;
        }
        TRACE(1, "%s: id = %d,  chip_dual_en = %d, total_size = 0x%x, ", __func__, id, chip_dual_en, total_size);
        norflash_api_protection_bp_init(id, total_size);
        _flash_protection_map_dump(id);
        total_size = total_size << 1;
    }
    TRACE(1, "%s done", __func__);
    TRACE_FLUSH();
}

static SRAM_TEXT_LOC void _flash_protection_test(enum HAL_FLASH_ID_T id)
{
#ifdef FLASH_PROTECTION_MAP_TEST
    _flash_protection_map_test(id);
#endif
#ifdef FLASH_PROTECTION_CHECK
    uint32_t total_size = 0;
    total_size = hal_norflash_get_flash_total_size(id);
    TRACE(1, "%s: id = %d, total_size = 0x%x", __func__, id, total_size);
    norflash_api_protection_bp_init(id, total_size);
    _flash_protection_map_dump(id);
    _flash_protection_check(id);
#endif
}
#else
static SRAM_TEXT_LOC void _flash_protection_test(enum HAL_FLASH_ID_T id)
{
    TRACE(1,"FLASH_PROTECTION Undifined!");
}
#endif // FLASH_PROTECTION
#endif // FLASH_PROTECTION_TEST

#ifdef FLASH_UNIQUE_ID
uint8_t flash_unique_id[HAL_NORFLASH_UNIQUE_ID_LEN] = {0};
#ifdef FLASH_DUAL_CHIP
uint8_t flash1_unique_id[HAL_NORFLASH_UNIQUE_ID_LEN] = {0};
#endif

SRAM_TEXT_LOC int32_t _read_uinque_id(enum HAL_FLASH_ID_T id)
{
#ifdef FLASH_DUAL_CHIP
    if(FLASH_IS_DUAL_CHIP(id))
    {
        hal_norflash_dual_chip_get_unique_id(id, flash_unique_id, flash1_unique_id, sizeof(flash_unique_id));
    }
    else
#endif
    {
        hal_norflash_get_unique_id(id, flash_unique_id, sizeof(flash_unique_id));
    }


   /*
   if (flash_unique_id[15] == 0x78) {
       MT_TRACE(0, "PUYA: KGD good ID %x", flash_unique_id[15]);
       return 0;
   } else {
       MT_TRACE(0, "PUYA: KGD bad ID %x", flash_unique_id[15]);
       return -1;
   }
   */
   return 0;
}
#endif

#ifdef FLASH_SUSPEND_TEST

#define FLASH_SUSPEND_TEST_LOOP_COUNT  20
static uint32_t timer_cnt, total_timer_cnt;
static const uint32_t timer_interval_ms = 2;

static void norflash_api_test_timer_handler(void const *param);
osTimerDef(NORFALSH_API_TEST_TIMER, norflash_api_test_timer_handler);
osTimerDef(NORFALSH_API_TEST_TIMER1, norflash_api_test_timer_handler);

static SRAM_TEXT_LOC void norflash_api_test_timer_handler(void const *param)
{
    timer_cnt++;
    total_timer_cnt++;
    if ((total_timer_cnt % 1000) == 0) {
        TRACE(1, "total_timer_cnt=%u", total_timer_cnt);
    }
}

int SRAM_TEXT_LOC _flash_suspend_test(enum HAL_FLASH_ID_T id)
{
    uint32_t cache_base_addr, no_cache_base_addr;
    uint32_t addr;
    uint32_t len = sizeof(buff_r);
    uint32_t max_suspend_cnt = 100000;
    enum HAL_NORFLASH_RET_T nret = HAL_NORFLASH_OK;
    uint32_t lock;
    uint32_t i;
    uint32_t suspend_cnt;
    uint8_t loop = 0;
    int ret = 0;
    osTimerId timer_id;

    cache_base_addr = _get_base_addr(id, true);
    no_cache_base_addr = _get_base_addr(id, false);
    addr = no_cache_base_addr + 4 * norflash_api_get_block_size(id);
    timer_id = osTimerCreate(osTimer(NORFALSH_API_TEST_TIMER), osTimerOnce, NULL);
    osTimerStart(timer_id, timer_interval_ms);
    TR_INFO(0, "Start timer %u ms", timer_interval_ms);
#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif
    while (loop < FLASH_SUSPEND_TEST_LOOP_COUNT) {
        suspend_cnt = 0;
_erase:
        lock = int_lock();
        timer_cnt = 0;
        if (nret == HAL_NORFLASH_SUSPENDED) {
            nret = hal_norflash_erase_resume(id, true);
        } else {
            nret = hal_norflash_erase_suspend(id, addr, len, true);
        }
        int_unlock(lock);
        if (nret == HAL_NORFLASH_SUSPENDED) {
            suspend_cnt++;
            if (suspend_cnt == 1 || (suspend_cnt % 1000) == 0) {
                TR_INFO(0, "[%u] erase suspended: timer_cnt=%u suspend_cnt=%u", loop, timer_cnt, suspend_cnt);
            }
            if(suspend_cnt > max_suspend_cnt) {
               TR_INFO(1, "[%u] erase timeout: suspend_cnt=%u", loop, suspend_cnt);
               ret = -1;
               goto func_fail;
            }
            goto _erase;
        }
        if(nret != HAL_NORFLASH_OK)
        {
            TR_INFO(1, "Failed to erase: %d, addr = 0x%x,len = 0x%x", nret, addr, len);
            ret = -2;
            goto func_fail;
        }
        for (i = 0; i < sizeof(buff_r); i++) {
            uint8_t d;

            d = *(volatile uint8_t *)(addr + i);
            if(d != 0xff)
            {
                TR_INFO(1, "[0x%x] verifed failed @ 0x%x: flash=0x%02X",
                        loop, i, d);
                DUMP8("0x%x", (uint8_t*)addr + i, 32);
                DUMP8("0x%x", (uint8_t*)((addr & ~cache_base_addr) | no_cache_base_addr) + i, 32);
                ret = -3;
                goto func_fail;
            }
        }
        TR_INFO(0, "[%u] erase ok: timer_cnt=%u suspend_cnt=%u", loop, timer_cnt, suspend_cnt);
        for (i = 0; i < sizeof(buff_r); i++) {
            buff_r[i] = loop;
        }
        suspend_cnt = 0;
_write:
        lock = int_lock();
        timer_cnt = 0;
        if (nret == HAL_NORFLASH_SUSPENDED) {
            nret = hal_norflash_write_resume(id, true);
        } else {
            nret = hal_norflash_write_suspend(id, addr, buff_r, len, true);
        }
        int_unlock(lock);
        if (nret == HAL_NORFLASH_SUSPENDED) {
            suspend_cnt++;
            if (suspend_cnt == 1 || (suspend_cnt % 1000) == 0) {
                TR_INFO(0, "[%u] write suspended: timer_cnt=%u suspend_cnt=%u", loop, timer_cnt, suspend_cnt);
            }
            if(suspend_cnt > max_suspend_cnt) {
               TR_INFO(1, "[%u] write timeout: suspend_cnt=%u", loop, suspend_cnt);
               ret = -4;
               goto func_fail;
            }
            goto _write;
        }

        if(nret != HAL_NORFLASH_OK)
        {
            TR_INFO(1, "Failed to write: %d", nret);
            ret = -5;
            goto func_fail;
        }

        TR_INFO(0, "[%u] write ok: timer_cnt=%u suspend_cnt=%u", loop, timer_cnt, suspend_cnt);
        for (i = 0; i < sizeof(buff_r); i++) {
            uint8_t d;

            d = *(volatile uint8_t *)(addr + i);
            if(d != buff_r[i])
            {
                TR_INFO(1, "[%u] verifed failed @ %u: flash=0x%02X buf=0x%02X",
                        loop, i, d, buff_r[i]);
                ret = -6;
                goto func_fail;
            }
        }
        TR_INFO(0, "[%u] verify ok: timer_cnt=%u", loop, timer_cnt);
        loop++;
    }

    osTimerStop(timer_id);
    flash_test_result[id].suspend_pass = true;
    TR_INFO(0, "flash suspend test done");

#ifdef FLASH_PROTECTION
    hal_norflash_enable_protection(id);
#endif
    return ret;
func_fail:
    flash_test_result[id].suspend_pass = false;
    TR_INFO(0, "flash suspend test fail!!! ret = %d", ret);
#ifdef FLASH_PROTECTION
    hal_norflash_enable_protection(id);
#endif
    return ret;
}
#endif

#if defined(FLASH_LIFE_TEST) || defined(FLASH_BASE_FUNCTION_TEST)

static SRAM_TEXT_LOC void _set_print_details(bool print_details)
{
    test_print_details =  print_details;
}

POSSIBLY_UNUSED
static SRAM_TEXT_LOC int32_t _flash_chip_erase(enum HAL_FLASH_ID_T id, uint8_t is_restore)
{
    uint32_t start_time;
    uint32_t restore_size;
    enum HAL_NORFLASH_RET_T result;
    int32_t ret = 0;
    uint32_t i;
    uint32_t addr;
    uint8_t *p;
    uint32_t lock;
#ifdef FLASH_CACHE_TEST
    uint32_t satrt_addr = _get_base_addr(id, true);
#else
    uint32_t satrt_addr = _get_base_addr(id, false);
#endif
    osDelay(4);
    restore_size = (((uint32_t)__flash_end) & HAL_NORFLASH_ADDR_MASK) - (satrt_addr & HAL_NORFLASH_ADDR_MASK);
    restore_size = ((restore_size + NORFLASH_API_BLOCK_MAX_SIZE - 1) / NORFLASH_API_BLOCK_MAX_SIZE) * NORFLASH_API_BLOCK_MAX_SIZE;
    TRACE(1, "%s: satrt_addr = 0x%x, restore_size = 0x%x", __func__, satrt_addr, restore_size);
    lock = int_lock_global();
    if(is_restore)
    {
        if(restore_size > sizeof(buff_b))
        {
            ret = -1;
            goto _func_end;
        }
    }
#ifdef NO_FLASH1_ADDR_ACCESS
    if(id > HAL_FLASH_ID_0)
    {
        hal_norflash_read(id, satrt_addr, buff_b, sizeof(buff_b));
    }
    else
#endif
    {
        memcpy(buff_b, (uint8_t*)satrt_addr, sizeof(buff_b));
    }

    TRACE(1,"%s: chip erase start.",
              __func__);
    start_time = hal_fast_sys_timer_get();
    result = hal_norflash_erase_chip(id);
    flash_test_result[id].chip_erase_time = (hal_fast_sys_timer_get() - start_time);
    if(result != HAL_NORFLASH_OK)
    {
        TRACE(1,CONST_STR_BOOT_SRAM("%s: chip erase fail! ret = %d."),
              __func__, ret);
        ret = -2;
        goto _func_end;
    }
    TRACE(1, CONST_STR_BOOT_SRAM("%s: chip erase dnoe."), __func__);
    satrt_addr = _get_base_addr(id, false);
    p = (uint8_t*) satrt_addr;
    for(i = 0; i < restore_size; i++)
    {
        if(p[i] != 0xff)
        {
            TRACE(1,CONST_STR_BOOT_SRAM("%s: chip erase verify error! addr = 0x%x, value = 0x%x, leftover = %d,"),
            __func__,satrt_addr + i, p[i], restore_size - i);
            DUMP8(CONST_STR_BOOT_SRAM("0x%x"), (uint8_t*)satrt_addr + i, 32);
            ret = -3;
            goto _func_end;
        }
    }
    TRACE(1, CONST_STR_BOOT_SRAM("%s: chip erase verify dnoe."), __func__);

    if(is_restore != 0)
    {
        for(i = 0; i < sizeof(buff_b) / NORFLASH_API_SECTOR_MAX_SIZE; i++ )
        {
            addr = satrt_addr + i * NORFLASH_API_SECTOR_MAX_SIZE;
            result = hal_norflash_write(id, addr, buff_b + i * NORFLASH_API_SECTOR_MAX_SIZE, NORFLASH_API_SECTOR_MAX_SIZE);
            if(result != HAL_NORFLASH_OK)
            {
                TRACE(1,"%s: write fail! ret = %d.",
                      __func__, ret);
                ret = -4;
                goto _func_end;
            }
        }
    }
    TRACE(1,"%s: done.", __func__);
_func_end:
    int_unlock_global(lock);

    TRACE(1,"%s: return %d.", __func__, ret);
    osDelay(4);
    return ret;
}

static SRAM_TEXT_LOC  int _norflash_test(enum HAL_FLASH_ID_T id, uint32_t count, uint32_t erase_size, uint8_t is_restore, uint32_t offs, uint32_t size, uint32_t flash_size)
{
    uint32_t start_addr;
    uint32_t start_time,cur_time;
    int32_t ret = 0;
    uint32_t i,j;
    uint32_t step = 1;

    step = is_restore ? 2 : 1;
    if(is_restore)
    {
        ASSERT(count >= 2, "%s: count too few! count = %d.",
            __func__, count);
        ASSERT((count/2)*2 == count, "%s: count not a multiple of 2! count = %d.",
            __func__, count);
        count = count/2;
    }

    ASSERT(offs + size <= flash_size, "write overflow. offs = 0x%x, size = 0x%x, flash_size = 0x%x", offs, size, flash_size);
    TRACE(1,"%s begin count = %08d", __func__,count);
    start_time = hal_fast_sys_timer_get();
#ifdef FLASH_CACHE_TEST
    start_addr = _get_base_addr(id, true);
#else
    start_addr = _get_base_addr(id, false);
#endif
    start_addr += offs;
    for(i = 0; i < count; i++)
    {
        test_erase_time = 0;
        test_write_time = 0;
        test_read_time = 0;
        for(j = 0; j < size/erase_size; j++) {
            // if is_restore = 1, erase/write twice per call
            ret = _flash_ewr(id, start_addr + j*erase_size, erase_size, is_restore, 0, 0, 0);
            if(ret) {
                TRACE(0,"%s: _flash_ewr failed ret: %d, index: %d", __func__, ret,j);
                goto _func_fail;
            }
        }

        cur_time = hal_fast_sys_timer_get();
        osDelay(2);
        TRACE(0,"%s: E/W/R(0x%08x--0x%08x) count: %08d,%08d",
                __func__,
                start_addr,
                start_addr + size,
                (i+1)*step,
                TICKS_TO_MS(cur_time - start_time));
        test_erase_time = FAST_TICKS_TO_MS(test_erase_time);
        test_write_time = FAST_TICKS_TO_MS(test_write_time);
        test_read_time = FAST_TICKS_TO_MS(test_read_time);
        if(erase_size == norflash_api_get_block_size(id))
        {
            flash_test_result[id].erase_speed_block = (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_erase_time));
        }
        else if(erase_size == norflash_api_get_sector_size(id))
        {
            flash_test_result[id].erase_speed_sector = (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_erase_time));
        }
        else
        {
            flash_test_result[id].erase_speed_page = (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_erase_time));
        }
        flash_test_result[id].write_speed = (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_write_time));

        TRACE(1,"e_speed(%08d)kb/s,w_speed(%08d)kb/s,r_speed(%08d)kb/s ",
            (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_erase_time)),
            (uint32_t)((((((uint64_t)step)*size)/1024)*1000)/(test_write_time)),
            (uint32_t)((((((uint64_t)step*2)*size)/1024)*1000)/(test_read_time)));

    }

    TRACE(1,"%s done", __func__);
    return 0;

 _func_fail:
    TRACE(1,"%s failed! ret = %d", __func__,ret);
    return ret;
}
#endif

#ifdef FLASH_BASE_FUNCTION_TEST

static SRAM_TEXT_LOC void _flash_base_function_test(enum HAL_FLASH_ID_T id)
{
    int ret;
    uint32_t flash_size = hal_norflash_get_flash_total_size(id);

#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif
    TRACE(1,"%s begin", __func__);
    _set_print_details(true);
#ifdef FLASH_CHIP_ERASE_TEST
    ret = _flash_chip_erase(id, 1);
    if(ret)
    {
        TRACE(1,"%s _flash_chip_erase fail! ret = %d", __func__, ret);
    }
#endif

#ifdef FLASH_BLOCK_ERASE_TEST

    ret = _norflash_test(id, 2, norflash_api_get_block_size(id), true, 0, flash_size, flash_size);
    if(ret)
    {
        goto _func_end;
    }
#endif

    ret = _norflash_test(id, 2, norflash_api_get_sector_size(id), true, 0, flash_size, flash_size);
    if(ret)
    {
        goto _func_end;
    }

#ifdef PUYA_FLASH_ERASE_PAGE_ENABLE
    ret = _norflash_test(id, 2, norflash_api_get_page_size(id), true, 0, flash_size, flash_size);
    if(ret)
    {
        goto _func_end;
    }
#endif

_func_end:
    if(ret) {
        flash_test_result[id].base_func_pass = false;
        flash_test_result[id].erase_pass = false;
        flash_test_result[id].write_pass = false;
        flash_test_result[id].read_pass = false;
        TRACE(0,"%s: _flash_ewr failed ret: %d", __func__, ret);
    }
    else
    {
        flash_test_result[id].base_func_pass = true;
        flash_test_result[id].erase_pass = true;
        flash_test_result[id].write_pass = true;
        flash_test_result[id].read_pass = true;
        TRACE(0,"%s done", __func__);
    }
#ifdef FLASH_PROTECTION
    hal_norflash_enable_protection(id);
#endif

}
#endif // FLASH_LIFE_TEST

#ifdef FLASH_SIMPLE_TEST
static void _flash_read_speed_test(enum HAL_FLASH_ID_T id);
POSSIBLY_UNUSED
static SRAM_TEXT_LOC void _flash_base_simple_test(enum HAL_FLASH_ID_T id)
{
    int ret;
    uint32_t addr, erase_size, offs;
    uint32_t total_size = 0;
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t n;
#ifdef FLASH_CACHE_TEST
    uint32_t satrt_addr = _get_base_addr(id, true);
#else
    uint32_t satrt_addr = _get_base_addr(id, false);
#endif

#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif
    TRACE(1,"%s begin", __func__);
    _set_print_details(true);
#ifdef FLASH_SUSPEND_TEST
    ret = _flash_suspend_test(id);
    if(ret)
    {
        TRACE(1,"%s _flash_suspend_test fail! ret = %d", __func__, ret);
    }
#endif
#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif

#ifdef FLASH_CHIP_ERASE_TEST
    ret = _flash_chip_erase(id, 1);
    if(ret)
    {
        TRACE(1,"%s _flash_chip_erase fail! ret = %d", __func__, ret);
    }
#endif
     hal_norflash_get_size(id, &total_size, &block_size, &sector_size,           &page_size);
#ifdef FLASH_BLOCK_ERASE_TEST
    n = 0;
    erase_size = block_size;
    do
    {
        offs = erase_size*((1<<n++)-1);
        if(offs > total_size - erase_size)
        {
            break;
        }
        addr = satrt_addr + offs;
        ret = _flash_ewr(id, addr, erase_size, true, 0, 0, 0);
        if(ret)
        {
            TRACE(0,"%s: _flash_ewr failed ret: %d", __func__, ret);
            goto _func_end;
        }
    }while(1);
#endif
    n = 0;
    erase_size = sector_size;
    do
    {
        offs = erase_size*((1<<n++)-1);
        if(offs > total_size - erase_size)
        {
            break;
        }
        addr = satrt_addr + offs;

        ret = _flash_ewr(id, addr, erase_size, true, 0, 0, 0);
        if(ret)
        {
            TRACE(0,"%s: _flash_ewr failed ret: %d", __func__, ret);
            goto _func_end;
        }
    }while(1);
#ifdef PUYA_FLASH_ERASE_PAGE_ENABLE
    n = 0;
    erase_size = page_size;
    do
    {
        offs = erase_size*((1<<n++)-1);
        if(offs > total_size - erase_size)
        {
            break;
        }

        addr = satrt_addr + offs;

        ret = _flash_ewr(id, addr, erase_size, true, 0, 0, 0);
        if(ret)
        {
            TRACE(0,"%s: _flash_ewr failed ret: %d", __func__, ret);
            goto _func_end;
        }
    }while(1);
#endif

#ifdef FLASH_READ_SPEED
    _flash_read_speed_test(id);
#endif

_func_end:
    if(ret) {
        flash_test_result[id].base_func_pass = false;
        flash_test_result[id].erase_pass = false;
        flash_test_result[id].write_pass = false;
        flash_test_result[id].read_pass = false;
        TRACE(0,"%s: _flash_ewr failed ret: %d", __func__, ret);
    }
    else
    {
        flash_test_result[id].base_func_pass = true;
        flash_test_result[id].erase_pass = true;
        flash_test_result[id].write_pass = true;
        flash_test_result[id].read_pass = true;
        TRACE(0,"%s done", __func__);
    }
#ifdef FLASH_PROTECTION
    hal_norflash_enable_protection(id);
#endif

}
#endif // FLASH_LIFE_TEST


#ifdef FLASH_READ_SPEED
#include "dwt.h"

SRAM_TEXT_LOC void read_flash(long *a, uint32_t asize)
{
    uint32_t cyc_cnt = -1u;
    long temp_val = 0;
    long *src = a;

    dwt_reset_cycle_cnt();
    while (src < (a + asize)) {
        /*
         * Note:check the asm code make sure the next instruction is
         * aligned to 4
         */
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);

        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);
        asm volatile ("ldr.w %0, [%1], #4":"=r" (temp_val), "+r"(src)::);

    }
     cyc_cnt = dwt_read_cycle_cnt();
     cyc_cnt = cyc_cnt;
    if (cyc_cnt != -1u)
        TRACE(1, "\tcpu cycles %d \n", cyc_cnt);
}

static SRAM_TEXT_LOC  void _flash_read_speed_test(enum HAL_FLASH_ID_T id)
{
    uint32_t addr;
    uint32_t flash_size = 0;
    uint32_t start_time, end_time;
    uint32_t len;
    uint32_t total_time = 0;;
    uint32_t speed;
    uint32_t long_size;

    for(int i = 0; i < 2; i++)
    {
        if(i == 0)
        {
            addr = _get_base_addr(id, true);
        }
        else
        {
            addr = _get_base_addr(id, false);
        }
        long_size = sizeof(long);
        TRACE(0,"%s begin, long_size = %08d", __func__, long_size);
        flash_size = hal_norflash_get_flash_total_size(id);

        len = flash_size/long_size;
        start_time = hal_fast_sys_timer_get();
        read_flash((long*)addr,len);
        end_time = hal_fast_sys_timer_get();
        total_time = (end_time - start_time);

        speed = (((uint64_t)flash_size/1024)*1000)/FAST_TICKS_TO_MS(total_time);
        if(i == 0)
        {
            flash_test_result[id].read_speed_cache = speed;
            TRACE(0,"%s FLASH CACHE read speed = %08d(KB/S)", __func__, speed);
        }
        else
        {
            flash_test_result[id].read_speed_no_cache = speed;
            TRACE(0,"%s FLASH NO CACHE read speed = %08d(KB/S)", __func__, speed);
        }

     }
     TRACE(1,"%s done", __func__);
}
#endif // FLASH_READ_SPEED

#ifdef FLASH_NORFLASH_API_TEST

enum NAPI_TESTC_CASE_ID{
    NAPI_TEST_CASE_REMAP_0,
    NAPI_TEST_CASE_REMAP_A,
    NAPI_TEST_CASE_REMAP_B,
};

typedef struct {
    enum NAPI_TESTC_CASE_ID case_id;
    enum HAL_NORFLASH_REMAP_ID_T remap_id;
}NAPI_TEST_REMAP_T;

typedef int (* NAPI_TESTC_TEST_CASE_HANDLER)(enum HAL_FLASH_ID_T id, void* param);

typedef struct{
    NAPI_TEST_REMAP_T remap;
    NAPI_TESTC_TEST_CASE_HANDLER pfunc;
}NAPI_TESTC_CASE_T;

static SRAM_TEXT_LOC int _noflash_api_test_remap(enum HAL_FLASH_ID_T id, void* param);

#ifdef FLASH_REMAP

static NAPI_TESTC_CASE_T napi_test_case[] =
{
 {{NAPI_TEST_CASE_REMAP_A, HAL_NORFLASH_REMAP_ID_0}, (NAPI_TESTC_TEST_CASE_HANDLER)_noflash_api_test_remap},
 {{NAPI_TEST_CASE_REMAP_B, HAL_NORFLASH_REMAP_ID_0}, (NAPI_TESTC_TEST_CASE_HANDLER)_noflash_api_test_remap},
};

#else

static NAPI_TESTC_CASE_T napi_test_case[]={{{NAPI_TEST_CASE_REMAP_0, HAL_NORFLASH_REMAP_ID_0}, (NAPI_TESTC_TEST_CASE_HANDLER)_noflash_api_test_remap}};

#endif

POSSIBLY_UNUSED
static SRAM_TEXT_LOC int _noflash_api_test_remap(enum HAL_FLASH_ID_T id, void* param)
{
    NAPI_TEST_REMAP_T *test_case = (NAPI_TEST_REMAP_T*)param;
    enum HAL_FLASH_ID_T dev_id;
    enum NAPI_TESTC_CASE_ID case_id;
    enum HAL_NORFLASH_REMAP_ID_T remap_id;
    int ret = 0;

    dev_id = id;
    case_id = test_case->case_id;
    remap_id = test_case->remap_id;
    TRACE(1, "%s: id = %d, case_id = %d, remap_ip = %d", __func__, id, case_id, remap_id);
    if(case_id == NAPI_TEST_CASE_REMAP_0)
    {
        TRACE(1, "\nNORFLASH API TEST CASE 0: FLASH_REMAP UNDIFINED\n");
        ret = 0;
    }
    if(case_id == NAPI_TEST_CASE_REMAP_A)
    {
        TRACE(1, "\nNORFLASH API TEST CASE 1: FLASH_REMAP DISABLE\n");
        ret = hal_norflash_get_remap_status(dev_id, remap_id);
        if(ret == 1)
        {
            ret = hal_norflash_disable_remap(dev_id, remap_id);
            TRACE(1, "%s remap disable, ret = %d", __func__, ret);
        }
    }
    else if(case_id == NAPI_TEST_CASE_REMAP_B)
    {
        TRACE(1, "\nNORFLASH API TEST CASE 2: FLASH_REMAP ENABLE\n");
        ret = hal_norflash_get_remap_status(dev_id, remap_id);
        if(ret == 0)
        {
            ret = hal_norflash_enable_remap(dev_id, remap_id);
            TRACE(1, "%s remap enalbe, ret = %d", __func__, ret);
        }
    }
    return ret;
}

static SRAM_TEXT_LOC void _noflash_api_test_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;
     opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    TRACE(1,"%s:%d,0x%x,0x%x,%d,%d,%d.\n",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->remain_num,
                opera_result->result,
                opera_result->suspend_num);

}

SRAM_TEXT_LOC void _flash_flush(enum NORFLASH_API_MODULE_ID_T module,
                                enum NORFLASH_API_OPRATION_TYPE type)
{
    do
    {
        norflash_api_flush();
        if (NORFLASH_API_ALL != type)
        {
            if (0 == norflash_api_get_used_buffer_count(module, type))
            {
                break;
            }
        }
        else
        {
            if (norflash_api_buffer_is_free(module))
            {
                break;
            }
        }
        osDelay(10);
    } while (1);
}

SRAM_TEXT_LOC  void _norflash_api_test(enum HAL_FLASH_ID_T id)
{
#define REGISTER_ADDR_LEFT_REDUCTION   0x2000
#define REGISTER_ADDR_RIGHT_REDUCTION  0x1000

    uint32_t base_addr;
    uint32_t start_addr = 0;
    uint32_t end_addr = 0;
    uint32_t register_start_addr;
    uint32_t register_len;
    uint32_t addr;
    uint32_t len = 0;
    uint32_t wlen = 0;
    bool is_async = true;
    int32_t ret = 0;
    uint32_t flash_total_size = 0;
    uint32_t flash_block_size = 0;
    uint32_t flash_sector_size = 0;
    uint32_t flash_page_size = 0;
    uint8_t remap_opened = 0;
    uint8_t remap_status = 0;
    uint32_t remap_addr = 0;
    uint32_t remap_len = 0;
    uint32_t keepout_start = 0xffffffff;
    uint32_t keepout_end   = 0xffffffff;
    uint32_t c,i,j;
    uint8_t* p;
    enum NORFLASH_API_MODULE_ID_T mod_id;
    static uint8_t val = 0;

#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(id, true);
#else
    base_addr = _get_base_addr(id, false);
#endif

    TRACE(0,"%s begin", __func__);
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(id);
    len = end_addr - start_addr;

    register_start_addr = start_addr + REGISTER_ADDR_LEFT_REDUCTION;
    register_len = len - REGISTER_ADDR_LEFT_REDUCTION - REGISTER_ADDR_RIGHT_REDUCTION;
    TRACE(0, "%s: id = %d, start_addr = 0x%x, end_addr = 0x%x, len = 0x%x.",
                 __func__, id, start_addr, end_addr, len);

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, &flash_page_size);

    osDelay(4);

    mod_id = id == HAL_FLASH_ID_0 ? NORFLASH_API_MODULE_ID_TEST1: NORFLASH_API_MODULE_ID_TEST2;
    ret = norflash_api_register(
                          mod_id,
                          id,
                          register_start_addr,
                          register_len,
                          flash_block_size,
                          flash_sector_size,
                          flash_page_size,
                          flash_sector_size,
                          _noflash_api_test_callback
                          );

    if(ret) {
        TRACE(0,"%s: norflash_api_register failed ret:%d ", __func__, ret);
        goto _func_fail;
    }
    TRACE(0, "%s: norflash_api_register done. start_addr = 0x%x, len = 0x%x.",
                 __func__, register_start_addr, register_len);

    // write/read checking
    wlen = flash_sector_size;

    for(c = 0; c < sizeof(napi_test_case)/sizeof(napi_test_case[0]); c++)
    {
        ret = napi_test_case[c].pfunc(id, &napi_test_case[c].remap);
        if(ret)
        {
            TRACE(0,"%s remap init faile!!! ret = %d", __func__, ret);
            goto _func_fail;
        }

        // Get flash remap keepout.
        ret = norflash_api_get_remap_info(mod_id,
                                   &remap_opened,
                                   &remap_status,
                                   &remap_addr,
                                   &remap_len);
        if(ret == NORFLASH_API_OK)
        {
            if(remap_opened == 1)
            {
                TRACE(0,"%s remap is opened, status: %d, remap_addr: 0x%x, remap_len: 0x%x",
                      __func__, remap_status, remap_addr, remap_len);
                keepout_start = base_addr | (remap_addr + remap_len);
                keepout_end = base_addr | (remap_addr + 2*remap_len);
                TRACE(0,"%s: keepout: 0x%x -- 0x%x",
                      __func__, keepout_start, keepout_end);
            }
            else
            {
                TRACE(0,"%s remap is closed", __func__);
            }
        }
        else
        {
            TRACE(0,"%s norflash_api_remap_info faile ret = %d", __func__, ret);
            goto _func_fail;
        }

        for(i = 0; i < len/wlen; i++) {
            addr = (uint32_t)start_addr + i*wlen;

            if(addr >= keepout_start &&
               addr < keepout_end)
            {
                TRACE(0,"%s: (0x%x)in keekout,skip! ", __func__, addr);
                osDelay(1);
                continue;
            }
            ret = norflash_api_erase(mod_id,addr,wlen,is_async);
            if(ret == NORFLASH_API_OK) {

                if(addr < register_start_addr || addr + wlen > register_start_addr + register_len)
                {
                    TRACE(0,"%s norflash_api_erase overflow, but no erro returned! ", __func__);
                    TRACE(0,"addr = 0x%x, len = 0x%x, valid(0x%x, 0x%x)", addr, len, register_start_addr, register_len);
                    goto _func_fail;
                }
                TRACE(0,"%s norflash_api_erase ok", __func__);
                //break;
            }
            else if(ret != NORFLASH_API_BUFFER_FULL) {
                if(ret == NORFLASH_API_BAD_ADDR)
                {
                    if(addr < register_start_addr || addr + wlen > register_start_addr + register_len)
                    {
                        TRACE(0,"%s norflash_api_erase overflow! ret = %d", __func__,ret);
                        TRACE(0,"addr = 0x%x, len = 0x%x, valid(0x%x, 0x%x)", addr, len, register_start_addr, register_len);
                    }
                    else
                    {
                         TRACE(0,"%s norflash_api_erase failed ret = %d", __func__,ret);
                         goto _func_fail;
                    }
                }
                else
                {
                    TRACE(0,"%s norflash_api_erase failed ret = %d", __func__,ret);
                    goto _func_fail;
                }
            }
            else
            {
                _flash_flush(mod_id, NORFLASH_API_ERASING);
            }

            TRACE(0,"%s norflash_api_write(0x%x, 0x%x)", __func__, addr, wlen);

            for(j = 0; j < wlen; j++)
            {
                buff_w[j] = val;
            }
            val++;

            ret = norflash_api_write(mod_id,addr,buff_w,wlen,is_async);
            if(ret == NORFLASH_API_OK) {
                if(addr < register_start_addr || addr + wlen > register_start_addr + register_len)
                {
                    TRACE(0,"%s norflash_api_write overflow, but no erro returned! ", __func__);
                    TRACE(0,"addr = 0x%x, len = 0x%x, valid(0x%x, 0x%x)", addr, len, register_start_addr, register_len);
                    goto _func_fail;
                }
                TRACE(0,"%s norflash_api_write ok", __func__);

            }
            else if(ret != NORFLASH_API_BUFFER_FULL) {
                if(ret == NORFLASH_API_BAD_ADDR)
                {
                    if(addr < register_start_addr || addr + wlen > register_start_addr + register_len)
                    {
                        TRACE(0,"%s norflash_api_write overflow! ret = %d", __func__,ret);
                        TRACE(0,"addr = 0x%x, len = 0x%x, valid(0x%x, 0x%x)", addr, len, register_start_addr, register_len);
                    }
                    else
                    {
                         TRACE(0,"%s norflash_api_erase failed ret = %d", __func__,ret);
                         goto _func_fail;
                    }
                }
                else
                {
                    TRACE(0,"%s norflash_api_erase failed ret = %d", __func__,ret);
                    goto _func_fail;
                }
            }
            else
            {
                _flash_flush(mod_id, NORFLASH_API_WRITTING);
            }

            _flash_flush(mod_id, NORFLASH_API_ALL);
            norflash_api_remap_start(mod_id, addr, wlen);
#ifdef FLASH_CACHE_TEST
            hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, addr, wlen);
            hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, addr, wlen);
#endif
            if(ret != NORFLASH_API_BAD_ADDR)
            {
                p = (uint8_t*)addr;
                for(j = 0; j < wlen; j++)
                {
                    if(p[j] != buff_w[j])
                    {
                        TRACE(0,"%s verify error! j = %d", __func__, j);
                        DUMP8("0x%x,", p, 16);
                        DUMP8("0x%x,", buff_w, 16);
                        goto _func_fail;
                        break;
                    }
                }
            }
            norflash_api_remap_done(mod_id, addr, wlen);
            TRACE_FLUSH();
        }
    }

    _flash_flush(mod_id, NORFLASH_API_ALL);

    TRACE(1,"%s: e/w (0x%x - 0x%x) is_async(%d) ok.",
        __func__, start_addr, end_addr, is_async);

    flash_test_result[id].norflash_api_pass = true;

    TRACE(0,"%s done", __func__);
    return;

 _func_fail:
    flash_test_result[id].norflash_api_pass = false;
    TRACE(0,"%s failed! ret = %d", __func__,ret);
    return;
}
#endif // FLASH_NORFLASH_API_TEST

#ifdef FLASH_SECURITY_REGISTER_TEST

#define SEC_REG_FLASH_CTRL_ID_MASK          0xFF000000
#define FLASH_SEC_REG_ADDR_MASK             (~(SEC_REG_FLASH_CTRL_ID_MASK))
SRAM_TEXT_LOC int _read_security_register(enum HAL_FLASH_ID_T id, unsigned int addr, unsigned char *data, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_read(id, addr, data, len);
}

SRAM_TEXT_LOC int _erase_security_register(enum HAL_FLASH_ID_T id, unsigned int addr, unsigned int len) {
#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_erase(id, addr, len);
}

SRAM_TEXT_LOC int _get_security_register_lock_status(enum HAL_FLASH_ID_T id, unsigned int addr, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif

    return hal_norflash_security_register_is_locked(id, addr, len);
}

SRAM_TEXT_LOC int _lock_security_register(enum HAL_FLASH_ID_T id, unsigned int addr, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_lock(id, addr, len);
}

SRAM_TEXT_LOC int _burn_security_register(enum HAL_FLASH_ID_T id, unsigned int addr, const unsigned char *data, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_write(id, addr, data, len);
}

SRAM_TEXT_LOC int _verify_security_register_data(enum HAL_FLASH_ID_T id, unsigned int addr, unsigned char *load_buf, const unsigned char *data, unsigned int len)
{
    int ret;
    const unsigned char *fptr;
    const unsigned char *mptr;
    unsigned char fval;
    unsigned char mval;
    int i;
    uint32_t sr_addr;

#ifdef FLASH1_CTRL_BASE
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif

    sr_addr = addr;
    ret = hal_norflash_security_register_read(id, sr_addr, load_buf, len);
    if (ret) {
        TRACE(1,"*** Failed to read sec reg: %d", ret);
        return 1;
    }

    fptr = load_buf;
    mptr = data;
    for (i = 0; i < len; i++) {
        fval = *fptr++;
        mval = *mptr++;
        if (fval != mval) {
            --fptr;
            --mptr;
            TRACE(0, "*** Verify sec reg data failed: 0x%02X @ %p != 0x%02X @ %p", fval, (void *)(addr + (fptr - load_buf)), mval, (void *)mptr);
            DUMP8("0x%02X ", load_buf + i, 16);
            return fval - mval;
        }
    }
    if(0)
    {
        for (i = 0; i < len/16; i ++)
        {
            uint8_t *p = (uint8_t*)load_buf + i*16;
            DUMP8("0x%02X ", p, 16);
            TRACE_FLUSH();
        }
    }
    return 0;
}
#define FLASH_SECURITY_SECTOR_SIZE    (1024)
#define FLASH_SECURITY_SIZE (3*FLASH_SECURITY_SECTOR_SIZE)
SRAM_TEXT_LOC int _flash_security_register_test(enum HAL_FLASH_ID_T id)
{
#ifdef FLASH_SECURITY_REGISTER
    uint32_t i;
    uint32_t addr;
    int ret;
    uint32_t lock;

    TRACE(1, "%s, id = %d", __func__, id);
    for(i = 0; i < FLASH_SECURITY_SIZE/FLASH_SECURITY_SECTOR_SIZE; i++)
    {
        addr = i*FLASH_SECURITY_SECTOR_SIZE;
        if(1)
        {
            lock = int_lock_global();
            ret = _get_security_register_lock_status(id, addr, FLASH_SECURITY_SECTOR_SIZE);
            int_unlock_global(lock);
            if(ret)
            {
                TRACE(1, "%s: (@0x%x,0x%x) is locked! ret = %d", __func__, addr, FLASH_SECURITY_SECTOR_SIZE, ret);
                goto _ERR;
            }
            else
            {
                TRACE(1, "%s: (@0x%x,0x%x) is unlocked ret = %d", __func__, addr, FLASH_SECURITY_SECTOR_SIZE, ret);

            }
        }

        if(1)
        {
            lock = int_lock_global();
            ret = _erase_security_register(id,  addr, FLASH_SECURITY_SECTOR_SIZE);
            int_unlock_global(lock);
            if(ret)
            {
                TRACE(1, "%s: _erase_security_register faile! ret = %d", __func__, ret);
                goto _ERR;
            }
            TRACE(1, "%s: _erase_security_register done", __func__);
        }

        memset(buff_w, i+0xA, FLASH_SECURITY_SECTOR_SIZE);
        if(1)
        {
            lock = int_lock_global();
            ret = _burn_security_register(id, addr, buff_w, FLASH_SECURITY_SECTOR_SIZE);
            int_unlock_global(lock);
            if(ret)
            {
                TRACE(1, "%s: _burn_security_register faile! ret = %d", __func__, ret);
                goto _ERR;
            }
            TRACE(1, "%s: _burn_security_register done", __func__);
        }
        lock = int_lock_global();
        ret = _verify_security_register_data(id, addr, buff_r, buff_w, FLASH_SECURITY_SECTOR_SIZE);
        int_unlock_global(lock);
        if(ret)
        {
            TRACE(1, "%s: _verify_security_register_data faile! ret = %d", __func__, ret);
            goto _ERR;
        }
        TRACE(1, "%s: _verify_security_register_data done", __func__);
    }
    flash_test_result[id].security_register_pass = true;
    TRACE(1, "id %d FLASH SECURITY TEST OK", id);
    TRACE_FLUSH();
    return 0;
_ERR:
    flash_test_result[id].security_register_pass = false;
    TRACE(1, "id %d FLASH SECURITY TEST FAIL", id);
    TRACE_FLUSH();
    return 1;
#else
    TRACE(1, "id %d UDDEFINE FLASH_SECURITY_REGISTER!!!", id);
    TRACE_FLUSH();
    return 2;
#endif
    return 0;
}

#endif // FLASH_SECURITY_REGISTER_TEST


// Current test.

#define CURRENT_TEST_COUNT_DOWN  5

SRAM_TEXT_LOC void _flash_erase_all(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t base_addr;
    uint32_t start_addr, end_addr;
    uint32_t len;
    uint32_t erase_size = norflash_api_get_block_size(dev_id);
    uint32_t i;

#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(dev_id, true);
#else
    base_addr = _get_base_addr(dev_id, false);
#endif
    TRACE(0,"%s", __func__);
    TRACE_FLUSH();
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_block_size(dev_id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(dev_id);
    len = end_addr - start_addr;
    TRACE(0,"%s erase 0x%x - 0x%x", __func__, start_addr, end_addr);
    TRACE_FLUSH();
    for(i = 0; i < len/erase_size; i++)
    {
        uint32_t addr = start_addr + i*erase_size;
        hal_norflash_erase(dev_id, addr, erase_size);
    }

    TRACE(0,"%s done", __func__);
    TRACE_FLUSH();
}

#ifdef FLASH_ERASE_CURRENT_TEST
SRAM_TEXT_LOC void _flash_erase_current(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t base_addr;
    uint32_t start_addr, end_addr;
    uint32_t len;
    uint32_t erase_size = norflash_api_get_sector_size(dev_id);
    uint32_t i;
    uint32_t index = 0;
#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(dev_id, true);
#else
    base_addr = _get_base_addr(dev_id, false);
#endif
    TRACE(0,"%s begin", __func__);
    TRACE_FLUSH();
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(dev_id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(dev_id);
    len = end_addr - start_addr;
    while(1)
    {
        TRACE(1,"%s erase times: %d", __func__, index++);
        TRACE_FLUSH();
        for(i = 0; i < len/erase_size; i++)
        {
            uint32_t addr = start_addr + i*erase_size;
            hal_norflash_erase(dev_id, addr, erase_size);
        }

#if defined(FLASH_ERASE_CURRENT_TEST) && defined(FLASH_PROGRAM_CURRENT_TEST) && defined(FLASH_READ_CURRENT_TEST)
        if(index >= 1)
        {
            TRACE(1,"%s erase done", __func__);
            break;
        }
#endif
    }
}

#endif

#ifdef FLASH_PROGRAM_CURRENT_TEST

SRAM_TEXT_LOC void _flash_program_current(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t base_addr;
    uint32_t start_addr, end_addr;
    uint32_t len;
    uint32_t erase_size = norflash_api_get_sector_size(dev_id);
    uint32_t i;
    uint32_t index = 0;
#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(dev_id, true);
#else
    base_addr = _get_base_addr(dev_id, false);
#endif
    TRACE(0,"%s begin", __func__);
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(dev_id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(dev_id);
    len = end_addr - start_addr;
    while(1)
    {
        TRACE(1,"%s program times: %d", __func__, index++);
        TRACE_FLUSH();
        for(i = 0; i < len/erase_size; i++)
        {
            uint32_t addr = start_addr + i*erase_size;
            hal_norflash_write(dev_id, addr, buff_w, erase_size);
        }

#if defined(FLASH_ERASE_CURRENT_TEST) && defined(FLASH_PROGRAM_CURRENT_TEST) && defined(FLASH_READ_CURRENT_TEST)
        if(index >= 1)
        {
            TRACE(1,"%s program done", __func__);
            break;
        }
#endif
    }
}
#endif

#ifdef FLASH_READ_CURRENT_TEST
SRAM_TEXT_LOC void _flash_read_current(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t base_addr;
    uint32_t start_addr, end_addr;
    uint32_t len;
    uint32_t index = 0;
    uint32_t total_size;
#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(dev_id, true);
#else
    base_addr = _get_base_addr(dev_id, false);
#endif
    total_size = hal_norflash_get_flash_total_size(dev_id);
    TRACE(0,"%s begin", __func__);
    TRACE(0,"%s total_size = 0x%x", __func__, total_size);
    TRACE_FLUSH();
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(dev_id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(dev_id);
    len = end_addr - start_addr;
    while(1)
    {
        TRACE(1,"%s read times: %d", __func__, index++);
        TRACE_FLUSH();
        read_flash((long*)start_addr,len);
#if defined(FLASH_ERASE_CURRENT_TEST) && defined(FLASH_PROGRAM_CURRENT_TEST) && defined(FLASH_READ_CURRENT_TEST)
        if(index >= 2)
        {
            break;
        }
#endif
    }
}
#endif

#ifdef FLASH_IDLE_CURRENT_TEST

SRAM_TEXT_LOC void _flash_idle_current_1(void)
{
    uint32_t start_time,end_time;

    TRACE(1, "%s: start", __func__);
    TRACE_FLUSH();
    start_time = hal_sys_timer_get();
    do
    {
        end_time = hal_sys_timer_get();
        if(TICKS_TO_MS(end_time - start_time) >= 20000)
        {
            break;
        }

    }while(1);
    TRACE(1, "%s: done", __func__);
    TRACE_FLUSH();
}

SRAM_TEXT_LOC void _flash_idle_current_2(void)
{
    uint32_t start_time,end_time;
    TRACE(1, "%s: start, System is entering idle, until shut down or reset.", __func__);
    TRACE_FLUSH();
    start_time = hal_sys_timer_get();
    do
    {
        end_time = hal_sys_timer_get();
        if(TICKS_TO_MS(end_time - start_time) >= 20000)
        {
            break;
        }
    }while(1);
    TRACE(1, "%s: done", __func__);
    TRACE_FLUSH();
}

#endif

POSSIBLY_UNUSED
SRAM_TEXT_LOC void _count_down(uint32_t sec, uint8_t *tra)
{
    uint32_t start_time,end_time;

    TRACE(1, "%s: count down %d seconds start", tra, sec--);
    TRACE_FLUSH();
    start_time = hal_sys_timer_get();
    do
    {
        if(sec == 0)
        {
            break;
        }
        end_time = hal_sys_timer_get();
        if(TICKS_TO_MS(end_time - start_time) >= 1000)
        {
            TRACE(1, "count down %d ", sec--);
            TRACE_FLUSH();
            start_time = hal_sys_timer_get();
        }

    }while(1);
    TRACE(1, "count down: done");
    TRACE_FLUSH();
}

#if defined(FLASH_ERASE_CURRENT_TEST) || defined(FLASH_PROGRAM_CURRENT_TEST) || defined(FLASH_READ_CURRENT_TEST) || defined(FLASH_IDLE_CURRENT_TEST)
SRAM_TEXT_LOC static void _flash_current_test(void)
{
    uint32_t i;
    for(i = 0; i < HAL_FLASH_ID_NUM; i ++)
    {
#ifdef FLASH_PROTECTION
        hal_norflash_disable_protection(i);
#endif
        memset(buff_w, 0x5a, sizeof(buff_w));
        TRACE(1, "%s: begin, flash id: %d, w_buffer: 0x%x", __func__, i, buff_w[0]);

#ifdef FLASH_PROGRAM_CURRENT_TEST
        _flash_erase_all(i);
#endif

#ifdef FLASH_IDLE_CURRENT_TEST
        _count_down(CURRENT_TEST_COUNT_DOWN, (uint8_t*)"FLASH_IDLE");
        _flash_idle_current_1();
#endif

#ifdef FLASH_PROGRAM_CURRENT_TEST
        _count_down(CURRENT_TEST_COUNT_DOWN, (uint8_t*)"FLASH_PROGRAM");
        _flash_program_current(i);
#endif

#ifdef FLASH_READ_CURRENT_TEST
        _count_down(CURRENT_TEST_COUNT_DOWN, (uint8_t*)"FLASH_READ");
        _flash_read_current(i);
#endif

#ifdef FLASH_ERASE_CURRENT_TEST
        _count_down(CURRENT_TEST_COUNT_DOWN, (uint8_t*)"FLASH_ERASE");
        _flash_erase_current(i);
#endif

#ifdef FLASH_IDLE_CURRENT_TEST
        if(0)
        {
            _count_down(CURRENT_TEST_COUNT_DOWN, (uint8_t*)"FLASH_IDLE");
            _flash_idle_current_2();
        }
#endif
        TRACE(1, "%s: flash id: %d current test done", __func__, i);

    }
}
#endif

#ifdef FLASH_RANDOM_READ_TEST

SRAM_TEXT_LOC void flash_random_read_test(void)
{
    uint32_t *addr;
    uint32_t *addr1;
    uint32_t val, val1;
    uint64_t index = 0;
    uint32_t rand_val;

#ifdef CHIP_BEST2003
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_9, HAL_CMU_FREQ_260M);
    TRACE_IMM(1, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
#endif

    TRACE_IMM(1, "FLASH RANDOM READ TEST");
    TRACE_IMM(1, "START ADDR: 0x%X, BAK START ADDR: 0x%X, len = 0x%x",
        RANDOM_READ_START,
        BAK_RANDOM_READ_START,
        RANDOM_READ_LEN);
    pmu_rtc_enable();
    uint32_t rtc_val = pmu_rtc_get();
    if(rtc_val <= 100 )
    {
        pmu_rtc_set(100);
    }
    TRACE_IMM(1, "srandom 0x%x", rtc_val);
    srandom(rtc_val);
    while(1)
    {
        rand_val = (uint32_t)random();
        addr = (uint32_t*)(RANDOM_READ_START + ((uint32_t)rand_val % RANDOM_READ_LEN));

        addr1 = (uint32_t*)((uint32_t)addr + (BAK_RANDOM_READ_START-RANDOM_READ_START));
        val = *addr;
        val1 = *addr1;

        if(val != val1)
        {
            TRACE_IMM(1, "@0x%x @0x%x 0x%08x != 0x%08x", (uint32_t)addr, (uint32_t)addr1, val, val1);
            break;
        }
        else
        {
            if(index++ % 0x100000 == 0)
            {
                TRACE_IMM(1, "RANDOM READ TEST: 0x%08x%08x", (uint32_t)(0xffffffff & (index >> 32)),(uint32_t)(0xffffffff & index));
                osDelay(100);
            }
   #ifdef RAND_READ_REBOOT_COUNT
            uint32_t reboot_cnt = RAND_READ_REBOOT_COUNT + (rand_val % 1000);
            if(index >= reboot_cnt)
            {
                TRACE_IMM(1,"REBOOT-0x%08x", reboot_cnt);
                pmu_reboot();
            }
   #endif
            //TRACE_IMM(1, "@0x%x @0x%x: 0x%08x ", (uint32_t)addr, (uint32_t)addr1, val);
        }
    }

    while(1)
    {
        TRACE_IMM(1, "@0x%x @0x%x 0x%08x != 0x%08x", (uint32_t)addr, (uint32_t)addr1,  val, val1);
        TRACE_IMM(1, "RANDOM READ FAIL! RANDOM READ FAIL! RANDOM READ FAIL!");
        osDelay(10000);
    }
}
#endif

#ifdef FLASH_SET_DUAL_CHIP_TEST
SRAM_TEXT_LOC void _flash_set_dual_chip_test(enum NORFLASH_API_MODULE_ID_T mod_id)
{
#define REGISTER_ADDR_LEFT_REDUCTION   0x2000
#define REGISTER_ADDR_RIGHT_REDUCTION  0x1000

    uint32_t base_addr;
    uint32_t start_addr = 0;
    uint32_t end_addr = 0;
    uint32_t register_start_addr;
    uint32_t register_len;
    uint32_t len = 0;
    int32_t ret = 0;
    uint32_t flash_total_size = 0;
    uint32_t flash_block_size = 0;
    uint32_t flash_sector_size = 0;
    uint32_t flash_page_size = 0;
    uint32_t addr;

    int dual_flash = 0;
    int dual_sec_reg = 0;
    //uint32_t lock;
    enum HAL_FLASH_ID_T id = HAL_FLASH_ID_0;
    uint32_t i,j;

#ifdef FLASH_CACHE_TEST
    base_addr = _get_base_addr(id, true);
#else
    base_addr = _get_base_addr(id, false);
#endif
    uint32_t lock = int_lock_global();

    TRACE_IMM(0,CONST_STR_BOOT_SRAM("_flash_set_dual_chip_test(%d)"), mod_id);
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(id);
    len = end_addr - start_addr;

    register_start_addr = start_addr + REGISTER_ADDR_LEFT_REDUCTION;
    register_len = len - REGISTER_ADDR_LEFT_REDUCTION - REGISTER_ADDR_RIGHT_REDUCTION;
    TRACE_IMM(0, CONST_STR_BOOT_SRAM("id = %d, start_addr = 0x%x, end_addr = 0x%x, len = 0x%x."),
                  id, start_addr, end_addr, len);

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, &flash_page_size);

    // TRACE_IMM(0, CONST_STR_BOOT_SRAM("id = %d, total size = 0x%x, block size = 0x%x, sector size = 0x%x, page size = 0x%x."),
    //              id, flash_total_size, flash_block_size, flash_sector_size, flash_page_size);
    for(j = 0; j < flash_sector_size; j++)
    {
        buff_w[j] = j;
    }
    mod_id = id == HAL_FLASH_ID_0 ? NORFLASH_API_MODULE_ID_TEST1: NORFLASH_API_MODULE_ID_TEST2;
    ret = norflash_api_register(
                          mod_id,
                          id,
                          register_start_addr,
                          register_len,
                          flash_block_size,
                          flash_sector_size,
                          flash_page_size,
                          flash_sector_size,
                          _noflash_api_test_callback
                          );

    if(ret)
    {
        TRACE_IMM(0, CONST_STR_BOOT_SRAM("_flash_set_dual_chip_test norflash_api_register failed ret:%d "), ret);
        goto _fail;
    }
    // TRACE_IMM(0, CONST_STR_BOOT_SRAM("norflash_api_register done. start_addr = 0x%x, len = 0x%x."),
    //               register_start_addr, register_len);

    ret = norflash_api_get_dual_chip_mode(mod_id, &dual_flash, &dual_sec_reg);
    if(ret)
    {
        TRACE_IMM(1, CONST_STR_BOOT_SRAM("hal_norflash_get_dual_chip_mode fail, ret = %d"), ret);
        goto _fail;
    }
    TRACE_IMM(1, CONST_STR_BOOT_SRAM("get dual chip mode(%d, %d, %d)"), mod_id, dual_flash, dual_sec_reg);

    dual_flash = !dual_flash;
    ret = norflash_api_set_dual_chip_mode(mod_id, dual_flash, dual_sec_reg);
    if(ret)
    {
        TRACE_IMM(1, CONST_STR_BOOT_SRAM("hal_norflash_set_dual_chip_mode fail, ret = %d"), ret);
        goto _fail;
    }
    TRACE_IMM(1, CONST_STR_BOOT_SRAM("set dual chip mode(%d, %d, %d) done"), mod_id, dual_flash, dual_sec_reg);

    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, norflash_api_get_sector_size(id));
    end_addr = base_addr | (uint32_t)hal_norflash_get_flash_total_size(id);
    len = end_addr - start_addr;

    register_start_addr = start_addr + REGISTER_ADDR_LEFT_REDUCTION;
    register_len = len - REGISTER_ADDR_LEFT_REDUCTION - REGISTER_ADDR_RIGHT_REDUCTION;
    TRACE_IMM(0, CONST_STR_BOOT_SRAM("id = %d, start_addr = 0x%x, end_addr = 0x%x, len = 0x%x."),
                 id, start_addr, end_addr, len);

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, &flash_page_size);

    //TRACE_IMM(0, CONST_STR_BOOT_SRAM("id = %d, total size = 0x%x, block size = 0x%x, sector size = 0x%x, page size = 0x%x."),
    //             id, flash_total_size, flash_block_size, flash_sector_size, flash_page_size);

    mod_id = id == HAL_FLASH_ID_0 ? NORFLASH_API_MODULE_ID_TEST1: NORFLASH_API_MODULE_ID_TEST2;
    ret = norflash_api_register(
                          mod_id,
                          id,
                          register_start_addr,
                          register_len,
                          flash_block_size,
                          flash_sector_size,
                          flash_page_size,
                          flash_sector_size,
                          _noflash_api_test_callback
                          );

    if(ret)
    {
        TRACE_IMM(0, CONST_STR_BOOT_SRAM("norflash_api_register failed ret:%d "), ret);
        goto _fail;
    }
    //TRACE_IMM(0, CONST_STR_BOOT_SRAM("norflash_api_register done. start_addr = 0x%x, len = 0x%x."),
    //              register_start_addr, register_len);

    for(i = 0; i < register_len / flash_sector_size; i++)
    {
        addr = register_start_addr + i*flash_sector_size;
        ret = norflash_api_erase(mod_id, addr, flash_sector_size, false);
        if(ret)
        {
            TRACE_IMM(0, CONST_STR_BOOT_SRAM("erase sector failed, addr = 0x%x, ret:%d "), addr, ret);
            goto _fail;
        }
        else
        {
          //  TRACE_INFO(0, CONST_STR_BOOT_SRAM("erase sector ok, addr = 0x%x "), addr);
        }

        ret = norflash_api_write(mod_id, addr, buff_w, flash_sector_size, false);
        if(ret)
        {
            TRACE_IMM(0, CONST_STR_BOOT_SRAM("write sector failed, addr = 0x%x, ret:%d "), addr, ret);
            goto _fail;
        }
        else
        {
           // TRACE_INFO(0, CONST_STR_BOOT_SRAM("erase write ok, addr = 0x%x "), addr);
        }

        ret = norflash_api_read(mod_id, addr, buff_r, flash_sector_size);
        if(ret)
        {
            TRACE_IMM(0, CONST_STR_BOOT_SRAM("read sector failed, addr = 0x%x, ret:%d "), addr, ret);
            goto _fail;
        }
        else
        {
            //TRACE_INFO(0, CONST_STR_BOOT_SRAM("erase read ok, addr = 0x%x "), addr);
        }
        for(j = 0; j < flash_sector_size; j++)
        {
            if(buff_r[j] != buff_w[j])
            {
                TRACE_IMM(0, CONST_STR_BOOT_SRAM("@0x%x R:0x%x != W:0x%x "), addr + j, buff_r[j], buff_w[j]);
                goto _fail;
            }
        }
        if(addr % 0x100000 == 0)
        {
            TRACE_IMM(0, CONST_STR_BOOT_SRAM("WRITE VERIFY ok, addr = 0x%x "), addr);
        }
       // TRACE_FLUSH();
    }

    TRACE_IMM(0, CONST_STR_BOOT_SRAM("_flash_set_dual_chip_test(%d) done "), mod_id);


    dual_flash = !dual_flash;
    ret = norflash_api_set_dual_chip_mode(mod_id, dual_flash, dual_sec_reg);
    if(ret)
    {
        TRACE_IMM(1, CONST_STR_BOOT_SRAM("hal_norflash_set_dual_chip_mode fail, ret = %d"), ret);
        goto _fail;
    }
    TRACE_IMM(1, CONST_STR_BOOT_SRAM("set dual chip mode(%d, %d, %d) done"), mod_id, dual_flash, dual_sec_reg);
    flash_test_result[id].set_dual_chip_pass = true;
    int_unlock_global(lock);
    return;
_fail:
    TRACE_IMM(0, CONST_STR_BOOT_SRAM("_flash_set_dual_chip_test(%d) fail!"), mod_id);
    int_unlock_global(lock);
    flash_test_result[id].set_dual_chip_pass = false;
    return;
}
#endif

#if defined(FLASH1_CTRL_BASE) && defined(CHIP_BEST2003) && defined(USE_MULTI_FLASH)
static void _init_flash1(void)
{
    // 32MB + 32MB
    // HAL_NORFLASH_ADDR_MASK: 0x01FFFFFF
    // FLASH0 MAX SIZE: 32MB
    // FLASH_BASE:     0x2C000000
    // FLASH_NC_BASE:  0x28000000
    // FLASH1_BASE:     0x2E000000
    // FLASH1_NC_BASE:  0x2A000000
    hal_cmu_set_flash0_size(HAL_FLASH0_SIZE_32M);
    hal_cmu_flash1_io_enable();
    hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH1);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH1);
}
#endif

#ifdef FLASH_RANDOM_WRITE_TEST

#define FLASH_SUSPEND_ALL_IRQ

#if defined(FLASH_SUSPEND) && defined(FLASH_SUSPEND_ALL_IRQ)
static IRQn irq_list[] = {
    DISPLAY_IRQn,
    DSI_IRQn,
    BT_STAMP_IRQn,
    BT_IRQn,
    SDMMC0_IRQn,
    BTH_IRQn,
    SEDMA_IRQn,
    SEC_ENG_IRQn,
    I2C0_IRQn,
    SPI0_IRQn,
    I2S0_IRQn,
    SPDIF0_IRQn,
    TRNG_IRQn,
    GPU_IRQn,
    IMGDMA_IRQn,
    PMU1_IRQn,
    PMU_IRQn,
    USB_PIN_IRQn,
    AON_GPIO_IRQn,
    EMMC_IRQn,
    WAKEUP_IRQn,
    PROT_IRQn,
    SPY_IRQn,
    SYS_WDT_IRQn,
    AON_IRQn,
    UART0_IRQn,
    SYS_TIMER00_IRQn,
    SYS_TIMER01_IRQn,
    SYS_TIMER10_IRQn,
    SYS_TIMER11_IRQn,
    SYS_TIMER20_IRQn,
    SYS_TIMER21_IRQn,
    AON_TIMER00_IRQn,
    AON_TIMER01_IRQn,
    UART1_IRQn,
    CODEC_TX_PEAK_IRQn,
    CODEC_IRQn,
    AUDMA_IRQn,
    GPDMA_IRQn,
    XDMA_IRQn,
    SYS2BTH_DONE_IRQn,
    SYS2BTH_DONE1_IRQn,
    BTH2SYS_DATA_IRQn,
    BTH2SYS_DATA1_IRQn,
    DSP2MCU_DATA_IRQn,
    DSP2MCU_DATA1_IRQn,
    DSP2MCU_DATA2_IRQn,
    DSP2MCU_DATA3_IRQn,
    MCU2DSP_DONE_IRQn,
    MCU2DSP_DONE1_IRQn,
    MCU2DSP_DONE2_IRQn,
    MCU2DSP_DONE3_IRQn,
    SENS_IRQn,
    SENS2SYS_DATA_IRQn,
    SENS2SYS_DATA1_IRQn,
    SENS2SYS_DATA2_IRQn,
    SENS2SYS_DATA3_IRQn,
    SYS2SENS_DONE_IRQn,
    SYS2SENS_DONE1_IRQn,
    SYS2SENS_DONE2_IRQn,
    SYS2SENS_DONE3_IRQn,
    I2S1_IRQn,
    USB_INSERT_IRQn,
    I2C1_IRQn,
    SYS_GPIO_IRQn,
    AON_WDT_IRQn,
    ISDONE_IRQn,
    ISDONE1_IRQn,
    LM_SENSOR_IRQn,
    AON_TIMER10_IRQn,
    AON_TIMER11_IRQn,
    SYS_TIMER30_IRQn,
    SYS_TIMER31_IRQn,
    BTPCM_IRQn,
    USER_IRQn_QTY,
    };
static void _set_irq_check(void)
{
    uint32_t i;
    for(i = 0; i < ARRAY_SIZE(irq_list); i++)
    {
        hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, irq_list[i], true);
    }
}

#else
static void _set_irq_check(void)
{
#ifdef FLASH_SUSPEND
    uint32_t i;
    const int16_t *irq = NULL;
    uint32_t irq_cnt = 0;

    hal_dma_get_irq_list(&irq, &irq_cnt);
    for(i = 0; i < irq_cnt; i++)
    {
        hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, irq[i], true);
    }
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA_IRQn, true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA1_IRQn, true);
#ifdef BLE_USB_AUDIO_SUPPORT
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, USB_IRQn, true);
#endif
#endif // FLASH_SUSPEND
}

#endif

static SRAM_TEXT_LOC int32_t _flash_suspend_random_write(enum HAL_FLASH_ID_T id)
{
    uint32_t addr;
    uint32_t len;
    uint32_t start_addr;
    uint32_t i,j;
    uint8_t val = 0;
    uint8_t tmp_len;
    uint32_t cur_addr;
    uint32_t cur_len;
    uint32_t offs;
    uint32_t remain;
    int32_t ret = 0;
    uint8_t* pstart;
    uint32_t lock;
    uint32_t flash_total_size = 0;
    uint32_t flash_block_size = 0;
    uint32_t flash_sector_size = 0;
    uint32_t flash_page_size = 0;
    uint32_t suspend_cnt;
    uint32_t write_cnt = 0;
    osTimerId timer_id;
    uint16_t status = 0;

    TRACE_IMM(1, "SUSPEND RANDOM WRITE TEST");
    _set_irq_check();

#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif

    timer_id = osTimerCreate(osTimer(NORFALSH_API_TEST_TIMER), osTimerPeriodic, NULL);
    osTimerStart(timer_id, timer_interval_ms);

    hal_norflash_get_size(id, &flash_total_size, &flash_block_size, &flash_sector_size, &flash_page_size);
    addr = _get_base_addr(id, false) + 256*1024;
    len = flash_total_size - 256*1024;

    for(i = 0; i < len/flash_block_size; i++)
    {
        suspend_cnt = 0;
        start_addr = addr + i*flash_block_size;
        lock = int_lock_global();
        ret = hal_norflash_erase_suspend(id, start_addr, flash_block_size, true);
        int_unlock_global(lock);
        while(1)
        {
            if(ret == HAL_NORFLASH_OK)
            {
                break;
            }
            if(ret != HAL_NORFLASH_SUSPENDED)
            {
                TRACE_IMM(1, "%s: erase suspend(%d, 0x%x, 0x%x, true) fail! ret = %d",
                    __func__, id, start_addr, flash_block_size, ret);
                goto _func_fail;
            }
            hal_sys_timer_delay_us(40);
            suspend_cnt ++;
            lock = int_lock_local();
            ret = hal_norflash_erase_resume(id, true);
            int_unlock_global(lock);
            hal_sys_timer_delay_us(100);
        }
         pstart = (uint8_t*)start_addr;
        for(j = 0; j < flash_block_size; j++)
        {

            if(pstart[j] != 0xFF)
            {
                TRACE_IMM(1, "%s: erase suspend verify fail! @0x%02x = 0x%02X",
                    __func__, (uint32_t)&pstart[j], pstart[j]);
                ret = 100;
                goto _func_fail;
            }
        }
        TRACE_IMM(1, "suspend erase(0x%x), suspend_cnt= %d", start_addr, suspend_cnt);

        tmp_len = 1;
        val = 0;
        remain = flash_block_size;
        offs = 0;
        suspend_cnt = 0;
        write_cnt = 0;
        while(1)
        {
            if(remain == 0)
            {
                break;
            }

            cur_addr = start_addr + offs;
            cur_len = remain >= tmp_len ? tmp_len : remain;
            tmp_len ++;

            for(j = 0; j < cur_len; j++)
            {
                buff_w[j] = val ++;
            }
            lock = int_lock_local();
            ret = hal_norflash_write_suspend(id, cur_addr, buff_w, cur_len, true);
            int_unlock_global(lock);
            write_cnt ++;
            while(1)
            {
                if(ret == HAL_NORFLASH_OK)
                {
                   break;
                }
                if(ret != HAL_NORFLASH_SUSPENDED)
                {
                    TRACE_IMM(1, "%s: erase suspend(%d, 0x%x, 0x%x, true) fail! ret = %d",
                        __func__, id, start_addr, cur_len, ret);
                    goto _func_fail;
                }
                suspend_cnt ++;
                hal_sys_timer_delay_us(40);
                lock = int_lock_local();
                ret = hal_norflash_write_resume(id, true);
                int_unlock_global(lock);
                hal_sys_timer_delay_us(100);
            }
            offs += cur_len;
            remain -= cur_len;
        }

        val = 0;
        pstart = (uint8_t*)start_addr;
        for(j = 0; j < flash_block_size; j++)
        {
            if(pstart[j] != val)
            {
                TRACE_IMM(1, "%s: write suspend verify fail! @0x%08x: x%02X  != 0x%02X",
                    __func__, (uint32_t)&pstart[j], pstart[j], val);
                ret = 101;
                goto _func_fail;
            }
            val ++;
        }
        TRACE_IMM(1, "suspend write(0x%x), write_cnt = %d, suspend_cnt= %d", start_addr, write_cnt, suspend_cnt);
        norflash_pre_operation(id);
        norflash_read_status(id, &status);
        norflash_post_operation(id);
        TRACE_IMM(1,"flash SR: 0x%04X.", status);
    }
    TRACE_IMM(1, "SUSPEND RANDOM WRITE DONE");
    flash_test_result[i].random_write_pass = true;
    return 0;

_func_fail:
    TRACE_IMM(1,"SUSPEND RANDOM TEST FAIL, ret(%d).", ret);

    norflash_pre_operation(id);
    norflash_read_status(id, &status);
    norflash_post_operation(id);
    TRACE_IMM(1,"flash SR: 0x%04X.", status);
    flash_test_result[i].random_write_pass = false;

    return ret;
}
#endif // FLASH_RANDOM_WRITE_TEST

#ifdef FLASH_LIFE_TEST
#define NORFLASH_LIFE_TEST_COUNT 100000
SRAM_TEXT_LOC  void norflash_life_test(enum HAL_FLASH_ID_T id)
{
    uint32_t offs = 0;
    uint32_t size = 0;
    uint32_t flash_size = hal_norflash_get_flash_total_size(id);

    TRACE_IMM(0,"%s begin", __func__);

#ifdef FLASH_PROTECTION
    hal_norflash_disable_protection(id);
#endif
    _set_print_details(false);

#if defined(FLASH_LIFE_TEST_OFFSET)
    offs = FLASH_LIFE_TEST_OFFSET;
#endif

#if defined(FLASH_LIFE_TEST_SIZE)
    size = FLASH_LIFE_TEST_SIZE;
#else
    size = flash_size;
#endif
    TRACE(1, "%s: offs = 0x%x, size = 0x%x, flash_size = 0x%x", __func__, offs, size, flash_size);
    _norflash_test(id, NORFLASH_LIFE_TEST_COUNT, norflash_api_get_block_size(id), true, offs, size, flash_size);
    TRACE_IMM(0,"%s done", __func__);
#ifdef FLASH_PROTECTION
    hal_norflash_enable_protection(id);
#endif

}
#endif // FLASH_LIFE_TEST

#if 1
#define TO_STR_A(s)                     # s
#define TO_STR(s)                       TO_STR_A(s)

typedef struct{
    const struct NORFLASH_CFG_T *cfg;
    uint8_t model_name[32];
}TEST_FLASH_INFO_T;


// GigaDevice
extern const struct NORFLASH_CFG_T gd25le255e_cfg;
extern const struct NORFLASH_CFG_T gd25le128e_cfg;
extern const struct NORFLASH_CFG_T gd25uf128e_cfg;
extern const struct NORFLASH_CFG_T gd25lf128e_cfg;
extern const struct NORFLASH_CFG_T gd25q128e_cfg;
extern const struct NORFLASH_CFG_T gd25lq64c_cfg;
extern const struct NORFLASH_CFG_T gd25uf64e_cfg;
extern const struct NORFLASH_CFG_T gd25lf64e_cfg;
extern const struct NORFLASH_CFG_T gd25lq32c_cfg;
extern const struct NORFLASH_CFG_T gd25lf32e_cfg;
extern const struct NORFLASH_CFG_T gd25lq16c_cfg;
extern const struct NORFLASH_CFG_T gd25lf16e_cfg;
extern const struct NORFLASH_CFG_T gd25lq80c_cfg;
extern const struct NORFLASH_CFG_T gd25lf80e_cfg;
extern const struct NORFLASH_CFG_T gd25q32c_cfg;
extern const struct NORFLASH_CFG_T gd25q80c_cfg;
extern const struct NORFLASH_CFG_T gd25d40c_cfg;
extern const struct NORFLASH_CFG_T gd25d20c_cfg;

// Puya
extern const struct NORFLASH_CFG_T p25q256l_cfg;
extern const struct NORFLASH_CFG_T p25q128l_cfg;
extern const struct NORFLASH_CFG_T py25q128laa_cfg;
extern const struct NORFLASH_CFG_T py25q128lac_cfg;
extern const struct NORFLASH_CFG_T p25q128sn_cfg;
extern const struct NORFLASH_CFG_T p25q64l_cfg;
extern const struct NORFLASH_CFG_T p25q64ha_cfg;
extern const struct NORFLASH_CFG_T p25q64sn_cfg;
extern const struct NORFLASH_CFG_T p25q32l_cfg;
extern const struct NORFLASH_CFG_T p25q32sn_cfg;
extern const struct NORFLASH_CFG_T p25q16l_cfg;
extern const struct NORFLASH_CFG_T p25q80h_cfg;
extern const struct NORFLASH_CFG_T p25q21h_cfg;
extern const struct NORFLASH_CFG_T p25q40h_cfg;

extern const struct NORFLASH_CFG_T p25q32sl_cfg;
extern const struct NORFLASH_CFG_T p25q16sl_cfg;

// XTS
extern const struct NORFLASH_CFG_T xt25q08b_cfg;

// XMC
extern const struct NORFLASH_CFG_T xm25qu256c_cfg;
extern const struct NORFLASH_CFG_T xm25qu128c_cfg;
extern const struct NORFLASH_CFG_T xm25lu64c_cfg;
extern const struct NORFLASH_CFG_T xm25qh16c_cfg;
extern const struct NORFLASH_CFG_T xm25qh80b_cfg;

// ZBIT
extern const struct NORFLASH_CFG_T zb25vq128b_cfg;
extern const struct NORFLASH_CFG_T zb25lq64a_cfg;

// EON
extern const struct NORFLASH_CFG_T en25s80b_cfg;

// WINBOND
extern const struct NORFLASH_CFG_T w25q32fw_cfg;
extern const struct NORFLASH_CFG_T w25q128jw_cfg;

// SiliconKaiser
extern const struct NORFLASH_CFG_T sk25le032_cfg;
extern const struct NORFLASH_CFG_T sk25le064_cfg;

static TEST_FLASH_INFO_T test_flash_info[] = {
 {&gd25le255e_cfg   ,   "GD25LE255E"       },
 {&gd25le128e_cfg   ,   "GD25LE128E"       },
 {&gd25uf128e_cfg   ,   "GD25UF128E"       },
 {&gd25lf128e_cfg   ,   "GD25LF128E"        },
 {&gd25q128e_cfg    ,   "GD25Q128E"       },
 {&gd25lq64c_cfg    ,   "GD25LQ64C"        },
 {&gd25uf64e_cfg    ,   "GD25UF64E"        },
 {&gd25lf64e_cfg    ,   "GD25LF64E"        },
 {&gd25lq32c_cfg    ,   "GD25LQ32C"        },
 {&gd25lf32e_cfg    ,   "GD25LF32E"        },
 {&gd25lq16c_cfg    ,   "GD25LQ16C"        },
 {&gd25lf16e_cfg    ,   "GD25LF16E"        },
 {&gd25lq80c_cfg    ,   "GD25LQ80C"        },
 {&gd25lf80e_cfg    ,   "GD25LF80E"        },
 {&gd25q32c_cfg     ,   "GD25Q32C"         },
 {&gd25q80c_cfg     ,   "GD25Q80C"         },
 {&gd25d40c_cfg     ,   "GD25D40C"         },
 {&gd25d20c_cfg     ,   "GD25D20C"         },
 {&p25q256l_cfg     ,   "P25Q256L"         },
 {&p25q128l_cfg     ,   "P25Q128L"         },
 {&py25q128laa_cfg   ,  "PY25Q128LAA"      },
 {&py25q128lac_cfg  ,   "PY25Q128LAC"      },
 {&p25q128sn_cfg    ,   "P25Q128SN"         },
 {&p25q64l_cfg      ,   "P25Q64L"          },
 {&p25q64ha_cfg      ,  "p25q64ha_cfg"     },
 {&p25q64sn_cfg     ,   "P25Q64SN"         },
 {&p25q32l_cfg      ,   "P25Q32L"          },
 {&p25q32sn_cfg     ,   "P25Q32SN"         },
 {&p25q16l_cfg      ,   "P25Q16L"          },
 {&p25q80h_cfg      ,   "P25Q80H"          },
 {&p25q21h_cfg      ,   "P25Q21H"          },
 {&p25q40h_cfg      ,   "P25Q40H"          },
 {&p25q32sl_cfg     ,   "P25Q32SL"         },
 {&p25q16sl_cfg     ,   "P25Q16SL"         },
 {&xt25q08b_cfg     ,   "XT25Q08B"         },
 {&xm25qu256c_cfg   ,   "XM25QU256C"       },
 {&xm25qu128c_cfg   ,   "XM25QU128C"       },
 {&xm25lu64c_cfg   ,    "XM25LU64C"       },
 {&xm25qh16c_cfg    ,   "XM25QH16C"        },
 {&xm25qh80b_cfg    ,   "XM25QH80B"        },
 {&zb25vq128b_cfg   ,   "ZB25VQ128B"       },
 {&zb25lq64a_cfg    ,   "ZB25LQ64A"       },
 {&en25s80b_cfg     ,   "EN25S80B"         },
 {&w25q32fw_cfg     ,   "W25Q32FW"         },
 {&w25q128jw_cfg    ,   "W25Q128JW"        },
 {&sk25le064_cfg    ,   "SK25LE064"        },
 {&sk25le032_cfg    ,   "SK25LE032"        },
};
static void _print_flash_test_environment(enum HAL_FLASH_ID_T id)
{
    uint32_t ind = 0xffff;
    const uint8_t *tmp_id;
     uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
#ifdef FLASH_DUAL_CHIP
    uint8_t flash_id1[HAL_NORFLASH_DEVICE_ID_LEN];
#endif
    uint32_t cache_base_addr;
    uint32_t no_cache_base_addr;

    TRACE(1, "\n--------FLASH TEST ENVIRONMENT----------\n");
#ifdef FLASH_DUAL_CHIP
    if(FLASH_IS_DUAL_CHIP(id))
    {
        hal_norflash_dual_chip_get_id(HAL_FLASH_ID_0, flash_id, flash_id1, ARRAY_SIZE(flash_id));
    }
    else
#endif
    {
        hal_norflash_get_id(id, flash_id, ARRAY_SIZE(flash_id));
    }

    for(uint32_t i = 0; i < sizeof(test_flash_info)/sizeof(test_flash_info[0]); i++)
    {
        tmp_id = (uint8_t*)(test_flash_info[i].cfg->id);

        if(tmp_id[0] == flash_id[0] &&
           tmp_id[1] == flash_id[1] &&
           tmp_id[2] == flash_id[2] )
        {
            ind = i;
        }
    }
    if(ind == 0xffff)
    {
        TRACE(1, "\nUndefined flash id!!!\n");
        return;
    }


#ifdef FLASH_UNIQUE_ID
    _read_uinque_id(id);
#endif

    TRACE(1, "CHIP:               %s",TO_STR(CHIP));
    TRACE(1, "CPU FREQ:           %u", hal_sys_timer_calc_cpu_freq(5, 0));
#ifdef FLASH_DUAL_CHIP
    if(FLASH_IS_DUAL_CHIP(id))
    {
        TRACE(1, "FLASH_ID:           %02X-%02X-%02X / %02X-%02X-%02X",
                 flash_id[0], flash_id[1], flash_id[2], flash_id1[0], flash_id1[1], flash_id1[2]);
    }
    else
#endif
    {
        TRACE(1, "FLASH_ID:           %02X-%02X-%02X", flash_id[0], flash_id[1], flash_id[2]);
    }
#ifdef FLASH_UNIQUE_ID
    TRACE(1, "FLASH_UNIQUE_ID:  %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",
                flash_unique_id[0],
                flash_unique_id[1],
                flash_unique_id[2],
                flash_unique_id[3],
                flash_unique_id[4],
                flash_unique_id[5],
                flash_unique_id[6],
                flash_unique_id[7],
                flash_unique_id[8],
                flash_unique_id[9],
                flash_unique_id[10],
                flash_unique_id[11],
                flash_unique_id[12],
                flash_unique_id[13],
                flash_unique_id[14],
                flash_unique_id[15]);
#ifdef FLASH_DUAL_CHIP
    if(FLASH_IS_DUAL_CHIP(id))
    {
        TRACE(1, "FLASH1_UNIQUE_ID: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",
                flash1_unique_id[0],
                flash1_unique_id[1],
                flash1_unique_id[2],
                flash1_unique_id[3],
                flash1_unique_id[4],
                flash1_unique_id[5],
                flash1_unique_id[6],
                flash1_unique_id[7],
                flash1_unique_id[8],
                flash1_unique_id[9],
                flash1_unique_id[10],
                flash1_unique_id[11],
                flash1_unique_id[12],
                flash1_unique_id[13],
                flash1_unique_id[14],
                flash1_unique_id[15]);
    }
#endif
#endif
    TRACE(1, "FLASH_MODEL:        %s", test_flash_info[ind].model_name);
    const struct HAL_NORFLASH_CONFIG_T *cfg = hal_norflash_get_init_config();
    if((cfg->mode & test_flash_info[ind].cfg->mode) & HAL_NORFLASH_OP_MODE_DTR)
    {
#ifdef FLASH_DUAL_CHIP
        if(FLASH_IS_DUAL_CHIP(id))
        {
            TRACE(1, "FLASH_R/W MODE:     8 wire, DTR");
        }
        else
#endif
        {
            TRACE(1, "FLASH_R/W MODE:     4 wire, DTR");
        }
    }
    else
    {
#ifdef FLASH_DUAL_CHIP
        if(FLASH_IS_DUAL_CHIP(id))
        {
            TRACE(1, "FLASH_R/W MODE:     8 wire, SDR");
        }
        else
#endif
        {
            TRACE(1, "FLASH_R/W MODE:     4 wire, SDR");
        }
    }

    cache_base_addr = _get_base_addr(id, true);
    no_cache_base_addr = _get_base_addr(id, false);
    TRACE(1, "FLASH_BASE_ADDR CACHE:    0x%x", cache_base_addr);
    TRACE(1, "FLASH_BASE_ADDR NO CACHE: 0x%x", no_cache_base_addr);

    hal_norflash_show_calib_result(id);

    TRACE(1, "\n------------------------------\n");
    return;

}

static void flash_test_result_init(void)
{
    uint32_t i;
    for(i = 0; i < sizeof(flash_test_result)/sizeof(flash_test_result[0]); i++)
    {
        flash_test_result[i].protection_pass = false;
        flash_test_result[i].suspend_pass = false;
        flash_test_result[i].norflash_api_pass = false;
        flash_test_result[i].security_register_pass = false;
        flash_test_result[i].base_func_pass = false;
        flash_test_result[i].erase_pass = false;
        flash_test_result[i].write_pass = false;
        flash_test_result[i].read_pass = false;
        flash_test_result[i].set_dual_chip_pass = false;
        flash_test_result[i].random_write_pass = false;
        flash_test_result[i].read_speed_cache = 0;
        flash_test_result[i].read_speed_no_cache = 0;
        flash_test_result[i].chip_erase_time = 0;
        flash_test_result[i].erase_speed_block = 0;
        flash_test_result[i].erase_speed_sector = 0;
        flash_test_result[i].erase_speed_page = 0;
        flash_test_result[i].write_speed = 0;
    }

    for(i = 0; i < sizeof(flash_test_case_switch)/sizeof(flash_test_case_switch[0]); i++)
    {
        flash_test_case_switch[i] = 0;

#ifdef FLASH_CACHE_TEST
        flash_test_case_switch[i] |= FLASH_CACHE_TEST_ON;
#endif

#ifdef FLASH_PROTECTION_TEST
        flash_test_case_switch[i] |= FLASH_PROTECTION_TEST_ON;
#endif

#ifdef FLASH_SUSPEND_TEST
        flash_test_case_switch[i] |= FLASH_SUSPEND_TEST_ON;
#endif

#ifdef FLASH_BASE_FUNCTION_TEST
        flash_test_case_switch[i] |= FLASH_BASE_FUNCTION_TEST_ON;
#endif

#ifdef FLASH_NORFLASH_API_TEST
        flash_test_case_switch[i] |= FLASH_NORFLASH_API_TEST_ON;
#endif

#ifdef FLASH_SECURITY_REGISTER
        flash_test_case_switch[i] |= FLASH_SECURITY_REG_TEST_ON;
#endif

#ifdef FLASH_READ_SPEED
        flash_test_case_switch[i] |= FLASH_READ_SPEED_ON;
#endif

#ifdef FLASH_SET_DUAL_CHIP_TEST
        flash_test_case_switch[i] |= FLASH_SET_DUAL_CHIP_TEST_ON;
#endif

#ifdef FLASH_LIFE_TEST
        flash_test_case_switch[i] |= FLASH_LIFE_TEST_ON;
#endif

#ifdef FLASH_RANDOM_WRITE_TEST
        flash_test_case_switch[i] |= FLASH_RANDOM_WRITE_TEST_ON;
#endif

    }
}

#endif

// FLASH_TEST
POSSIBLY_UNUSED
static SRAM_TEXT_LOC void _flash_test_do(enum HAL_FLASH_ID_T id)
{
    TRACE(1,"%s, start, FLASH ID: %d", __func__, id);
    osDelay(1);
#ifdef FLASH_PROTECTION_TEST
    _flash_protection_test(id);
#endif

#ifdef FLASH_SUSPEND_TEST
    _flash_suspend_test(id);
#endif

#ifdef FLASH_BASE_FUNCTION_TEST
    _flash_base_function_test(id);
#endif

#ifdef FLASH_READ_SPEED
    _flash_read_speed_test(id);
#endif

#ifdef FLASH_NORFLASH_API_TEST
    _norflash_api_test(id);
#endif

#ifdef FLASH_SECURITY_REGISTER_TEST
    _flash_security_register_test(id);
#endif


    TRACE(1,"%s, FLASH ID: %d done", __func__, id);
    TRACE_FLUSH();
}

SRAM_TEXT_LOC void function_test_norflash(void)
{
    uint32_t i;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_9, HAL_CMU_FREQ_208M);
    TRACE_IMM(1, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
#if defined(FLASH1_CTRL_BASE) && defined(CHIP_BEST2003) && defined(USE_MULTI_FLASH)
    _init_flash1();
#endif
    norflash_api_init();
    flash_test_result_init();

#if defined(FLASH_ERASE_CURRENT_TEST) || defined(FLASH_PROGRAM_CURRENT_TEST) || defined(FLASH_READ_CURRENT_TEST) || defined(FLASH_IDLE_CURRENT_TEST)
    _flash_current_test();
    while(1);
#endif

#ifdef FLASH_RANDOM_READ_TEST
    flash_random_read_test();
#endif

#ifdef FLASH_RANDOM_WRITE_TEST
    for(i = 0; i < HAL_FLASH_ID_NUM; i++)
    {
        uint32_t test_idx = 0;
        do {
            int ret = _flash_suspend_random_write(i);
            if(ret)
            {
                break;
            }
            test_idx ++;
            TRACE_IMM(1, "id %d FLASH SUSPEND RANDOM WRITE : %08d", i, test_idx);
        }while(test_idx < 10000);
    }
#endif

    for(i = 0; i < HAL_FLASH_ID_NUM; i++)
    {
#if !(defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH))
        if(i > HAL_FLASH_ID_0)
        {
            TRACE(1, "FLASH_%d macro USE_MULTI_FLASH not open--1!", i);
            continue;
        }
#endif
        if(!norflash_api_dev_opened((enum HAL_FLASH_ID_T)i))
        {
            TRACE(1, "FLASH_%d dev unopened!--2", i);
            continue;
        }
#ifdef FLASH_SET_DUAL_CHIP_TEST
        if(i == 0)
        {
            _flash_set_dual_chip_test(NORFLASH_API_MODULE_ID_TEST1);
        }
#endif

#ifdef FLASH_PROTECTION
        hal_norflash_disable_protection((enum HAL_FLASH_ID_T)i);
#endif
#ifdef FLASH_SIMPLE_TEST
        _flash_base_simple_test((enum HAL_FLASH_ID_T)i);
#else
        _flash_test_do((enum HAL_FLASH_ID_T)i);
#endif
    }

    for(i = 0; i < HAL_FLASH_ID_NUM; i++)
    {
#if !(defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH))
        if(i > HAL_FLASH_ID_0)
        {
            TRACE(1, "FLASH_%d macro USE_MULTI_FLASH not open--3!", i);
            continue;
        }
#endif
        if(!norflash_api_dev_opened((enum HAL_FLASH_ID_T)i))
        {
            TRACE(1, "FLASH_%d dev unopened!--4", i);
            continue;
        }
        _print_flash_test_environment((enum HAL_FLASH_ID_T)i);
        _print_flash_test_result((enum HAL_FLASH_ID_T)i , flash_test_case_switch[i], &flash_test_result[i]);
    }

#ifdef FLASH_LIFE_TEST
    for(i = 0; i < HAL_FLASH_ID_NUM; i++)
    {
#if !(defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH))
        if(i > HAL_FLASH_ID_0)
        {
            TRACE(1, "flash %d macro USE_MULTI_FLASH not open!--5", i);
            continue;
        }
#endif
        if(!norflash_api_dev_opened((enum HAL_FLASH_ID_T)i))
        {
            TRACE(1, "flash %d dev unopened!--6", i);
            continue;
        }

        norflash_life_test((enum HAL_FLASH_ID_T)i);
    }
#endif
    TRACE(1, "FLASH TEST DONE"); TRACE_FLUSH();
    while(1);
}

#endif // CHIP_FUNCTION_TEST_NORFLASH

