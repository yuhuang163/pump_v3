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
#include "card_mmc.h"
#include "emmc_dbg.h"
#include "sdhci_emphy.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"

#define SD_SWITCH_CHECK                     0
#define SD_SWITCH_SWITCH                    1

#define _sdmmc_be32_to_cpu(x) ((uint32_t)( \
                               (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
                               (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
                               (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
                               (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

static int sd_switch_voltage(struct mmc_host *host, int signal_voltage)
{
    int ret = MMC_NO_ERR;
    uint32_t status;
    POSSIBLY_UNUSED uint32_t timeout, busy_t;

    HAL_EMMC_TRACE(0, "%s:%d", __FUNCTION__, __LINE__);
    /*
     * Send CMD11 only if the request is to switch the card to 1.8V signalling
     * 3.3V signalling defalut DS  1.8V signalling default SDR12.
     * 3.3V signalling maximum support to 50M (HS)
     */
    if (signal_voltage == MMC_IO_VOLTAGE_3P3) {
        HAL_EMMC_ERROR(0, "%s, NOT SUPPORT", __FUNCTION__);
        return MMC_SWITCH_ERR;
    }

    ret = mmc_send_cmd_wait_resp(host, VOLTAGE_SWITCH, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    sdhci_stop_bus_clk(host);
    //Detect low level signals
    hal_sys_timer_delay_us(100);
    status = sdhciReadSdh_present_state_2Dat_level(host);
    if (status != 0xF) {
        HAL_EMMC_ERROR(0, "%s, BUS BUSY", __FUNCTION__);
        return MMC_SWITCH_ERR;
    }

    hal_sys_timer_delay(MS_TO_TICKS(5));
    sdhci_set_voltage(host, MMC_IO_VOLTAGE_1P8);
    sdhci_start_bus_clk(host);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    return ret;
}

static int sd_send_scr(struct mmc_host *host)
{
    /*ACMD51  response r1 and 64bit scr register information*/
    int ret = MMC_NO_ERR;
    unsigned int flags;
    unsigned int scr_buf[2];
    unsigned int scr[2];
    unsigned int scr_sd_ver = 0;
    unsigned int scr_bus_width = 0;
    unsigned int scr_sd_spec_3x = 0;
    sdhci_mask_irqs(host, IRQ_RD_DATA_CRC_ERR);

    if (host->dma_mode == MM4_32_BIT_ADMA_2) {
        ret = mmc_adma_init_desc(host, (void *)scr, 8, NULL);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
    mmc_prepare_transfer(host, scr_buf, CARD_READ, 8, 8, true);
    flags = MMC_DATA_PRESENT | MMC_DATA_READ | MMC_DATA_DMA;
    ret = mmc_send_cmd_wait_resp(host, APP_CMD, host->card->rca << 16, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = mmc_send_cmd_wait_resp(host, SEND_SCR, 0, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    sdhci_unmask_irqs(host, IRQ_RD_DATA_CRC_ERR);
    scr[0] = _sdmmc_be32_to_cpu(scr_buf[0]);
    scr[1] = _sdmmc_be32_to_cpu(scr_buf[1]);
    scr_sd_ver = scr[0] >> 24 & 0xf;
    scr_bus_width = scr[0] >> 16 & 0xf;
    scr_sd_spec_3x = scr[0] >> 15 & 0x1;

    host->card_info->device_version = scr_sd_ver;
    /*2.0 sdacrd support SDHC(32G) 3.0X support SDXC(2TB)*/
    HAL_EMMC_TRACE(1, "SCR: SD_SPEC=%X SD_SPEC3=%X", scr_sd_ver, scr_sd_spec_3x);
    HAL_EMMC_TRACE(1, "SCR: SD_BUS_WIDTH=%X", scr_bus_width);

    return ret;
}

int mmc_init_sd_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int r3_s18a;

    /*Determine if this card if a High Capacity card  bit30 CCS*/
    host->card->high_capacity = (host->card->ocr & ACCESS_MODE) == ACCESS_MODE__SECTOR_MODE;
    if (host->card->high_capacity) {
        host->card_info->cardtype = MMC_HIGH_CAPACITY_CARD;
        HAL_EMMC_TRACE(1, "SDcard High Capacity Card!");
    } else {
        host->card_info->cardtype = MMC_LOW_CAPACITY_CARD;
    }
    //switching to 1.8v accepted
    r3_s18a = host->card->ocr & ACCESS_MODE_S18A;

    //3.3 S18A value 0,the card does not support 1.8v signaling
    //1.8 S18A value 0,alreay switched to 1.8V
    if (r3_s18a) {
        sd_switch_voltage(host, MMC_IO_VOLTAGE_1P8);
    }

    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR);

    /*mmc_all_send_cid*/
    ret = mmc_send_cmd_wait_resp(host, ALL_SEND_CID, 0, MMC_RSP_R2, CARD_RESPONSE_R2, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    HAL_EMMC_TRACE(1, "CID: 0x%08X 0x%08X 0x%08X 0x%08X", host->card->raw_data[0], host->card->raw_data[1], host->card->raw_data[2], host->card->raw_data[3]);

    /*Enable interrupts (RCA response has valid cmd indx and cmd crc)*/
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    host->card->rca = MMC_RCA_VALUE_DEFAULT;

    ret = mmc_send_cmd_wait_resp(host, SET_RELATIVE_ADDR, (host->card->rca << 16), MMC_RSP_R6, CARD_RESPONSE_R6, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    host->card->rca = (host->cmd->resp[0] >> 16) & 0xffff;
    HAL_EMMC_TRACE(0, "SDCard RCA: 0x%08x", host->card->rca);

    /*Disable interrupts (CSD response has cmd indx all set to 1's)*/
    sdhci_mask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    /*mmc_send_csd*/
    ret = mmc_send_cmd_wait_resp(host, SEND_CSD, (host->card->rca << 16), MMC_RSP_R2, CARD_RESPONSE_R2, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    host->card_info->cardclass = host->card->csd.ccc;
    memcpy(&host->card_info->cid, &host->card->cid, sizeof(struct mmc_cid));
    memcpy(&host->card_info->csd, &host->card->csd, sizeof(struct mmc_csd));

    HAL_EMMC_TRACE(0, "CSD: 0x%08X 0x%08X 0x%08X 0x%08X", host->card->raw_data[0], host->card->raw_data[1], host->card->raw_data[2], host->card->raw_data[3]);

    /*Enable interrupts (RCA response has valid cmd indx and cmd crc)*/
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    /*Set bus rate to 26MHZ*/
    sdhci_set_bus_clk(host, F_DS);

    /*mmc_select_card*/
    ret = mmc_send_cmd_wait_resp(host, SELECT_CARD, (host->card->rca << 16), MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /*Check device is locked*/
    if (host->card->r1 & (1 << 25)) {
        HAL_EMMC_ERROR(2, "***%s:%d, device is locked", __FUNCTION__, __LINE__);

        /*unlocked device*/
        ret = mmc_send_cmd_wait_resp(host, LOCK_UNLOCK, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
    HAL_EMMC_TRACE(2, "---%s:%d, device is unlocked", __FUNCTION__, __LINE__);

    /*Check for card TRAN_STATE state*/
    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);
    if (ret) {
        HAL_EMMC_ERROR(0, "Card is NOT in TRANS-STATE mode!!");
        return ret;
    }

    /*Enable interrupts that identify Data Transfer Errors*/
    sdhci_unmask_irqs(host, IRQ_CMD_COMP | EMMC_IRQ_ERR_MASK | IRQ_TUNE_ERR);
    if (host->card_info->cardtype == MMC_LOW_CAPACITY_CARD) {
        //Calculate some sdcard information from cid & csd
        host->card_info->blocknbr = host->card->csd.c_size + 1U;
        host->card_info->blocknbr *= (1 << ((host->card->csd.c_size_mult & 0x07U) + 2U));
        host->card_info->blocksize = (1 << (host->card->csd.read_bl_len & 0x0FU));
        host->card_info->logblocknbr = host->card_info->blocknbr * (host->card_info->blocksize / 512U);
        host->card_info->logllocksize = CARD_BLOCK_LENGTH;
    } else {
        host->card_info->blocknbr = host->card->csd.c_size + 1U;
        host->card_info->blocksize = 512U * 1024U;
        host->card_info->logllocksize = CARD_BLOCK_LENGTH;
    }

    /*default DS/1BIT skip config card*/
    host->card->timing = DS;
    host->card->width = MMC_1BIT_WIDTH;
    return ret;
}

static int sd_switch(struct mmc_host *host, int mode, int group, int value)
{
    /* mode0 check funchtion, mode 1 set function*/
    /*group 3 for drive strength,group 0 for access mode*/
    int ret;
    uint32_t cmd6_arg;
    uint32_t timeout, busy_t;
    uint32_t status;
    cmd6_arg = (mode << 31) | 0xffffff;
    cmd6_arg &= ~(0xf << (group * 4));
    cmd6_arg |= value << (group * 4);

    ret = mmc_send_cmd_wait_resp(host, SWITCH_FUNC, cmd6_arg, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    //wait for bus idle
    timeout = MS_TO_FAST_TICKS(500);
    busy_t = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - busy_t < timeout) {
            status = sdhciReadSdh_present_state_2Dat_level(host);
            if (status == 0xF) {
                break;
            } else {
                host->delay(1);
            }
        } else {
            HAL_EMMC_ERROR(0, "%s, wait for bus idle timeout", __FUNCTION__);
            return MMC_BUS_IDLE_TIMEOUT;
        }
    }

    busy_t = hal_fast_sys_timer_get();
    do {
        status = mmc_card_raw_status(host);

        if (status & CARD_STATUS_SWITCH_ERROR) {
            HAL_EMMC_ERROR(0, "***%s:%d, LR=%p, mmc switch error!", __FUNCTION__, __LINE__, __builtin_return_address(0));
            return MMC_SWITCH_ERR;
        }
        if ((status & CARD_STATUS_RDY_FOR_DATA) && (status & CARD_STATUS_CURR_STATE) == CARD_STATE_TRANS) {
            break;
        }
        host->delay(1);
    } while (hal_fast_sys_timer_get() - busy_t < timeout);

    return ret;
}

static int sd_set_sdr104(struct mmc_host *host, unsigned width, unsigned int voltage, unsigned int drv)
{
    int ret = MMC_NO_ERR;
    uint32_t bus_speed = 3;

    /* cmd6 set speed mode switch to SDR104 */
    ret = sd_switch(host, SD_SWITCH_SWITCH, 0, bus_speed);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    sdhci_set_bus_timing(host, SDR104);

    /*Config PHY relate register for SDR104*/
    //tx config
    sdhci_sel_base_clk(host);/* Select clock from the original inverter of base clock or ddll output clock*/

    /*Raise the freq to 200MHZ*/
    sdhci_set_bus_clk(host, F_SDR104);
#ifdef EMMC_DEBUG
    unsigned int data;
    HAL_EMMC_TRACE(0, "------sdr104 tuning start");
    data = hal_fast_sys_timer_get();
#endif
    ret = sdhci_try_tuning(host, SEND_TUNING_BLOCK);
#ifdef EMMC_DEBUG
    data = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - data);
    HAL_EMMC_TRACE(1, "------sdr104 tuning end, %dus", data);
#endif
    return ret;
}

int mmc_config_sd_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;
    unsigned int bus_speed = 0;
    unsigned int bus_width = 0;
    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    //ACMD51 to checkout sd_spec
    ret = sd_send_scr(host);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    hal_sys_timer_delay_us(1000);

    /* V1.10 card support CMD6*/
    if (host->card_info->device_version == 0) {
        HAL_EMMC_TRACE(0, "V1.0 SDcard skips the configuration");
        return MMC_NO_ERR;
    }
    /*must send ACMD51 before cmd6, v1.10+ support cmd6*/
    ret = sd_switch(host, SD_SWITCH_CHECK, 0, 1);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    // acmd6 set bus  width
    switch (width) {
        case MMC_4BIT_WIDTH:
            bus_width = 2;
            break;
        case MMC_1BIT_WIDTH:
            bus_width = 0;
            break;
        default:
            HAL_EMMC_TRACE(0, "%s, Unsupported bus width", __FUNCTION__);
            ret = MMC_INVALID_ERR;
            break;
    }
    ret = mmc_send_cmd_wait_resp(host, APP_CMD, host->card->rca << 16, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = mmc_send_cmd_wait_resp(host, SWITCH_FUNC, bus_width, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    sdhci_set_bus_width(host, width);

    switch (timing) {
        case DS:
        case SDR12:
            bus_speed = 0;
            break;
        case HS:
        case SDR25:
            bus_speed = 1;
            break;
        case SDR50:
            bus_speed = 2;
            break;
        case SDR104:
            return sd_set_sdr104(host, width, MMC_IO_VOLTAGE_1P8, MMC_DRV_50OHM_X1);
            break;
        case DDR50:
            bus_speed = 4;
            break;
        default:
            HAL_EMMC_ERROR(0, "%s:%d, Unsupported timing mode", __FUNCTION__, __LINE__);
            return MMC_INVALID_ERR;
            break;
    }

    //cmd6 set speed mode
    ret = sd_switch(host, SD_SWITCH_SWITCH, 0, bus_speed);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /*mmc_set_blk_len*/
    ret = mmc_send_cmd_wait_resp(host, SET_BLOCKLEN, CARD_BLOCK_LENGTH, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    sdhci_set_bus_timing(host, timing);
    sdhci_set_bus_clk(host, khz);

    //tuning cmd line, data line cannot be calibrated in this mode
    sdhci_tuning_cmdline(host);

    return ret;
}

int sd_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int cmd, flags;
    unsigned int card_status;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    mmc_card_set_blocking_mode(host, blocking_mode);
    HAL_EMMC_TRACE(0, "write card blocking mode:%d, irq_en:%d, irq_en_cp:%d", blocking_mode, host->irq_en, host->irq_en_cp);

    if (host->tran_status) {
        card_status = mmc_card_status(host);
        if (card_status == CARD_TRAN_STATE) {
            host->tran_status = 0;
        } else {
            HAL_EMMC_ERROR(0, "******%s, Data exited abnormally, cmd12 needs to be sent", __FUNCTION__);
            ret = mmc_send_stop(host, BLOCKING_MODE_IRQ_OFF);
            if (ret) {
                HAL_EMMC_ERROR(0, "******%s, Send stop failed to send, ret:%d", __FUNCTION__, ret);
                return MMC_SEND_STOP_ERR;
            }
        }
    }

    if (sz % CARD_BLOCK_LENGTH) {
        blks = sz / CARD_BLOCK_LENGTH + 1;
        if (blks > EMMC_BLK_CNT_MAX) {
            ret = MMC_BLOCK_COUNT_ERR;
            HAL_EMMC_ERROR(0, "******%s, The value of blks %d is too large", __FUNCTION__, blks);
            goto write_error_out;
        }
    } else {
        blks = sz / CARD_BLOCK_LENGTH;
    }

    flags = MMC_DATA_PRESENT | MMC_DATA_WRITE;
    if (blks == 1) {
        cmd = WRITE_SINGLE_BLOCK;
    } else {
        cmd = WRITE_MULTIPLE_BLOCK;
        flags |= MMC_DATA_MB | MMC_DATA_BC_EN;
        switch (host->auto_cmd_type) {
            case MM4_AUTO_CMD23:
                flags |= MMC_DATA_AUTOCMD23;
                ret = mmc_set_auto_cmd23(host, blks, dma_en);
                if (ret != MMC_NO_ERR) {
                    goto write_error_out;
                }
                break;
            case MM4_AUTO_CMD12://Open-ended Multiple block write mode
                flags |= MMC_DATA_AUTOCMD12;
                /*do nothing*/
                break;
            case MM4_MANUAL_CMD23:
                ret = mmc_set_blk_cnt(host, blks);
                if (ret != MMC_NO_ERR) {
                    goto write_error_out;
                }
                break;
            case MM4_MANUAL_CMD12://Open-ended Multiple block write mode
                /*do nothing*/
                break;
        }
    }
    if (dma_en) {
        flags |= MMC_DATA_DMA;
    }

    host->dma_status = dma_en;
    mmc_set_tran_mode(host, blocking_mode);
    mmc_prepare_transfer(host, buf, CARD_WRITE, CARD_BLOCK_LENGTH, sz, dma_en);
    ret = mmc_send_cmd_wait_resp(host, newcmd ? newcmd : cmd, card_offset, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 1, CARD_RCV_STATE);
    if (ret != MMC_NO_ERR) {
        goto write_error_out;
    }

    if (blocking_mode) {
        ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
        if (ret != MMC_NO_ERR) {
            goto write_error_out;
        }
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            return MMC_NO_ERR;
        } else {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            if (ret != MMC_NO_ERR) {
                goto write_error_out;
            }
        }
    }

    if (blks != 1) {
        switch (host->auto_cmd_type) {
            case MM4_AUTO_CMD23:
            case MM4_AUTO_CMD12:
            case MM4_MANUAL_CMD23:
                /*do nothing*/
                break;
            case MM4_MANUAL_CMD12:
                ret = mmc_send_stop(host, blocking_mode);
                if (ret != MMC_NO_ERR) {
                    goto write_error_out;
                }
                break;
            default:
                HAL_EMMC_TRACE(0, "%s, Unknown auto cmd type", __FUNCTION__);
                ret = MMC_INVALID_ERR;
                goto write_error_out;
                break;
        }
    }

    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);

write_error_out:
    host->tran_status = 0;

    card_controller_clk_pause(host);
    host->in_use = 0;

    return ret;
}

int sd_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int cmd, flags;
    unsigned int card_status;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    mmc_card_set_blocking_mode(host, blocking_mode);
    HAL_EMMC_TRACE(0, "read card blocking mode:%d, irq_en:%d, irq_en_cp:%d", blocking_mode, host->irq_en, host->irq_en_cp);

    if (host->tran_status) {
        card_status = mmc_card_status(host);
        if (card_status == CARD_TRAN_STATE) {
            host->tran_status = 0;
        } else {
            HAL_EMMC_ERROR(0, "******%s, Data exited abnormally, cmd12 needs to be sent", __FUNCTION__);
            ret = mmc_send_stop(host, BLOCKING_MODE_IRQ_OFF);
            if (ret) {
                HAL_EMMC_ERROR(0, "******%s, Send stop failed to send, ret:%d", __FUNCTION__, ret);
                return MMC_SEND_STOP_ERR;
            }
        }
    }

    if (sz % CARD_BLOCK_LENGTH) {
        blks = sz / CARD_BLOCK_LENGTH + 1;
        if (blks > EMMC_BLK_CNT_MAX) {
            ret = MMC_BLOCK_COUNT_ERR;
            HAL_EMMC_ERROR(0, "******%s, The value of blks %d is too large", __FUNCTION__, blks);
            goto read_error_out;
        }
    } else {
        blks = sz / CARD_BLOCK_LENGTH;
    }

    flags = MMC_DATA_PRESENT | MMC_DATA_READ;
    if (blks == 1) {
        cmd = READ_SINGLE_BLOCK;
    } else {
        cmd = READ_MULTIPLE_BLOCK;
        flags |= MMC_DATA_MB | MMC_DATA_BC_EN;
        switch (host->auto_cmd_type) {
            case MM4_AUTO_CMD23:
                flags |= MMC_DATA_AUTOCMD23;
                ret = mmc_set_auto_cmd23(host, blks, dma_en);
                if (ret != MMC_NO_ERR) {
                    goto read_error_out;
                }
                break;
            case MM4_AUTO_CMD12://Open-ended Multiple block read mode
                flags |= MMC_DATA_AUTOCMD12;
                break;
            case MM4_MANUAL_CMD23:
                ret = mmc_set_blk_cnt(host, blks);
                if (ret != MMC_NO_ERR) {
                    goto read_error_out;
                }
                break;
            case MM4_MANUAL_CMD12://Open-ended Multiple block read mode
                break;
        }
    }
    if (dma_en) {
        flags |= MMC_DATA_DMA;
    }
    host->dma_status = dma_en;
    mmc_set_tran_mode(host, blocking_mode);
    mmc_prepare_transfer(host, buf, CARD_READ, CARD_BLOCK_LENGTH, sz, dma_en);
    ret = mmc_send_cmd_wait_resp(host, newcmd ? newcmd : cmd, card_offset, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 1, CARD_DATA_STATE);
    if (ret != MMC_NO_ERR) {
        goto read_error_out;
    }

    if (blocking_mode) {
        ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
        if (ret != MMC_NO_ERR) {
            goto read_error_out;
        }
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            return MMC_NO_ERR;
        } else {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            if (ret != MMC_NO_ERR) {
                goto read_error_out;
            }
        }
    }

    if (blks != 1) {
        switch (host->auto_cmd_type) {
            case MM4_AUTO_CMD23:
            case MM4_AUTO_CMD12:
            case MM4_MANUAL_CMD23:
                /*do nothing*/
                break;
            case MM4_MANUAL_CMD12:
                ret = mmc_send_stop(host, blocking_mode);
                if (ret != MMC_NO_ERR) {
                    goto read_error_out;
                }
                break;
            default:
                HAL_EMMC_TRACE(0, "%s, Unknown auto cmd type", __FUNCTION__);
                ret = MMC_INVALID_ERR;
                goto read_error_out;
                break;
        }
    }

    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);

read_error_out:
    host->tran_status = 0;

    card_controller_clk_pause(host);
    host->in_use = 0;

    return ret;
}
