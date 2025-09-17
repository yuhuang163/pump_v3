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
#ifndef __MMC_CARD_SDIO_H__
#define __MMC_CARD_SDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

enum SDIO_FUNC_NUM_T {
    SDIO_FUNC_0 = 0,
    SDIO_FUNC_1,
    SDIO_FUNC_2,
    SDIO_FUNC_3,
    SDIO_FUNC_4,
    SDIO_FUNC_5,
    SDIO_FUNC_6,
    SDIO_FUNC_7,
    SDIO_FUNC_NUM,
};

enum SDIO_DEVICE_RW {
    SDIO_DEVICE_READ = 0,
    SDIO_DEVICE_WRITE,
};

int mmc_init_sdio_card(struct mmc_host *host);
int mmc_config_sdio_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width);
int sdio_io_rw_extended(struct mmc_host *host, enum SDIO_DEVICE_RW read0_write1,enum SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t *rsp, uint8_t incr_addr, uint8_t *data_buff, uint32_t data_size, enum BLOCKING_MODE blocking_mode, bool dma_en);
int sdio_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);
int sdio_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);

/* get device information*/
int sdio_get_block_size(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num, uint16_t *block_size);
int sdio_get_bus_speed(struct mmc_host *host, uint8_t *bus_speed);
int sdio_get_capability(struct mmc_host *host, uint8_t *cap);

/* Special purpose use */
int sdio_host_gen_int_to_device(struct mmc_host *host);
int sdio_host_get_func1_rx_buf_cfg(struct mmc_host *host, uint8_t *buf_cnt, uint16_t *buf_len);
void sdio_host_func_int(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num);

//sdio interrupt
int sdio_get_int_pending(struct mmc_host *host, uint8_t *int_pending);
int sdio_set_func_abort(struct mmc_host *host, enum SDIO_FUNC_NUM_T func_num);
#ifdef __cplusplus
}
#endif

#endif
