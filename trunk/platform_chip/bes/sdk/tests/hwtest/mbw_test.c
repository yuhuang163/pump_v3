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
#ifdef MBW_TEST

#include "cmsis.h"
#include "plat_addr_map.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef MBW_TEST_ONLY
#include "hwtimer_list.h"
#include "hal_iomux.h"
#endif
#include "hal_cache.h"
#include "hal_dma.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_psram.h"
#include "hal_psramuhs.h"
#include "dwt.h"
#include "hal_location.h"

#if !defined(MBW_PSRAM) && \
    !defined(MBW_PSRAM_NC) && \
    !defined(MBW_PSRAMUHS) && \
    !defined(MBW_PSRAMUHS_NC) && \
    !defined(MBW_A7_SRAM) && \
    !defined(MBW_FLASH) && \
    !defined(MBW_FLASH_NC)
#define MBW_SRAM
#endif

#if defined(MBW_FLASH_NC) || defined(MBW_FLASH)
#define MBW_TEST_FLASH
#else
#define MBW_TEST_RAM
#endif

/* how many runs to average by default */
#define DEFAULT_NR_LOOPS 10

/* test types */
enum mbw_test_item {
    TEST_MEMCPY = 0,
    TEST_MEMCPY_DMA,
    TEST_MEMSET,
    TEST_MEMMOVE,
    TEST_DUMB,
    TEST_MCBLOCK,
    TEST_READ,
    TEST_WRITE,
    TEST_FLASH_DMA_MEMCPY,
};
#define MBW_TEST_INVALID 0xff

static const int mbw_cases[] = {
#ifdef MBW_TEST_FLASH
    TEST_READ,
    TEST_FLASH_DMA_MEMCPY,
#elif defined(MBW_TEST_RW_ONLY)
    TEST_READ,
    TEST_WRITE,
#else
    TEST_MEMCPY,
    TEST_MEMCPY_DMA,
    TEST_MEMSET,
    TEST_MEMMOVE,
    TEST_READ,
    TEST_WRITE,
    /* TEST_DUMB, */
    /* TEST_MCBLOCK, */
#endif
    MBW_TEST_INVALID,
};

#ifndef MBW_ARRAY_SIZE_KB
#ifdef MBW_SRAM
/* #define MBW_ARRAY_SIZE_KB 256 */
#define MBW_ARRAY_SIZE_KB 16
#else
#define MBW_ARRAY_SIZE_KB 512
#endif
#endif
#ifndef MBW_BLOCK_SIZE_KB
#define MBW_BLOCK_SIZE_KB 16
#endif

static uint8_t *g_heap =
#if defined(MBW_SRAM)
    (uint8_t *) (RAM_BASE + RAM_SIZE / 2);
//  (uint8_t *) (SYS_RAM0_BASE );
#elif defined(MBW_PSRAM)
    (uint8_t *) PSRAM_BASE;
#elif defined(MBW_PSRAM_NC)
    (uint8_t *) PSRAM_NC_BASE;
#elif defined(MBW_PSRAMUHS)
    (uint8_t *) PSRAMUHS_BASE;
#elif defined(MBW_PSRAMUHS_NC)
    (uint8_t *) PSRAMUHS_NC_BASE;
#elif defined(MBW_A7_SRAM)
    (uint8_t *) (DSP_RAM_BASE + 0x00080000);
#elif defined(MBW_FLASH)
    (uint8_t *) FLASH_BASE;
#elif defined(MBW_FLASH_NC)
    (uint8_t *) FLASH_NC_BASE;
#else
    #error "Missing MBW test items"
#endif

static long *cur_heap = 0;

SRAM_TEXT_LOC
static void *mempcpy(void *d, const void *s, size_t len)
{
    long *dst = (long *)d;
    long *src = (long *)s;
    // 4 instructions (with one skip)
    while (dst < (long *)(d + len))
        *dst++ = *src++;

    return dst;
}

#define MEM_BANK_SZ     (256 * 1024)

SRAM_TEXT_LOC
static long *make_array(unsigned long asize)
{
    long *a;

    if (cur_heap == 0) {
        cur_heap = (long *)&g_heap[0];
    }

    a = (long *)cur_heap;

    uint32_t b;
    b = (uint32_t)cur_heap + asize + MEM_BANK_SZ;
    cur_heap = (long *)b;

    /* make sure both arrays are allocated, fill with pattern */
#ifdef MBW_TEST_RAM
    unsigned long t;
    for (t = 0; t < asize; t++) {
        a[t] = 0xaa;
    }
#endif
    return a;
}

//#define DMA_MULTI_CH_TEST
#define DMA_SEL_MASTER_CNT  5
SYNC_FLAGS_LOC
static struct HAL_DMA_DESC_T mem_dma_desc[DMA_SEL_MASTER_CNT][MBW_ARRAY_SIZE_KB / 16 + 1];
SRAM_TEXT_LOC
static int mem_dma_init(struct HAL_DMA_CH_CFG_T *dma_cfg, void *dst, const void *src, size_t len, uint8_t id)
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
    dma_cfg->master     = id;
    TRACE(0, "id:%d, dma_cfg->ch:%d", id, dma_cfg->ch);
    ASSERT(dma_cfg->ch != HAL_DMA_CHAN_NONE, "%s: Failed to get dma chan", __func__);

    dma_cfg->dst = (uint32_t)dst;
    dma_cfg->src = (uint32_t)src;

    uint16_t i;
    uint16_t link_cnt;
    enum HAL_DMA_RET_T dret;
    uint32_t remain_transfer_size;
    #define DMA_TSIZE_MAX      0xFFC

    remain_transfer_size = len / 4;
    if (remain_transfer_size > DMA_TSIZE_MAX) {
        if (remain_transfer_size % DMA_TSIZE_MAX) {
            link_cnt = remain_transfer_size / DMA_TSIZE_MAX + 1;
        } else {
            link_cnt = remain_transfer_size / DMA_TSIZE_MAX;
        }
    } else {
        link_cnt = 1;
    }

    //Generate dma link configuration
    //REL_TRACE_NOTS(0, "---link_cnt           :%d", link_cnt);
    for (i = 0; i < link_cnt; i++) {
        dma_cfg->dst = (uint32_t)(dst + DMA_TSIZE_MAX * 4 * i);
        if (remain_transfer_size > DMA_TSIZE_MAX) {
            dma_cfg->src_tsize = DMA_TSIZE_MAX;
            remain_transfer_size -= DMA_TSIZE_MAX;
        } else {
            dma_cfg->src_tsize = remain_transfer_size;
            remain_transfer_size = 0;
        }
        if (i + 1 == link_cnt) {
            dret = hal_dma_init_desc(&mem_dma_desc[id][i], dma_cfg, NULL, 0);
        } else {
            dret = hal_dma_init_desc(&mem_dma_desc[id][i], dma_cfg, &mem_dma_desc[id][i + 1], 0);
        }
        ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);

        //REL_TRACE_NOTS(0, "---dma_cfg->dst       :0x%x", dma_cfg->dst);
        //REL_TRACE_NOTS(0, "---dma_cfg->src_tsize :%d", dma_cfg->src_tsize);
        //REL_TRACE_NOTS(0, "mem_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
        //                   mem_dma_desc[id][i].src, mem_dma_desc[id][i].dst, mem_dma_desc[id][i].lli,
        //                   mem_dma_desc[id][i].ctrl, (uint32_t)&mem_dma_desc[id][i]);
    }

    return 0;
}

SRAM_TEXT_LOC
static int mem_dma_copy(void *dst, const void *src, size_t len)
{
    struct HAL_DMA_CH_CFG_T dma_cfg0;
#ifdef DMA_MULTI_CH_TEST
    struct HAL_DMA_CH_CFG_T dma_cfg1;
#endif
    static int dma_init = false;
    #define SEL_MASTER  HAL_DMA_MASTER_0

    if (!dma_init) {
#ifdef DMA_MULTI_CH_TEST
        mem_dma_init(&dma_cfg0, dst, src, len, HAL_DMA_MASTER_0);
        mem_dma_init(&dma_cfg1, dst + len, src + len, len, HAL_DMA_MASTER_1);
#else
        mem_dma_init(&dma_cfg0, dst, src, len, SEL_MASTER);
#endif
        dma_init = true;
    }
    hal_dma_sg_start(&mem_dma_desc[SEL_MASTER][0], &dma_cfg0);
#ifdef DMA_MULTI_CH_TEST
    hal_dma_sg_start(&mem_dma_desc[HAL_DMA_MASTER_1][0], &dma_cfg1);
#endif
    while (hal_dma_chan_busy(dma_cfg0.ch));
#ifdef DMA_MULTI_CH_TEST
    while (hal_dma_chan_busy(dma_cfg1.ch));
#endif

    return 0;
}

#define DMA_BLK     8 //kB
static struct HAL_DMA_DESC_T dma_desc[MBW_ARRAY_SIZE_KB / DMA_BLK];

SRAM_TEXT_LOC
static int flash_dma_init(struct HAL_DMA_CH_CFG_T *dma_cfg)
{
    memset(dma_cfg, 0, sizeof(*dma_cfg));
    dma_cfg->dst_bsize  = HAL_DMA_BSIZE_4;
    dma_cfg->dst_periph = HAL_GPDMA_MEM;
    dma_cfg->dst_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg->handler    = NULL;
    dma_cfg->src_bsize  = HAL_DMA_BSIZE_32;
    dma_cfg->src_periph = HAL_GPDMA_MEM;
    dma_cfg->src_width  = HAL_DMA_WIDTH_WORD;
    dma_cfg->type       = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg->try_burst  = false;
    dma_cfg->ch         = hal_dma_get_chan(dma_cfg->dst_periph, HAL_DMA_HIGH_PRIO);
    ASSERT(dma_cfg->ch != HAL_DMA_CHAN_NONE, "%s: Failed to get dma chan", __func__);

    dma_cfg->dst = ROM_BASE;
#ifdef MBW_FLASH_NC
    dma_cfg->src = (uint32_t) FLASH_NC_BASE + 0x100000;
#else
    dma_cfg->src = (uint32_t) FLASH_BASE + 0x100000;
#endif
    dma_cfg->src_tsize = (DMA_BLK * 1024) / 4;

    for (int i = 0; i < ARRAY_SIZE(dma_desc); i++) {
        if (i == (ARRAY_SIZE(dma_desc) - 1)) {
            hal_dma_init_desc(&dma_desc[i], dma_cfg, NULL, 0);
        } else {
            dma_cfg->src += dma_cfg->src_tsize;
            hal_dma_init_desc(&dma_desc[i], dma_cfg, &dma_desc[(i + 1)], 0);
        }
    }

    return 0;
}

SRAM_TEXT_LOC
static int flash_dma_memcpy(void)
{
    struct HAL_DMA_CH_CFG_T dma_cfg;
    static int dma_init = false;

    if (!dma_init) {
        flash_dma_init(&dma_cfg);
        dma_init = true;
    }
    hal_dma_sg_start(&dma_desc[0], &dma_cfg);
    while (hal_dma_chan_busy(dma_cfg.ch));

    return 0;
}

/* actual benchmark */
/* asize: number of type 'long' elements in test arrays
 * long_size: sizeof(long) cached
 * type: 0=use memcpy, 1=use dumb copy loop (whatever GCC thinks best)
 *
 * return value: elapsed time in seconds
 */

SRAM_TEXT_LOC
static uint32_t worker(unsigned long asize, long *a, long *b, int type, unsigned long block_size)
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
    } else if (type == TEST_MEMCPY_DMA) {  /* dma memcpy test */
        /* timer starts */
        starttime = hal_fast_sys_timer_get();
        dwt_reset_cycle_cnt();
        mem_dma_copy(b, a, array_bytes);
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
            bb = mempcpy(bb, aa, block_size);
        }

        if (bb < (void *)(b + asize)) {
            bb = mempcpy(bb, aa, (void *)(b + asize) - bb);
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
    } else if (type == TEST_FLASH_DMA_MEMCPY){
        starttime = hal_fast_sys_timer_get();
        flash_dma_memcpy();
        endtime = hal_fast_sys_timer_get();
    } else {
        TRACE(0, "ERROR");
        starttime = 0;
        endtime = 0;
    }

    te = FAST_TICKS_TO_US(endtime - starttime);

    if (cyc_cnt != -1u)
        REL_TRACE_NOCRLF_NOTS(0, "\tcpu cycles %d \n", cyc_cnt);
    return te;
}

/* ------------------------------------------------------ */

/* pretty print worker's output in human-readable terms */
/* te: elapsed time in seconds
 * mt: amount of transferred data in MiB
 * type: see 'worker' above
 *
 * return value: -
 */

SRAM_TEXT_LOC
POSSIBLY_UNUSED static void printout(uint32_t te, int mt, int type)
{
    switch (type) {
    case TEST_MEMCPY:
        REL_TRACE_NOCRLF_NOTS(0, "Method: MEMCPY\t");
        break;
    case TEST_MEMCPY_DMA:
        REL_TRACE_NOCRLF_NOTS(0, "Method: MEMCPY_DMA\t");
        break;
    case TEST_MEMSET:
        REL_TRACE_NOCRLF_NOTS(0, "Method: MEMSET\t");
        break;
    case TEST_MEMMOVE:
        REL_TRACE_NOCRLF_NOTS(0, "Method: MEMMOVE\t");
        break;
    case TEST_DUMB:
        REL_TRACE_NOCRLF_NOTS(0, "Method: DUMB  \t");
        break;
    case TEST_MCBLOCK:
        REL_TRACE_NOCRLF_NOTS(0, "Method: MCBLOCK\t");
        break;
    case TEST_READ:
        REL_TRACE_NOCRLF_NOTS(0, "Method: READ   \t");
        break;
    case TEST_WRITE:
        REL_TRACE_NOCRLF_NOTS(0, "Method: WRITE  \t");
        break;
    case TEST_FLASH_DMA_MEMCPY:
        REL_TRACE_NOCRLF_NOTS(0, "Method: FLASH_DMA_MEMCPY \t");
        break;
    }

    REL_TRACE_NOCRLF_NOTS(0, "Elapsed: %5d us. ", te);
    REL_TRACE_NOCRLF_NOTS(0, "KB: %4d. ", mt);
    uint64_t bytes = mt * 1024;
    uint32_t speed_mhz = bytes / te;
    uint32_t speed_remain = bytes * 100 / te - speed_mhz * 100;
    REL_TRACE_NOCRLF_NOTS(0, "Speed: %4u.%02u MB/s\n", speed_mhz, speed_remain);
    return;
}
SRAM_TEXT_LOC
static int mbw_test_array(int arraysize_KB, int blocksize_KB)
{
    int long_size;
    uint32_t te, te_sum;        /* time elapsed */
    int asize;                  /* array size (elements in array) */
    long *a, *b;                /* the two arrays to be copied from/to */

    /* how many runs to average? */
    int nr_loops = DEFAULT_NR_LOOPS;
    /* fixed memcpy block size for -t2 */
    int block_size = blocksize_KB * 1024;
    /* show average, -a */
    /* what tests to run (-t x) */
    int mt = arraysize_KB;      /* MiBytes transferred == array size in MiB */

    long_size = sizeof(long);   /* the size of long on this platform */
    asize = 1024 / long_size * mt;  /* how many longs then in one array? */

    if (asize * long_size < block_size) {
        TRACE(0, "Error: array size larger than block size (%d bytes)!", block_size);
        return -1;
    }

    TRACE(0, "Long uses %d bytes. \n", long_size);
    TRACE(0, "Allocating 2*%d elements = %d bytes of memory.", asize, (int)(2 * asize * long_size));
    TRACE(0, "Using %d bytes as blocks for memcpy block copy test.", block_size);

    a = make_array(asize);
    if (a == NULL)
        return -1;
    b = make_array(asize);
    if (b == NULL)
        return -1;
    TRACE(0, "memory a:%p, b:%p", a, b);

    TRACE_IMM(0, "Getting down to business... Doing %d runs per test.", nr_loops);

    /* run all tests requested, the proper number of times */
    const int *test_case = &mbw_cases[0];
    int testno = 0;

#ifndef __ARM_ARCH_ISA_ARM
    hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
//  hal_cache_monitor_enable(HAL_CACHE_ID_I_CACHE);
//  hal_cache_monitor_enable(HAL_CACHE_ID_D_CACHE);
#endif

    while (test_case[testno] != MBW_TEST_INVALID) {
        te_sum = 0;
        for (int i = 0; nr_loops == 0 || i < nr_loops; i++) {
            TRACE_FLUSH();
            hal_sys_timer_delay(16);
            uint32_t lock = int_lock();
            te = worker(asize, a, b, test_case[testno], block_size);
            int_unlock(lock);
            te_sum += te;
            REL_TRACE_NOCRLF_NOTS(0, "%d\t", i);
            printout(te, mt, test_case[testno]);
//          hal_cache_print_stats();
        }
        REL_TRACE_NOCRLF_NOTS(0, "AVG\t");
        printout(te_sum / nr_loops, mt, test_case[testno]);
        REL_TRACE_NOCRLF_NOTS(0, "\n\r");
        testno++;
    }
    return 0;
}

#if defined(MBW_PSRAM) || defined(MBW_PSRAM_NC)
POSSIBLY_UNUSED
static void psram_pll_init(void)
{
    // PLL will be enabled when setting mem freq for new chips
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    hal_cmu_init_pll_selection();
}
#endif

/* ------------------------------------------------------ */

SRAM_TEXT_LOC
void mbw_test()
{
#ifndef __ARM_ARCH_ISA_ARM
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
#endif
#if defined(MBW_PSRAM) || defined(MBW_PSRAM_NC)
#if defined(ROM_BUILD) || defined(PROGRAMMER) || !defined(PSRAM_ENABLE)
    psram_pll_init();
    hal_psram_init();
#endif
#elif defined(MBW_PSRAMUHS) || defined(MBW_PSRAMUHS_NC)
#if defined(ROM_BUILD) || defined(PROGRAMMER) || !defined(PSRAMUHS_ENABLE)
    hal_cmu_dsp_clock_enable();
    hal_cmu_dsp_reset_clear();
    hal_psramuhs_init();
#endif
#endif
#endif
    dwt_enable();

    //hal_cache_disable(HAL_CACHE_ID_I_CACHE);
    //hal_cache_disable(HAL_CACHE_ID_D_CACHE);

    enum HAL_CMU_FREQ_T sys_freq;
#ifdef __ARM_ARCH_ISA_ARM
    sys_freq = HAL_CMU_FREQ_780M;
#else
    sys_freq = HAL_CMU_FREQ_208M;
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, sys_freq);
    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    mbw_test_array(MBW_ARRAY_SIZE_KB, MBW_BLOCK_SIZE_KB);
    hal_cmu_simu_pass();
    REL_TRACE_NOTS(0, "===mbw test done===");
}

#endif
