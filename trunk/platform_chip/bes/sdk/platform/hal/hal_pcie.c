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
#include "plat_addr_map.h"
#ifdef PCIE_AHB_BASE
#include "hal_location.h"
#include "hal_pcie.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_chipid.h"
#include "hal_iomux.h"
#include "cmsis_nvic.h"
#include "pciephy.h"
#include "reg_pcie.h"
#include "pmu.h"

static struct PCIE_T * const pcie = (struct PCIE_T *)PCIE_AHB_BASE;

static struct PCIE_APB_T * const pcie_apb = (struct PCIE_APB_T *)PCIE_APB_BASE;

enum pcie_gen {
    PCIE_GEN1 = 4,
    PCIE_GEN2 = 5,
};

enum pcie_bar {
    BAR0 = 0,
    BAR1,
};

uint32_t hal_pcie_get_dig_status(void)
{
    return pcie->STATUS;
}

/*
**config pcie gen,default is gen3;
**val: 4 is gen1; 5 is gen2;
**default is gen3, gen3 use other register;
*/
void hal_pcie_gen_set(enum pcie_gen gen)
{
    if(gen == PCIE_GEN1)
        pcie_apb->REG21C = 0x1000;
    /*0x02800000 set Network controller*/
    pcie_apb->GEN = (0x2800000 | gen);
}

/*
**set pcie bar address
**call it after pcie_init()
*/
void hal_pcie_barAddr_set(enum pcie_bar bar, uint32_t addr)
{
    switch(bar)
    {
        case BAR0:
            pcie->CFG_BAR0 = addr;
            break;
        case BAR1:
            pcie->CFG_BAR1 = addr;
            break;
        default:
            break;
    }
}

#ifndef ROM_BUILD
void hal_pcie_sel_wifi(void)
{
    hal_cmu_pcie_wifi_init();
    hal_pcie_barAddr_set(BAR0, WIFI_SUBSYS_BASE);
    hal_cmu_pcie_sel_wifi();
}
#endif

bool hal_pcie_get_ts1_status(void)
{
    return ((pcie->STATUS & 0x10) == 0x10);
}

void hal_pcie_check_idle(void)
{
    pciephy_check_idle();
}

void hal_pcie_state_check(void)
{
    pciephy_state_check();
}

static void hal_pcie_irq_handler(void)
{
    hal_pcie_state_check();
}

void hal_pcie_register_irq(void)
{
    NVIC_SetVector(PCIE_IRQn, (uint32_t)hal_pcie_irq_handler);
    NVIC_SetPriority(PCIE_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(PCIE_IRQn);
    NVIC_EnableIRQ(PCIE_IRQn);
    pciephy_open_state_intr();
}

static void hal_pcie_dig_open(void)
{
    hal_cmu_pcie_clock_use_osc();
    pcie_apb->LTSSM_CTRL |= PCIE_APB_EN_ACCESS;
    pcie_apb->ID = PCIE_APB_VENDOR_ID(PCIE_VENDOR_ID) | PCIE_APB_DEVICE_ID(PCIE_DEVICE_ID);
    pcie_apb->LTSSM_CTRL &= ~PCIE_APB_EN_ACCESS;
    hal_cmu_pcie_clock_use_host();

#ifdef ROM_BUILD
    hal_pcie_barAddr_set(BAR0, RAM0_BASE);
#endif

#ifdef CHIP_BEST2005
    /*fc control*/
    pcie_apb->REG1010 = 0x450;
    /*block bit*/
    pcie->REG1 = 0x1801;
#endif
    hal_pcie_gen_set(PCIE_GEN1);
}

void hal_pcie_open(void)
{
    hal_cmu_pcie_clock_enable();
    hal_cmu_pcie_reset_clear();
    hal_sys_timer_delay_us(10);

    pciephy_open();
    hal_pcie_dig_open();
}

void hal_pcie_suspend(void)
{
    pciephy_sleep();
    hal_cmu_pcie_clock_disable();
}

void hal_pcie_resume(void)
{
    pciephy_wakeup();
    hal_cmu_pcie_clock_enable();
    hal_sys_timer_delay_us(10);
    hal_pcie_dig_open();
}

#endif
