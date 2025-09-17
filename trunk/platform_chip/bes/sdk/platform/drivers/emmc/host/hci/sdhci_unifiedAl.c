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
#include "sdhci_unifiedAl.h"

static unsigned short gw16sdhciSdh_force_event_acmd12errShadow;
static unsigned short gw16sdhciSdh_force_event_error_statusShadow;

/////////////////////////////////////
// Universal Register access functions
/////////////////////////////////////

inline unsigned int sdhciReadSdh_regs(uintptr_t reg_addr)
{
    return (*(((volatile uint32_t *)(uintptr_t)(reg_addr))));
}

inline void sdhciWriteSdh_regs(uintptr_t reg_addr, unsigned int w32Val)
{
    *((volatile uint32_t *)(uintptr_t)((volatile uint32_t *)(uintptr_t)(reg_addr))) = w32Val;
}

/////////////////////////////////////
// Register access functions
/////////////////////////////////////

inline unsigned int sdhciReadSdh_sysaddr(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0000)))));     // return the contents of Addr=((pContext->base + 0x0000))
}

inline void sdhciWriteSdh_sysaddr(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0000)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0000))
}

////////////////////////////////////////////////////////////////////
// Register SDH_SYSADDR Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_sysaddrSysaddr(struct mmc_host *pContext)
{
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0000))))));
}

inline void sdhciWriteSdh_sysaddrSysaddr(struct mmc_host *pContext,  unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0000)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0000))
}

inline unsigned short sdhciReadSdh_block_size(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004)))));     // return the contents of Addr=((pContext->base + 0x0004))
}

inline void sdhciWriteSdh_block_size(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_BLOCK_SIZE_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0004))
}

////////////////////////////////////////////////////////////////////
// Register SDH_BLOCK_SIZE Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_block_sizeHost_dma_bdry(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004))))));
    w16Val  &=   SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_MASK;         // mask contents to just the bitfield (mask=0x7000)
    w16Val >>=   SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_block_sizeHost_dma_bdry(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 7) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_MASK;              // mask the source data (mask=0x7000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_MASK)); // mask the Register data (mask=0x7000)
    mask &= ~(SDHCI_SDH_BLOCK_SIZE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x8000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0004))
}

inline unsigned short sdhciReadSdh_block_sizeBlock_size(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004))))));
    w16Val  &=   SDHCI_SDH_BLOCK_SIZE_BLOCK_SIZE_MASK;         // mask contents to just the bitfield (mask=0x0FFF)
    return (w16Val);
}

inline void sdhciWriteSdh_block_sizeBlock_size(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 2048) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_BLOCK_SIZE_BLOCK_SIZE_MASK;              // mask the source data (mask=0x0FFF)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLOCK_SIZE_BLOCK_SIZE_MASK));    // mask the Register data (mask=0x0FFF)
    mask &= ~(SDHCI_SDH_BLOCK_SIZE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x8000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0004)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0004))
}

inline unsigned short sdhciReadSdh_block_count(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0006)))));     // return the contents of Addr=((pContext->base + 0x0006))
}

inline void sdhciWriteSdh_block_count(struct mmc_host *pContext, unsigned short w16Val)
{
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0006)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0006))
}

////////////////////////////////////////////////////////////////////
// Register SDH_BLOCK_COUNT Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_block_countBlock_count(struct mmc_host *pContext)
{
    return (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0006))))));
}

inline void sdhciWriteSdh_block_countBlock_count(struct mmc_host *pContext,  unsigned short w16Val)
{
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0006)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0006))
}

inline unsigned int sdhciReadSdh_arg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0008)))));     // return the contents of Addr=((pContext->base + 0x0008))
}

inline void sdhciWriteSdh_arg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0008)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0008))
}

////////////////////////////////////////////////////////////////////
// Register SDH_ARG Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_argArg(struct mmc_host *pContext)
{
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0008))))));
}

inline void sdhciWriteSdh_argArg(struct mmc_host *pContext,  unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0008)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0008))
}

inline unsigned short sdhciReadSdh_transfer_mode(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))));     // return the contents of Addr=((pContext->base + 0x000C))
}

inline void sdhciWriteSdh_transfer_mode(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16Val;      // write the data into the register at ((pContext->base + 0x000C))
}

////////////////////////////////////////////////////////////////////
// Register SDH_TRANSFER_MODE Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_transfer_modeMulti_blk_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16Val  &=   SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_transfer_modeMulti_blk_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_MASK));  // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFC0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000C))
}

inline unsigned short sdhciReadSdh_transfer_modeTo_host_dir(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16Val  &=   SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_transfer_modeTo_host_dir(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_MASK));    // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFC0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000C))
}

inline unsigned short sdhciReadSdh_transfer_modeAuto_cmd_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16Val  &=   SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_MASK;         // mask contents to just the bitfield (mask=0x000C)
    w16Val >>=   SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_transfer_modeAuto_cmd_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_MASK;              // mask the source data (mask=0x000C)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_MASK));    // mask the Register data (mask=0x000C)
    mask &= ~(SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFC0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000C))
}

inline unsigned short sdhciReadSdh_transfer_modeBlk_cnt_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16Val  &=   SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_transfer_modeBlk_cnt_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_MASK)); // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFC0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000C))
}

inline unsigned short sdhciReadSdh_transfer_modeDma_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16Val  &=   SDHCI_SDH_TRANSFER_MODE_DMA_EN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_transfer_modeDma_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_TRANSFER_MODE_DMA_EN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TRANSFER_MODE_DMA_EN_MASK)); // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFC0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000C))
}

inline unsigned short sdhciReadSdh_cmd(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))));     // return the contents of Addr=((pContext->base + 0x000E))
}

inline void sdhciWriteSdh_cmd(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_CMD_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16Val;      // write the data into the register at ((pContext->base + 0x000E))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CMD Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_cmdCmd_index(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_CMD_INDEX_MASK;         // mask contents to just the bitfield (mask=0x3F00)
    w16Val >>=   SDHCI_SDH_CMD_CMD_INDEX_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdCmd_index(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 63) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CMD_CMD_INDEX_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_CMD_CMD_INDEX_MASK;              // mask the source data (mask=0x3F00)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_CMD_INDEX_MASK));    // mask the Register data (mask=0x3F00)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned short sdhciReadSdh_cmdCmd_type(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_CMD_TYPE_MASK;         // mask contents to just the bitfield (mask=0x00C0)
    w16Val >>=   SDHCI_SDH_CMD_CMD_TYPE_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdCmd_type(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 3) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CMD_CMD_TYPE_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_CMD_CMD_TYPE_MASK;              // mask the source data (mask=0x00C0)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_CMD_TYPE_MASK)); // mask the Register data (mask=0x00C0)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned short sdhciReadSdh_cmdData_present(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_DATA_PRESENT_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_CMD_DATA_PRESENT_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdData_present(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CMD_DATA_PRESENT_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_CMD_DATA_PRESENT_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_DATA_PRESENT_MASK)); // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned short sdhciReadSdh_cmdCmd_index_chk_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdCmd_index_chk_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_MASK)); // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned short sdhciReadSdh_cmdCmd_crc_chk_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_CMD_CRC_CHK_EN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_CMD_CMD_CRC_CHK_EN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdCmd_crc_chk_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CMD_CMD_CRC_CHK_EN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_CMD_CMD_CRC_CHK_EN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_CMD_CRC_CHK_EN_MASK));   // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned short sdhciReadSdh_cmdResp_type(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16Val  &=   SDHCI_SDH_CMD_RESP_TYPE_MASK;         // mask contents to just the bitfield (mask=0x0003)
    return (w16Val);
}

inline void sdhciWriteSdh_cmdResp_type(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 3) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_CMD_RESP_TYPE_MASK;              // mask the source data (mask=0x0003)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CMD_RESP_TYPE_MASK));    // mask the Register data (mask=0x0003)
    mask &= ~(SDHCI_SDH_CMD_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xC004)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x000E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x000E))
}

inline unsigned int sdhciReadSdh_respx(struct mmc_host *pContext, int iIndex)
{
    if ((iIndex > 3) || (iIndex < 0)) {
        sdhciAccessLayerRangeError("%s - Register Array Index is out of range <0> to <3>", __FUNCTION__); // call the error handler
    }
    return (*(((volatile unsigned int *)(uintptr_t)(((pContext->base + 0x0010)) + (iIndex * 0x4)))));     // return the contents of Addr=((pContext->base + 0x0010) + (iIndex))
}

////////////////////////////////////////////////////////////////////
// Register SDH_RESPx Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_respxResp(struct mmc_host *pContext, int iIndex)
{
    if ((iIndex > 3) || (iIndex < 0)) {
        sdhciAccessLayerRangeError("%s - Register Array Index is out of range <0> to <3>", __FUNCTION__); // call the error handler
    }
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)(((pContext->base + 0x0010)) + (iIndex * 0x4))))));
}

inline unsigned int sdhciReadSdh_dp_buffer(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0020)))));     // return the contents of Addr=((pContext->base + 0x0020))
}

inline void sdhciWriteSdh_dp_buffer(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0020)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0020))
}

////////////////////////////////////////////////////////////////////
// Register SDH_DP_BUFFER Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_dp_bufferBfr_data(struct mmc_host *pContext)
{
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0020))))));
}

inline void sdhciWriteSdh_dp_bufferBfr_data(struct mmc_host *pContext,  unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0020)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0020))
}

inline unsigned short sdhciReadSdh_present_state_1(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024)))));     // return the contents of Addr=((pContext->base + 0x0024))
}

////////////////////////////////////////////////////////////////////
// Register SDH_PRESENT_STATE_1 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_present_state_1Buffer_rd_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_BUFFER_RD_EN_MASK;         // mask contents to just the bitfield (mask=0x0800)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_BUFFER_RD_EN_SHIFT;        // shift the bitfield data down (shift=11)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Buffer_wr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_BUFFER_WR_EN_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_BUFFER_WR_EN_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Rx_active(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_RX_ACTIVE_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_RX_ACTIVE_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Tx_active(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_TX_ACTIVE_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_TX_ACTIVE_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Retuning_req(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_RETUNING_REQ_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_RETUNING_REQ_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Dat_active(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_DAT_ACTIVE_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_DAT_ACTIVE_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Cmd_inhibit_dat(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_DAT_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_DAT_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_1Cmd_inhibit_cmd(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0024))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_CMD_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026)))));     // return the contents of Addr=((pContext->base + 0x0026))
}

////////////////////////////////////////////////////////////////////
// Register SDH_PRESENT_STATE_2 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_present_state_2Cmd_level(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_CMD_LEVEL_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_2_CMD_LEVEL_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2Dat_level(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_DAT_LEVEL_MASK;         // mask contents to just the bitfield (mask=0x00F0)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_2_DAT_LEVEL_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2Write_prot(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_WRITE_PROT_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_2_WRITE_PROT_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2Card_detect(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_CARD_DETECT_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_2_CARD_DETECT_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2Card_stable(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_CARD_STABLE_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_PRESENT_STATE_2_CARD_STABLE_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_present_state_2Card_inserted(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0026))))));
    w16Val  &=   SDHCI_SDH_PRESENT_STATE_2_CARD_INSERTED_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_host_ctrl(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))));     // return the contents of Addr=((pContext->base + 0x0028))
}

inline void sdhciWriteSdh_host_ctrl(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0028))
}

////////////////////////////////////////////////////////////////////
// Register SDH_HOST_CTRL Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_host_ctrlBus_volt(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_BUS_VOLT_MASK;         // mask contents to just the bitfield (mask=0x0E00)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_BUS_VOLT_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlBus_volt(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if ((w16Val >= 5) && (w16Val <= 7)) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_BUS_VOLT_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_BUS_VOLT_MASK;              // mask the source data (mask=0x0E00)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_BUS_VOLT_MASK));   // mask the Register data (mask=0x0E00)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlBus_power(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_BUS_POWER_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_BUS_POWER_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlBus_power(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_BUS_POWER_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_BUS_POWER_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_BUS_POWER_MASK));  // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlCard_det_s(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_CARD_DET_S_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_CARD_DET_S_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlCard_det_s(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_CARD_DET_S_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_CARD_DET_S_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_CARD_DET_S_MASK)); // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlCard_det_l(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_CARD_DET_L_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_CARD_DET_L_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlCard_det_l(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_CARD_DET_L_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_CARD_DET_L_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_CARD_DET_L_MASK)); // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlEx_data_width(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlEx_data_width(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_MASK));  // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlDma_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_DMA_SEL_MASK;         // mask contents to just the bitfield (mask=0x0018)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_DMA_SEL_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlDma_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 3) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_DMA_SEL_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_DMA_SEL_MASK;              // mask the source data (mask=0x0018)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_DMA_SEL_MASK));    // mask the Register data (mask=0x0018)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlHi_speed_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlHi_speed_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_MASK));    // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlData_width(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_DATA_WIDTH_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_DATA_WIDTH_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlData_width(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_DATA_WIDTH_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_DATA_WIDTH_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_DATA_WIDTH_MASK)); // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_host_ctrlLed_ctrl(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_LED_CTRL_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrlLed_ctrl(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_HOST_CTRL_LED_CTRL_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_LED_CTRL_MASK));   // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0028)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0028))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrl(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))));     // return the contents of Addr=((pContext->base + 0x002A))
}

inline void sdhciWriteSdh_blk_gap_ctrl(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16Val;      // write the data into the register at ((pContext->base + 0x002A))
}

////////////////////////////////////////////////////////////////////
// Register SDH_BLK_GAP_CTRL Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_blk_gap_ctrlW_removal(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlW_removal(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_MASK));   // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlW_insertion(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlW_insertion(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_MASK)); // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlW_card_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlW_card_int(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_MASK));  // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlInt_blk_gap(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlInt_blk_gap(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_MASK)); // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlRd_wait_ctl(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlRd_wait_ctl(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_MASK)); // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlCont_req(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlCont_req(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_MASK));    // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_blk_gap_ctrlStop_at_block_gap_req(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16Val  &=   SDHCI_SDH_BLK_GAP_CTRL_STOP_AT_BLOCK_GAP_REQ_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_blk_gap_ctrlStop_at_block_gap_req(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_BLK_GAP_CTRL_STOP_AT_BLOCK_GAP_REQ_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_BLK_GAP_CTRL_STOP_AT_BLOCK_GAP_REQ_MASK));   // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002A))
}

inline unsigned short sdhciReadSdh_clock_ctrl(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))));     // return the contents of Addr=((pContext->base + 0x002C))
}

inline void sdhciWriteSdh_clock_ctrl(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16Val;      // write the data into the register at ((pContext->base + 0x002C))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CLOCK_CTRL Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_clock_ctrlFreq_sel_lo(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_MASK;         // mask contents to just the bitfield (mask=0xFF00)
    w16Val >>=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_clock_ctrlFreq_sel_lo(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;

    if (w16Val > 0xFF) {
        w16Val = 0xFF;
    }

    w16Val   <<=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_MASK;              // mask the source data (mask=0xFF00)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_MASK));   // mask the Register data (mask=0xFF00)
    mask &= ~(SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0018)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002C))
}

inline unsigned short sdhciReadSdh_clock_ctrlFreq_sel_hi(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_MASK;         // mask contents to just the bitfield (mask=0x00C0)
    w16Val >>=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_clock_ctrlFreq_sel_hi(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 3) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_MASK;              // mask the source data (mask=0x00C0)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_MASK));   // mask the Register data (mask=0x00C0)
    mask &= ~(SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0018)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002C))
}

inline unsigned short sdhciReadSdh_clock_ctrlClk_gen_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_clock_ctrlClk_gen_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_MASK));   // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0018)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002C))
}

inline unsigned short sdhciReadSdh_clock_ctrlClock_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_clock_ctrlClock_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_MASK));  // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0018)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002C))
}

inline unsigned short sdhciReadSdh_clock_ctrlInt_clk_stable(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_INT_CLK_STABLE_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_CLOCK_CTRL_INT_CLK_STABLE_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_clock_ctrlInt_clk_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16Val  &=   SDHCI_SDH_CLOCK_CTRL_INT_CLK_EN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_clock_ctrlInt_clk_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_CLOCK_CTRL_INT_CLK_EN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CLOCK_CTRL_INT_CLK_EN_MASK));    // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0018)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002C))
}

inline unsigned short sdhciReadSdh_timeout_ctrl_sw_reset(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))));     // return the contents of Addr=((pContext->base + 0x002E))
}

inline void sdhciWriteSdh_timeout_ctrl_sw_reset(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))) = w16Val;      // write the data into the register at ((pContext->base + 0x002E))
}

////////////////////////////////////////////////////////////////////
// Register SDH_TIMEOUT_CTRL_SW_RESET Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_dat(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16Val  &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_MASK)); // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002E))
}

inline unsigned short sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_cmd(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16Val  &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

void sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_MASK)); // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002E))
}

inline unsigned short sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_all(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16Val  &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_all(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_MASK)); // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002E))
}

inline unsigned short sdhciReadSdh_timeout_ctrl_sw_resetTimeout_value(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16Val  &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_TIMEOUT_VALUE_MASK;         // mask contents to just the bitfield (mask=0x000F)
    return (w16Val);
}

inline void sdhciWriteSdh_timeout_ctrl_sw_resetTimeout_value(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 14) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_TIMEOUT_VALUE_MASK;              // mask the source data (mask=0x000F)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_TIMEOUT_VALUE_MASK));  // mask the Register data (mask=0x000F)
    mask &= ~(SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xF8F0)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x002E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x002E))
}

inline unsigned short sdhciReadSdh_i_stat(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))));     // return the contents of Addr=((pContext->base + 0x0030))
}

inline void sdhciWriteSdh_i_stat(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_I_STAT_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0030))
}

////////////////////////////////////////////////////////////////////
// Register SDH_I_STAT Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_i_statErr_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_ERR_INT_MASK;         // mask contents to just the bitfield (mask=0x8000)
    w16Val >>=   SDHCI_SDH_I_STAT_ERR_INT_SHIFT;        // shift the bitfield data down (shift=15)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_i_statRetuning_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_RETUNING_INT_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_I_STAT_RETUNING_INT_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statRetuning_int(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_RETUNING_INT_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_I_STAT_RETUNING_INT_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_RETUNING_INT_MASK));  // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statInt_c(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_INT_C_MASK;         // mask contents to just the bitfield (mask=0x0800)
    w16Val >>=   SDHCI_SDH_I_STAT_INT_C_SHIFT;        // shift the bitfield data down (shift=11)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statInt_c(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_INT_C_SHIFT;             // shift the data into position (shift=11)
    w16Val    &=   SDHCI_SDH_I_STAT_INT_C_MASK;              // mask the source data (mask=0x0800)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_INT_C_MASK)); // mask the Register data (mask=0x0800)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statInt_b(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_INT_B_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_I_STAT_INT_B_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statInt_b(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_INT_B_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_I_STAT_INT_B_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_INT_B_MASK)); // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statInt_a(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_INT_A_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_I_STAT_INT_A_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statInt_a(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_INT_A_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_I_STAT_INT_A_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_INT_A_MASK)); // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statCard_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_CARD_INT_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_I_STAT_CARD_INT_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_i_statCard_rem_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_CARD_REM_INT_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_I_STAT_CARD_REM_INT_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statCard_rem_int(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_CARD_REM_INT_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_I_STAT_CARD_REM_INT_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_CARD_REM_INT_MASK));  // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statCard_ins_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_CARD_INS_INT_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_I_STAT_CARD_INS_INT_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statCard_ins_int(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_CARD_INS_INT_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_I_STAT_CARD_INS_INT_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_CARD_INS_INT_MASK));  // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statRx_rdy(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_RX_RDY_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_I_STAT_RX_RDY_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statRx_rdy(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_RX_RDY_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_I_STAT_RX_RDY_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_RX_RDY_MASK));    // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statTx_rdy(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_TX_RDY_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_I_STAT_TX_RDY_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statTx_rdy(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_TX_RDY_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_I_STAT_TX_RDY_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_TX_RDY_MASK));    // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statDma_int(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_DMA_INT_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_I_STAT_DMA_INT_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statDma_int(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_DMA_INT_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_I_STAT_DMA_INT_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_DMA_INT_MASK));   // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statBlock_gap_event(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statBlock_gap_event(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_MASK));   // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statXfer_complete(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_XFER_COMPLETE_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_I_STAT_XFER_COMPLETE_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statXfer_complete(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_XFER_COMPLETE_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_I_STAT_XFER_COMPLETE_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_XFER_COMPLETE_MASK)); // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_i_statCmd_complete(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16Val  &=   SDHCI_SDH_I_STAT_CMD_COMPLETE_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_i_statCmd_complete(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_I_STAT_CMD_COMPLETE_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_CMD_COMPLETE_MASK));  // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_I_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x6000)
    mask &= ~(SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x1EFF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0030)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0030))
}

inline unsigned short sdhciReadSdh_e_stat(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))));     // return the contents of Addr=((pContext->base + 0x0032))
}

inline void sdhciWriteSdh_e_stat(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_E_STAT_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0032))
}

////////////////////////////////////////////////////////////////////
// Register SDH_E_STAT Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_e_statCrc_status_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CRC_STATUS_ERR_MASK;         // mask contents to just the bitfield (mask=0x8000)
    w16Val >>=   SDHCI_SDH_E_STAT_CRC_STATUS_ERR_SHIFT;        // shift the bitfield data down (shift=15)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCrc_status_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CRC_STATUS_ERR_SHIFT;             // shift the data into position (shift=15)
    w16Val    &=   SDHCI_SDH_E_STAT_CRC_STATUS_ERR_MASK;              // mask the source data (mask=0x8000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CRC_STATUS_ERR_MASK));    // mask the Register data (mask=0x8000)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCpl_timeout_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_MASK;         // mask contents to just the bitfield (mask=0x4000)
    w16Val >>=   SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCpl_timeout_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_MASK;              // mask the source data (mask=0x4000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_MASK));   // mask the Register data (mask=0x4000)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statAxi_resp_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_AXI_RESP_ERR_MASK;         // mask contents to just the bitfield (mask=0x2000)
    w16Val >>=   SDHCI_SDH_E_STAT_AXI_RESP_ERR_SHIFT;        // shift the bitfield data down (shift=13)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statAxi_resp_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_AXI_RESP_ERR_SHIFT;             // shift the data into position (shift=13)
    w16Val    &=   SDHCI_SDH_E_STAT_AXI_RESP_ERR_MASK;              // mask the source data (mask=0x2000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_AXI_RESP_ERR_MASK));  // mask the Register data (mask=0x2000)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statSpi_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_SPI_ERR_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_E_STAT_SPI_ERR_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statSpi_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_SPI_ERR_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_E_STAT_SPI_ERR_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_SPI_ERR_MASK));   // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statTune_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_TUNE_ERR_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_E_STAT_TUNE_ERR_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statTune_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_TUNE_ERR_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_E_STAT_TUNE_ERR_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_TUNE_ERR_MASK));  // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statAdma_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_ADMA_ERR_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_E_STAT_ADMA_ERR_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statAdma_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_ADMA_ERR_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_E_STAT_ADMA_ERR_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_ADMA_ERR_MASK));  // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statAuto_cmd12_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statAuto_cmd12_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_MASK));    // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCur_limit_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCur_limit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_MASK)); // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statRd_data_end_bit_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statRd_data_end_bit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_MASK));   // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statRd_data_crc_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statRd_data_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_MASK));   // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statData_timeout_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statData_timeout_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_MASK));  // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCmd_index_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CMD_INDEX_ERR_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_E_STAT_CMD_INDEX_ERR_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCmd_index_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CMD_INDEX_ERR_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_E_STAT_CMD_INDEX_ERR_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CMD_INDEX_ERR_MASK)); // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCmd_end_bit_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCmd_end_bit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_MASK));   // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCmd_crc_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CMD_CRC_ERR_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_E_STAT_CMD_CRC_ERR_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCmd_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_CMD_CRC_ERR_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_E_STAT_CMD_CRC_ERR_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CMD_CRC_ERR_MASK));   // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_e_statCmd_timeout_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16Val  &=   SDHCI_SDH_E_STAT_CMD_TIMEOUT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_e_statCmd_timeout_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_E_STAT_CMD_TIMEOUT_ERR_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_CMD_TIMEOUT_ERR_MASK));   // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_E_STAT_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    mask &= ~(SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0xF7FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0032)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0032))
}

inline unsigned short sdhciReadSdh_i_stat_en(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))));     // return the contents of Addr=((pContext->base + 0x0034))
}

inline void sdhciWriteSdh_i_stat_en(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0034))
}

////////////////////////////////////////////////////////////////////
// Register SDH_I_STAT_EN Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_i_stat_enRetune_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enRetune_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_MASK));  // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enInt_c_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x0800)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_SHIFT;        // shift the bitfield data down (shift=11)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enInt_c_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_SHIFT;             // shift the data into position (shift=11)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_MASK;              // mask the source data (mask=0x0800)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_MASK));   // mask the Register data (mask=0x0800)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enInt_b_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enInt_b_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_MASK));   // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enInt_a_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enInt_a_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_MASK));   // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enCard_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_CARD_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_CARD_INT_EN_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enCard_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_CARD_INT_EN_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_CARD_INT_EN_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_CARD_INT_EN_MASK));    // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enCard_rem_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_CARD_REM_EN_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_CARD_REM_EN_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enCard_rem_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_CARD_REM_EN_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_CARD_REM_EN_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_CARD_REM_EN_MASK));    // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enCard_ins_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_CARD_INS_EN_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_CARD_INS_EN_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enCard_ins_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_CARD_INS_EN_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_CARD_INS_EN_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_CARD_INS_EN_MASK));    // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enRx_rdy_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_RX_RDY_EN_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_RX_RDY_EN_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enRx_rdy_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_RX_RDY_EN_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_RX_RDY_EN_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_RX_RDY_EN_MASK));  // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enTx_rdy_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_TX_RDY_EN_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_TX_RDY_EN_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enTx_rdy_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_TX_RDY_EN_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_TX_RDY_EN_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_TX_RDY_EN_MASK));  // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enDma_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_DMA_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_DMA_INT_EN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enDma_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_DMA_INT_EN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_DMA_INT_EN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_DMA_INT_EN_MASK)); // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enBgapevnt_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enBgapevnt_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_MASK));    // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enXfrcomp_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enXfrcomp_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_MASK)); // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_i_stat_enCmdcomp_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16Val  &=   SDHCI_SDH_I_STAT_EN_CMDCOMP_EN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_i_stat_enCmdcomp_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_I_STAT_EN_CMDCOMP_EN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_STAT_EN_CMDCOMP_EN_MASK)); // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0034)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0034))
}

inline unsigned short sdhciReadSdh_e_stat_en(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))));     // return the contents of Addr=((pContext->base + 0x0036))
}

inline void sdhciWriteSdh_e_stat_en(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0036))
}

////////////////////////////////////////////////////////////////////
// Register SDH_E_STAT_EN Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_e_stat_enCrc_stat_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x8000)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_SHIFT;        // shift the bitfield data down (shift=15)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCrc_stat_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_SHIFT;             // shift the data into position (shift=15)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_MASK;              // mask the source data (mask=0x8000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_MASK));    // mask the Register data (mask=0x8000)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCpl_to_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x4000)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCpl_to_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_MASK;              // mask the source data (mask=0x4000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_MASK));  // mask the Register data (mask=0x4000)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enAxi_resp_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x2000)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_SHIFT;        // shift the bitfield data down (shift=13)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enAxi_resp_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_SHIFT;             // shift the data into position (shift=13)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_MASK;              // mask the source data (mask=0x2000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_MASK));    // mask the Register data (mask=0x2000)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enSpi_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enSpi_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_MASK)); // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enTuning_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enTuning_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_MASK));  // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enAdma_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enAdma_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_MASK));    // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enAc12_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enAc12_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_MASK));    // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCurlmterr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCurlmterr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_MASK));   // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enDataenderr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enDataenderr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_MASK));  // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enDatacrcerr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enDatacrcerr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_MASK));  // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enData_toerr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enData_toerr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_MASK));  // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCmd_idxerr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCmd_idxerr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_MASK));  // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCmd_enderr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCmd_enderr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_MASK));  // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCmd_crcerr_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCmd_crcerr_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_MASK));  // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_e_stat_enCmd_to_err_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16Val  &=   SDHCI_SDH_E_STAT_EN_CMD_TO_ERR_EN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_e_stat_enCmd_to_err_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_E_STAT_EN_CMD_TO_ERR_EN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_STAT_EN_CMD_TO_ERR_EN_MASK));  // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0036)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0036))
}

inline unsigned short sdhciReadSdh_i_sign_en(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))));     // return the contents of Addr=((pContext->base + 0x0038))
}

inline void sdhciWriteSdh_i_sign_en(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0038))
}

////////////////////////////////////////////////////////////////////
// Register SDH_I_SIGN_EN Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_i_sign_enRetune_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enRetune_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_MASK));    // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enInt_c_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0800)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=11)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enInt_c_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_SHIFT;             // shift the data into position (shift=11)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_MASK;              // mask the source data (mask=0x0800)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_MASK)); // mask the Register data (mask=0x0800)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enInt_b_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enInt_b_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_MASK)); // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enInt_a_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enInt_a_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_MASK)); // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enCard_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enCard_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_MASK));  // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enCard_rem_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enCard_rem_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_MASK));  // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enCard_ins_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enCard_ins_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_MASK));  // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enRx_rdy_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enRx_rdy_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_MASK));    // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enTx_rdy_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enTx_rdy_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_MASK));    // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enDma_int_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enDma_int_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_MASK));   // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enBgapevnt_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enBgapevnt_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_MASK));  // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enXfrcomp_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enXfrcomp_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_MASK));   // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_i_sign_enCmdcomp_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16Val  &=   SDHCI_SDH_I_SIGN_EN_CMDCOMP_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_i_sign_enCmdcomp_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_I_SIGN_EN_CMDCOMP_SGEN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_I_SIGN_EN_CMDCOMP_SGEN_MASK));   // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xE000)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0038)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0038))
}

inline unsigned short sdhciReadSdh_e_sign_en(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))));     // return the contents of Addr=((pContext->base + 0x003A))
}

inline void sdhciWriteSdh_e_sign_en(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16Val;      // write the data into the register at ((pContext->base + 0x003A))
}

////////////////////////////////////////////////////////////////////
// Register SDH_E_SIGN_EN Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_e_sign_enCrc_stat_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x8000)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=15)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCrc_stat_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_SHIFT;             // shift the data into position (shift=15)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_MASK;              // mask the source data (mask=0x8000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_MASK));  // mask the Register data (mask=0x8000)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCpl_to_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x4000)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCpl_to_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_MASK;              // mask the source data (mask=0x4000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_MASK));    // mask the Register data (mask=0x4000)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enAxi_resp_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x2000)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=13)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enAxi_resp_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_SHIFT;             // shift the data into position (shift=13)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_MASK;              // mask the source data (mask=0x2000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_MASK));  // mask the Register data (mask=0x2000)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enSpi_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enSpi_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_MASK));   // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enTune_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enTune_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_SHIFT;             // shift the data into position (shift=10)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_MASK;              // mask the source data (mask=0x0400)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_MASK));  // mask the Register data (mask=0x0400)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enAdma_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enAdma_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_MASK));  // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enAc12_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enAc12_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_MASK));  // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCurlmterr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCurlmterr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_MASK)); // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enDataenderr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enDataenderr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_MASK));    // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enDatacrcerr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enDatacrcerr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_MASK));    // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enData_toerr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enData_toerr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_MASK));    // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCmd_idxerr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCmd_idxerr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_MASK));    // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCmd_enderr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCmd_enderr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_MASK));    // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCmd_crcerr_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCmd_crcerr_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_MASK));    // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_e_sign_enCmd_to_err_sgen(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16Val  &=   SDHCI_SDH_E_SIGN_EN_CMD_TO_ERR_SGEN_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_e_sign_enCmd_to_err_sgen(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_E_SIGN_EN_CMD_TO_ERR_SGEN_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_E_SIGN_EN_CMD_TO_ERR_SGEN_MASK));    // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003A)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003A))
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_status(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))));     // return the contents of Addr=((pContext->base + 0x003C))
}

inline void sdhciWriteSdh_auto_cmd12_error_status(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16Val;      // write the data into the register at ((pContext->base + 0x003C))
}

////////////////////////////////////////////////////////////////////
// Register SDH_AUTO_CMD12_ERROR_STATUS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusCmd_not_issued(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_CMD_NOT_ISSUED_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_CMD_NOT_ISSUED_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_index_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_index_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_MASK));   // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x001F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003C))
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_end_bit_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_end_bit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_MASK)); // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x001F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003C))
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_crc_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_MASK)); // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x001F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003C))
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_timeout_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline void sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_timeout_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_MASK)); // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x001F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003C))
}

inline unsigned short sdhciReadSdh_auto_cmd12_error_statusAuto_cmd12_not_exe(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16Val  &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD12_NOT_EXE_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline void sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd12_not_exe(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD12_NOT_EXE_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD12_NOT_EXE_MASK));   // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK); // clear set2clear bits that came in (mask=0x001F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003C)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003C))
}

inline unsigned short sdhciReadSdh_host_ctrl_2(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))));     // return the contents of Addr=((pContext->base + 0x003E))
}

inline void sdhciWriteSdh_host_ctrl_2(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16Val;      // write the data into the register at ((pContext->base + 0x003E))
}

////////////////////////////////////////////////////////////////////
// Register SDH_HOST_CTRL_2 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_host_ctrl_2Pre_val_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_MASK;         // mask contents to just the bitfield (mask=0x8000)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_SHIFT;        // shift the bitfield data down (shift=15)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Pre_val_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_SHIFT;             // shift the data into position (shift=15)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_MASK;              // mask the source data (mask=0x8000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_MASK));   // mask the Register data (mask=0x8000)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Async_int_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_MASK;         // mask contents to just the bitfield (mask=0x4000)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Async_int_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_MASK;              // mask the source data (mask=0x4000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_MASK)); // mask the Register data (mask=0x4000)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Sampling_clk_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Sampling_clk_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_MASK)); // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Exe_tuning(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Exe_tuning(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_MASK));   // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Drv_strength_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_MASK;         // mask contents to just the bitfield (mask=0x0030)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Drv_strength_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_MASK;              // mask the source data (mask=0x0030)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_MASK)); // mask the Register data (mask=0x0030)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Sdh_v18_en(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Sdh_v18_en(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_MASK));   // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_host_ctrl_2Uhs_mode_sel(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16Val  &=   SDHCI_SDH_HOST_CTRL_2_UHS_MODE_SEL_MASK;         // mask contents to just the bitfield (mask=0x0007)
    return (w16Val);
}

inline void sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 6) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_HOST_CTRL_2_UHS_MODE_SEL_MASK;              // mask the source data (mask=0x0007)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_HOST_CTRL_2_UHS_MODE_SEL_MASK)); // mask the Register data (mask=0x0007)
    mask &= ~(SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x3F00)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x003E)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x003E))
}

inline unsigned short sdhciReadSdh_capabilities_1(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0040)))));     // return the contents of Addr=((pContext->base + 0x0040))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_1 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_capabilities_1Base_freq(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0040))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_1_BASE_FREQ_MASK;         // mask contents to just the bitfield (mask=0xFF00)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_1_BASE_FREQ_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_1Timeout_unit(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0040))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_1_TIMEOUT_UNIT_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_1_TIMEOUT_UNIT_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_1Timeout_freq(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0040))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_1_TIMEOUT_FREQ_MASK;         // mask contents to just the bitfield (mask=0x003F)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042)))));     // return the contents of Addr=((pContext->base + 0x0042))
}

inline void sdhciWriteSdh_capabilities_2(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_CAPABILITIES_2_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0042))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_2 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_capabilities_2Cfg_slot_type(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline void sdhciWriteSdh_capabilities_2Cfg_slot_type(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 2) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_MASK;              // mask the source data (mask=0xC000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_MASK)); // mask the Register data (mask=0xC000)
    mask &= ~(SDHCI_SDH_CAPABILITIES_2_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0800)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0042))
}

inline unsigned short sdhciReadSdh_capabilities_2Async_int_supp(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_ASYNC_INT_SUPP_MASK;         // mask contents to just the bitfield (mask=0x2000)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_ASYNC_INT_SUPP_SHIFT;        // shift the bitfield data down (shift=13)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Sys_bus_64_supp(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_SYS_BUS_64_SUPP_MASK;         // mask contents to just the bitfield (mask=0x1000)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_SYS_BUS_64_SUPP_SHIFT;        // shift the bitfield data down (shift=12)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Vlg_18_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_VLG_18_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_VLG_18_SUPPORT_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Vlg_30_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_VLG_30_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0200)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_VLG_30_SUPPORT_SHIFT;        // shift the bitfield data down (shift=9)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Vlg_33_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_VLG_33_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0100)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_VLG_33_SUPPORT_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Sus_res_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_SUS_RES_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0080)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_SUS_RES_SUPPORT_SHIFT;        // shift the bitfield data down (shift=7)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Sdma_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_SDMA_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_SDMA_SUPPORT_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Hi_speed_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_HI_SPEED_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_HI_SPEED_SUPPORT_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Adma1_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_ADMA1_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_ADMA1_SUPPORT_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Adma2_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_ADMA2_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_ADMA2_SUPPORT_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Ex_data_width_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_EX_DATA_WIDTH_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_2_EX_DATA_WIDTH_SUPPORT_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_2Max_blk_len(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0042))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_2_MAX_BLK_LEN_MASK;         // mask contents to just the bitfield (mask=0x0003)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044)))));     // return the contents of Addr=((pContext->base + 0x0044))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_3 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_capabilities_3Retune_modes(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_RETUNE_MODES_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_RETUNE_MODES_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Sdr50_tune(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_SDR50_TUNE_MASK;         // mask contents to just the bitfield (mask=0x2000)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_SDR50_TUNE_SHIFT;        // shift the bitfield data down (shift=13)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Tmr_retune(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_TMR_RETUNE_MASK;         // mask contents to just the bitfield (mask=0x0F00)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_TMR_RETUNE_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Drv_type_d(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_D_MASK;         // mask contents to just the bitfield (mask=0x0040)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_D_SHIFT;        // shift the bitfield data down (shift=6)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Drv_type_c(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_C_MASK;         // mask contents to just the bitfield (mask=0x0020)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_C_SHIFT;        // shift the bitfield data down (shift=5)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Drv_type_a(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_A_MASK;         // mask contents to just the bitfield (mask=0x0010)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_A_SHIFT;        // shift the bitfield data down (shift=4)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Ddr50_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_DDR50_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_DDR50_SUPPORT_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Sdr104_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_SDR104_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_CAPABILITIES_3_SDR104_SUPPORT_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_3Sdr50_support(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0044))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_3_SDR50_SUPPORT_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_capabilities_4(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0046)))));     // return the contents of Addr=((pContext->base + 0x0046))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_4 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_capabilities_4Clk_multiplier(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0046))))));
    w16Val  &=   SDHCI_SDH_CAPABILITIES_4_CLK_MULTIPLIER_MASK;         // mask contents to just the bitfield (mask=0x00FF)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_max_current_1(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0048)))));     // return the contents of Addr=((pContext->base + 0x0048))
}

////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_1 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_max_current_1Max_cur_30(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0048))))));
    w16Val  &=   SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_30_MASK;         // mask contents to just the bitfield (mask=0xFF00)
    w16Val >>=   SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_30_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_max_current_1Max_cur_33(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0048))))));
    w16Val  &=   SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_33_MASK;         // mask contents to just the bitfield (mask=0x00FF)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_max_current_2(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x004A)))));     // return the contents of Addr=((pContext->base + 0x004A))
}

////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_2 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_max_current_2Max_cur_18(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x004A))))));
    w16Val  &=   SDHCI_SDH_MAX_CURRENT_2_MAX_CUR_18_MASK;         // mask contents to just the bitfield (mask=0x00FF)
    return (w16Val);
}

////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_3 Bitfield access functions
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_4 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline void sdhciWriteSdh_force_event_acmd12err(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow = w16Val; // update the shadow register
}

////////////////////////////////////////////////////////////////////
// Register SDH_FORCE_EVENT_ACMD12ERR Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_issue_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_issue_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_MASK)); // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_index_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_index_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_MASK)); // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_ebit_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_ebit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_MASK));  // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_crc_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_MASK));   // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_to_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_to_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_MASK));    // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_acmd12errF_acmd12_nexe_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_acmd12errShadow;               // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_NEXE_ERR_MASK; // mask off the bitfield
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_acmd12errF_acmd12_nexe_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_NEXE_ERR_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_NEXE_ERR_MASK));  // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFF60)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK);           // clear the w/o bits returned by register access (mask=0x009F)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0050)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0050))
    gw16sdhciSdh_force_event_acmd12errShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_NEXE_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_acmd12errShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline void sdhciWriteSdh_force_event_error_status(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow = w16Val; // update the shadow register
}

////////////////////////////////////////////////////////////////////
// Register SDH_FORCE_EVENT_ERROR_STATUS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_force_event_error_statusF_crc_status_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_crc_status_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_SHIFT;             // shift the data into position (shift=15)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_MASK;              // mask the source data (mask=0x8000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_MASK));    // mask the Register data (mask=0x8000)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_cpl_timeout_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_cpl_timeout_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_SHIFT;             // shift the data into position (shift=14)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_MASK;              // mask the source data (mask=0x4000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_MASK));   // mask the Register data (mask=0x4000)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_axi_resp_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_axi_resp_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_SHIFT;             // shift the data into position (shift=13)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_MASK;              // mask the source data (mask=0x2000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_MASK));  // mask the Register data (mask=0x2000)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_spi_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_spi_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_SHIFT;             // shift the data into position (shift=12)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_MASK;              // mask the source data (mask=0x1000)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_MASK));   // mask the Register data (mask=0x1000)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_adma_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_adma_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_SHIFT;             // shift the data into position (shift=9)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_MASK;              // mask the source data (mask=0x0200)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_MASK));  // mask the Register data (mask=0x0200)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_acmd12_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_acmd12_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_SHIFT;             // shift the data into position (shift=8)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_MASK;              // mask the source data (mask=0x0100)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_MASK));    // mask the Register data (mask=0x0100)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_current_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_current_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_SHIFT;             // shift the data into position (shift=7)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_MASK;              // mask the source data (mask=0x0080)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_MASK));   // mask the Register data (mask=0x0080)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_dat_end_bit_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_dat_end_bit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_SHIFT;             // shift the data into position (shift=6)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_MASK;              // mask the source data (mask=0x0040)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_MASK));   // mask the Register data (mask=0x0040)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_dat_crc_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_dat_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_SHIFT;             // shift the data into position (shift=5)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_MASK;              // mask the source data (mask=0x0020)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_MASK));   // mask the Register data (mask=0x0020)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_dat_to_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_dat_to_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_SHIFT;             // shift the data into position (shift=4)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_MASK;              // mask the source data (mask=0x0010)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_MASK));    // mask the Register data (mask=0x0010)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_cmd_index_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_cmd_index_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_SHIFT;             // shift the data into position (shift=3)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_MASK;              // mask the source data (mask=0x0008)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_MASK)); // mask the Register data (mask=0x0008)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_cmd_end_bit_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_cmd_end_bit_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_MASK));   // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_cmd_crc_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_MASK; // mask off the bitfield
    w16Val >>= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_SHIFT;        // shift into position
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_cmd_crc_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_SHIFT;             // shift the data into position (shift=1)
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_MASK;              // mask the source data (mask=0x0002)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_MASK));   // mask the Register data (mask=0x0002)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_force_event_error_statusF_cmd_to_errShadow(void)
{
    unsigned short w16Val = gw16sdhciSdh_force_event_error_statusShadow;                // get Shadow register contents
    w16Val &= SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_TO_ERR_MASK; // mask off the bitfield
    return (w16Val);   // return the shadow information
}

inline void sdhciWriteSdh_force_event_error_statusF_cmd_to_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_TO_ERR_MASK;              // mask the source data (mask=0x0001)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_TO_ERR_MASK));    // mask the Register data (mask=0x0001)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x0C00)
    mask &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK);           // clear the w/o bits returned by register access (mask=0xF3FF)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0052)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0052))
    gw16sdhciSdh_force_event_error_statusShadow   &= ~(SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_TO_ERR_MASK);             // clear the bitfield span to 0 in the shadow register
    gw16sdhciSdh_force_event_error_statusShadow   |= w16Val;        // or in the new data for the bitfield into the shadow register
}

inline unsigned short sdhciReadSdh_adma_error_status(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054)))));     // return the contents of Addr=((pContext->base + 0x0054))
}

inline void sdhciWriteSdh_adma_error_status(struct mmc_host *pContext, unsigned short w16Val)
{
    if (w16Val & SDHCI_SDH_ADMA_ERROR_STATUS_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054)))) = w16Val;      // write the data into the register at ((pContext->base + 0x0054))
}

////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_ERROR_STATUS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_adma_error_statusAdma_len_err(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054))))));
    w16Val  &=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline void sdhciWriteSdh_adma_error_statusAdma_len_err(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val <= 1) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val   <<=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_SHIFT;             // shift the data into position (shift=2)
    w16Val    &=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_MASK;              // mask the source data (mask=0x0004)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_MASK));   // mask the Register data (mask=0x0004)
    mask &= ~(SDHCI_SDH_ADMA_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFF8)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0054))
}

inline unsigned short sdhciReadSdh_adma_error_statusAdma_state(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054))))));
    w16Val  &=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_STATE_MASK;         // mask contents to just the bitfield (mask=0x0003)
    return (w16Val);
}

inline void sdhciWriteSdh_adma_error_statusAdma_state(struct mmc_host *pContext,  unsigned short w16Val)
{
    unsigned short w16RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w16Val == 0) {
        iInRange = 1;
    }
    if (w16Val == 1) {
        iInRange = 1;
    }
    if (w16Val == 3) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w16Val    &=   SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_STATE_MASK;              // mask the source data (mask=0x0003)
    unsigned short mask = (unsigned short)(~(SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_STATE_MASK)); // mask the Register data (mask=0x0003)
    mask &= ~(SDHCI_SDH_ADMA_ERROR_STATUS_RESERVED_0_MASK);   // force reserved0 bits low (mask=0xFFF8)
    w16RegVal = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054))))));
    w16RegVal &= mask;
    w16RegVal |= w16Val;                             // combine the new data with the register contents
    *((volatile unsigned short *)(uintptr_t)((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0054)))) = w16RegVal;      // write the data into the register at ((pContext->base + 0x0054))
}

inline unsigned int sdhciReadSdh_adma_sysaddr0(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0058)))));     // return the contents of Addr=((pContext->base + 0x0058))
}

inline void sdhciWriteSdh_adma_sysaddr0(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0058)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0058))
}

////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_SYSADDR0 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_adma_sysaddr0Admasysaddr(struct mmc_host *pContext)
{
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0058))))));
}

inline void sdhciWriteSdh_adma_sysaddr0Admasysaddr(struct mmc_host *pContext,  unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0058)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0058))
}

inline unsigned int sdhciReadSdh_adma_sysaddr1(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x005C)))));     // return the contents of Addr=((pContext->base + 0x005C))
}

inline void sdhciWriteSdh_adma_sysaddr1(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x005C)))) = w32Val;      // write the data into the register at ((pContext->base + 0x005C))
}

////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_SYSADDR1 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadSdh_adma_sysaddr1Admasysaddr(struct mmc_host *pContext)
{
    return (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x005C))))));
}

inline void sdhciWriteSdh_adma_sysaddr1Admasysaddr(struct mmc_host *pContext,  unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x005C)))) = w32Val;      // write the data into the register at ((pContext->base + 0x005C))
}

inline unsigned short sdhciReadPreset_value_for_init(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0060)))));     // return the contents of Addr=((pContext->base + 0x0060))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_INIT Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_initDrv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0060))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_INIT_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_INIT_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_initClkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0060))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_INIT_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_INIT_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_initClk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0060))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_INIT_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_ds(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0062)))));     // return the contents of Addr=((pContext->base + 0x0062))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_DS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_dsDrv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0062))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DS_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_DS_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_dsClkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0062))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DS_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_DS_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_dsClk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0062))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DS_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_hs(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0064)))));     // return the contents of Addr=((pContext->base + 0x0064))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_HS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_hsDrv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0064))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_HS_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_HS_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_hsClkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0064))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_HS_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_HS_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_hsClk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0064))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_HS_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr12(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0066)))));     // return the contents of Addr=((pContext->base + 0x0066))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR12 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_sdr12Drv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0066))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR12_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR12_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr12Clkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0066))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR12_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR12_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr12Clk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0066))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR12_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr25(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0068)))));     // return the contents of Addr=((pContext->base + 0x0068))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR25 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_sdr25Drv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0068))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR25_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR25_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr25Clkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0068))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR25_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR25_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr25Clk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x0068))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR25_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr50(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006A)))));     // return the contents of Addr=((pContext->base + 0x006A))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR50 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_sdr50Drv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006A))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR50_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR50_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr50Clkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006A))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR50_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR50_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr50Clk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006A))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR50_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr104(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006C)))));     // return the contents of Addr=((pContext->base + 0x006C))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR104 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_sdr104Drv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006C))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR104_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR104_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr104Clkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006C))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR104_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_SDR104_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_sdr104Clk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006C))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_SDR104_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_ddr50(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006E)))));     // return the contents of Addr=((pContext->base + 0x006E))
}

////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_DDR50 Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadPreset_value_for_ddr50Drv_strength_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006E))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DDR50_DRV_STRENGTH_VAL_MASK;         // mask contents to just the bitfield (mask=0xC000)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_DDR50_DRV_STRENGTH_VAL_SHIFT;        // shift the bitfield data down (shift=14)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_ddr50Clkgen_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006E))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DDR50_CLKGEN_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x0400)
    w16Val >>=   SDHCI_PRESET_VALUE_FOR_DDR50_CLKGEN_SEL_VAL_SHIFT;        // shift the bitfield data down (shift=10)
    return (w16Val);
}

inline unsigned short sdhciReadPreset_value_for_ddr50Clk_freq_sel_val(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x006E))))));
    w16Val  &=   SDHCI_PRESET_VALUE_FOR_DDR50_CLK_FREQ_SEL_VAL_MASK;         // mask contents to just the bitfield (mask=0x03FF)
    return (w16Val);
}

inline unsigned int sdhciReadShared_bus_ctrl(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0)))));     // return the contents of Addr=((pContext->base + 0x00E0))
}

inline void sdhciWriteShared_bus_ctrl(struct mmc_host *pContext, unsigned int w32Val)
{
    if (w32Val & SDHCI_SHARED_BUS_CTRL_RESERVED_0_MASK) {
        sdhciAccessLayerRangeError("%s - Attempt to alter reserved bits", __FUNCTION__);
    }
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0)))) = w32Val;      // write the data into the register at ((pContext->base + 0x00E0))
}

////////////////////////////////////////////////////////////////////
// Register SHARED_BUS_CTRL Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned int sdhciReadShared_bus_ctrlBend_pwr_ctrl(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_MASK;         // mask contents to just the bitfield (mask=0x7F000000)
    w32Val >>=   SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_SHIFT;        // shift the bitfield data down (shift=24)
    return (w32Val);
}

inline void sdhciWriteShared_bus_ctrlBend_pwr_ctrl(struct mmc_host *pContext,  unsigned int w32Val)
{
    unsigned int w32RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w32Val <= 127) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w32Val   <<=   SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_SHIFT;             // shift the data into position (shift=24)
    w32Val    &=   SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_MASK;              // mask the source data (mask=0x7F000000)
    unsigned int mask = (unsigned int)(~(SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_MASK));    // mask the Register data (mask=0x7F000000)
    mask &= ~(SDHCI_SHARED_BUS_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x808880C8)
    w32RegVal = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32RegVal &= mask;
    w32RegVal |= w32Val;                             // combine the new data with the register contents
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0)))) = w32RegVal;      // write the data into the register at ((pContext->base + 0x00E0))
}

inline unsigned int sdhciReadShared_bus_ctrlInt_pin_sel(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_MASK;         // mask contents to just the bitfield (mask=0x00700000)
    w32Val >>=   SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_SHIFT;        // shift the bitfield data down (shift=20)
    return (w32Val);
}

inline void sdhciWriteShared_bus_ctrlInt_pin_sel(struct mmc_host *pContext,  unsigned int w32Val)
{
    unsigned int w32RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w32Val <= 7) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w32Val   <<=   SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_SHIFT;             // shift the data into position (shift=20)
    w32Val    &=   SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_MASK;              // mask the source data (mask=0x00700000)
    unsigned int mask = (unsigned int)(~(SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_MASK));  // mask the Register data (mask=0x00700000)
    mask &= ~(SDHCI_SHARED_BUS_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x808880C8)
    w32RegVal = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32RegVal &= mask;
    w32RegVal |= w32Val;                             // combine the new data with the register contents
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0)))) = w32RegVal;      // write the data into the register at ((pContext->base + 0x00E0))
}

inline unsigned int sdhciReadShared_bus_ctrlClk_pin_sel(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_MASK;         // mask contents to just the bitfield (mask=0x00070000)
    w32Val >>=   SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_SHIFT;        // shift the bitfield data down (shift=16)
    return (w32Val);
}

inline void sdhciWriteShared_bus_ctrlClk_pin_sel(struct mmc_host *pContext,  unsigned int w32Val)
{
    unsigned int w32RegVal;
    int iInRange = 0;       // check that the value to be sent to the bitfield is in range
    if (w32Val <= 7) {
        iInRange = 1;
    }
    if (iInRange == 0) {
        sdhciAccessLayerRangeError("%s Bitfield Not In Range", __FUNCTION__);    // call the error handler
    }
    w32Val   <<=   SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_SHIFT;             // shift the data into position (shift=16)
    w32Val    &=   SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_MASK;              // mask the source data (mask=0x00070000)
    unsigned int mask = (unsigned int)(~(SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_MASK));  // mask the Register data (mask=0x00070000)
    mask &= ~(SDHCI_SHARED_BUS_CTRL_RESERVED_0_MASK);   // force reserved0 bits low (mask=0x808880C8)
    w32RegVal = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32RegVal &= mask;
    w32RegVal |= w32Val;                             // combine the new data with the register contents
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0)))) = w32RegVal;      // write the data into the register at ((pContext->base + 0x00E0))
}

inline unsigned int sdhciReadShared_bus_ctrlBus_width_preset(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_MASK;         // mask contents to just the bitfield (mask=0x00007F00)
    w32Val >>=   SDHCI_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_SHIFT;        // shift the bitfield data down (shift=8)
    return (w32Val);
}

inline unsigned int sdhciReadShared_bus_ctrlNum_int_pins(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_NUM_INT_PINS_MASK;         // mask contents to just the bitfield (mask=0x00000030)
    w32Val >>=   SDHCI_SHARED_BUS_CTRL_NUM_INT_PINS_SHIFT;        // shift the bitfield data down (shift=4)
    return (w32Val);
}

inline unsigned int sdhciReadShared_bus_ctrlNum_clk_pins(struct mmc_host *pContext)
{
    unsigned int w32Val;
    w32Val = (*((volatile unsigned int *)(uintptr_t)(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x00E0))))));
    w32Val  &=   SDHCI_SHARED_BUS_CTRL_NUM_CLK_PINS_MASK;         // mask contents to just the bitfield (mask=0x00000007)
    return (w32Val);
}

inline unsigned short sdhciReadSdh_slot_int_status(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FC)))));     // return the contents of Addr=((pContext->base + 0x00FC))
}

////////////////////////////////////////////////////////////////////
// Register SDH_SLOT_INT_STATUS Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_slot_int_statusSlot_int3(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FC))))));
    w16Val  &=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT3_MASK;         // mask contents to just the bitfield (mask=0x0008)
    w16Val >>=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT3_SHIFT;        // shift the bitfield data down (shift=3)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_slot_int_statusSlot_int2(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FC))))));
    w16Val  &=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT2_MASK;         // mask contents to just the bitfield (mask=0x0004)
    w16Val >>=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT2_SHIFT;        // shift the bitfield data down (shift=2)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_slot_int_statusSlot_int1(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FC))))));
    w16Val  &=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT1_MASK;         // mask contents to just the bitfield (mask=0x0002)
    w16Val >>=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT1_SHIFT;        // shift the bitfield data down (shift=1)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_slot_int_statusSlot_int0(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FC))))));
    w16Val  &=   SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT0_MASK;         // mask contents to just the bitfield (mask=0x0001)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_ctrl_ver(struct mmc_host *pContext)
{
    return (*(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FE)))));     // return the contents of Addr=((pContext->base + 0x00FE))
}

////////////////////////////////////////////////////////////////////
// Register SDH_CTRL_VER Bitfield access functions
////////////////////////////////////////////////////////////////////

inline unsigned short sdhciReadSdh_ctrl_verVendor_ver(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FE))))));
    w16Val  &=   SDHCI_SDH_CTRL_VER_VENDOR_VER_MASK;         // mask contents to just the bitfield (mask=0xFF00)
    w16Val >>=   SDHCI_SDH_CTRL_VER_VENDOR_VER_SHIFT;        // shift the bitfield data down (shift=8)
    return (w16Val);
}

inline unsigned short sdhciReadSdh_ctrl_verHost_ver(struct mmc_host *pContext)
{
    unsigned short w16Val;
    w16Val = (*((volatile unsigned short *)(uintptr_t)(((volatile unsigned short *)(uintptr_t)((pContext->base + 0x00FE))))));
    w16Val  &=   SDHCI_SDH_CTRL_VER_HOST_VER_MASK;         // mask contents to just the bitfield (mask=0x00FF)
    return (w16Val);
}

inline unsigned int sdhciReadSDHC_DLINE_CTRL(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0130)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_DLINE_CTRL(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0130)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_DLINE_CFG(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0134)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_DLINE_CFG(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0134)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSdhc_bypass_cfg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0138)))));     // return the contents of Addr=((pContext->base + 0x0138)) // write the data into the register at ((pContext->base + 0x0138))
}

inline void sdhciWriteSdhc_bypass_cfg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0138)))) = w32Val; // write the data into the register at ((pContext->base + 0x0138))
}

inline unsigned int sdhciReadSDHC_PHY_CTRL(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0160)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_CTRL(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0160)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_FUNC(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0164)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_FUNC(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0164)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_DLLCFG(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0168)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_DLLCFG(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0168)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_DLLCFG1(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x016C)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_DLLCFG1(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x016C)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_DLLSTS(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0170)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_DLLSTS1(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0174)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_PADCFG(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0178)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_PADCFG(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0178)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_PADCFG1(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x017C)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_PADCFG1(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x017C)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_LBCTRL(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0180)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_LBCTRL(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0180)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_LBFUNC(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0184)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_LBFUNC(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0184)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_LBCNT(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0188)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_LBCNT(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0188)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_LBSTS(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x018C)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_PHY_LBDATA(struct mmc_host *pContext)
{
    return 0;//Does not support phy,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0190)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_PHY_LBDATA(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support phy,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0190)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_CQE_CQBDCTRL(struct mmc_host *pContext)
{
    return 0;//Does not support,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x01F0)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_CQE_CQBDCTRL(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x01F0)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}

inline unsigned int sdhciReadSDHC_CQE_CQDINFO(struct mmc_host *pContext)
{
    return 0;//Does not support,bes,2021-04-28
    //return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x01F4)))));     // return the contents of Addr=((pContext->base + 0x0130))
}

inline void sdhciWriteSDHC_CQE_CQDINFO(struct mmc_host *pContext, unsigned int w32Val)
{
    return ;//Does not support,bes,2021-04-28
    //*((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x01F4)))) = w32Val; // write the data into the register at ((pContext->base + 0x0130))
}


inline unsigned int sdhciReadSdhc_gpio_cfg_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x012C)))));     // return the contents of Addr=((pContext->base + 0x012C))
}

inline void sdhciWriteSdhc_gpio_cfg_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x012C)))) = w32Val;      // write the data into the register at ((pContext->base + 0x012C))
}

inline unsigned int sdhciReadSdhc_roundtrip_timing_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0128)))));     // return the contents of Addr=((pContext->base + 0x0128))
}

inline void sdhciWriteSdhc_roundtrip_timing_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0128)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0128))
}

inline unsigned int sdhciReadSdhc_hwtune_cfg2_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0124)))));     // return the contents of Addr=((pContext->base + 0x0124))
}

inline void sdhciWriteSdhc_hwtune_cfg2_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0124)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0124))
}

inline unsigned int sdhciReadSdhc_hwtune_cfg_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0120)))));     // return the contents of Addr=((pContext->base + 0x0120))
}

inline void sdhciWriteSdhc_hwtune_cfg_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0120)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0120))
}

inline unsigned int sdhciReadSdhc_tx_cfg_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x011C)))));     // return the contents of Addr=((pContext->base + 0x011C))
}

inline  void sdhciWriteSdhc_tx_cfg_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x011C)))) = w32Val;      // write the data into the register at ((pContext->base + 0x011C))
}

inline unsigned int sdhciReadSdhc_rx_cfg_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0118)))));     // return the contents of Addr=((pContext->base + 0x0118))
}

inline  void sdhciWriteSdhc_rx_cfg_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0118)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0118))
}

inline unsigned int sdhciReadSdhc_mmc_ctrl_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0114)))));
}

inline void sdhciWriteSdhc_mmc_ctrl_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0114)))) = w32Val;
}

inline unsigned int sdhciReadSdhc_legacy_ceata_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0110)))));     // return the contents of Addr=((pContext->base + 0x0110))
}

inline unsigned int sdhciWriteSdhc_legacy_ceata_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0110)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0110))
    return 0;
}

inline unsigned int sdhciReadSdhc_legacy_ctrl_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x010C)))));     // return the contents of Addr=((pContext->base + 0x010C))
}

inline void sdhciWriteSdhc_legacy_ctrl_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x010C)))) = w32Val;      // write the data into the register at ((pContext->base + 0x010C))
}

inline unsigned int sdhciReadSdhc_op_ext_reg(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0108)))));     // return the contents of Addr=((pContext->base + 0x0108))
}

inline void sdhciWriteSdhc_op_ext_reg(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0108)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0108))
}

inline unsigned int sdhciReadSdhc_op_ctrl(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0104)))));     // return the contents of Addr=((pContext->base + 0x0104))
}

inline void sdhciWriteSdhc_op_ctrl(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0104)))) = w32Val;      // write the data into the register at ((pContext->base + 0x0104))
}

inline unsigned int sdhciReadSdh_vid_pid(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0100)))));     // return the contents of Addr=((pContext->base + 0x0100))
}

//BES added,2021-12-03
inline unsigned int sdhciReadSdhc_bypass_netlist(struct mmc_host *pContext)
{
    return (*(((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0800)))));
}

inline void sdhciWriteSdhc_bypass_netlist(struct mmc_host *pContext, unsigned int w32Val)
{
    *((volatile unsigned int *)(uintptr_t)((volatile unsigned int *)(uintptr_t)((pContext->base + 0x0800)))) = w32Val;
}

void sdhciWriteSdh_clear_all_regs(struct mmc_host *pContext)
{
    //sdhciWriteSdh_regs(pContext->base + 0x000, 0);//unnecessary
    //sdhciWriteSdh_regs(pContext->base + 0x004, 0);
    //sdhciWriteSdh_regs(pContext->base + 0x008, 0);
    //sdhciWriteSdh_regs(pContext->base + 0x00C, 0);
    //sdhciWriteSdh_regs(pContext->base + 0x020, 0);
    sdhciWriteSdh_regs(pContext->base + 0x028, 0);
    sdhciWriteSdh_regs(pContext->base + 0x02C, 0);
    sdhciWriteSdh_regs(pContext->base + 0x03C, 0x40000000);//default value
    //sdhciWriteSdh_regs(pContext->base + 0x058, 0);//unnecessary
    //sdhciWriteSdh_regs(pContext->base + 0x05C, 0);
    sdhciWriteSdh_regs(pContext->base + 0x0E0, 0);
    sdhciWriteSdh_regs(pContext->base + 0x104, 0x000000CE);//default value
    sdhciWriteSdh_regs(pContext->base + 0x108, 0x75090400);
    sdhciWriteSdh_regs(pContext->base + 0x10C, 0x4A000005);
    sdhciWriteSdh_regs(pContext->base + 0x110, 0x3FFF0000);
    sdhciWriteSdh_regs(pContext->base + 0x114, 0xFF000800);
    sdhciWriteSdh_regs(pContext->base + 0x118, 0);
    sdhciWriteSdh_regs(pContext->base + 0x11C, 0x003700C5);
    sdhciWriteSdh_regs(pContext->base + 0x120, 0x00002827);
    sdhciWriteSdh_regs(pContext->base + 0x124, 0);
    sdhciWriteSdh_regs(pContext->base + 0x128, 0x02030500);
    sdhciWriteSdh_regs(pContext->base + 0x130, 0);
    sdhciWriteSdh_regs(pContext->base + 0x134, 0);
#if defined(CHIP_BEST1600)
    sdhciWriteSdh_regs(pContext->base + 0x800, 0x00000680);
#else
    sdhciWriteSdh_regs(pContext->base + 0x800, 0x00300500);
    sdhciWriteSdh_regs(pContext->base + 0x808, 0);
    sdhciWriteSdh_regs(pContext->base + 0x80C, 0);
    sdhciWriteSdh_regs(pContext->base + 0x818, 0x01020040);
    sdhciWriteSdh_regs(pContext->base + 0x81C, 0xAAAAA014);
    sdhciWriteSdh_regs(pContext->base + 0x820, 0x4A200000);
    sdhciWriteSdh_regs(pContext->base + 0x824, 0x01000000);
    sdhciWriteSdh_regs(pContext->base + 0x828, 0);
    sdhciWriteSdh_regs(pContext->base + 0x82C, 0x00FF0000);
    sdhciWriteSdh_regs(pContext->base + 0x830, 0);
    sdhciWriteSdh_regs(pContext->base + 0x834, 0x00000642);
#endif
    sdhciWriteSdh_regs(pContext->base + 0x030, 0xFFFF);//clear int flag
    sdhciWriteSdh_regs(pContext->base + 0x034, 0);
    sdhciWriteSdh_regs(pContext->base + 0x038, 0);
}

#if !defined(CHIP_BEST1600)
#include "hal_cmu.h"

#define EMMCPHY_PULL_RES_NULL           0
#define EMMCPHY_PULL_RES_50KOHM         1
#define EMMCPHY_PULL_RES_30KOHM         2
#define EMMCPHY_PULL_RES_10KOHM         3
#define EMMCPHY_PULL_RES_5KOHM          3
#define EMMCPHY_PULL_ALL_RES_50KOHM     0x5555
#define EMMCPHY_PULL_ALL_RES_30KOHM     0xAAAA
#define EMMCPHY_PULL_ALL_RES_35KOHM     0xAAAA
#define EMMCPHY_PULL_ALL_RES_10KOHM     0xFFFF

enum EMMCPHY_CLK_SEL_T {
    EMMCPHY_CLK_SEL_PCK         = 0x00,
    EMMCPHY_CLK_SEL_PCK_DELAY   = 0x01,
    EMMCPHY_CLK_SEL_DS_DELAY    = 0x03,
};

void emmcphy_start(int enable)
{
    hal_cmu_emmc_phy_start(enable);
}

static void emmcphy_rd_range_config(uint8_t step_time)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x818);
    //data line tuning step
    val = SET_BITFIELD(val, EMMC_PHY_RD_RANGE, step_time);
    //cmd line tuning step
    val = SET_BITFIELD(val, EMMC_PHY_RD_RANGE2, step_time);
    sdhciWriteSdh_regs(EMMC_BASE + 0x818, val);
}

void emmcphy_open(struct mmc_host *host)
{
    #define TUNING_STEP_43PS    0
    #define TUNING_STEP_68PS    1
    #define TUNING_STEP_83PS    2
    #define TUNING_STEP_98PS    3

#if defined(CHIP_BEST2009)
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x834);
    val &= ~EMMC_PHY_LDO_EN;
    val |= EMMC_PHY_LDO_PU_DR | EMMC_PHY_LDO_PRECHARGE_DR | EMMC_PHY_LDO_PU | \
           EMMC_PHY_LDO_PRECHARGE | EMMC_PHY_CK_EN | EMMC_PHY_PHY_EN | \
           EMMC_PHY_CK_EN_DR | EMMC_PHY_PHY_EN_DR | EMMC_PHY_PU_EMMC | \
           EMMC_PHY_PU_EMMC_DR | EMMC_PHY_DLL_REFRESH | EMMC_PHY_DLL_REFRESH_DR | \
           EMMC_PHY_PU1P8 | EMMC_PHY_PU1P8_DR | EMMC_PHY_CORE_PU;
    sdhciWriteSdh_regs(EMMC_BASE + 0x834, val);
#else
    hal_cmu_emmc_phy_open();
#endif
    if (host->speed_mode == EHS400 || host->speed_mode == HS400) {
        emmcphy_rd_range_config(TUNING_STEP_68PS);  //set delay range
    } else {
        emmcphy_rd_range_config(TUNING_STEP_43PS);  //set delay range
    }
    emmcphy_clk_sel_pck_delay();                    //phy clock source selection
}

void emmcphy_close(void)
{
#if defined(CHIP_BEST2009)
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x834);
    val &= ~(EMMC_PHY_LDO_PU_DR | EMMC_PHY_LDO_PRECHARGE_DR | EMMC_PHY_LDO_PU | \
             EMMC_PHY_LDO_PRECHARGE | EMMC_PHY_CK_EN | EMMC_PHY_PHY_EN | \
             EMMC_PHY_CK_EN_DR | EMMC_PHY_PHY_EN_DR | EMMC_PHY_PU_EMMC | \
             EMMC_PHY_PU_EMMC_DR | EMMC_PHY_DLL_REFRESH | EMMC_PHY_DLL_REFRESH_DR | \
             EMMC_PHY_PU1P8 | EMMC_PHY_PU1P8_DR | EMMC_PHY_CORE_PU | EMMC_PHY_LDO_EN);
    sdhciWriteSdh_regs(EMMC_BASE + 0x834, val);
#else
    hal_cmu_emmc_phy_close();
#endif
    emmcphy_pin_pull_down_all();
}

void emmcphy_sleep(void)
{
#if defined(CHIP_BEST2009)
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x834);
    val &= ~(EMMC_PHY_LDO_PU | EMMC_PHY_LDO_PRECHARGE | EMMC_PHY_CK_EN | EMMC_PHY_PHY_EN | \
             EMMC_PHY_PU_EMMC | EMMC_PHY_DLL_REFRESH | EMMC_PHY_CORE_PU);
    sdhciWriteSdh_regs(EMMC_BASE + 0x834, val);
#else
    hal_cmu_emmc_phy_sleep();
#endif
}

void emmcphy_wakeup(void)
{
#if defined(CHIP_BEST2009)
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x834);
    val |= EMMC_PHY_LDO_PU | EMMC_PHY_LDO_PRECHARGE | EMMC_PHY_CK_EN | EMMC_PHY_PHY_EN | \
           EMMC_PHY_PU_EMMC | EMMC_PHY_DLL_REFRESH | EMMC_PHY_CORE_PU;
    sdhciWriteSdh_regs(EMMC_BASE + 0x834, val);
#else
    hal_cmu_emmc_phy_wakeup();
#endif
}

void emmcphy_pck_reverse(int rev_cmd_en, int rev_data_en)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x818);

    //data line
    if (rev_data_en) {
        if (val & EMMC_PHY_PCK_REVERSE) {
            val &= ~EMMC_PHY_PCK_REVERSE;
        } else {
            val |= EMMC_PHY_PCK_REVERSE;
        }
    }
    //cmd line
    if (rev_cmd_en) {
        if (val & EMMC_PHY_PCK_REVERSE2) {
            val &= ~EMMC_PHY_PCK_REVERSE2;
        } else {
            val |= EMMC_PHY_PCK_REVERSE2;
        }
    }
    sdhciWriteSdh_regs(EMMC_BASE + 0x818, val);
}

void emmcphy_tuning_correct(void)
{
    /*
        This function will be implemented after
        tuning is completed and will not be carried out here
    */
}

void emmcphy_rd_tune_dly_cali(uint8_t cali_val_cmd, uint8_t cali_val_data)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x818);
    //data line, range:0-63, default:32
    val = SET_BITFIELD(val, EMMC_PHY_RD_TUNE_DLY, cali_val_data);
    //cmd line, range:0-63, default:32
    val = SET_BITFIELD(val, EMMC_PHY_RD_TUNE_DLY2, cali_val_cmd);
    sdhciWriteSdh_regs(EMMC_BASE + 0x818, val);
}

void emmcphy_rd_tune_dly_read(uint8_t *cmd_dly, uint8_t *data_dly)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x818);
    if (cmd_dly) {
        *cmd_dly = GET_BITFIELD(val, EMMC_PHY_RD_TUNE_DLY2);
    }
    if (data_dly) {
        *data_dly = GET_BITFIELD(val, EMMC_PHY_RD_TUNE_DLY);
    }
}

static void emmcphy_clk_sel(uint8_t sel)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x818);
    //data line
    val = SET_BITFIELD(val, EMMC_PHY_CK_SEL, sel);
    //cmd line
    val = SET_BITFIELD(val, EMMC_PHY_CK_SEL2, sel);
    sdhciWriteSdh_regs(EMMC_BASE + 0x818, val);
}

void emmcphy_clk_sel_pck(void)
{
    emmcphy_clk_sel(EMMCPHY_CLK_SEL_PCK);
}

void emmcphy_clk_sel_pck_delay(void)
{
    emmcphy_clk_sel(EMMCPHY_CLK_SEL_PCK_DELAY);
}

void emmcphy_clk_sel_ds_delay(void)
{
    emmcphy_clk_sel(EMMCPHY_CLK_SEL_DS_DELAY);
}

void emmcphy_hs_host_mode_enable(void)
{
    emmcphy_rd_tune_dly_cali(0, 0);//The value is the same as 1600
}

void emmcphy_hs_host_mode_disable(void)
{
    //No need to see it currently
}

void emmcphy_ddr_mode_enable(void)
{
    emmcphy_rd_tune_dly_cali(38, 38);//The value is the same as 1600
}

void emmcphy_ddr_mode_disable(void)
{
    //No need
}

void emmcphy_pin_pull_config(void)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x81C);
    /*
        1. Reserve driver strength
        2. Clear the pull-up and pull-down resistors of the internal cmd, ds, and clk line
        3. Clear the pull-up resistors of the internal data0-data7
    */
    val &= 0x0F;
    //cmd pin pull-up
#if defined(CHIP_BEST2009)
    val |= (EMMCPHY_PULL_RES_5KOHM << 4);
#else
    val |= (EMMCPHY_PULL_RES_10KOHM << 4);
#endif
    //clk pin does not need to be pulled up or pulled down
    /* val |= (EMMCPHY_PULL_RES_10KOHM << 8); */
    //ds pin pull-down
    val |= (EMMCPHY_PULL_RES_50KOHM << 12);
    //data0-data7 pin pull-up
#if defined(CHIP_BEST2009)
    val |= (EMMCPHY_PULL_ALL_RES_35KOHM << 16);
#else
    val |= (EMMCPHY_PULL_ALL_RES_30KOHM << 16);
#endif
    sdhciWriteSdh_regs(EMMC_BASE + 0x81C, val);

    val = sdhciReadSdh_regs(EMMC_BASE + 0x820);
    //clear the pull-down resistor on the data0-data7 line
    val &= ~0xFFFF;
    sdhciWriteSdh_regs(EMMC_BASE + 0x820, val);
}

void emmcphy_pin_pull_clear_all(void)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x81C);
    /*
        1. Reserve driver strength
        2. Clear the pull-up and pull-down resistors of the internal cmd, ds, and clk line
        3. Clear the pull-up resistors of the internal data0-data7
    */
    val &= 0x0F;
    sdhciWriteSdh_regs(EMMC_BASE + 0x81C, val);

    val = sdhciReadSdh_regs(EMMC_BASE + 0x820);
    //clear the pull-down resistor on the data0-data7 line
    val &= ~0xFFFF;
    sdhciWriteSdh_regs(EMMC_BASE + 0x820, val);
}

void emmcphy_pin_pull_down_all(void)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x81C);
    /*
        1. Reserve driver strength
        2. Clear the pull-up and pull-down resistors of the internal cmd, ds, and clk line
        3. Clear the pull-up resistors of the internal data0-data7
    */
    val &= 0x0F;
    //cmd pin pull-down
    val |= (EMMCPHY_PULL_RES_30KOHM << 10);
    //ds pin pull-down
    val |= (EMMCPHY_PULL_RES_30KOHM << 12);
    //clk pin pull-down
    val |= (EMMCPHY_PULL_RES_30KOHM << 14);
    sdhciWriteSdh_regs(EMMC_BASE + 0x81C, val);

    val = sdhciReadSdh_regs(EMMC_BASE + 0x820);
    //set the pull-down resistor on the data line
    val &= ~0xFFFF;
    val |= EMMCPHY_PULL_ALL_RES_30KOHM;
    sdhciWriteSdh_regs(EMMC_BASE + 0x820, val);
}

static void emmcphy_pin_set_io_drv_raw(uint8_t drv)
{
    uint32_t val;

    val = sdhciReadSdh_regs(EMMC_BASE + 0x81C);
    val &= ~0xF;
    val |= drv & 0xF;
    sdhciWriteSdh_regs(EMMC_BASE + 0x81C, val);
}

void emmcphy_pin_set_io_drv(void)
{
    #define EMMCPHY_DRV_700_OHM             1
    #define EMMCPHY_DRV_600_OHM             2
    #define EMMCPHY_DRV_500_OHM             3
    #define EMMCPHY_DRV_400_OHM             4
    #define EMMCPHY_DRV_300_OHM             5
    #define EMMCPHY_DRV_200_OHM             6
    #define EMMCPHY_DRV_180_OHM             7
    #define EMMCPHY_DRV_160_OHM             8
    #define EMMCPHY_DRV_140_OHM             9
    #define EMMCPHY_DRV_100_OHM             10
    #define EMMCPHY_DRV_75_OHM              11
    #define EMMCPHY_DRV_50_OHM              12
    #define EMMCPHY_DRV_40_OHM              13
    #define EMMCPHY_DRV_30_OHM              14
    #define EMMCPHY_DRV_20_OHM              15
    emmcphy_pin_set_io_drv_raw(EMMCPHY_DRV_50_OHM);
}

#endif
