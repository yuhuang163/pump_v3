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
#include "card.h"
#include "core.h"
#include "error.h"
#include "string.h"
#include "card_mmc.h"
#include "card_sdio.h"
#include "emmc_dbg.h"
#include "sdhci_emphy.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "emmc_platform.h"
#include "sdhci_unifiedAl.h"
#ifdef EMMC_EXTENDED_USE
    #include "hal_dma.h"
#endif

static struct mmc_cmd  g_mmc_cmd;
static struct mmc_data g_mmc_data;

#define mmc_unpack_bits(resp,start,size)                                    \
    ({                                                                      \
        const          int __size = size;                                   \
        const unsigned int __mask = (__size < 32 ? 1 << __size : 0) - 1;    \
        const          int __off = 3 - ((start) / 32);                      \
        const          int __shft = (start) & 31;                           \
        unsigned int   __res;                                               \
                                                                            \
        __res = resp[__off] >> __shft;                                      \
        if (__size + __shft > 32)                                           \
            __res |= resp[__off-1] << ((32 - __shft) % 32);                 \
        __res & __mask;                                                     \
    })

void mmc_unpack_cid(struct mmc_host *host, unsigned int *resp)
{
    switch (host->card->type) {
        case CARD_MMC:
            host->card->cid.mid    = mmc_unpack_bits(resp, 112,  8);
            host->card->cid.cbx    = mmc_unpack_bits(resp, 104,  8);
            host->card->cid.oid    = mmc_unpack_bits(resp,  96,  8);
            host->card->cid.pnm[0] = mmc_unpack_bits(resp,  88,  8);
            host->card->cid.pnm[1] = mmc_unpack_bits(resp,  80,  8);
            host->card->cid.pnm[2] = mmc_unpack_bits(resp,  72,  8);
            host->card->cid.pnm[3] = mmc_unpack_bits(resp,  64,  8);
            host->card->cid.pnm[4] = mmc_unpack_bits(resp,  56,  8);
            host->card->cid.pnm[5] = mmc_unpack_bits(resp,  48,  8);
            host->card->cid.prv    = mmc_unpack_bits(resp,  40,  8);
            host->card->cid.psn    = mmc_unpack_bits(resp,   8, 32);
            host->card->cid.mdt    = mmc_unpack_bits(resp,   0,  8);
            break;
        case CARD_SD:
            host->card->cid.mid    = mmc_unpack_bits(resp, 112,  8);
            host->card->cid.oid    = mmc_unpack_bits(resp,  96, 16);
            host->card->cid.pnm[0] = mmc_unpack_bits(resp,  88,  8);
            host->card->cid.pnm[1] = mmc_unpack_bits(resp,  80,  8);
            host->card->cid.pnm[2] = mmc_unpack_bits(resp,  72,  8);
            host->card->cid.pnm[3] = mmc_unpack_bits(resp,  64,  8);
            host->card->cid.pnm[4] = mmc_unpack_bits(resp,  56,  8);
            host->card->cid.prv    = mmc_unpack_bits(resp,  48,  8);
            host->card->cid.psn    = mmc_unpack_bits(resp,  16, 32);
            host->card->cid.mdt    = mmc_unpack_bits(resp,   0, 12);
            break;
        case CARD_SDIO:
        case CARD_CEATA:
        default:
            break;
    }

    HAL_EMMC_TRACE(1, "Manufacturer ID       : %d", host->card->cid.mid);
    if (host->card->type == CARD_MMC) {
        HAL_EMMC_TRACE(1, "OEM/Application ID    : 0x%X", host->card->cid.oid & 0xFF);
        HAL_EMMC_TRACE(1, "Device/BGA            : 0x%X", host->card->cid.cbx);
        HAL_EMMC_TRACE(1, "Product Name          : %c%c%c%c%c%c",  host->card->cid.pnm[0], \
                                                                   host->card->cid.pnm[1], \
                                                                   host->card->cid.pnm[2], \
                                                                   host->card->cid.pnm[3], \
                                                                   host->card->cid.pnm[4], \
                                                                   (host->card->cid.pnm[5] & 0x7E) < 0x20 ? ' ' : (host->card->cid.pnm[5] & 0x7E));//character range
    } else if (host->card->type == CARD_SD) {
        HAL_EMMC_TRACE(1, "OEM/Application ID       : 0x%X", host->card->cid.oid);
        HAL_EMMC_TRACE(1, "Product Name             : %c%c%c%c%c",  host->card->cid.pnm[0], \
                                                                    host->card->cid.pnm[1], \
                                                                    host->card->cid.pnm[2], \
                                                                    host->card->cid.pnm[3], \
                                                                    (host->card->cid.pnm[4] & 0x7E) < 0x20 ? ' ' : (host->card->cid.pnm[4] & 0x7E));//character range
    }
    HAL_EMMC_TRACE(1, "Product Revision      : %d.%d", (host->card->cid.prv >> 4), (host->card->cid.prv & 0xF));
    HAL_EMMC_TRACE(1, "Product Serial No     : 0x%X", host->card->cid.psn);
    HAL_EMMC_TRACE(1, "Manufacture Date      : %d/%d", 2000 + (host->card->cid.mdt >> 4),  host->card->cid.mdt & 0xF);
}

void mmc_unpack_csd(struct mmc_host *host, unsigned int *resp)
{
    switch (host->card->type) {
        case CARD_MMC:
            host->card->csd.csd_structure      = mmc_unpack_bits(resp, 118,  2);
            host->card->csd.spec_vers          = mmc_unpack_bits(resp, 114,  4);
            host->card->csd.taac               = mmc_unpack_bits(resp, 104,  8);
            host->card->csd.nsac               = mmc_unpack_bits(resp,  96,  8);
            host->card->csd.tran_speed         = mmc_unpack_bits(resp,  88,  8);
            host->card->csd.ccc                = mmc_unpack_bits(resp,  76, 12);
            host->card->csd.read_bl_len        = mmc_unpack_bits(resp,  72,  4);
            host->card->csd.read_bl_partial    = mmc_unpack_bits(resp,  71,  1);
            host->card->csd.write_blk_misalign = mmc_unpack_bits(resp,  70,  1);
            host->card->csd.read_blk_misalign  = mmc_unpack_bits(resp,  69,  1);
            host->card->csd.dsr_imp            = mmc_unpack_bits(resp,  68,  1);
            host->card->csd.c_size             = mmc_unpack_bits(resp,  54, 12);
            host->card->csd.vdd_r_curr_min     = mmc_unpack_bits(resp,  51,  3);
            host->card->csd.vdd_r_curr_max     = mmc_unpack_bits(resp,  48,  3);
            host->card->csd.vdd_w_curr_min     = mmc_unpack_bits(resp,  45,  3);
            host->card->csd.vdd_w_curr_max     = mmc_unpack_bits(resp,  42,  3);
            host->card->csd.c_size_mult        = mmc_unpack_bits(resp,  39,  3);
            host->card->csd.erase_grp_size     = mmc_unpack_bits(resp,  34,  5);
            host->card->csd.erase_grp_mult     = mmc_unpack_bits(resp,  29,  5);
            host->card->csd.wp_grp_size        = mmc_unpack_bits(resp,  24,  5);
            host->card->csd.wp_grp_enable      = mmc_unpack_bits(resp,  23,  1);
            host->card->csd.default_ecc        = mmc_unpack_bits(resp,  21,  2);
            host->card->csd.r2w_factor         = mmc_unpack_bits(resp,  18,  3);
            host->card->csd.write_bl_len       = mmc_unpack_bits(resp,  14,  4);
            host->card->csd.write_bl_partial   = mmc_unpack_bits(resp,  13,  1);
            host->card->csd.content_prot_app   = mmc_unpack_bits(resp,   8,  1);
            host->card->csd.file_format_grp    = mmc_unpack_bits(resp,   7,  1);
            host->card->csd.copy               = mmc_unpack_bits(resp,   6,  1);
            host->card->csd.perm_write_protect = mmc_unpack_bits(resp,   5,  1);
            host->card->csd.tmp_write_protect  = mmc_unpack_bits(resp,   4,  1);
            host->card->csd.file_format        = mmc_unpack_bits(resp,   2,  2);
            host->card->csd.ecc                = mmc_unpack_bits(resp,   0,  2);
            break;
        case CARD_SD:
            host->card->csd.csd_structure      = mmc_unpack_bits(resp, 118,  2);
            host->card->csd.taac               = mmc_unpack_bits(resp, 104,  8);
            host->card->csd.nsac               = mmc_unpack_bits(resp,  96,  8);
            host->card->csd.tran_speed         = mmc_unpack_bits(resp,  88,  8);
            host->card->csd.ccc                = mmc_unpack_bits(resp,  76, 12);
            host->card->csd.read_bl_len        = mmc_unpack_bits(resp,  72,  4);
            host->card->csd.read_bl_partial    = mmc_unpack_bits(resp,  71,  1);
            host->card->csd.write_blk_misalign = mmc_unpack_bits(resp,  70,  1);
            host->card->csd.read_blk_misalign  = mmc_unpack_bits(resp,  69,  1);
            host->card->csd.dsr_imp            = mmc_unpack_bits(resp,  68,  1);
            if (host->card->csd.csd_structure == 0) {       //V1.0
                host->card->csd.c_size         = mmc_unpack_bits(resp,  54, 12);
                host->card->csd.vdd_r_curr_min = mmc_unpack_bits(resp,  51,  3);
                host->card->csd.vdd_r_curr_max = mmc_unpack_bits(resp,  48,  3);
                host->card->csd.vdd_w_curr_min = mmc_unpack_bits(resp,  45,  3);
                host->card->csd.vdd_w_curr_max = mmc_unpack_bits(resp,  42,  3);
                host->card->csd.c_size_mult    = mmc_unpack_bits(resp,  39,  3);
            } else if (host->card->csd.csd_structure == 1) {//V2.0
                host->card->csd.c_size         = mmc_unpack_bits(resp,  40, 22);//69~48
            } else if (host->card->csd.csd_structure == 2) {//V3.0
                host->card->csd.c_size         = mmc_unpack_bits(resp,  40, 28);//75~48
            }
            host->card->csd.erase_blk_en       = mmc_unpack_bits(resp,  38,  1);
            host->card->csd.sector_size        = mmc_unpack_bits(resp,  31,  7);
            host->card->csd.wp_grp_size        = mmc_unpack_bits(resp,  24,  7);
            host->card->csd.wp_grp_enable      = mmc_unpack_bits(resp,  23,  1);
            host->card->csd.r2w_factor         = mmc_unpack_bits(resp,  18,  3);
            host->card->csd.write_bl_len       = mmc_unpack_bits(resp,  14,  4);
            host->card->csd.write_bl_partial   = mmc_unpack_bits(resp,  13,  1);
            host->card->csd.file_format_grp    = mmc_unpack_bits(resp,   7,  1);
            host->card->csd.copy               = mmc_unpack_bits(resp,   6,  1);
            host->card->csd.perm_write_protect = mmc_unpack_bits(resp,   5,  1);
            host->card->csd.tmp_write_protect  = mmc_unpack_bits(resp,   4,  1);
            host->card->csd.file_format        = mmc_unpack_bits(resp,   2,  2);
            break;
        case CARD_SDIO:
        case CARD_CEATA:
        default:
            break;
    }
}

static void mmc_transfer_pio(struct mmc_host *host)
{
    unsigned int i, j;
    unsigned int xfer_bytes;
    xfer_bytes = host->data->sz;
    j = host->data->xferd_bytes;
    for (i = 0; ((i < host->data->blksz / 4) && (j < xfer_bytes / 4)); i++, j++) {
        if (host->data->direction == MMC_CARD_WRITE) {
            sdhciWriteSdh_dp_buffer(host, ((unsigned int *)(host->aligned_buf))[j]);
        }
        if (host->data->direction == MMC_CARD_READ) {
            ((unsigned int *)(host->aligned_buf))[j] = sdhciReadSdh_dp_buffer(host);
        }
    }
    host->data->xferd_bytes = j;
}

int mmc_dma_mode_write(struct mmc_host *host, uint8_t new_dma_mode)
{
    if (host) {
        host->dma_mode = new_dma_mode;
        return MMC_NO_ERR;
    }

    return MMC_INVALID_ERR;
}

int mmc_dma_mode_read(struct mmc_host *host, uint8_t *dma_mode)
{
    if (host) {
        *dma_mode = host->dma_mode;
        return MMC_NO_ERR;
    }

    return MMC_INVALID_ERR;
}

int mmc_desc_type_write(struct mmc_host *host, uint8_t new_desc_type)
{
    if (host) {
        host->desc_type = new_desc_type;
        return MMC_NO_ERR;
    }

    return MMC_INVALID_ERR;
}

int mmc_desc_type_read(struct mmc_host *host, uint8_t *desc_type)
{
    if (host) {
        *desc_type = host->desc_type;
        return MMC_NO_ERR;
    }

    return MMC_INVALID_ERR;
}

int mmc_adma_init_desc(struct mmc_host *host, const void *src_dst, uint32_t size, const void *next_src_dst)
{
    struct mmc_adma_desc *adma_desc;

    if (host->desc_type != ADMA2_DESC_SCATTER || !src_dst) {
        HAL_EMMC_ERROR(0, "%s, Input parameter error", __FUNCTION__);
        return MMC_INVALID_ERR;
    }

#ifdef EMMC_ASSERT
    HAL_EMMC_ASSERT(size <= host->desc_ctrl_max_len, "EMMC_ADMA_DESC_CNT value is too small");
    HAL_EMMC_ASSERT(host->desc_index < host->desc_cnt, "EMMC_ADMA_DESC_CNT has been used up");
    HAL_EMMC_ASSERT(host->desc, "host->desc can not be NULL");
#else
    if (size > host->desc_ctrl_max_len) {
        HAL_EMMC_ERROR(0, "%s, error:EMMC_ADMA_DESC_CNT value is too small", __FUNCTION__);
        return MMC_INVALID_ERR;
    }
    if (host->desc_index >= host->desc_cnt) {
        HAL_EMMC_ERROR(0, "%s, error:EMMC_ADMA_DESC_CNT has been used up", __FUNCTION__);
        return MMC_INVALID_ERR;
    }
    if (!host->desc) {
        HAL_EMMC_ERROR(0, "%s, error:host->desc can not be NULL", __FUNCTION__);
        return MMC_INVALID_ERR;
    }
#endif

    adma_desc = (struct mmc_adma_desc *)((uintptr_t)host->desc);
    //HAL_EMMC_TRACE(4, "---adma_desc val=0x%08X, src/dst addr=0x%08X, size=%d, max size=%d", (uint32_t)adma_desc,
    //               (uint32_t)src_dst, size, host->desc_ctrl_max_len);
    if (size <= ADMA2_DESC_MAX_LEN) {//one desc
        adma_desc[host->desc_index].adma_desc_ctrl  = (uint32_t)(size << 16);
        adma_desc[host->desc_index].adma_desc_ctrl |= ADMA2_ATTRIBUTE_ACT_TRAN | ADMA2_ATTRIBUTE_VALID;
        adma_desc[host->desc_index].adma_desc_addr  = ADDR_CPU_TO_DEV((uintptr_t)src_dst);

        HAL_EMMC_TRACE(4, "---[TRAN]adma_desc[%02d].addr=0x%08X .ctrl=0x%08X@0x%08X", host->desc_index, adma_desc[host->desc_index].adma_desc_addr,
                       adma_desc[host->desc_index].adma_desc_ctrl, (uintptr_t)&adma_desc[host->desc_index].adma_desc_ctrl);

        host->desc_index++;
        if (!next_src_dst) {
            //nop/end desc
            adma_desc[host->desc_index].adma_desc_ctrl  = ADMA2_ATTRIBUTE_ACT_NOP | ADMA2_ATTRIBUTE_VALID | ADMA2_ATTRIBUTE_END;
            adma_desc[host->desc_index].adma_desc_addr  = 0;

            HAL_EMMC_TRACE(4, "---[NOP ]adma_desc[%02d].addr=0x%08X .ctrl=0x%08X@0x%08X", host->desc_index, adma_desc[host->desc_index].adma_desc_addr,
                           adma_desc[host->desc_index].adma_desc_ctrl, (uintptr_t)&adma_desc[host->desc_index].adma_desc_ctrl);

            host->desc_index = 0;
        }
    } else {//more than one desc
        uint32_t i;
        uint32_t integer;
        uint32_t remainder;

        integer = size / ADMA2_DESC_MAX_LEN;
        remainder = size % ADMA2_DESC_MAX_LEN;
        for (i = 0; i < integer; i++) {
            adma_desc[host->desc_index].adma_desc_ctrl  = 0;
            adma_desc[host->desc_index].adma_desc_ctrl |= ADMA2_ATTRIBUTE_ACT_TRAN | ADMA2_ATTRIBUTE_VALID;
            adma_desc[host->desc_index].adma_desc_addr  = ADDR_CPU_TO_DEV((uintptr_t)src_dst + i * ADMA2_DESC_MAX_LEN);

            HAL_EMMC_TRACE(4, "---[TRAN]adma_desc[%02d].addr=0x%08X .ctrl=0x%08X@0x%08X", host->desc_index, adma_desc[host->desc_index].adma_desc_addr,
                           adma_desc[host->desc_index].adma_desc_ctrl, (uintptr_t)&adma_desc[host->desc_index].adma_desc_ctrl);

            host->desc_index++;
        }

        if (remainder) {
            adma_desc[host->desc_index].adma_desc_ctrl  = (uint32_t)((size - integer * ADMA2_DESC_MAX_LEN) << 16);
            adma_desc[host->desc_index].adma_desc_ctrl |= ADMA2_ATTRIBUTE_ACT_TRAN | ADMA2_ATTRIBUTE_VALID;
            adma_desc[host->desc_index].adma_desc_addr  = ADDR_CPU_TO_DEV((uintptr_t)src_dst + integer * ADMA2_DESC_MAX_LEN);

            HAL_EMMC_TRACE(4, "---[TRAN]adma_desc[%02d].addr=0x%08X .ctrl=0x%08X@0x%08X", host->desc_index, adma_desc[host->desc_index].adma_desc_addr,
                           adma_desc[host->desc_index].adma_desc_ctrl, (uintptr_t)&adma_desc[host->desc_index].adma_desc_ctrl);

            host->desc_index++;
        }

        if (!next_src_dst) {
            //nop/end desc
            adma_desc[host->desc_index].adma_desc_ctrl  = ADMA2_ATTRIBUTE_ACT_NOP | ADMA2_ATTRIBUTE_VALID | ADMA2_ATTRIBUTE_END;
            adma_desc[host->desc_index].adma_desc_addr  = 0;

            HAL_EMMC_TRACE(4, "---[NOP ]adma_desc[%02d].addr=0x%08X .ctrl=0x%08X@0x%08X", host->desc_index, adma_desc[host->desc_index].adma_desc_addr,
                           adma_desc[host->desc_index].adma_desc_ctrl, (uintptr_t)&adma_desc[host->desc_index].adma_desc_ctrl);

            host->desc_index = 0;
        }
    }

    return MMC_NO_ERR;
}

static void mmc_create_adma2_desc(struct mmc_host *host)
{
    unsigned int *currdesc;
    unsigned int xfersize;
    uintptr_t localbuff;
    unsigned int attribute;
    unsigned int desc_nr = 0;
    unsigned int desc_nr_p;
    unsigned int descsz = 0;

    if (host->desc_type == ADMA2_DESC_SCATTER) {
        return;//This function is implemented on the external function
    }

    xfersize = host->data->sz;
    switch (host->desc_type) {
        case ADMA2_DESC_SCATTER:
            break;
        case ADMA2_DESC_FIXED_SZ:
            desc_nr = xfersize / host->desc_sz;
            descsz = host->desc_sz;
            break;
        case ADMA2_DESC_FIXED_NR:
            /*
            * For fixed descriptor number, do we have to make each descriptor line as the x blocks?
            * Here we donot do that.
            */
            desc_nr = host->desc_nr;
            descsz = xfersize / host->desc_nr;
            break;
    }
    HAL_EMMC_TRACE(1, "ADMA2 sz = 0x%0x descriptors = %d", xfersize, desc_nr);
    memset(host->desc, 0, host->desc_len);
    currdesc = (unsigned int *)host->desc;
    localbuff = (uintptr_t)host->aligned_buf;
    HAL_EMMC_TRACE(1, "Descriptor location = %p", currdesc);

    desc_nr_p = 1;
    while (xfersize > 0) {
        switch (host->desc_type) {
            case ADMA2_DESC_SCATTER:
                break;
            case ADMA2_DESC_FIXED_SZ:
                /*when the left size is less than descriptor length*/
                if (xfersize < descsz) {
                    descsz = xfersize;
                    desc_nr += 1;
                }
                break;
            case ADMA2_DESC_FIXED_NR:
                /*put the left into the last descriptor*/
                if (desc_nr_p == desc_nr) {
                    /*when the last descriptor > the 64KB, we have to add one*/
                    if (xfersize >= ADMA2_DESC_MAX_LEN) {
                        descsz = ADMA2_DESC_MAX_LEN - host->blksz;
                        desc_nr += 1;
                    } else {
                        descsz = xfersize;
                    }
                }
                break;
        }
        HAL_EMMC_TRACE(1, "descsz = 0x%08x", descsz);
        switch (descsz) {
            default:
                xfersize -= descsz;
                attribute = (descsz << 16) | (ADMA2_ATTRIBUTE_ACT_TRAN + ADMA2_ATTRIBUTE_VALID);
                if (xfersize <= 0) {
                    attribute |= ADMA2_ATTRIBUTE_END;
                }
                *currdesc++ = attribute;
                *currdesc++ = localbuff;
                localbuff  += descsz;
                break;
        }
        desc_nr_p++;
    }
}

void mmc_prepare_transfer(struct mmc_host *host, void *buf, unsigned int dir, unsigned int blksz, unsigned int sz, bool dma_en)
{
    struct mmc_data *data = &g_mmc_data;

    if (host->dma_bdry != MM4_HOST_DMA_BDRY_512KB) {
        host->dma_bdry = MM4_HOST_DMA_BDRY_512KB;
    }

    host->actual_buf = (void *)buf;
    memset(data, 0, sizeof(struct mmc_data));
    data->direction = dir;
    data->blksz = blksz;
    data->blks = sz / blksz;
    data->sz = sz;
    host->data = data;

    /* Less than one block is counted as one block */
    if (sz % blksz) {
        host->data->blks++;

        /*Update the sz*/
        host->data->sz = host->data->blks * host->data->blksz;
    }

    if (host->quirks & MMC_QUIRKS_PERFORMANCE) {
#ifdef EMMC_EXTENDED_USE
        if (((uint32_t)(host->actual_buf) % 32)) {
            host->aligned_buf = host->alloc_p;
            if (host->data->direction == MMC_CARD_WRITE) {
                // TODO: Be careful not to cross the boundary
                memcpy((void *)host->aligned_buf, host->actual_buf, sz);
            }
        } else {
            host->aligned_buf = host->actual_buf;
        }
        hal_dma_sync_for_device(host->aligned_buf, sz);
#else
        host->aligned_buf = host->actual_buf;
#endif
    } else {
        /*Copy data into the aligned buffer for Write*/
        if (host->data->direction == MMC_CARD_WRITE) {
            if (sz <= MMC_MAX_XFER_SZ) {
                memcpy((void *)host->aligned_buf, (void *)host->actual_buf, sz);
            } else {
#ifdef EMMC_ASSERT
                HAL_EMMC_ASSERT(false, "%s:%d, The data length of copy %d > the length of buf %d", __FUNCTION__, __LINE__, sz, MMC_MAX_XFER_SZ);
#else
                memcpy((void *)host->aligned_buf, (void *)host->actual_buf, MMC_MAX_XFER_SZ);
                HAL_EMMC_ERROR(0, "%s:%d, error:The data length of copy %d > the length of buf %d", __FUNCTION__, __LINE__, sz, MMC_MAX_XFER_SZ);
#endif
            }
        }
#ifdef EMMC_EXTENDED_USE
        hal_dma_sync_for_device(host->actual_buf, sz);
#endif
    }
    if (dma_en) {
        sdhci_mask_irqs(host, IRQ_RX_RDY | IRQ_TX_RDY);
        sdhci_unmask_irqs(host, IRQ_DMA_INT | IRQ_XFR_COMP);//other error interrupts opened in function mmc_init_mmc_card()

        /* Use max. boundary size as default */
        sdhciWriteSdh_block_sizeHost_dma_bdry(host, host->dma_bdry);
        switch (host->dma_mode) {
            case MM4_SDMA:
                sdhciWriteSdh_host_ctrlDma_sel(host, 0);
                sdhciWriteSdh_sysaddr(host, ADDR_CPU_TO_DEV((unsigned long)(host->aligned_buf)));
                break;
            case MM4_ADMA_1:
                break;
            case MM4_32_BIT_ADMA_2:
                sdhciWriteSdh_host_ctrlDma_sel(host, 2);
                mmc_create_adma2_desc(host);
                sdhciWriteSdh_adma_sysaddr0(host, ADDR_CPU_TO_DEV((unsigned long)host->desc));
                break;
            case MM4_64_BIT_ADMA_2:
                break;
            default:
                break;
        }
    } else {
        sdhci_mask_irqs(host, IRQ_DMA_INT);
        if (host->data->direction == MMC_CARD_WRITE) {
            sdhci_mask_irqs(host, IRQ_RX_RDY);
            sdhci_unmask_irqs(host, IRQ_TX_RDY | IRQ_XFR_COMP);//other error interrupts opened in function mmc_init_mmc_card()
        } else {
            sdhci_mask_irqs(host, IRQ_TX_RDY);
            sdhci_unmask_irqs(host, IRQ_RX_RDY | IRQ_XFR_COMP);//other error interrupts opened in function mmc_init_mmc_card()
        }
    }

    /* Set the Block Size for the Controller*/
    sdhciWriteSdh_block_sizeBlock_size(host, host->data->blksz);

    /* Set the Number of Blocks for this Transfer*/
    sdhciWriteSdh_block_count(host, host->data->blks);

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);
    HAL_EMMC_TRACE(1, "SDH_SYSADDR[%04X]     = 0x%08X", (unsigned int)(SDH_SYSADDR), sdhciReadSdh_sysaddrSysaddr(host));
    HAL_EMMC_TRACE(1, "SDH_BLOCK_SIZE[%04X]  = 0x%04X", (unsigned int)(SDH_BLOCK_SIZE), sdhciReadSdh_block_size(host));
    HAL_EMMC_TRACE(1, "SDH_BLOCK_COUNT[%04X] = 0x%04X", (unsigned int)(SDH_BLOCK_COUNT), sdhciReadSdh_block_count(host));
    HAL_EMMC_TRACE(1, "SDH_TIMEOUT_CTRL_SW_RESET[0x%04X] = 0x%04X", (unsigned int)(SDH_TIMEOUT_CTRL_SW_RESET), sdhciReadSdh_timeout_ctrl_sw_reset(host));
}

static void mmc_finish_command(struct mmc_host *host)
{
}

static void mmc_finish_transfer(struct mmc_host *host)
{
    if (host->quirks & MMC_QUIRKS_PERFORMANCE) {
        if (host->data->direction == MMC_CARD_READ) {
#ifdef EMMC_EXTENDED_USE
            if (host->actual_buf != host->aligned_buf) {
                if (host->aligned_buf) {
                    hal_dma_sync_for_cpu(host->aligned_buf, (uint32_t)host->data->sz);

                    // TODO: Be careful not to cross the boundary
                    memcpy((void *)host->actual_buf, (void *)host->aligned_buf, host->data->sz);
                }
            } else {
                hal_dma_sync_for_cpu(host->actual_buf, (uint32_t)host->data->sz);
            }
#else
            host->actual_buf = host->aligned_buf;
#endif
        }
    } else {
#ifdef EMMC_EXTENDED_USE
#ifdef EMMC_ASSERT
        HAL_EMMC_ASSERT(false, "%s:%d, The current mode does not support", __FUNCTION__, __LINE__);
#else
        HAL_EMMC_ERROR(0, "%s:%d, error:The current mode does not support", __FUNCTION__, __LINE__);
#endif
#endif
        if (host->data->direction == MMC_CARD_READ) {
            if (host->data->sz <= MMC_MAX_XFER_SZ) {
                memcpy((void *)host->actual_buf, (void *)host->aligned_buf, host->data->sz);
            } else {
#ifdef EMMC_ASSERT
                HAL_EMMC_ASSERT(false, "%s:%d, The data length of copy %d > the length of buf %d", __FUNCTION__, __LINE__, host->data->sz, MMC_MAX_XFER_SZ);
#else
                memcpy((void *)host->actual_buf, (void *)host->aligned_buf, MMC_MAX_XFER_SZ);
                HAL_EMMC_ERROR(0, "%s:%d, error:The data length of copy %d > the length of buf %d", __FUNCTION__, __LINE__, host->data->sz, MMC_MAX_XFER_SZ);
#endif
            }
        }
    }
}

static void mmc_cmd_irq(struct mmc_host *host, unsigned int state)
{
    unsigned int clr_e_bits = 0;
    unsigned int err_mask = EMMC_IRQ_CMD_ERR_MASK;

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    /*Command Error*/
    if (state & err_mask) {
        if ((state & IRQ_CMD_TIMEOUT_ERR) && (state & IRQ_CMD_CRC_ERR)) {
            clr_e_bits |= IRQ_CMD_TIMEOUT_ERR | IRQ_CMD_CRC_ERR;
            host->host_error |= EMMC_HOST_CMD_LINE_CONFLICT_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d error", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_CMD_LINE_CONFLICT_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******cmd%d error:0x%X", host->cmd->opcode, host->host_error);
                    HAL_EMMC_TRACE(1, "****** Command Line Conflict - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        } else {
            if (state & IRQ_CMD_TIMEOUT_ERR) {
                clr_e_bits |= IRQ_CMD_TIMEOUT_ERR;
                host->host_error |= EMMC_HOST_CMD_TIMEOUT_ERR;
                if (!host->tuning_status) {
                    if (host->emmc_host_error) {
                        HAL_EMMC_ERROR(1, "******cmd%d error", host->cmd->opcode);
                        host->emmc_host_error(EMMC_HOST_CMD_TIMEOUT_ERR);
                    } else {
                        HAL_EMMC_ERROR(1, "******cmd%d error:0x%X", host->cmd->opcode, host->host_error);
                        HAL_EMMC_TRACE(1, "****** Command Timed Out - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                    }
                }
            }
            if (state & IRQ_CMD_CRC_ERR) {
                clr_e_bits |= IRQ_CMD_CRC_ERR;
                host->host_error |= EMMC_HOST_CMD_CRC_ERR;
                if (!host->tuning_status) {
                    if (host->emmc_host_error) {
                        HAL_EMMC_ERROR(1, "******cmd%d error", host->cmd->opcode);
                        host->emmc_host_error(EMMC_HOST_CMD_CRC_ERR);
                    } else {
                        HAL_EMMC_ERROR(1, "******cmd%d error:0x%X", host->cmd->opcode, host->host_error);
                        HAL_EMMC_TRACE(1, "****** Command CRC - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                    }
                }
            }
        }
        if (state & IRQ_CMD_ENDBIT_ERR) {
            clr_e_bits |= IRQ_CMD_ENDBIT_ERR;
            host->host_error |= EMMC_HOST_CMD_END_BIT_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d error", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_CMD_END_BIT_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******cmd%d error:0x%X", host->cmd->opcode, host->host_error);
                    HAL_EMMC_TRACE(1, "****** Command End Bit - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        if (state & IRQ_CMD_INDEX_ERR) {
            clr_e_bits |= IRQ_CMD_INDEX_ERR;
            host->host_error |= EMMC_HOST_CMD_INDEX_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d error", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_CMD_INDEX_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******cmd%d error:0x%X", host->cmd->opcode, host->host_error);
                    HAL_EMMC_TRACE(1, "****** Command Index - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        sdhciWriteSdh_e_stat(host, (clr_e_bits >> 16));             //Clear the REG32:ERROR STATUS interrupt flag
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);     //Soft Reset for Command Part of Logic
        if (sdhciReadSdh_cmdData_present(host)) {
            sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1); //Soft Reset for Data Port of Logic
        }
        host->cmd->resp[0] = 0;
        host->cmd->resp[1] = 0;
        host->cmd->resp[2] = 0;
        host->cmd->resp[3] = 0;
        host->cmd->error = 1;
        host->in_use = 0;
    }
#ifdef EMMC_DEBUG
    if (!clr_e_bits) {
        HAL_EMMC_TRACE(1, "%s:%d, command success", __FUNCTION__, __LINE__);
    } else {
        HAL_EMMC_TRACE(1, "%s:%d, command error", __FUNCTION__, __LINE__);
    }
#endif
    /*Cmd complete*/
    if (state & IRQ_CMD_COMP) {
        HAL_EMMC_TRACE(1, "----- command complete - SDH_NORMAL_STATUS: 0x%04X -----", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statCmd_complete(host, IRQ_CMD_COMP);//Clear the IRQ_CMD_COMP interrupt flag

        host->cmd->resp[0] = sdhciReadSdh_respx(host, 0);
        host->cmd->resp[1] = sdhciReadSdh_respx(host, 1);
        host->cmd->resp[2] = sdhciReadSdh_respx(host, 2);
        host->cmd->resp[3] = sdhciReadSdh_respx(host, 3);
        if (!host->cmd->error) {
            host->cmd->complete = 1;
            if (!host->tran_status) {
                if (host->cmd->opcode == READ_SINGLE_BLOCK || host->cmd->opcode == READ_MULTIPLE_BLOCK
                    || host->cmd->opcode == WRITE_SINGLE_BLOCK || host->cmd->opcode == WRITE_MULTIPLE_BLOCK
                    || host->cmd->opcode == IO_RW_EXTENDED) {
                    host->tran_status = 1;
                }
            }
        }
    }
}

static void mmc_data_irq(struct mmc_host *host, unsigned int state)
{
    unsigned int clr_e_bits = 0;
    unsigned short err_status;
    unsigned int err_mask = EMMC_IRQ_DATA_ERR_MASK;

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    /*Data Error*/
    if (state & err_mask) {
        if (state & IRQ_DATA_TIMEOUT_ERR) {
            clr_e_bits |= IRQ_DATA_TIMEOUT_ERR;
            host->host_error |= EMMC_HOST_DATA_TIMEOUT_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_DATA_TIMEOUT_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** Data Timed Out - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        if (state & IRQ_RD_DATA_CRC_ERR) {
            clr_e_bits |= IRQ_RD_DATA_CRC_ERR;
            host->host_error |= EMMC_HOST_RD_DATA_CRC_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_RD_DATA_CRC_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** Data CRC - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        if (state & IRQ_RD_DATA_ENDBIT_ERR) {
            clr_e_bits |= IRQ_RD_DATA_ENDBIT_ERR;
            host->host_error |= EMMC_HOST_RD_DATA_END_BIT_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_RD_DATA_END_BIT_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** Data End Bit - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        if (state & IRQ_AUTOCMD_ERR) {
            err_status = sdhciReadSdh_auto_cmd12_error_status(host);
            sdhciWriteSdh_auto_cmd12_error_status(host, err_status);
            clr_e_bits |= IRQ_AUTOCMD_ERR;
            host->host_error |= EMMC_HOST_AUTO_CMD12_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_AUTO_CMD12_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** Auto CMD - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                    HAL_EMMC_TRACE(1, "****** +++++ SDH_AUTO_CMD_ERROR_STATUS: 0x%04X +++++ ******", err_status);
                }
            }
        }
        if (state & IRQ_ADMA_ERR) {
            err_status = sdhciReadSdh_adma_error_status(host);
            sdhciWriteSdh_adma_error_status(host, err_status);
            clr_e_bits |= IRQ_ADMA_ERR;
            host->host_error |= EMMC_HOST_ADMA_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_ADMA_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** Advanced DMA - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                    HAL_EMMC_TRACE(1, "****** +++++ SDH_ADMA_ERROR_STATUS: 0x%04X +++++ ******", err_status);
                }
            }
        }
        if (state & (unsigned int)CRC_STATUS_ERR) {
            clr_e_bits |= (unsigned int)CRC_STATUS_ERR;
            host->host_error |= EMMC_HOST_CRC_STATUS_ERR;
            if (!host->tuning_status) {
                if (host->emmc_host_error) {
                    HAL_EMMC_ERROR(1, "******cmd%d", host->cmd->opcode);
                    host->emmc_host_error(EMMC_HOST_CRC_STATUS_ERR);
                } else {
                    HAL_EMMC_ERROR(1, "******data error:0x%X, cmd%d", host->host_error, host->cmd->opcode);
                    HAL_EMMC_TRACE(1, "****** CRC Status error - SDH_ERROR_STATUS: 0x%04X ******", sdhciReadSdh_e_stat(host));
                }
            }
        }
        sdhciWriteSdh_e_stat(host, (clr_e_bits >> 16));
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);
        sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1);
        host->cmd->resp[0] = 0;
        host->cmd->resp[1] = 0;
        host->cmd->resp[2] = 0;
        host->cmd->resp[3] = 0;
        host->data->error = 1;
        host->in_use = 0;
    }
#ifdef EMMC_DEBUG
    if (!clr_e_bits) {
        HAL_EMMC_TRACE(1, "data success");
    } else {
        HAL_EMMC_TRACE(1, "data error");
    }
#endif
    /* Determine which data transfer interrupt got asserted*/
    if (state & IRQ_DMA_INT) {//dma tx/rx
        HAL_EMMC_TRACE(1, "----- DMA Interrupt Asserted - SDH_NORMAL_STATUS: 0x%04X -----", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statDma_int(host, 1);
        sdhciWriteSdh_sysaddr(host, sdhciReadSdh_sysaddr(host));//The dma boundary problem is dealt with here,2022-01-08
    } else if ((state & IRQ_TX_RDY) && (host->data->direction == MMC_CARD_WRITE)) {//no dma tx
        HAL_EMMC_TRACE(1, "----- TX Interrupt Asserted - SDH_NORMAL_STATUS: 0x%04X -----", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statTx_rdy(host, 1);
        mmc_transfer_pio(host);
    } else if ((state & IRQ_RX_RDY) && (host->data->direction == MMC_CARD_READ)) {//no dma rx
        HAL_EMMC_TRACE(1, "----- RX Interrupt Asserted - SDH_NORMAL_STATUS: 0x%04X -----", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statRx_rdy(host, 1);
        mmc_transfer_pio(host);
        if ((sdhciReadSdh_cmdCmd_index(host) == MMC_SEND_TUNING_BLOCK) || (sdhciReadSdh_cmdCmd_index(host) == SEND_TUNING_BLOCK)) {
            if (!host->data->error) {
                host->data->complete = 1;
            }
        }
    }
    /*Check for Transfer Complete*/
    if (state & IRQ_XFR_COMP) {
        HAL_EMMC_TRACE(1, "----- Xfer Complete - SDH_NORMAL_STATUS: 0x%04X -----", sdhciReadSdh_i_stat(host));
        if (host->auto_cmd_type == MM4_MANUAL_CMD12) {
            if ((host->dma_status) && (!host->tran_mode)) {//used for no-blocking mode
                if (host->cmd->flags & MMC_DATA_MB) {
                    mmc_send_stop(host, BLOCKING_MODE_NONE);
                    if (!host->data->direction) {//only for write operation
                        unsigned int start, busy_timeout;
                        busy_timeout = MS_TO_FAST_TICKS(10);
                        start = hal_fast_sys_timer_get();
                        while (sdhciReadSdh_present_state_2Dat_level(host) != 0xF) {
                            if (hal_fast_sys_timer_get() - start > busy_timeout) {
                                HAL_EMMC_ERROR(0, "Manul CMD12 wait bus idle timeout");
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (sdhciReadSdh_transfer_modeDma_en(host)) {
            sdhciWriteSdh_i_statDma_int(host, 1);
        }
        sdhciWriteSdh_i_statXfer_complete(host, 1);
        if (!host->data->error) {
            host->data->complete = 1;
        }

        if ((host->speed_mode == HS400 || host->speed_mode == EHS400) && (!host->tran_mode)) {
#ifdef EMMC_HS400_SYSFREQ_RAISE
            hal_sysfreq_req(HAL_SYSFREQ_USER_EMMC, HAL_CMU_FREQ_32K);
#endif
            if (host->strobe) {
                sdhci_host_emphy_clk_sel_pck_delay(host);//revert back to the original clk
            }
        }

        if ((host->dma_status) && (!host->tran_mode)) {//dma + no-blocking mode
            host->dma_status = 0;
            host->tran_status = 0;
            card_controller_clk_pause(host);
            if ((host->emmc_host_dma_done) && (!host->data->error)) {
                host->emmc_host_dma_done();
            }
            host->in_use = 0;
        }
    }
    if (state & IRQ_BLK_GAP_EVENT) {
        HAL_EMMC_ERROR(1, "****** Block Gap Event - SDH_NORMAL_STATUS: 0x%04X ******", sdhciReadSdh_i_stat(host));
        host->blk_gap_evt = 1;
    }
    if (state & IRQ_RETUNING_INT) {
        HAL_EMMC_ERROR(1, "****** Retuning Event - SDH_NORMAL_STATUS: 0x%04X ******", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statRetuning_int(host, 1);
    }
}

void mmc_card_irq(struct mmc_host *host, unsigned int state)
{
    if (state & IRQ_CARD_INSERTED) {
        HAL_EMMC_ERROR(1, "****** Card Insert Asserted - SDH_NORMAL_STATUS: 0x%04X ******", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statCard_ins_int(host, 1);
        host->card_inserted = 1;
    }
    if (state & IRQ_CARD_REMOVED) {
        HAL_EMMC_ERROR(1, "****** Card Remove Asserted - SDH_NORMAL_STATUS: 0x%04X ******", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statCard_rem_int(host, 1);
        host->card_removed = 1;
    }
    if (state & IRQ_CARD_INT) {
        HAL_EMMC_ERROR(1, "****** Card Interrupt Asserted - SDH_NORMAL_STATUS: 0x%04X ******", sdhciReadSdh_i_stat(host));
        host->card_interrupt = 1;
    }
}

#ifdef EMMC_RUN_RECORD
SRAM_BSS_LOC volatile uint32_t mmc_status_ori[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_status_ori_index = 0;
SRAM_BSS_LOC volatile uint32_t mmc_status_ori_cmd[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_status_ori_cmd_index = 0;

SRAM_BSS_LOC volatile uint8_t  mmc_opcode_ori[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_opcode_ori_index = 0;
SRAM_BSS_LOC volatile uint8_t  mmc_opcode_copy[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_opcode_copy_index = 0;

SRAM_BSS_LOC volatile uint32_t mmc_resp_normal[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_resp_normal_index = 0;
SRAM_BSS_LOC volatile uint32_t mmc_resp_error[EMMC_RUN_RECORD_CNT] = {0};
SRAM_BSS_LOC volatile uint16_t mmc_resp_error_index = 0;
#endif

void mmc_irq(void *private)
{
    unsigned int state;
    struct mmc_host *host = (struct mmc_host *)private;
    unsigned int cmd_mask = EMMC_IRQ_CMD_ERR_MASK | IRQ_CMD_COMP;
    unsigned int dat_mask = EMMC_IRQ_DATA_ERR_MASK | IRQ_DMA_INT | IRQ_TX_RDY | IRQ_RX_RDY | \
                            IRQ_XFR_COMP | IRQ_BLK_GAP_EVENT | IRQ_RETUNING_INT;
    unsigned int card_mask = EMMC_IRQ_CARD_MASK;

    state = (sdhciReadSdh_e_stat(host) << 16) + sdhciReadSdh_i_stat(host);
#ifdef EMMC_RUN_RECORD
    mmc_status_ori[mmc_status_ori_index++] = state;
    if (mmc_status_ori_index >= EMMC_RUN_RECORD_CNT) {
        mmc_status_ori_index = 0;
    }
#endif

    host->host_error = 0;
    if (state & cmd_mask) {
        mmc_cmd_irq(host, state & cmd_mask);
    }
    if (state & dat_mask) {
        mmc_data_irq(host, state & dat_mask);
#ifdef EMMC_EXTENDED_USE
        host->data_irq = 1;
#endif
    }
    if (state & card_mask) {
        mmc_card_irq(host, state & card_mask);
    }
    if (host->ops && host->ops->vendor_irq) {
        host->ops->vendor_irq(host);
    }
}

static void mmc_irq_cmd(void *private)
{
    unsigned int state;
    struct mmc_host *host = (struct mmc_host *)private;

    state = sdhciReadSdh_i_stat(host);
#ifdef EMMC_RUN_RECORD
    mmc_status_ori_cmd[mmc_status_ori_cmd_index++] = state;
    if (mmc_status_ori_cmd_index >= EMMC_RUN_RECORD_CNT) {
        mmc_status_ori_cmd_index = 0;
    }
#endif

    if (state & IRQ_CMD_COMP) {
        HAL_EMMC_TRACE(1, "===== command complete - SDH_NORMAL_STATUS: 0x%04X =====", sdhciReadSdh_i_stat(host));
        sdhciWriteSdh_i_statCmd_complete(host, IRQ_CMD_COMP);//Clear the REG30:IRQ_CMD_COMP interrupt flag
        host->cmd->resp[0] = sdhciReadSdh_respx(host, 0);
        host->cmd->resp[1] = sdhciReadSdh_respx(host, 1);
        host->cmd->resp[2] = sdhciReadSdh_respx(host, 2);
        host->cmd->resp[3] = sdhciReadSdh_respx(host, 3);
        host->cmd->complete = 1;
    }
}

int mmc_send_cmmand(struct mmc_host *host, unsigned int opcode, unsigned int arg, unsigned int flags)
{
    unsigned int present1;
    unsigned int sdh_cmd;
    unsigned int trans_mode;
    unsigned int start;
    unsigned int timeout;
    struct mmc_cmd *cmd = &g_mmc_cmd;

#ifdef EMMC_RUN_RECORD
    mmc_opcode_ori[mmc_opcode_ori_index++] = opcode;
    if (mmc_opcode_ori_index >= EMMC_RUN_RECORD_CNT) {
        mmc_opcode_ori_index = 0;
    }
#endif

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);
    card_controller_clk_continue(host);

    if (host->data) {
        host->data->complete = 0;
        host->data->error = 0;
    }
    if (host->cmd) {
        host->cmd->complete = 0;
        host->cmd->error = 0;
    }

    memset(cmd, 0, sizeof(struct mmc_cmd));
    host->cmd = cmd;
    host->cmd->opcode = opcode;
    host->cmd->arg = arg;
    host->cmd->flags = flags;
    present1 = SDH_CMD_INHIBIT;
    if (host->cmd->flags & MMC_DATA_PRESENT) {
        present1 |= SDH_DAT_INHIBIT;
    }

    //Waiting for the current transfer to complete
    HAL_EMMC_TRACE(1, "wait bus idle until %ums", WAIT_SM_IDLE_TIMEOUT + WAIT_BUS_IDLE_TIMEOUT);
    timeout = MS_TO_FAST_TICKS(WAIT_SM_IDLE_TIMEOUT);
    start = hal_fast_sys_timer_get();
    while (sdhciReadSdh_present_state_1(host) & present1) {
        if (hal_fast_sys_timer_get() - start > timeout) {
            //reset cmd state machine
            sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(host, 1);
            if (sdhciReadSdh_cmdData_present(host)) {
                //reset data state machine
                sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(host, 1);
            }
            HAL_EMMC_ERROR(0, "%s, TIMEOUT: SDH_INT_ERR_STATUS=0x%X SDH_INT_NORMANL_STATUS=0x%X", __FUNCTION__,
                           sdhciReadSdh_e_stat(host), sdhciReadSdh_i_stat(host));
            HAL_EMMC_ERROR(0, "Host state machine status(0x24)=0x%X", sdhciReadSdh_present_state_1(host));
            break;
        }
        host->delay(1);
    }

    //Waiting for the bus idle
    timeout = MS_TO_FAST_TICKS(WAIT_SM_IDLE_TIMEOUT + WAIT_BUS_IDLE_TIMEOUT);
    while (sdhciReadSdh_present_state_1(host) & present1) {
        if (hal_fast_sys_timer_get() - start > timeout) {
            HAL_EMMC_ERROR(0, "%s, TIMEOUT: SDH_INT_ERR_STATUS=0x%X SDH_INT_NORMANL_STATUS=0x%X", __FUNCTION__,
                           sdhciReadSdh_e_stat(host), sdhciReadSdh_i_stat(host));
            HAL_EMMC_ERROR(0, "Host state machine status(0x24)=0x%X", sdhciReadSdh_present_state_1(host));
            return MMC_DEVICE_BUSY;
        }
        host->delay(1);
    }
    HAL_EMMC_TRACE(1, "wait done, %uus", FAST_TICKS_TO_US(hal_fast_sys_timer_get() - start));

    /*argument*/
    sdhciWriteSdh_arg(host, host->cmd->arg);
    /*transfer mode*/
    trans_mode = 0;
    if (host->cmd->flags & MMC_DATA_PRESENT) {
        if (host->cmd->flags & MMC_DATA_DMA) {
            trans_mode |= SDH_DMA_EN;
        }
        if (host->cmd->flags & MMC_DATA_BC_EN) {
            trans_mode |= SDH_BLK_CNT_EN;
        }
        /*autocmd*/
        if (host->cmd->flags & MMC_DATA_AUTOCMD23) {
            trans_mode |= SDH_AUTO_CMD23;
        } else if (host->cmd->flags & MMC_DATA_AUTOCMD12) {
            trans_mode |= SDH_AUTO_CMD12;
        } else {
            trans_mode |= SDH_AUTO_CMD_DIS;
        }
        if (host->cmd->flags & MMC_DATA_READ) {
            trans_mode |= SDH_READ;
        } else {
            trans_mode |= SDH_WRITE;
        }
        if (host->cmd->flags & MMC_DATA_MB) {
            trans_mode |= SDH_MULTI_BLK_SEL;
        }
    }
    sdhciWriteSdh_transfer_mode(host, trans_mode);
    /*response type*/
    if (!(host->cmd->flags & MMC_RSP_PRESENT)) {
        sdh_cmd = SDH_CMD_RESP_NONE;
    } else if (host->cmd->flags & MMC_RSP_136) {
        sdh_cmd = SDH_CMD_RESP_LONG;
    } else if (host->cmd->flags & MMC_RSP_BUSY) {
        sdh_cmd = SDH_CMD_RESP_SHORT_BUSY;
    } else {
        sdh_cmd = SDH_CMD_RESP_SHORT;
    }
    /*command crc and index check*/
    if (host->cmd->flags & MMC_RSP_CRC) {
        sdh_cmd |= SDH_CMD_CRC;
    }
    if (host->cmd->flags & MMC_RSP_OPCODE) {
        sdh_cmd |= SDH_CMD_INDEX;
    }
    /*data present*/
    if (host->cmd->flags & MMC_DATA_PRESENT) {
        sdh_cmd |= SDH_CMD_DATA;
    }
    /*command type*/
    if (host->cmd->flags & MMC_CMD_AC) {
        sdh_cmd |= SDH_CMD_ABORT;
    } else if (host->cmd->flags & MMC_CMD_RC) {
        sdh_cmd |= SDH_CMD_RESUME;
    } else if (host->cmd->flags & MMC_CMD_SC) {
        sdh_cmd |= SDH_CMD_SUSPEND;
    } else {
        sdh_cmd |= SDH_CMD_NORMAL;
    }
    /*command index*/
    sdh_cmd |= SDH_MAKE_CMD(host->cmd->opcode);
    HAL_EMMC_TRACE(1, "Sending CMD%d, ARG:0x%08X", host->cmd->opcode, host->cmd->arg);
    sdhciWriteSdh_cmd(host, sdh_cmd);
    if (host->resp_polling_mode) {
        if (host->cmd->opcode == READ_SINGLE_BLOCK || host->cmd->opcode == READ_MULTIPLE_BLOCK
            || host->cmd->opcode == WRITE_SINGLE_BLOCK || host->cmd->opcode == WRITE_MULTIPLE_BLOCK
            || host->cmd->opcode == IO_RW_EXTENDED) {
            host->tran_status = 1;
        }
    }
    HAL_EMMC_TRACE(1, "SDH_CMD[%04X]           = 0x%04X", (unsigned int)(SDH_CMD), sdhciReadSdh_cmd(host));
    HAL_EMMC_TRACE(1, "SDH_ARGUMENT[%04X]      = 0x%08X", (unsigned int)(SDH_ARGUMENT), sdhciReadSdh_arg(host));
    HAL_EMMC_TRACE(1, "SDH_TRANSFER_MODE[%04X] = 0x%04X", (unsigned int)(SDH_TRANSFER_MODE), sdhciReadSdh_transfer_mode(host));
    HAL_EMMC_TRACE(1, "%s:done", __FUNCTION__);

#ifdef EMMC_RUN_RECORD
    mmc_opcode_copy[mmc_opcode_copy_index++] = opcode;
    if (mmc_opcode_copy_index >= EMMC_RUN_RECORD_CNT) {
        mmc_opcode_copy_index = 0;
    }
#endif

    return MMC_NO_ERR;
}

int mmc_wait_for_resp(struct mmc_host *host, unsigned int resp_type)
{
    int ret = MMC_NO_ERR;
    unsigned int start;
    unsigned int timeout = MS_TO_FAST_TICKS(WAIT_RESPONSE_TIMEOUT);

    HAL_EMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    //When there is subsequent data transmission, no longer wait for cmd resp
    if (host->cmd->opcode == READ_SINGLE_BLOCK || host->cmd->opcode == READ_MULTIPLE_BLOCK
        || host->cmd->opcode == WRITE_SINGLE_BLOCK || host->cmd->opcode == WRITE_MULTIPLE_BLOCK
        || host->cmd->opcode == IO_RW_EXTENDED) {
        return ret;
    }

    start = hal_fast_sys_timer_get();
    if (host->resp_polling_mode) {
        while (!host->cmd->complete && !host->cmd->error) {
            if (hal_fast_sys_timer_get() - start > timeout) {
                HAL_EMMC_ERROR(1, "Software Timeout for CMD! %s:%d", __FUNCTION__, __LINE__);
                HAL_EMMC_ERROR(1, "SDH_INT_ERR_STATUS = 0x%04x SDH_INT_NORMANL_STATUS = 0x%04x", sdhciReadSdh_e_stat(host), sdhciReadSdh_i_stat(host));
                host->cmd->error = 1;
            }
            mmc_irq_cmd((void *)host);
        }
    } else {
        if (host->irq_en == true) {
            while (!host->cmd->complete && !host->cmd->error) {
                if (hal_fast_sys_timer_get() - start > timeout) {
                    HAL_EMMC_ERROR(1, "Software Timeout for CMD! %s:%d", __FUNCTION__, __LINE__);
                    HAL_EMMC_ERROR(1, "SDH_INT_ERR_STATUS = 0x%04x SDH_INT_NORMANL_STATUS = 0x%04x", sdhciReadSdh_e_stat(host), sdhciReadSdh_i_stat(host));
                    host->cmd->error = 1;
                }
            }
        } else {
            while (!host->cmd->complete && !host->cmd->error) {
                if (hal_fast_sys_timer_get() - start > timeout) {
                    HAL_EMMC_ERROR(1, "Software Timeout for CMD! %s:%d", __FUNCTION__, __LINE__);
                    HAL_EMMC_ERROR(1, "SDH_INT_ERR_STATUS = 0x%04x SDH_INT_NORMANL_STATUS = 0x%04x", sdhciReadSdh_e_stat(host), sdhciReadSdh_i_stat(host));
                    host->cmd->error = 1;
                }
                mmc_irq((void *)host);
            }
        }
    }
    mmc_finish_command(host);
    if (host->cmd->error) {
#ifdef EMMC_RUN_RECORD
        if (host->cmd->opcode != MMC_SEND_TUNING_BLOCK) {
            mmc_resp_error[mmc_resp_error_index++] = sdhciReadSdh_respx(host, 0);
            if (mmc_resp_error_index >= EMMC_RUN_RECORD_CNT) {
                mmc_resp_error_index = 0;
            }
        }
#endif
        if (host->host_error) {
            if (!host->tuning_status) {
                HAL_EMMC_ERROR(1, "******%s, cmd%d error:0x%X", __FUNCTION__, host->cmd->opcode, host->host_error);
            }
            return host->host_error;
        } else {
            HAL_EMMC_ERROR(1, "******%s, cmd%d error:0x%X", __FUNCTION__, host->cmd->opcode, MMC_CMD_ERR);
            return MMC_CMD_ERR;
        }
    }
    switch (resp_type) {
        case CARD_RESPONSE_NONE:
            break;
        case CARD_RESPONSE_R1:
        case CARD_RESPONSE_R1B:
            host->card->r1 = host->cmd->resp[0];
            HAL_EMMC_TRACE(1, "R1 RESP: 0x%08x", host->card->r1);
            break;
        case CARD_RESPONSE_R2:
            host->card->raw_data[0] = host->cmd->resp[3];//high
            host->card->raw_data[1] = host->cmd->resp[2];
            host->card->raw_data[2] = host->cmd->resp[1];
            host->card->raw_data[3] = host->cmd->resp[0];//low
            if (sdhciReadSdh_cmdCmd_index(host) == SEND_CSD) {
                mmc_unpack_csd(host, host->card->raw_data);
            } else {
                mmc_unpack_cid(host, host->card->raw_data);
            }
            break;
        case CARD_RESPONSE_R3:
            host->card->ocr = host->cmd->resp[0];
            HAL_EMMC_TRACE(1, "R3 RESP(OCR): 0x%08x", host->card->ocr);
            break;
        case CARD_RESPONSE_R4:
            host->card->ocr = host->cmd->resp[0] & 0x00FFFFFF;
            HAL_EMMC_TRACE(1, "R4 RESP: 0x%08x OCR: 0x%08x", host->cmd->resp[0], host->card->ocr);
            break;
        case CARD_RESPONSE_R5:
        case CARD_RESPONSE_R5B:
            host->card->r5 = host->cmd->resp[0];
            HAL_EMMC_TRACE(1, "R5 RESP: 0x%08x", host->card->r5);
            break;
        case CARD_RESPONSE_R6:
            break;
        case CARD_RESPONSE_R7:
            host->card->sd_vhs = host->cmd->resp[0];
            break;
    }
#ifdef EMMC_RUN_RECORD
    mmc_resp_normal[mmc_resp_normal_index++] = host->cmd->resp[0];
    if (mmc_resp_normal_index >= EMMC_RUN_RECORD_CNT) {
        mmc_resp_normal_index = 0;
    }
#endif
    return ret;
}

int mmc_wait_for_xfer(struct mmc_host *host, unsigned  int timeout_ms, enum BLOCKING_MODE blocking_mode)
{
    int ret = MMC_NO_ERR;
    unsigned int start;
    unsigned int timeout;

    if (blocking_mode == BLOCKING_MODE_NONE) {
        return ret;
    }

    timeout = MS_TO_TICKS(timeout_ms);
    if ((blocking_mode == BLOCKING_MODE_IRQ_OFF) && (host->irq_en == false)) {
        start = hal_sys_timer_get();
        while (!host->data->complete && !host->data->error) {
            if (hal_sys_timer_get() - start > timeout) {
                HAL_EMMC_ERROR(0, "waiting for xfer timeout: polling mode");
                host->data->error = 1;
            }
            mmc_irq((void *)host);
        }
    } else {
        start = hal_sys_timer_get();
        while (!host->data->complete && !host->data->error) {
            if (hal_sys_timer_get() - start > timeout) {
                HAL_EMMC_ERROR(0, "waiting for xfer timeout: int mode");
                host->data->error = 1;
            }
            host->delay(1);
        }
    }
    if (host->data->error) {
        ret = MMC_DATA_XFER_ERR;
    }

    mmc_finish_transfer(host);

    return ret;
}
