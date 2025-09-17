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

#ifdef WFTSF_BASE

#include "hal_cmu.h"
#include "hal_wifitsf.h"
#include "reg_wifitsf.h"
#define HW_READ_REG(a) *(volatile uint32_t *)(a)
#define HW_WRITE_REG(a,v) *(volatile uint32_t *)(a) = v

static struct WIFITSF_T * const wifitsf = (struct WIFITSF_T *)WFTSF_BASE;

void hal_wifitsf_enable(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_TSF);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_TSF);

    // use event0, dma rising edge
     wifitsf->CTRL = TIMER_CONTROL_TSF_ENABLE | DMA_TRIG_TSf_ENABLE | USB_TRIG_TSf_ENABLE | WIFI_TSF_UPDATA_ENABLE |
        TSF_PRESCALE(hal_cmu_get_crystal_freq()/1000000 - 1);

    wifitsf->INT_MASK = TSF_EVENT0_INT_MASK | TSF_USB_TRIG_INT_MASK;
}

void hal_wifitsf_disable(void)
{
    wifitsf->CTRL &= ~TIMER_CONTROL_TSF_ENABLE;

    hal_cmu_reset_set(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_reset_set(HAL_CMU_MOD_O_TSF);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_WF_TSF);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_TSF);
}

void hal_wifitsf_timer_set(uint32_t low, uint32_t high)
{
    wifitsf->CTRL &= ~TIMER_CONTROL_TSF_ENABLE;
    wifitsf->TIME_LOW = low;
    wifitsf->TIME_HIGH = high;
    wifitsf->CTRL |= TIMER_CONTROL_TSF_ENABLE;
}

uint64_t hal_wifitsf_timer_get(void)
{
    uint32_t low = wifitsf->TIME_LOW;
    uint32_t high = wifitsf->TIME_HIGH;

    uint64_t wifitsf_timer = high;
    wifitsf_timer = (wifitsf_timer << 32) | low;

    return wifitsf_timer;
}

uint64_t hal_wifitsf_dma_time_get(void)
{
    uint32_t low = wifitsf->DMA_TIME_LOW;
    uint32_t high = wifitsf->DMA_TIME_HIGH;

    uint64_t wifitsf_dma_timer = high;
    wifitsf_dma_timer = (wifitsf_dma_timer << 32) | low;

    return wifitsf_dma_timer;
}

uint32_t hal_wifitsf_dma_time_get_high(void)
{
    return wifitsf->DMA_TIME_HIGH;
}

void hal_wifitsf_set_usb_tirg_shreshold(uint32_t val)
{
    if (val > 0)
        wifitsf->USB_CTRL = USB_TRIG_THRESHOLD(val - 1);
}

uint32_t hal_wifitsf_usb_time_get()
{
    return wifitsf->USB_TIME_LOW;
}

void hal_wifitsf_set_trigger_time(uint32_t low, uint32_t high)
{
#if CHIP_WIFITSF_VER >= 2
    wifitsf->EVENT0_LOW = low;
    wifitsf->EVENT0_HIGH = high;
    wifitsf->EVENT0 = TSF_EVENT0_ENABLE;
#else
    wifitsf->EVENT0 = TSF_EVENT0_ENABLE | TSF_EVENT0(low);
#endif
}

#endif /* WFTSF_BASE */
