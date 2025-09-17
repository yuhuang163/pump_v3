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
#ifndef __MMC_CARD_SD_H__
#define __MMC_CARD_SD_H__

#ifdef __cplusplus
extern "C" {
#endif

int mmc_init_sd_card(struct mmc_host *host);
int mmc_config_sd_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width);
int sd_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);
int sd_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);

#ifdef __cplusplus
}
#endif

#endif
