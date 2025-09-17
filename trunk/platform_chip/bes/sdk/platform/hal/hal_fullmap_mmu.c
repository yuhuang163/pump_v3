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

#if defined(FULLMAP_MMU0_BASE) || defined(FULLMAP_MMU1_BASE)

#include "reg_fullmap_mmu.h"
#include "hal_fullmap_mmu.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "stdlib.h"

#ifdef FULLMAP_MMU_DEBUG
    #ifdef FPGA
        #define HAL_FULLMAP_MMU_TRACE(n, s, ...)      TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
        #define HAL_FULLMAP_MMU_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_FULLMAP_MMU_TRACE(n, s, ...)      TRACE(n, s , ##__VA_ARGS__)
        #define HAL_FULLMAP_MMU_ASSERT(n, s, ...)     ASSERT(n, s , ##__VA_ARGS__)
    #endif
#else
    #define HAL_FULLMAP_MMU_TRACE(n, s, ...)          TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #ifdef FPGA
        #define HAL_FULLMAP_MMU_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_FULLMAP_MMU_ASSERT(n, s, ...)     ASSERT(n, s , ##__VA_ARGS__)
    #endif
#endif

#define RANDOM_FILL_LUT_EN
#define PAGE_ID_MAX             16383

//----------------------------Local global variable-----------------------------
static struct FULLMAP_MMU_T *const fullmap_mmu[HAL_FULLMAP_MMU_ID_QTY] = {
#ifdef FULLMAP_MMU0_BASE
    (struct FULLMAP_MMU_T *)FULLMAP_MMU0_BASE,
#endif
#ifdef FULLMAP_MMU1_BASE
    (struct FULLMAP_MMU_T *)FULLMAP_MMU1_BASE,
#endif
};

//-------------------------------Common function--------------------------------
static inline void hal_fullmap_mmu_config_enable(enum HAL_FULLMAP_MMU_ID_T id)
{
    fullmap_mmu[id]->CONTROL &= ~MMU_MMU_CFG_LOCK;
}

static inline void hal_fullmap_mmu_config_disable(enum HAL_FULLMAP_MMU_ID_T id)
{
    fullmap_mmu[id]->CONTROL |= MMU_MMU_CFG_LOCK;
}

static void hal_fullmap_mmu_config(enum HAL_FULLMAP_MMU_ID_T id, struct HAL_FULLMAP_MMU_CONFIG_T *cfg)
{
    uint32_t val;

    if (cfg != NULL) {
        val = fullmap_mmu[id]->CONTROL;
        HAL_FULLMAP_MMU_TRACE(0, "fullmap_mmu original cfg:0x%x", val);

        //bit0~3
        if (cfg->over_provison_mode) {
            val |= MMU_LUT_OVERPROVISION;
            if (cfg->lut_sel == HAL_FULLMAP_MMU_LUT_SEL_LOW_32M) {
                val |= MMU_LUT_LOWER_INIT_DONE;
                val &= ~(MMU_LUT_UPPER_INIT_DONE | MMU_LUT_UPPER_HALF);
            } else {
                val &= ~MMU_LUT_LOWER_INIT_DONE;
                val |= MMU_LUT_UPPER_INIT_DONE | MMU_LUT_UPPER_HALF;
            }
        } else {
            val |= MMU_LUT_LOWER_INIT_DONE | MMU_LUT_UPPER_INIT_DONE;
            val &= ~(MMU_LUT_OVERPROVISION | MMU_LUT_UPPER_HALF);
        }

        //bit4
        val |= cfg->trans_error_en ? MMU_RESP_CFG : 0;

        //bit8~10
        if (cfg->check_en) {
            val &= ~MMU_ALL_CHECK_BYPASS;
            val &= ~MMU_WR_CHECK_BYPASS;
            val &= ~MMU_RD_CHECK_BYPASS;
        } else {
            val |= MMU_ALL_CHECK_BYPASS;
            val |= MMU_WR_CHECK_BYPASS;
            val |= MMU_RD_CHECK_BYPASS;
        }

        //bit11,16
        val &= ~(MMU_KEEP_FIRST_INFO | MMU_MMU_CFG_LOCK);

        //bit27
        if (cfg->low_power_mode) {
            val &= ~MMU_MODE_HCLK_ACCESS;
        } else {
            val |= MMU_MODE_HCLK_ACCESS;
        }

        //bit28,clk gating is enabled by default
        val &= ~MMU_BYPASS_LUT_CG;
    } else {
        //bypass mode
        val = MMU_ALL_CHECK_BYPASS | MMU_WR_CHECK_BYPASS | MMU_RD_CHECK_BYPASS;
    }

    HAL_FULLMAP_MMU_TRACE(0, "%s,write cfg:0x%x", __func__, val);
    hal_fullmap_mmu_config_enable(id);
    fullmap_mmu[id]->CONTROL = val;
    hal_fullmap_mmu_config_disable(id);
    HAL_FULLMAP_MMU_TRACE(0, "%s,read cfg:0x%x", __func__, fullmap_mmu[id]->CONTROL);
}

static inline uint32_t hal_fullmap_mmu_get_irq_state(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->IRQ_STATE;
}

static inline void hal_fullmap_mmu_irq_clear(enum HAL_FULLMAP_MMU_ID_T id, uint32_t val)
{
    fullmap_mmu[id]->IRQ_CLEAR = val;
}

static inline void hal_fullmap_mmu_irq_enable(enum HAL_FULLMAP_MMU_ID_T id, uint32_t val)
{
    fullmap_mmu[id]->IRQ_ENABLE = val;
}

inline uint32_t hal_fullmap_mmu_get_irq_info1_write(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->IRQ_INFO1_WRITE;
}

inline uint32_t hal_fullmap_mmu_get_irq_info2_write(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->IRQ_INFO2_WRITE;
}

inline uint32_t hal_fullmap_mmu_get_irq_info1_read(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->IRQ_INFO1_READ;
}

inline uint32_t hal_fullmap_mmu_get_irq_info2_read(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->IRQ_INFO2_READ;
}

POSSIBLY_UNUSED static inline void hal_fullmap_mmu_irq_manual_trigger(enum HAL_FULLMAP_MMU_ID_T id)
{
    fullmap_mmu[id]->IRQ_SET = MMU_IRQ_SET;
}

inline uint32_t hal_fullmap_mmu_get_trans_cnt_write(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->TRAN_CNT_WRITE;
}

inline uint32_t hal_fullmap_mmu_get_trans_cnt_read(enum HAL_FULLMAP_MMU_ID_T id)
{
    return fullmap_mmu[id]->TRAN_CNT_READ;
}

inline void hal_fullmap_mmu_read_access_bit_clear(enum HAL_FULLMAP_MMU_ID_T id)
{
    fullmap_mmu[id]->READ_ACCESS_BIT_CLEAR = MMU_READ_ACC_GLOBAL_CLEAR;
}

inline uint32_t hal_fullmap_mmu_get_read_access_bit(enum HAL_FULLMAP_MMU_ID_T id, uint32_t page_id)
{
    if (page_id > PAGE_ID_MAX) {
        page_id = PAGE_ID_MAX;
    }

    return fullmap_mmu[id]->READ_ACCESS_BIT[page_id / 32];
}

static void hal_fullmap_mmu_lut_config(enum HAL_FULLMAP_MMU_ID_T id, struct HAL_FULLMAP_MMU_CONFIG_T *cfg)
{
    uint32_t i;
    uint32_t j;
    uint32_t start_index;
    uint32_t end_index;
    uint16_t wr_valid;
    uint16_t rd_valid;

#ifdef RANDOM_FILL_LUT_EN
    uint32_t new_data;
    uint32_t old_data;
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
#endif

    if (!cfg->over_provison_mode) {
        start_index = 0;
        end_index = PAGE_ID_MAX;
    } else {
        if (cfg->lut_sel == HAL_FULLMAP_MMU_LUT_SEL_LOW_32M) {
            start_index = 0;
            end_index = PAGE_ID_MAX / 2;
        } else if (cfg->lut_sel == HAL_FULLMAP_MMU_LUT_SEL_HIGH_32M) {
            start_index = PAGE_ID_MAX / 2 + 1;
            end_index = PAGE_ID_MAX;
        } else {
            HAL_FULLMAP_MMU_ASSERT(false, "%s,wrong lut_sel setting", __func__);
        }
    }
    j = end_index;

    wr_valid = cfg->virtual_write_en ? MMU_WR_VALID_0 : 0;
    rd_valid = cfg->virtual_read_en ? MMU_RD_VALID_0 : 0;
    for (i = start_index; i <= end_index; i++) {
#ifdef RANDOM_FILL_LUT_EN
        fullmap_mmu[id]->LOOK_UP_TABLE[i] = wr_valid | rd_valid | i;
#else
        fullmap_mmu[id]->LOOK_UP_TABLE[i] = wr_valid | rd_valid | j--;
#endif
    }

#ifdef RANDOM_FILL_LUT_EN
    for (i = start_index; i <= end_index; i++) {
        //Generate a random number in the range of [a,b]:rand()%(b-a+1)+a
        j = rand() % (end_index - start_index + 1) + start_index;

        //do not exchange with oneself
        if (i == j)
            continue;

        //read index subscript value
        new_data = fullmap_mmu[id]->LOOK_UP_TABLE[j];

        //exchange data
        old_data = fullmap_mmu[id]->LOOK_UP_TABLE[i];
        fullmap_mmu[id]->LOOK_UP_TABLE[i] = new_data;
        fullmap_mmu[id]->LOOK_UP_TABLE[j] = old_data;
    }
#endif
}

//------------------------Interrupt processing function-------------------------
static void fullmap_mmu_base_irq_handler(enum HAL_FULLMAP_MMU_ID_T id)
{
    uint32_t state;

    state = hal_fullmap_mmu_get_irq_state(id);
    hal_fullmap_mmu_irq_clear(id, state);
    HAL_FULLMAP_MMU_TRACE(0, "%s,id:%d,state:0x%x", __func__, id, state);
}

#ifdef FULLMAP_MMU0_BASE
static void hal_fullmap_mmu0_irq_handler(void)
{
    fullmap_mmu_base_irq_handler(HAL_FULLMAP_MMU_ID_0);
}
#endif

#ifdef FULLMAP_MMU1_BASE
static void hal_fullmap_mmu1_irq_handler(void)
{
    fullmap_mmu_base_irq_handler(HAL_FULLMAP_MMU_ID_1);
}
#endif

static void hal_fullmap_mmu_int_enable(enum HAL_FULLMAP_MMU_ID_T id)
{
    uint32_t irq_num = INVALID_IRQn;

    if (0) {
#ifdef FULLMAP_MMU0_BASE
    } else if (id == HAL_FULLMAP_MMU_ID_0) {
        irq_num = FULLMAP_MMU0_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_fullmap_mmu0_irq_handler);
#endif
#ifdef FULLMAP_MMU1_BASE
    } else if (id == HAL_FULLMAP_MMU_ID_1) {
        irq_num = FULLMAP_MMU1_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_fullmap_mmu1_irq_handler);
#endif
    }

    NVIC_SetPriority(irq_num, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq_num);
    NVIC_EnableIRQ(irq_num);
}

static void hal_fullmap_mmu_int_disable(enum HAL_FULLMAP_MMU_ID_T id)
{
    uint32_t irq_num = INVALID_IRQn;

    if (0) {
#ifdef FULLMAP_MMU0_BASE
    } else if (id == HAL_FULLMAP_MMU_ID_0) {
        irq_num = FULLMAP_MMU0_IRQn;
#endif
#ifdef FULLMAP_MMU1_BASE
    } else if (id == HAL_FULLMAP_MMU_ID_1) {
        irq_num = FULLMAP_MMU1_IRQn;
#endif
    }

    NVIC_DisableIRQ(irq_num);
}

//-------------------------------open and close---------------------------------
void hal_fullmap_mmu_open(enum HAL_FULLMAP_MMU_ID_T id, struct HAL_FULLMAP_MMU_CONFIG_T *cfg)
{
    HAL_FULLMAP_MMU_ASSERT(cfg != NULL, "%s,fullmap_mmu cfg can not be NULL", __func__);

    HAL_FULLMAP_MMU_TRACE(0, "%s,id:%d", __func__, id);
    HAL_FULLMAP_MMU_TRACE(0, "cfg->over_provison_mode :%d", cfg->over_provison_mode);
    if (cfg->over_provison_mode) {
        HAL_FULLMAP_MMU_TRACE(0, "cfg->lut_sel            :%s", cfg->lut_sel == HAL_FULLMAP_MMU_LUT_SEL_LOW_32M ? "low 32M" : "high 32M");
    }
    HAL_FULLMAP_MMU_TRACE(0, "cfg->trans_error_en     :%d", cfg->trans_error_en);
    HAL_FULLMAP_MMU_TRACE(0, "cfg->virtual_write_en   :%d", cfg->virtual_write_en);
    HAL_FULLMAP_MMU_TRACE(0, "cfg->virtual_read_en    :%d", cfg->virtual_read_en);
    HAL_FULLMAP_MMU_TRACE(0, "cfg->low_power_mode     :%d", cfg->low_power_mode);
    HAL_FULLMAP_MMU_TRACE(0, "  ");

    hal_fullmap_mmu_lut_config(id, cfg);
    hal_fullmap_mmu_read_access_bit_clear(id);
    hal_fullmap_mmu_irq_clear(id, MMU_CLEAR_IRQ_W | MMU_CLEAR_IRQ_R);
    hal_fullmap_mmu_irq_enable(id, MMU_ENABLE_IRQ_W | MMU_ENABLE_IRQ_R);
    hal_fullmap_mmu_config(id, cfg);
    hal_fullmap_mmu_int_enable(id);
}

void hal_fullmap_mmu_close(enum HAL_FULLMAP_MMU_ID_T id)
{
    hal_fullmap_mmu_int_disable(id);
    hal_fullmap_mmu_irq_enable(id, 0);
    hal_fullmap_mmu_config(id, NULL);//bypass mode
}

#endif

