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
#ifndef __EMMC_SDHCI_H__
#define __EMMC_SDHCI_H__

#ifdef __cplusplus
extern "C" {
#endif

struct mmc_host;

typedef struct {
    unsigned int version;
    int (*init)(struct mmc_host *);
} sdhci_host_t, *p_sdhci_host_t;

struct emmc_timing_shmoo {
    char *name;
    unsigned int p1;
    unsigned int p2;
    unsigned int p3;
    unsigned int p4;
    unsigned int p5;
    char *desc;
    unsigned int *rst;
};

#define SDHCI_REG_READ_WRITE_TEST                   1
#define SDHCI_DATA_XFER_TEST                        3
#define SDHCI_STRESS_TEST                           4
#define SDHCI_EPHY_LOOPBACK_TEST                    11
#define SDHCI_EMMC_BOOT_TEST                        12
#define SDHCI_EMMC_ASYNC_SHMOO_TEST                 13
#define SDHCI_EMMC_TUNING_SHMOO_TEST                14
#define SDHCI_EMMC_APPS_SHMOO_TEST                  15
#define SDHCI_EMMC_DLLM_SHMOO_TEST                  16
#define SDHCI_EMMC_PHY_TIMING_ADJ_SHMOO_TEST        17
#define SDHIC_SIV_TEST                              18
#define SDHIC_EMMC_VOLTAGE_FOCUS_SHMOO_TEST         19
#define SDHCI_EMMC_PHY_DLLM_BYPASS_SHMOO_TEST       20
#define SDHIC_EMMC_DLLMBYPASS_FOCUS_SHMOO_TEST      21
#define SDHCI_EMMC_DVC_TEST                         22
#define SDHCI_PERFORMANCE_TEST                      23
#define SDHIC_EMMC_DLLMBYPASS_FOCUS_SHMOO_VLO_TEST  24

/* The paltform init/deinit for EMMC host */
int sdhci_platform_init(bool init, struct mmc_host *host);

/* The EMMC host init/deinit */
int sdhci_host_init(bool init, struct mmc_host *host);

int sdhci_host_all_init(struct mmc_host *host, bool enable);//include platform and host init
int sdhci_int_init(struct mmc_host *host, bool enable);

#ifdef __cplusplus
}
#endif

#endif
