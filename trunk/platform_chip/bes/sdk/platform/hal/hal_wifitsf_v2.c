/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#if defined(WFTSF_BASE) && (CHIP_WIFITSF_VER == 2)

#include "hal_cmu.h"
#include "hal_wifitsf.h"
#include "reg_wifitsf_v2.h"

static struct WIFITSF_T * const wifitsf = (struct WIFITSF_T *)WFTSF_BASE;

void hal_wifitsf_enable(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_TSF);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_TSF);

    // use event0, dma rising edge
    wifitsf->CTRL = TSF_TIMER_ENABLE | CAPTURE_TRIG0_ENABLE | TSF_WIFI_UPDATE_ENABLE |
        TSF_PRESCALE(hal_cmu_get_crystal_freq()/1000000 - 1);

    wifitsf->INT_MASK = TSF_EVENT0_INT_MASK | CAPTURE_TRIG0_INT_MASK;
#ifndef CHIP_SUBSYS_BTH
    wifitsf->TRIG_CTRL = TSF_CODEC_TRIG_EN;
#endif
}

void hal_wifitsf_disable(void)
{
    wifitsf->CTRL &= ~TSF_TIMER_ENABLE;

    hal_cmu_reset_set(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_reset_set(HAL_CMU_MOD_O_TSF);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_TSF);
}

void hal_wifitsf_timer_set(uint32_t low, uint32_t high)
{
    wifitsf->CTRL &= ~TSF_TIMER_ENABLE;
    wifitsf->TIME_LOW = low;
    wifitsf->TIME_HIGH = high;
    wifitsf->CTRL |= TSF_TIMER_ENABLE;
}

uint64_t hal_wifitsf_timer_get(void)
{
    uint32_t low = wifitsf->TIME_LOW;
    uint32_t high = wifitsf->TIME_HIGH;

    uint64_t wifitsf_timer = high;
    wifitsf_timer = (wifitsf_timer << 32) | low;

    return wifitsf_timer;
}

void hal_wifitsf_dma_sel_cfg(uint8_t idx, uint8_t sel)
{
#if defined(CHIP_BEST1700) && !defined(CHIP_SUBSYS_BTH)
    /* dma sel offset is 0 in 1700 sys */
#else
    /* dma sel offset is 8 in 1700 bth, 2007, 2009 */
    sel += 8;
#endif

    wifitsf->DMA_TRIG_SEL &= ~(DMA_INT0_TRIG_SEL_MASK << (idx * 8));
    wifitsf->DMA_TRIG_SEL |= DMA_INT0_TRIG_SEL(sel) << (idx * 8);
}

uint64_t hal_wifitsf_dma_time_get(void)
{
    uint32_t low = wifitsf->DMA_TIME0_LOW;
    uint32_t high = wifitsf->DMA_TIME0_HIGH;

    uint64_t wifitsf_dma_timer = high;
    wifitsf_dma_timer = (wifitsf_dma_timer << 32) | low;

    return wifitsf_dma_timer;
}

uint32_t hal_wifitsf_dma_time_get_high(void)
{
    return wifitsf->DMA_TIME0_HIGH;
}

void hal_wifitsf_set_usb_tirg_shreshold(uint32_t val)
{
    if (val > 0)
        wifitsf->USB_TRIG_THD = USB_TRIG_THRESHOLD(val - 1);
}

void hal_wifitsf_set_trigger_time(uint32_t low, uint32_t high)
{
    wifitsf->EVENT0_LOW = low;
    wifitsf->EVENT0_HIGH = high;
    wifitsf->EVENT0 = TSF_EVENT0_ENABLE;
}

#endif /* WFTSF_BASE */
