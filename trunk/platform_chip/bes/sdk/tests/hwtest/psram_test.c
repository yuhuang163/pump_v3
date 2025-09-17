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
#include "stdlib.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "plat_addr_map.h"
#ifdef PSRAM_PROD_TEST
#include "mem_test.h"
#else
#include "hwtest.h"
#endif

#ifdef PSRAM_PROD_TEST
#ifdef PSRAM_PROD_CACHE
#define PSRAM_CACHE_TEST                    1
#else
#define PSRAM_CACHE_TEST                    0
#endif

#else
#define PSRAM_CACHE_TEST                    1
#endif

#ifdef PSRAM_SLEEP_TEST
#define PSRAM_CHECK_TIME                    MS_TO_TICKS(150)
#define PSRAM_SLEEP_TIME                    MS_TO_TICKS(50)
#endif


void psram_current_test(void);

static uint32_t gen_index(uint8_t i)
{
    uint32_t index, offset;
    uint32_t j;

    j = 4;
    if (i <= j) {
        return i;
    }
    index = j;
    offset = 2;
    while (j < i) {
        index += offset;
        j++;
        if ((j & 1) == 0) {
            offset *= 2;
        }
    }

    return index;
}

static uint8_t gen_pattern_8(uint8_t pattern, uint8_t i)
{
    return pattern + i;
}

static uint16_t gen_pattern_16(uint16_t pattern, uint8_t i)
{
    return pattern + ((i << 0) | (i << 8));
}

static uint32_t gen_pattern_32(uint32_t pattern, uint8_t i)
{
    return pattern + ((i << 0) | (i << 8) | (i << 16) | (i << 24));
}

int psram_cache_rw8_test(uint32_t cached_addr, uint32_t uncached_addr, enum HAL_CACHE_ID_T id)
{
    volatile uint8_t *c_addr = (volatile uint8_t *)cached_addr;
    volatile uint8_t *nc_addr = (volatile uint8_t *)uncached_addr;
    int i;
    const int word_cnt = 16;
    const uint8_t cached_pattern = 0x5a;
    const uint8_t uncached_pattern = 0xe3;
    volatile uint8_t *test_addr = NULL;
    uint32_t test_val = 0;
    uint32_t expect_val = 0;
    uint32_t line;

    if (PSRAM_CACHE_TEST) {
        for (i = 0; i < word_cnt; i++) {
            c_addr[gen_index(i)] = gen_pattern_8(cached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_8(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)];
        }

#ifdef __ARM_ARCH_ISA_ARM
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)] = gen_pattern_8(uncached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_8(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
#endif
        hal_cache_sync_all(id);
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = c_addr[gen_index(i)];
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            expect_val = gen_pattern_8(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    for (i = 0; i < word_cnt; i++) {
        nc_addr[gen_index(i)] = gen_pattern_8(uncached_pattern, i);
        __DSB();
    }
    hal_psram_write_buffer_invalidate();
    for (i = 0; i < word_cnt; i++) {
        test_addr = &nc_addr[gen_index(i)];
        test_val = *test_addr;
        expect_val = gen_pattern_8(uncached_pattern, i);
        //TRACE_IMM(0, "1:rw8/check: [%p] 0x%08X vs 0x%08X", test_addr, test_val, expect_val);
        if (test_val != expect_val) {
            line = __LINE__;
                goto _err;
        }
        if (PSRAM_CACHE_TEST) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_8(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    if (PSRAM_CACHE_TEST) {
        hal_cache_invalidate(id, (uint32_t)&c_addr[gen_index(0)], (gen_index(word_cnt - 1) + 1) * sizeof(c_addr[0]));
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_8(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    return 0;

_err:
    TRACE_IMM(0, "%s:Error@line %u: [%p] 0x%08X != 0x%08X", __func__, line, test_addr, test_val, expect_val);
    hal_cmu_simu_fail();
    return 1;
}

int psram_cache_rw16_test(uint32_t cached_addr, uint32_t uncached_addr, enum HAL_CACHE_ID_T id)
{
    volatile uint16_t *c_addr = (volatile uint16_t *)cached_addr;
    volatile uint16_t *nc_addr = (volatile uint16_t *)uncached_addr;
    int i;
    const int word_cnt = 16;
    const uint16_t cached_pattern = 0xbe57;
    const uint16_t uncached_pattern = (uint16_t)~0xbe57;
    volatile uint16_t *test_addr = NULL;
    uint32_t test_val = 0;
    uint32_t expect_val = 0;
    uint32_t line;

    if (PSRAM_CACHE_TEST) {
        for (i = 0; i < word_cnt; i++) {
            c_addr[gen_index(i)] = gen_pattern_16(cached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_16(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)];
        }

#ifdef __ARM_ARCH_ISA_ARM
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)] = gen_pattern_16(uncached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_16(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
#endif
        hal_cache_sync_all(id);
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = c_addr[gen_index(i)];
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            expect_val = gen_pattern_16(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    for (i = 0; i < word_cnt; i++) {
        nc_addr[gen_index(i)] = gen_pattern_16(uncached_pattern, i);
        __DSB();
    }
    hal_psram_write_buffer_invalidate();
    for (i = 0; i < word_cnt; i++) {
        test_addr = &nc_addr[gen_index(i)];
        test_val = *test_addr;
        expect_val = gen_pattern_16(uncached_pattern, i);
        //TRACE_IMM(0, "1:rw16/check: [%p] 0x%08X vs 0x%08X", test_addr, test_val, expect_val);
        if (test_val != expect_val) {
            line = __LINE__;
                goto _err;
        }
        if (PSRAM_CACHE_TEST) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_16(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    if (PSRAM_CACHE_TEST) {
        hal_cache_invalidate(id, (uint32_t)&c_addr[gen_index(0)], (gen_index(word_cnt - 1) + 1) * sizeof(c_addr[0]));
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_16(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    return 0;

_err:
    TRACE_IMM(0, "%s:Error@line %u: [%p] 0x%08X != 0x%08X", __func__, line, test_addr, test_val, expect_val);
    hal_cmu_simu_fail();
    return 1;
}

int psram_cache_rw32_test(uint32_t cached_addr, uint32_t uncached_addr, enum HAL_CACHE_ID_T id)
{
    volatile uint32_t *c_addr = (volatile uint32_t *)cached_addr;
    volatile uint32_t *nc_addr = (volatile uint32_t *)uncached_addr;
    int i;
    const int word_cnt = 16;
    const uint32_t cached_pattern = 0x12345678;
    const uint32_t uncached_pattern = 0xabcdef01;
    volatile uint32_t *test_addr = NULL;
    uint32_t test_val = 0;
    uint32_t expect_val = 0;
    uint32_t line;

    if (PSRAM_CACHE_TEST) {
        for (i = 0; i < word_cnt; i++) {
            c_addr[gen_index(i)] = gen_pattern_32(cached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_32(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)];
        }

#ifdef __ARM_ARCH_ISA_ARM
        for (i = 0; i < word_cnt; i++) {
            nc_addr[gen_index(i)] = gen_pattern_32(uncached_pattern, i);
            __DSB();
        }
        hal_psram_write_buffer_invalidate();
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_32(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
#endif
        hal_cache_sync_all(id);
        for (i = 0; i < word_cnt; i++) {
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = c_addr[gen_index(i)];
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            expect_val = gen_pattern_32(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    for (i = 0; i < word_cnt; i++) {
        nc_addr[gen_index(i)] = gen_pattern_32(uncached_pattern, i);
        __DSB();
    }
    hal_psram_write_buffer_invalidate();
    for (i = 0; i < word_cnt; i++) {
        test_addr = &nc_addr[gen_index(i)];
        test_val = *test_addr;
        expect_val = gen_pattern_32(uncached_pattern, i);
        //TRACE_IMM(0, "rw32/check: [%p] 0x%08X vs 0x%08X", test_addr, test_val, expect_val);
        if (test_val != expect_val) {
            line = __LINE__;
                goto _err;
        }
        if (PSRAM_CACHE_TEST) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_32(cached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    if (PSRAM_CACHE_TEST) {
        hal_cache_invalidate(id, (uint32_t)&c_addr[gen_index(0)], (gen_index(word_cnt - 1) + 1) * sizeof(c_addr[0]));
        for (i = 0; i < word_cnt; i++) {
            test_addr = &c_addr[gen_index(i)];
            test_val = *test_addr;
            expect_val = gen_pattern_32(uncached_pattern, i);
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
            test_addr = &nc_addr[gen_index(i)];
            test_val = *test_addr;
            if (test_val != expect_val) {
                line = __LINE__;
                goto _err;
            }
        }
    }

    return 0;

_err:
    TRACE_IMM(0, "%s:Error@line %u: [%p] 0x%08X != 0x%08X", __func__, line, test_addr, test_val, expect_val);
    hal_cmu_simu_fail();
    return 1;
}

#ifdef PSRAM_BASE

//#1  PSRAM_TEST
#define PSRAM_CODE_C_MEM_START          (PSRAMX_BASE)
#define PSRAM_CODE_C_MEM_END            (PSRAMX_BASE + (1024) * 1024)

#define PSRAM_CODE_NC_MEM_START         (PSRAMX_NC_BASE)
#define PSRAM_CODE_NC_MEM_END           (PSRAMX_NC_BASE + (1024) * 1024)

#define PSRAM_C_MEM_START               (PSRAM_BASE)
#define PSRAM_C_MEM_END                 (PSRAM_BASE + (1024) * 1024)

#define PSRAM_NC_MEM_START              (PSRAM_NC_BASE)
#define PSRAM_NC_MEM_END                (PSRAM_NC_BASE + (1024) * 1024)

typedef struct _MEM_TEST_T
{
    char *name;
    uint32_t start;
    uint32_t end;
} MEM_TEST_T;

static const MEM_TEST_T mem_banks_test[] = {
    {"psram cache",PSRAM_C_MEM_START, PSRAM_C_MEM_END},
    {"psram non-cache",PSRAM_NC_MEM_START, PSRAM_NC_MEM_END},
    {"psram code cache",PSRAM_CODE_C_MEM_START, PSRAM_CODE_C_MEM_END},
    {"psram code non-cache",PSRAM_CODE_NC_MEM_START, PSRAM_CODE_NC_MEM_END},
};

static const uint32_t mem_banks_test_pt[] =
{
    0xAAAAAAAA,
    0x55555555,
    0x12345678,
    0xA5A5A5A5,
    0x5A5A5A5A,
    0xFEFEFEFE,
    0xEFFEEFFE,
    0x00000000,
    0xFFFFFFFF,
    0x11111111,
};

int psram_mem_range_test(uint32_t *start, uint32_t *end, uint32_t pattern, uint32_t inc, bool continous)
{
    volatile uint8_t *dst;
    uint8_t val;
    int err = 0;

    TRACE_IMM(0, "start=%p end=%p pattern=0x%08X inc=%u", start, end, pattern, inc);
    dst = (uint8_t *)start;
    val= pattern & 0xff;

    for (; dst < (uint8_t *)end; dst++) {
        *dst = val;
        if (*dst != val) {
            TRACE(3,"Error: addr=0x%08x val=0x%08x expect=0x%08x", (uint32_t)dst, *dst, val);
            //hal_cmu_simu_fail();
            //while (1);
            err++;
            if (!continous) {
                break;
            }
        }
        else {
//            TRACE(3,"addr=0x%08x val=0x%08x expect=0x%08x", (uint32_t)dst, *dst, val);
        }
    }
    return err;
}

int psram_simple_test(void)
{
    uint32_t i, j;
    int err = 0;

    TRACE(0,"%s", __func__);

    for (j = 0; j < ARRAY_SIZE(mem_banks_test); j++) {
        for (i = 0; i < ARRAY_SIZE(mem_banks_test_pt); i++) {
            uint32_t start_addr, end_addr, patt;
            const char *name;
            uint32_t time;

            name = mem_banks_test[j].name;
            start_addr = mem_banks_test[j].start;
            end_addr = mem_banks_test[j].end;
            patt = mem_banks_test_pt[i];

            TRACE(0, "block[%d]: %s, [%08X - %08X], patt=%08X", j, name, start_addr, end_addr, patt);
            time = hal_sys_timer_get();
            err = psram_mem_range_test((uint32_t *)start_addr, (uint32_t *)end_addr, patt, 0, false);
            time = TICKS_TO_MS(hal_sys_timer_get() - time);
            if (err) {
                TRACE(0, "block[%d] test error=%d, time=%d ms", j, err, time);
                goto _exit;
            } else {
                TRACE(0, "block[%d]: test done, time=%d ms", j, time);
            }
        }
    }

_exit:
    if (!err) {
        TRACE(0, "PSRAM SIMPLE TEST SUCCESS !!!");
    } else {
        TRACE(0, "PSRAM SIMPLE TEST FAILED %d", err);
    }
    return err;
}

#define TEST_BAND 4
#define PSRAM_BAND_LEN 0x8000
#define SRAM_CALI_BUFF_LEN (TEST_BAND*PSRAM_BAND_LEN)
//#ifndef __ARM_ARCH_ISA_ARM
//__attribute__((section(".psram_test_buf")))
//#endif
static uint8_t cali_buf[SRAM_CALI_BUFF_LEN];
//static uint8_t *cali_buf = (uint8_t *)0x20040000;
static uint32_t cali_val[TEST_BAND];
/*
 ************************************************************************************************************
 * psram_addr0 (len:PSRAM_BAND_LEN*2)
 * |low        | high      |
 ************************************************************************************************************
 *                          psram_addr1(len:PSRAM_BAND_LEN*2)
 *                         |low        | high      |
 ************************************************************************************************************
 *                                                  psram_addr2(len:PSRAM_BAND_LEN*2)
 *                                                 |low        | high      |
 ************************************************************************************************************
 *                                                                          psram_addr3(len:PSRAM_BAND_LEN*2)
 *                                                                         |low        | high      |
 ************************************************************************************************************
 * sram_addr (len:SRAM_CALI_BUFF_LEN)
 * |cali psram0|cali psram1|cali psram2|cali psram3|
 ************************************************************************************************************
 * 1. check low region with high region and sram region
 * 2. check lower 16bits with higher 16bits for each word
 */

int psram_rand_test(uint32_t psram_addr0, uint32_t psram_addr1, uint32_t psram_addr2, uint32_t psram_addr3)
{
    static uint32_t total_cnt = 0;
    uint32_t print_time = 0, now_time = 0;
    uint32_t cnt = 0;
    uint32_t addr_arry[TEST_BAND];
    volatile uint32_t *addr_h, *addr_l, *addr_cali;
    bool init_cali = true;
#ifdef PSRAM_SLEEP_TEST
#ifndef PSRAM_ENABLE
#error "PSRAM_SLEEP_TEST requires PSRAM_ENABLE"
#endif
    static bool init_done = false;
    uint32_t start_time;

    if (init_done) {
        init_cali = false;
    }
    init_done = true;
#endif

    addr_arry[0] = psram_addr0;
    addr_arry[1] = psram_addr1;
    addr_arry[2] = psram_addr2;
    addr_arry[3] = psram_addr3;

    if (init_cali) {
        for (uint32_t i=0; i<TEST_BAND; ++i) {
            cali_val[i] = (uint32_t)(&cali_buf[i*PSRAM_BAND_LEN]);
            TRACE_IMM(3,"TEST region %d, 0x%x-0x%x", i, addr_arry[i], addr_arry[i]+PSRAM_BAND_LEN);
            TRACE_IMM(3,"TEST region %d, 0x%x-0x%x", i, addr_arry[i]+PSRAM_BAND_LEN, addr_arry[i]+PSRAM_BAND_LEN+PSRAM_BAND_LEN);
            for (uint32_t j=0; j<PSRAM_BAND_LEN; j+=4) {
                *(volatile uint32_t *)(addr_arry[i]+j) = 0xABAC5453;
                *(volatile uint32_t *)(addr_arry[i]+j+PSRAM_BAND_LEN) = 0xABAC5453;
                *(volatile uint32_t *)(cali_val[i]+j) =  0xABAC5453;
            }
        }
        __DSB();
    }

    print_time = hal_sys_timer_get();
#ifdef PSRAM_SLEEP_TEST
    start_time = print_time;
#endif

    while (
#ifdef PSRAM_SLEEP_TEST
            hal_sys_timer_get() - start_time <= PSRAM_CHECK_TIME &&
#endif
            1) {
        total_cnt++;
        cnt++;
        uint32_t arry_index = rand();
        uint32_t addr_offset = rand();
        //TRACE(2,"addy_index:0x%x, addr_offset:0x%x", arry_index, addr_offset);
        addr_cali = (volatile uint32_t *)(cali_val[arry_index%TEST_BAND] + ((addr_offset<<2)&(PSRAM_BAND_LEN-1)));// addr is 4Byte aligne
        addr_l = (volatile uint32_t *)(addr_arry[arry_index%TEST_BAND] + ((addr_offset<<2)&(PSRAM_BAND_LEN-1)));// addr is 4Byte aligned
        addr_h = addr_l + PSRAM_BAND_LEN/sizeof(uint32_t);
        if (rand() & 0x1) {//write
            //TRACE_IMM(3,"WRITE add_h:%p, addr_l:%p, add_cali:%p", addr_h, addr_l, addr_cali);
            uint32_t temp_val;
            temp_val = rand() & 0xffff;
            temp_val = ((~(temp_val)) << 16) | (temp_val);
            *addr_cali = *addr_h = *addr_l = temp_val;
            __DSB();
            //TRACE_IMM(4,"write addr %p, %p, %p, val:0x%x", addr_h, addr_l, addr_cali, temp_val);
        } else { //read & check
            //TRACE_IMM(3,"READ add_h:%p, addr_l:%p, add_cali:%p", addr_h, addr_l, addr_cali);
            uint32_t v_l, v_h, v_c;
            v_l = *addr_l;
            v_h = *addr_h;
            v_c = *addr_cali;
            if (v_l == v_h && v_l == v_c) {
                if ((~v_l)>>16 == (v_l&0xffff)) {
                    now_time = hal_sys_timer_get();
                    if ((now_time - print_time) >= MS_TO_TICKS(100)) {
                        TRACE_IMM(4,"check ok. addr %p, %p, %p, val:0x%x (cnt=%u/%u)",
                            addr_h, addr_l, addr_cali, *addr_h, cnt, total_cnt);
                        print_time = now_time;
                        cnt = 0;
                    }
                    continue;
                }
            }
            TRACE_IMM(6,"check fail. addr %p, %p, %p, val:0x%x 0x%x 0x%x",
                addr_h, addr_l, addr_cali, v_h, v_l, v_c);
            TRACE_IMM(6,"check fail. addr %p, %p, %p, val:0x%x 0x%x 0x%x",
                addr_h, addr_l, addr_cali, *addr_h, *addr_l, *addr_cali);
            TRACE_IMM(0, "(cnt=%u/%u)", cnt, total_cnt);
            return 1;
        }
    }
    return 0;
}

static void psram_rand_test_loop(void)
{
    uint32_t time;
    int err;

#ifdef PSRAM_SLEEP_TEST
    static uint32_t last_test_time;
    static bool init_done = false;
    static HWTIMER_ID timer_id = NULL;

    if (!init_done) {
        sleep_test_register_loop_callback(psram_rand_test_loop);
        timer_id = hwtimer_alloc(NULL, NULL);
        ASSERT(timer_id, "%s: Failed to alloc timer id", __func__);
    }
    init_done = true;

    if (hal_sys_timer_get() - last_test_time < PSRAM_SLEEP_TIME) {
        return;
    }
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_26M);
#endif

    time = hal_sys_timer_get();
    err = psram_rand_test(mem_banks_test[0].start, mem_banks_test[1].start+0x100000,
        mem_banks_test[2].start+0x200000, mem_banks_test[3].start+0x300000);
    TRACE(1,"  %u ms", TICKS_TO_MS(hal_sys_timer_get() - time));
    if (err) {
        TRACE(0, "PSRAM RAND TEST FAILED");
    } else {
        TRACE(0, "PSRAM RAND TEST SUCCESS");
    }

#ifdef PSRAM_SLEEP_TEST
    ASSERT(err == 0, "PSRAM RAND TEST FAILED");
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    TRACE_FLUSH();
    osDelay(2);
    last_test_time = hal_sys_timer_get();
    hwtimer_start(timer_id, PSRAM_SLEEP_TIME);
#endif
}

POSSIBLY_UNUSED static void psram_pll_init(void)
{
    // PLL will be enabled when setting mem freq for new chips
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_26M);
    hal_cmu_init_pll_selection();
}

static void psram_init(void)
{
#ifndef __ARM_ARCH_ISA_ARM
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif
#if defined(ROM_BUILD) || defined(PROGRAMMER) || !defined(PSRAM_ENABLE)
    psram_pll_init();
    hal_psram_init();
#endif
#endif
}

int psram_simu_test(void)
{
    int ret;
    enum HAL_CACHE_ID_T cache_id;

    FUNC_ENTRY_TRACE();

    cache_id = HAL_CACHE_ID_D_CACHE;

    ret = psram_cache_rw8_test(PSRAM_BASE, PSRAM_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }
    ret = psram_cache_rw16_test(PSRAM_BASE, PSRAM_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }
    ret = psram_cache_rw32_test(PSRAM_BASE, PSRAM_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }

#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    cache_id = HAL_CACHE_ID_I_CACHE;
#endif

    ret = psram_cache_rw8_test(PSRAMX_BASE, PSRAMX_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }
    ret = psram_cache_rw16_test(PSRAMX_BASE, PSRAMX_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }
    ret = psram_cache_rw32_test(PSRAMX_BASE, PSRAMX_NC_BASE, cache_id);
    if (ret) {
        goto _exit;
    }
#endif

_exit:
    if (ret) {
        hal_cmu_simu_fail();
        TRACE(0, "%s: failed", __func__);
    } else {
        hal_cmu_simu_pass();
        TRACE(0, "%s: passed", __func__);
    }
    return ret;
}

void psram_test(void)
{
    TRACE(0, "psram_test");

    psram_init();

#if defined(PSRAM_CURRENT_TEST)
    psram_current_test();
#endif

#if defined(PSRAM_SIMU_TEST)
    psram_simu_test();
#elif defined(PSRAM_SIMPLE_TEST) || (CHIP_PSRAM_CTRL_VER == 1)
    psram_simple_test();
#else
    psram_rand_test_loop();
#endif

    TRACE(0, "PSRAM TEST END");
}

#ifdef PSRAM_PROD_TEST
static void mem_test_psram_init_state(void)
{
    mem_test_set_module_state(MTEST_MODULE_SRAM, MTEST_CASE_PSRAM_RW32, MEM_TEST_FAILED);
#ifdef PSRAM_PROD_FULL
    mem_test_set_module_state(MTEST_MODULE_SRAM, MTEST_CASE_PSRAM_RW16, MEM_TEST_FAILED);
    mem_test_set_module_state(MTEST_MODULE_SRAM, MTEST_CASE_PSRAM_RW8,  MEM_TEST_FAILED);
#endif
}

static void psram_set_pass_state(int tc)
{
    mem_test_set_module_state(MTEST_MODULE_PSRAM, (enum MTEST_CASE_T)tc, MEM_TEST_PASSED);
}

static void psram_set_case_tag(int tc)
{
	mem_test_set_module_case_tag((enum MTEST_CASE_T)tc);
}

int psram_prod_test(void *arguments)
{
    int ret = 0;
    uint32_t time, time1;
    enum HAL_CACHE_ID_T dcache_id = HAL_CACHE_ID_D_CACHE;
#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
    enum HAL_CACHE_ID_T icache_id;
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    icache_id = HAL_CACHE_ID_I_CACHE;
#else
    icache_id = HAL_CACHE_ID_D_CACHE;
#endif
#endif

    TRACE(1, "%s: start", __func__);

    hal_cmu_simu_tag(MTEST_MODULE_PSRAM);
    mem_test_set_module_tag(MTEST_MODULE_PSRAM);
    mem_test_psram_init_state();

    psram_init();

    time = flash_hal_sys_timer_get();
    time1 = time;

    psram_set_case_tag(MTEST_CASE_PSRAM_RW32);
#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
    ret = psram_cache_rw32_test(PSRAMX_BASE, PSRAMX_NC_BASE, icache_id);
    TRACE(1,"RW32 TEST1: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
#endif

    time = flash_hal_sys_timer_get();
    ret = psram_cache_rw32_test(PSRAM_BASE, PSRAM_NC_BASE, dcache_id);
    TRACE(1,"RW32 TEST2: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
    psram_set_pass_state(MTEST_CASE_PSRAM_RW32);

#ifdef PSRAM_PROD_FULL
    psram_set_case_tag(MTEST_CASE_PSRAM_RW16);
    time = flash_hal_sys_timer_get();
    ret = psram_cache_rw16_test(PSRAM_BASE, PSRAM_NC_BASE, dcache_id);
    TRACE(1,"RW16 TEST1: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
    time = flash_hal_sys_timer_get();
    ret = psram_cache_rw16_test(PSRAMX_BASE, PSRAMX_NC_BASE, icache_id);
    TRACE(1,"RW16 TEST2: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
#endif
    psram_set_pass_state(MTEST_CASE_PSRAM_RW16);

    psram_set_case_tag(MTEST_CASE_PSRAM_RW8);
    time = flash_hal_sys_timer_get();
    ret = psram_cache_rw8_test(PSRAM_BASE, PSRAM_NC_BASE, dcache_id);
    TRACE(1,"RW8 TEST1: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
    time = flash_hal_sys_timer_get();
    ret = psram_cache_rw8_test(PSRAMX_BASE, PSRAMX_NC_BASE, icache_id);
    TRACE(1,"RW8 TEST2: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time));
    if (ret) {
        goto _exit;
    }
#endif
    psram_set_pass_state(MTEST_CASE_PSRAM_RW8);
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_104M);
_exit:
    TRACE(1,"PSRAM COST TIME: %5u us", TICKS_TO_US(flash_hal_sys_timer_get() - time1));
    if (ret) {
        TRACE(1,"========> PSRAM TEST ERROR ###: %d", ret);
        hal_cmu_simu_fail();
        mem_test_save_error2(ret, 0, MTEST_ERRCODE_PSRAM, 0);
        mem_test_update_info();
        test_exit();
    } else {
        TRACE(1, "========> PSRAM TEST SUCCESS !!");
    }
    return 0;
}

MTEST_CASE_DEF(psram_test_inst,
    "PSRAM_PRODUCTION_TEST",
    MTEST_MODULE_PSRAM,
    MTEST_CASE_PSRAM_RW32,
    MTEST_ERRCODE_PSRAM,
    psram_prod_test,
    NULL,
    NULL);

#endif /* PSRAM_PROD_TEST */

#endif

#ifdef PSRAMUHS_BASE
static const MEM_TEST_T psramuhs_banks_test[] = {
    {"psramuhs cache",PSRAMUHS_BASE, PSRAMUHS_BASE + 0x100000},
    {"psramuhs non-cache",PSRAMUHS_NC_BASE, PSRAMUHS_NC_BASE + 0x100000},
};
#define PU_TEST_BAND 2
#define PU_PSRAM_BAND_LEN 0x10000
#define PU_SRAM_CALI_BUFF_LEN (PU_TEST_BAND*PU_PSRAM_BAND_LEN)
//#ifndef __ARM_ARCH_ISA_ARM
//__attribute__((section(".psramuhs_test_buf")))
//#endif
static uint8_t pu_cali_buf[PU_SRAM_CALI_BUFF_LEN];
static uint32_t pu_cali_val[PU_TEST_BAND];
//__attribute__((section(".psramuhs_text")))
/*
 ************************************************************************************************************
 * psram_addr0 (len:PSRAM_BAND_LEN*2)
 * |low        | high      |
 ************************************************************************************************************
 *                          psram_addr1(len:PSRAM_BAND_LEN*2)
 *                         |low        | high      |
 ************************************************************************************************************
 * sram_addr (len:SRAM_CALI_BUFF_LEN)
 * |cali psram0|cali psram1|
 ************************************************************************************************************
 * 1. check low region with high region and sram region
 * 2. check lower 16bits with higher 16bits for each word
 */
void psramuhs_rand_test(uint32_t psram_addr0, uint32_t psram_addr1)
{
    static uint32_t total_cnt = 0;
    uint32_t print_time = 0, now_time = 0;
    uint32_t cnt = 0;
    uint32_t addr_arry[PU_TEST_BAND];
    volatile uint32_t *addr_h, *addr_l, *addr_cali;
    addr_arry[0] = psram_addr0;
    addr_arry[1] = psram_addr1;
    for (uint32_t i=0; i<PU_TEST_BAND; ++i) {
        pu_cali_val[i] = (uint32_t)(&pu_cali_buf[i*PU_PSRAM_BAND_LEN]);
        TRACE_IMM(3,"TEST region %d, 0x%x-0x%x", i, addr_arry[i], addr_arry[i]+PU_PSRAM_BAND_LEN);
        TRACE_IMM(3,"TEST region %d, 0x%x-0x%x", i, addr_arry[i]+PU_PSRAM_BAND_LEN, addr_arry[i]+2*PU_PSRAM_BAND_LEN);
        for (uint32_t j=0; j<PU_PSRAM_BAND_LEN; j+=4) {
            //TRACE(1,"write:0x%x", addr_arry[i]+j, addr_arry[i]+j+PU_PSRAM_BAND_LEN);
            *(volatile uint32_t *)(addr_arry[i]+j) = 0xABAB5454;
            *(volatile uint32_t *)(addr_arry[i]+j+PU_PSRAM_BAND_LEN) = 0xABAB5454;
            //TRACE(1,"write:0x%x", pu_cali_val[i]+j);
            *(volatile uint32_t *)(pu_cali_val[i]+j) =  0xABAB5454;
        }
    }

    print_time = hal_sys_timer_get();

    while (1) {
        total_cnt++;
        cnt++;
        uint32_t arry_index = rand();
        uint32_t addr_offset = rand();
        //TRACE(2,"addy_index:0x%x, addr_offset:0x%x", arry_index, addr_offset);
        addr_cali = (volatile uint32_t *)(pu_cali_val[arry_index%PU_TEST_BAND] + ((addr_offset<<2)&(PU_PSRAM_BAND_LEN-1)));// addr is 4Byte aligne
        addr_l = (volatile uint32_t *)(addr_arry[arry_index%PU_TEST_BAND] + ((addr_offset<<2)&(PU_PSRAM_BAND_LEN-1)));// addr is 4Byte aligned
        addr_h = addr_l + PU_PSRAM_BAND_LEN/sizeof(uint32_t);
        if (rand() & 0x1) {//write
            //TRACE(3,"WRITE add_h:%p, addr_l:%p, add_cali:%p", addr_h, addr_l, addr_cali);
            uint32_t temp_val;
            temp_val = rand() & 0xffff;
            temp_val = ((~(temp_val)) << 16) | (temp_val);
            *addr_cali = *addr_h = *addr_l = temp_val;
            //TRACE_IMM(4,"write addr %p, %p, %p, val:0x%x", addr_h, addr_l, addr_cali, temp_val);
        } else { //read & check
            //TRACE(3,"READ add_h:%p, addr_l:%p, add_cali:%p", addr_h, addr_l, addr_cali);
            uint32_t val_l, val_h, val_cali;
            val_l = *addr_l;
            val_h = *addr_h;
            val_cali = *addr_cali;
            if (val_l == val_h && val_h == val_cali) {
                if ((~(val_l))>>16 == ((val_l)&0xffff)) {
                    now_time = hal_sys_timer_get();
                    if ((now_time - print_time) >= MS_TO_TICKS(100)) {
                        TRACE(0, "check ok. addr 0x%x, 0x%x, 0x%x, val:0x%x (cnt=%u/%u)",
                            (uint32_t)addr_h, (uint32_t)addr_l, (uint32_t)addr_cali, val_h, cnt, total_cnt);
                        print_time = now_time;
                        cnt = 0;
                    }
                    continue;
                }
            }
            TRACE(6,"check fail. addr %p, %p, %p, val:0x%x 0x%x 0x%x",
                addr_h, addr_l, addr_cali, *addr_h, *addr_l, *addr_cali);
            TRACE(0, "(cnt=%u/%u)", cnt, total_cnt);
            break;
        }
    }
}

static void psramuhs_init(void)
{
#ifndef __ARM_ARCH_ISA_ARM
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif

#if defined(ROM_BUILD) || defined(PROGRAMMER) || !defined(PSRAMUHS_ENABLE)
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();

    hal_psramuhs_init();
#endif
#endif
}

void psramuhs_simu_test(void)
{
    int ret;
    enum HAL_CACHE_ID_T cache_id;

    cache_id = HAL_CACHE_ID_D_CACHE;

    ret = psram_cache_rw8_test(PSRAMUHS_BASE, PSRAMUHS_NC_BASE, cache_id);
    if (ret) {
        return;
    }
    ret = psram_cache_rw16_test(PSRAMUHS_BASE, PSRAMUHS_NC_BASE, cache_id);
    if (ret) {
        return;
    }
    ret = psram_cache_rw32_test(PSRAMUHS_BASE, PSRAMUHS_NC_BASE, cache_id);
    if (ret) {
        return;
    }

#if defined(ICACHE_CTRL_BASE) && !defined(__ARM_ARCH_ISA_ARM)
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    cache_id = HAL_CACHE_ID_I_CACHE;
#endif

    ret = psram_cache_rw8_test(PSRAMUHSX_BASE, PSRAMUHSX_NC_BASE, cache_id);
    if (ret) {
        return;
    }
    ret = psram_cache_rw16_test(PSRAMUHSX_BASE, PSRAMUHSX_NC_BASE, cache_id);
    if (ret) {
        return;
    }
    ret = psram_cache_rw32_test(PSRAMUHSX_BASE, PSRAMUHSX_NC_BASE, cache_id);
    if (ret) {
        return;
    }
#endif

    hal_cmu_simu_pass();
    while (1) {}
}

void psramuhs_test(void)
{
    psramuhs_init();

#if defined(PSRAMUHS_SIMU_TEST)
    psramuhs_simu_test();
#else
    psramuhs_rand_test(psramuhs_banks_test[0].start, psramuhs_banks_test[1].start+0x100000);
#endif
}

#endif


#ifdef PSRAM_CURRENT_TEST
#include "hal_dma.h"
#include "hal_location.h"
#include <string.h>

#define DMA_TSIZE_PER_TRANSFER      0xFFC
#define PSRAM_DMA_BUF_SIZE     (1024*1024)

SYNC_FLAGS_LOC
static struct HAL_DMA_DESC_T mem_dma_desc[PSRAM_DMA_BUF_SIZE/DMA_TSIZE_PER_TRANSFER + 1];

SRAM_TEXT_LOC
static int mem_dma_init(struct HAL_DMA_CH_CFG_T *dma_cfg, void *dst, const void *src, size_t len, uint8_t loop)
{
    memset(dma_cfg, 0, sizeof(*dma_cfg));
    dma_cfg->dst_bsize  = HAL_DMA_BSIZE_16;
    dma_cfg->dst_periph = HAL_GPDMA_MEM;
    dma_cfg->dst_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg->handler    = NULL;
    dma_cfg->src_bsize  = HAL_DMA_BSIZE_16;
    dma_cfg->src_periph = HAL_GPDMA_MEM;
    dma_cfg->src_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg->type       = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg->try_burst  = false;
    dma_cfg->ch         = hal_dma_get_chan(dma_cfg->dst_periph, HAL_DMA_HIGH_PRIO);
    TRACE(0, "dma_cfg->ch:%d", dma_cfg->ch);
    ASSERT(dma_cfg->ch != HAL_DMA_CHAN_NONE, "%s: Failed to get dma chan", __func__);

    dma_cfg->dst = (uint32_t)dst;
    dma_cfg->src = (uint32_t)src;

    uint16_t i;
    uint16_t link_cnt;
    enum HAL_DMA_RET_T dret;
    uint32_t remain_transfer_size;

    remain_transfer_size = len / 4;
    if (remain_transfer_size > DMA_TSIZE_PER_TRANSFER) {
        if (remain_transfer_size % DMA_TSIZE_PER_TRANSFER) {
            link_cnt = remain_transfer_size / DMA_TSIZE_PER_TRANSFER + 1;
        } else {
            link_cnt = remain_transfer_size / DMA_TSIZE_PER_TRANSFER;
        }
    } else {
        link_cnt = 1;
    }

    for (i = 0; i < link_cnt; i++) {
        dma_cfg->dst = (uint32_t)(dst + DMA_TSIZE_PER_TRANSFER * 4 * i);
        if (remain_transfer_size > DMA_TSIZE_PER_TRANSFER) {
            dma_cfg->src_tsize = DMA_TSIZE_PER_TRANSFER;
            remain_transfer_size -= DMA_TSIZE_PER_TRANSFER;
        } else {
            dma_cfg->src_tsize = remain_transfer_size;
            remain_transfer_size = 0;
        }
        if (i + 1 == link_cnt) {
            dret = hal_dma_init_desc(&mem_dma_desc[i], dma_cfg, loop?&mem_dma_desc[0]:NULL, 0);
        } else {
            dret = hal_dma_init_desc(&mem_dma_desc[i], dma_cfg, &mem_dma_desc[i + 1], 0);
        }
        ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);
    }

    return 0;
}

#if 1
#include "hal_iomux.h"
#include "hal_gpio.h"
static const struct HAL_IOMUX_PIN_FUNCTION_MAP psram_gpio_iomux[] = {
    {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
};

void psram_gpio_set(int pin)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)psram_gpio_iomux[pin].pin);
}
void psram_gpio_clr(int pin)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)psram_gpio_iomux[pin].pin);
}
void psram_gpio_open(void)
{
    int i = 0;
    hal_iomux_init(psram_gpio_iomux, ARRAY_SIZE(psram_gpio_iomux));
    for (i = 0; i < ARRAY_SIZE(psram_gpio_iomux); ++i) {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)psram_gpio_iomux[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
}
#endif

SRAM_TEXT_LOC
static int mem_dma_copy_loop(void *dst, const void *src, size_t len)
{
    struct HAL_DMA_CH_CFG_T dma_cfg0;
    static int dma_init = false;

    if (!dma_init) {
        mem_dma_init(&dma_cfg0, dst, src, len, 1);
        dma_init = true;
    }
    hal_dma_sg_start(&mem_dma_desc[0], &dma_cfg0);
    // while (hal_dma_chan_busy(dma_cfg0.ch));
    //
    while (1) {
    	hal_sys_timer_delay(MS_TO_TICKS(500));
    }

    return 0;
}

SRAM_TEXT_LOC
static int mem_dma_copy_1hz(void *dst, const void *src, size_t len)
{
    struct HAL_DMA_CH_CFG_T dma_cfg0;
    static int dma_init = false;

_test_again:
    if (!dma_init) {
        mem_dma_init(&dma_cfg0, dst, src, len, 0);
        dma_init = true;
    }
    psram_gpio_set(0);
    hal_dma_sg_start(&mem_dma_desc[0], &dma_cfg0);
    // while (hal_dma_chan_busy(dma_cfg0.ch));
    //

    while (hal_dma_chan_busy(dma_cfg0.ch)) {
        hal_sys_timer_delay(MS_TO_TICKS(250));
    }
    psram_gpio_clr(0);
    hal_sys_timer_delay(MS_TO_TICKS(250));
    goto _test_again;

    return 0;
}

PSRAM_BSS_LOC unsigned char psram_src_buf[PSRAM_DMA_BUF_SIZE];
PSRAM_BSS_LOC unsigned char psram_dst_buf[PSRAM_DMA_BUF_SIZE];

static void psram_current_test_dma_rw_loop(void)
{
    TRACE_IMM(0, "%s", __func__);
    mem_dma_copy_loop(psram_dst_buf, psram_src_buf, PSRAM_DMA_BUF_SIZE);
}

static void psram_current_test_dma_rw_1hz(void)
{
    TRACE_IMM(0, "%s", __func__);
    mem_dma_copy_1hz(psram_dst_buf, psram_src_buf, PSRAM_DMA_BUF_SIZE);
}

void psram_current_test(void)
{
    psram_gpio_open();
#ifdef PSRAM_CURRENT_TEST_DMA_RW_LOOP
    psram_current_test_dma_rw_loop();
#elif defined(PSRAM_CURRENT_TEST_DMA_RW_1HZ)
    psram_current_test_dma_rw_1hz();
#endif
}
#endif /* PSRAM_CURRENT_TEST */
