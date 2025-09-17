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
#include "host.h"
#include "core.h"
#include "error.h"
#include "sdhci.h"
#include "string.h"
#if defined(CHIP_BEST1600)
#include "emmcphy.h"
#endif
#include "emmc_dbg.h"
#include "card_mmc.h"
#include "sdhci_emphy.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"

enum EMMCPHY_OP_T {
    EMMCPHY_OP_OPEN,
    EMMCPHY_OP_CLOSE,
    EMMCPHY_OP_ENABLE,
    EMMCPHY_OP_DISABLE,
    EMMCPHY_OP_SLEEP,
    EMMCPHY_OP_WAKEUP,
    EMMCPHY_OP_HS_ENABLE,
    EMMCPHY_OP_HS_DISABLE,
    EMMCPHY_OP_HS_DDR_ENABLE,
    EMMCPHY_OP_HS_DDR_DISABLE,

    EMMCPHY_OP_QTY,
};

extern void emmc_test_wait_cmd(const char *s, int mark, bool show);
void emmcphy_op_callback(enum EMMCPHY_OP_T op, void *param, bool before)
{
#if defined(EMMC_TEST) && defined(EMMC_TEST_WAIT)
    if (before) {
        //TODO: CHECK state
    } else {
        switch (op) {
            case EMMCPHY_OP_OPEN:
                emmc_test_wait_cmd("emmc phy is open, before init device", op, true);
                break;
            case EMMCPHY_OP_CLOSE:
                emmc_test_wait_cmd("emmc phy is close, before init device", op, true);
                break;
            case EMMCPHY_OP_ENABLE:
                emmc_test_wait_cmd("emmc phy is ready, before init device", op, true);
                break;
            case EMMCPHY_OP_DISABLE:
                emmc_test_wait_cmd("emmc phy is disable, before init device", op, true);
                break;
            case EMMCPHY_OP_SLEEP:
                emmc_test_wait_cmd("emmc phy is sleep, before init device", op, true);
                break;
            case EMMCPHY_OP_WAKEUP:
                emmc_test_wait_cmd("emmc phy is wakeup, before init device", op, true);
                break;
            case EMMCPHY_OP_HS_ENABLE:
                emmc_test_wait_cmd("emmc hs mode is ready, before init device", op, true);
                break;
            case EMMCPHY_OP_HS_DISABLE:
                emmc_test_wait_cmd("emmc hs mode is disable, before init device", op, true);
                break;
            case EMMCPHY_OP_HS_DDR_ENABLE:
                emmc_test_wait_cmd("emmc hs ddr mode is enable, before init device", op, true);
                break;
            case EMMCPHY_OP_HS_DDR_DISABLE:
                emmc_test_wait_cmd("emmc hs ddr mode is disenable, before init device", op, true);
                break;
            default:
                break;
        }
    }
#endif
}

//Only power up the PHY and PHY will not work at this time
int sdhci_host_emphy_early_open(struct mmc_host *host)
{
    emmcphy_pin_pull_clear_all();   //Remove all pull-up and pull-down resistors
    emmcphy_open(host);             //PHY is powered on
    emmcphy_start(false);           //Close EN so that the EMMC pin is in a high-impedance state
    return 0;
}

int sdhci_host_emphy_open(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_OPEN, (void *)host, true);
    emmcphy_open(host);
    emmcphy_op_callback(EMMCPHY_OP_OPEN, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_close(struct mmc_host *host)
{
    emmcphy_close();
    return 0;
}

int sdhci_host_emphy_sleep(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_SLEEP, (void *)host, true);
    emmcphy_sleep();
    emmcphy_op_callback(EMMCPHY_OP_SLEEP, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_wakeup(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_WAKEUP, (void *)host, true);
    emmcphy_wakeup();
    emmcphy_op_callback(EMMCPHY_OP_WAKEUP, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_pull_config(struct mmc_host *host)
{
    emmcphy_pin_pull_config();
    return 0;
}

int sdhci_host_emphy_pull_clear_all(struct mmc_host *host)
{
    emmcphy_pin_pull_clear_all();
    return 0;
}

int sdhci_host_emphy_set_io_drv(struct mmc_host *host)
{
    emmcphy_pin_set_io_drv();
    return 0;
}

int sdhci_host_emphy_high_speed_sdr_enable(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_HS_ENABLE, (void *)host, true);
    emmcphy_hs_host_mode_enable();
    emmcphy_op_callback(EMMCPHY_OP_HS_ENABLE, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_high_speed_sdr_disable(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_HS_DISABLE, (void *)host, true);
    emmcphy_hs_host_mode_disable();
    emmcphy_op_callback(EMMCPHY_OP_HS_DISABLE, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_high_speed_ddr_enable(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_HS_DDR_ENABLE, (void *)host, true);
    emmcphy_ddr_mode_enable();
    emmcphy_op_callback(EMMCPHY_OP_HS_DDR_ENABLE, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_high_speed_ddr_disable(struct mmc_host *host)
{
    emmcphy_op_callback(EMMCPHY_OP_HS_DDR_DISABLE, (void *)host, true);
    emmcphy_hs_host_mode_disable();
    emmcphy_ddr_mode_disable();
    emmcphy_op_callback(EMMCPHY_OP_HS_DDR_DISABLE, (void *)host, false);
    return 0;
}

int sdhci_host_emphy_pck_reverse(struct mmc_host *host, int rev_cmd_en, int rev_data_en)
{
    emmcphy_pck_reverse(rev_cmd_en, rev_data_en);
    return 0;
}

int sdhci_host_emphy_rd_tune_dly_cali(struct mmc_host *host, uint8_t cali_val_cmd, uint8_t cali_val_data)
{
    emmcphy_rd_tune_dly_cali(cali_val_cmd, cali_val_data);
    return 0;
}

int sdhci_host_emphy_rd_tune_dly_read(struct mmc_host *host, uint8_t *cmd_dly, uint8_t *data_dly)
{
    emmcphy_rd_tune_dly_read(cmd_dly, data_dly);
    return 0;
}

int sdhci_host_emphy_tuning_correct(struct mmc_host *host)
{
    emmcphy_tuning_correct();
    return 0;
}

int sdhci_host_emphy_clk_sel_pck(struct mmc_host *host)
{
    emmcphy_clk_sel_pck();
    return 0;
}

int sdhci_host_emphy_clk_sel_pck_delay(struct mmc_host *host)
{
    emmcphy_clk_sel_pck_delay();
    return 0;
}

int sdhci_host_emphy_clk_sel_ds(struct mmc_host *host)
{
    emmcphy_clk_sel_ds_delay();
    return 0;
}

int sdhci_host_emphy_init(bool init, struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int data = 0;
    int timeout = 0;

    HAL_EMMC_TRACE(1, "%s: init=%d", __FUNCTION__, init);

    if (init) {
        /*Software reset all */
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_all(host, 1);
        hal_sys_timer_delay_us(6);

        /*set timeout value*/
        sdhciWriteSdh_timeout_ctrl_sw_resetTimeout_value(host, 0xE);

        /*set bus voltage 3.3v firstly*/
        sdhciWriteSdh_host_ctrlBus_volt(host, 0x7);
        sdhciWriteSdh_host_ctrlBus_power(host, 1);
        hal_sys_timer_delay_us(10);

        /* open emmc phy */
        sdhci_host_emphy_open(host);
        sdhci_host_emphy_pull_config(host);
        sdhci_host_emphy_set_io_drv(host);

        if ((host->speed_mode == DDR26) || (host->speed_mode == DDR50) || (host->speed_mode == DDR52) || \
            (host->speed_mode == HS400) || (host->speed_mode == EHS400)) {
            sdhci_host_emphy_high_speed_ddr_enable(host);
        } else if ((host->speed_mode == SDR25) || (host->speed_mode == SDR50) || (host->speed_mode == SDR52) || \
                   (host->speed_mode == SDR104) || (host->speed_mode == HS25) || (host->speed_mode == HS50) || \
                   (host->speed_mode == HS100) || (host->speed_mode == HS200)) {
            sdhci_host_emphy_high_speed_sdr_enable(host);
        } else {
            //ds mode, use default values
        }

        /*Enable internal and bus clock*/
        ret = sdhci_start_int_clk(host);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(1, "%s: set bus clk failed %d", __FUNCTION__, ret);
            return ret;
        }
        sdhci_set_bus_clk(host, F_OD);

        /*reset emmc card*/
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data &= ~(0x1 << 11);
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);
        hal_sys_timer_delay_us(2);

        /*emmc mode, reset*/
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data |= (0x1 << 11) | (0x1 << 12);
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);

        /*set 1.8v for emmc*/
        sdhciWriteSdh_host_ctrl_2Sdh_v18_en(host, 1);

        /*provide first 74 clocks before any cmd after power up*/
        /*clear and enable misc_int first*/
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data |= (0x1 << 1) | (0x1 << 2);
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);

        /*generate 74 clocks*/
        data = sdhciReadSdhc_legacy_ctrl_reg(host);
        data &= ~(0xff << 24);
        data |= (0x4A << 24) | (0x1 << 6);
        sdhciWriteSdhc_legacy_ctrl_reg(host, data);

        /*wait 74 clocks done*/
        timeout = 5000;
        do {
            data = sdhciReadSdhc_mmc_ctrl_reg(host);
            hal_sys_timer_delay_us(1);
        } while (((data & 0x4) == 0) && (timeout--));
        if (timeout < 0) {
            HAL_EMMC_ERROR(0, "%s: wait clk ready failed, data:0x%X", __FUNCTION__, data);
            return MMC_INT_CLK_ERR;
        }

        /*clear misc int*/
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data |= (0x1 << 2);
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);

        /*disable misc_int*/
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data &= ~(0x1 << 1);
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);

        if (!host->internal_gating_en) {
            //Close the clk gating of internal cmd and data
            sdhci_close_internal_cmd_gate(host);
        }

        host->ios.timing = DS;
        host->ios.khz = F_OD;
        host->ios.drv = MMC_DRV_50OHM_X1;
        host->ios.voltage = MMC_IO_VOLTAGE_1P8;
        host->ios.width = MMC_1BIT_WIDTH;
    } else {
        sdhci_host_emphy_sleep(host);
    }
    return ret;
}

int sdhci_host_emphy_calib(struct mmc_host *host)
{
    return 0;
}
