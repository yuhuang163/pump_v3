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
#ifndef __MMC_CORE_H__
#define __MMC_CORE_H__

#include "host.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
*   Command Defintion
*/
#define GO_IDLE_STATE           0x00    // CMD0
#define GO_PRE_IDLE_STATE       0x00    // CMD0
#define BOOT_INITIATION         0x00    // CMD0
#define SEND_OP_COND            0x01    // CMD1     (MMC only)
#define ALL_SEND_CID            0x02    // CMD2
#define SET_RELATIVE_ADDR       0x03    // CMD3     (MMC only)
#define SEND_RELATIVE_ADDR      0x03    // CMD3     (SD only)
#define SET_DSR                 0x04    // CMD4
#define IO_SEND_OP_COND         0x05    // CMD5     (SDIO only)
#define SLEEP                   0x05    // CMD5     (eMMC only)
#define AWAKE                   0x05    // CMD5     (eMMC only)
#define SWITCH                  0x06    // CMD6     (MMC only)
#define SWITCH_FUNC             0x06    // CMD6     (SD only)
#define SELECT_CARD             0x07    // CMD7
#define DESELECT_CARD           0x07    // CMD7
#define SEND_EXT_CSD            0x08    // CMD8     (MMC only)
#define SEND_IF_COND            0x08    // CMD8     (SD/SDIO only)
#define SEND_CSD                0x09    // CMD9
#define SEND_CID                0x0A    // CMD10
#define READ_DAT_UNTIL_STOP     0x0B    // CMD11    (MMC only)
#define VOLTAGE_SWITCH          0x0B    // CMD11    (SD only)
#define STOP_TRANSMISSION       0x0C    // CMD12
#define SEND_STATUS             0x0D    // CMD13
#define BUSTEST_R               0x0E    // CMD14    (MMC only)
#define GO_INACTIVE_STATE       0x0F    // CMD15
#define SET_BLOCKLEN            0x10    // CMD16
#define READ_SINGLE_BLOCK       0x11    // CMD17
#define READ_MULTIPLE_BLOCK     0x12    // CMD18
#define BUSTEST_W               0x13    // CMD19    (MMC only)
#define SEND_TUNING_BLOCK       0x13    // CMD19    (SD only)
#define WRITE_DAT_UNTIL_STOP    0x14    // CMD20    (MMC only)
#define MMC_SEND_TUNING_BLOCK   0x15    // CMD21    (MMC only)
#define SPEED_CLASS_CONTROL     0x14    // CMD20    (SD only)
#define SET_BLOCK_COUNT         0x17    // CMD23
#define WRITE_SINGLE_BLOCK      0x18    // CMD24
#define WRITE_MULTIPLE_BLOCK    0x19    // CMD25
#define PROGRAM_CID             0x1A    // CMD26    (MMC only)
#define PROGRAM_CSD             0x1B    // CMD27
#define SET_WRITE_PROT          0x1C    // CMD28
#define CLR_WRITE_PROT          0x1D    // CMD29
#define SEND_WRITE_PROT         0x1E    // CMD30
#define ERASE_WR_BLK_START      0x20    // CMD32    (SD only)
#define ERASE_WR_BLK_END        0x21    // CMD33    (SD only)
#define ERASE_GROUP_START       0x23    // CMD35    (MMC only)
#define ERASE_GROUP_END         0x24    // CMD36    (MMC only)
#define ERASE                   0x26    // CMD38
#define FAST_IO                 0x27    // CMD39    (MMC only)
#define GO_IRQ_STATE            0x28    // CMD40
#define LOCK_UNLOCK             0x2A    // CMD42
#define IO_RW_DIRECT            0x34    // CMD52    (SDIO only)
#define IO_RW_EXTENDED          0x35    // CMD53    (SDIO only)
#define APP_CMD                 0x37    // CMD55
#define GEN_CMD                 0x38    // CMD56
#define CMD60                   0x3C    // CMD60
#define SET_BUS_WIDTH           0x06    // ACMD6
#define SD_STATUS               0x0D    // ACMD13
#define SEND_NUM_WR_BLOCKS      0x16    // ACMD22
#define SET_WR_BLK_ERASE_COUNT  0x17    // ACMD23
#define SD_SEND_OP_COND         0x29    // AMCD41
#define SET_CLR_CARD_DETECT     0x2A    // AMCD42
#define SEND_SCR                0x33    // ACMD51

/*
*   struct mmc_cmd
*/
struct mmc_cmd {
    unsigned int opcode;
    unsigned int arg;
    unsigned int flags;
#define MMC_RSP_PRESENT         (1 << 0)
#define MMC_RSP_136             (1 << 1)        /* 136 bit response */
#define MMC_RSP_CRC             (1 << 2)        /* expect valid crc */
#define MMC_RSP_BUSY            (1 << 3)        /* card may send busy */
#define MMC_RSP_OPCODE          (1 << 4)        /* response contains opcode */
#define MMC_CMD_NC              (1 << 5)        /* normal command*/
#define MMC_CMD_SC              (1 << 6)        /* suspend command*/
#define MMC_CMD_RC              (1 << 7)        /* resume command*/
#define MMC_CMD_AC              (1 << 8)        /* abort command*/
#define MMC_DATA_PRESENT        (1 << 9)
#define MMC_DATA_DMA            (1 << 10)
#define MMC_DATA_BC_EN          (1 << 11)
#define MMC_DATA_AUTOCMD_DIS    (1 << 12)
#define MMC_DATA_AUTOCMD12      (1 << 13)
#define MMC_DATA_AUTOCMD23      (1 << 14)
#define MMC_DATA_WRITE          (1 << 15)
#define MMC_DATA_READ           (1 << 16)
#define MMC_DATA_MB             (1 << 17)

    unsigned int resp[4];
    volatile unsigned int complete;
    volatile unsigned int error;

#define MMC_RSP_NONE    (0)
#define MMC_RSP_R1      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B     (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2      (MMC_RSP_PRESENT|MMC_RSP_136)
#define MMC_RSP_R3      (MMC_RSP_PRESENT)
#define MMC_RSP_R4      (MMC_RSP_PRESENT)
#define MMC_RSP_R5      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

};

/*
*   struct mmc_data
*/
struct mmc_data {
    unsigned int direction;

#define MMC_CARD_WRITE      0
#define MMC_CARD_READ       1

    unsigned int blksz;
    unsigned int blks;
    unsigned int sz;

    unsigned int xferd_bytes;

    char *buf;  /*aligned buffer*/

    volatile unsigned int complete;
    volatile unsigned int error;
};

//Used to wait for the data transmission to complete, it has no direct relationship with DMA
enum BLOCKING_MODE {
    BLOCKING_MODE_NONE,      //irq + dma + non-blocking mode
    BLOCKING_MODE_IRQ_ON,    //irq + dma/no dma + blocking mode
    BLOCKING_MODE_IRQ_OFF    //polling + dma/no dma + blocking mode
};

void mmc_unpack_cid(struct mmc_host *host, unsigned int *resp);
void mmc_unpack_csd(struct mmc_host *host, unsigned int *resp);
void mmc_prepare_transfer(struct mmc_host *host, void *buf, unsigned int dir, unsigned int blksz, unsigned int sz, bool dma_en);
void mmc_card_irq(struct mmc_host *host, unsigned int state);
void mmc_irq(void *private);
int mmc_send_cmmand(struct mmc_host *host, unsigned int opcode, unsigned int arg, unsigned int flags);
int mmc_wait_for_resp(struct mmc_host *host, unsigned int resp_type);
int mmc_wait_for_xfer(struct mmc_host *host, unsigned int timeout_ms, enum BLOCKING_MODE blocking_mode);

//adma2
int mmc_dma_mode_write(struct mmc_host *host, uint8_t new_dma_mode);
int mmc_dma_mode_read(struct mmc_host *host, uint8_t *dma_mode);
int mmc_desc_type_write(struct mmc_host *host, uint8_t new_desc_type);
int mmc_desc_type_read(struct mmc_host *host, uint8_t *desc_type);
int mmc_adma_init_desc(struct mmc_host *host, const void *src_dst, uint32_t size, const void *next_src_dst);

#include "core_if.h"

#ifdef __cplusplus
}
#endif

#endif
