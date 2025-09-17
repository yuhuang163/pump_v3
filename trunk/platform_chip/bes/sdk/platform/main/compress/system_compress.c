/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "bes_lzma_api.h"
#include "heap_api.h"

#define MCU_LZMA_BUF_SIZE (16*1024)

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
// CAUTION:
// O2 option in GCC 10 will optimize the copy loop to an invokation of memcpy,
// whereas memcpy might be located in RAM and not initialized yet.
#define BOOT_CODE_OPT_ATTR                  __attribute__((optimize("no-tree-loop-distribute-patterns")))
#else
#define BOOT_CODE_OPT_ATTR
#endif

extern uint32_t __sram_text_data_start__[];
extern uint32_t __sram_data_start__[];
extern uint32_t __sram_bss_start__[];
extern uint32_t __sram_bss_end__[];
extern uint32_t __sram_nc_data_start_flash__[];
extern uint32_t __sram_nc_data_end_flash__[];
extern uint32_t __sram_nc_data_start__[];
extern uint32_t __sram_nc_data_end__[];
extern uint32_t __sram_nc_bss_start__[];
extern uint32_t __sram_nc_bss_end__[];
extern uint32_t __fast_sram_text_data_start__[];
extern uint32_t __fast_sram_text_data_end__[];
extern uint32_t __cp_text_sram_start[];
extern uint32_t __cp_text_sram_end[];
extern uint32_t __cp_data_sram_start[];
extern uint32_t __cp_data_sram_end[];
extern uint32_t __cp_bss_sram_start[];
extern uint32_t __cp_bss_sram_end[];

#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)
extern uint32_t __psram_text_data_start__[];
extern uint32_t __psram_text_data_start_flash__[];
extern uint32_t __psram_text_data_end_flash__[];
extern uint32_t __psram_bss_start__[];
extern uint32_t __psram_bss_end__[];
extern uint32_t __psram_nc_bss_start__[];
extern uint32_t __psram_nc_bss_end__[];
#endif

#ifdef COMPRESS_SRAM_CODE
extern uint32_t __lzma_mcu_sram_bin_start[];
extern uint32_t __lzma_mcu_sram_bin_end[];
extern uint32_t __lzma_mcu_fsram_bin_start[];
extern uint32_t __lzma_mcu_fsram_bin_end[];
extern uint32_t __lzma_mcu_cptext_bin_start[];
extern uint32_t __lzma_mcu_cptext_bin_end[];
extern uint32_t __lzma_mcu_cpdata_bin_start[];
extern uint32_t __lzma_mcu_cpdata_bin_end[];
#endif
#ifdef COMPRESS_PSRAM_CODE
extern uint32_t __lzma_mcu_psram_bin_start[];
extern uint32_t __lzma_mcu_psram_bin_end[];
#endif

#ifdef COMPRESS_SRAM_CODE
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_sram_sections(void)
{
    uint32_t *dst;

    uint32_t uncompressed_size = 0;
    uint32_t size = (uint32_t)__lzma_mcu_sram_bin_end - (uint32_t)__lzma_mcu_sram_bin_start;
    bes_lzma_dec_code((unsigned char *)__sram_text_data_start__, &uncompressed_size,
        (unsigned char *)__lzma_mcu_sram_bin_start, size);

    uint32_t uncompressed_size_fa = 0;
    uint32_t size_fa = (uint32_t)__lzma_mcu_fsram_bin_end - (uint32_t)__lzma_mcu_fsram_bin_start;
    bes_lzma_dec_code((unsigned char *)__fast_sram_text_data_start__, &uncompressed_size_fa,
        (unsigned char *)__lzma_mcu_fsram_bin_start, size_fa);

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
    uint32_t *src;

    for (dst = __sram_nc_data_start__, src = __sram_nc_data_start_flash__;
            src < __sram_nc_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __sram_nc_bss_start__; dst < __sram_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif
}

void boot_init_sram_sections_cp(int load_code)
{
    uint32_t *dst;
    uint32_t uncompressed_size = 0;
    uint32_t size = 0;
    if (load_code) {
        size = (uint32_t)__lzma_mcu_cptext_bin_end - (uint32_t)__lzma_mcu_cptext_bin_start;
        bes_lzma_dec_code((unsigned char *)__cp_text_sram_start, &uncompressed_size,
            (unsigned char *)__lzma_mcu_cptext_bin_start, size);
    }
    size = (uint32_t)__lzma_mcu_cpdata_bin_end - (uint32_t)__lzma_mcu_cpdata_bin_start;
    bes_lzma_dec_code((unsigned char *)__cp_data_sram_start, &uncompressed_size,
        (unsigned char *)__lzma_mcu_cpdata_bin_start, size);

    dst = __cp_bss_sram_start;
    for (; dst < __cp_bss_sram_end; dst++) {
        *dst = 0;
    }
}
#endif

#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE) && defined(COMPRESS_PSRAM_CODE)
void BOOT_TEXT_FLASH_LOC BOOT_CODE_OPT_ATTR boot_init_psram_sections(void)
{
#if (PSRAM_SIZE !=0)
    uint32_t *dst, *src;

    uint32_t uncompressed_size = 0;
    uint32_t size = (uint32_t)__lzma_mcu_psram_bin_end - (uint32_t)__lzma_mcu_psram_bin_start;
    bes_lzma_dec_code((unsigned char *)__psram_text_data_start__, &uncompressed_size,
        (unsigned char *)__lzma_mcu_psram_bin_start, size);

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

#ifdef __ARM_ARCH_ISA_ARM
#ifdef COMPRESS_DSP_CODE
#include "machine/endian.h"
#define be32toh __ntohl
#include "hal_sysfreq.h"
extern uint32_t __text_start[];
extern uint32_t heap_start[];
void dsp_decompress(void)
{
    uint32_t *dst, *src;
    uint32_t uncompressed_size = 0;
    unsigned char *_src = (unsigned char *)heap_start;
    uint32_t __dsp_code2_size = be32toh(*((uint32_t *)_src));
    uint32_t dsp_code2_start = (uint32_t)(_src+4);
    ASSERT((__dsp_code2_size < OS_HEAP_SIZE), "%s dsp lzma bin size=0x%X should < OS_HEAP_SIZE=0x%X\n", __FUNCTION__, __dsp_code2_size, OS_HEAP_SIZE)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_780M);
    int ret = bes_lzma_dec_code((unsigned char *)__text_start, &uncompressed_size, (unsigned char *)dsp_code2_start, __dsp_code2_size);
    if (ret != 0) {
        return;
    }
    // empty heap
    memset((void *)heap_start, 0, OS_HEAP_SIZE);
}
#endif
#endif
