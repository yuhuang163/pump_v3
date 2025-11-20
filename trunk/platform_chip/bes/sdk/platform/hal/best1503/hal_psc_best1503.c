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
#include "cmsis.h"
#include "hal_chipid.h"
#include "hal_gpio.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include CHIP_SPECIFIC_HDR(reg_psc)

// #define MCU_RAM_CE_ALWAYS_ON

#define PSC_WRITE_ENABLE                    0xCAFE0000

enum PSC_INTR_MASK3_T {
    PSC_INTR_MASK3_GPIO2_15_0   = (0xFFFF << 0),
    PSC_INTR_MASK3_TIMER        = (1 << 16),
    PSC_INTR_MASK3_INTPMU       = (1 << 17),
    PSC_INTR_MASK3_WDT          = (1 << 18),
    PSC_INTR_MASK3_EXTPMU       = (1 << 19),
    PSC_INTR_MASK3_VAD          = (1 << 20),
    PSC_INTR_MASK3_CODEC        = (1 << 21),
    PSC_INTR_MASK3_WAKEUP_BT    = (1 << 22),
    PSC_INTR_MASK3_WAKEUP_SENS  = (1 << 23),
    PSC_INTR_MASK3_WAKEUP_MCU   = (1 << 24),
};

static struct AONPSC_T * const psc = (struct AONPSC_T *)AON_PSC_BASE;

static uint8_t dis_mod_map;
STATIC_ASSERT(sizeof(dis_mod_map) * 8 >= HAL_PSC_DISPLAY_MOD_QTY, "dis_mod_map size too small");

void hal_psc_jtag_select_mcu(void)
{
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 3);
    psc->REG_064 |= PSC_AON_CODEC_RESERVED(1 << 2);
}

void hal_psc_jtag_select_bt(void)
{
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 3);
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 2);
}

void hal_psc_jtag_select_sens(void)
{
    psc->REG_064 |= PSC_AON_CODEC_RESERVED(1 << 3);
}

void BOOT_TEXT_FLASH_LOC hal_psc_init(void)
{
    uint32_t mcu_wake_mask;
    uint32_t sens_wake_mask;
    uint32_t reg_07c;
    uint32_t reg_0fc;
    uint32_t reg_0f8;
    mcu_wake_mask = 0;
    sens_wake_mask = 0;

#ifdef INTPMU_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_INTPMU;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_INTPMU;
#endif
#ifdef WDT_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_WDT;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_WDT;
#endif
#ifdef EXTPMU_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_EXTPMU;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_EXTPMU;
#endif
#ifdef CODEC_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_CODEC;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_CODEC;
#endif

    // Setup MCU wakeup mask
    psc->REG_080 = 0;
    psc->REG_084 = 0;
    psc->REG_088 = PSC_INTR_MASK3_TIMER | PSC_INTR_MASK3_WAKEUP_MCU | mcu_wake_mask;

    // Setup BT wakeup mask
    psc->REG_098 = 0;
    psc->REG_09C = 0;
    psc->REG_0A0 = PSC_INTR_MASK3_WAKEUP_BT;

    // Setup SENS wakeup mask
    psc->REG_0B0 = 0;
    psc->REG_0B4 = 0;
    psc->REG_0B8 = PSC_INTR_MASK3_VAD | PSC_INTR_MASK3_WAKEUP_SENS | sens_wake_mask;

    // Setup power down/up timer
    psc->REG_008 = PSC_AON_MCU_POWERDN_TIMER4(3) | PSC_AON_MCU_POWERDN_TIMER5(0);
    psc->REG_00C = PSC_AON_MCU_POWERUP_TIMER2(0)| PSC_AON_MCU_POWERUP_TIMER4(7);

    psc->REG_048 = SET_BITFIELD(psc->REG_048, PSC_AON_MCU_RETMEM_PE_POWERDN_TIMER1, 1);

    reg_07c = psc->REG_07C;
#ifdef CAPSENSOR_ENABLE
    // only version A~C need to do this
    if (hal_cmu_get_aon_revision_id() <= 0x8) {
        reg_07c |= PSC_AON_SRAM4_ISO_EN_REG | PSC_AON_SRAM5_ISO_EN_REG | PSC_AON_SRAM6_ISO_EN_REG | PSC_AON_MCU_RETMEM_ISO_EN_DR;
        reg_07c &= ~(PSC_AON_SRAM4_ISO_EN_DR | PSC_AON_SRAM5_ISO_EN_DR | PSC_AON_SRAM6_ISO_EN_DR | PSC_AON_MCU_RETMEM_ISO_EN_REG);
    } else {
        reg_07c |= PSC_AON_SRAM4_ISO_EN_REG | PSC_AON_SRAM5_ISO_EN_REG | PSC_AON_SRAM6_ISO_EN_REG | PSC_AON_MCU_RETMEM_ISO_EN_REG;
        reg_07c &= ~(PSC_AON_SRAM4_ISO_EN_DR | PSC_AON_SRAM5_ISO_EN_DR | PSC_AON_SRAM6_ISO_EN_DR | PSC_AON_MCU_RETMEM_ISO_EN_DR);
    }
#else
    reg_07c |= PSC_AON_SRAM4_ISO_EN_REG | PSC_AON_SRAM5_ISO_EN_REG | PSC_AON_SRAM6_ISO_EN_REG | PSC_AON_MCU_RETMEM_ISO_EN_REG;
    reg_07c &= ~(PSC_AON_SRAM4_ISO_EN_DR | PSC_AON_SRAM5_ISO_EN_DR | PSC_AON_SRAM6_ISO_EN_DR | PSC_AON_MCU_RETMEM_ISO_EN_DR);
#endif
    psc->REG_07C = PSC_WRITE_ENABLE | reg_07c;

    reg_0fc = psc->REG_0FC;
#if defined(CORE_SLEEP_POWER_DOWN)
    //mcu ram
#ifdef MCU_RAM_CE_ALWAYS_ON
    reg_0fc &= ~PSC_AON_MCU_RETMEM_PDEN;
#else
    reg_0fc |= PSC_AON_MCU_RETMEM_PDEN;
#endif
    //share mem
    reg_0fc |= PSC_AON_SRAM4_PE_PSW_EN_REG | PSC_AON_SRAM5_PE_PSW_EN_REG | PSC_AON_SRAM6_PE_PSW_EN_REG;
    reg_0fc &= ~(PSC_AON_SRAM4_CE_PSW_EN_REG | PSC_AON_SRAM5_CE_PSW_EN_REG | PSC_AON_SRAM6_CE_PSW_EN_REG |
            PSC_AON_SRAM4_PE_PSW_EN_DR | PSC_AON_SRAM5_PE_PSW_EN_DR | PSC_AON_SRAM6_PE_PSW_EN_DR);
#else
    reg_0fc &= ~PSC_AON_MCU_RETMEM_PDEN;
#endif
    psc->REG_0FC = PSC_WRITE_ENABLE | reg_0fc;

    reg_0f8= psc->REG_0F8;
#ifdef CAPSENSOR_ENABLE
    // only version A~C need to do this
    if (hal_cmu_get_aon_revision_id() <= 0x8) {
        reg_0f8 |= PSC_AON_MCU_RETMEM_PE_PSW_EN_DR;
        reg_0f8 &= ~(PSC_AON_BTC_RETMEM_PDEN | PSC_AON_MCU_RETMEM_PE_PSW_EN_REG);
    } else {
        reg_0f8 &= ~(PSC_AON_MCU_RETMEM_PE_PSW_EN_DR | PSC_AON_BTC_RETMEM_PDEN);
    }
#else
    reg_0f8 &= ~(PSC_AON_MCU_RETMEM_PE_PSW_EN_DR | PSC_AON_BTC_RETMEM_PDEN);
#endif
    psc->REG_0F8 = PSC_WRITE_ENABLE | reg_0f8;
    psc->REG_0EC = PSC_AON_REG_RAMRET_CFG(0xFF);

#ifndef ARM_CMSE
    uint32_t ram_map;
    // Power down unused share ram
    ram_map = 0x0007 & ~(
    gen_bth_shr_ram_mask(RAM_S_TO_NS(RAM_BASE), RAM_SIZE + AP_MAILBOX_SIZE)
#if defined(MEM_POOL_BASE) && defined(MEM_POOL_SIZE)
    | gen_bth_shr_ram_mask(RAM_S_TO_NS(MEM_POOL_BASE), MEM_POOL_SIZE)
#endif
                        );
    hal_psc_shr_ram_config(ram_map, HAL_PSC_SHR_RAM_FORCE_PD);
#endif

    hal_psc_bt_disable();
    hal_psc_display_disable(HAL_PSC_DISPLAY_MOD_ALL);
}

uint32_t hal_psc_get_power_loop_cycle_cnt(void)
{
    return 6 + 14 + 14; // 1.5 + 4.5 + 14 + 14
}

void SRAM_TEXT_LOC hal_psc_mcu_auto_power_down(void)
{
    psc->REG_018 = PSC_WRITE_ENABLE | 0;
    psc->REG_000 = PSC_WRITE_ENABLE | PSC_AON_MCU_PG_AUTO_EN;
    psc->REG_010 = PSC_WRITE_ENABLE | PSC_AON_MCU_POWERDN_START;
}

void SRAM_TEXT_LOC hal_psc_mcu_auto_power_up(void)
{
    psc->REG_014 = PSC_WRITE_ENABLE | PSC_AON_MCU_POWERUP_START;
}

void SRAM_TEXT_LOC hal_psc_mcu_rcmode_manual_pu(void)
{
    psc->REG_018 = PSC_WRITE_ENABLE |
        PSC_AON_MCU_MEM_PSW_EN_DR |
        PSC_AON_MCU_PSW_EN_DR |
        PSC_AON_MCU_RESETN_ASSERT_DR |
        PSC_AON_MCU_ISO_EN_DR |
        PSC_AON_MCU_CLK_STOP_DR;

    psc->REG_000 = PSC_WRITE_ENABLE | PSC_AON_MCU_PG_AUTO_EN | PSC_AON_MCU_PG_HW_EN;
}

void SRAM_TEXT_LOC hal_psc_mcu_rcmode_manual_pu_resume(void)
{
    psc->REG_000 = PSC_WRITE_ENABLE | PSC_AON_MCU_PG_AUTO_EN;
}

void BOOT_TEXT_FLASH_LOC hal_psc_display_enable(enum HAL_PSC_DISPLAY_MOD_T mod)
{
    uint32_t lock;
    bool set = true;

    if (mod > HAL_PSC_DISPLAY_MOD_QTY) {
        return;
    };

    lock = int_lock();
    if (mod < HAL_PSC_DISPLAY_MOD_ALL) {
        if (dis_mod_map) {
            set = false;
        }
        dis_mod_map |= (1 << mod);
    }
    int_unlock(lock);

    if (!set) {
        while (psc->REG_028 & PSC_AON_DIS_CLK_STOP_REG) {
			hal_sys_timer_delay(MS_TO_TICKS(1));
        }
        return;
    }

    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
        PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR |
        PSC_AON_DIS_CLK_STOP_DR;
}

void BOOT_TEXT_FLASH_LOC hal_psc_display_disable(enum HAL_PSC_DISPLAY_MOD_T mod)
{
    uint32_t lock;
    bool set = true;

    if (mod > HAL_PSC_DISPLAY_MOD_QTY) {
        return;
    };

    lock = int_lock();

    if (mod < HAL_PSC_DISPLAY_MOD_ALL) {
        dis_mod_map &= ~(1 << mod);
        if (dis_mod_map) {
            set = false;
        }
    }

    if (set) {
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR |
            PSC_AON_DIS_ISO_EN_DR |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR | PSC_AON_DIS_MEM_PSW_EN_REG |
            PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    }

    int_unlock(lock);
}

void BOOT_TEXT_FLASH_LOC hal_psc_codec_enable(void)
{
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR;
}

void BOOT_TEXT_FLASH_LOC hal_psc_codec_disable(void)
{
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR | PSC_AON_CODEC_MEM_PSW_EN_REG |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
}

void BOOT_TEXT_FLASH_LOC hal_psc_bt_enable(void)
{
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR;

#ifdef JTAG_BT
    hal_psc_jtag_select_bt();
#endif
}

void BOOT_TEXT_FLASH_LOC hal_psc_bt_disable(void)
{
#ifdef JTAG_BT
    hal_psc_jtag_select_mcu();
#endif

    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR | PSC_AON_BT_MEM_PSW_EN_REG |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
}

void hal_psc_bt_enable_auto_power_down(void)
{
    psc->REG_020 = PSC_WRITE_ENABLE | PSC_AON_BT_PG_AUTO_EN | PSC_AON_BT_PG_HW_EN;
    psc->REG_038 = PSC_WRITE_ENABLE | 0;

    psc->REG_024 |= PSC_AON_BT_SLEEP_NO_WFI;
}

void hal_psc_rf_inf_enable_auto_power_down(void)
{
    uint32_t reg_044 = 0;

    reg_044 = psc->REG_044;
    reg_044 &= ~(PSC_AON_REG_PU_RF_INF_DR);
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;
}

void hal_psc_rf_inf_disable_auto_power_down(void)
{
    uint32_t reg_044 = 0;

    reg_044 = psc->REG_044;
    reg_044 |= PSC_AON_REG_PU_RF_INF_DR | PSC_AON_REG_PU_RF_INF;
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;
}

void SRAM_TEXT_LOC hal_psc_core_auto_power_down(int core_only)
{
#ifdef CHIP_ROLE_CP
        hal_cmu_cp_sleep_signal_set();
        SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
        __DSB();
        __WFI();
#else
#ifdef ARM_CMNS
    TZ_hal_sleep_deep_sleep_S();
#else
    hal_psc_mcu_auto_power_down();
#endif
#endif
}

BOOT_TEXT_FLASH_LOC
uint32_t gen_bth_shr_ram_mask(uint32_t base, uint32_t size)
{
    uint32_t start, end, idx, cnt;
    uint32_t mask;

    if (base <= RAM4_BASE) {
        start = 0;
    } else {
        start = base - RAM4_BASE;
    }

    if (base + size <= RAM4_BASE) {
        end = 0;
    } else {
        end = base + size - RAM4_BASE;
    }
    idx = start / SHR_RAM_BLK_SIZE;
    cnt = ((end - start) + SHR_RAM_BLK_SIZE - 1)/ SHR_RAM_BLK_SIZE;
    mask = ((1 << cnt) - 1) << idx;
    return mask;
}

BOOT_TEXT_SRAM_LOC
void hal_psc_shr_ram_config(uint32_t map, enum HAL_PSC_SHR_RAM_PD_CFG_T cfg)
{
    uint32_t reg_0fc = 0;
    uint32_t bfv = 0;
    uint32_t dr_mask = 0;
    uint32_t i = 0;

    reg_0fc = psc->REG_0FC;

    for (i = 0; i < 3; i++) {
        if (map & (1 << i)) {
            bfv |= (PSC_AON_SRAM4_PE_PSW_EN_REG << i) | (PSC_AON_SRAM4_CE_PSW_EN_REG << i);
            dr_mask |= (PSC_AON_SRAM4_PE_PSW_EN_DR << i);
        }
    }

    if (cfg == HAL_PSC_SHR_RAM_FORCE_PU) {
        reg_0fc |= dr_mask;
        reg_0fc &= ~bfv;
    } else if (cfg == HAL_PSC_SHR_RAM_FORCE_PD) {
        reg_0fc |= dr_mask | bfv;
    } else if (cfg == HAL_PSC_SHR_RAM_SLEEP_PU) {
        reg_0fc &= ~dr_mask;
        reg_0fc &= ~bfv;
    } else {
        reg_0fc &= ~dr_mask;
        reg_0fc |= bfv;
    }
    psc->REG_0FC = PSC_WRITE_ENABLE | reg_0fc;
}

void hal_psc_mcu_gpio_irq_enable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_080 |= (1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_084 |= (1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_088 |= (1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

void hal_psc_mcu_gpio_irq_disable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_080 &= ~(1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_084 &= ~(1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_088 &= ~(1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

uint32_t hal_psc_mcu_gpio_irq_get_status(uint32_t *status, uint32_t cnt)
{
    volatile uint32_t * const irq_status[] = {
        &psc->REG_08C,
        &psc->REG_090,
        &psc->REG_094,
    };
    const uint32_t irq_tab_sz = ARRAY_SIZE(irq_status);
    uint32_t i;

    if (cnt > irq_tab_sz) {
        cnt = irq_tab_sz;
    }
    for (i = 0; i < cnt; i++) {
        status[i] = *irq_status[i];
        if (i == (irq_tab_sz - 1))
            status[i] &= PSC_INTR_MASK3_GPIO2_15_0;
    }

    return cnt;
}

void hal_psc_gpio_irq_enable(enum HAL_GPIO_PIN_T pin)
{
    hal_psc_mcu_gpio_irq_enable(pin);
}

void hal_psc_gpio_irq_disable(enum HAL_GPIO_PIN_T pin)
{
    hal_psc_mcu_gpio_irq_disable(pin);
}

uint32_t hal_psc_gpio_irq_get_status(uint32_t *status, uint32_t cnt)
{
    return hal_psc_mcu_gpio_irq_get_status(status, cnt);
}
