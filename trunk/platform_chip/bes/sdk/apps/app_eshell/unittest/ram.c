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

#if defined(UTILS_ESHELL_EN) && !defined(CHIP_SUBSYS_SENS)

#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "dwt.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "plat_addr_map.h"
#include "hal_location.h"

#include "ut_test.h"

#define  TEST_TYPE_SRAM        1
#define  TEST_TYPE_PSRAM       2
#define  TEST_TYPE_PSRAM_NC    3

#define  TEST_RAM_BANK      2
#define  TEST_RAM_SIZE      0x4000

#ifdef PSRAM_ENABLE
static volatile uint8_t psram_buf[TEST_RAM_SIZE*8] ALIGNED(4) PSRAM_BSS_LOC;
static volatile uint8_t psram_nc_buf[TEST_RAM_SIZE*8] ALIGNED(4) PSRAM_NC_BSS_LOC;
static uint32_t psram_banks[TEST_RAM_BANK] = {
    (uint32_t)psram_nc_buf,
    (uint32_t)psram_buf,
};

static volatile uint8_t psram_cali_buf[TEST_RAM_BANK*TEST_RAM_SIZE] ALIGNED(4);
static uint32_t psram_cali_banks[TEST_RAM_BANK] = {
    (uint32_t)psram_cali_buf,
    (uint32_t)psram_cali_buf+TEST_RAM_SIZE,
};
#endif

static volatile uint8_t sram_buf[TEST_RAM_BANK*TEST_RAM_SIZE*2] ALIGNED(4);
static uint32_t sram_banks[TEST_RAM_BANK] = {
    (uint32_t)sram_buf,
    (uint32_t)sram_buf+TEST_RAM_SIZE*2,
};

static volatile uint8_t sram_cali_buf[TEST_RAM_BANK*TEST_RAM_SIZE] ALIGNED(4);
static uint32_t sram_cali_banks[TEST_RAM_BANK] = {
    (uint32_t)sram_cali_buf,
    (uint32_t)sram_cali_buf+TEST_RAM_SIZE,
};

#define WRITE32(reg, val) \
    do { \
        *(volatile uint32_t *)(reg) = val; \
    } while(0)

#define READ32(reg) \
    (*((volatile uint32_t *)(reg)))


int test_sram_stab(uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t print_time = 0;
    uint32_t now_time = 0;
    uint32_t addr_h, addr_l, addr_cali;
    uint32_t val_h, val_l, val_cali;

    UT_LOG("Test SRAM Stability, by random pattern & random orders..");

    for (int i = 0; i < TEST_RAM_BANK; i++) {
        memset((void *)sram_cali_banks[i], 0, TEST_RAM_SIZE);
        memset((void *)sram_banks[i], 0, TEST_RAM_SIZE*2);
    }

    while (1) {
        uint32_t sel_bank = rand() % TEST_RAM_BANK;
        uint32_t sel_addr = (rand() % TEST_RAM_SIZE) & ~0x3;
        addr_l = sram_banks[sel_bank] + sel_addr;
        addr_h = addr_l + TEST_RAM_SIZE;
        addr_cali = sram_cali_banks[sel_bank] + sel_addr;
        if (rand() & 0x1) { /* Test Write */
            uint32_t val = rand() & 0xffff;
            val = (~val << 16) | val;
            WRITE32(addr_l, val);
            WRITE32(addr_h, val);
            WRITE32(addr_cali, val);
            __DSB();
        } else { /* Test Read */
            val_l = READ32(addr_l);
            val_h = READ32(addr_h);
            val_cali = READ32(addr_cali);

            now_time = hal_sys_timer_get();
            if (TICKS_TO_MS(now_time-start_time) >= timeout_ms)
                break;

            if (val_l == val_h && val_l == val_cali &&
                ((~val_l) >> 16) == (val_l & 0xffff)) {
                /* Print some progress.. */
                if ((now_time - print_time) >= MS_TO_TICKS(500)) {
                    UT_LOG("Check OK: addr 0x%x 0x%x, val 0x%x 0x%x",
                            addr_l, addr_h, val_l, val_h);
                    print_time = now_time;
                }
                continue;
            } else if (val_l != 0 && val_h != 0 && val_cali != 0){
                UT_ERR("ERROR: addr 0x%x 0x%x, val 0x%x 0x%x, expect 0x%x",
                            addr_l, addr_h,
                            val_l, val_h, val_cali);
                err++;
                //break;
            }
        }
    }
    if (err > 0) {
        UT_LOG("Test SRAM Stability failed, found %d errors.", err);
    } else {
        UT_LOG("Test SRAM Stability Success!");
    }

    return err;
}

int test_sram_wr(bool random_pattern, uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = 0, now_time = 0;
    uint32_t addr_h, addr_l, addr_cali;
    uint32_t val_h, val_l, val_cali;

    UT_LOG("Test SRAM R/W by %s pattern..",
            random_pattern ? "random" : "fixed");

    start_time = hal_sys_timer_get();

    while (1) {
        for (uint32_t i = 0; i < TEST_RAM_BANK; i++) {
            for (uint32_t j = 0; j < TEST_RAM_SIZE; j += 4) {
                uint32_t val;
                if (random_pattern)
                    val = rand() & 0xffff;
                else
                    val = 0xa55a + j;
                val = (~val << 16) | val;
                addr_l = sram_banks[i] + j;
                addr_h = addr_l + TEST_RAM_SIZE;
                addr_cali = sram_cali_banks[i] + j;
                WRITE32(addr_l, val);
                WRITE32(addr_h, val);
                WRITE32(addr_cali, val);
                /*UT_LOG(" addr %p %p %p, write 0x%x, now 0x%x 0x%x 0x%x\r\n",
                            addr_l, addr_h, addr_cali, val,
                            READ32(addr_l),
                            READ32(addr_h),
                            READ32(addr_cali)
                            );*/
            }
        }
        __DSB();
        for (uint32_t i = 0; i < TEST_RAM_BANK; i++) {
            for (uint32_t j = 0; j < TEST_RAM_SIZE; j += 4) {
                addr_l = sram_banks[i] + j;
                addr_h = addr_l + TEST_RAM_SIZE;
                addr_cali = sram_cali_banks[i] + j;
                val_l = READ32(addr_l);
                val_h = READ32(addr_h);
                val_cali = READ32(addr_cali);
                if (val_l == val_h && val_l == val_cali &&
                    ((~val_l) >> 16) == (val_l & 0xffff)) {
                    continue;
                } else {
                    UT_ERR("ERROR: addr 0x%x 0x%x, val 0x%x 0x%x, expect 0x%x",
                            addr_l, addr_h,
                            val_l, val_h, val_cali);
                    err++;
                    //break;
                }
            }
        }

        now_time = hal_sys_timer_get();
        if (TICKS_TO_MS(now_time - start_time) >= timeout_ms)
            break;
        hal_sys_timer_delay_us(2000);
    }
    if (err > 0) {
        UT_LOG("Test SRAM R/W failed, found %d errors.", err);
    } else {
        UT_LOG("Test SRAM R/W Success!");
    }

    return err;
}

int test_psram_stab(uint32_t timeout_ms)
{
#ifndef PSRAM_ENABLE
    UT_LOG("PSRAM is disabled.\r\n");
    return 0;
#else
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t print_time = 0;
    uint32_t now_time = 0;
    uint32_t addr_h, addr_l, addr_cali;
    uint32_t val_h, val_l, val_cali;

    UT_LOG("Test PSRAM Stability, by random pattern & random orders..");

    for (int i = 0; i < TEST_RAM_BANK; i++) {
        memset((void *)psram_cali_banks[i], 0, TEST_RAM_SIZE);
        memset((void *)psram_banks[i], 0, TEST_RAM_SIZE*2);
    }

    while (1) {
        uint32_t sel_bank = rand() % TEST_RAM_BANK;
        uint32_t sel_addr = (rand() % TEST_RAM_SIZE) & ~0x3;
        addr_l = psram_banks[sel_bank] + sel_addr;
        addr_h = addr_l + TEST_RAM_SIZE;
        addr_cali = psram_cali_banks[sel_bank] + sel_addr;
        if (rand() & 0x1) { /* Test Write */
            uint32_t val = rand() & 0xffff;
            val = (~val << 16) | val;
            WRITE32(addr_l, val);
            WRITE32(addr_h, val);
            WRITE32(addr_cali, val);
            __DSB();
        } else { /* Test Read */
            val_l = READ32(addr_l);
            val_h = READ32(addr_h);
            val_cali = READ32(addr_cali);

            now_time = hal_sys_timer_get();
            if (TICKS_TO_MS(now_time-start_time) >= timeout_ms)
                break;

            if (val_l == val_h && val_l == val_cali &&
                ((~val_l) >> 16) == (val_l & 0xffff)) {
                /* Print some progress.. */
                if ((now_time - print_time) >= MS_TO_TICKS(500)) {
                    UT_LOG("Check OK: addr 0x%x 0x%x, val 0x%x 0x%x",
                            addr_l, addr_h, val_l, val_h);
                    print_time = now_time;
                }
                continue;
            } else if (val_l != 0 && val_h != 0 && val_cali != 0){
                UT_LOG("ERROR: addr 0x%x 0x%x, val 0x%x 0x%x, expect 0x%x",
                            addr_l, addr_h,
                            val_l, val_h, val_cali);
                err++;
                //break;
            }
        }
    }
    if (err > 0) {
        UT_LOG("Test PSRAM Stability failed, found %d errors.", err);
    } else {
        UT_LOG("Test PSRAM Stability Success!");
    }


    return err;
#endif
}

int test_psram_wr(bool random_pattern, uint32_t timeout_ms)
{
#ifndef PSRAM_ENABLE
    UT_LOG("PSRAM is disabled.");
    return 0;
#else
    int err = 0;
    uint32_t start_time = 0, now_time = 0;
    uint32_t addr_h, addr_l, addr_cali;
    uint32_t val_h, val_l, val_cali;

    UT_LOG("Test PSRAM R/W by %s pattern..",
            random_pattern ? "random" : "fixed");

    start_time = hal_sys_timer_get();

    while (1) {
        for (uint32_t i = 0; i < TEST_RAM_BANK; i++) {
            for (uint32_t j = 0; j < TEST_RAM_SIZE; j += 4) {
                uint32_t val;
                if (random_pattern)
                    val = rand() & 0xffff;
                else
                    val = 0xa55a + j;
                val = (~val << 16) | val;
                addr_l = psram_banks[i] + j;
                addr_h = addr_l + TEST_RAM_SIZE;
                addr_cali = psram_cali_banks[i] + j;
                WRITE32(addr_l, val);
                WRITE32(addr_h, val);
                WRITE32(addr_cali, val);
                /*UT_LOG(" addr %p %p %p, write 0x%x, now 0x%x 0x%x 0x%x\r\n",
                            addr_l, addr_h, addr_cali, val,
                            READ32(addr_l),
                            READ32(addr_h),
                            READ32(addr_cali)
                            );*/
            }
        }
        __DSB();
        for (uint32_t i = 0; i < TEST_RAM_BANK; i++) {
            for (uint32_t j = 0; j < TEST_RAM_SIZE; j += 4) {
                addr_l = psram_banks[i] + j;
                addr_h = addr_l + TEST_RAM_SIZE;
                addr_cali = psram_cali_banks[i] + j;
                val_l = READ32(addr_l);
                val_h = READ32(addr_h);
                val_cali = READ32(addr_cali);
                if (val_l == val_h && val_l == val_cali &&
                    ((~val_l) >> 16) == (val_l & 0xffff)) {
                    continue;
                } else {
                    UT_ERR("ERROR: addr 0x%x 0x%x, val 0x%x 0x%x 0x%x",
                            addr_l, addr_h,
                            val_l, val_h, val_cali);
                    err++;
                    //break;
                }
            }
        }

        now_time = hal_sys_timer_get();
        if (TICKS_TO_MS(now_time - start_time) >= timeout_ms)
            break;
        hal_sys_timer_delay_us(2000);
    }
    if (err > 0) {
        UT_LOG("Test PSRAM R/W failed, found %d errors.", err);
    } else {
        UT_LOG("Test PSRAM R/W Success!");
    }

    return err;
#endif
}


static int test_bitflip(uint32_t test_addr, uint32_t test_size)
{
    int err = 0;
    volatile uint32_t *addr = (volatile uint32_t *)test_addr;
    uint32_t pat, val;

    pat = 0;
    for (uint32_t i = 0; i < test_size; i += 8) {
        addr[i+0] = pat;
        addr[i+1] = pat;
        addr[i+2] = pat;
        addr[i+3] = pat;
        addr[i+4] = pat;
        addr[i+5] = pat;
        addr[i+6] = pat;
        addr[i+7] = pat;
        pat++;
    }

    pat = 0;
    for (uint32_t i = 0; i < test_size; i += 8) {
        val = addr[i+0];
        val += addr[i+1];
        val += addr[i+2];
        val += addr[i+3];
        val += addr[i+4];
        val += addr[i+5];
        val += addr[i+6];
        val += addr[i+7];
        if (val != pat * 8) {
            UT_ERR("ERROR: addr %p ~ %p, val: ", &addr[i+0], &addr[i+7]);
            for (uint32_t j = 0; j < 8; j++)
                UT_ERR("   %x", addr[i+j]);
            err++;
        }
        pat++;
    }

    pat = 0;
    for (uint32_t i = 0; i < test_size; i += 8) {
        addr[i+0] = ~pat;
        addr[i+1] = ~pat;
        addr[i+2] = ~pat;
        addr[i+3] = ~pat;
        addr[i+4] = ~pat;
        addr[i+5] = ~pat;
        addr[i+6] = ~pat;
        addr[i+7] = ~pat;
        pat++;
    }

    pat = 0;
    for (uint32_t i = 0; i < test_size; i += 8) {
        val = addr[i+0];
        val += addr[i+1];
        val += addr[i+2];
        val += addr[i+3];
        val += addr[i+4];
        val += addr[i+5];
        val += addr[i+6];
        val += addr[i+7];
        if (val != (~pat) * 8) {
            UT_ERR("ERROR: addr %p ~ %p, val", &addr[i+0], &addr[i+7]);
            for (uint32_t j = 0; j < 8; j++)
                UT_ERR("   %x", addr[i+j]);
            err++;
        }
        pat++;
    }
    return err;
}

static int test_ram_bitflip(uint8_t ram_type, uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t sram_addr = (uint32_t)sram_buf;
    uint32_t test_size = TEST_RAM_SIZE;
#ifdef PSRAM_ENABLE
    uint32_t psram_addr = (uint32_t)psram_banks[0];
    uint32_t test_addr = (ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
#else
    uint32_t test_addr = sram_addr;

    if (ram_type == TEST_TYPE_PSRAM) {
        UT_LOG("PSRAM is disabled.\r\n");
        return 0;
    }
#endif

    UT_LOG("Test %s bit flip..",
                     (ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM");

    while (1) {
        err += test_bitflip(test_addr, test_size);

        if (TICKS_TO_MS(hal_sys_timer_get()-start_time) >= timeout_ms)
            break;
        hal_sys_timer_delay_us(2000);
    }

    return err;
}

static int test_memset_8bit(uint32_t test_addr, uint32_t test_size)
{
    int err = 0;
    uint8_t pat = rand() & 0xff, p;
    volatile uint8_t *addr;

    p = pat;
    for (uint32_t i = 0; i < test_size; i++) {
        addr = (volatile uint8_t *)(test_addr + i);
        *addr = p + i;
        __DSB();
    }

    p = pat;
    for (uint32_t i = 0; i < test_size; i++) {
        addr = (volatile uint8_t *)(test_addr + i);
        if (*addr != p + i) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        addr, *addr, p + i);
            err++;
        }
    }

    return err;
}

static int test_memset_16bit(uint32_t test_addr, uint32_t test_size)
{
    int err = 0;
    uint16_t pat = rand() & 0xffff, p;
    volatile uint16_t *addr;

    p = pat;
    for (uint32_t i = 0; i < test_size; i += 2) {
        addr = (volatile uint16_t *)(test_addr + i);
        *addr = p + i;
        __DSB();
    }

    p = pat;
    for (uint32_t i = 0; i < test_size; i += 2) {
        addr = (volatile uint16_t *)(test_addr + i);
        if (*addr != p + i) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        addr, *addr, p + i);
            err++;
        }
    }

    return err;
}

static int test_memset_32bit(uint32_t test_addr, uint32_t test_size)
{
    int err;
    uint32_t pat = rand() & 0xffffffff, p;
    volatile uint32_t *addr;

    p = pat;
    for (uint32_t i = 0; i < test_size; i += 4) {
        addr = (volatile uint32_t *)(test_addr + i);
        *addr = p + i;
        __DSB();
    }

    p = pat;
    for (uint32_t i = 0; i < test_size; i += 4) {
        addr = (volatile uint32_t *)(test_addr + i);
        if (*addr != p + i) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        addr, *addr, p + i);
            err++;
        }
    }

    return err;
}

static int test_memcpy_8bit(uint32_t src_addr, uint32_t dst_addr,
                        uint32_t test_size)
{
    int err = 0;
    volatile uint8_t *src_p, *dst_p;

    /* fill source addresses by random pattern */
    for (uint32_t i = 0; i < test_size; i++) {
        src_p = (volatile uint8_t *)(src_addr + i);
        *src_p = rand() & 0xff;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i++) {
        src_p = (volatile uint8_t *)(src_addr + i);
        dst_p = (volatile uint8_t *)(dst_addr + i);
        *dst_p = *src_p;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i++) {
        src_p = (volatile uint8_t *)(src_addr + i);
        dst_p = (volatile uint8_t *)(dst_addr + i);
        if (*dst_p != *src_p) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        dst_p, *dst_p, *src_p);
            err++;
        }
    }

    return err;
}

static int test_memcpy_16bit(uint32_t src_addr, uint32_t dst_addr,
                        uint32_t test_size)
{
    int err = 0;
    volatile uint16_t *src_p, *dst_p;

    /* fill source addresses by random pattern */
    for (uint32_t i = 0; i < test_size; i += 2) {
        src_p = (volatile uint16_t *)(src_addr + i);
        *src_p = rand() & 0xffff;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i += 2) {
        src_p = (volatile uint16_t *)(src_addr + i);
        dst_p = (volatile uint16_t *)(dst_addr + i);
        *dst_p = *src_p;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i += 2) {
        src_p = (volatile uint16_t *)(src_addr + i);
        dst_p = (volatile uint16_t *)(dst_addr + i);
        if (*dst_p != *src_p) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        dst_p, *dst_p, *src_p);
            err++;
        }
    }

    return err;
}

static int test_memcpy_32bit(uint32_t src_addr, uint32_t dst_addr,
                        uint32_t test_size)
{
    int err = 0;
    volatile uint32_t *src_p, *dst_p;

    /* fill source addresses by random pattern */
    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        *src_p = rand() & 0xffffffff;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        *dst_p = *src_p;
        __DSB();
    }

    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        if (*dst_p != *src_p) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        dst_p, *dst_p, *src_p);
            err++;
        }
    }

    return err;
}


static int test_ram_memset(uint8_t ram_type, uint8_t test_bits, uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t sram_addr = (uint32_t)sram_buf;
    uint32_t test_size = TEST_RAM_SIZE;
#ifdef PSRAM_ENABLE
    uint32_t psram_addr = (uint32_t)psram_banks[0];
    uint32_t test_addr = (ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
#else
    uint32_t test_addr = sram_addr;

    if (ram_type == TEST_TYPE_PSRAM) {
        UT_LOG("PSRAM is disabled.");
        return 0;
    }
#endif

    UT_LOG("Test %s %dbit memset..",
                     (ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM",
                     test_bits);

    while (1) {
        if (test_bits == 8)
            err += test_memset_8bit(test_addr, test_size);
        else if (test_bits == 16)
            err += test_memset_16bit(test_addr, test_size);
        else if (test_bits == 32)
            err += test_memset_32bit(test_addr, test_size);

        if (TICKS_TO_MS(hal_sys_timer_get()-start_time) >= timeout_ms)
            break;
        hal_sys_timer_delay_us(2000);
    }

    return err;
}

static int test_ram_memcpy(uint8_t src_ram_type, uint8_t dst_ram_type,
                    uint8_t test_bits, uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t sram_addr = (uint32_t)sram_buf;
    uint32_t test_size = TEST_RAM_SIZE;
#ifdef PSRAM_ENABLE
    uint32_t psram_addr = (uint32_t)psram_banks[0];
    uint32_t src_addr = (src_ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
    uint32_t dst_addr = (dst_ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
#else
    uint32_t src_addr = sram_addr;
    uint32_t dst_addr = sram_addr;

    if (src_ram_type == TEST_TYPE_PSRAM || dst_ram_type == TEST_TYPE_PSRAM) {
        UT_LOG("PSRAM is disabled.");
        return 0;
    }
#endif

    if (src_addr == dst_addr)
        dst_addr += test_size;

    UT_LOG("Test %s -> %s %dbit memcpy..",
                     (src_ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM",
                     (dst_ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM",
                     test_bits);

    while (1) {
        if (test_bits == 8)
            err += test_memcpy_8bit(src_addr, dst_addr, test_size);
        else if (test_bits == 16)
            err += test_memcpy_16bit(src_addr, dst_addr, test_size);
        else if (test_bits == 32)
            err += test_memcpy_32bit(src_addr, dst_addr, test_size);

        if (TICKS_TO_MS(hal_sys_timer_get()-start_time) >= timeout_ms)
            break;
        hal_sys_timer_delay_us(2000);
    }

    return err;
}

#define TEST_DMA_MAX_SIZE   1024
static struct HAL_DMA_DESC_T g_dma_desc = {0};
static bool dma_working = false;

static void dma_irq_handler(uint8_t chan, uint32_t remain_tsize,
        uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if (error) {
        UT_ERR("ERROR: DMA copy error!");
    } else {
        /* release the DMA channel. */
        hal_dma_free_chan(chan);
    }
    dma_working = false;
}

static void dma_start_cb(uint8_t chan)
{
    UT_DBG("Test DMA copy start, chan[%d] used.\r\n", chan);
}

static int test_dmacpy_data(uint32_t src_addr, uint32_t dst_addr, uint32_t test_size)
{
    int err = 0;
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    volatile uint32_t *src_p, *dst_p;
    int timeout = 200*1000; //us

    /* fill source addresses by random pattern */
    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        *src_p = rand() & 0xffffffff;
        *dst_p = 0;
        __DSB();
    }

    hal_dma_open();

    /* desc config */
    struct HAL_DMA_CH_CFG_T cfg = {0};
    memset(&cfg, 0, sizeof(cfg));

    cfg.type = HAL_DMA_FLOW_M2M_DMA;

    /* src config. */
    cfg.src = src_addr;
    cfg.src_tsize = test_size / HAL_DMA_BSIZE_4;
    cfg.src_bsize = HAL_DMA_BSIZE_4;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_DMA_PERIPH_NULL;

    /* dst config. */
    cfg.dst = dst_addr;
    cfg.dst_bsize = HAL_DMA_BSIZE_4;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_DMA_PERIPH_NULL;

    cfg.try_burst = 1;
    cfg.handler = dma_irq_handler;
    cfg.start_cb = dma_start_cb;
    cfg.ch = hal_dma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        UT_ERR("ERROR: all DMA channels are busy.");
        return 1;
    }

    /* cofig: g_dma_desc. src, dst, lli, and ctrl */
    ret = hal_dma_init_desc(&g_dma_desc, &cfg, NULL, 1);
    if (ret) {
        hal_dma_free_chan(cfg.ch);
        UT_ERR("ERROR: DMA init failed.");
        return 1;
    }

    dma_working = true;

    /* start */
    ret = hal_dma_sg_start(&g_dma_desc, &cfg);
    if (ret) {
        hal_dma_free_chan(cfg.ch);
        UT_ERR("ERROR: DMA start failed.");
        return 1;
    }

    while(dma_working && timeout > 0) {
        hal_sys_timer_delay_us(100);
        timeout -= 100;
    }

    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        if (*dst_p != *src_p) {
            UT_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        dst_p, *dst_p, *src_p);
            err++;
        }
    }

    return err;
}


static int test_ram_dmacpy(uint8_t src_ram_type, uint8_t dst_ram_type, uint32_t timeout_ms)
{
    int err = 0;
    uint32_t start_time = hal_sys_timer_get();
    uint32_t sram_addr = (uint32_t)sram_buf;
    POSSIBLY_UNUSED volatile uint32_t *src_p, *dst_p;
    uint32_t test_size = TEST_RAM_SIZE;
#ifdef PSRAM_ENABLE
    uint32_t psram_addr = (uint32_t)psram_banks[0];
    uint32_t src_addr = (src_ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
    uint32_t dst_addr = (dst_ram_type == TEST_TYPE_SRAM) ? sram_addr : psram_addr;
#else
    uint32_t src_addr = sram_addr;
    uint32_t dst_addr = sram_addr;

    if (src_ram_type == TEST_TYPE_PSRAM || dst_ram_type == TEST_TYPE_PSRAM) {
        UT_LOG("PSRAM is disabled.");
        return 0;
    }
#endif

    if (src_addr == dst_addr)
        dst_addr += test_size;

    UT_LOG("Test %s -> %s DMA copy..",
                     (src_ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM",
                     (dst_ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM");

    while (1) {
        if (test_size > TEST_DMA_MAX_SIZE) {
            uint32_t remain_size = test_size;
            uint32_t dma_size = TEST_DMA_MAX_SIZE;
            uint32_t dma_src = src_addr;
            uint32_t dma_dst = dst_addr;
            while (remain_size > 0) {
                if (remain_size > dma_size)
                    err += test_dmacpy_data(dma_src, dma_dst, dma_size);
                else
                    err += test_dmacpy_data(dma_src, dma_dst, remain_size);

                remain_size -= dma_size;
                dma_src += dma_size;
                dma_dst += dma_size;
            }
        } else {
            test_dmacpy_data(src_addr, dst_addr, test_size);
        }

        if (TICKS_TO_MS(hal_sys_timer_get()-start_time) >= timeout_ms)
            break;

        hal_sys_timer_delay_us(2000);
    }

    return err;
}


enum mbw_test_item {
    TEST_MEMCPY = 0,
    TEST_MEMSET,
    TEST_MEMMOVE,
    TEST_DUMB,
    TEST_MCBLOCK,
    TEST_READ,
    TEST_WRITE,
};

static void *mbw_mempcpy(void *d, const void *s, size_t len)
{
    long *dst = (long *)d;
    long *src = (long *)s;
    // 4 instructions (with one skip)
    while (dst < (long *)(d + len))
        *dst++ = *src++;

    return dst;
}

static uint32_t mbw_worker(unsigned long asize, long *a, long *b,
        int type, unsigned long block_size)
{
    uint32_t starttime, endtime;
    uint32_t te;
    unsigned int long_size = sizeof(long);
    /* array size in bytes */
    unsigned long array_bytes = asize * long_size;
    uint32_t cyc_cnt = -1u;

    if (type == TEST_MEMCPY) {  /* memcpy test */
        /* timer starts */
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();
        memcpy(b, a, array_bytes);
        cyc_cnt = dwt_read_cycle_cnt();
        /* timer stops */
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_MEMSET) {  /* memset test */
        /* timer starts */
        starttime = hal_fast_sys_timer_get();

        dwt_reset_cycle_cnt();
        memset(a, 0xaa, array_bytes);
        cyc_cnt = dwt_read_cycle_cnt();

        /* timer stops */
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_MEMMOVE) {  /* memmove test */
        /* timer starts */
        starttime = hal_fast_sys_timer_get();

        dwt_reset_cycle_cnt();
        memmove(a, b, array_bytes);
        cyc_cnt = dwt_read_cycle_cnt();
        /* timer stops */
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_DUMB) { /* dumb test */
        starttime = hal_fast_sys_timer_get();
        long *dst = a;
        long *src = b;
        // 4 instructions (with one skip)
        while (dst < a + asize)
            *dst++ = *src++;
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_MCBLOCK) {  /* memcpy block test */
        void *aa = (char *)a;
        void *bb = (char *)b;
        starttime = hal_fast_sys_timer_get();
        while (bb + block_size <= (void *)(b + asize)) {
            bb = mbw_mempcpy(bb, aa, block_size);
        }

        if (bb < (void *)(b + asize)) {
            bb = mbw_mempcpy(bb, aa, (void *)(b + asize) - bb);
        }
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_READ) { /* read test */
        long temp_val = 0;
        long *src = a;
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();
        while (src < a + asize) {
            /*
             * Note:check the asm code make sure the next instruction is
             * aligned to 4
             */
#if 0
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);

            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);
            asm volatile ("VLDRW.S32 q0, [%1], #16":"+r"(src)::);

#else
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
#endif
        }
        cyc_cnt = dwt_read_cycle_cnt();
        endtime = hal_fast_sys_timer_get();
    } else if (type == TEST_WRITE) {    /* write test */
        long temp_val = 0xff;
        long *dst = a;
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();
        while (dst < a + asize) {
            /*
             * Note:check the asm code make sure the next instruction is
             * aligned to 4
             */
#if 0
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);

            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
            asm volatile ("VSTRW.S32 q0, [%1], #16":"+r"(dst)::);
#else

            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);

            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
            asm volatile ("str.w %1, [%0], #4": "+r"(dst): "r"(temp_val):);
#endif
        }
        cyc_cnt = dwt_read_cycle_cnt();
        endtime = hal_fast_sys_timer_get();
    } else {
        eshell_putstring("ERROR!\r\n");
        starttime = 0;
        endtime = 0;
    }

    te = FAST_TICKS_TO_US(endtime - starttime);

    if (cyc_cnt != -1u)
        eshell_putstring("\tcpu cycles %d \r\n", cyc_cnt);
    return te;
}

POSSIBLY_UNUSED static void mbw_printout(uint32_t te, int mt, int type)
{
    switch (type) {
    case TEST_MEMCPY:
        eshell_putstring("Method: MEMCPY\t");
        break;
    case TEST_MEMSET:
        eshell_putstring("Method: MEMSET\t");
        break;
    case TEST_MEMMOVE:
        eshell_putstring("Method: MEMMOVE\t");
        break;
    case TEST_DUMB:
        eshell_putstring("Method: DUMB  \t");
        break;
    case TEST_MCBLOCK:
        eshell_putstring("Method: MCBLOCK\t");
        break;
    case TEST_READ:
        eshell_putstring("Method: READ   \t");
        break;
    case TEST_WRITE:
        eshell_putstring("Method: WRITE  \t");
        break;
    }

    eshell_putstring("Elapsed: %5d us. ", te);
    eshell_putstring("KB: %4d. ", mt);
    uint64_t bytes = mt * 1024;
    uint32_t speed_mhz = bytes / te;
    uint32_t speed_remain = bytes * 100 / te - speed_mhz * 100;
    eshell_putstring("Speed: %4u.%02u MB/s\n", speed_mhz, speed_remain);
    return;
}

#define MBW_TEST_TOTAL_SIZE_KB      (TEST_RAM_SIZE/1024)
#define MBW_TEST_BLOCK_SIZE_KB      16
static int test_ram_mbw(uint8_t ram_type, uint32_t loop_count)
{
    uint32_t te, te_sum;        /* time elapsed */
    long *a, *b;                /* the two arrays to be copied from/to */

    /* run all tests requested, the proper number of times */
    int test_cases[] = {TEST_READ, TEST_WRITE, -1};
    int testno = 0;

    /* how many runs to average? */
    int nr_loops = loop_count;
    /* fixed memcpy block size for -t2 */
    int block_size = MBW_TEST_BLOCK_SIZE_KB * 1024;
    /* show average, -a */
    /* what tests to run (-t x) */
    int mt = MBW_TEST_TOTAL_SIZE_KB;    /* KB transferred == array size in KB */
    int asize; /* how many longs then in one array? */

    if (ram_type == TEST_TYPE_PSRAM || ram_type == TEST_TYPE_PSRAM_NC)
        mt *= 4;

    asize = 1024 / sizeof(long) * mt;

    if (block_size > mt)
        block_size = mt;

    if (ram_type == TEST_TYPE_SRAM) {
        a = (long *)sram_buf;
        b = (long *)(sram_buf + TEST_RAM_SIZE);
    } else {
#ifdef PSRAM_ENABLE
        if (ram_type == TEST_TYPE_PSRAM_NC) {
            a = (long *)psram_nc_buf;
            b = (long *)(psram_nc_buf + TEST_RAM_SIZE);
        } else {
            a = (long *)psram_buf;
            b = (long *)(psram_buf + TEST_RAM_SIZE);
        }
#else
        eshell_putstring("PSRAM is disabled.\r\n");
        return 0;
#endif
    }
    eshell_putstring("Test MBW, %s address: %p, %p, %uKB\r\n",
                (ram_type == TEST_TYPE_SRAM) ? "SRAM" : "PSRAM",
                a, b, mt);

    dwt_enable();
    hal_sysfreq_req(APP_SYSFREQ_USER_ESHELL, HAL_CMU_FREQ_208M);

    hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);

//  hal_cache_monitor_enable(HAL_CACHE_ID_I_CACHE);
//  hal_cache_monitor_enable(HAL_CACHE_ID_D_CACHE);

    while (test_cases[testno] != -1) {
        te_sum = 0;
        for (int i = 0; nr_loops == 0 || i < nr_loops; i++) {
            TRACE_FLUSH();
            hal_sys_timer_delay(16);
            uint32_t lock = int_lock();
            te = mbw_worker(asize, a, b, test_cases[testno], block_size);
            int_unlock(lock);
            te_sum += te;
            eshell_putstring("%d\t", i);
            mbw_printout(te, mt, test_cases[testno]);
//          hal_cache_print_stats();
        }
        eshell_putstring("AVG\t");
        mbw_printout(te_sum / nr_loops, mt, test_cases[testno]);
        eshell_putstring("\n\r");
        testno++;
    }

    hal_sysfreq_req(APP_SYSFREQ_USER_ESHELL, HAL_CMU_FREQ_32K);
    return 0;
}

static void test_ram_usage(void)
{
    eshell_putstring("Usage: \r\n");
    eshell_putstring("  utest_ram stab sram|psram [timeout_ms], SRAM/PSRAM stability test\r\n");
    eshell_putstring("  utest_ram rw sram|psram [timeout_ms], SRAM/PSRAM read/write test\r\n");
    eshell_putstring("  utest_ram bitflip sram|psram [timeout_ms], bit flip test\r\n");
    eshell_putstring("  utest_ram memset sram|psram 8|16|32 [timeout_ms], data set test\r\n");
    eshell_putstring("  utest_ram memcpy sram|psram sram|psram 8|16|32 [timeout_ms], data copy test\r\n");
    eshell_putstring("  utest_ram dmacpy sram|psram sram|psram [timeout_ms], DMA data copy test\r\n");
    eshell_putstring("  utest_ram mbw sram|psram|psram_nc [loop_count], MBW test\r\n");
}

static void unitest_ram(int argc, char *argv[])
{
    uint32_t timeout = 1000; //ms

    if (argc < 3)
        goto usage;

    if (strncmp(argv[1], "stab", 4) == 0) {
        if (argc >= 4) {
            timeout = atoi(argv[3]);
            if (timeout < 0) {
                timeout = 0;
            }
        }

        if (strncmp(argv[2], "sram", 4) == 0)
            test_sram_stab(timeout);
        else if (strncmp(argv[2], "psram", 5) == 0)
            test_psram_stab(timeout);
        else
            goto usage;

        return;

    } else if (strncmp(argv[1], "rw", 2) == 0) {
        if (argc >= 4) {
            timeout = atoi(argv[3]);
            if (timeout < 0) {
                timeout = 0;
            }
        }

        if (strncmp(argv[2], "sram", 4) == 0)
            test_sram_wr(true, timeout);
        else if (strncmp(argv[2], "psram", 5) == 0)
            test_psram_wr(true, timeout);
        else
            goto usage;

        return;

    } else if (strncmp(argv[1], "mbw", 3) == 0) {
        int count = 10;
        if (argc >= 4) {
            count = atoi(argv[3]);
            if (count < 0) {
                timeout = 10;
            }
        }

        if (strncmp(argv[2], "sram", 4) == 0)
            test_ram_mbw(TEST_TYPE_SRAM, count);
        else if (strncmp(argv[2], "psram_nc", 8) == 0)
            test_ram_mbw(TEST_TYPE_PSRAM_NC, count);
        else if (strncmp(argv[2], "psram", 5) == 0)
            test_ram_mbw(TEST_TYPE_PSRAM, count);
        else
            goto usage;

        return;

    } else if (strncmp(argv[1], "bitflip", 7) == 0) {
        uint8_t ram_type = TEST_TYPE_PSRAM;

        if (argc >= 4) {
            timeout = atoi(argv[3]);
            if (timeout < 0) {
                timeout = 0;
            }
        }

        if (strncmp(argv[2], "sram", 4) == 0)
            ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[2], "psram", 4) == 0)
            ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        test_ram_bitflip(ram_type, timeout);
        return;

    } else if (strncmp(argv[1], "memset", 6) == 0) {
        uint8_t ram_type = TEST_TYPE_PSRAM;
        uint8_t test_bits = 8;

        if (argc < 5)
            goto usage;

        if (strncmp(argv[2], "sram", 4) == 0)
            ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[2], "psram", 4) == 0)
            ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        test_bits = (uint8_t) atoi(argv[3]);
        if (test_bits != 8 && test_bits != 16 && test_bits != 32)
            goto usage;

        if (argc >= 5) {
            timeout = atoi(argv[4]);
            if (timeout < 0)
                timeout = 0;
        }

        test_ram_memset(ram_type, test_bits, timeout);
        return;

    } else if (strncmp(argv[1], "memcpy", 6) == 0) {
        uint8_t src_ram_type = TEST_TYPE_SRAM;
        uint8_t dst_ram_type = TEST_TYPE_PSRAM;
        uint8_t test_bits = 8;

        if (argc < 5)
            goto usage;

        if (strncmp(argv[2], "sram", 4) == 0)
            src_ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[2], "psram", 4) == 0)
            src_ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        if (strncmp(argv[3], "sram", 4) == 0)
            dst_ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[3], "psram", 4) == 0)
            dst_ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        test_bits = (uint8_t) atoi(argv[4]);
        if (test_bits != 8 && test_bits != 16 && test_bits != 32)
            goto usage;

        if (argc >= 6) {
            timeout = atoi(argv[5]);
            if (timeout < 0)
                timeout = 0;
        }

        test_ram_memcpy(src_ram_type, dst_ram_type, test_bits, timeout);
        return;

    } else if (strncmp(argv[1], "dmacpy", 6) == 0) {
        uint8_t src_ram_type = TEST_TYPE_SRAM;
        uint8_t dst_ram_type = TEST_TYPE_PSRAM;

        if (argc < 4)
            goto usage;

        if (strncmp(argv[2], "sram", 4) == 0)
            src_ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[2], "psram", 4) == 0)
            src_ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        if (strncmp(argv[3], "sram", 4) == 0)
            dst_ram_type = TEST_TYPE_SRAM;
        else if (strncmp(argv[3], "psram", 4) == 0)
            dst_ram_type = TEST_TYPE_PSRAM;
        else
            goto usage;

        if (argc >= 5) {
            timeout = atoi(argv[4]);
            if (timeout < 0)
                timeout = 0;
        }

        test_ram_dmacpy(src_ram_type, dst_ram_type, timeout);
        return;
    }

usage:
    test_ram_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_ram",
                        "usage: utest_ram help", unitest_ram);

#endif //UTILS_ESHELL_EN
