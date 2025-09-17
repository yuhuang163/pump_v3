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
#ifndef __MMC_CARD_MMC_H__
#define __MMC_CARD_MMC_H__

#ifdef __cplusplus
extern "C" {
#endif

struct emmc_boot_test_args {
    char *name;
    unsigned int type;
#define EMMC_DEFAULT_BOOT           0
#define EMMC_ALTERNATIVE_BOOT       1
    unsigned int boot_mode;
#define EMMC_BOOT_DS                0
#define EMMC_BOOT_HS                1
#define EMMC_BOOT_DDR               2
    unsigned int boot_condition;
#define EMMC_BOOT_BY_DEFAULT        0
#define EMMC_BOOT_RETAIN            1
    unsigned int boot_wdith;
#define EMMC_BOOT_1BIT              0
#define EMMC_BOOT_4BIT              1
#define EMMC_BOOT_8BIT              2
    unsigned int boot_ack;
#define EMMC_BOOT_ACK_YES           1
#define EMMC_BOOT_ACK_NO            0
    unsigned int parti;
#define EMMC_BOOT_PARTI_1           1
#define EMMC_BOOT_PARTI_2           2
#define EMMC_BOOT_USR_PARTI         3
    unsigned int card_offset;
    uintptr_t dst;
};

/*
*   EXT_CSD fileds to config timing
*/
#define MMC_HS400_VCCIO_1P2V        0x80
#define MMC_HS400_VCCIO_1P8V        0x40
#define MMC_HS200_VCCIO_1P2V        0x20
#define MMC_HS200_VCCIO_1P8V        0x10
#define MMC_DDR50_VCCIO_1P2V        0x08
#define MMC_DDR50_VCCIO_1P8V_3P3V   0x04
#define MMC_HS52_VCCIO              0x02
#define MMC_HS26_VCCIO              0x01

#define MMC_DRV_50OHM_X1            0x01    /*Default Driver Type, supports ups to 200MHZ*/
#define MMC_DRV_33OHM_X1P5          0x02    /*Supports ups to 200MHZ*/
#define MMC_DRV_66OHM_X0P75         0x04    /*The weakest Driver that supports to 200MHZ*/
#define MMC_DRV_100OHM_X0P5         0x08    /*For low noise and low EMI systems. Maximal operating frequency is decided by Host design.*/
#define MMC_DRV_40OHM_X1P2          0x10    /*HS400 only*/

enum EMMC_RST_N_MODE {
    EMMC_RST_N_TEMP_DISABLE = 0,    //RST_n signal is temporarily disabled (default)
    EMMC_RST_N_PERM_ENABLE,         //RST_n signal is permanently enabled
    EMMC_RST_N_PERM_DISABLE         //RST_n signal is permanently disabled
};

enum EMMC_POWER_OFF_MODE {
    EMMC_POWER_OFF_SHORT    = 2,    //POWER_OFF_SHORT
    EMMC_POWER_OFF_LONG     = 3,    //POWER_OFF_LONG
    EMMC_SLEEP_NOTIFICATION = 4     //SLEEP_NOTIFICATION
};

int mmc_rst_n_mode_set(struct mmc_host *host, enum EMMC_RST_N_MODE mode);
int mmc_power_off_notification_enable(struct mmc_host *host);
int mmc_power_off_notification_config(struct mmc_host *host, enum EMMC_POWER_OFF_MODE mode);
int mmc_send_cmd_wait_resp(struct mmc_host *host, unsigned int opcode, unsigned int arg, unsigned int flags,
                           unsigned int resp_type, bool send_stop, unsigned int state);
int mmc_card_set_blocking_mode(struct mmc_host *host, enum BLOCKING_MODE blocking_mode);
int mmc_set_tran_mode(struct mmc_host *host, enum BLOCKING_MODE blocking_mode);
int mmc_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);
int mmc_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd);
unsigned int mmc_card_status(struct mmc_host *host);        //current state
unsigned int mmc_card_raw_status(struct mmc_host *host);    //raw status
unsigned int mmc_card_sleep_status(struct mmc_host *host);  //sleep status
int mmc_check_card_status(struct mmc_host *host, unsigned int status, unsigned char en_delay);
int mmc_send_stop(struct mmc_host *host, enum BLOCKING_MODE blocking_mode);
int mmc_switch(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset);
int mmc_switch_ext(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset, unsigned int timeout_ms);
void mmc_print_ext_csd(struct mmc_host *host);
int mmc_init_mmc_card(struct mmc_host *host);
int mmc_config_mmc_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width);
int mmc_bus_test(struct mmc_host *host, unsigned int width);
int mmc_alter_boot(struct mmc_host *host);
int mmc_put_to_reset(struct mmc_host *host);
int mmc_boot_config(struct mmc_host *host, struct emmc_boot_test_args *args);
int mmc_switch_to_rpmb(struct mmc_host *host, enum EMMC_PARTITION_ACCESS mmc_parti);
int mmc_rpmb_get_counter(struct mmc_host *host, uint32_t *counter, uint32_t *result);
int mmc_rpmb_write(struct mmc_host *host, uint32_t write_counter, unsigned short card_offset, void *buf, unsigned int sz, const uint8_t *key, uint32_t *result);
int mmc_rpmb_read(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, uint32_t *result);
int mmc_rpmb_set_key(struct mmc_host *host, uint8_t *key, uint32_t *result);
int mmc_select_cards(struct mmc_host *host);
int mmc_deselect_cards(struct mmc_host *host);
int mmc_sleep(struct mmc_host *host);
int mmc_awake(struct mmc_host *host);
int mmc_gen_cmd(struct mmc_host *host, uint32_t arg, uint8_t *buf, uint16_t len, enum BLOCKING_MODE blocking_mode, bool dma_en);
int mmc_cmd60(struct mmc_host *host, uint32_t arg);
int mmc_set_auto_cmd23(struct mmc_host *host, unsigned int blks, bool dma_en);
int mmc_send_ext_csd(struct mmc_host *host);
int mmc_set_blk_cnt(struct mmc_host *host, unsigned int blks);
int mmc_cache_control(struct mmc_host *host, uint8_t enable_disable);//enable_disable : 0 close cache, 1 open cache
int mmc_cache_flush(struct mmc_host *host);

struct mmc_cmd_ext {
    uint16_t cmdidx;
    uint16_t resp_type;
    uint32_t cmdarg;
    uint32_t resp[4];//resp[0]: high, resp[3]: low
};

struct mmc_data_ext {
    union {
        char *dest;
        const char *src;
    };
    uint32_t direction;
    uint32_t blocks;
    uint32_t blocksize;
};

int mmc_send_cmd_ext(struct mmc_host *host, struct mmc_cmd_ext *cmd, struct mmc_data_ext *data, enum BLOCKING_MODE blocking_mode, bool dma_en);
int mmc_config_ios(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width);

#ifdef __cplusplus
}
#endif

#endif
