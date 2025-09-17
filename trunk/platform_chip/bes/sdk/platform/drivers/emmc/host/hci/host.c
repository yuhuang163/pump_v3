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
#include "card.h"
#include "core.h"
#include "host.h"
#include "error.h"
#include "string.h"
#include "emmc_dbg.h"
#include "sdhci_emphy.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"

static void sdhci_utility_xml_window(unsigned int voltage, unsigned int s, unsigned int e, const char *type)
{
    HAL_EMMC_TRACE(1, "voltage=%d, type=%s, timing=%d-%d", voltage, type, s, e);
}

void sdhci_open_internal_cmd_gate(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_op_ext_reg(host);
    val &= ~(0x3 << 8);
    sdhciWriteSdhc_op_ext_reg(host, val);
}

void sdhci_close_internal_cmd_gate(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_op_ext_reg(host);
    val |= 0x3 << 8;
    sdhciWriteSdhc_op_ext_reg(host, val);
}

void sdhci_set_clk_cnt_bf_gate(struct mmc_host *host, unsigned int clkcnt)
{
    uint32_t val;

    if ((clkcnt >= 8) && (clkcnt <= 0xF)) {//default 9, min 8
        val = sdhciReadSdhc_op_ext_reg(host);
        val &= ~(0xF << 16);
        val |= clkcnt << 16;
        sdhciWriteSdhc_op_ext_reg(host, val);
    }
}

int sdhci_get_clk_cnt_bf_gate(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_op_ext_reg(host);
    val = (val >> 16) & 0xF;
    return val;
}

int sdhci_set_turn_around_cycle(struct mmc_host *host, unsigned int data0_cycle, unsigned int cmd_cycle)
{
    uint32_t val;
    uint8_t flag = 0;

    val = sdhciReadSdhc_roundtrip_timing_reg(host);

    //cfg bit19-16
    if (data0_cycle <= 0xF) {
        val &= ~(0xF << 16);
        val |= data0_cycle << 16;
        flag = 1;
    }

    //cfg bit11-8
    if (cmd_cycle <= 0xF) {
        val &= ~(0xF << 8);
        val |= cmd_cycle << 8;
        flag = 1;
    }

    if (flag) {
        switch (host->speed_mode) {
            case SDR50:
            case SDR52:
            case DDR50:
            case DDR52:
                //cfg bit1
                val |= 1 << 1;
                break;
            case HS200:
            case HS100:
            case HS50:
            case HS25:
            case SDR104:
                //cfg bit0
                val |= 1 << 0;
                break;
            case HS400:
            case EHS400:
                //cfg bit2
                val |= 1 << 2;
                break;
            default:
                HAL_EMMC_ERROR(0, "%s: speed mode %d does not support this function", __FUNCTION__, host->speed_mode);
                return MMC_INVALID_ERR;
        }
        sdhciWriteSdhc_roundtrip_timing_reg(host, val);
    }

    return MMC_NO_ERR;
}

void sdhci_get_turn_around_cycle(struct mmc_host *host, unsigned int *data0_cycle, unsigned int *cmd_cycle)
{
    uint32_t val;

    val = sdhciReadSdhc_roundtrip_timing_reg(host);

    if (data0_cycle) {
        *data0_cycle = (val >> 16) & 0x0F;
    }

    if (cmd_cycle) {
        *cmd_cycle = (val >> 8) & 0x0F;
    }
}

void sdhci_set_oclk_out(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdh_regs(host->base + 0x808);
    val &= ~(1 << 8);
    val |= (1 << 5);
    sdhciWriteSdh_regs(host->base + 0x808, val);
}

void sdhci_force_clk_on(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_op_ext_reg(host);
    val |= (3 << 11); //set bit11 and bit12
    sdhciWriteSdhc_op_ext_reg(host, val);
}

void sdhci_force_clk_off(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_op_ext_reg(host);
    val &= ~(3 << 11); //clear bit11 and bit12
    sdhciWriteSdhc_op_ext_reg(host, val);
}

void sdhci_sel_base_clk(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_tx_cfg_reg(host);
    val &= ~(1 << 30);
    sdhciWriteSdhc_tx_cfg_reg(host, val);
}

void sdhci_sel_work_clk(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_tx_cfg_reg(host);
    val |= (1 << 30);
    sdhciWriteSdhc_tx_cfg_reg(host, val);
}

void sdhci_set_clk_div_bypass(struct mmc_host *host)
{
    uint32_t val;

    val = sdhciReadSdhc_bypass_cfg(host);
    val |= 1 << 31;
    sdhciWriteSdhc_bypass_cfg(host, val);
}

int sdhci_start_int_clk(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int start;
    unsigned int timeout = MS_TO_FAST_TICKS(WAIT_INTE_CLK_TIMEOUT);

    sdhciWriteSdh_clock_ctrlInt_clk_en(host, 1);
    start = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - start < timeout) {
            if (sdhciReadSdh_clock_ctrlInt_clk_stable(host)) {
                break;
            }
            host->delay(1);
        } else {
            HAL_EMMC_ERROR(0, "Failed to enable SDH internal clock!");
            ret = MMC_INT_CLK_ERR;
            break;
        }
    }

    return ret;
}

int sdhci_stop_int_clk(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int start;
    unsigned int timeout = MS_TO_FAST_TICKS(WAIT_INTE_CLK_TIMEOUT);

    sdhciWriteSdh_clock_ctrlInt_clk_en(host, 0);
    start = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - start < timeout) {
            if (sdhciReadSdh_clock_ctrlInt_clk_stable(host)) {
                break;
            }
            host->delay(1);
        } else {
            HAL_EMMC_ERROR(0, "Failed to disable SDH internal clock!");
            ret = MMC_INT_CLK_ERR;
            break;
        }
    }

    return ret;
}

void sdhci_start_bus_clk(struct mmc_host *host)
{
    sdhciWriteSdh_clock_ctrlClock_en(host, 1);
}

void sdhci_stop_bus_clk(struct mmc_host *host)
{
    sdhciWriteSdh_clock_ctrlClock_en(host, 0);
}

static void sdhci_clear_set_irqs(struct mmc_host *host, unsigned int clear, unsigned int set, unsigned int status_en)
{
    unsigned int ier;

    ier = (sdhciReadSdh_e_stat_en(host) << 16) + sdhciReadSdh_i_stat_en(host);
    ier &= ~clear;
    ier |= set | status_en;
    sdhciWriteSdh_e_stat_en(host, (ier >> 16));
    sdhciWriteSdh_i_stat_en(host, ier);
    HAL_EMMC_TRACE(1, "SDH_NORMAL_STATUS_EN..........[0x%04X] = 0x%04X", (unsigned int)(SDH_NORMAL_STATUS_EN), sdhciReadSdh_i_stat_en(host));
    HAL_EMMC_TRACE(1, "SDH_ERROR_STATUS_EN...........[0x%04X] = 0x%04X", (unsigned int)(SDH_ERROR_STATUS_EN), sdhciReadSdh_e_stat_en(host));

    /*interrupt signal may have different setting*/
    ier = (sdhciReadSdh_e_sign_en(host) << 16) + sdhciReadSdh_i_sign_en(host);
    ier &= ~clear;
    ier |= set;
    sdhciWriteSdh_e_sign_en(host, (ier >> 16));
    sdhciWriteSdh_i_sign_en(host, ier);
    HAL_EMMC_TRACE(1, "SDH_NORMAL_STATUS_INT_EN......[0x%04X] = 0x%04X", (unsigned int)(SDH_NORMAL_STATUS_INT_EN), sdhciReadSdh_i_sign_en(host));
    HAL_EMMC_TRACE(1, "SDH_ERROR_STATUS_INT_EN.......[0x%04X] = 0x%04X", (unsigned int)(SDH_ERROR_STATUS_INT_EN), sdhciReadSdh_e_sign_en(host));
}

static void sdhci_clear_set_raw_status(struct mmc_host *host, unsigned int clear, unsigned int set)
{
    unsigned int ier;

    ier = (sdhciReadSdh_e_stat_en(host) << 16) + sdhciReadSdh_i_stat_en(host);
    ier &= ~clear;
    ier |= set;
    sdhciWriteSdh_e_stat_en(host, (ier >> 16));
    sdhciWriteSdh_i_stat_en(host, ier);
    HAL_EMMC_TRACE(1, "SDH_NORMAL_STATUS_EN..........[0x%04X] = 0x%04X", (unsigned int)(SDH_NORMAL_STATUS_EN), sdhciReadSdh_i_stat_en(host));
    HAL_EMMC_TRACE(1, "SDH_ERROR_STATUS_EN...........[0x%04X] = 0x%04X", (unsigned int)(SDH_ERROR_STATUS_EN), sdhciReadSdh_e_stat_en(host));
}

void sdhci_mask_irqs(struct mmc_host *host, unsigned int irqs)
{
    sdhci_clear_set_irqs(host, irqs, 0, 0);
}

void sdhci_unmask_irqs(struct mmc_host *host, unsigned int irqs)
{
    sdhci_clear_set_irqs(host, 0, irqs, 0);
}

void sdhci_unmask_irqs_ext(struct mmc_host *host, unsigned int irqs, unsigned int status_en)
{
    sdhci_clear_set_irqs(host, 0, irqs, status_en);
}

void sdhci_mask_raw_status(struct mmc_host *host, unsigned int status)
{
    sdhci_clear_set_raw_status(host, status, 0);
}

void sdhci_unmask_raw_status(struct mmc_host *host, unsigned int status)
{
    sdhci_clear_set_raw_status(host, 0, status);
}

void sdhci_set_bus_clk(struct mmc_host *host, unsigned int khz)
{
    unsigned int div;

    if (host->ios.khz == khz) {
        HAL_EMMC_TRACE(1, "Host already in %d khz, skip..", khz);
        return;
    }

    if (!host->clk_in_khz) {
        HAL_EMMC_ERROR(1, "%s, host->clk_in_khz cannot be 0", __FUNCTION__);
        return;
    }

    HAL_EMMC_TRACE(0, "host->clk_in_khz:%uKHz", host->clk_in_khz);
    div = host->clk_in_khz / khz;
    HAL_EMMC_TRACE(2, "%s: divider-begin = %d", __FUNCTION__, div);
    //BES modified the frequency division method of div: 2*(n + 1), 2021-12-22
    div >>= 1;
    if (div >= 1) {
        div -= 1;
    }

    //bypss clock frequency division method of div: 2*n, also bypass div
#if !defined(CHIP_BEST1600) && !defined(AUD_PLL_DOUBLE)
    if (khz == F_HS200) {    //div invalid
        sdhci_set_clk_div_bypass(host);
        HAL_EMMC_TRACE(0, "%s: divider-final(No rounding here) = 0, bypass mode", __FUNCTION__);
    } else {
        HAL_EMMC_TRACE(0, "%s: divider-final(No rounding here) = %d", __FUNCTION__, div);
    }
#else
    HAL_EMMC_TRACE(0, "%s: divider-final(No rounding here) = %d", __FUNCTION__, div);
#endif

    sdhci_stop_bus_clk(host);
    sdhciWriteSdh_clock_ctrlFreq_sel_hi(host, (div & 0x300) >> 8);
    sdhciWriteSdh_clock_ctrlFreq_sel_lo(host, (div & 0x0ff) >> 0);
    sdhci_start_bus_clk(host);

    host->ios.khz = khz;
    HAL_EMMC_TRACE(1, "%s: speed=%d KHz, div=%d", __FUNCTION__, khz, div);
    HAL_EMMC_TRACE(1, "SDH_CLOCK_CTRL[0x%04X] = 0x%04X", (unsigned int)(SDH_CLOCK_CTRL), sdhciReadSdh_clock_ctrl(host));
}

void sdhci_set_bus_width(struct mmc_host *host, unsigned int width)
{
    if (host->ios.width == width) {
        HAL_EMMC_TRACE(1, "Host already in bus width %d, skip..",  width);
        return;
    }
    switch (width) {
        case MMC_8BIT_WIDTH:
            sdhciWriteSdh_host_ctrlData_width(host, 0);
            sdhciWriteSdh_host_ctrlEx_data_width(host, 1);
            break;
        case MMC_4BIT_WIDTH:
            sdhciWriteSdh_host_ctrlData_width(host, 1);
            sdhciWriteSdh_host_ctrlEx_data_width(host, 0);
            break;
        case MMC_1BIT_WIDTH:
            sdhciWriteSdh_host_ctrlData_width(host, 0);
            sdhciWriteSdh_host_ctrlEx_data_width(host, 0);
            break;
    }
    host->ios.width = width;
    HAL_EMMC_TRACE(1, "%s: width=%d", __FUNCTION__, width);
    HAL_EMMC_TRACE(1, "SDH_HOST_CTRL[0x%04X] = 0x%04X", (unsigned int)(SDH_HOST_CTRL), sdhciReadSdh_host_ctrl(host));
}

void sdhci_set_bus_timing(struct mmc_host *host, unsigned int timing)
{
    if (host->ios.timing == timing) {
        HAL_EMMC_TRACE(1, "Host already in %d timing, skip..", timing);
        return;
    }
    switch (timing) {
        case EHS400:
        case HS400:
#if 1
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);//8bit ddr mode
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 4);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
#else
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);//16bit ddr mode,PHY does not support
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 6);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
#endif
            break;
        case HS200:
        case HS100:
        case HS50:
        case HS25:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 5);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case DDR50:
        case DDR26:
        case DDR52:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 4);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case SDR104:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 3);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case SDR50:
        case SDR52:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 2);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case SDR25:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 1);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case SDR12:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);
            sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(host, 0);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case HS:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 0);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 1);
            break;
        case DS:
            sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 0);
            sdhciWriteSdh_host_ctrlHi_speed_en(host, 0);
            break;
        default:
#ifdef EMMC_ASSERT
            HAL_EMMC_ASSERT(false, "%s:%d, Unsupported speed mode", __FUNCTION__, __LINE__);
#else
            HAL_EMMC_ERROR(0, "%s:%d, error:Unsupported speed mode", __FUNCTION__, __LINE__);
            return;
#endif
            break;
    }
    host->ios.timing = timing;
    HAL_EMMC_TRACE(1, "%s: timing=%d", __FUNCTION__, timing);
    HAL_EMMC_TRACE(1, "SDH_HOST_CTRL.................[0x%04X] = 0x%04X", (unsigned int)(SDH_HOST_CTRL), sdhciReadSdh_host_ctrl(host));
    HAL_EMMC_TRACE(1, "SDH_HOST_CTRL_2...............[0x%04X] = 0x%04X", (unsigned int)(SDH_HOST_CTRL_2), sdhciReadSdh_host_ctrl_2(host));
}

void sdhci_set_drv_strength(struct mmc_host *host, unsigned int drv)
{
    if (host->ios.drv == drv) {
        HAL_EMMC_TRACE(1, "Host already in %d driver strength, skip..", drv);
        return;
    }
    switch (drv) {
        case MMC_DRV_STRENGTH_B:
            sdhciWriteSdh_host_ctrl_2Drv_strength_sel(host, 0);
            break;
        case MMC_DRV_STRENGTH_A:
            sdhciWriteSdh_host_ctrl_2Drv_strength_sel(host, 1);
            break;
        case MMC_DRV_STRENGTH_C:
            sdhciWriteSdh_host_ctrl_2Drv_strength_sel(host, 2);
            break;
        case MMC_DRV_STRENGTH_D:
            sdhciWriteSdh_host_ctrl_2Drv_strength_sel(host, 3);
            break;
    }
    if (host->ops && host->ops->set_drv_strength) {
        host->ops->set_drv_strength(host, drv);
    }
    host->ios.drv = drv;
    HAL_EMMC_TRACE(1, "%s: drv=%d", __FUNCTION__, drv);
    HAL_EMMC_TRACE(1, "SDH_HOST_CTRL_2[0x%04X] = 0x%04X", (unsigned int)(SDH_HOST_CTRL_2), sdhciReadSdh_host_ctrl_2(host));
}

void sdhci_set_voltage(struct mmc_host *host, unsigned int voltage)
{
    if (host->ios.voltage == voltage) {
        HAL_EMMC_TRACE(1, "Host IO already in %d voltageh, skip..", voltage);
        return;
    }
    switch (voltage) {
        case MMC_IO_VOLTAGE_1P2:
            break;
        case MMC_IO_VOLTAGE_1P8:
            sdhciWriteSdh_host_ctrlBus_volt(host, 0x5);
            break;
        case MMC_IO_VOLTAGE_3P3:
            sdhciWriteSdh_host_ctrlBus_volt(host, 0x7);
            break;
    }
    if (host->ops && host->ops->set_io_voltage) {
        host->ops->set_io_voltage(host, voltage);
    }
    sdhciWriteSdh_host_ctrlBus_power(host, 1);
    host->ios.voltage = voltage;
    HAL_EMMC_TRACE(1, "%s: voltage=%d", __FUNCTION__, voltage);
    HAL_EMMC_TRACE(1, "SDH_HOST_CTRL[0x%04X] = 0x%04X", (unsigned int)(SDH_HOST_CTRL), sdhciReadSdh_host_ctrl(host));
}

static void sdhci_software_tunning_prepare(struct mmc_host *host)
{
    unsigned int sdhc_rx_dline_reg_data = 0x09;
    unsigned int reg;

    /***RX dline configure = 0x9****/
    /*bit[1:]=01, 896mv
        bit[3:2]=10, delay unit cell = 0x408ns
        bit[5:4]=00, pre-delay unit cell =0
        bit[6]=0, second delay stage is disable
    */
    /*Dline config*/
    reg = sdhciReadSDHC_DLINE_CFG(host);
    reg &= (~(0xFF << 0));
    reg |= sdhc_rx_dline_reg_data << 0;
    sdhciWriteSDHC_DLINE_CFG(host, reg);

    /*power up dline*/
    reg = sdhciReadSDHC_DLINE_CTRL(host);
    reg |= 0x1;
    sdhciWriteSDHC_DLINE_CTRL(host, reg);

    /*Select clock output from ddl*/
    reg = sdhciReadSdhc_rx_cfg_reg(host);
    reg &= (~(0x03 << 2));
    reg |= (0x1 << 2);
    sdhciWriteSdhc_rx_cfg_reg(host, reg);
}

static void sdhci_sw_tuning_delaycode(struct mmc_host *host, unsigned int delay)
{
    unsigned int reg;

    reg = sdhciReadSDHC_DLINE_CTRL(host);
    reg &= (~(0xFF << 16));
    reg |= (delay << 16);
    sdhciWriteSDHC_DLINE_CTRL(host, reg);
    host->delay(1);
}

#define TUNING_DELAY_MID                32UL
#define TUNING_DELAY_MAX                64UL

#ifndef EMMC_USE_HEAP
    static uint32_t rcvd_tuning[128 / sizeof(uint32_t)];
#endif
/**
*   The pre-defined data pattern for tuning, i.e., Max ISI.
*/
static unsigned int tune_blk_4bit[] = {0x00ff0fff, 0xccc3ccff, 0xffcc3cc3, 0xeffefffe,
                                       0xddffdfff, 0xfbfffbff, 0xff7fffbf, 0xefbdf777,
                                       0xf0fff0ff, 0x3cccfc0f, 0xcfcc33cc, 0xeeffefff,
                                       0xfdfffdff, 0xffbfffdf, 0xfff7ffbb, 0xde7b7ff7
                                      };
static unsigned int tune_blk_8bit[] = {0xff00ffff, 0x0000ffff, 0xccccffff, 0xcccc33cc,
                                       0xcc3333cc, 0xffffcccc, 0xffffeeff, 0xffeeeeff,
                                       0xffddffff, 0xddddffff, 0xbbffffff, 0xbbffffff,
                                       0xffffffbb, 0xffffff77, 0x77ff7777, 0xffeeddbb,
                                       0x00ffffff, 0x00ffffff, 0xccffff00, 0xcc33cccc,
                                       0x3333cccc, 0xffcccccc, 0xffeeffff, 0xeeeeffff,
                                       0xddffffff, 0xddffffff, 0xffffffdd, 0xffffffbb,
                                       0xffffbbbb, 0xffff77ff, 0xff7777ff, 0xeeddbb77
                                      };

static int sdhci_exe_software_tuning(struct mmc_host *host, unsigned int cmd)
{
    int ret = MMC_NO_ERR;
    unsigned int delay, max_delay = 256;
    unsigned int step = 1;
    unsigned int *rcvd, *expected;
    unsigned int i, sz;
    unsigned char *windows;
    unsigned int optd = 0, optl = 0;
    unsigned int curs = 0, cure = 0, curd = 0, curl = 0;
    unsigned int found = 0;
    uint8_t retries = EMMC_ERROR_RETRY_CNT;

    /*DLL setting for varies Host IP*/
    sdhci_software_tunning_prepare(host);
    if (sdhciReadSdh_host_ctrlEx_data_width(host)) {
        sz = 128;
        expected = tune_blk_8bit;
    } else {
        sz = 64;
        expected = tune_blk_4bit;
    }
#ifdef EMMC_USE_HEAP
    rcvd = (unsigned int *)emmc_heap_malloc(sz * sizeof(unsigned int));
    windows = (unsigned char *)emmc_heap_malloc(max_delay);
#else
    uint8_t windows_sw_tuning[256];
    rcvd = (unsigned int *)rcvd_tuning;
    windows = (unsigned char *)windows_sw_tuning;
#endif

    HAL_EMMC_ERROR(0, "  ");
    HAL_EMMC_ERROR(0, "%s start, data size:%d", __FUNCTION__, sz);
    /*reset tuning logic*/
    sdhciWriteSdh_host_ctrl_2Exe_tuning(host, 0);
    sdhciWriteSdh_host_ctrl_2Sampling_clk_sel(host, 0);
    for (delay = 0; delay < max_delay / step; delay += step) {
        HAL_EMMC_TRACE(1, "Delay %d", delay);
        sdhci_sw_tuning_delaycode(host, delay);
        mmc_prepare_transfer(host, (void *)rcvd, CARD_READ, sz, sz, false);
        do {
            ret = mmc_send_cmmand(host, cmd, 0, MMC_RSP_R1 | MMC_DATA_PRESENT | MMC_DATA_READ);
        } while (ret && retries--);
        if (ret) {
            HAL_EMMC_TRACE(0, "%s, mmc_send_cmmand error:%d", __FUNCTION__, ret);
        } else {
            ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1);
            if (ret) {
                HAL_EMMC_TRACE(0, "%s, mmc_wait_for_resp error:%d", __FUNCTION__, ret);
            } else {
                ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            }
        }
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_TRACE(1, "CMD%d - Waiting for Buffer Read Timed out!!! - SDH_STAT: 0x%08X *****", cmd, ((sdhciReadSdh_e_stat(host) << 16) | sdhciReadSdh_i_stat(host)));
            sdhciWriteSdh_e_stat(host, sdhciReadSdh_e_stat(host));
            sdhciWriteSdh_i_stat(host, sdhciReadSdh_i_stat(host));
            windows[delay / step] = 0;
        } else {
            windows[delay / step]  = 1;
            for (i = 0; i < sz / 4; i++) {
                HAL_EMMC_TRACE(1, "rcvd = 0x%08x expectd = 0x%08x %s", rcvd[i], expected[i], (rcvd[i] == expected[i]) ? "PASS" : "FAIL");
                if (rcvd[i] != expected[i]) {
                    windows[delay / step]  = 0;
                    break;
                }
            }
        }

        /*
        *   Q: need to reset cmd and data even pass the tuning cmd to avoid cmd inhibit
        */
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1);
    }

    /*Cal the optimal setting*/
    for (i = 0; i < max_delay / step; i += step) {
        if (found == 0 && windows[i] == 1) {
            curs = i;
            found = 1;
        }
        if (found == 1 && (windows[i] == 0 || i == (max_delay / step - step))) {
            cure = i - 1;
            curl = cure - curs + 1;
            curd = curs + curl / 2;
            HAL_EMMC_TRACE(1, "window [%d-%d]", curs, cure);
            if (host->quirks & MMC_QUIRKS_SHMOO_XML) {
                sdhci_utility_xml_window(host->ios.vccmain, curs, cure, "RX");
            }
            if (optl < curl) {
                optd = curd;
                optl = curl;
            }
            found = 0;
        }
    }

    /*tunning done, set the opt delay value*/
    HAL_EMMC_ERROR(1, "software tunning done, sampling point = [%d]", optd);
    sdhci_sw_tuning_delaycode(host, optd);

#ifdef EMMC_USE_HEAP
    emmc_heap_free((void *)rcvd);
    emmc_heap_free((void *)windows);
#endif

    return ret;
}

POSSIBLY_UNUSED
static int sdhci_exe_phy_tuning_cmdline(struct mmc_host *host, unsigned int cmd, uint8_t *window_width, uint8_t *tuning_val,
                                        uint8_t step, uint8_t tuning_val_data)
{
    int ret = MMC_NO_ERR;
    unsigned int delay;
    unsigned int i;
    unsigned char *windows;
    unsigned int cur/*current cursor*/, curs = 0/*start cursor*/, cur_mid;
    unsigned int found = 0;
    uint8_t retries = 2;

    uint8_t windows_phy_tuning[TUNING_DELAY_MAX];
    windows = (unsigned char *)windows_phy_tuning;

    HAL_EMMC_ERROR(0, "  ");
    HAL_EMMC_ERROR(0, "%s start", __FUNCTION__);
    for (delay = 0; delay < TUNING_DELAY_MAX; delay += step) {
        HAL_EMMC_TRACE(0, "Delay %d", delay);
        sdhci_host_emphy_rd_tune_dly_cali(host, delay, tuning_val_data);

        //send cmd
        do {
            ret = mmc_send_cmmand(host, cmd, (host->card->rca << 16), MMC_RSP_R1);
        } while (ret && retries--);
        if (ret) {
            HAL_EMMC_TRACE(0, "%s, mmc_send_cmmand error:%d", __FUNCTION__, ret);
        } else {
            ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1);
            if (ret) {
                HAL_EMMC_TRACE(0, "%s, mmc_wait_for_resp error:%d", __FUNCTION__, ret);
            }
        }

        if (ret != MMC_NO_ERR) {
            sdhciWriteSdh_e_stat(host, sdhciReadSdh_e_stat(host));
            sdhciWriteSdh_i_stat(host, sdhciReadSdh_i_stat(host));
            windows[delay] = 0;
        } else {
            windows[delay] = 1;
        }
        HAL_EMMC_TRACE(0, "windows[%d] = %d", delay, windows[delay]);

        /*
        *   Q: need to reset cmd and data even pass the tuning cmd to avoid cmd inhibit
        */
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1);
    }
    HAL_EMMC_TRACE(0, "  ");

    /*Cal the optimal setting*/
    for (i = 0; i < TUNING_DELAY_MAX; i += step) {
        if (found == 0 && windows[i] == 1) {
            cur = i;
            found = 1;
        }

        if (found) {
            if (windows[i]) {
                if (cur != i) {
                    windows[cur] += windows[i];
                }

                if (i >= (TUNING_DELAY_MAX - step)) {
                    if (windows[cur] > windows[0]) {
                        windows[0] = windows[cur];  //save count
                        curs = cur;                 //save index
                    }

                    HAL_EMMC_TRACE(0, "windows index, current = %d, final = %d", cur, curs);
                    HAL_EMMC_TRACE(0, "windows width, current = %d, final = %d", windows[cur], windows[0]);
                }
            } else {
                if (windows[cur] > windows[0]) {
                    windows[0] = windows[cur];  //save count
                    curs = cur;                 //save index
                }

                HAL_EMMC_TRACE(0, "windows index, current = %d, final = %d", cur, curs);
                HAL_EMMC_TRACE(0, "windows width, current = %d, final = %d", windows[cur], windows[0]);

                found = 0;//finad the next windows
            }
        }
    }
    HAL_EMMC_ERROR(0, "  ");

    if (windows[0]) {
        windows[0] = (windows[0] - 1) * step + 1;
        cur_mid = curs + windows[0] / 2;
        HAL_EMMC_ERROR(0, "window [%d-%d], width:%d", curs, curs + windows[0] - 1, windows[0]);
    } else {
        cur_mid = 32;
        HAL_EMMC_ERROR(0, "window [0-0], width:0, sampling point is set to 32");
    }

    /*tunning done, set the opt delay value*/
    HAL_EMMC_ERROR(0, "phy tunning done, sampling point = [%d]", cur_mid);
    sdhci_host_emphy_rd_tune_dly_cali(host, cur_mid, tuning_val_data);

    if (window_width) {
        *window_width = windows[0];
    }
    if (tuning_val) {
        *tuning_val = cur_mid;
    }

    return ret;
}

POSSIBLY_UNUSED
static int sdhci_tuning_cmdline_raw(struct mmc_host *host, uint8_t step, uint8_t window_thd, uint8_t *tuning_val_cmd)
{
    uint8_t width_cmd, width_cmd_rev, width_cmd_final;
    uint8_t val_cmd, val_cmd_rev, val_cmd_final;
    uint8_t data_dly_ori;
    int8_t diff1, diff2;

    host->tuning_status = 1;

    //read data delay
    sdhci_host_emphy_rd_tune_dly_read(host, NULL, &data_dly_ori);

    //tuning cmd line
    sdhci_exe_phy_tuning_cmdline(host, SEND_STATUS, &width_cmd, &val_cmd, step, data_dly_ori);
    HAL_EMMC_TRACE(0, "window width cmd: %d", width_cmd);
    if (width_cmd < window_thd) {//Force a negative edge tuning
        sdhci_host_emphy_pck_reverse(host, 1, 0);//reverse cmd line
        sdhci_exe_phy_tuning_cmdline(host, SEND_STATUS, &width_cmd_rev, &val_cmd_rev, step, data_dly_ori);
        HAL_EMMC_TRACE(0, "window width cmd: %d", width_cmd_rev);
        if (width_cmd > width_cmd_rev) {
            sdhci_host_emphy_pck_reverse(host, 1, 0);//reverse cmd line
            sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd, data_dly_ori);
            val_cmd_final = val_cmd;
            width_cmd_final = width_cmd;
        } else if (width_cmd == width_cmd_rev) {
            //Select the window closest to the midpoint
            if (val_cmd >= TUNING_DELAY_MID) {
                diff1 = val_cmd - TUNING_DELAY_MID;
            } else {
                diff1 = TUNING_DELAY_MID - val_cmd;
            }
            if (val_cmd_rev >= TUNING_DELAY_MID) {
                diff2 = val_cmd_rev - TUNING_DELAY_MID;
            } else {
                diff2 = TUNING_DELAY_MID - val_cmd_rev;
            }
            //Restore to the sampling point before reverse
            if (diff1 < diff2) {
                sdhci_host_emphy_pck_reverse(host, 1, 0);//reverse cmd line
                sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd, data_dly_ori);
                val_cmd_final = val_cmd;
                width_cmd_final = width_cmd;
            } else {
                val_cmd_final = val_cmd_rev;
                width_cmd_final = width_cmd_rev;
            }
        } else {
            val_cmd_final = val_cmd_rev;
            width_cmd_final = width_cmd_rev;
        }
    } else {
        val_cmd_final = val_cmd;
        width_cmd_final = width_cmd;
    }
    HAL_EMMC_ERROR(0, "final cmd window width: %d", width_cmd_final);

    //check cmd sampling piont value
    if (!val_cmd_final) {
        val_cmd_final = 32;
        if (tuning_val_cmd == NULL) {
            sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd_final, data_dly_ori);
        }
        HAL_EMMC_ERROR(0, "Forcefully modify the cmd sampling point to: %d", val_cmd_final);
    } else {
        if (host->speed_mode == EHS400 || host->speed_mode == HS400) {
            width_cmd_final = width_cmd_final / 4;
            if (val_cmd_final > width_cmd_final) {
                val_cmd_final = val_cmd_final - width_cmd_final;//Shift left 1/2
                HAL_EMMC_ERROR(0, "At HS400, cmd sampling point: %d", val_cmd_final);
                sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd_final, data_dly_ori);
            } else {
                HAL_EMMC_ERROR(0, "HS400 cmd window is too small to move left 1/2");
            }
        }
    }

    if (tuning_val_cmd) {
        *tuning_val_cmd = val_cmd_final;
    }
    HAL_EMMC_ERROR(0, "final cmd sampling point: %d", val_cmd_final);

    host->tuning_status = 0;
    return MMC_NO_ERR;
}

/*
Implemented functions:
  1. Compatible with the old version (1600 chip), just set step=1, tuning_val_cmd=0
  2. The old version did not distinguish between cmd and data tuning, but the new version does
  3. New function: only tune data, cmd uses the passed value
*/
static int sdhci_exe_phy_tuning(struct mmc_host *host, unsigned int cmd, uint8_t *window_width, uint8_t *tuning_val,
                                uint8_t step, uint8_t tuning_val_cmd)
{
    int ret = MMC_NO_ERR;
    unsigned int delay;
    unsigned int *rcvd, *expected;
    unsigned int i, sz;
    unsigned char *windows;
    unsigned int cur/*current cursor*/, curs = 0/*start cursor*/, cur_mid;
    unsigned int found = 0;
    uint8_t retries = 2;

    host->tuning_status = 1;
    if (sdhciReadSdh_host_ctrlEx_data_width(host)) {
        sz = 128;
        expected = tune_blk_8bit;
    } else {
        sz = 64;
        expected = tune_blk_4bit;
    }
#ifdef EMMC_USE_HEAP
    rcvd = (unsigned int *)emmc_heap_malloc(sz * sizeof(unsigned int));
    windows = (unsigned char *)emmc_heap_malloc(TUNING_DELAY_MAX);
#else
    uint8_t windows_phy_tuning[TUNING_DELAY_MAX];
    rcvd = (unsigned int *)rcvd_tuning;
    windows = (unsigned char *)windows_phy_tuning;
#endif

    HAL_EMMC_ERROR(0, "  ");
    HAL_EMMC_ERROR(0, "%s start, data size:%d", __FUNCTION__, sz);
    for (delay = 0; delay < TUNING_DELAY_MAX; delay += step) {
        HAL_EMMC_TRACE(0, "Delay %d", delay);
        sdhci_host_emphy_rd_tune_dly_cali(host, tuning_val_cmd, delay);
        mmc_prepare_transfer(host, (void *)rcvd, CARD_READ, sz, sz, false);//tuning do not support DMA mode
        do {
            ret = mmc_send_cmmand(host, cmd, 0, MMC_RSP_R1 | MMC_DATA_PRESENT | MMC_DATA_READ);
        } while (ret && retries--);
        if (ret) {
            HAL_EMMC_TRACE(0, "%s, mmc_send_cmmand error:%d", __FUNCTION__, ret);
        } else {
            ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1);
            if (ret) {
                HAL_EMMC_TRACE(0, "%s, mmc_wait_for_resp error:%d", __FUNCTION__, ret);
            } else {
                ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            }
        }
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_TRACE(0, "CMD%d - Waiting for Buffer Read Timed out!!! - SDH_STAT: 0x%08X *****", cmd, ((sdhciReadSdh_e_stat(host) << 16) | sdhciReadSdh_i_stat(host)));
            sdhciWriteSdh_e_stat(host, sdhciReadSdh_e_stat(host));
            sdhciWriteSdh_i_stat(host, sdhciReadSdh_i_stat(host));
            windows[delay] = 0;
        } else {
            windows[delay] = 1;
            for (i = 0; i < sz / 4; i++) {
                //HAL_EMMC_TRACE(0, "rcvd = 0x%08x expectd = 0x%08x %s", rcvd[i], expected[i], (rcvd[i] == expected[i]) ? "PASS" : "FAIL");
                if (rcvd[i] != expected[i]) {
                    windows[delay] = 0;
                    break;
                }
            }
        }
        HAL_EMMC_TRACE(0, "windows[%d] = %d", delay, windows[delay]);

        /*
        *   Q: need to reset cmd and data even pass the tuning cmd to avoid cmd inhibit
        */
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1);
    }
    HAL_EMMC_TRACE(0, "  ");

    /*Cal the optimal setting*/
    for (i = 0; i < TUNING_DELAY_MAX; i += step) {
        if (found == 0 && windows[i] == 1) {
            cur = i;
            found = 1;
        }

        if (found) {
            if (windows[i]) {
                if (cur != i) {
                    windows[cur] += windows[i];
                }

                if (i >= (TUNING_DELAY_MAX - step)) {
                    if (windows[cur] > windows[0]) {
                        windows[0] = windows[cur];  //save count
                        curs = cur;                 //save index
                    }

                    HAL_EMMC_TRACE(0, "windows index, current = %d, final = %d", cur, curs);
                    HAL_EMMC_TRACE(0, "windows width, current = %d, final = %d", windows[cur], windows[0]);
                }
            } else {
                if (windows[cur] > windows[0]) {
                    windows[0] = windows[cur];  //save count
                    curs = cur;                 //save index
                }

                HAL_EMMC_TRACE(0, "windows index, current = %d, final = %d", cur, curs);
                HAL_EMMC_TRACE(0, "windows width, current = %d, final = %d", windows[cur], windows[0]);

                found = 0;//finad the next windows
            }
        }
    }
    HAL_EMMC_ERROR(0, "  ");

    if (windows[0]) {
        windows[0] = (windows[0] - 1) * step + 1;
        cur_mid = curs + windows[0] / 2;
        HAL_EMMC_ERROR(0, "window [%d-%d], width:%d", curs, curs + windows[0] - 1, windows[0]);
    } else {
        cur_mid = 32;
        HAL_EMMC_ERROR(0, "window [0-0], width:0, sampling point is set to 32");
    }

    /*tunning done, set the opt delay value*/
    HAL_EMMC_ERROR(0, "phy tunning done, sampling point = [%d]", cur_mid);
    sdhci_host_emphy_rd_tune_dly_cali(host, tuning_val_cmd, cur_mid);

    if (window_width) {
        *window_width = windows[0];
    }
    if (tuning_val) {
        *tuning_val = cur_mid;
    }

#ifdef EMMC_USE_HEAP
    emmc_heap_free((void *)rcvd);
    emmc_heap_free((void *)windows);
#endif

    host->tuning_status = 0;
    return ret;
}

int sdhci_tuning_cmdline(struct mmc_host *host)
{
#if defined(CHIP_BEST1600)
    return 0;
#else
    return sdhci_tuning_cmdline_raw(host, 2, 61, NULL);
#endif
}

int sdhci_try_tuning(struct mmc_host *host, unsigned int cmd)
{
    int ret = MMC_NO_ERR;

    /*We do not do tuning here if we run shmoo tool to generate xml log*/
    if (!(host->quirks & MMC_QUIRKS_SHMOO_XML)) {
        if (host->quirks & MMC_QUIRKS_SW_TUNING) {
            sdhci_exe_software_tuning(host, MMC_SEND_TUNING_BLOCK);//BES hardware is not supported temporarily
        } else if (host->quirks & MMC_QUIRKS_HW_TUNING) {
            POSSIBLY_UNUSED uint32_t time_base;
            POSSIBLY_UNUSED uint32_t time[2];
            int8_t diff1, diff2;

            #define DISPLAY_TUNING_TIME
            #define TUNING_STEP                     2
            #define TUNING_STEP_CMD                 2
            #define TUNING_STEP_DATA                2
            #define TUNING_WINDOW_THRESHOLD         61  //max 63[0 - 63]
            #define TUNING_WINDOW_THRESHOLD_CMD     61  //max 63[0 - 63]
            #define TUNING_WINDOW_THRESHOLD_DATA    61  //max 63[0 - 63]

#if defined(CHIP_BEST1600)
            uint8_t width, width_rev;
            uint8_t val, val_rev;

#ifdef DISPLAY_TUNING_TIME
            time_base = hal_fast_sys_timer_get();
#endif
            sdhci_exe_phy_tuning(host, cmd, &width, &val, TUNING_STEP, 0);
            HAL_EMMC_TRACE(0, "window width: %d", width);
            if (width < TUNING_WINDOW_THRESHOLD) {//Force a negative edge tuning
                sdhci_host_emphy_pck_reverse(host, 0, 0);
                sdhci_exe_phy_tuning(host, cmd, &width_rev, &val_rev, TUNING_STEP, 0);
                HAL_EMMC_TRACE(0, "window width: %d", width_rev);
                if (width > width_rev) {
                    sdhci_host_emphy_pck_reverse(host, 0, 0);
                    sdhci_host_emphy_rd_tune_dly_cali(host, 0, val);//The second parameter is invalid
                    HAL_EMMC_ERROR(0, "final window width: %d", width);
                } else if (width == width_rev) {
                    //Select the window closest to the midpoint
                    if (val >= TUNING_DELAY_MID) {
                        diff1 = val - TUNING_DELAY_MID;
                    } else {
                        diff1 = TUNING_DELAY_MID - val;
                    }
                    if (val_rev >= TUNING_DELAY_MID) {
                        diff2 = val_rev - TUNING_DELAY_MID;
                    } else {
                        diff2 = TUNING_DELAY_MID - val_rev;
                    }
                    //Restore to the sampling point before reverse
                    if (diff1 < diff2) {
                        sdhci_host_emphy_pck_reverse(host, 0, 0);
                        sdhci_host_emphy_rd_tune_dly_cali(host, 0, val);//The second parameter is invalid
                        HAL_EMMC_ERROR(0, "final window width: %d", width);
                    }
                }  else {
                    HAL_EMMC_ERROR(0, "final window width: %d", width_rev);
                }
            } else {
                HAL_EMMC_ERROR(0, "final window width: %d", width);
            }
#ifdef DISPLAY_TUNING_TIME
            time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);
            HAL_EMMC_ERROR(0, "======tuning time(includ log time)======");
            HAL_EMMC_ERROR(0, "===tuning time: %uus", time[0]);
            HAL_EMMC_ERROR(0, "========================================");
#endif

#else   /* defined(CHIP_BEST1600) */
            uint8_t val_cmd_final;
            uint8_t width_data, width_data_rev, width_data_final;
            uint8_t val_data, val_data_rev, val_data_final;

            //read data delay
#ifdef DISPLAY_TUNING_TIME
            time_base = hal_fast_sys_timer_get();
#endif
            //tuning cmd line
            sdhci_tuning_cmdline_raw(host, TUNING_STEP_CMD, TUNING_WINDOW_THRESHOLD_CMD, &val_cmd_final);

            //tuning data line
#ifdef DISPLAY_TUNING_TIME
            time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);
            time_base = hal_fast_sys_timer_get();
#endif
            sdhci_exe_phy_tuning(host, cmd, &width_data, &val_data, TUNING_STEP_DATA, val_cmd_final);
            HAL_EMMC_TRACE(0, "window width data: %d", width_data);
            if (width_data < TUNING_WINDOW_THRESHOLD_DATA) {//Force a negative edge tuning
                sdhci_host_emphy_pck_reverse(host, 0, 1);//reverse data line
                sdhci_exe_phy_tuning(host, cmd, &width_data_rev, &val_data_rev, TUNING_STEP_DATA, val_cmd_final);
                HAL_EMMC_TRACE(0, "window width data: %d", width_data_rev);
                if (width_data > width_data_rev) {
                    sdhci_host_emphy_pck_reverse(host, 0, 1);//reverse data line
                    sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd_final, val_data);
                    val_data_final = val_data;
                    width_data_final = width_data;
                } else if (width_data == width_data_rev) {
                    //Select the window closest to the midpoint
                    if (val_data >= TUNING_DELAY_MID) {
                        diff1 = val_data - TUNING_DELAY_MID;
                    } else {
                        diff1 = TUNING_DELAY_MID - val_data;
                    }
                    if (val_data_rev >= TUNING_DELAY_MID) {
                        diff2 = val_data_rev - TUNING_DELAY_MID;
                    } else {
                        diff2 = TUNING_DELAY_MID - val_data_rev;
                    }
                    //Restore to the sampling point before reverse
                    if (diff1 < diff2) {
                        sdhci_host_emphy_pck_reverse(host, 0, 1);//reverse data line
                        sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd_final, val_data);
                        val_data_final = val_data;
                        width_data_final = width_data;
                    } else {
                        val_data_final = val_data_rev;
                        width_data_final = width_data_rev;
                    }
                }  else {
                    val_data_final = val_data_rev;
                    width_data_final = width_data_rev;
                }
            } else {
                val_data_final = val_data;
                width_data_final = width_data;
            }
            HAL_EMMC_ERROR(0, "final data window width: %d", width_data_final);
            if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
                width_data_final = width_data_final / 4;
                if (val_data_final > width_data_final) {
                    val_data_final = val_data_final - width_data_final;//Shift left 1/2
                    HAL_EMMC_ERROR(0, "At HS400, data sampling point: %d", val_data_final);
                    sdhci_host_emphy_rd_tune_dly_cali(host, val_cmd_final, val_data_final);
                } else {
                    HAL_EMMC_ERROR(0, "HS400 data window is too small to move left 1/2");
                }
            }
            HAL_EMMC_ERROR(0, "final data sampling point: %d", val_data_final);
#ifdef DISPLAY_TUNING_TIME
            time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_base);
            HAL_EMMC_ERROR(0, "======tuning time(includ log time)======");
            HAL_EMMC_ERROR(0, "===tuning cmd : %uus", time[0]);
            HAL_EMMC_ERROR(0, "===tuning data: %uus", time[1]);
            HAL_EMMC_ERROR(0, "===total time : %uus", time[0] + time[1]);
            HAL_EMMC_ERROR(0, "========================================");
#endif
#endif
            HAL_EMMC_ERROR(0, "  ");
        } else if (host->quirks & MMC_QUIRKS_FIXED_DELAY) {
            /*DLL setting for varies Host IP*/
            if (host->ops && host->ops->tuning_pre) {
                host->ops->tuning_pre(host);
            }
            if (host->ops && host->ops->set_delay) {
                host->ops->set_delay(host, host->rx);
            }
            if (host->ops && host->ops->tuning_post) {
                host->ops->tuning_post(host);
            }
        }
    }

    return ret;
}

void sdhci_reg_dump(struct mmc_host *host)
{
    TRACE(1, "======================================================================");
    TRACE(1, "SDH_CONTROLLER_%d @ BASE_ADDRESS: 0x%08X", host->id, host->base);
    TRACE(1, "SDH_SYSADDR[0x%04X] = 0x%08X", (SDH_SYSADDR), sdhciReadSdh_sysaddrSysaddr(host));
    TRACE(1, "SDH_BLOCK_SIZE[0x%04X] = 0x%04X", (SDH_BLOCK_SIZE), sdhciReadSdh_block_size(host));
    TRACE(1, "SDH_BLOCK_COUNT[0x%04X] = 0x%04X", (SDH_BLOCK_COUNT), sdhciReadSdh_block_count(host));
    TRACE(1, "SDH_ARGUMENT[0x%04X] = 0x%08X", (SDH_ARGUMENT), sdhciReadSdh_arg(host));
    TRACE(1, "SDH_TRANSFER_MODE.[0x%04X] = 0x%04X", (SDH_TRANSFER_MODE), sdhciReadSdh_transfer_mode(host));
    TRACE(1, "SDH_CMD[0x%04X] = 0x%04X", (SDH_CMD), sdhciReadSdh_cmd(host));
    TRACE(1, "SDH_RESP0[0x%04X] = 0x%08X", (SDH_RESP0), sdhciReadSdh_respx(host, 0));
    TRACE(1, "SDH_RESP1[0x%04X] = 0x%08X", (SDH_RESP1), sdhciReadSdh_respx(host, 1));
    TRACE(1, "SDH_RESP2[0x%04X] = 0x%08X", (SDH_RESP2), sdhciReadSdh_respx(host, 2));
    TRACE(1, "SDH_RESP3[0x%04X] = 0x%08X", (SDH_RESP3), sdhciReadSdh_respx(host, 3));
    TRACE(1, "SDH_DP_BUFFER[0x%04X] = 0x%08X", (SDH_DP_BUFFER), sdhciReadSdh_dp_buffer(host));
    TRACE(1, "SDH_PRESENT_STATE_1[0x%04X] = 0x%04X", (SDH_PRESENT_STATE_1), sdhciReadSdh_present_state_1(host));
    TRACE(1, "SDH_PRESENT_STATE_2[0x%04X] = 0x%04X", (SDH_PRESENT_STATE_2), sdhciReadSdh_present_state_2(host));
    TRACE(1, "SDH_HOST_CTRL[0x%04X] = 0x%04X", (SDH_HOST_CTRL), sdhciReadSdh_host_ctrl(host));
    TRACE(1, "SDH_BLK_GAP_CTRL[0x%04X] = 0x%04X", (SDH_BLK_GAP_CTRL), sdhciReadSdh_blk_gap_ctrl(host));
    TRACE(1, "SDH_CLOCK_CTRL[0x%04X] = 0x%04X", (SDH_CLOCK_CTRL), sdhciReadSdh_clock_ctrl(host));
    TRACE(1, "SDH_TIMEOUT_CTRL_SW_RESET[0x%04X] = 0x%04X", (SDH_TIMEOUT_CTRL_SW_RESET), sdhciReadSdh_timeout_ctrl_sw_reset(host));
    TRACE(1, "SDH_NORMAL_STATUS[0x%04X] = 0x%04X", (SDH_NORMAL_STATUS), sdhciReadSdh_i_stat(host));
    TRACE(1, "SDH_ERROR_STATUS[0x%04X] = 0x%04X", (SDH_ERROR_STATUS), sdhciReadSdh_e_stat(host));
    TRACE(1, "SDH_NORMAL_STATUS_EN[0x%04X] = 0x%04X", (SDH_NORMAL_STATUS_EN), sdhciReadSdh_i_stat_en(host));
    TRACE(1, "SDH_ERROR_STATUS_EN[0x%04X] = 0x%04X", (SDH_ERROR_STATUS_EN), sdhciReadSdh_e_stat_en(host));
    TRACE(1, "SDH_NORMAL_STATUS_INT_EN[0x%04X] = 0x%04X", (SDH_NORMAL_STATUS_INT_EN), sdhciReadSdh_i_sign_en(host));
    TRACE(1, "SDH_ERROR_STATUS_INT_EN[0x%04X] = 0x%04X", (SDH_ERROR_STATUS_INT_EN), sdhciReadSdh_e_sign_en(host));
    TRACE(1, "SDH_AUTO_CMD_ERROR_STATUS[0x%04X] = 0x%04X", (SDH_AUTO_CMD_ERROR_STATUS), sdhciReadSdh_auto_cmd12_error_status(host));
    TRACE(1, "SDH_HOST_CTRL_2[0x%04X] = 0x%04X", (SDH_HOST_CTRL_2), sdhciReadSdh_host_ctrl_2(host));
    TRACE(1, "SDH_CAPABILITIES_1[0x%04X] = 0x%04X", (SDH_CAPABILITIES_1), sdhciReadSdh_capabilities_1(host));
    TRACE(1, "SDH_CAPABILITIES_2[0x%04X] = 0x%04X", (SDH_CAPABILITIES_2), sdhciReadSdh_capabilities_2(host));
    TRACE(1, "SDH_CAPABILITIES_3[0x%04X] = 0x%04X", (SDH_CAPABILITIES_3), sdhciReadSdh_capabilities_3(host));
    TRACE(1, "SDH_CAPABILITIES_4[0x%04X] = 0x%04X", (SDH_CAPABILITIES_4), sdhciReadSdh_capabilities_4(host));
    TRACE(1, "SDH_MAX_CURRENT_1[0x%04X] = 0x%04X", (SDH_MAX_CURRENT_1), sdhciReadSdh_max_current_1(host));
    TRACE(1, "SDH_MAX_CURRENT_2[0x%04X] = 0x%04X", (SDH_MAX_CURRENT_2), sdhciReadSdh_max_current_2(host));
    TRACE(1, "SDH_ADMA_ERROR_STATUS[0x%04X] = 0x%04X", (SDH_ADMA_ERROR_STATUS), sdhciReadSdh_adma_error_status(host));
    TRACE(1, "SDH_ADMA_SYSADDR0[0x%04X] = 0x%08X", (SDH_ADMA_SYSADDR0), sdhciReadSdh_adma_sysaddr0(host));
    TRACE(1, "SDH_ADMA_SYSADDR1[0x%04X] = 0x%08X", (SDH_ADMA_SYSADDR1), sdhciReadSdh_adma_sysaddr1(host));
    TRACE(1, "PRESET_VALUE_FOR_INIT[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_INIT), sdhciReadPreset_value_for_init(host));
    TRACE(1, "PRESET_VALUE_FOR_DS[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_DS), sdhciReadPreset_value_for_ds(host));
    TRACE(1, "PRESET_VALUE_FOR_HS[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_HS), sdhciReadPreset_value_for_hs(host));
    TRACE(1, "PRESET_VALUE_FOR_SDR12[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_SDR12), sdhciReadPreset_value_for_sdr12(host));
    TRACE(1, "PRESET_VALUE_FOR_SDR25[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_SDR25), sdhciReadPreset_value_for_sdr25(host));
    TRACE(1, "PRESET_VALUE_FOR_SDR50[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_SDR50), sdhciReadPreset_value_for_sdr50(host));
    TRACE(1, "PRESET_VALUE_FOR_SDR104[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_SDR104), sdhciReadPreset_value_for_sdr104(host));
    TRACE(1, "PRESET_VALUE_FOR_DDR50[0x%04X] = 0x%04X", (PRESET_VALUE_FOR_DDR50), sdhciReadPreset_value_for_ddr50(host));
    TRACE(1, "SHARED_BUS_CTRL[0x%04X] = 0x%08X", (SHARED_BUS_CTRL), sdhciReadShared_bus_ctrl(host));
    TRACE(1, "SDH_SLOT_INT_STATUS[0x%04X] = 0x%04X", (SDH_SLOT_INT_STATUS), sdhciReadSdh_slot_int_status(host));
    TRACE(1, "SDH_CTRL_VER[0x%04X] = 0x%04X", (SDH_CTRL_VER), sdhciReadSdh_ctrl_ver(host));
    if (host->ops && host->ops->verdor_reg_dump) {
        host->ops->verdor_reg_dump(host);
    }
    TRACE(1, "===========================================================================");
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/
int sdhci_set_bus_speed_and_clksrc(struct mmc_host *host, uint8_t bus_speed_mode, enum EMMC_CLK_SRC_SEL_T clk_src_sel)
{
    return MMC_NO_ERR;
}
