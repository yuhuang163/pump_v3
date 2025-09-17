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
#include "emmc_dbg.h"
#include "card_mmc.h"
#include "cmsis_nvic.h"
#include "sdhci_emphy.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"

#ifdef EMMC_RPMB_ENABLE
    #include "stdlib.h"
    #include "hal_trace.h"
    #include "mbedtls/md.h"

    SYNC_FLAGS_LOC static struct mmc_rpmb_frame mmc_rpmb_frame;
#endif

int mmc_card_set_blocking_mode(struct mmc_host *host, enum BLOCKING_MODE blocking_mode)
{
    if (host == NULL) {
        return MMC_ERR_NULL_POINTER;
    }

    if (blocking_mode == BLOCKING_MODE_IRQ_OFF) {
        host->irq_en = false;
        if (host->irq_en_cp != host->irq_en) {
            host->irq_en_cp = host->irq_en;
            NVIC_DisableIRQ(EMMC_IRQn);
        }
    } else {
        host->irq_en = true;
        if (host->irq_en_cp != host->irq_en) {
            host->irq_en_cp = host->irq_en;
            NVIC_EnableIRQ(EMMC_IRQn);
        }
    }
    return MMC_NO_ERR;
}

int mmc_check_card_status(struct mmc_host *host, unsigned int status, unsigned char en_delay)
{
    int ret;
    unsigned int card_status;
    unsigned int start;
    unsigned int timeout = MS_TO_FAST_TICKS(WAIT_STATUS_TIMEOUT);

    start = hal_fast_sys_timer_get();
    while (1) {
        if (hal_fast_sys_timer_get() - start < timeout) {
            card_status = mmc_card_status(host);
            if (card_status == status) {
                ret = MMC_NO_ERR;
                break;
            }
            if (en_delay) {
                host->delay(1);
            }
        } else {
            ret = MMC_CARD_STATUS_ERR;
            break;
        }
    }

    return ret;
}

int mmc_send_cmd_wait_resp(struct mmc_host *host, unsigned int opcode, unsigned int arg, unsigned int flags,
                           unsigned int resp_type, bool send_stop, unsigned int state)
{
    int ret = MMC_NO_ERR;
    unsigned int status;
    uint8_t cmd_retries, flow_retries;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    cmd_retries = 0;
    flow_retries = 0;
    do {
        ret = mmc_send_cmmand(host, opcode, arg, flags);
        if (ret) {
            cmd_retries++;
            HAL_EMMC_ERROR(0, "%s, cmd%d retries:%d", __FUNCTION__, opcode, cmd_retries);
            if (cmd_retries >= EMMC_ERROR_RETRY_CNT) {
                return ret;
            }
        } else {
            cmd_retries = 0;
            ret = mmc_wait_for_resp(host, resp_type);
            if (ret != MMC_NO_ERR) {
                if (send_stop) {
                    status = mmc_card_raw_status(host);
                    HAL_EMMC_ERROR(0, "******%s, card raw status:0x%X, state:%d", __FUNCTION__, status, (status >> 9) & 0xF);
                    if (((status >> 9) & 0xF) == state) {
                        HAL_EMMC_ERROR(0, "******%s, cmd%d manual stop(cmd12)", __FUNCTION__, opcode);
                        mmc_send_stop(host, BLOCKING_MODE_IRQ_OFF);
                    }
                }

                flow_retries++;
                HAL_EMMC_ERROR(0, "******%s, cmd%d flow retries:%d", __FUNCTION__, opcode, flow_retries);
                if (flow_retries >= EMMC_ERROR_RETRY_CNT) {
                    return ret;
                }
            }
        }
    } while (ret);

    return MMC_NO_ERR;
}

int mmc_set_tran_mode(struct mmc_host *host, enum BLOCKING_MODE blocking_mode)
{
    if (blocking_mode) {
        host->tran_mode = 1;
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            host->tran_mode = 0;
        } else {
            host->tran_mode = 1;
        }
    }

    return MMC_NO_ERR;
}

int mmc_write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int sec_conut;
    unsigned int cmd, flags;
    unsigned int card_status;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    sec_conut = (unsigned int)(*(unsigned int *)&host->card->ext_csd[212]);//get max sector conut
    HAL_EMMC_TRACE(0, "%s:%d, sector conut[SEC_COUNT]:0x%X", __FUNCTION__, __LINE__, sec_conut);

    if (card_offset >= sec_conut) {
        return MMC_OFFSET_ERR;
    } else if (((uint64_t)card_offset * 512 + sz) > (uint64_t)sec_conut * 512) {
        return MMC_RW_SIZE_ERR;
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

#ifdef EMMC_HS400_SYSFREQ_RAISE
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
        //In this mode, the cpu runs at least 200M
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
    }
#endif

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
#ifdef EMMC_HS400_SYSFREQ_RAISE
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
    }
#endif

    card_controller_clk_pause(host);
    host->in_use = 0;

    return ret;
}

#ifdef EMMC_RPMB_ENABLE
static int mmc_rpmb_write_raw(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, bool re_write)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int sec_conut;
    unsigned int cmd, flags;
    unsigned int card_status;
    bool dma_en = true;
    enum BLOCKING_MODE blocking_mode = BLOCKING_MODE_IRQ_OFF;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    sec_conut = (unsigned int)(*(unsigned int *)&host->card->ext_csd[212]);//get max sector conut
    HAL_EMMC_TRACE(0, "%s:%d, sector conut[SEC_COUNT]:0x%X", __FUNCTION__, __LINE__, sec_conut);

    if (card_offset >= sec_conut) {
        return MMC_OFFSET_ERR;
    } else if (((uint64_t)card_offset * 512 + sz) > (uint64_t)sec_conut * 512) {
        return MMC_RW_SIZE_ERR;
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

#ifdef EMMC_HS400_SYSFREQ_RAISE
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
        //In this mode, the cpu runs at least 200M
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
    }
#endif

    if (sz % CARD_BLOCK_LENGTH) {
        blks = sz / CARD_BLOCK_LENGTH + 1;
        if (blks > EMMC_BLK_CNT_MAX) {
            ret = MMC_BLOCK_COUNT_ERR;
            HAL_EMMC_ERROR(0, "******%s, The value of blks %d is too large", __FUNCTION__, blks);
            goto write_rpmb_error_out;
        }
    } else {
        blks = sz / CARD_BLOCK_LENGTH;
    }

    flags = MMC_DATA_PRESENT | MMC_DATA_WRITE;

    cmd = WRITE_MULTIPLE_BLOCK;
    flags |= MMC_DATA_MB | MMC_DATA_BC_EN;

    if (re_write) {
        blks |= (unsigned int)(1 << 31);
    }

    mmc_set_blk_cnt(host, blks);

    if (dma_en) {
        flags |= MMC_DATA_DMA;
    }
    host->dma_status = dma_en;
    mmc_set_tran_mode(host, blocking_mode);
    mmc_prepare_transfer(host, buf, CARD_WRITE, CARD_BLOCK_LENGTH, sz, dma_en);
    ret = mmc_send_cmd_wait_resp(host, cmd, card_offset, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 1, CARD_RCV_STATE);
    if (ret != MMC_NO_ERR) {
        goto write_rpmb_error_out;
    }

    if (blocking_mode) {
        ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
        if (ret != MMC_NO_ERR) {
            goto write_rpmb_error_out;
        }
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            return MMC_NO_ERR;
        } else {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            if (ret != MMC_NO_ERR) {
                goto write_rpmb_error_out;
            }
        }
    }

    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);

write_rpmb_error_out:
    host->tran_status = 0;
#ifdef EMMC_HS400_SYSFREQ_RAISE
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
    }
#endif

    return ret;
}

static int mmc_rpmb_read_raw(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int sec_conut;
    unsigned int cmd, flags;
    unsigned int card_status;
    bool dma_en = true;
    enum BLOCKING_MODE blocking_mode = BLOCKING_MODE_IRQ_OFF;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    sec_conut = (unsigned int)(*(unsigned int *)&host->card->ext_csd[212]);//get max sector conut
    HAL_EMMC_TRACE(0, "%s:%d, sector conut[SEC_COUNT]:0x%X", __FUNCTION__, __LINE__, sec_conut);

    if (card_offset >= sec_conut) {
        return MMC_OFFSET_ERR;
    } else if (((uint64_t)card_offset * 512 + sz) > (uint64_t)sec_conut * 512) {
        return MMC_RW_SIZE_ERR;
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

    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        //In this mode, the cpu runs at least 200M
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
#endif
        if (host->strobe) {
            sdhci_host_emphy_clk_sel_ds(host);
        }
    }

    if (sz % CARD_BLOCK_LENGTH) {
        blks = sz / CARD_BLOCK_LENGTH + 1;
        if (blks > EMMC_BLK_CNT_MAX) {
            ret = MMC_BLOCK_COUNT_ERR;
            HAL_EMMC_ERROR(0, "******%s, The value of blks %d is too large", __FUNCTION__, blks);
            goto read_rpmb_error_out;
        }
    } else {
        blks = sz / CARD_BLOCK_LENGTH;
    }

    flags = MMC_DATA_PRESENT | MMC_DATA_READ;

    cmd = READ_MULTIPLE_BLOCK;
    flags |= MMC_DATA_MB | MMC_DATA_BC_EN;

    ret = mmc_set_blk_cnt(host, blks);
    if (ret != MMC_NO_ERR) {
        goto read_rpmb_error_out;
    }

    if (dma_en) {
        flags |= MMC_DATA_DMA;
    }
    host->dma_status = dma_en;
    mmc_set_tran_mode(host, blocking_mode);
    mmc_prepare_transfer(host, buf, CARD_READ, CARD_BLOCK_LENGTH, sz, dma_en);
    ret = mmc_send_cmd_wait_resp(host, cmd, card_offset, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 1, CARD_DATA_STATE);
    if (ret != MMC_NO_ERR) {
        goto read_rpmb_error_out;
    }

    if (blocking_mode) {
        ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
        if (ret != MMC_NO_ERR) {
            goto read_rpmb_error_out;
        }
    } else {
        if (host->dma_status && host->emmc_host_dma_done) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            return MMC_NO_ERR;
        } else {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
            if (ret != MMC_NO_ERR) {
                goto read_rpmb_error_out;
            }
        }
    }

    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);

read_rpmb_error_out:
    host->tran_status = 0;
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
#endif
        if (host->strobe) {
            sdhci_host_emphy_clk_sel_pck_delay(host);//revert back to the original clk
        }
    }

    return ret;
}
#endif

int mmc_read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, enum BLOCKING_MODE blocking_mode, bool dma_en, unsigned int newcmd)
{
    int ret = MMC_NO_ERR;
    unsigned int blks;
    unsigned int sec_conut;
    unsigned int cmd, flags;
    unsigned int card_status;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    sec_conut = (unsigned int)(*(unsigned int *)&host->card->ext_csd[212]);//get max sector conut
    HAL_EMMC_TRACE(0, "%s:%d, sector conut[SEC_COUNT]:0x%X", __FUNCTION__, __LINE__, sec_conut);

    if (card_offset >= sec_conut) {
        return MMC_OFFSET_ERR;
    } else if (((uint64_t)card_offset * 512 + sz) > (uint64_t)sec_conut * 512) {
        return MMC_RW_SIZE_ERR;
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

    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        //In this mode, the cpu runs at least 200M
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
#endif
        if (host->strobe) {
            sdhci_host_emphy_clk_sel_ds(host);
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
    if (host->speed_mode == HS400 || host->speed_mode == EHS400) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
#endif
        if (host->strobe) {
            sdhci_host_emphy_clk_sel_pck_delay(host);//revert back to the original clk
        }
    }

    card_controller_clk_pause(host);
    host->in_use = 0;

    return ret;
}

unsigned int mmc_card_raw_status(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int status;
    uint8_t cmd_retries, flow_retries;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return 0;
    }

    host->card->r1 = 0;
    cmd_retries = 0;
    flow_retries = 0;
    do {
        ret = mmc_send_cmmand(host, SEND_STATUS, (host->card->rca << 16), MMC_RSP_R1);
        if (ret) {
            cmd_retries++;
            HAL_EMMC_ERROR(0, "%s, cmd13 retries:%d", __FUNCTION__, cmd_retries);
            if (cmd_retries >= EMMC_ERROR_RETRY_CNT) {
                return 0;
            }
        } else {
            cmd_retries = 0;
            ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1);
            if (ret != MMC_NO_ERR) {
                flow_retries++;
                HAL_EMMC_ERROR(0, "******%s, cmd13 flow retries:%d", __FUNCTION__, flow_retries);
                if (flow_retries >= EMMC_ERROR_RETRY_CNT) {
                    return 0;
                }
            }
        }
    } while (ret);

    status = host->card->r1;
    HAL_EMMC_TRACE(1, "CARD_STATUS = 0x%08x", status);
    return status;
}

unsigned int mmc_card_status(struct mmc_host *host)
{
    unsigned int status;

    status = mmc_card_raw_status(host);
    return ((status >> 9) & 0xF);
}

int mmc_send_stop(struct mmc_host *host, enum BLOCKING_MODE blocking_mode)
{
    int ret = MMC_NO_ERR;
    uint8_t cmd_retries, flow_retries;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    cmd_retries = 0;
    flow_retries = 0;
    do {
        ret = mmc_send_cmmand(host, STOP_TRANSMISSION, 0, MMC_RSP_R1 | MMC_RSP_BUSY | MMC_CMD_AC);
        if (ret) {
            cmd_retries++;
            HAL_EMMC_ERROR(0, "%s, cmd12 retries:%d, ret:%d", __FUNCTION__, cmd_retries, ret);
            if (cmd_retries >= EMMC_ERROR_RETRY_CNT) {
                return ret;
            }
        } else {
            cmd_retries = 0;
            ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1);
            if (ret != MMC_NO_ERR) {
                flow_retries++;
                HAL_EMMC_ERROR(0, "******%s, cmd12 flow retries:%d, ret:%d", __FUNCTION__, flow_retries, ret);
                if (flow_retries >= EMMC_ERROR_RETRY_CNT) {
                    return ret;
                }
            } else {
                host->tran_status = 0;
            }
        }
    } while (ret);

    ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, blocking_mode);
    return ret;
}

int mmc_set_auto_cmd23(struct mmc_host *host, unsigned int blks, bool dma_en)
{
    int ret = MMC_NO_ERR;
    if (dma_en && host->dma_mode == MM4_SDMA) {
        HAL_EMMC_TRACE(1, "SDMA does not support AutoCMD23!");
        return MMC_INVALID_ERR;
    }
    sdhciWriteSdh_sysaddr(host, blks);
    return ret;
}

int mmc_set_blk_cnt(struct mmc_host *host, unsigned int blks)
{
    return mmc_send_cmd_wait_resp(host, SET_BLOCK_COUNT, blks, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
}

int mmc_switch_ext(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset, unsigned int timeout_ms)
{
    int ret;
    uint32_t cmd6_arg;
    uint32_t status;
    uint32_t timeout, busy_t;

    HAL_EMMC_TRACE(0, "  ");
    HAL_EMMC_TRACE(2, "%s:%d, Enter function", __FUNCTION__, __LINE__);
    HAL_EMMC_TRACE(1, "---cmd6 access: 0x%x", type);
    HAL_EMMC_TRACE(1, "---cmd6 index : %d", index);
    HAL_EMMC_TRACE(1, "---cmd6 value : 0x%x", value);
    HAL_EMMC_TRACE(1, "---cmd6 cmdset: 0x%x", cmdset);

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    ret = mmc_check_card_status(host, CARD_TRAN_STATE, 0);
    if (ret) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    cmd6_arg = (type << 24) | ((index & 0xff) << 16) | ((value & 0xff) << 8) | (cmdset & 0x7);
    ret = mmc_send_cmd_wait_resp(host, SWITCH, cmd6_arg, MMC_RSP_R1B, CARD_RESPONSE_R1B, 0, 0);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "%s:%d, error:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    //wait for bus idle
    timeout = MS_TO_FAST_TICKS(timeout_ms);
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

    HAL_EMMC_TRACE(0, "%s:%d, Exit function", __FUNCTION__, __LINE__);
    return ret;
}

int mmc_switch(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset)
{
    uint32_t timeout_ms = DEFAULT_CMD6_TIMEOUT;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host->card_info->generic_cmd6_time) {
        timeout_ms = host->card_info->generic_cmd6_time;
    }

    return mmc_switch_ext(host, type, index, value, cmdset, timeout_ms);
}

POSSIBLY_UNUSED static int mmc_partition_switch(struct mmc_host *host, unsigned int parti)
{
    uint32_t timeout_ms = DEFAULT_CMD6_TIMEOUT;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host->card->ext_csd[EXT_CSD__PARTITION_SWITCH_TIME]) {
        timeout_ms = host->card->ext_csd[EXT_CSD__PARTITION_SWITCH_TIME] * 10;
    }

    return mmc_switch_ext(host, CMD6_WR_BYTE, EXT_CSD__PARTITION_CONFIG, parti, EXT_CSD__S_CMD_SET__STD_MMC, timeout_ms);
}

#ifdef EMMC_RPMB_ENABLE
static int hmac_sha256(struct mmc_rpmb_frame *rpmb_frame, uint32_t frame_len, const uint8_t *key)
{
    int ret;
    mbedtls_md_type_t md_type;
    const mbedtls_md_info_t *info;

    uint32_t digest[8];
    uint8_t *msg = (uint8_t *)malloc(284 * frame_len);

    if (msg == NULL)
        return MMC_INVALID_ERR;

    for (int i = 0; i < frame_len; i++)
        memcpy(msg + 284 * i, &(rpmb_frame[i].data[0]), 284);

    HAL_EMMC_TRACE(0, " ");
    HAL_EMMC_TRACE(0, "%s: Start", __FUNCTION__);
    HAL_EMMC_TRACE(0, " ");

    md_type = MBEDTLS_MD_SHA256;
    info = mbedtls_md_info_from_type(md_type);

    ret = mbedtls_md_hmac(info, key, 32, msg, 284 * frame_len, (unsigned char *)&digest[0]);
#ifdef EMMC_ASSERT
    HAL_EMMC_ASSERT(ret == 0, "%s: ret=%d", __FUNCTION__, ret);
#else
    if (ret) {
        HAL_EMMC_ERROR(0, "%s, ret:%d", __FUNCTION__, ret);
        return MMC_RPMB_KEY_ERR;
    }
#endif
    for (int i = 0; i < 32; i++) {
        HAL_EMMC_TRACE(0, "%s: Get digest[%d]: 0x%x", __FUNCTION__, i, digest[i]);
    }
    for (int i = 0; i < 8; i++) {
        rpmb_frame[frame_len - 1].mac[4 * i] = digest[i] & 0x000000FF;
        rpmb_frame[frame_len - 1].mac[4 * i + 1] = (digest[i] & 0x0000FF00) >> 8;
        rpmb_frame[frame_len - 1].mac[4 * i + 2] = (digest[i] & 0x00FF0000) >> 16;
        rpmb_frame[frame_len - 1].mac[4 * i + 3] = (digest[i] & 0xFF000000) >> 24;
    }

    free(msg);
    return 0;
}

int mmc_switch_to_rpmb(struct mmc_host *host, enum EMMC_PARTITION_ACCESS mmc_parti)
{
    int ret = MMC_NO_ERR;
    unsigned int ext_rpmb_config = 0;
    switch (mmc_parti) {
        case NO_ACCESS_BOOT_DEFAULT:
            ext_rpmb_config = 0;
            break;
        case EMMC_RPMB_PART:
            ext_rpmb_config = (3 << 0);
            break;

        default:
            break;
    }

    ret = mmc_partition_switch(host, ext_rpmb_config);
    return ret;
}

int mmc_rpmb_set_key(struct mmc_host *host, uint8_t *key, uint32_t *result)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    memset(&mmc_rpmb_frame, 0, sizeof(struct mmc_rpmb_frame));

    for (int i = 0; i < RPMB_SZ_MAC; i++)
        mmc_rpmb_frame.mac[i] = ((char *)key)[i];

    mmc_rpmb_frame.request = (RPMB_REQ_KEY << 8);

    ret = mmc_rpmb_write_raw(host, 0, &mmc_rpmb_frame, sizeof(mmc_rpmb_frame), true);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        return ret;
    }

    memset(&mmc_rpmb_frame, 0, sizeof(struct mmc_rpmb_frame));
    mmc_rpmb_frame.request = (RPMB_REQ_STATUS << 8);
    ret = mmc_rpmb_write_raw(host, 0, &mmc_rpmb_frame, sizeof(mmc_rpmb_frame), false);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        return ret;
    }

    ret = mmc_rpmb_read_raw(host, 0, &mmc_rpmb_frame, sizeof(mmc_rpmb_frame));
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc read rpmb error,ret = %d", ret);
        return ret;
    }

    *result = ((mmc_rpmb_frame.result & 0xff) << 8) | ((mmc_rpmb_frame.result & 0xff00) >> 8);
    HAL_EMMC_TRACE(1, "emmc rpmb write key result:");
    HAL_EMMC_TRACE(1, "emmc set key,result = 0x%x", mmc_rpmb_frame.result);
    HAL_EMMC_TRACE(1, "emmc set key,request = 0x%x", mmc_rpmb_frame.request);
    HAL_EMMC_TRACE(1, " ");

    return ret;
}

int mmc_rpmb_get_counter(struct mmc_host *host, uint32_t *counter, uint32_t *result)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    memset(&mmc_rpmb_frame, 0, sizeof(struct mmc_rpmb_frame));

    for (int i = 0; i < RPMB_SZ_NONCE; i++)
        mmc_rpmb_frame.nonce[i] = 0;
    mmc_rpmb_frame.request = (RPMB_REQ_WCOUNTER << 8);

    ret = mmc_rpmb_write_raw(host, 0, &mmc_rpmb_frame, sizeof(mmc_rpmb_frame), false);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        return ret;
    }

    ret = mmc_rpmb_read_raw(host, 0, &mmc_rpmb_frame, sizeof(mmc_rpmb_frame));
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc read rpmb error,ret = %d", ret);
        return ret;
    }

    *counter = (mmc_rpmb_frame.write_counter & 0x000000FF) << 24;
    *counter |= (mmc_rpmb_frame.write_counter & 0x0000FF00) << 8;
    *counter |= (mmc_rpmb_frame.write_counter & 0x00FF0000) >> 8;
    *counter |= (mmc_rpmb_frame.write_counter & 0xFF000000) >> 24;
    *result = ((mmc_rpmb_frame.result & 0xff) << 8) | ((mmc_rpmb_frame.result & 0xff00) >> 8);

    HAL_EMMC_TRACE(1, " ");
    HAL_EMMC_TRACE(1, "emmc rpmb get counter result:");
    HAL_EMMC_TRACE(1, "emmc get counter,write counter = 0x%x", mmc_rpmb_frame.write_counter);
    HAL_EMMC_TRACE(1, "emmc get counter,result = 0x%x", mmc_rpmb_frame.result);
    HAL_EMMC_TRACE(1, "emmc get counter,request = 0x%x", mmc_rpmb_frame.request);
    HAL_EMMC_TRACE(1, " ");

    return ret;
}

int mmc_rpmb_write(struct mmc_host *host, uint32_t write_counter, unsigned short card_offset, void *buf, unsigned int sz, const uint8_t *key, uint32_t *result)
{
    int ret = MMC_NO_ERR;
    unsigned short blks = 0;
    int hmac_val = 0;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    if (sz % CARD_RPMB_DATA_LENGTH) {
        blks = sz / CARD_RPMB_DATA_LENGTH + 1;
    } else {
        blks = sz / CARD_RPMB_DATA_LENGTH;
    }
    struct mmc_rpmb_frame *rpmb_frame = (struct mmc_rpmb_frame *)malloc(blks * sizeof(struct mmc_rpmb_frame));

    if (rpmb_frame == NULL)
        return MMC_INVALID_ERR;

    memset(rpmb_frame, 0, blks * sizeof(struct mmc_rpmb_frame));

    for (int i = 0; i < blks; i++) {
        rpmb_frame[i].address = ((card_offset & 0xff00) >> 8) | ((card_offset & 0x00ff) << 8);
        rpmb_frame[i].block_count = ((blks & 0xff00) >> 8) | ((blks & 0x00ff) << 8);
        for (int j = 0; j < RPMB_SZ_NONCE; j++)
            rpmb_frame[i].nonce[j] = 0;
        memcpy(rpmb_frame[i].data, (const void *)((uint32_t)buf + i * CARD_RPMB_DATA_LENGTH), CARD_RPMB_DATA_LENGTH);
        rpmb_frame[i].write_counter = (write_counter & 0x000000FF) << 24;
        rpmb_frame[i].write_counter |= (write_counter & 0x0000FF00) << 8;
        rpmb_frame[i].write_counter |= (write_counter & 0x00FF0000) >> 8;
        rpmb_frame[i].write_counter |= (write_counter & 0xFF000000) >> 24;
        rpmb_frame[i].result = 0;
        rpmb_frame[i].request = (RPMB_REQ_WRITE_DATA << 8);
    }

    hmac_val = hmac_sha256(rpmb_frame, blks, key);
    if (hmac_val != 0) {
        HAL_EMMC_ERROR(1, "hmac_sha256 error,ret = %d", hmac_val);
        goto _exit;
    }

    ret = mmc_rpmb_write_raw(host, 0, rpmb_frame, blks * sizeof(struct mmc_rpmb_frame), true);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        goto _exit;
    }

    memset(rpmb_frame, 0, blks * sizeof(struct mmc_rpmb_frame));
    rpmb_frame[0]. request = (RPMB_REQ_STATUS << 8);

    ret = mmc_rpmb_write_raw(host, 0, rpmb_frame, sizeof(struct mmc_rpmb_frame), false);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        goto _exit;
    }

    ret = mmc_rpmb_read_raw(host, 0, rpmb_frame, sizeof(struct mmc_rpmb_frame));
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc read rpmb error,ret = %d", ret);
        goto _exit;
    }

    *result = ((rpmb_frame[0].result & 0xff) << 8) | ((rpmb_frame[0].result & 0xff00) >> 8);
    HAL_EMMC_TRACE(1, "emmc rpmb write data result:");
    HAL_EMMC_TRACE(1, "emmc write, write counter = 0x%x", rpmb_frame[0].write_counter);
    HAL_EMMC_TRACE(1, "emmc write, address = 0x%x", rpmb_frame[0].address);
    HAL_EMMC_TRACE(1, "emmc write, block_count = 0x%x", rpmb_frame[0].block_count);
    HAL_EMMC_TRACE(1, "emmc write, result = 0x%x", rpmb_frame[0].result);
    HAL_EMMC_TRACE(1, "emmc write, request = 0x%x", rpmb_frame[0].request);
    HAL_EMMC_TRACE(1, " ");

_exit:
    free(rpmb_frame);
    return ret;
}

int mmc_rpmb_read(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int sz, uint32_t *result)
{
    int ret = MMC_NO_ERR;
    unsigned short blks = 0;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    if (sz % CARD_RPMB_DATA_LENGTH) {
        blks = sz / CARD_RPMB_DATA_LENGTH + 1;
    } else {
        blks = sz / CARD_RPMB_DATA_LENGTH;
    }
    struct mmc_rpmb_frame *rpmb_frame = (struct mmc_rpmb_frame *)malloc(blks * sizeof(struct mmc_rpmb_frame));

    if (rpmb_frame == NULL)
        return MMC_INVALID_ERR;

    memset(rpmb_frame, 0, blks * sizeof(struct mmc_rpmb_frame));

    for (int i = 0; i < blks; i++) {
        rpmb_frame[i].address = ((card_offset & 0xff00) >> 8) | ((card_offset & 0x00ff) << 8);
        for (int j = 0; j < RPMB_SZ_NONCE; j++)
            rpmb_frame[i].nonce[j] = 0;
        rpmb_frame[i].request = (RPMB_REQ_READ_DATA << 8);
    }

    ret = mmc_rpmb_write_raw(host, 0, rpmb_frame, sizeof(struct mmc_rpmb_frame), false);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc write rpmb error,ret = %d", ret);
        goto _exit;
    }

    ret = mmc_rpmb_read_raw(host, 0, rpmb_frame, blks * sizeof(struct mmc_rpmb_frame));
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(1, "emmc read rpmb error,ret = %d", ret);
        goto _exit;
    }

    for (int j = 0; j < blks; j++) {
        memcpy((void *)((uint32_t)buf + j * CARD_RPMB_DATA_LENGTH), rpmb_frame[j].data, CARD_RPMB_DATA_LENGTH);
        host->delay(10);
    }

    *result = ((rpmb_frame[0].result & 0xff) << 8) | ((rpmb_frame[0].result & 0xff00) >> 8);
    HAL_EMMC_TRACE(1, "emmc rpmb read data result:");
    HAL_EMMC_TRACE(1, "emmc read, address = 0x%x", rpmb_frame[0].address);
    HAL_EMMC_TRACE(1, "emmc read, block_count = 0x%x", rpmb_frame[0].block_count);
    HAL_EMMC_TRACE(1, "emmc read, result = 0x%x", rpmb_frame[0].result);
    HAL_EMMC_TRACE(1, "emmc read, request = 0x%x", rpmb_frame[0].request);

_exit:
    free(rpmb_frame);
    return ret;
}
#endif

void mmc_print_ext_csd(struct mmc_host *host)
{
    int i;

    HAL_EMMC_TRACE(0, "  ");
    HAL_EMMC_TRACE(0, "Extended CSD Register:");
    for (i = 0; i < 512; i += 4) {
        HAL_EMMC_TRACE(1, "[%03d]=%02X [%03d]=%02X [%03d]=%02X [%03d]=%02X", i, host->card->ext_csd[i], i + 1,
                       host->card->ext_csd[i + 1], i + 2, host->card->ext_csd[i + 2],  i + 3, host->card->ext_csd[i + 3]);
    }
    HAL_EMMC_TRACE(0, "  ");
}

int mmc_send_ext_csd(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;
    unsigned int flags;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host->dma_mode == MM4_32_BIT_ADMA_2) {
        ret = mmc_adma_init_desc(host, (void *)host->card->ext_csd, MMC_EXT_CSD_LENGTH, NULL);
        if (ret != MMC_NO_ERR) {
            return ret;
        }
    }
    mmc_prepare_transfer(host, (void *)host->card->ext_csd, CARD_READ, MMC_EXT_CSD_LENGTH, MMC_EXT_CSD_LENGTH, true);
    flags = MMC_DATA_PRESENT | MMC_DATA_READ | MMC_DATA_DMA;
    ret = mmc_send_cmd_wait_resp(host, SEND_EXT_CSD, 0, MMC_RSP_R1 | flags, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    if (host->card->ext_csd[184]) {
        host->strobe = 1;
    }

#ifdef EMMC_DEBUG
    mmc_print_ext_csd(host);
#endif

    return ret;
}

int mmc_bus_test(struct mmc_host *host, unsigned int width)
{
    int ret = MMC_NO_ERR;
    unsigned char *dpw, *dpr;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

#ifdef EMMC_USE_HEAP
    dpw = (uint8_t *)emmc_heap_malloc(CARD_BLOCK_LENGTH);
    dpr = (uint8_t *)emmc_heap_malloc(CARD_BLOCK_LENGTH);
#else
    uint32_t dpw_buf[CARD_BLOCK_LENGTH / 4];
    uint32_t dpr_buf[CARD_BLOCK_LENGTH / 4];
    dpw = (uint8_t *)dpw_buf;
    dpr = (uint8_t *)dpr_buf;
#endif

    memset(dpw, 0, CARD_BLOCK_LENGTH);
    memset(dpr, 0, CARD_BLOCK_LENGTH);

    /*Setup Data Pattern*/
    switch (width) {
        case MMC_8BIT_WIDTH:
            dpw[0] = 0x55;
            dpw[1] = 0xaa;
            break;
        case MMC_4BIT_WIDTH:
            dpw[0] = 0x5a;
            break;
        case MMC_1BIT_WIDTH:
            dpw[0] = 0x40;
            break;
    }
    sdhci_set_bus_width(host, width);

    /*Since Host do not how many pins can be ok, so we disable the crc check*/
    sdhci_mask_irqs(host, IRQ_RD_DATA_CRC_ERR);

    /*write pattern*/
    if (host->dma_mode == MM4_32_BIT_ADMA_2) {
        ret = mmc_adma_init_desc(host, (void *)dpw, CARD_BLOCK_LENGTH, NULL);
        if (ret != MMC_NO_ERR) {
            return ret;
        }
    }
    mmc_prepare_transfer(host, dpw, CARD_WRITE, CARD_BLOCK_LENGTH, CARD_BLOCK_LENGTH, true);
    ret = mmc_send_cmd_wait_resp(host, BUSTEST_W, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*read pattern*/
    if (host->dma_mode == MM4_32_BIT_ADMA_2) {
        ret = mmc_adma_init_desc(host, (void *)dpr, CARD_BLOCK_LENGTH, NULL);
        if (ret != MMC_NO_ERR) {
            return ret;
        }
    }
    mmc_prepare_transfer(host, dpr, CARD_READ, CARD_BLOCK_LENGTH, CARD_BLOCK_LENGTH, true);
    ret = mmc_send_cmd_wait_resp(host, BUSTEST_R, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    ret = mmc_wait_for_xfer(host, SMALL_DATA_XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    sdhci_unmask_irqs(host, IRQ_RD_DATA_CRC_ERR);

    /*Verify Data Pattern*/
    switch (width) {
        case MMC_8BIT_WIDTH:
            if ((dpw[0] != dpr[0]) || (dpw[1] != dpr[1])) {
                return MMC_BUS_TEST_ERR;
            }
            break;
        case MMC_4BIT_WIDTH:
        case MMC_1BIT_WIDTH:
            if (dpw[0] != dpr[0]) {
                return MMC_BUS_TEST_ERR;
            }
            break;
    }

#ifdef EMMC_USE_HEAP
    emmc_heap_free(dpw);
    emmc_heap_free(dpr);
#endif

    return ret;
}

static int mmc_set_hs200(struct mmc_host *host, unsigned width, unsigned int voltage, unsigned int drv)
{
    int ret = MMC_NO_ERR;
    unsigned int device_type;
    unsigned int drv_strength;
    unsigned int ext_csd_hs_timing;
    unsigned int ext_csd_bus_width;
    unsigned int data;

    /*Check if the card support HS200*/
    device_type = host->card->ext_csd[196];
    HAL_EMMC_TRACE(1, "EXT_CSD[196]: device_type=%x", device_type);
    if (!is_sle()) {
        if (voltage == MMC_IO_VOLTAGE_1P8) {
            if (!(device_type & MMC_HS200_VCCIO_1P8V)) {
                HAL_EMMC_ERROR(1, "%s: The card does not support HS200 @ 1.8V IO.", __FUNCTION__);
                return MMC_INVALID_ERR;
            }
        } else if (voltage == MMC_IO_VOLTAGE_1P2) {
            if (!(device_type & MMC_HS200_VCCIO_1P2V)) {
                HAL_EMMC_ERROR(1, "%s: The card does not support HS200 @ 1.2V IO.", __FUNCTION__);
                return MMC_INVALID_ERR;
            }
        } else {
            HAL_EMMC_ERROR(1, "%s: Unsupported Vcc IO for HS200.", __FUNCTION__);
            return MMC_INVALID_ERR;
        }

        /*Check the device supported driver strength*/
        drv_strength = host->card->ext_csd[197];
        HAL_EMMC_TRACE(1, "EXT_CSD[197] = 0x%02x", drv_strength);
        if (!(drv_strength & drv)) {
            HAL_EMMC_ERROR(1, "%s: The card does not support this driver strength!", __FUNCTION__);
            return MMC_INVALID_ERR;
        }
    }

    /*Bus width*/
    if (width) {
        switch (width) {
            default:
            case MMC_8BIT_WIDTH:
                ext_csd_bus_width = 2;
                break;
            case MMC_4BIT_WIDTH:
                ext_csd_bus_width = 1;
                break;
        }
        HAL_EMMC_TRACE(1, "Bus width %d selected (EXT_CSD__BUS_WIDTH = 0x%0x)", width * 2, ext_csd_bus_width);
        ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BUS_WIDTH, ext_csd_bus_width, EXT_CSD__S_CMD_SET__STD_MMC);
        if (ret != MMC_NO_ERR) {
            return ret;
        }
        sdhci_set_bus_width(host, width);
    }

    /*Switch to HS200*/
    drv = 0;
    ext_csd_hs_timing = (drv << 4) | 0x2;
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__HS_TIMING, ext_csd_hs_timing, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    if (host->speed_mode == HS25) {
        data = HS25;
    } else if (host->speed_mode == HS50) {
        data = HS50;
    } else if (host->speed_mode == HS100) {
        data = HS100;
    } else {
        data = HS200;
    }
    sdhci_set_bus_timing(host, data);

    /*Config PHY relate register for HS200*/
    //tx config
    sdhci_sel_base_clk(host);/* Select clock from the original inverter of base clock or ddll output clock*/

    //set mmc_hs200 mode enable
    data = sdhciReadSdhc_mmc_ctrl_reg(host);
    data |= (1 << 10) | (1 << 12); /*MMC HS200 MODE | MMC CARD mode*/
    data &= ~((1 << 9) | (1 << 8)); /*clear MMC HS400 MODE |ENHANCE_STROBE_EN*/
    sdhciWriteSdhc_mmc_ctrl_reg(host, data);

    /*Raise the freq to 200MHZ*/
    if (host->speed_mode == HS25) {
        data = F_HS25;
    } else if (host->speed_mode == HS50) {
        data = F_HS50;
    } else if (host->speed_mode == HS100) {
        data = F_HS100;
    } else {
        data = F_HS200;
    }
    sdhci_set_bus_clk(host, data);
#ifdef EMMC_DEBUG
    HAL_EMMC_TRACE(0, "------hs200 tuning start");
    data = hal_fast_sys_timer_get();
#endif
    ret = sdhci_try_tuning(host, MMC_SEND_TUNING_BLOCK);
#ifdef EMMC_DEBUG
    data = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - data);
    HAL_EMMC_TRACE(1, "------hs200 tuning end, %dus", data);
#endif
    return ret;
}

static int mmc_set_hs400(struct mmc_host *host, unsigned int voltage, unsigned int drv)
{
    int ret = MMC_NO_ERR;
    unsigned int device_type;
    unsigned int drv_strength;
    unsigned int ext_csd_hs_timing;
    unsigned int data;

    /*Check if the card support HS400*/
    device_type = host->card->ext_csd[196];
    HAL_EMMC_TRACE(1, "EXT_CSD[196]: device_type=%x", device_type);
    if (!is_sle()) {
        if (voltage == MMC_IO_VOLTAGE_1P8) {
            if (!(device_type & MMC_HS400_VCCIO_1P8V)) {
                HAL_EMMC_ERROR(1, "%s: The card does not support HS400 @ 1.8V IO.", __FUNCTION__);
                return MMC_INVALID_ERR;
            }
        } else if (voltage == MMC_IO_VOLTAGE_1P2) {
            if (!(device_type & MMC_HS400_VCCIO_1P2V)) {
                HAL_EMMC_ERROR(1, "%s: The card does not support HS400 @ 1.2V IO.", __FUNCTION__);
                return MMC_INVALID_ERR;
            }
        } else {
            HAL_EMMC_ERROR(1, "%s: Unsupported Vcc IO for HS400.", __FUNCTION__);
            return MMC_INVALID_ERR;
        }

        /*Check the device supported driver strength*/
        drv_strength = host->card->ext_csd[197];
        HAL_EMMC_TRACE(1, "EXT_CSD[197] = 0x%02x", drv_strength);
        if (!(drv_strength & drv)) {
            HAL_EMMC_ERROR(1, "%s: The card does not support this driver strength!", __FUNCTION__);
            return MMC_INVALID_ERR;
        }
    }

    /*8Bit*/
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BUS_WIDTH, 0x2, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    sdhci_set_bus_width(host, MMC_8BIT_WIDTH);

    /*Switch to HS200*/
    drv = 0;
    ext_csd_hs_timing = (drv << 4) | 0x2;
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__HS_TIMING, ext_csd_hs_timing, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    sdhci_set_bus_timing(host, HS200);

    /*Config PHY relate register for HS200*/
    //tx config
    sdhci_sel_base_clk(host);/* Select clock from the original inverter of base clock or ddll output clock*/

    //set mmc_hs200 mode enable
    data = sdhciReadSdhc_mmc_ctrl_reg(host);
    data |= (1 << 10) | (1 << 12); /*MMC HS200 MODE | MMC CARD mode*/
    data &= ~((1 << 9) | (1 << 8)); /*clear MMC HS400 MODE |ENHANCE_STROBE_EN*/
    sdhciWriteSdhc_mmc_ctrl_reg(host, data);

    /*Raise the freq to 200MHZ*/
    sdhci_set_bus_clk(host, F_HS200);
#ifdef EMMC_DEBUG
    HAL_EMMC_TRACE(0, "------hs400 tuning start");
    data = hal_fast_sys_timer_get();
#endif
    ret = sdhci_try_tuning(host, MMC_SEND_TUNING_BLOCK);
    sdhci_host_emphy_tuning_correct(host);
#ifdef EMMC_DEBUG
    data = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - data);
    HAL_EMMC_TRACE(1, "------hs400 tuning end, %dus", data);
#endif
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*Config PHY relate register for HS mode*/
    //clear mmc_hs200 mode enable
    data = sdhciReadSdhc_mmc_ctrl_reg(host);
    data &= ~(1 << 10); /*clear MMC HS200 MODE*/
    sdhciWriteSdhc_mmc_ctrl_reg(host, data);

    /*
    *   Have to set HS_TIMING to 1 before switch to 8bit DDR
    *   Otherwise, we will have switch error.
    */
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__HS_TIMING, 0x1, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret) {
        return ret;
    }

    /*
    *   After swith HS_TIMING to HS, we have to set the clock rate down for the following command.
    *   Once we finish the the HS400 switch, we are able to raise the bus clock to 200MHZ.
    */
    sdhci_set_bus_clk(host, F_DS);

    /*Swith to 8 bit DDR*/
    if (host->strobe) {
        ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BUS_WIDTH, (0x6 | (0x1 << 7)), EXT_CSD__S_CMD_SET__STD_MMC);
    } else {
        ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BUS_WIDTH, 0x6, EXT_CSD__S_CMD_SET__STD_MMC);
    }
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*Swith to HS400 then*/
    drv = 0;
    ext_csd_hs_timing = (drv << 4) | 0x3;
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__HS_TIMING, ext_csd_hs_timing, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*Config PHY relate register for HS400*/
    //set mmc_hs400 mode enable
    if (host->strobe) {
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data |= (1 << 9) | (1 << 8) | (1 << 12); /*MMC HS400 MODE | ENHANCE_STROBE_EN | MMC CARD mode*/
        data &= ~(1 << 10); /*clear HS200*/
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);
    } else {
        data = sdhciReadSdhc_mmc_ctrl_reg(host);
        data |= (1 << 9) | (1 << 10) | (1 << 12); /*MMC HS400 MODE | MMC HS200 MODE | MMC CARD mode*/
        data &= ~(1 << 8); /*ENHANCE_STROBE_EN*/
        sdhciWriteSdhc_mmc_ctrl_reg(host, data);
    }

    sdhci_set_bus_clk(host, F_HS200);
    sdhci_set_bus_timing(host, HS400);

    return ret;
}

int mmc_init_mmc_card(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    /*Determine if this card if a High Capacity card*/
    host->card->high_capacity = (host->card->ocr & ACCESS_MODE) == ACCESS_MODE__SECTOR_MODE;
    if (host->card->high_capacity) {
        host->card_info->cardtype = MMC_HIGH_CAPACITY_CARD;
        HAL_EMMC_TRACE(1, "MMC High Capacity Card!");
    } else {
        host->card_info->cardtype = MMC_LOW_CAPACITY_CARD;
    }
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR);

    /*mmc_all_send_cid*/
    ret = mmc_send_cmd_wait_resp(host, ALL_SEND_CID, 0, MMC_RSP_R2, CARD_RESPONSE_R2, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    HAL_EMMC_TRACE(1, "CID: 0x%08X 0x%08X 0x%08X 0x%08X", host->card->raw_data[0], host->card->raw_data[1], host->card->raw_data[2], host->card->raw_data[3]);

    /*Enable interrupts (RCA response has valid cmd indx and cmd crc)*/
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    /*mmc_set_rca*/
    host->card->rca = host->card->cid.psn & MMC_RCA_MASK;
    if (!host->card->rca) {
        host->card->rca = MMC_RCA_VALUE_DEFAULT;
    }

    ret = mmc_send_cmd_wait_resp(host, SET_RELATIVE_ADDR, (host->card->rca << 16), MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    host->card_info->relcardadd = host->card->rca;
    HAL_EMMC_TRACE(1, "RCA: 0x%08x", host->card->rca);

    /*Check for card STAND-BY state*/
    ret = mmc_check_card_status(host, CARD_STBY_STATE, 0);
    if (ret) {
        HAL_EMMC_ERROR(0, "Card is NOT in STAND-BY mode!!");
        return ret;
    }

    /*Disable interrupts (CSD response has cmd indx all set to 1's)*/
    sdhci_mask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    /*mmc_send_csd*/
    ret = mmc_send_cmd_wait_resp(host, SEND_CSD, (host->card->rca << 16), MMC_RSP_R2, CARD_RESPONSE_R2, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    host->card_info->cardclass = host->card->csd.ccc;
    memcpy(&host->card_info->cid, &host->card->cid, sizeof(struct mmc_cid));
    memcpy(&host->card_info->csd, &host->card->csd, sizeof(struct mmc_csd));
    host->card->read_bl_len = 1 << (host->card->csd.read_bl_len & 0xf);
    if (host->card->read_bl_len > CARD_BLOCK_LENGTH) {
        host->card->read_bl_len = CARD_BLOCK_LENGTH;
    }

    if (host->card->type == CARD_SD) {
        host->card->write_bl_len = host->card->read_bl_len ;
    } else {
        host->card->write_bl_len = 1 << (host->card->csd.write_bl_len & 0xf);
    }
    if (host->card->write_bl_len > CARD_BLOCK_LENGTH) {
        host->card->write_bl_len = CARD_BLOCK_LENGTH;
    }
    HAL_EMMC_TRACE(0, "read_bl_len: %d, write_bl_len: %d", host->card->read_bl_len, host->card->write_bl_len);
    HAL_EMMC_TRACE(0, "CSD: 0x%08X 0x%08X 0x%08X 0x%08X", host->card->raw_data[0], host->card->raw_data[1], host->card->raw_data[2], host->card->raw_data[3]);

    /*Enable interrupts (RCA response has valid cmd indx and cmd crc)*/
    sdhci_unmask_irqs(host, IRQ_CMD_CRC_ERR | IRQ_CMD_INDEX_ERR);

    /*Set bus rate to 26MHZ*/
    sdhci_set_bus_clk(host, F_DS);

    /*mmc_select_card*/
    ret = mmc_send_cmd_wait_resp(host, SELECT_CARD, (host->card->rca << 16), MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*Check device is locked*/
    if (host->card->r1 & (1 << 25)) {
        HAL_EMMC_TRACE(2, "***%s:%d, device is locked", __FUNCTION__, __LINE__);

        /*unlocked device*/
        ret = mmc_send_cmd_wait_resp(host, LOCK_UNLOCK, 0, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
        if (ret != MMC_NO_ERR) {
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
    sdhci_unmask_irqs(host, EMMC_IRQ_ERR_MASK | IRQ_TUNE_ERR);//All error interrupts are turned on here

    ret = mmc_send_ext_csd(host);
    if (ret != MMC_NO_ERR) {
        HAL_EMMC_ERROR(0, "Read EXT_CSD error:%d", ret);
        return ret;
    }

    host->card_info->ext_csd_rev = host->card->ext_csd[192];
    host->card_info->firmware_version_h32 = (unsigned int)(*(unsigned int *)&host->card->ext_csd[258]);
    host->card_info->firmware_version = (unsigned int)(*(unsigned int *)&host->card->ext_csd[254]);
    host->card_info->device_version = (unsigned int)(*(unsigned short *)&host->card->ext_csd[262]);
    host->card_info->cache_size = (unsigned int)(*(unsigned int *)&host->card->ext_csd[249]);
    host->card_info->cache_size *= (1024 / 8);//convert to bytes
    host->card_info->cache_ctrl = 0;
    host->card_info->cache_flush_policy = host->card->ext_csd[240];
    host->card_info->generic_cmd6_time = DEFAULT_CMD6_TIMEOUT;
    if (host->card_info->ext_csd_rev >= 6) {/* eMMC v4.5 or later */
        host->card_info->generic_cmd6_time = 10 * host->card->ext_csd[248];
    }
    if (host->card->high_capacity == MMC_LOW_CAPACITY_CARD) {
        host->card_info->blocknbr = host->card->csd.c_size + 1U;
        host->card_info->blocknbr *= (1 << ((host->card->csd.c_size_mult & 0x07U) + 2U));
        host->card_info->blocksize = (1 << (host->card->csd.read_bl_len & 0x0FU));
        host->card_info->logblocknbr = host->card_info->blocknbr * (host->card_info->blocksize / 512U);
        host->card_info->logllocksize = CARD_BLOCK_LENGTH;
    } else {
        host->card_info->blocknbr = (unsigned int)(*(unsigned int *)&host->card->ext_csd[212]);
        host->card_info->blocksize = CARD_BLOCK_LENGTH;
        host->card_info->logblocknbr = host->card_info->blocknbr;
        host->card_info->logllocksize = host->card_info->blocksize;
    }

    if (host->card->ext_csd[EXT_CSD__ERASE_GROUP_DEF] & 0x01) {
        /* Read out group size from ext_csd */
        host->card->erase_grp_size_card =
            host->card->ext_csd[EXT_CSD__HC_ERASE_GRP_SIZE] * 1024;
    } else {
        /* Calculate the group size from the csd value. */
        host->card->erase_grp_size_card = (host->card->csd.erase_grp_size + 1)
                                          * (host->card->csd.erase_grp_mult + 1);
    }
    host->card->can_trim = !!(host->card->ext_csd[EXT_CSD__SEC_FEATURE_SUPPORT] & (1 << 4));
    HAL_EMMC_TRACE(0, "card erase_grp_size:%d, can trim:%d", host->card->erase_grp_size_card, host->card->can_trim);

    /*mmc_set_blk_len*/
    ret = mmc_send_cmd_wait_resp(host, SET_BLOCKLEN, CARD_BLOCK_LENGTH, MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    host->card->timing = DS;
    host->card->width = MMC_1BIT_WIDTH;
    return ret;
}

int mmc_config_mmc_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;
    unsigned int ext_csd_hs_timing = 0;
    unsigned int ext_csd_bus_width = 0;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    /*Bus timing*/
    switch (timing) {
        case EHS400:
        case HS400:
            return mmc_set_hs400(host, MMC_IO_VOLTAGE_1P8, MMC_DRV_50OHM_X1);
            break;
        case HS200:
        case HS100:
        case HS50:
        case HS25:
            return mmc_set_hs200(host, width, MMC_IO_VOLTAGE_1P8, MMC_DRV_50OHM_X1);
            break;
        case DDR50:
        case DDR26:
        case DDR52:
            ext_csd_hs_timing = 0x1;
            break;
        case SDR50:
        case SDR52:
            ext_csd_hs_timing = 0x1;
            break;
        case DS:
            ext_csd_hs_timing = 0x0;
            break;
        default:
#ifdef EMMC_ASSERT
            HAL_EMMC_ASSERT(false, "%s:%d, Unsupported timing mode", __FUNCTION__, __LINE__);
#else
            HAL_EMMC_ERROR(0, "%s:%d, Unsupported timing mode", __FUNCTION__, __LINE__);
            return MMC_INVALID_ERR;
#endif
            break;
    }
    HAL_EMMC_TRACE(1, "Bus timing %d selected (EXT_CSD__HS_TIMING = 0x%0x)", timing, ext_csd_hs_timing);
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__HS_TIMING, ext_csd_hs_timing, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    /*Bus width*/
    switch (width) {
        case MMC_8BIT_WIDTH:
            if (timing == DDR26 || timing == DDR52 || timing == DDR50 || timing == HS400 || timing == EHS400) {
                ext_csd_bus_width = 6;
            } else {
                ext_csd_bus_width = 2;
            }
            break;
        case MMC_4BIT_WIDTH:
            if (timing == DDR26 || timing == DDR52 || timing == DDR50 || timing == HS400 || timing == EHS400) {
                ext_csd_bus_width = 5;
            } else {
                ext_csd_bus_width = 1;
            }
            break;
        case MMC_1BIT_WIDTH:
            if (timing == DDR26 || timing == DDR52 || timing == DDR50 || timing == HS25 || timing == HS50 || \
                timing == HS100 || timing == HS200 || timing == HS400 || timing == EHS400) {
                HAL_EMMC_ERROR(1, "EHS400/HS400/HS200/HS100/HS50/HS25/DDR26/DDR52/DDR50 does not support 1 bit width");
                return MMC_INVALID_ERR;
            } else {
                ext_csd_bus_width = 0;
            }
            break;
    }

    HAL_EMMC_TRACE(1, "Bus width %d selected (EXT_CSD__BUS_WIDTH = 0x%0x)", width * 2, ext_csd_bus_width);
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BUS_WIDTH, ext_csd_bus_width, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }

    sdhci_set_bus_timing(host, timing);
    sdhci_set_bus_width(host, width);
    sdhci_set_bus_clk(host, khz);

    //tuning cmd line, data line cannot be calibrated in this mode
    sdhci_tuning_cmdline(host);

    return ret;
}

int mmc_alter_boot(struct mmc_host *host)
{
    return mmc_send_cmd_wait_resp(host, GO_PRE_IDLE_STATE, 0xFFFFFFA, MMC_RSP_NONE, CARD_RESPONSE_NONE, 0, 0);
}

int mmc_put_to_reset(struct mmc_host *host)
{
    return mmc_send_cmd_wait_resp(host, GO_PRE_IDLE_STATE, 0, MMC_RSP_NONE, CARD_RESPONSE_NONE, 0, 0);
}

int mmc_boot_config(struct mmc_host *host, struct emmc_boot_test_args *args)
{
    int ret = MMC_NO_ERR;
    unsigned int ext_boot_config;
    unsigned int ext_boot_bus_condition;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    ext_boot_config = 0;
    if (args->boot_ack == EMMC_BOOT_ACK_YES) {
        ext_boot_config |= (1 << 6);
    }
    switch (args->parti) {
        case EMMC_BOOT_PARTI_1:
            ext_boot_config |= (1 << 3);
            ext_boot_config |= (1 << 0);
            break;
        case EMMC_BOOT_PARTI_2:
            ext_boot_config |= (2 << 3);
            ext_boot_config |= (2 << 0);
            break;
        case EMMC_BOOT_USR_PARTI:
            ext_boot_config |= (7 << 3);
            break;
    }
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__PARTITION_CONFIG, ext_boot_config, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    ext_boot_bus_condition = 0;
    if (args->boot_condition == EMMC_BOOT_RETAIN) {
        ext_boot_bus_condition = (1 << 2);
    }
    switch (args->boot_mode) {
        case EMMC_BOOT_DS:
            ext_boot_bus_condition |= (0 << 3);
            break;
        case EMMC_BOOT_HS:
            ext_boot_bus_condition |= (1 << 3);
            break;
        case EMMC_BOOT_DDR:
            ext_boot_bus_condition |= (2 << 3);
            break;
    }
    switch (args->boot_wdith) {
        case EMMC_BOOT_1BIT:
            ext_boot_bus_condition |= (0 << 0);
            break;
        case EMMC_BOOT_4BIT:
            ext_boot_bus_condition |= (1 << 0);
            break;
        case EMMC_BOOT_8BIT:
            ext_boot_bus_condition |= (2 << 0);
            break;
    }
    ret = mmc_switch(host, CMD6_WR_BYTE, EXT_CSD__BOOT_BUS_WIDTH, ext_boot_bus_condition, EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret != MMC_NO_ERR) {
        return ret;
    }
    return ret;
}

int mmc_select_cards(struct mmc_host *host)
{
    return mmc_send_cmd_wait_resp(host, SELECT_CARD, (host->card->rca << 16), MMC_RSP_R1, CARD_RESPONSE_R1, 0, 0);
}

int mmc_deselect_cards(struct mmc_host *host)
{
    return mmc_send_cmd_wait_resp(host, DESELECT_CARD, 0, MMC_RSP_NONE, CARD_RESPONSE_NONE, 0, 0);
}

static int mmc_can_sleep(struct mmc_host *host)
{
    return (host && host->card->ext_csd[192] >= 3);
}

#define S_A_TIMEOUT_INDEX_END   0x18
static const uint32_t s_a_timeout_us[S_A_TIMEOUT_INDEX_END] = {
    0, 1, 1, 1, 2, 4, 7, 13, 26, 52, 103, 205, 410, 820, 1639, 3277,
    6554, 13108, 26215, 52429, 104858, 209716, 419431, 838861
};

int mmc_sleep(struct mmc_host *host)
{
    int ret = MMC_INVALID_ERR;
    uint16_t status;
    uint32_t timeout, busy_t;
    uint8_t retries = EMMC_ERROR_RETRY_CNT;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (mmc_can_sleep(host)) {
        //deselect card
        ret = mmc_deselect_cards(host);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s, deselect card error", __FUNCTION__);
            return ret;
        }

        //send sleep cmd
        do {
            ret = mmc_send_cmmand(host, SLEEP, (host->card->rca << 16) | (1 << 15), MMC_RSP_R1B);
        } while (ret && retries--);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s, send cmd error", __FUNCTION__);
            return ret;
        }

        //wait for resp
        ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1B);
        if (ret != MMC_NO_ERR) {
            HAL_EMMC_ERROR(0, "%s, wait for resp error", __FUNCTION__);
            return ret;
        }

        //wait for the sleep state is reached
        if (host->card->ext_csd[217] && host->card->ext_csd[217] < S_A_TIMEOUT_INDEX_END) {
            timeout = US_TO_FAST_TICKS(s_a_timeout_us[host->card->ext_csd[217]]);
            busy_t = hal_fast_sys_timer_get();
            while (hal_fast_sys_timer_get() - busy_t < timeout) {
                status = sdhciReadSdh_present_state_2Dat_level(host);
                if (status == 0xF) {
                    host->card->has_sleep = 1;
                    return ret;
                } else {
                    host->delay(1);
                }
            }
            HAL_EMMC_ERROR(0, "%s, wait for sleep timeout", __FUNCTION__);
            return MMC_SLEEP_TIMEOUT;
        } else {
            HAL_EMMC_ERROR(0, "%s, ext_csd[217] error", __FUNCTION__);
            while (sdhciReadSdh_present_state_2Dat_level(host) != 0xF);
        }
    } else {
        ret = MMC_NOT_SUPPORT;
        HAL_EMMC_ERROR(0, "%s, not support", __FUNCTION__);
    }

    return ret;
}

int mmc_awake(struct mmc_host *host)
{
    int ret = MMC_INVALID_ERR;
    uint16_t status;
    uint32_t timeout, busy_t;
    int8_t cnt = EMMC_ERROR_RETRY_CNT;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (mmc_can_sleep(host)) {
        //send awake cmd
        do {
            ret = mmc_send_cmmand(host, AWAKE, (host->card->rca << 16), MMC_RSP_R1B);
            if (ret == MMC_NO_ERR) {
                ret = mmc_wait_for_resp(host, CARD_RESPONSE_R1B);
                if (ret != MMC_NO_ERR) {
                    HAL_EMMC_ERROR(0, "%s, wait for resp error:%d, retry:%d", __FUNCTION__, ret, EMMC_ERROR_RETRY_CNT - cnt);
                }
            } else {
                HAL_EMMC_ERROR(0, "%s, send cmd error:%d", __FUNCTION__, ret);
            }
        } while (ret && cnt--);

        if (ret == MMC_NO_ERR) {
            //wait for the standby state is reached
            if (host->card->ext_csd[217] && host->card->ext_csd[217] < S_A_TIMEOUT_INDEX_END) {
                timeout = US_TO_FAST_TICKS(s_a_timeout_us[host->card->ext_csd[217]]);
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
                        HAL_EMMC_ERROR(0, "%s, wait for awake timeout", __FUNCTION__);
                        return MMC_AWAKE_TIMEOUT;
                    }
                }
            } else {
                HAL_EMMC_ERROR(0, "%s, ext_csd[217] error", __FUNCTION__);
                while (sdhciReadSdh_present_state_2Dat_level(host) != 0xF);
            }

            //select card:standby state to transfer state
            cnt = EMMC_ERROR_RETRY_CNT;
            do {
                ret = mmc_select_cards(host);
            } while (ret && cnt--);
            if (ret) {
                HAL_EMMC_ERROR(0, "%s, mmc select cards error:%d", __FUNCTION__, ret);
            } else {
                host->card->has_sleep = 0;
            }
        } else {
            HAL_EMMC_ERROR(0, "%s, awake cards error:%d", __FUNCTION__, ret);
        }
    } else {
        ret = MMC_NOT_SUPPORT;
        HAL_EMMC_ERROR(0, "%s, not support", __FUNCTION__);
    }

    return ret;
}

int mmc_gen_cmd(struct mmc_host *host, uint32_t arg, uint8_t *buf, uint16_t len, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    //0: write, 1: read
    return (arg & 0x01) ? mmc_read_card(host, arg, buf, len, blocking_mode, dma_en, GEN_CMD) : mmc_write_card(host, arg, buf, len, blocking_mode, dma_en, GEN_CMD);
}

int mmc_cmd60(struct mmc_host *host, uint32_t arg)
{
    int ret;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host->in_use) {
        ret = MMC_IN_USE;
        host->error_code = ret;
        return ret;
    } else {
        host->in_use = 1;
    }

    ret = mmc_send_cmd_wait_resp(host, CMD60, arg, MMC_RSP_NONE, CARD_RESPONSE_NONE, 0, 0);
    host->in_use = 0;
    return ret;
}

#define MIN_CACHE_EN_TIMEOUT_MS     1600
#define CACHE_FLUSH_TIMEOUT_MS      30000
int mmc_cache_control(struct mmc_host *host, uint8_t enable_disable)
{
    int ret = MMC_NO_ERR;
    unsigned int timeout_ms = MIN_CACHE_EN_TIMEOUT_MS;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    /*
     * If cache size is higher than 0, this indicates the existence of cache
     * and it can be turned on. Note that some eMMCs from Micron has been
     * reported to need ~800 ms timeout, while enabling the cache after
     * sudden power failure tests. Let's extend the timeout to a minimum of
     * MIN_CACHE_EN_TIMEOUT_MS and do it for all cards.
     */
    if (host && (host->card_info->cache_size > 0)) {
        enable_disable = !!enable_disable;

        if (host->card_info->cache_ctrl ^ enable_disable) {
            timeout_ms = host->card_info->generic_cmd6_time > timeout_ms ? host->card_info->generic_cmd6_time : timeout_ms;
            ret = mmc_switch_ext(host, CMD6_SET_BITS, EXT_CSD__CACHE_CTRL, enable_disable,
                                 EXT_CSD__S_CMD_SET__STD_MMC, timeout_ms);
            if (ret) {
                HAL_EMMC_ERROR(1, "%s, emmc cache %s error %d", __FUNCTION__, enable_disable ? "on" : "off", ret);
            } else {
                host->card_info->cache_ctrl = enable_disable;
            }
        }
    }

    return ret;
}

static bool mmc_cache_enabled(struct mmc_host *host)
{
    return host->card_info->cache_size > 0 &&
           host->card_info->cache_ctrl & 1;
}

//Flush the internal cache of the eMMC to non-volatile storage
int mmc_cache_flush(struct mmc_host *host)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (mmc_cache_enabled(host)) {
        ret = mmc_switch_ext(host, CMD6_SET_BITS,
                             EXT_CSD__FLUSH_CACHE, 1,
                             EXT_CSD__S_CMD_SET__STD_MMC,
                             CACHE_FLUSH_TIMEOUT_MS);
        if (ret) {
            HAL_EMMC_ERROR(1, "%s, emmc cache flush error %d", __FUNCTION__, ret);
        }
    }

    return ret;
}

int mmc_rst_n_mode_set(struct mmc_host *host, enum EMMC_RST_N_MODE mode)
{
    int ret;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    ret = mmc_switch(host, CMD6_WR_BYTE,
                     EXT_CSD__RST_n_FUNCTION, mode,
                     EXT_CSD__S_CMD_SET__STD_MMC);
    if (ret) {
        HAL_EMMC_ERROR(1, "%s error %d", __FUNCTION__, ret);
    }

    return ret;
}

int mmc_power_off_notification_enable(struct mmc_host *host)
{
    int ret = MMC_NOT_SUPPORT;

    HAL_EMMC_TRACE(0, "%s", __FUNCTION__);

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (host->card_info->ext_csd_rev >= 6) {/* eMMC v4.5 or later */
        ret = mmc_switch(host, CMD6_WR_BYTE,
                         EXT_CSD__PWR_OFF_NOTIFY, 1,
                         EXT_CSD__S_CMD_SET__STD_MMC);
        if (ret) {
            HAL_EMMC_ERROR(1, "%s error %d", __FUNCTION__, ret);
        }
    }

    return ret;
}

#define SLEEP_NOTIFICATION_TIME_INDEX_END   0x18
static const uint32_t sleep_notification_time_us[SLEEP_NOTIFICATION_TIME_INDEX_END] = {
    0, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 20480, 40960, 81920,
    163840, 327680, 655360, 1310720, 2621440, 5242880, 10485760, 20971520,
    41943040, 83886080,
};

int mmc_power_off_notification_config(struct mmc_host *host, enum EMMC_POWER_OFF_MODE mode)
{
    int ret;
    uint32_t timeout_ms = DEFAULT_CMD6_TIMEOUT;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    if (mode == EMMC_POWER_OFF_LONG) {
        if (host->card->ext_csd[EXT_CSD__POWER_OFF_LONG_TIME]) {
            timeout_ms = host->card->ext_csd[EXT_CSD__POWER_OFF_LONG_TIME] * 10;
        }
    } else if (mode == EMMC_SLEEP_NOTIFICATION) {
        if (host->card->ext_csd[EXT_CSD__SLEEP_NOTIFY_TIME] &&
            host->card->ext_csd[EXT_CSD__SLEEP_NOTIFY_TIME] < SLEEP_NOTIFICATION_TIME_INDEX_END) {
            timeout_ms = sleep_notification_time_us[host->card->ext_csd[EXT_CSD__SLEEP_NOTIFY_TIME]] / 1000;
        }
    } else {
        if (host->card_info->generic_cmd6_time) {
            timeout_ms = host->card_info->generic_cmd6_time;
        }
    }

    HAL_EMMC_TRACE(0, "%s, mode %d, time:%dms", __FUNCTION__, mode, timeout_ms);
    ret = mmc_switch_ext(host, CMD6_WR_BYTE,
                         EXT_CSD__PWR_OFF_NOTIFY, mode,
                         EXT_CSD__S_CMD_SET__STD_MMC, timeout_ms);
    if (ret) {
        HAL_EMMC_ERROR(1, "%s error %d", __FUNCTION__, ret);
    }

    return ret;
}

unsigned int mmc_card_sleep_status(struct mmc_host *host)
{
    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    return host->card->has_sleep;
}

int mmc_send_cmd_ext(struct mmc_host *host, struct mmc_cmd_ext *cmd, struct mmc_data_ext *data, enum BLOCKING_MODE blocking_mode, bool dma_en)
{
    int ret = MMC_NO_ERR;
    unsigned int size;
    unsigned int response;
    unsigned int flags = 0;
    unsigned int card_status;

    HAL_EMMC_TRACE(0, "%s:%d", __FUNCTION__, __LINE__);
    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }
    if (!cmd) {
        HAL_EMMC_ERROR(0, "%s: cmd is NULL", __FUNCTION__);
        return MMC_INVALID_ERR;
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

    if (host->speed_mode == HS400 && data) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        //In this mode, the cpu runs at least 200M
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_208M);
#endif
        if (host->strobe && data->direction == MMC_CARD_READ) {
            sdhci_host_emphy_clk_sel_ds(host);
        }
    }

    if (data) {
        flags = MMC_DATA_PRESENT;
        if (dma_en) {
            flags |= MMC_DATA_DMA;
        }
        host->dma_status = dma_en;
        mmc_set_tran_mode(host, blocking_mode);

        if (data->blocks > 1) {
            flags |= MMC_DATA_MB | MMC_DATA_BC_EN;
            switch (host->auto_cmd_type) {
                case MM4_AUTO_CMD23:
                    flags |= MMC_DATA_AUTOCMD23;
                    ret = mmc_set_auto_cmd23(host, data->blocks, dma_en);
                    if (ret != MMC_NO_ERR) {
                        goto sendcmd_out;
                    }
                    break;
                case MM4_AUTO_CMD12://Open-ended Multiple block read mode
                    flags |= MMC_DATA_AUTOCMD12;
                    break;
                case MM4_MANUAL_CMD23:
                    ret = mmc_set_blk_cnt(host, data->blocks);
                    if (ret != MMC_NO_ERR) {
                        goto sendcmd_out;
                    }
                    break;
                case MM4_MANUAL_CMD12://Open-ended Multiple block read mode
                    break;
            }
        }
        size = data->blocks * data->blocksize;
        if (data->direction == MMC_CARD_READ) {
            flags |= MMC_DATA_READ;
            mmc_prepare_transfer(host, (void *)data->dest, data->direction, data->blocksize, size, dma_en);
        } else {
            flags |= MMC_DATA_WRITE;
            mmc_prepare_transfer(host, (void *)data->src, data->direction, data->blocksize, size, dma_en);
        }
    }

    switch (cmd->resp_type) {
        case CARD_RESPONSE_R1:
            response = MMC_RSP_R1;
            break;
        case CARD_RESPONSE_R1B:
            response = MMC_RSP_R1B;
            break;
        case CARD_RESPONSE_R2:
            response = MMC_RSP_R2;
            break;
        case CARD_RESPONSE_R3:
            response = MMC_RSP_R3;
            break;
        case CARD_RESPONSE_R4:
            response = MMC_RSP_R4;
            break;
        case CARD_RESPONSE_R5:
        case CARD_RESPONSE_R5B:
            response = MMC_RSP_R5;
            break;
        case CARD_RESPONSE_R6:
            response = MMC_RSP_R6;
            break;
        case CARD_RESPONSE_R7:
            response = MMC_RSP_R7;
            break;
        default:
            response = MMC_RSP_NONE;
            break;
    }
    mmc_send_cmmand(host, cmd->cmdidx, cmd->cmdarg, response | flags);
    ret = mmc_wait_for_resp(host, cmd->resp_type);
    if (ret != MMC_NO_ERR) {
        goto sendcmd_out;
    }

    if (cmd->resp_type == CARD_RESPONSE_R2) {
        cmd->resp[0] = host->cmd->resp[3];//high
        cmd->resp[1] = host->cmd->resp[2];
        cmd->resp[2] = host->cmd->resp[1];
        cmd->resp[3] = host->cmd->resp[0];//low
    } else {
        cmd->resp[0] = host->cmd->resp[0];
    }

    if (data) {
        if (blocking_mode) {
            ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
            if (ret != MMC_NO_ERR) {
                goto sendcmd_out;
            }
        } else {
            if (host->dma_status && host->emmc_host_dma_done) {
                ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, blocking_mode);
                return MMC_NO_ERR;
            } else {
                ret = mmc_wait_for_xfer(host, XFER_TIMEOUT, BLOCKING_MODE_IRQ_ON);
                if (ret != MMC_NO_ERR) {
                    goto sendcmd_out;
                }
            }
        }

        if (data->blocks > 1) {
            switch (host->auto_cmd_type) {
                case MM4_AUTO_CMD23:
                case MM4_AUTO_CMD12:
                case MM4_MANUAL_CMD23:
                    /*do nothing*/
                    break;
                case MM4_MANUAL_CMD12:
                    ret = mmc_send_stop(host, blocking_mode);
                    if (ret != MMC_NO_ERR) {
                        goto sendcmd_out;
                    }
                    break;
                default:
                    HAL_EMMC_TRACE(0, "%s, Unknown auto cmd type", __FUNCTION__);
                    ret = MMC_INVALID_ERR;
                    goto sendcmd_out;
                    break;
            }
        }
    }

sendcmd_out:
    host->tran_status = 0;
    if (host->speed_mode == HS400 && data) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
        hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
#endif
        if (host->strobe && data->direction == MMC_CARD_READ) {
            sdhci_host_emphy_clk_sel_pck_delay(host);//revert back to the original clk
        }
    }

    return ret;
}

static int emmc_set_ios_hs200(struct mmc_host *host, unsigned width)
{
    int ret = MMC_NO_ERR;
    unsigned int data;

    /*Bus width*/
    sdhci_set_bus_width(host, width);

    /*Switch to HS200*/
    sdhci_set_bus_timing(host, HS200);

    /*Config PHY relate register for HS200*/
    //tx config
    sdhci_sel_base_clk(host);/* Select clock from the original inverter of base clock or ddll output clock*/

    //set mmc_hs200 mode enable
    data = sdhciReadSdhc_mmc_ctrl_reg(host);
    data |= (1 << 10) | (1 << 12); /*MMC HS200 MODE | MMC CARD mode*/
    data &= ~((1 << 9) | (1 << 8)); /*clear MMC HS400 MODE |ENHANCE_STROBE_EN*/
    sdhciWriteSdhc_mmc_ctrl_reg(host, data);

    /*Raise the freq to 200MHZ*/
    sdhci_set_bus_clk(host, F_HS200);
#ifdef EMMC_DEBUG
    HAL_EMMC_TRACE(0, "------hs200 tuning start");
    data = hal_fast_sys_timer_get();
#endif
    ret = sdhci_try_tuning(host, MMC_SEND_TUNING_BLOCK);
#ifdef EMMC_DEBUG
    data = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - data);
    HAL_EMMC_TRACE(1, "------hs200 tuning end, %dus", data);
#endif
    return ret;
}

static int emmc_set_ios(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;

    HAL_EMMC_TRACE(1, "%s:%d", __FUNCTION__, __LINE__);
    switch (timing) {
        case HS400:
            return -20230217;
            break;
        case HS200:
            return emmc_set_ios_hs200(host, width);
            break;
        default:
            sdhci_set_bus_timing(host, timing);
            if (width) {
                sdhci_set_bus_width(host, width);
            }
            if (khz) {
                sdhci_set_bus_clk(host, khz);
            }
            break;
    }
    return ret;
}

int mmc_config_ios(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width)
{
    int ret = MMC_NO_ERR;

    if (host == NULL) {
        HAL_EMMC_ERROR(0, "%s, NULL poiter", __FUNCTION__);
        return MMC_ERR_NULL_POINTER;
    }

    HAL_EMMC_TRACE(1, "%s:timing=%d,khz=%d,width=%d", __FUNCTION__, timing, khz, width);
    ret = emmc_set_ios(host, timing, khz, width);
    if (ret) {
        return ret;
    } else {
        host->card->timing = timing;
        host->card->width = width;
        host->error_code = ret;
    }

    return ret;
}
