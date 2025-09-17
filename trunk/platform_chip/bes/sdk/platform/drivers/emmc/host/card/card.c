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
#include "host.h"
#include "core.h"
#include "card.h"
#include "error.h"
#include "string.h"
#include "card_sd.h"
#include "emmc_dbg.h"
#include "card_mmc.h"
#include "card_sdio.h"
#include "hal_location.h"
#include "emmc_platform.h"

#define MAX_TRYCNT 0x8000

SYNC_FLAGS_LOC static struct mmc_card card;
static struct mmc_card_info card_info;

void card_controller_clk_continue(struct mmc_host *host)
{
    if (host == NULL) {
        host->error_code = MMC_ERR_NULL_POINTER;
        return;
    }
    if (host->clk_gating_en) {
        if (host->clk_gating_status) {
            hal_cmu_emmc_clock_continue();
            host->clk_gating_status = 0;
        }
    }
}

void card_controller_clk_pause(struct mmc_host *host)
{
    if (host == NULL) {
        host->error_code = MMC_ERR_NULL_POINTER;
        return;
    }
    if (host->clk_gating_en) {
        if (!host->clk_gating_status) {
            hal_cmu_emmc_clock_pause();
            host->clk_gating_status = 1;
        }
    }
}

int detect_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int arg;
    unsigned int repeats;

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    /*Alloc card structure*/
    host->card = &card;
    memset(host->card, 0, sizeof(struct mmc_card));
    memset(host->card->ext_csd, 0, MMC_EXT_CSD_LENGTH);

    if (host->resp_polling_mode) {
        sdhci_unmask_irqs_ext(host, EMMC_IRQ_CMD_ERR_MASK, IRQ_CMD_COMP);
    } else {
        sdhci_unmask_irqs(host, IRQ_CMD_COMP | EMMC_IRQ_CMD_ERR_MASK);//The IRQ_CMD_COMP interrupt is turned on here
    }

    /* CMD0 memory reset,sdio not need this command,for it used to select SPI Mode */
    if (host->id != DEV_TYPE_SDIO) {
        ret = mmc_send_cmd_wait_resp(host, GO_IDLE_STATE, 0, MMC_RSP_NONE, CARD_RESPONSE_NONE, 0, 0);
        if (ret != MMC_NO_ERR) {
            goto detect_out;
        }
    }

    if (host->id == DEV_TYPE_SD) {
        /*check for high capacity card*/
        repeats = 0;
        do {
            arg = (SDVHS_2_7_TO_3_6 << 8) | SDVHSCHECKPATTERN;

            mmc_send_cmmand(host, SEND_IF_COND, arg, MMC_RSP_R7);
            mmc_wait_for_resp(host, CARD_RESPONSE_R7);
            HAL_EMMC_TRACE(1, "SD_VHS : 0x%08x", host->card->sd_vhs);
            host->card->high_capacity = host->card->sd_vhs == (arg & ~(SDVHS_2_7_TO_3_6 << 8));
        } while (!host->card->high_capacity && ++repeats < 2);
        if (!host->card->high_capacity) {
            HAL_EMMC_TRACE(1, "SEND_IF_COND(CMD8) failed - Not a high Capacity Card!");
        }
    }

    /*Disable interrupts (OCR response has cmd indx and cmd crc set to all 1's)*/
    sdhci_mask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);
    host->card->type = host->id;

    /*
    *   First time, pass NULL argument to get back values card is compatible with
    *   Send appropriate CMD Sequence to Identify the type of card inserted
    */
    repeats = 0;
    arg = 0;
    do {
        switch (host->card->type) {
            case CARD_SDIO:
                HAL_EMMC_TRACE(1, "Trying SDIO Protocol...");
                mmc_send_cmmand(host, IO_SEND_OP_COND, arg, MMC_RSP_R4);
                mmc_wait_for_resp(host, CARD_RESPONSE_R4);
                if (0 == host->card->ocr) {
                    HAL_EMMC_TRACE(1, "IO_SEND_OP_COND (CMD5) failed...");
                    arg = 0;
                }
                break;
            case CARD_SD:
                HAL_EMMC_TRACE(1, "Trying SD Protocol...");
                mmc_send_cmmand(host, APP_CMD, 0, MMC_RSP_R1);
                mmc_wait_for_resp(host, CARD_RESPONSE_R1);
                mmc_send_cmmand(host, SD_SEND_OP_COND, arg, MMC_RSP_R3);
                mmc_wait_for_resp(host, CARD_RESPONSE_R3);
                if (0 == host->card->ocr) {
                    HAL_EMMC_TRACE(1, "SD_SEND_OP_COND (ACMD41) failed...");
                    arg = 0;
                }
                break;
            case CARD_MMC:
                HAL_EMMC_TRACE(1, "Trying MMC Protocol...");
                mmc_send_cmmand(host, SEND_OP_COND, arg, MMC_RSP_R3);
                mmc_wait_for_resp(host, CARD_RESPONSE_R3);
                if (0 == host->card->ocr) {
                    HAL_EMMC_TRACE(1, "SEND_OP_COND (CMD1) failed...");
                }
                break;
        }

        /*Update the argument*/
        switch (host->card->type) {
            case CARD_SDIO:
                host->card->ocr = host->card->ocr & 0x00FFFFFF;
                arg = host->card->ocr | SWITCH_1_8V_ACCEPTED;
                break;
            case CARD_SD:
                arg = host->card->ocr | CARD_CAPACITY_STATUS | SWITCH_1_8V_ACCEPTED;
                break;
            case CARD_MMC:
                arg = host->card->ocr | CARD_CAPACITY_STATUS;
                break;
            default:
                arg = host->card->ocr;
                break;
        }
        HAL_EMMC_TRACE(1, "OCR: 0x%08x", host->card->ocr);
        if ((host->cmd->resp[0] & CARD_POWER_UP_STATUS) == CARD_POWER_UP_STATUS) {
            host->delay(1);
        }
        repeats++;
    } while (((host->cmd->resp[0] & CARD_POWER_UP_STATUS) != CARD_POWER_UP_STATUS) && (repeats < MAX_TRYCNT));

    if (host->id == DEV_TYPE_SDIO) {
        do {
            mmc_send_cmmand(host, IO_SEND_OP_COND, arg, MMC_RSP_R4);
            mmc_wait_for_resp(host, CARD_RESPONSE_R4);
        } while ((((host->cmd->resp[0] & CARD_POWER_UP_STATUS) != CARD_POWER_UP_STATUS) && (repeats < MAX_TRYCNT)));
    }

    if (repeats >= MAX_TRYCNT) {
        HAL_EMMC_ERROR(1, "%s: FAILED", __FUNCTION__);
        ret = MMC_DETECT_CARD_ERR;
        goto detect_out;
    } else {
        HAL_EMMC_TRACE(1, "%s: SUCCESS", __FUNCTION__);
    }

detect_out:
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

void init_card_info(struct mmc_host *host)
{
    if (host == NULL) {
        host->error_code = MMC_ERR_NULL_POINTER;
        return;
    }
    host->card_info = &card_info;
}

int init_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);
    host->card_info = &card_info;

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            ret = mmc_init_sdio_card(host);
            break;
        case CARD_SD:
            ret = mmc_init_sd_card(host);
            break;
        case CARD_MMC:
            ret = mmc_init_mmc_card(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

int config_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;

    HAL_EMMC_TRACE(1, "%s:timing=%d,khz=%d,width=%d", __FUNCTION__, timing, khz, width);

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->card->timing == timing && host->card->width == width) {
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            ret = mmc_config_sdio_card(host, timing, khz, width);
            break;
        case CARD_SD:
            ret = mmc_config_sd_card(host, timing, khz, width);
            break;
        case CARD_MMC:
            ret = mmc_config_mmc_card(host, timing, khz, width);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            goto config_out;
            break;
    }
    host->card->timing = timing;
    host->card->width = width;

config_out:
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

static int write_card_raw(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    int ret = MMC_NO_ERR;

    HAL_EMMC_TRACE(1, "%s:card_offset=0x%x,buf=0x%x,sz=%d", __FUNCTION__, card_offset, (uintptr_t)buf, data_size);

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            ret = sdio_write_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        case CARD_SD:
            ret = sd_write_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        case CARD_MMC:
            ret = mmc_write_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    if (ret) {
        host->in_use = 0;
    }
    return ret;
}

static int read_card_raw(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    int ret = MMC_NO_ERR;

    HAL_EMMC_TRACE(1, "%s:card_offset=%x,buf=0x%x,sz=%d", __FUNCTION__, card_offset, (uintptr_t)buf, data_size);

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            ret = sdio_read_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        case CARD_SD:
            ret = sd_read_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        case CARD_MMC:
            ret = mmc_read_card(host, card_offset, buf, data_size, blocking_mode, dma_en, 0);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    if (ret) {
        host->in_use = 0;
    }
    return ret;
}

int write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return write_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_IRQ_OFF, true);
}

int read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return read_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_IRQ_OFF, true);
}

int write_card_ext(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    int ret = MMC_NO_ERR;
    uintptr_t addr;
    uint32_t cur_size;

    addr = (uintptr_t)buf;
    do {
        cur_size = (data_size > CARD_XFER_SIZE_MAX) ? CARD_XFER_SIZE_MAX : data_size;
        ret = write_card(host, card_offset, (void *)(addr), cur_size);
        if (ret) {
            HAL_EMMC_ERROR(0, "emmc write error %d", ret);
            return ret;
        }
        data_size -= cur_size;
        addr += cur_size;
        card_offset += cur_size / host->blksz;
    } while (data_size);

    return ret;
}

int read_card_ext(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    int ret = MMC_NO_ERR;
    uintptr_t addr;
    uint32_t cur_size;

    addr = (uintptr_t)buf;
    do {
        cur_size = (data_size > CARD_XFER_SIZE_MAX) ? CARD_XFER_SIZE_MAX : data_size;
        ret = read_card(host, card_offset, (void *)(addr), cur_size);
        if (ret) {
            HAL_EMMC_ERROR(0, "emmc read error %d", ret);
            return ret;
        }
        data_size -= cur_size;
        addr += cur_size;
        card_offset += cur_size / host->blksz;
    } while (data_size);

    return ret;
}

int write_card_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return write_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_IRQ_OFF, false);
}

int read_card_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return read_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_IRQ_OFF, false);
}

int write_card_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return write_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_NONE, true);
}

int read_card_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size)
{
    return read_card_raw(host, card_offset, buf, data_size, BLOCKING_MODE_NONE, true);
}

int write_card_blks_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt)
{
    return write_card_raw(host, card_offset, buf, block_cnt * CARD_BLOCK_LENGTH, BLOCKING_MODE_IRQ_OFF, false);
}

int read_card_blks_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt)
{
    return read_card_raw(host, card_offset, buf, block_cnt * CARD_BLOCK_LENGTH, BLOCKING_MODE_IRQ_OFF, false);
}

int write_card_blks_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt)
{
    return write_card_raw(host, card_offset, buf, block_cnt * CARD_BLOCK_LENGTH, BLOCKING_MODE_NONE, true);
}

int read_card_blks_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt)
{
    return read_card_raw(host, card_offset, buf, block_cnt * CARD_BLOCK_LENGTH, BLOCKING_MODE_NONE, true);
}

static int read_write_card_raw(struct mmc_host *host, uint32_t arg, uint8_t *buf, uint16_t len, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_gen_cmd(host, arg, buf, len, blocking_mode, dma_en);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    if (ret) {
        host->in_use = 0;
    }
    return ret;
}

int read_write_card_blocking_mode(struct mmc_host *host, uint32_t arg, uint8_t *buf)
{
    return read_write_card_raw(host, arg, buf, 512, BLOCKING_MODE_IRQ_OFF, false);
}

int read_write_card_dma_mode(struct mmc_host *host, uint32_t arg, uint8_t *buf)
{
    return read_write_card_raw(host, arg, buf, 512, BLOCKING_MODE_NONE, true);
}

int read_write_card_stuff_blocking_mode(struct mmc_host *host, uint32_t stuff_bits, uint8_t read1_write0, uint8_t *buf)
{
    uint32_t arg;

    arg = (stuff_bits << 1) | (!!read1_write0);
    return read_write_card_raw(host, arg, buf, 512, BLOCKING_MODE_IRQ_OFF, false);
}

int read_write_card_stuff_dma_mode(struct mmc_host *host, uint32_t stuff_bits, uint8_t read1_write0, uint8_t *buf)
{
    uint32_t arg;

    arg = (stuff_bits << 1) | (!!read1_write0);
    return read_write_card_raw(host, arg, buf, 512, BLOCKING_MODE_NONE, true);
}

int send_custom_data(struct mmc_host *host, uint32_t data)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }
    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_cmd60(host, data);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    return ret;
}

int switch_card(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_switch(host, type, index, value, cmdset);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

static uint32_t __div64_32(uint64_t *n, uint32_t base)
{
    uint64_t rem = *n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t) high << 32;
        rem -= (uint64_t)(high * base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return rem;
}

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
#define do_div(n,base) ({                               \
        uint32_t __base = (base);                       \
        uint32_t __rem;                                 \
        (void)(((typeof((n)) *)0) == ((uint64_t *)0));  \
        if (((n) >> 32) == 0) {                         \
            __rem = (uint32_t)(n) % __base;             \
            (n) = (uint32_t)(n) / __base;               \
        } else                                          \
            __rem = __div64_32(&(n), __base);           \
        __rem;                                          \
    })

static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
    *remainder = do_div(dividend, divisor);
    return dividend;
}

static int mmc_erase_t(struct mmc_host *host, uintptr_t start, uintptr_t blkcnt, uint32_t args)
{
    uintptr_t end;
    int ret, start_cmd, end_cmd;

    if (host->card->high_capacity) {
        end = start + blkcnt - 1;
    } else {
        end = (start + blkcnt - 1) * host->card->write_bl_len;
        start *= host->card->write_bl_len;
    }

    if (host->card->type == CARD_SD) {
        start_cmd = ERASE_WR_BLK_START;
        end_cmd = ERASE_WR_BLK_END;
    } else {
        start_cmd = ERASE_GROUP_START;
        end_cmd = ERASE_GROUP_END;
    }

    //set start addr
    ret = mmc_send_cmd_wait_resp(host, start_cmd, start, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    //set end addr
    ret = mmc_send_cmd_wait_resp(host, end_cmd, end, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    //Send command
    ret = mmc_send_cmd_wait_resp(host, ERASE, args ? args : 0, MMC_RSP_R1B, CARD_RESPONSE_R1B, 0, 0);

    return ret;
}

static int mmc_erase_blk(struct mmc_host *host, uintptr_t start_blk, uintptr_t blk_cnt)
{
    int err = MMC_INVALID_ERR;
    uint32_t card_status;
    uint32_t time_start, timeout;
    uintptr_t blk = 0, blk_r = 0;
    uint32_t start_rem, blkcnt_rem, erase_args = MMC_ERASE_ARG;

    HAL_EMMC_TRACE(0, "%s:start_blk=%x, blk_cnt=%x", __FUNCTION__, start_blk, blk_cnt);
    /*
     * We want to see if the requested start or total block count are
     * unaligned.  We discard the whole numbers and only care about the
     * remainder.
     */
    div_u64_rem(start_blk, host->card->erase_grp_size_card, &start_rem);
    div_u64_rem(blk_cnt, host->card->erase_grp_size_card, &blkcnt_rem);
    HAL_EMMC_TRACE(0, "erase_grp_size_card=%d, start_rem=%d, blkcnt_rem=%d",
                   host->card->erase_grp_size_card,
                   start_rem, blkcnt_rem);
    if (start_rem || blkcnt_rem) {
        if (host->card->can_trim) {
            /* Trim function applies the erase operation to write
            * blocks instead of erase groups.
            */
            erase_args = MMC_TRIM_ARG;
        } else {
            /* The card ignores all LSB's below the erase group
            * size, rounding down the addess to a erase group
            * boundary.
            */
            HAL_EMMC_ERROR(0, "\n\nCaution! Your devices Erase group is 0x%x\n"
                           "The erase range would be change to "
                           "0x%x ~ 0x%x\n\n",
                           host->card->erase_grp_size_card, start_blk & ~(host->card->erase_grp_size_card - 1),
                           ((start_blk + blk_cnt + host->card->erase_grp_size_card - 1)
                            & ~(host->card->erase_grp_size_card - 1)) - 1);
        }
    }

    while (blk < blk_cnt) {
        if ((host->card->type == CARD_SD) && 0/* mmc->ssr.au */) {
            // TODO: This mode is yet to be implemented
            /*blk_r = ((blk_cnt - blk) > mmc->ssr.au) ?
                    mmc->ssr.au : (blk_cnt - blk);*/
        } else {
            blk_r = ((blk_cnt - blk) > host->card->erase_grp_size_card) ?
                    host->card->erase_grp_size_card : (blk_cnt - blk);
        }
        err = mmc_erase_t(host, start_blk + blk, blk_r, erase_args);
        if (err)
            break;

        blk += blk_r;

        /* Waiting for the ready status */
        timeout = MS_TO_TICKS(host->card->ext_csd[EXT_CSD__ERASE_TIMEOUT_MULTI] * 300);
        time_start = hal_sys_timer_get();
        do {
            card_status = mmc_card_status(host);
            if ((hal_sys_timer_get() - time_start) > timeout) {
                HAL_EMMC_ERROR(0, "error: erase timeout within %dms", timeout);
                return MMC_ERASE_TIMEOUT;
            }
        } while (card_status != CARD_TRAN_STATE);
    }

    return err;
}

int erase_card(struct mmc_host *host, uintptr_t start_blk, uintptr_t blk_cnt)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
        case CARD_MMC:
            ret = mmc_erase_blk(host, start_blk, blk_cnt);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

int sleep_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_sleep(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

int awake_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_awake(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

int select_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_select_cards(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    host->in_use = 0;
    return ret;
}

int deselect_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = mmc_deselect_cards(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    host->in_use = 0;
    return ret;
}

int get_card_info(struct mmc_card_info *pcard_info)
{
    if (pcard_info) {
        memcpy(pcard_info, &card_info, sizeof(struct mmc_card_info));
        return MMC_NO_ERR;
    }

    return MMC_INVALID_ERR;
}

int get_card_status(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
        case CARD_MMC:
            ret = (int)mmc_card_status(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    host->in_use = 0;
    return ret;
}

int card_cache_control(struct mmc_host *host, uint8_t enable_disable)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = (int)mmc_cache_control(host, enable_disable);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}

int card_cache_flush(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        ret = MMC_ERR_NULL_POINTER;
        host->error_code = ret;
        return ret;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    switch (host->card->type) {
        case CARD_SDIO:
            break;
        case CARD_SD:
            break;
        case CARD_MMC:
            ret = (int)mmc_cache_flush(host);
            break;
        default:
            HAL_EMMC_ERROR(0, "%s, Unknown Card Type", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    host->error_code = ret;
    card_controller_clk_pause(host);
    host->in_use = 0;
    return ret;
}
