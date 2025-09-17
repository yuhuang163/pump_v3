/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "plat_types.h"
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "tool_msg.h"
#include "pmu.h"
#include "mem_scan.h"
#include "watchdog.h"
#include "hal_location.h"
#include "hal_bootmode.h"
#include "nvrecord_extension.h"

#define MTEST_BOOT_MODE HAL_SW_BOOTMODE_ROM_RESERVED_17

extern uint32_t __mem_scan_start_flash[];
extern uint32_t __mem_scan_end_flash[];

typedef void (*RAM_ENTRY)(void);

static BOOT_TEXT_FLASH_LOC NORETURN void __mem_scan_open(void)
{
    uint32_t seg_size;
    RAM_ENTRY entry;

    seg_size = (uint32_t)( __mem_scan_end_flash - 2) - (uint32_t) __mem_scan_start_flash;
    /*
     * code offset locate in the end of program_boot_struct_sz, but this structure is not
     * export, and builtin the binary
     */
    uint32_t code_offs = *(__mem_scan_end_flash - 2);
    uint32_t src = (uint32_t)__mem_scan_start_flash + code_offs;

    uint32_t dst = *(__mem_scan_end_flash - 1);
    uint32_t len = seg_size/4;
    uint32_t dst_orig = dst;
    while (len--) {
        *(uint32_t *)dst = *(uint32_t *)src;
        src += 4;
        dst += 4;
    }

    entry = (RAM_ENTRY)((uint32_t)((struct exec_struct_t *)dst_orig)->entry | 1);

    //start mem_test_pgm.bin
    entry();

    SAFE_PROGRAM_STOP();
}

#if defined(CHIP_BEST1600)
BOOT_TEXT_FLASH_LOC NAKED void mem_scan_open(void)
{
    uint32_t boot_mode;
    register uint32_t saved_lr __asm__("r4");

    saved_lr = (uint32_t)__builtin_return_address(0);

    boot_mode = hal_sw_bootmode_get();

    if(boot_mode & MTEST_BOOT_MODE) {
        __mem_scan_open();
    } else {
        asm volatile ("mov lr, %0                          \n"
                      "ldr r5, =hal_cmu_boot_m55_start_bth \n"
                      "bx r5                               \n"
                     ::"r"(saved_lr):);
    }
}

#if 0
static POSSIBLY_UNUSED uint32_t mem_scan_get_result(void)
{
    return pmu_memtest_get_flag();
}
#endif

/*
 * this should start memory scan in bootloader
 */
void boot_loader_entry_hook(void) __attribute__((alias("mem_scan_open")));
void BOOT_TEXT_FLASH_LOC NORETURN hal_cmu_boot_m55_start_bth(void);

#else

/*
 * Why this register?
 * The targets of mem_test and normal target (such as best2500p_ep) define their
 * own reboot_param, and the address of reboot_param maybe different.
 * This register must be located in normal target's reboot_param section;
 * mem_test store value to this register and normal code check the value;
 */
#define MTEST_RES_SAVE_REG (0x20040150)
static uint32_t mem_scan_get_persistent_result(void)
{
    return *(volatile uint32_t*)MTEST_RES_SAVE_REG;
}

BOOT_TEXT_FLASH_LOC void mem_scan_open(void)
{
    if (!(hal_sw_bootmode_get() & MTEST_BOOT_MODE)) {
        return;
    }
    hal_sw_bootmode_clear(MTEST_BOOT_MODE);
    __mem_scan_open();
    return;
}

void boot_loader_pre_init_hook(void) __attribute__((alias("mem_scan_open")));
#endif

void mem_scan_start(void)
{
    watchdog_hw_stop();
    hal_sw_bootmode_set(MTEST_BOOT_MODE);
    pmu_reboot();
}

#if !defined(MTEST_BUILTIN_OTA)

#include "hal_cmu.h"

/*
 * check the memory scan result
 */
mscan_result_t mem_scan_check(void)
{
    uint32_t val;

    //read from flash
    uint32_t mem_res = nv_extention_get_mem_scan_result();
    if (mem_res == CMU_SIMU_RES_PASSED) {
        TRACE_IMM(0, "####mem test in flash success%x", mem_res);
        return MEM_SCAN_SUCCESS;
    } else if (mem_res == CMU_SIMU_RES_FAILED) {
        TRACE_IMM(0, "####mem test in flash fail%x", mem_res);
        return MEM_SCAN_FAILED;
    } else {
        val = mem_scan_get_persistent_result();
        TRACE_IMM(0, "####mem test result %x", val);
        if ((val == CMU_SIMU_RES_PASSED) || (val == CMU_SIMU_RES_FAILED)) {
            nv_extention_set_mem_scan_result(val);
        }

        if (val == CMU_SIMU_RES_PASSED) {
            return MEM_SCAN_SUCCESS;
        } else if (val == CMU_SIMU_RES_FAILED) {
            return MEM_SCAN_FAILED;
        } else {
            return MEM_SCAN_INVALID;
        }
    }
}

/*
 * check the memory scan result, if never did memory scan, do it...
 */
void mem_scan_check_and_start(void)
{
    mscan_result_t mscan_res;

    mscan_res = mem_scan_check();

    if (mscan_res == MEM_SCAN_SUCCESS) {
        TRACE_IMM(0, "mem scan result ok");
    } else if (mscan_res == MEM_SCAN_FAILED) {
        TRACE_IMM(0, "mem scan result fail");
    } else {
        TRACE_IMM(0, "mem scan never done, start");
        mem_scan_start();
    }
}
#endif

