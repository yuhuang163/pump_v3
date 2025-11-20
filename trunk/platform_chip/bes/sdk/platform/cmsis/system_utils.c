/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "cmsis.h"
#ifdef COMMON_IRQ_ENTRY
#include "cmsis_nvic.h"
#endif
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
#include "hal_norflash.h"
#endif
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "hal_trace.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#if !defined(NOSTD) && defined(ACCURATE_DB_TO_FLOAT)
#include <math.h>
#endif
#include "mpu_cfg.h"
#include "tool_msg.h"
#include "hal_timer.h"

BOOT_BSS_LOC
struct irq_masked_address irq_masked_addr;

BOOT_BSS_LOC
struct irq_masked_address irq_g_masked_addr;

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
// CAUTION:
// O2 option in GCC 10 will optimize the copy loop to an invokation of memcpy,
// whereas memcpy might be located in RAM and not initialized yet.
#define BOOT_CODE_OPT_ATTR                  __attribute__((optimize("no-tree-loop-distribute-patterns")))
#else
#define BOOT_CODE_OPT_ATTR
#endif

#if !defined(__ARM_ARCH_ISA_ARM) || defined(CONFIG_BOOT_RUNFROMFLASH)
// -----------------------------------------------------------
// Boot initialization
// CAUTION: This function and all the functions it called must
//          NOT access global data/bss, for the global data/bss
//          is not available at that time.
// -----------------------------------------------------------
extern uint32_t __boot_text_sram_start_flash__[];
extern uint32_t __boot_text_sram_end_flash__[];
extern uint32_t __boot_data_sram_start_flash__[];
extern uint32_t __boot_data_sram_end_flash__[];
extern uint32_t __boot_text_data_sram_start__[];
extern uint32_t __boot_data_sram_start__[];
extern uint32_t __boot_bss_sram_start__[];
extern uint32_t __boot_bss_sram_end__[];

extern uint32_t __sram_text_start_flash__[];
extern uint32_t __sram_text_end_flash__[];
extern uint32_t __sram_text_data_start__[];
extern uint32_t __sram_data_start_flash__[];
extern uint32_t __sram_data_end_flash__[];
extern uint32_t __sram_data_start__[];
extern uint32_t __sram_bss_start__[];
extern uint32_t __sram_bss_end__[];
extern uint32_t __fast_sram_text_data_start__[];
extern uint32_t __fast_sram_text_data_end__[];
extern uint32_t __fast_sram_text_start_flash__[];
extern uint32_t __fast_sram_text_end_flash__[];
#ifdef RAM_NC_BASE
extern uint32_t __sram_nc_data_start_flash__[];
extern uint32_t __sram_nc_data_end_flash__[];
extern uint32_t __sram_nc_data_start__[];
extern uint32_t __sram_nc_data_end__[];
extern uint32_t __sram_nc_bss_start__[];
extern uint32_t __sram_nc_bss_end__[];
#endif
#ifdef RAM_EXTRA0_BASE
extern uint32_t __ram_extra0_text_data_start_flash__[];
extern uint32_t __ram_extra0_text_data_end_flash__[];
extern uint32_t __ram_extra0_text_data_start__[];
extern uint32_t __ram_extra0_text_data_end__[];
extern uint32_t __ram_extra0_bss_start__[];
extern uint32_t __ram_extra0_bss_end__[];
#endif
#ifdef AON_MAILBOX_BASE
extern uint32_t __aon_mbx_data_start_load__[];
extern uint32_t __aon_mbx_data_end_load__[];
extern uint32_t __aon_mbx_data_start__[];
extern uint32_t __aon_mbx_bss_start__[];
extern uint32_t __aon_mbx_bss_end__[];
#endif
#ifdef ARM_CMSE
extern uint32_t __sram_nsc_text_start__[];
extern uint32_t __sram_nsc_text_start_flash__[];
extern uint32_t __sram_nsc_text_end_flash__[];
#endif

#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)
extern uint32_t __psram_text_data_start__[];
extern uint32_t __psram_text_data_start_flash__[];
extern uint32_t __psram_text_data_end_flash__[];
extern uint32_t __psram_bss_start__[];
extern uint32_t __psram_bss_end__[];
extern uint32_t __psram_nc_data_start_flash__[];
extern uint32_t __psram_nc_data_end_flash__[];
extern uint32_t __psram_nc_data_start__[];
extern uint32_t __psram_nc_data_end__[];
extern uint32_t __psram_nc_bss_start__[];
extern uint32_t __psram_nc_bss_end__[];
#endif

#if defined(CHIP_HAS_PSRAMUHS) && defined(PSRAMUHS_ENABLE) && (MCU_PSRAMUHS_SIZE !=0)
extern uint32_t __psramuhs_text_data_start__[];
extern uint32_t __psramuhs_text_data_start_flash__[];
extern uint32_t __psramuhs_text_data_end_flash__[];
extern uint32_t __psramuhs_bss_start__[];
extern uint32_t __psramuhs_bss_end__[];
extern uint32_t __psramuhs_nc_data_start_flash__[];
extern uint32_t __psramuhs_nc_data_end_flash__[];
extern uint32_t __psramuhs_nc_data_start__[];
extern uint32_t __psramuhs_nc_data_end__[];
extern uint32_t __psramuhs_nc_bss_start__[];
extern uint32_t __psramuhs_nc_bss_end__[];
#endif

#if defined(CP_IN_SAME_EE)
extern uint32_t __cp_always_on_data_sram_start_flash__[];
extern uint32_t __cp_always_on_data_sram_start[];
extern uint32_t __cp_always_on_data_sram_end[];
extern uint32_t __cp_always_on_bss_sram_start[];
extern uint32_t __cp_always_on_bss_sram_end[];
#endif

void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_boot_sections(void)
{
    uint32_t *dst, *src;

    for (dst = __boot_text_data_sram_start__, src = __boot_text_sram_start_flash__;
            src < __boot_text_sram_end_flash__;
            dst++, src++) {
        *dst = *src;
    }
    for (dst = __boot_data_sram_start__, src = __boot_data_sram_start_flash__;
            src < __boot_data_sram_end_flash__;
            dst++, src++) {
        *dst = *src;
    }
#ifdef MAIN_RAM_USE_SYS_RAM
    hal_cache_boot_sync_all(HAL_CACHE_ID_D_CACHE);
#else
    __DSB();
    __ISB();
#endif

    for (dst = __boot_bss_sram_start__; dst < __boot_bss_sram_end__; dst++) {
        *dst = 0;
    }
}

#ifndef COMPRESS_SRAM_CODE
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_sram_sections(void)
{
    uint32_t *dst, *src;

    for (dst = __sram_text_data_start__, src = __sram_text_start_flash__;
            src < __sram_text_end_flash__;
            dst++, src++) {
        *dst = *src;
    }
    for (dst = __sram_data_start__, src = __sram_data_start_flash__;
            src < __sram_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __fast_sram_text_data_start__, src = __fast_sram_text_start_flash__;
            src < __fast_sram_text_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

#ifdef AON_MAILBOX_BASE
    for (dst = __aon_mbx_data_start__, src = __aon_mbx_data_start_load__;
            src < __aon_mbx_data_end_load__;
            dst++, src++) {
        *dst = *src;
    }
#endif

#if defined(CP_IN_SAME_EE) && defined(CP_ALWAYS_ON_DATA)
    dst = __cp_always_on_data_sram_start;
    src = __cp_always_on_data_sram_start_flash__;
    for (; dst < __cp_always_on_data_sram_end; dst++, src++) {
        *dst = *src;
    }

    dst = __cp_always_on_bss_sram_start;
    for (; dst < __cp_always_on_bss_sram_end; dst++) {
        *dst = 0;
    }
#endif

#ifdef MAIN_RAM_USE_SYS_RAM
    hal_cache_boot_sync_all(HAL_CACHE_ID_D_CACHE);
#else
    __DSB();
    __ISB();
#endif

    for (dst = __sram_bss_start__; dst < __sram_bss_end__; dst++) {
        *dst = 0;
    }

#ifdef RAM_NC_BASE
    for (dst = __sram_nc_data_start__, src = __sram_nc_data_start_flash__;
            src < __sram_nc_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __sram_nc_bss_start__; dst < __sram_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif

#ifdef RAM_EXTRA0_BASE
    for (dst = __ram_extra0_text_data_start__, src = __ram_extra0_text_data_start_flash__;
            src < __ram_extra0_text_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __ram_extra0_bss_start__; dst < __ram_extra0_bss_end__; dst++) {
        *dst = 0;
    }
#endif

#ifdef AON_MAILBOX_BASE
    for (dst = __aon_mbx_bss_start__; dst < __aon_mbx_bss_end__; dst++) {
        *dst = 0;
    }
#endif
}
#endif

#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE) && !defined(COMPRESS_PSRAM_CODE)
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_psram_sections(void)
{
#if (PSRAM_SIZE !=0)
    uint32_t *dst, *src;
    for (dst = __psram_text_data_start__, src = __psram_text_data_start_flash__;
        src < __psram_text_data_end_flash__;
        dst++, src++) {
        *dst = *src;
    }
#if (defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)) || \
        (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
    // Sync D-Cache so that I-Cache can see the changes
    hal_cache_boot_sync_all(HAL_CACHE_ID_D_CACHE);
#endif
    __DSB();
    __ISB();

    for (dst = __psram_bss_start__; dst < __psram_bss_end__; dst++) {
        *dst = 0;
    }

    for (dst = __psram_nc_data_start__, src = __psram_nc_data_start_flash__;
            src < __psram_nc_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __psram_nc_bss_start__; dst < __psram_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif
}
#endif

#if defined(CHIP_HAS_PSRAMUHS) && defined(PSRAMUHS_ENABLE)
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_psramuhs_sections(void)
{
#if (MCU_PSRAMUHS_SIZE !=0)
    uint32_t *dst, *src;
    for (dst = __psramuhs_text_data_start__, src = __psramuhs_text_data_start_flash__;
        src < __psramuhs_text_data_end_flash__;
        dst++, src++) {
        *dst = *src;
    }
#if (defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)) || \
        (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
    // Sync D-Cache so that I-Cache can see the changes
    hal_cache_boot_sync_all(HAL_CACHE_ID_D_CACHE);
#endif
    __DSB();
    __ISB();

    for (dst = __psramuhs_bss_start__; dst < __psramuhs_bss_end__; dst++) {
        *dst = 0;
    }

    for (dst = __psramuhs_nc_data_start__, src = __psramuhs_nc_data_start_flash__;
            src < __psramuhs_nc_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __psramuhs_nc_bss_start__; dst < __psramuhs_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif
}
#endif

#if defined(ARM_CMSE) && (RAM_NSC_SIZE != 0)
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_security_sections(void)
{
    uint32_t *dst, *src;

    for (dst = __sram_nsc_text_start__, src = __sram_nsc_text_start_flash__;
            src < __sram_nsc_text_end_flash__;
            dst++, src++) {
        *dst = *src;
    }
    __DSB();
    __ISB();
}
#endif

void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR BootInit(void)
{
#ifdef MPU_INIT_TABLE
    // Init memory map
    mpu_boot_cfg();
#endif

#ifndef NO_CACHE
    // Enable icache
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    // Enable write buffer
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    // Enable write back
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    // Enable dcache
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif

#if !defined(FPGA)
    // Init GOT base register
    GotBaseInit();
#endif

#if !defined( NANDFLASH_BUILD) && !defined(ROM_BUILD)
    boot_init_boot_sections();
#endif

#if defined( FPGA) || defined(HOST_SIMU)
    hal_cmu_fpga_setup();
#else
    hal_cmu_setup();
#endif

#if !defined(NANDFLASH_BUILD) && !defined(ROM_BUILD)
    boot_init_sram_sections();
#endif

#if defined(ARM_CMSE) && (RAM_NSC_SIZE != 0)
    boot_init_security_sections();
#endif

#ifdef COMMON_IRQ_ENTRY
    // Init common irq entry after initializing all the sram sections
    NVIC_InitCommonIrqEntry();
#endif

#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)
    // Init psram
#if !defined(ARM_CMNS)
    hal_psram_init();
#endif
    boot_init_psram_sections();
#endif

#if defined(CHIP_HAS_PSRAMUHS) && defined(PSRAMUHS_ENABLE)
    // Init psramuhs
#if !defined(ARM_CMNS)
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();
    hal_psramuhs_init();
#endif
    boot_init_psramuhs_sections();
#endif
}
#endif

int subsys_check_boot_struct(uint32_t boot_struct_addr, uint32_t *code_start_addr)
{
    volatile struct boot_hdr_t *boot_hdr;
    uint32_t code_start;
    uint32_t magic;

    boot_hdr = (struct boot_hdr_t *)boot_struct_addr;

#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
    enum HAL_NORFLASH_RET_T ret;
    enum HAL_FLASH_ID_T id;
    uint32_t addr;

    addr = (uint32_t)&boot_hdr->magic;
    id = hal_norflash_addr_to_id(addr);
    ASSERT(id < HAL_FLASH_ID_NUM, "%s: addr not in flash: 0x%08X", __func__, addr);
    ret = hal_norflash_read(id, addr, (uint8_t *)&magic, sizeof(boot_hdr->magic));
    ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to read flash: %d", __func__, ret);
#else
    // Ignore possible first reading error
    boot_hdr->magic;
    hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, (uint32_t)&boot_hdr->magic, sizeof(boot_hdr->magic));
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)&boot_hdr->magic, sizeof(boot_hdr->magic));
    magic = boot_hdr->magic;
#endif

    TR_INFO(0, "%s: boot_addr=%p magic=0x%08X", __func__, boot_hdr, magic);

    if (magic != BOOT_MAGIC_NUMBER) {
        return 1;
    }

    code_start = (uint32_t)(boot_hdr + 1) | 1;

    TR_INFO(0, "%s: code_start=0x%08X", __func__, code_start);

    *code_start_addr = code_start;

    return 0;
}

#ifdef ROM_IN_FLASH
void BOOT_TEXT_FLASH_LOC boot_init_rom_in_flash(void)
{
#ifdef __ARMCC_VERSION
    // Workaround to include .code_start_addr section
    extern uint32_t __code_start_addr[];
    asm volatile("" : : "r"(__code_start_addr));
#endif

#ifndef INTSRAM_RUN
    // Enable icache
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    // Enable dcache
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif

    // Init ram code and data
    boot_init_boot_sections();
    boot_init_sram_sections();
}
#endif

// -----------------------------------------------------------
// Mutex flag
// -----------------------------------------------------------

int BOOT_TEXT_SRAM_LOC mutex_flag_lock(MUTEX_FLAG_T *flag)
{
    MUTEX_FLAG_T busy;

    do {
        busy = (MUTEX_FLAG_T)__LDREXB((unsigned char *)flag);
        if (busy) {
            __CLREX();
            return -1;
        }
    } while (__STREXB(true, (unsigned char *)flag));
    __DMB();

    return 0;
}

void BOOT_TEXT_SRAM_LOC mutex_flag_unlock(MUTEX_FLAG_T *flag)
{
    *flag = false;
    __DMB();
}

// -----------------------------------------------------------
// Misc
// -----------------------------------------------------------

float db_to_float(float db)
{
    float coef;

#if !defined(NOSTD) && defined(ACCURATE_DB_TO_FLOAT)
    // The math lib will consume 4K+ bytes of space
    coef = powf(10, db / 20);
#else
    static const float factor_m9db      = 0.354813389234;
    static const float factor_m3db      = 0.707945784384;
    static const float factor_m1db      = 0.891250938134;
    static const float factor_m0p5db    = 0.944060876286;
    static const float factor_m0p1db    = 0.988553094657;

    coef = 1.0;

    if (db < 0) {
        while (1) {
            if (db <= -9.0) {
                db += 9.0;
                coef *= factor_m9db;
            } else if (db <= -3.0) {
                db += 3.0;
                coef *= factor_m3db;
            } else if (db <= -1.0) {
                db += 1.0;
                coef *= factor_m1db;
            } else if (db <= -0.5) {
                db += 0.5;
                coef *= factor_m0p5db;
            } else if (db <= -0.1 / 2) {
                db += 0.1;
                coef *= factor_m0p1db;
            } else {
                break;
            }
        }
    } else if (db > 0) {
        while (1) {
            if (db >= 9.0) {
                db -= 9.0;
                coef /= factor_m9db;
            } else if (db >= 3.0) {
                db -= 3.0;
                coef /= factor_m3db;
            } else if (db >= 1.0) {
                db -= 1.0;
                coef /= factor_m1db;
            } else if (db >= 0.5) {
                db -= 0.5;
                coef /= factor_m0p5db;
            } else if (db >= 0.1 / 2) {
                db -= 0.1;
                coef /= factor_m0p1db;
            } else {
                break;
            }
        }
    }
#endif

    return coef;
}

uint32_t get_msb_pos(uint32_t val)
{
    uint32_t lead_zero;

    lead_zero = __CLZ(val);
    return (lead_zero >= 32) ? 32 : 31 - lead_zero;
}

uint32_t get_lsb_pos(uint32_t val)
{
    return __CLZ(__RBIT(val));
}

uint32_t integer_sqrt(uint32_t val)
{
    unsigned int msb;
    unsigned int x;
    unsigned int y;

    if (val == 0) {
        return 0;
    }

    for (msb = 31; msb > 0; msb--) {
        if (val & (1 << msb)) {
            break;
        }
    }

    x = ((1 << msb) + 1) / 2;

    while (1) {
        y = (x + val / x) / 2;
        if (y >= x) {
            break;
        }
        x = y;
    }

    return x;
}

uint32_t integer_sqrt_nearest(uint32_t val)
{
    unsigned int s;

    s = integer_sqrt(val * 4);
    return (s + 1) / 2;
}

#ifdef DEBUG_IRQ_LATENCY
#define MAX_IRQ_LATENCY 30 /*30ms*/

BOOT_BSS_LOC
uint32_t irq_lock_time;

BOOT_BSS_LOC
uint32_t irq_g_lock_time;

BOOT_TEXT_SRAM_LOC
void irq_lock_save_ts(int global)
{
    uint32_t time = hal_sys_timer_get();

    if (global)
        irq_g_lock_time = time;
    else
        irq_lock_time = time;
}


BOOT_TEXT_SRAM_LOC
void irq_lock_check_ts(int global)
{
    uint32_t now = hal_sys_timer_get();
    uint32_t dur;

    if (global) {
        dur = __SLIM_TICKS_TO_MS(now - irq_g_lock_time);
        if (dur > MAX_IRQ_LATENCY)
            TRACE(0, "WARNING:irq global lock in pc=%x lr=%x too long %d,", irq_g_masked_addr.pc, irq_g_masked_addr.lr, dur);
    } else {
        dur = __SLIM_TICKS_TO_MS(now - irq_lock_time);
        if (dur > MAX_IRQ_LATENCY)
            TRACE(0, "WARNING:irq lock in pc=%x lr=%x too long %d,", irq_masked_addr.pc, irq_g_masked_addr.lr, dur);
    }
}
#endif
