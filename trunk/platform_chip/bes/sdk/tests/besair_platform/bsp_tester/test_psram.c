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
#ifdef TEST_PSRAM
#include "stdlib.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "plat_addr_map.h"
#include "hal_location.h"



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
}MEM_TEST_T;

static const MEM_TEST_T mem_banks_test[] = {
    {"psram cache",PSRAM_C_MEM_START, PSRAM_C_MEM_END},
    {"psram non-cache",PSRAM_NC_MEM_START, PSRAM_NC_MEM_END},
    {"psram code cache",PSRAM_CODE_C_MEM_START, PSRAM_CODE_C_MEM_END},
    {"psram code non-cache",PSRAM_CODE_NC_MEM_START, PSRAM_CODE_NC_MEM_END},
};


#define TEST_BAND 4
#define PSRAM_BAND_LEN 0x8000
#define SRAM_CALI_BUFF_LEN (TEST_BAND*PSRAM_BAND_LEN)
static uint8_t cali_buf[SRAM_CALI_BUFF_LEN];
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

void psram_rand_data_test(uint32_t psram_addr0, uint32_t psram_addr1, uint32_t psram_addr2, uint32_t psram_addr3)
{
    uint32_t print_time = 0, now_time = 0;
    uint32_t total_cnt = 0, cnt = 0;
    uint32_t addr_arry[TEST_BAND];
    volatile uint32_t *addr_h, *addr_l, *addr_cali;
    addr_arry[0] = psram_addr0;
    addr_arry[1] = psram_addr1;
    addr_arry[2] = psram_addr2;
    addr_arry[3] = psram_addr3;

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

    while (total_cnt < 0x80000000 || 1) {
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
            *addr_l = rand() & 0xffff;
            *addr_l = ((~(*addr_l)) << 16) | (*addr_l);
            *addr_cali = *addr_h = *addr_l;
            __DSB();
            //TRACE_IMM(4,"write addr %p, %p, %p, val:0x%x", addr_h, addr_l, addr_cali, *addr_l);
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

            break;
        }
    }
}


void test_psram(void)
{
    uint32_t time;
    TRACE(0,"psram_test start!");
    time = hal_sys_timer_get();
    psram_rand_data_test(mem_banks_test[0].start, mem_banks_test[1].start+0x100000,
        mem_banks_test[2].start+0x200000, mem_banks_test[3].start+0x300000);
    TRACE(1,"  %u ms", TICKS_TO_MS(hal_sys_timer_get() - time));
    TRACE(0,"psram_test done!");
    while (1) {}
}

#endif

#endif
