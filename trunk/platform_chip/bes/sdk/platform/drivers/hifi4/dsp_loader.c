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
#if !(defined(CHIP_ROLE_CP) || defined(CHIP_SUBSYS_SENS) || (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU)))

#include "plat_addr_map.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "tool_msg.h"
#include "dsp_loader.h"
#include "hal_psc.h"
#include "hal_cmu.h"
#ifndef BTH_AS_MAIN_MCU
#include "hal_cache.h"
#endif
#include "hal_rmt_ipc.h"
#include "hal_sysfreq.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "string.h"

#if defined(__NuttX__)
#define OPENAMP_ENABLE
#endif

#if defined(OPENAMP_ENABLE) || defined(SUBSYS_CUSTOMIZED_COPY)
#include "system_subsys.h"
#endif
#ifdef RTOS
#include "cmsis_os.h"
#endif

#include "rx_dsp_hifi4_trc.h"

#ifdef DSP_COMBINE_BIN
extern uint32_t __dsp_hifi4_code_start_flash[];
extern uint32_t __dsp_hifi4_code_end_flash[];
#endif

//#define DSP_BOOT_FROM_FLASH

#ifndef DSP_LOGIC_SROM_BASE
#ifdef BTH_AS_MAIN_MCU
#define DSP_LOGIC_SROM_BASE     (FLASH_BASE + 0x50000)
#else
#define DSP_LOGIC_SROM_BASE     (FLASH_NC_BASE + 0x50000)
#endif
#endif

struct dsp_memory_map_t {
#ifdef DSP_BOOT_FROM_PSRAM
    uint32_t reset_vector_addr;
#endif
    uint32_t iram_start;
    uint32_t iram_end;
    uint32_t dram_start;
    uint32_t dram_end;
    uint32_t sram_start;
    uint32_t sram_end;
};

struct dsp_segments_info_t {
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t store_addr;
};

struct dsp_binary_struct_t {
    struct boot_hdr_t hdr;
    struct dsp_memory_map_t memory_map;
    struct dsp_segments_info_t segments_info[0];
};

static uint32_t dsp_load_base = 0;

static void dsp_wdt_irq_handler(void)
{
    TRACE(0, "%s", __func__);
    NVIC_ClearPendingIRQ(DSP_WDT_IRQn);
    NVIC_DisableIRQ(DSP_WDT_IRQn);
}

static void dsp_wdt_start(void)
{
    NVIC_SetVector(DSP_WDT_IRQn, (uint32_t)dsp_wdt_irq_handler);
    NVIC_SetPriority(DSP_WDT_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(DSP_WDT_IRQn);
    NVIC_EnableIRQ(DSP_WDT_IRQn);;
}

static int POSSIBLY_UNUSED dsp_load_code(uint32_t load_base)
{
    uint32_t *dst_start, *src_start;
    uint32_t *dst, *src, *end;
    struct dsp_binary_struct_t * dsp_boot_struct;
    struct dsp_segments_info_t *segments_loader = NULL;
#ifdef DSP_HIFI4_ADDR_CPU_TO_DEV
    uint32_t *last_dst_start = NULL;
    uint32_t *old_dst_start;
    uint32_t *old_end;
#endif

    dsp_boot_struct = (struct dsp_binary_struct_t *)load_base;
    segments_loader = dsp_boot_struct->segments_info;
    while(segments_loader->start_addr
            && segments_loader->end_addr
            && segments_loader->store_addr) {

        TRACE(0, "start_addr:%08x end_addr:%08x store_addr:%08x @:%08x",
                segments_loader->start_addr,
                segments_loader->end_addr,
                segments_loader->store_addr,
                load_base + (segments_loader->store_addr - DSP_LOGIC_SROM_BASE));

        if (segments_loader->start_addr < segments_loader->end_addr) {
            dst_start = (uint32_t *)segments_loader->start_addr;
            end = (uint32_t *)segments_loader->end_addr;
#ifdef DSP_HIFI4_ADDR_CPU_TO_DEV
            old_dst_start = dst_start;
            old_end = end;
            dst_start = (uint32_t *)dsp_hifi4_addr_cpu_to_dev((uint32_t)dst_start);
            end = (uint32_t *)dsp_hifi4_addr_cpu_to_dev((uint32_t)end);
            // Reduce trace
            if ((uint32_t)dst_start - (uint32_t)last_dst_start >= 0x00100000) {
                TRACE(0, "dst_start: (%08x => %08x) end: (%08x => %08x)",
                    (uint32_t)old_dst_start, (uint32_t)dst_start, (uint32_t)old_end, (uint32_t)end);
            }
            last_dst_start = dst_start;
#endif
            src_start = (uint32_t *)(load_base + (segments_loader->store_addr -
                                                    DSP_LOGIC_SROM_BASE));
            if (0) {
#ifdef SUBSYS_CUSTOMIZED_COPY
            } else if (subsys_customized_copy((void *)dst_start, (void *)src_start, (end - dst_start) * sizeof(uint32_t)) == 0) {
#endif
            } else {
                for (dst = dst_start, src = src_start; dst < end; dst++, src++) {
                    *dst = *src;
                }
#ifdef DSP_MEM_CHECK
                __DSB();
                for (dst = dst_start, src = src_start; dst < end; dst++, src++) {
                    if (*dst != *src){
                        TRACE(0, "%p != %p %08x/%08x", dst, src, *dst, *src);
                    }
                }
#endif
            }
        }
        segments_loader++;
    }
#ifndef BTH_AS_MAIN_MCU
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
#endif
    return 0;
}

static int dsp_check_and_startup(uint32_t load_base, uint32_t load_size)
{
    struct dsp_binary_struct_t *dsp_boot_struct;

    dsp_boot_struct = (struct dsp_binary_struct_t *)load_base;
    TRACE_IMM(0, "%s base:%p size:0x%08x", __func__, dsp_boot_struct, load_size);
    TRACE(0, "binary_struct ver:%04x ", dsp_boot_struct->hdr.version);
#ifdef DSP_BOOT_FROM_PSRAM
    TRACE(0, "hifi reset vector addr:0x%x ", dsp_boot_struct->memory_map.reset_vector_addr);
#endif
    TRACE(0, "iram:%08x~%08x dram:%08x~%08x sram:%08x~%08x",
                            dsp_boot_struct->memory_map.iram_start,
                            dsp_boot_struct->memory_map.iram_end,
                            dsp_boot_struct->memory_map.dram_start,
                            dsp_boot_struct->memory_map.dram_end,
                            dsp_boot_struct->memory_map.sram_start,
                            dsp_boot_struct->memory_map.sram_end);
#if (HIFI4_ITCM_SIZE > 0)
    ASSERT(dsp_boot_struct->memory_map.iram_start == HIFI4_ITCM_BASE, \
            "%s: Bad itcm s-addr=0x%X", __func__, HIFI4_ITCM_BASE);
    ASSERT(dsp_boot_struct->memory_map.iram_end == (HIFI4_ITCM_BASE + HIFI4_ITCM_SIZE), \
            "%s: Bad itcm e-addr=0x%X", __func__, (HIFI4_ITCM_BASE + HIFI4_ITCM_SIZE));
#endif
#if (HIFI4_DTCM_SIZE > 0)
    ASSERT(dsp_boot_struct->memory_map.dram_start == HIFI4_DTCM_BASE, \
            "%s: Bad dtcm s-addr=0x%X", __func__, HIFI4_DTCM_BASE);
    ASSERT(dsp_boot_struct->memory_map.dram_end == (HIFI4_DTCM_BASE + HIFI4_DTCM_SIZE), \
            "%s: Bad dtcm e-addr=0x%X", __func__, (HIFI4_DTCM_BASE + HIFI4_DTCM_SIZE));
#endif
#if(HIFI4_SYS_RAM_SIZE > 0)
    ASSERT(dsp_boot_struct->memory_map.sram_start == HIFI4_SYS_RAM_BASE, \
            "%s: Bad sram s-addr=0x%X", __func__, HIFI4_SYS_RAM_BASE);
    ASSERT(dsp_boot_struct->memory_map.sram_end == (HIFI4_SYS_RAM_BASE + HIFI4_SYS_RAM_SIZE), \
            "%s: Bad sram e-addr=0x%X", __func__, (HIFI4_SYS_RAM_BASE + HIFI4_SYS_RAM_SIZE));
#endif

    dsp_load_base = load_base;

    hal_psc_sys_dsp_enable();
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();
#ifdef DSP_BOOT_FROM_PSRAM
    hal_cmu_dsp_cpu_reset_clear(dsp_boot_struct->memory_map.reset_vector_addr);
#else
    hal_cmu_dsp_cpu_reset_clear(dsp_boot_struct->memory_map.iram_start);
#endif
    osDelay(2);

#if !(defined (DSP_BOOT_FROM_FLASH) || defined (DSP_BOOT_FROM_PSRAM))
    dsp_load_code(load_base);
#endif

#if defined(CHIP_BEST1600) || defined(CHIP_BEST1603)
    uint32_t ipc_mailbox_start = 0;
#if defined(DSP_HIFI4_TRC_TO_MCU) && defined(RMT_TRC_IN_MSG_CHAN)
    ipc_mailbox_start = dsp_boot_struct->memory_map.dram_end - HIFI4_MAILBOX_SIZE + sizeof(struct HAL_RMT_IPC_CFG_T *) * 2;
#else
    ipc_mailbox_start = dsp_boot_struct->memory_map.dram_end - HIFI4_MAILBOX_SIZE;
#endif
#ifdef DSP_HIFI4_ADDR_CPU_TO_DEV
    ipc_mailbox_start = dsp_hifi4_addr_cpu_to_dev(ipc_mailbox_start);
#endif

    *(volatile uint32_t *)(ipc_mailbox_start) = dsp_boot_struct->memory_map.dram_end;

    void *peer_loc, *local_loc;
#ifdef BTH_AS_MAIN_MCU
    peer_loc = (void *)(ipc_mailbox_start);
    local_loc = (void *)(ipc_mailbox_start + sizeof(struct HAL_RMT_IPC_CFG_T *));
#else
    peer_loc = (void *)(ipc_mailbox_start + sizeof(struct HAL_RMT_IPC_CFG_T *));
    local_loc = (void *)(ipc_mailbox_start);
#endif
    hal_rmt_ipc_update_cfg_loc(HAL_RMT_IPC_CORE_SYS_HIFIC0, peer_loc, local_loc);
#endif

    hal_cmu_dsp_start_cpu();

    dsp_wdt_start();

#ifdef DSP_HIFI4_TRC_TO_MCU
    dsp_hifi4_trace_server_open();
#endif
    return 0;
}

#ifdef DSP_PRELOAD_BIN
//dsp bin need preload from fs\emmc\nand\etc
/*
static int dsp_preload_bin(uint32_t *code_start_addr, uint32_t *code_size)
{
// add some code for mv dsp bin to right base
    *code_start_addr = 0;
    *code_size = 0;
}
*/
#elif DSP_COMBINE_BIN
static int dsp_preload_bin(uint32_t *code_start_addr, uint32_t *code_size)
{
#ifdef DSP_BOOT_FROM_PSRAM
    uint8_t *src = (uint8_t *)__dsp_hifi4_code_start_flash;
    uint8_t *dst = (uint8_t *)HIFI4_PSRAM_BASE;
    memcpy(dst, src, HIFI4_PSRAM_SIZE);
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

    *code_start_addr = HIFI4_PSRAM_BASE;
    *code_size = HIFI4_PSRAM_SIZE;
#else
    *code_start_addr = (uint32_t)__dsp_hifi4_code_start_flash;
    *code_size = (uint32_t)__dsp_hifi4_code_end_flash - (uint32_t)__dsp_hifi4_code_start_flash;
#endif
    return 0;
}
#else
//dsp bin need preload from norflash to psram or xip
static int dsp_preload_bin(uint32_t *code_start_addr, uint32_t *code_size)
{
#ifdef DSP_BOOT_FROM_PSRAM
    uint8_t *src = (uint8_t *)DSP_LOGIC_SROM_BASE;
    uint8_t *dst = (uint8_t *)HIFI4_PSRAM_BASE;
    memcpy(dst, src, HIFI4_PSRAM_SIZE);
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

    *code_start_addr = HIFI4_PSRAM_BASE;
    *code_size = HIFI4_PSRAM_SIZE;
#else
    *code_start_addr = DSP_LOGIC_SROM_BASE;
    *code_size = 0;
#endif
    return 0;
}
#endif

static int dsp_shutdown(void)
{
#ifdef DSP_HIFI4_TRC_TO_MCU
    dsp_hifi4_trace_server_close();
#endif

    hal_cmu_dsp_reset_set();
    hal_cmu_dsp_clock_disable();
    hal_psc_sys_dsp_disable();

    return 0;
}

static bool dsp_hifi4_inited = false;

int dsp_open(void)
{
    uint32_t code_start_addr;
    uint32_t code_size;
    if (dsp_hifi4_inited)
        return 0;

    dsp_preload_bin(&code_start_addr, &code_size);
    TRACE_IMM(0, "%s, %d code_start/0x%x,code_size/0x%x", __func__, __LINE__, code_start_addr, code_size);
    dsp_check_and_startup(code_start_addr, code_size);

    dsp_hifi4_inited = true;
    TRACE_IMM(0, "%s, %d", __func__, __LINE__);

    return 0;
}

int dsp_close(void)
{
    if (!dsp_hifi4_inited) {
        return 0;
    }

    dsp_shutdown();

    dsp_hifi4_inited = false;
    return 0;
}

uint32_t dsp_get_load_base(void)
{
    if (!dsp_load_base) {
        ASSERT(0, "%s: Bad load base", __func__);
    }

    return dsp_load_base;
}

#endif
