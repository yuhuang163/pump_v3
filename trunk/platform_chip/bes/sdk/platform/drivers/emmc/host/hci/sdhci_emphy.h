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
#ifndef __EMMC_SDHCI_EMPHY_H__
#define __EMMC_SDHCI_EMPHY_H__

#ifdef __cplusplus
extern "C" {
#endif

int sdhci_host_emphy_early_open(struct mmc_host *host);
int sdhci_host_emphy_open(struct mmc_host *host);
int sdhci_host_emphy_close(struct mmc_host *host);
int sdhci_host_emphy_sleep(struct mmc_host *host);
int sdhci_host_emphy_wakeup(struct mmc_host *host);
int sdhci_host_emphy_pull_config(struct mmc_host *host);
int sdhci_host_emphy_pull_clear_all(struct mmc_host *host);
int sdhci_host_emphy_set_io_drv(struct mmc_host *host);
int sdhci_host_emphy_high_speed_sdr_enable(struct mmc_host *host);
int sdhci_host_emphy_high_speed_sdr_disable(struct mmc_host *host);
int sdhci_host_emphy_high_speed_ddr_enable(struct mmc_host *host);
int sdhci_host_emphy_high_speed_ddr_disable(struct mmc_host *host);
int sdhci_host_emphy_pck_reverse(struct mmc_host *host, int rev_cmd_en, int rev_data_en);
int sdhci_host_emphy_rd_tune_dly_cali(struct mmc_host *host, uint8_t cali_val_cmd, uint8_t cali_val_data);
int sdhci_host_emphy_rd_tune_dly_read(struct mmc_host *host, uint8_t *cmd_dly, uint8_t *data_dly);
int sdhci_host_emphy_tuning_correct(struct mmc_host *host);
int sdhci_host_emphy_clk_sel_pck(struct mmc_host *host);
int sdhci_host_emphy_clk_sel_pck_delay(struct mmc_host *host);
int sdhci_host_emphy_clk_sel_ds(struct mmc_host *host);
int sdhci_host_emphy_calib(struct mmc_host *host);
int sdhci_host_emphy_init(bool init, struct mmc_host *host);

#ifdef __cplusplus
}
#endif

#endif
