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
#ifndef __ARM_ARCH_ISA_ARM

#include "cmsis_nvic.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_location.h"
#include "hal_sleep_core_pd.h"
#include "hal_trace.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "mpu_cfg.h"
#include "plat_types.h"
#include "plat_addr_map.h"
#include "string.h"
#include "system_subsys.h"
#include "tool_msg.h"

#ifdef SUBSYS_LOAD_FROM_FS
#include "errno.h"
#include "stdio.h"
#endif

#define SUBSYS_IMAGE_DESC_VERSION               0

#define SUBSYS_IMAGE_TYPE_SIMPLE                0
#define SUBSYS_IMAGE_TYPE_SEGMENT               1

#define SUBSYS_IMAGE_COMPRESS_NULL              0
#define SUBSYS_IMAGE_COMPRESS_LZMA              1

#if defined(CHIP_ROLE_CP) || defined(CHIP_SUBSYS_SENS) || \
        defined(CHIP_SUBSYS_CSP) || \
        (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU))

extern uint32_t __code_start_offset[];
extern uint32_t __flash_image_size[];
extern uint32_t __code_seg_map_size[];
extern uint32_t __sram_text_start_load_offset__[];
extern uint32_t __sram_text_data_start__[];
extern uint32_t __sram_text_data_size__[];
extern uint32_t __sram_text_seg_size__[];
extern uint32_t __sram_rodata_start__[];
extern uint32_t __sram_rodata_start_load_offset__[];
extern uint32_t __sram_data_seg_size__[];
extern uint32_t __sram_data_start__[];
extern uint32_t __sram_data_end__[];
extern uint32_t __sram_data_start_load__[];
extern uint32_t __text_data_start__[];
extern uint32_t __text_start_load_offset__[];
extern uint32_t __text_seg_size__[];
extern uint32_t __data_start__[];
extern uint32_t __data_start_load_offset__[];
extern uint32_t __data_seg_size__[];

extern uint32_t __boot_bss_sram_start__[];
extern uint32_t __boot_bss_sram_end__[];

#ifdef RAM_NC_BASE
extern uint32_t __sram_nc_data_start_flash__[];
extern uint32_t __sram_nc_data_end_flash__[];
extern uint32_t __sram_nc_data_start__[];
extern uint32_t __sram_nc_data_end__[];
extern uint32_t __sram_nc_data_start_load_offset__[];
extern uint32_t __sram_nc_data_size__[];
extern uint32_t __sram_nc_bss_start__[];
extern uint32_t __sram_nc_bss_end__[];
#endif

#ifdef AON_MAILBOX_BASE
extern uint32_t __aon_mbx_data_start_load__[];
extern uint32_t __aon_mbx_data_end_load__[];
extern uint32_t __aon_mbx_data_start__[];
extern uint32_t __aon_mbx_data_start_load_offset__[];
extern uint32_t __aon_mbx_data_size__[];
extern uint32_t __aon_mbx_bss_start__[];
extern uint32_t __aon_mbx_bss_end__[];
#endif

#ifdef SUBSYS_FLASH_BOOT
#define RESET_HANDLER_LOC_SUBSYS                __attribute__((section(".boot_loader")))
#else // !SUBSYS_FLASH_BOOT
#define RESET_HANDLER_LOC_SUBSYS                __attribute__((section(".reset_handler_subsys")))
#define IMG_DESC_LOC                            __attribute__((section(".img_desc_subsys")))
#define CODE_SEG_MAP_LOC                        __attribute__((section(".code_seg_map")))

#if defined(SEPARATE_RAM_RAMX) && !defined(SUBSYS_IMAGE_SEGMENT)
#define SUBSYS_IMAGE_SEGMENT
#endif
#if (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
#if (defined(MAIN_RAM_USE_TCM) || defined(RAM_NC_BASE) || defined(AON_MAILBOX_BASE)) && !defined(SUBSYS_IMAGE_SEGMENT)
#define SUBSYS_IMAGE_SEGMENT
#endif
#if defined(MAIN_RAM_USE_SYS_RAM) && !defined(RAM_NC_BASE) && defined(SUBSYS_IMAGE_SEGMENT)
#error "MAIN_RAM_USE_SYS_RAM w/o RAM_NC_BASE conflicts with SUBSYS_IMAGE_SEGMENT"
#endif
#endif

IMG_DESC_LOC
const struct SUBSYS_IMAGE_DESC_T subsys_image_desc = {
    .version = SUBSYS_IMAGE_DESC_VERSION,
#ifdef SUBSYS_IMAGE_SEGMENT
    .type = SUBSYS_IMAGE_TYPE_SEGMENT,
#else
    .type = SUBSYS_IMAGE_TYPE_SIMPLE,
#endif
    .compress = SUBSYS_IMAGE_COMPRESS_NULL,
    .image_size = (uint32_t)__flash_image_size,
    .code_start_offset = (uint32_t)__code_start_offset,
    .u = {
#ifdef SUBSYS_IMAGE_SEGMENT
        .seg_map_size = (uint32_t)__code_seg_map_size,
#else
        .exec_addr = (uint32_t)__sram_text_data_start__,
#endif
    },
};

#ifdef SUBSYS_IMAGE_SEGMENT
CODE_SEG_MAP_LOC
const struct CODE_SEG_MAP_ITEM_T code_seg_map[] = {
#ifdef MAIN_RAM_USE_SYS_RAM
    {
        .exec_addr = (uint32_t)__sram_text_data_start__,
        .load_offset = (uint32_t)__sram_text_start_load_offset__,
        .size = (uint32_t)__sram_text_data_size__,
    },

#else // MAIN_RAM_USE_SYS_RAM
    {
        .exec_addr = (uint32_t)__sram_text_data_start__,
        .load_offset = (uint32_t)__sram_text_start_load_offset__,
        .size = (uint32_t)__sram_text_seg_size__,
    },

    {
        .exec_addr = (uint32_t)__text_data_start__,
        .load_offset = (uint32_t)__text_start_load_offset__,
        .size = (uint32_t)__text_seg_size__,
    },

    {
        .exec_addr = (uint32_t)__sram_rodata_start__,
        .load_offset = (uint32_t)__sram_rodata_start_load_offset__,
        .size = (uint32_t)__sram_data_seg_size__,
    },

    {
        .exec_addr = (uint32_t)__data_start__,
        .load_offset = (uint32_t)__data_start_load_offset__,
        .size = (uint32_t)__data_seg_size__,
    },
#endif // MAIN_RAM_USE_SYS_RAM

#ifdef RAM_NC_BASE
    {
        .exec_addr = (uint32_t)__sram_nc_data_start__,
        .load_offset = (uint32_t)__sram_nc_data_start_load_offset__,
        .size = (uint32_t)__sram_nc_data_size__,
    },
#endif

#ifdef AON_MAILBOX_BASE
    {
        .exec_addr = (uint32_t)__aon_mbx_data_start__,
        .load_offset = (uint32_t)__aon_mbx_data_start_load_offset__,
        .size = (uint32_t)__aon_mbx_data_size__,
    },
#endif
};

extern uint32_t __code_seg_map_start[];
extern uint32_t __code_seg_map_end[];
extern uint32_t __code_seg_map_offset[];

SUBSYS_SEGMENT_LOC
const struct SUBSYS_SEGMENT code_seg = {
    .seg_name = ".code_seg_map",
    .vaddr_start = (uint32_t)__code_seg_map_start,
    .vaddr_end = (uint32_t)__code_seg_map_end,
    .laddr_offset = (uint32_t)__code_seg_map_offset,
};

#endif // SUBSYS_IMAGE_SEGMENT
#endif // !SUBSYS_FLASH_BOOT

void NAKED system_subsys_start(void)
{
    asm volatile (
        "ldr r3, =" TO_STRING(__StackTop) ";"
        "msr msp, r3;"
#ifdef __ARM_ARCH_8M_MAIN__
        "ldr r0, =" TO_STRING(__StackLimit) ";"
        "msr msplim, r0;"
#endif
        "movs r4, 0;"
        "mov r5, r4;"
        "mov r6, r4;"
        "mov r7, r4;"
        "mov r8, r4;"
        "mov r9, r4;"
        "mov r10, r4;"
        "mov r11, r4;"
        "mov r12, r4;"
#if !defined(__SOFTFP__) && defined(__ARM_FP) && (__ARM_FP >= 4)
        "ldr.w r0, =0xE000ED88;"
        "ldr r1, [r0];"
        "orr r1, r1, #(0xF << 20);"
        "str r1, [r0];"
        "dsb;"
        "isb;"
        "vmov s0, s1, r4, r5;"
        "vmov s2, s3, r4, r5;"
        "vmov s4, s5, r4, r5;"
        "vmov s6, s7, r4, r5;"
        "vmov s8, s9, r4, r5;"
        "vmov s10, s11, r4, r5;"
        "vmov s12, s13, r4, r5;"
        "vmov s14, s15, r4, r5;"
        "vmov s16, s17, r4, r5;"
        "vmov s18, s19, r4, r5;"
        "vmov s20, s21, r4, r5;"
        "vmov s22, s23, r4, r5;"
        "vmov s24, s25, r4, r5;"
        "vmov s26, s27, r4, r5;"
        "vmov s28, s29, r4, r5;"
        "vmov s30, s31, r4, r5;"
#endif

        "bl system_subsys_init;"

#ifdef SUBSYS_FLASH_BOOT
        "ldr r1, =" TO_STRING(__etext) ";"
        "ldr r2, =" TO_STRING(__data_start__) ";"
        "ldr r3, =" TO_STRING(__data_end__) ";"
        "_loop_data:;"
        "cmp r2, r3;"
        "ittt lt;"
        "ldrlt r0, [r1], #4;"
        "strlt r0, [r2], #4;"
        "blt _loop_data;"
#endif

#if defined(NOSTD) || defined(__ARMCC_VERSION) || defined(__NuttX__)
        "ldr r1, =" TO_STRING(__bss_start__) ";"
        "ldr r2, =" TO_STRING(__bss_end__) ";"
        "movs r0, 0;"
        "_loop_bss:;"
        "cmp r1, r2;"
        "itt lt;"
        "strlt r0, [r1], #4;"
        "blt _loop_bss;"
#endif

#if defined(__ARMCC_VERSION) && !defined(NOSTD)
        "bl __rt_entry;"
#else
        "bl _start;"
#endif
    );
}

void NAKED RESET_HANDLER_LOC_SUBSYS system_subsys_reset_handler(void)
{
    // Jump to code address space
    asm volatile (
        "ldr r3, =system_subsys_start;"
        "bx r3;"
    );
}
#ifdef SUBSYS_FLASH_BOOT
void Boot_Loader(void) __attribute__((alias("system_subsys_reset_handler")));
#else
void Reset_Handler(void) __attribute__((alias("system_subsys_reset_handler")));
#endif

void system_subsys_init(void)
{
    POSSIBLY_UNUSED uint32_t *dst, *src;
    NVIC_InitVectors();

    SystemInit();

#ifdef MPU_INIT_TABLE
    // Init memory map
    mpu_boot_cfg();
#endif
#if defined(SUBSYS_CACHE_ENABLE) || defined(SUBSYS_FLASH_BOOT) || defined(MAIN_RAM_USE_SYS_RAM)
    // Enable icache
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    // Enable write buffer
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    // Enable write back
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    // Enable dcache
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif

#ifdef SUBSYS_FLASH_BOOT
    boot_init_boot_sections();
#else
    for (dst = __boot_bss_sram_start__; dst < __boot_bss_sram_end__; dst++) {
        *dst = 0;
    }

#ifdef RAM_NC_BASE
    for (dst = __sram_nc_bss_start__; dst < __sram_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif

#ifdef AON_MAILBOX_BASE
    for (dst = __aon_mbx_bss_start__; dst < __aon_mbx_bss_end__; dst++) {
        *dst = 0;
    }
#endif
#endif

#ifdef CORE_SLEEP_POWER_DOWN
    hal_cmu_set_wakeup_vector(SCB->VTOR);
    NVIC_SetResetHandler(hal_sleep_core_power_up);
#endif

#if defined(SUBSYS_LOAD_SRAM_DATA)
#if defined(CHIP_BEST2007P) && defined(CHIP_SUBSYS_CSP)
    hal_cmu_csp_wait_mcu_active();
#endif
    for (dst = __sram_data_start__, src = __sram_data_start_load__;
            dst < __sram_data_end__; dst++, src++) {
        *dst = *src;
    }
#endif

    hal_cmu_subsys_setup();

#ifdef SUBSYS_FLASH_BOOT
    boot_init_sram_sections();
#endif
#if defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)
    boot_init_psram_sections();
#endif
}

void system_subsys_term(void)
{
}

#else // !(CHIP_SUBSYS_SENS || (CHIP_SUBSYS_BTH ^ BTH_AS_MAIN_MCU))

static int subsys_loader_check_image(uint32_t addr, uint32_t len, int check_magic)
{
    const struct boot_hdr_t *hdr;
    const struct SUBSYS_IMAGE_DESC_T *desc;
    uint32_t hdr_size;
    uint32_t seg_map_size;

    hdr_size = sizeof(*hdr) + sizeof(*desc);
    if (len < hdr_size) {
        TRACE(0, "%s: Bad len=%u", __func__, len);
        return 1;
    }

    hdr = (const struct boot_hdr_t *)addr;
    if (check_magic && hdr->magic != BOOT_MAGIC_NUMBER) {
        TRACE(0, "%s: Bad magic=0x%08X", __func__, hdr->magic);
        return 2;
    }

    desc = (const struct SUBSYS_IMAGE_DESC_T *)(hdr + 1);
    TRACE(0, "%s: version=%u type=%d image_size=%u", __func__, desc->version, desc->type, desc->image_size);
    TRACE(0, "%s: code_start_offset=%u exec_addr/seg_map_size=0x%08X", __func__, desc->code_start_offset, desc->u.exec_addr);

    if (desc->version != SUBSYS_IMAGE_DESC_VERSION) {
        TRACE(0, "%s: Bad version=%u", __func__, desc->version);
        return 3;
    }

    if (desc->type == SUBSYS_IMAGE_TYPE_SIMPLE) {
        seg_map_size = 0;
    } else if (desc->type == SUBSYS_IMAGE_TYPE_SEGMENT) {
        seg_map_size = desc->u.seg_map_size;
    } else {
        TRACE(0, "%s: Bad type=%u", __func__, desc->type);
        return 4;
    }

    if (desc->type == SUBSYS_IMAGE_TYPE_SEGMENT) {
        if (seg_map_size == 0) {
            TRACE(0, "%s: seg_map_size cannot be 0 for type=%u", __func__, desc->type);
            return 5;
        }
    }

    if (desc->image_size < desc->code_start_offset) {
        TRACE(0, "%s: Bad image_size=%u code_start_offset=%u", __func__, desc->image_size, desc->code_start_offset);
        return 6;
    }

    return 0;
}

static const struct SUBSYS_IMAGE_DESC_T *subsys_loader_get_image_desc(uint32_t addr)
{
    const struct boot_hdr_t *hdr;

    hdr = (const struct boot_hdr_t *)addr;
    return (const struct SUBSYS_IMAGE_DESC_T *)(hdr + 1);
}

static const struct CODE_SEG_MAP_ITEM_T *subsys_loader_get_seg_map(uint32_t addr, struct SUBSYS_SEGMENT *seg)
{
    const struct boot_hdr_t *hdr;
    const struct SUBSYS_IMAGE_DESC_T *desc;

    hdr = (const struct boot_hdr_t *)addr;
    desc = (const struct SUBSYS_IMAGE_DESC_T *)(hdr + 1);
    if (desc->type != SUBSYS_IMAGE_TYPE_SEGMENT) {
        return NULL;
    }
    return (const struct CODE_SEG_MAP_ITEM_T *)((uint32_t)addr + seg->laddr_offset);
}

static int load_simple_image(const struct SUBSYS_IMAGE_DESC_T *desc, uint32_t addr,
                             SUBSYS_EXEC_ADDR_CONV_CB conv_cb, uint32_t *code_start_addr)
{
    int ret = 0;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_RET_T dma_ret;
    uint32_t load_start;
    uint32_t code_start;
    uint32_t remains;

    if (desc->type != SUBSYS_IMAGE_TYPE_SIMPLE) {
        TR_INFO(0, "%s: Bad image type: %d", __func__, desc->type);
        return 2;
    }

    load_start = addr + desc->code_start_offset;
    code_start = desc->u.exec_addr;
    if (conv_cb) {
        uint32_t old_code_start;

        old_code_start = code_start;
        code_start = conv_cb(old_code_start);
        TR_INFO(0, "%s: code_start 0x%08X => 0x%08X", __func__, old_code_start, code_start);
    }
    remains = (desc->image_size - desc->code_start_offset) / sizeof(uint32_t);

    TR_INFO(0, "%s: load_start=0x%08X code_start=0x%08X remains=0x%X (%u)", __func__, load_start, code_start, remains, remains);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_periph = HAL_GPDMA_MEM;
    dma_cfg.ch = hal_dma_get_chan(dma_cfg.dst_periph, HAL_DMA_LOW_PRIO);
    if (0) {
#ifdef SUBSYS_CUSTOMIZED_COPY
    } else if (subsys_customized_copy((void *)code_start, (void *)load_start, remains * sizeof(uint32_t)) == 0) {
#endif
    } else if (dma_cfg.ch == HAL_DMA_CHAN_NONE) {
        memcpy((void *)code_start, (void *)load_start, remains * sizeof(uint32_t));
    } else {
        dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
        dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.handler = NULL;
        dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
        dma_cfg.src_periph = HAL_GPDMA_MEM;
        dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
        dma_cfg.try_burst = false;

        dma_cfg.dst = code_start;
        dma_cfg.src = load_start;

        while (remains > 0) {
            if (remains > HAL_DMA_MAX_DESC_XFER_SIZE) {
                dma_cfg.src_tsize = HAL_DMA_MAX_DESC_XFER_SIZE;
            } else {
                dma_cfg.src_tsize = remains;
            }
            dma_ret = hal_dma_start(&dma_cfg);
            if (dma_ret != HAL_DMA_OK) {
                TR_INFO(0, "%s: Failed to start dma: %d", __func__, dma_ret);
                ret = 21;
                break;
            }

            while (hal_dma_chan_busy(dma_cfg.ch));

            dma_cfg.dst += dma_cfg.src_tsize * 4;
            dma_cfg.src += dma_cfg.src_tsize * 4;
            remains -= dma_cfg.src_tsize;
        }

        hal_dma_free_chan(dma_cfg.ch);
    }

    if (ret) {
        return ret;
    }

    *code_start_addr = code_start;

    return 0;
}

int load_segment_image(const struct SUBSYS_IMAGE_DESC_T *desc, uint32_t addr,
                       SUBSYS_EXEC_ADDR_CONV_CB conv_cb, uint32_t *code_start_addr)
{
    int ret = 0;
    const struct CODE_SEG_MAP_ITEM_T *seg_item;
    struct HAL_DMA_CH_CFG_T dma_cfg;
    enum HAL_DMA_RET_T dma_ret;
    uint32_t seg_cnt;
    uint32_t i;
    uint32_t remains;
    uint32_t dst;
    uint32_t src;
    uint32_t entry;

    if (desc->type != SUBSYS_IMAGE_TYPE_SEGMENT) {
        TR_INFO(0, "%s: Bad image type: %d", __func__, desc->type);
        return 2;
    }

    struct SUBSYS_SEGMENT *code_seg = subsys_loader_get_segment(addr, ".code_seg_map");
    if (!code_seg) {
        TR_INFO(0, "%s: Bad seg", __func__);
        return 3;
    }
    seg_cnt = (code_seg->vaddr_end - code_seg->vaddr_start) / sizeof(struct CODE_SEG_MAP_ITEM_T);
    TR_INFO(0, "%s: seg_size=0x%X seg_cnt=%u", __func__, desc->u.seg_map_size, seg_cnt);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_periph = HAL_GPDMA_MEM;
    dma_cfg.ch = hal_dma_get_chan(dma_cfg.dst_periph, HAL_DMA_LOW_PRIO);
    if (dma_cfg.ch != HAL_DMA_CHAN_NONE) {
        dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
        dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.handler = NULL;
        dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
        dma_cfg.src_periph = HAL_GPDMA_MEM;
        dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
        dma_cfg.try_burst = false;
    }

    seg_item = subsys_loader_get_seg_map(addr, code_seg);

    // Entry is in the first segment
    entry = seg_item->exec_addr | 1;

    for (i = 0; i < seg_cnt; i++) {
        if ((seg_item->load_offset & 3) || (seg_item->size & 3)) {
            TR_INFO(0, "%s: Bad alignment: load_offset=0x%X size=0x%X", __func__, seg_item->load_offset, seg_item->size);
            ret = 11;
            break;
        }
        if (desc->code_start_offset > seg_item->load_offset || seg_item->load_offset >= desc->image_size) {
            TR_INFO(0, "%s: Bad load_offset=0x%X", __func__, seg_item->load_offset);
            ret = 12;
            break;
        }
        if (seg_item->load_offset + seg_item->size > desc->image_size) {
            TR_INFO(0, "%s: Bad load_offset=0x%X size=0x%X", __func__, seg_item->load_offset, seg_item->size);
            ret = 13;
            break;
        }

        dst = seg_item->exec_addr;
        if (conv_cb) {
            dst = conv_cb(dst);
        }
        src = addr + seg_item->load_offset;
        remains = seg_item->size / 4;
        TR_INFO(0, "%s:[%u] dst=0x%08X src=0x%08X size=0x%X", __func__, i, dst, src, seg_item->size);

        if (0) {
#ifdef SUBSYS_CUSTOMIZED_COPY
        } else if (subsys_customized_copy((void *)dst, (void *)src, remains * 4) == 0) {
#endif
        } else if (dma_cfg.ch == HAL_DMA_CHAN_NONE) {
            memcpy((void *)dst, (void *)src, remains * 4);
        } else {
            dma_cfg.dst = dst;
            dma_cfg.src = src;

            while (remains > 0) {
                if (remains > HAL_DMA_MAX_DESC_XFER_SIZE) {
                    dma_cfg.src_tsize = HAL_DMA_MAX_DESC_XFER_SIZE;
                } else {
                    dma_cfg.src_tsize = remains;
                }
                dma_ret = hal_dma_start(&dma_cfg);
                if (dma_ret != HAL_DMA_OK) {
                    TR_INFO(0, "%s: Failed to start dma: %d", __func__, dma_ret);
                    ret = 21;
                    break;
                }

                while (hal_dma_chan_busy(dma_cfg.ch));

                dma_cfg.dst += dma_cfg.src_tsize * 4;
                dma_cfg.src += dma_cfg.src_tsize * 4;
                remains -= dma_cfg.src_tsize;
            }
        }

        seg_item++;
    }

    if (dma_cfg.ch != HAL_DMA_CHAN_NONE) {
       hal_dma_free_chan(dma_cfg.ch);
    }

    if (ret) {
        return ret;
    }

    *code_start_addr = entry;

    return 0;
}

int subsys_loader_load_image(uint32_t addr, uint32_t len, int check_magic,
                             SUBSYS_EXEC_ADDR_CONV_CB conv_cb, uint32_t *code_start_addr)
{
    int ret = 0;
    const struct SUBSYS_IMAGE_DESC_T *desc;

    TR_INFO(0, "%s: addr=0x%08X len=0x%X (%u)", __func__, addr, len, len);

    ret = subsys_loader_check_image(addr, len, check_magic);
    if (ret) {
        TR_INFO(0, "%s: Failed to check image: %d", __func__, ret);
        return 1;
    }

    desc = subsys_loader_get_image_desc(addr);
    if (desc->type == SUBSYS_IMAGE_TYPE_SIMPLE) {
        ret = load_simple_image(desc, addr, conv_cb, code_start_addr);
    } else {
        ret = load_segment_image(desc, addr, conv_cb, code_start_addr);
    }

    return ret;
}

#ifdef SUBSYS_LOAD_FROM_FS
static int load_simple_image_fs(FILE *fp, const struct SUBSYS_IMAGE_DESC_T *desc,
                                SUBSYS_EXEC_ADDR_CONV_CB conv_cb, uint32_t *code_start_addr)
{
    int ret = 0;
    uint32_t code_start;
    uint32_t remains;

    code_start = desc->u.exec_addr;
    if (conv_cb) {
        uint32_t old_code_start;

        old_code_start = code_start;
        code_start = conv_cb(old_code_start);
        TR_INFO(0, "%s: code_start 0x%08X => 0x%08X", __func__, old_code_start, code_start);
    }
    remains = (desc->image_size - desc->code_start_offset) / sizeof(uint32_t);

    ret = fseek(fp, desc->code_start_offset, SEEK_SET);
    if (ret) {
        TR_INFO(0, "%s: Failed to seek to code: %d", __func__, errno);
        return 81;
    }
    ret = fread((void *)code_start, 4, remains, fp);
    if ((uint32_t)ret != remains) {
        TR_INFO(0, "%s: Failed to read code: %u (expecting %u)", __func__, ret, remains);
        return 82;
    }

    *code_start_addr = code_start;

    return 0;
}

int subsys_loader_load_image_fs(const char *file_name,
                                SUBSYS_EXEC_ADDR_CONV_CB conv_cb, uint32_t *code_start_addr)
{
    int ret = 0;
    const struct SUBSYS_IMAGE_DESC_T *desc;
    FILE *fp = NULL;
    uint32_t addr;
    uint32_t len;
    const uint32_t hdr_size = sizeof(struct boot_hdr_t) + sizeof(struct SUBSYS_IMAGE_DESC_T);
    uint32_t fs_data[(hdr_size + 3) / 4];

    if (!file_name) {
        return 70;
    }

    TR_INFO(0, "%s: file=%s", __func__, file_name);
    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        TR_INFO(0, "%s: Failed to open file: %d", __func__, errno);
        ret = 71;
        goto _exit;
    }
    ret = fseek(fp, 0, SEEK_END);
    if (ret) {
        TR_INFO(0, "%s: Failed to seek to end: %d", __func__, errno);
        ret = 72;
        goto _exit;
    }
    len = ftell(fp);
    ret = fread(fs_data, 1, hdr_size, fp);
    if ((uint32_t)ret != hdr_size) {
        TR_INFO(0, "%s: Failed to read hdr: %u (expecting %u)", __func__, ret, hdr_size);
        ret = 73;
        goto _exit;
    }
    addr = (uint32_t)&fs_data[0];

    ret = subsys_loader_check_image(addr, len, false);
    if (ret) {
        TR_INFO(0, "%s: Failed to check image: %d", __func__, ret);
        ret = 1;
        goto _exit;
    }

    desc = subsys_loader_get_image_desc(addr);
    if (desc->type == SUBSYS_IMAGE_TYPE_SIMPLE) {
        ret = load_simple_image_fs(fp, desc, conv_cb, code_start_addr);
    } else {
        ASSERT(false, "Not support to load segment image from fs yet");
        ret = 100;
    }

_exit:
    if (fp) {
        fclose(fp);
    }

    return ret;
}
#endif

struct SUBSYS_SEGMENT_TABLE {
    uint32_t table_start;
    uint32_t table_sz;
};

static int subsys_loader_get_segment_table(uint32_t image_addr, struct SUBSYS_SEGMENT_TABLE *table)
{
    const struct SUBSYS_IMAGE_DESC_T *desc;
    uint32_t seg_table_laddr;

    desc = subsys_loader_get_image_desc(image_addr);

    seg_table_laddr = (uint32_t)(desc + 1);
    table->table_sz = *(uint32_t *)seg_table_laddr;
    table->table_start = seg_table_laddr + 4;

    return 0;
}

struct SUBSYS_SEGMENT *subsys_loader_get_segment(uint32_t image_addr, const char *seg_name)
{
    struct SUBSYS_SEGMENT *seg;
    uint32_t seg_cnt;
    struct SUBSYS_SEGMENT_TABLE table;
    uint32_t i;

    subsys_loader_get_segment_table(image_addr, &table);
    seg = (struct SUBSYS_SEGMENT *)table.table_start;

    if(!seg)
        return NULL;

    seg_cnt = table.table_sz / sizeof(*seg);
    for (i = 0; i < seg_cnt; i++) {
        if (!strncmp(seg_name, seg->seg_name, SEG_NAME_LEN))
            return seg;
        seg++;
    }

    return NULL;
}

int subsys_loader_get_segment_fs(const char *file_name, const char *seg_name, struct SUBSYS_SEGMENT *seg_out)
{
#ifdef SUBSYS_LOAD_FROM_FS
    const uint32_t hdr_size = sizeof(struct boot_hdr_t) + sizeof(struct SUBSYS_IMAGE_DESC_T);
    int ret;
    FILE *fp = NULL;
    uint32_t table_sz;
    uint32_t seg_cnt;
    uint32_t i;
    struct SUBSYS_SEGMENT seg;

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        TR_INFO(0, "%s: Failed to open file: %d", __func__, errno);
        ret = 11;
        goto _exit;
    }
    ret = fseek(fp, hdr_size, SEEK_SET);
    if (ret) {
        TR_INFO(0, "%s: Failed to seek to table size: %d", __func__, errno);
        ret = 12;
        goto _exit;
    }
    ret = fread((void *)&table_sz, 4, 1, fp);
    if ((uint32_t)ret != 1) {
        TR_INFO(0, "%s: Failed to read table size: %u (expecting 1)", __func__, ret);
        ret = 13;
        goto _exit;
    }

    seg_cnt = table_sz / sizeof(seg);
    for (i = 0; i < seg_cnt; i++) {
        ret = fread(&seg, sizeof(seg), 1, fp);
        if ((uint32_t)ret != 1) {
            TR_INFO(0, "%s: Failed to read seg: %u (expecting 1)", __func__, ret);
            ret = 14;
            goto _exit;
        }
        if (!strncmp(seg_name, seg.seg_name, SEG_NAME_LEN)) {
            break;
        }
    }

    if (i < seg_cnt) {
        memcpy(seg_out, &seg, sizeof(seg));
        ret = 0;
    } else {
        ret = 21;
    }

_exit:
    if (fp) {
        fclose(fp);
    }
    return ret;
#else
    return -1;
#endif
}

#endif // !(CHIP_SUBSYS_SENS || (CHIP_SUBSYS_BTH ^ BTH_AS_MAIN_MCU))
#endif // !__ARM_ARCH_ISA_ARM

