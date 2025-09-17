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
#include "plat_addr_map.h"

#ifdef CPUDUMP_BASE

#include "hal_cmu.h"
#include "hal_cpudump.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "reg_cpudump.h"
#include "string.h"
#include "cmsis.h"

#ifndef DUMPPC_NUM
#define DUMPPC_NUM                  256
#endif

#ifndef DUMPLR_NUM
#define DUMPLR_NUM                  32
#endif

#ifndef DUMP_NUM_ONCE
#define DUMP_NUM_ONCE               8
#endif

#if defined(DUMPLR_TOTAL_NUM) && (DUMPLR_TOTAL_NUM > 0)
#if defined(BTC_CPUDUMP_BASE) && !defined(BTC_DUMPLR_TOTAL_NUM)
#define BTC_DUMPLR_TOTAL_NUM        DUMPLR_TOTAL_NUM
#endif
#if defined(WIFI_CPUDUMP_BASE) && !defined(WIFI_DUMPLR_TOTAL_NUM)
#define WIFI_DUMPLR_TOTAL_NUM       DUMPLR_TOTAL_NUM
#endif
#if defined(SENS_CPUDUMP_BASE) && !defined(SENS_DUMPLR_TOTAL_NUM)
#define SENS_DUMPLR_TOTAL_NUM       DUMPLR_TOTAL_NUM
#endif
#endif

#if defined(BTC_CPUDUMP_BASE) && !defined(BTC_DUMPPC_TOTAL_NUM)
#define BTC_DUMPPC_TOTAL_NUM        DUMPPC_TOTAL_NUM
#endif
#if defined(WIFI_CPUDUMP_BASE) && !defined(WIFI_DUMPPC_TOTAL_NUM)
#define WIFI_DUMPPC_TOTAL_NUM       DUMPPC_TOTAL_NUM
#endif
#if defined(SENS_CPUDUMP_BASE) && !defined(SENS_DUMPPC_TOTAL_NUM)
#define SENS_DUMPPC_TOTAL_NUM       DUMPPC_TOTAL_NUM
#endif

typedef uint32_t (*GET_ADDR_FUNC)(void);

enum CORE_TYPE_T {
    CORTEX_M = 0,
    RISCV_EH2,
    HIFI4,
};

enum DUMP_TYPE_T {
    CPUDUMP_PC = 0,
#if defined(DUMPLR_TOTAL_NUM) && (DUMPLR_TOTAL_NUM > 0) || \
        defined(BTC_DUMPLR_TOTAL_NUM) && (BTC_DUMPLR_TOTAL_NUM > 0) || \
        defined(WIFI_DUMPLR_TOTAL_NUM) && (WIFI_DUMPLR_TOTAL_NUM > 0) || \
        defined(SENS_DUMPLR_TOTAL_NUM) && (SENS_DUMPLR_TOTAL_NUM > 0) || \
        defined(SYS_DUMPLR_TOTAL_NUM) && (SYS_DUMPLR_TOTAL_NUM > 0) || \
        defined(BTH_DUMPLR_TOTAL_NUM) && (BTH_DUMPLR_TOTAL_NUM > 0)
    CPUDUMP_LR = 1,
#else
    CPUDUMP_LR = CPUDUMP_PC,
#endif

    HAL_CPUDUMP_TYPE_QTY
};

struct HAL_CPUDUMP_INFO_T {
    GET_ADDR_FUNC last_addr;
    uint16_t dump_buf_len;
};

struct HAL_CPUDUMP_DESC_T {
    struct CPUDUMP_T *base;
    struct HAL_CPUDUMP_INFO_T info[HAL_CPUDUMP_TYPE_QTY];
};

#ifdef BTC_CPUDUMP_BASE
WEAK uint32_t hal_cmu_cpudump_get_btc_last_pc_addr(void)
{
    return BTC_DUMPPC_TOTAL_NUM;
}

#if defined(BTC_DUMPLR_TOTAL_NUM) && (BTC_DUMPLR_TOTAL_NUM > 0)
WEAK uint32_t hal_cmu_cpudump_get_btc_last_lr_addr(void)
{
    return BTC_DUMPLR_TOTAL_NUM;
}
#endif
#endif

static const struct HAL_CPUDUMP_DESC_T cpudump[HAL_CPUDUMP_ID_QTY] = {
    {
        .base = (struct CPUDUMP_T *)CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_last_pc_addr,
            .dump_buf_len = DUMPPC_TOTAL_NUM,
        },
#if defined(DUMPLR_TOTAL_NUM) && (DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_last_lr_addr,
            .dump_buf_len = DUMPLR_TOTAL_NUM,
        },
#endif
    },

#ifdef BTC_CPUDUMP_BASE
    {
        // TODO BTC
        .base = (struct CPUDUMP_T *)BTC_CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_btc_last_pc_addr,
            .dump_buf_len = BTC_DUMPPC_TOTAL_NUM,
        },
#if defined(BTC_DUMPLR_TOTAL_NUM) && (BTC_DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_btc_last_lr_addr,
            .dump_buf_len = BTC_DUMPLR_TOTAL_NUM,
        },
#endif
    },
#endif

#ifdef WIFI_CPUDUMP_BASE
    {
        .base = (struct CPUDUMP_T *)WIFI_CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_wf_last_pc_addr,
            .dump_buf_len = WIFI_DUMPPC_TOTAL_NUM,
        },
#if defined(WIFI_DUMPLR_TOTAL_NUM) && (WIFI_DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_wf_last_lr_addr,
            .dump_buf_len = WIFI_DUMPLR_TOTAL_NUM,
        },
#endif
    },
#endif

#ifdef SENS_CPUDUMP_BASE
    {
        .base = (struct CPUDUMP_T *)SENS_CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_sens_last_pc_addr,
            .dump_buf_len = SENS_DUMPPC_TOTAL_NUM,
        },
#if defined(SENS_DUMPLR_TOTAL_NUM) && (SENS_DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_sens_last_lr_addr,
            .dump_buf_len = SENS_DUMPLR_TOTAL_NUM,
        },
#endif
    },
#endif

#if defined(SYS_CPUDUMP_BASE) && defined(BTH_AS_MAIN_MCU)
    {
        .base = (struct CPUDUMP_T *)SYS_CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_sys_last_pc_addr,
            .dump_buf_len = SYS_DUMPPC_TOTAL_NUM,
        },
#if defined(SYS_DUMPLR_TOTAL_NUM) && (SYS_DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_sys_last_lr_addr,
            .dump_buf_len = SYS_DUMPLR_TOTAL_NUM,
        },
#endif
    },
#endif

#if defined(BTH_CPUDUMP_BASE) && !defined(BTH_AS_MAIN_MCU)
    {
        .base = (struct CPUDUMP_T *)BTH_CPUDUMP_BASE,
        .info[CPUDUMP_PC] = {
            .last_addr = hal_cmu_cpudump_get_bth_last_pc_addr,
            .dump_buf_len = BTH_DUMPPC_TOTAL_NUM,
        },
#if defined(BTH_DUMPLR_TOTAL_NUM) && (BTH_DUMPLR_TOTAL_NUM > 0)
        .info[CPUDUMP_LR] = {
            .last_addr = hal_cmu_cpudump_get_bth_last_lr_addr,
            .dump_buf_len = BTH_DUMPLR_TOTAL_NUM,
        },
#endif
    },
#endif
};

void BOOT_TEXT_FLASH_LOC hal_cpudump_enable(enum HAL_CPUDUMP_ID_T id)
{
    if (HAL_CPUDUMP_ID_0 == id) {
        hal_cpudump_clock_enable();
    }
    cpudump[id].base->CPUDUMP_EN = CMU_REG_DUMP_EN;
    cpudump[id].base->CPUDUMP_CTRL = CMU_REG_DUMP_START;
    cpudump[id].base->CPUDUMP_FAULT_MASK = CMU_REG_BPT_FAULT | CMU_REG_SV_FAULT
        | CMU_REG_SF_FAULT | CMU_REG_FAULT_MASK_EN;
}

void hal_cpudump_disable(enum HAL_CPUDUMP_ID_T id)
{
    cpudump[id].base->CPUDUMP_CTRL = CMU_REG_DUMP_STOP;
    hal_sys_timer_delay_us(10);
    cpudump[id].base->CPUDUMP_EN = ~CMU_REG_DUMP_EN;
}

static uint32_t hal_cpudump_find_last_pos(const uint32_t* addr, uint16_t len)
{
    for (int i = 0; i < len; i++){
        if ((addr[i] & 0x1) == 1) {
            return i;
        }
    }
    REL_TRACE_NOTS(0, "cant find last element");
    return 0;
}

static int hal_cpudump_read(enum HAL_CPUDUMP_ID_T id, enum DUMP_TYPE_T type, uint32_t *buff, uint16_t size)
{
    if (id >= HAL_CPUDUMP_ID_QTY) {
        return -1;
    } else if (type >= HAL_CPUDUMP_TYPE_QTY) {
        return -2;
    }

    bool need_reopen = false;
    if (cpudump[id].base->CPUDUMP_EN & CMU_REG_DUMP_EN) {
        hal_cpudump_disable(id);
        need_reopen = true;
    }

    uint16_t cur_pos;
    uint16_t cnt, remain, read_pos;
    const int bufSize = cpudump[id].info[type].dump_buf_len;
    const uint32_t* base = (uint32_t*)((CPUDUMP_PC == type) ? cpudump[id].base->PC_DATA : cpudump[id].base->LR_DATA);
    if (cpudump[id].info[type].last_addr) {
        cur_pos = cpudump[id].info[type].last_addr();
    } else {
        cur_pos = bufSize;
    }
    if (bufSize == cur_pos) {
        cur_pos = hal_cpudump_find_last_pos(base, bufSize);
    }
    remain = (size > bufSize) ? bufSize : size;
    // REL_TRACE_NOTS(0, "cur_pos %d %d", cur_pos, remain);
    cur_pos = (cur_pos + 1 + bufSize - remain) % bufSize;
    for (cnt = 0; cnt < remain; cnt++) {
        read_pos = cur_pos + cnt;
        if (read_pos > bufSize) {
            read_pos -= bufSize;
        }
        *(buff + cnt) = *(base + read_pos);
    }

    if (need_reopen) {
        hal_cpudump_enable(id);
    }

    return cnt;
}

__STATIC_INLINE void hal_cpudump_dump(enum HAL_CPUDUMP_ID_T id, enum DUMP_TYPE_T type, uint32_t* cpu_data, uint32_t size)
{
    uint16_t dump_size, read_size, remain_size;
    read_size = hal_cpudump_read(id, type, cpu_data, size);
    for (int cnt = 0; cnt < read_size; cnt += dump_size) {
        remain_size = read_size - cnt;
        dump_size = (remain_size > DUMP_NUM_ONCE) ? DUMP_NUM_ONCE : remain_size;
        DUMP32(" %08x", cpu_data + cnt, dump_size);
    }
}

void hal_cpudump_dump_pc(enum HAL_CPUDUMP_ID_T id)
{
    uint32_t cpu_data[DUMPPC_NUM];
    REL_TRACE_NOTS(0, "dump pc");
    hal_cpudump_dump(id, CPUDUMP_PC, cpu_data, ARRAY_SIZE(cpu_data));
}

void hal_cpudump_dump_lr(enum HAL_CPUDUMP_ID_T id)
{
    uint32_t cpu_data[DUMPLR_NUM];
    REL_TRACE_NOTS(0, "dump lr");
    hal_cpudump_dump(id, CPUDUMP_LR, cpu_data, ARRAY_SIZE(cpu_data));
}

int hal_cpudump_pc_read(enum HAL_CPUDUMP_ID_T id, uint32_t *buff, uint16_t size)
{
    return hal_cpudump_read(id, CPUDUMP_PC, buff, size);
}

int hal_cpudump_lr_read(enum HAL_CPUDUMP_ID_T id, uint32_t *buff, uint16_t size)
{
    return hal_cpudump_read(id, CPUDUMP_LR, buff, size);
}

#endif
