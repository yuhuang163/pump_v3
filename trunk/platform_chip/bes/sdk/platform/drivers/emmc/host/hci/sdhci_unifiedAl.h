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
#ifndef __EMMC_SDHCI_UNIFIEDAL_H__
#define __EMMC_SDHCI_UNIFIEDAL_H__
#include <stdint.h>
#include "host.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct mmc_host sdhci_hosts[5];

////////////////////////////////////////////////////////////////////
// Register SDH_SYSADDR Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_SYSADDR_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SYSADDR_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SYSADDR_RW_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_SYSADDR


////////////////////////////////////////////////////////////////////
// Register SDH_BLOCK_SIZE Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_BLOCK_SIZE_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_BLOCK_SIZE_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_BLOCK_SIZE_RESERVED_0_MASK      0x8000
#define SDHCI_SDH_BLOCK_SIZE_RW_MASK              0x7FFF
//Bitfield Constants Register SDH_BLOCK_SIZE
#define SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_MASK                    0x7000
#define SDHCI_SDH_BLOCK_SIZE_HOST_DMA_BDRY_SHIFT                   12
#define SDHCI_SDH_BLOCK_SIZE_BLOCK_SIZE_MASK                       0x0FFF


////////////////////////////////////////////////////////////////////
// Register SDH_BLOCK_COUNT Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_BLOCK_COUNT_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_BLOCK_COUNT_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_BLOCK_COUNT_RW_MASK              0xFFFF
//Bitfield Constants Register SDH_BLOCK_COUNT


////////////////////////////////////////////////////////////////////
// Register SDH_ARG Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_ARG_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_ARG_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_ARG_RW_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_ARG


////////////////////////////////////////////////////////////////////
// Register SDH_TRANSFER_MODE Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_TRANSFER_MODE_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_TRANSFER_MODE_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_TRANSFER_MODE_RESERVED_0_MASK      0xFFC0
#define SDHCI_SDH_TRANSFER_MODE_RW_MASK              0x003F
//Bitfield Constants Register SDH_TRANSFER_MODE
#define SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_MASK                    0x0020
#define SDHCI_SDH_TRANSFER_MODE_MULTI_BLK_SEL_SHIFT                   5
#define SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_MASK                      0x0010
#define SDHCI_SDH_TRANSFER_MODE_TO_HOST_DIR_SHIFT                     4
#define SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_MASK                      0x000C
#define SDHCI_SDH_TRANSFER_MODE_AUTO_CMD_EN_SHIFT                     2
#define SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_MASK                       0x0002
#define SDHCI_SDH_TRANSFER_MODE_BLK_CNT_EN_SHIFT                      1
#define SDHCI_SDH_TRANSFER_MODE_DMA_EN_MASK                           0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_CMD Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CMD_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_CMD_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_CMD_RESERVED_0_MASK      0xC004
#define SDHCI_SDH_CMD_RW_MASK              0x3FFB
//Bitfield Constants Register SDH_CMD
#define SDHCI_SDH_CMD_CMD_INDEX_MASK                        0x3F00
#define SDHCI_SDH_CMD_CMD_INDEX_SHIFT                       8
#define SDHCI_SDH_CMD_CMD_TYPE_MASK                         0x00C0
#define SDHCI_SDH_CMD_CMD_TYPE_SHIFT                        6
#define SDHCI_SDH_CMD_DATA_PRESENT_MASK                     0x0020
#define SDHCI_SDH_CMD_DATA_PRESENT_SHIFT                    5
#define SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_MASK                 0x0010
#define SDHCI_SDH_CMD_CMD_INDEX_CHK_EN_SHIFT                4
#define SDHCI_SDH_CMD_CMD_CRC_CHK_EN_MASK                   0x0008
#define SDHCI_SDH_CMD_CMD_CRC_CHK_EN_SHIFT                  3
#define SDHCI_SDH_CMD_RESP_TYPE_MASK                        0x0003


////////////////////////////////////////////////////////////////////
// Register SDH_RESPx Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_RESPX_INSTANCES            4
#define SDHCI_SDH_RESPX_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_RESPX_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_RESPX_RO_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_RESPx


////////////////////////////////////////////////////////////////////
// Register SDH_DP_BUFFER Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_DP_BUFFER_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_DP_BUFFER_RW_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_DP_BUFFER


////////////////////////////////////////////////////////////////////
// Register SDH_PRESENT_STATE_1 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_PRESENT_STATE_1_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_PRESENT_STATE_1_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_PRESENT_STATE_1_RESERVED_0_MASK      0xF0F0
#define SDHCI_SDH_PRESENT_STATE_1_RO_MASK              0x0F0F
//Bitfield Constants Register SDH_PRESENT_STATE_1
#define SDHCI_SDH_PRESENT_STATE_1_BUFFER_RD_EN_MASK                     0x0800
#define SDHCI_SDH_PRESENT_STATE_1_BUFFER_RD_EN_SHIFT                    11
#define SDHCI_SDH_PRESENT_STATE_1_BUFFER_WR_EN_MASK                     0x0400
#define SDHCI_SDH_PRESENT_STATE_1_BUFFER_WR_EN_SHIFT                    10
#define SDHCI_SDH_PRESENT_STATE_1_RX_ACTIVE_MASK                        0x0200
#define SDHCI_SDH_PRESENT_STATE_1_RX_ACTIVE_SHIFT                       9
#define SDHCI_SDH_PRESENT_STATE_1_TX_ACTIVE_MASK                        0x0100
#define SDHCI_SDH_PRESENT_STATE_1_TX_ACTIVE_SHIFT                       8
#define SDHCI_SDH_PRESENT_STATE_1_RETUNING_REQ_MASK                     0x0008
#define SDHCI_SDH_PRESENT_STATE_1_RETUNING_REQ_SHIFT                    3
#define SDHCI_SDH_PRESENT_STATE_1_DAT_ACTIVE_MASK                       0x0004
#define SDHCI_SDH_PRESENT_STATE_1_DAT_ACTIVE_SHIFT                      2
#define SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_DAT_MASK                  0x0002
#define SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_DAT_SHIFT                 1
#define SDHCI_SDH_PRESENT_STATE_1_CMD_INHIBIT_CMD_MASK                  0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_PRESENT_STATE_2 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_PRESENT_STATE_2_REGISTER_RESET_VALUE 0x01FF
#define SDHCI_SDH_PRESENT_STATE_2_REGISTER_RESET_MASK  0x01FF
#define SDHCI_SDH_PRESENT_STATE_2_RESERVED_0_MASK      0xFE00
#define SDHCI_SDH_PRESENT_STATE_2_RO_MASK              0x01FF
//Bitfield Constants Register SDH_PRESENT_STATE_2
#define SDHCI_SDH_PRESENT_STATE_2_CMD_LEVEL_MASK                        0x0100
#define SDHCI_SDH_PRESENT_STATE_2_CMD_LEVEL_SHIFT                       8
#define SDHCI_SDH_PRESENT_STATE_2_DAT_LEVEL_MASK                        0x00F0
#define SDHCI_SDH_PRESENT_STATE_2_DAT_LEVEL_SHIFT                       4
#define SDHCI_SDH_PRESENT_STATE_2_WRITE_PROT_MASK                       0x0008
#define SDHCI_SDH_PRESENT_STATE_2_WRITE_PROT_SHIFT                      3
#define SDHCI_SDH_PRESENT_STATE_2_CARD_DETECT_MASK                      0x0004
#define SDHCI_SDH_PRESENT_STATE_2_CARD_DETECT_SHIFT                     2
#define SDHCI_SDH_PRESENT_STATE_2_CARD_STABLE_MASK                      0x0002
#define SDHCI_SDH_PRESENT_STATE_2_CARD_STABLE_SHIFT                     1
#define SDHCI_SDH_PRESENT_STATE_2_CARD_INSERTED_MASK                    0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_HOST_CTRL Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_HOST_CTRL_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_HOST_CTRL_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_HOST_CTRL_RESERVED_0_MASK      0xF000
#define SDHCI_SDH_HOST_CTRL_RW_MASK              0x0FFF
//Bitfield Constants Register SDH_HOST_CTRL
#define SDHCI_SDH_HOST_CTRL_BUS_VOLT_MASK                         0x0E00
#define SDHCI_SDH_HOST_CTRL_BUS_VOLT_SHIFT                        9
#define SDHCI_SDH_HOST_CTRL_BUS_POWER_MASK                        0x0100
#define SDHCI_SDH_HOST_CTRL_BUS_POWER_SHIFT                       8
#define SDHCI_SDH_HOST_CTRL_CARD_DET_S_MASK                       0x0080
#define SDHCI_SDH_HOST_CTRL_CARD_DET_S_SHIFT                      7
#define SDHCI_SDH_HOST_CTRL_CARD_DET_L_MASK                       0x0040
#define SDHCI_SDH_HOST_CTRL_CARD_DET_L_SHIFT                      6
#define SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_MASK                    0x0020
#define SDHCI_SDH_HOST_CTRL_EX_DATA_WIDTH_SHIFT                   5
#define SDHCI_SDH_HOST_CTRL_DMA_SEL_MASK                          0x0018
#define SDHCI_SDH_HOST_CTRL_DMA_SEL_SHIFT                         3
#define SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_MASK                      0x0004
#define SDHCI_SDH_HOST_CTRL_HI_SPEED_EN_SHIFT                     2
#define SDHCI_SDH_HOST_CTRL_DATA_WIDTH_MASK                       0x0002
#define SDHCI_SDH_HOST_CTRL_DATA_WIDTH_SHIFT                      1
#define SDHCI_SDH_HOST_CTRL_LED_CTRL_MASK                         0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_BLK_GAP_CTRL Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_BLK_GAP_CTRL_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_BLK_GAP_CTRL_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_BLK_GAP_CTRL_RESERVED_0_MASK      0xF8F0
#define SDHCI_SDH_BLK_GAP_CTRL_RW_MASK              0x070F
//Bitfield Constants Register SDH_BLK_GAP_CTRL
#define SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_MASK                        0x0400
#define SDHCI_SDH_BLK_GAP_CTRL_W_REMOVAL_SHIFT                       10
#define SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_MASK                      0x0200
#define SDHCI_SDH_BLK_GAP_CTRL_W_INSERTION_SHIFT                     9
#define SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_MASK                       0x0100
#define SDHCI_SDH_BLK_GAP_CTRL_W_CARD_INT_SHIFT                      8
#define SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_MASK                      0x0008
#define SDHCI_SDH_BLK_GAP_CTRL_INT_BLK_GAP_SHIFT                     3
#define SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_MASK                      0x0004
#define SDHCI_SDH_BLK_GAP_CTRL_RD_WAIT_CTL_SHIFT                     2
#define SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_MASK                         0x0002
#define SDHCI_SDH_BLK_GAP_CTRL_CONT_REQ_SHIFT                        1
#define SDHCI_SDH_BLK_GAP_CTRL_STOP_AT_BLOCK_GAP_REQ_MASK            0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_CLOCK_CTRL Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CLOCK_CTRL_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_CLOCK_CTRL_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_CLOCK_CTRL_RESERVED_0_MASK      0x0018
#define SDHCI_SDH_CLOCK_CTRL_RW_MASK              0xFFE5
#define SDHCI_SDH_CLOCK_CTRL_RO_MASK              0x0002
//Bitfield Constants Register SDH_CLOCK_CTRL
#define SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_MASK                      0xFF00
#define SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_LO_SHIFT                     8
#define SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_MASK                      0x00C0
#define SDHCI_SDH_CLOCK_CTRL_FREQ_SEL_HI_SHIFT                     6
#define SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_MASK                      0x0020
#define SDHCI_SDH_CLOCK_CTRL_CLK_GEN_SEL_SHIFT                     5
#define SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_MASK                         0x0004
#define SDHCI_SDH_CLOCK_CTRL_CLOCK_EN_SHIFT                        2
#define SDHCI_SDH_CLOCK_CTRL_INT_CLK_STABLE_MASK                   0x0002
#define SDHCI_SDH_CLOCK_CTRL_INT_CLK_STABLE_SHIFT                  1
#define SDHCI_SDH_CLOCK_CTRL_INT_CLK_EN_MASK                       0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_TIMEOUT_CTRL_SW_RESET Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RESERVED_0_MASK      0xF8F0
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_RW_MASK              0x070F
//Bitfield Constants Register SDH_TIMEOUT_CTRL_SW_RESET
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_MASK                       0x0400
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_DAT_SHIFT                      10
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_MASK                       0x0200
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_CMD_SHIFT                      9
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_MASK                       0x0100
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_SW_RST_ALL_SHIFT                      8
#define SDHCI_SDH_TIMEOUT_CTRL_SW_RESET_TIMEOUT_VALUE_MASK                    0x000F


////////////////////////////////////////////////////////////////////
// Register SDH_I_STAT Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_I_STAT_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_I_STAT_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_I_STAT_RESERVED_0_MASK      0x6000
#define SDHCI_SDH_I_STAT_SET_TO_CLEAR_MASK    0x1EFF
#define SDHCI_SDH_I_STAT_STC_MASK             0x1EFF
#define SDHCI_SDH_I_STAT_RO_MASK              0x8100
//Bitfield Constants Register SDH_I_STAT
#define SDHCI_SDH_I_STAT_ERR_INT_MASK                          0x8000
#define SDHCI_SDH_I_STAT_ERR_INT_SHIFT                         15
#define SDHCI_SDH_I_STAT_RETUNING_INT_MASK                     0x1000
#define SDHCI_SDH_I_STAT_RETUNING_INT_SHIFT                    12
#define SDHCI_SDH_I_STAT_INT_C_MASK                            0x0800
#define SDHCI_SDH_I_STAT_INT_C_SHIFT                           11
#define SDHCI_SDH_I_STAT_INT_B_MASK                            0x0400
#define SDHCI_SDH_I_STAT_INT_B_SHIFT                           10
#define SDHCI_SDH_I_STAT_INT_A_MASK                            0x0200
#define SDHCI_SDH_I_STAT_INT_A_SHIFT                           9
#define SDHCI_SDH_I_STAT_CARD_INT_MASK                         0x0100
#define SDHCI_SDH_I_STAT_CARD_INT_SHIFT                        8
#define SDHCI_SDH_I_STAT_CARD_REM_INT_MASK                     0x0080
#define SDHCI_SDH_I_STAT_CARD_REM_INT_SHIFT                    7
#define SDHCI_SDH_I_STAT_CARD_INS_INT_MASK                     0x0040
#define SDHCI_SDH_I_STAT_CARD_INS_INT_SHIFT                    6
#define SDHCI_SDH_I_STAT_RX_RDY_MASK                           0x0020
#define SDHCI_SDH_I_STAT_RX_RDY_SHIFT                          5
#define SDHCI_SDH_I_STAT_TX_RDY_MASK                           0x0010
#define SDHCI_SDH_I_STAT_TX_RDY_SHIFT                          4
#define SDHCI_SDH_I_STAT_DMA_INT_MASK                          0x0008
#define SDHCI_SDH_I_STAT_DMA_INT_SHIFT                         3
#define SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_MASK                  0x0004
#define SDHCI_SDH_I_STAT_BLOCK_GAP_EVENT_SHIFT                 2
#define SDHCI_SDH_I_STAT_XFER_COMPLETE_MASK                    0x0002
#define SDHCI_SDH_I_STAT_XFER_COMPLETE_SHIFT                   1
#define SDHCI_SDH_I_STAT_CMD_COMPLETE_MASK                     0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_E_STAT Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_E_STAT_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_E_STAT_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_E_STAT_RESERVED_0_MASK      0x0800
#define SDHCI_SDH_E_STAT_SET_TO_CLEAR_MASK    0xF7FF
#define SDHCI_SDH_E_STAT_STC_MASK             0xF7FF
//Bitfield Constants Register SDH_E_STAT
#define SDHCI_SDH_E_STAT_CRC_STATUS_ERR_MASK                   0x8000
#define SDHCI_SDH_E_STAT_CRC_STATUS_ERR_SHIFT                  15
#define SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_MASK                  0x4000
#define SDHCI_SDH_E_STAT_CPL_TIMEOUT_ERR_SHIFT                 14
#define SDHCI_SDH_E_STAT_AXI_RESP_ERR_MASK                     0x2000
#define SDHCI_SDH_E_STAT_AXI_RESP_ERR_SHIFT                    13
#define SDHCI_SDH_E_STAT_SPI_ERR_MASK                          0x1000
#define SDHCI_SDH_E_STAT_SPI_ERR_SHIFT                         12
#define SDHCI_SDH_E_STAT_TUNE_ERR_MASK                         0x0400
#define SDHCI_SDH_E_STAT_TUNE_ERR_SHIFT                        10
#define SDHCI_SDH_E_STAT_ADMA_ERR_MASK                         0x0200
#define SDHCI_SDH_E_STAT_ADMA_ERR_SHIFT                        9
#define SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_MASK                   0x0100
#define SDHCI_SDH_E_STAT_AUTO_CMD12_ERR_SHIFT                  8
#define SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_MASK                    0x0080
#define SDHCI_SDH_E_STAT_CUR_LIMIT_ERR_SHIFT                   7
#define SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_MASK              0x0040
#define SDHCI_SDH_E_STAT_RD_DATA_END_BIT_ERR_SHIFT             6
#define SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_MASK                  0x0020
#define SDHCI_SDH_E_STAT_RD_DATA_CRC_ERR_SHIFT                 5
#define SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_MASK                 0x0010
#define SDHCI_SDH_E_STAT_DATA_TIMEOUT_ERR_SHIFT                4
#define SDHCI_SDH_E_STAT_CMD_INDEX_ERR_MASK                    0x0008
#define SDHCI_SDH_E_STAT_CMD_INDEX_ERR_SHIFT                   3
#define SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_MASK                  0x0004
#define SDHCI_SDH_E_STAT_CMD_END_BIT_ERR_SHIFT                 2
#define SDHCI_SDH_E_STAT_CMD_CRC_ERR_MASK                      0x0002
#define SDHCI_SDH_E_STAT_CMD_CRC_ERR_SHIFT                     1
#define SDHCI_SDH_E_STAT_CMD_TIMEOUT_ERR_MASK                  0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_I_STAT_EN Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_I_STAT_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_I_STAT_EN_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_I_STAT_EN_RESERVED_0_MASK      0xE000
#define SDHCI_SDH_I_STAT_EN_RW_MASK              0x1FFF
//Bitfield Constants Register SDH_I_STAT_EN
#define SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_MASK                    0x1000
#define SDHCI_SDH_I_STAT_EN_RETUNE_INT_EN_SHIFT                   12
#define SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_MASK                     0x0800
#define SDHCI_SDH_I_STAT_EN_INT_C_INT_EN_SHIFT                    11
#define SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_MASK                     0x0400
#define SDHCI_SDH_I_STAT_EN_INT_B_INT_EN_SHIFT                    10
#define SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_MASK                     0x0200
#define SDHCI_SDH_I_STAT_EN_INT_A_INT_EN_SHIFT                    9
#define SDHCI_SDH_I_STAT_EN_CARD_INT_EN_MASK                      0x0100
#define SDHCI_SDH_I_STAT_EN_CARD_INT_EN_SHIFT                     8
#define SDHCI_SDH_I_STAT_EN_CARD_REM_EN_MASK                      0x0080
#define SDHCI_SDH_I_STAT_EN_CARD_REM_EN_SHIFT                     7
#define SDHCI_SDH_I_STAT_EN_CARD_INS_EN_MASK                      0x0040
#define SDHCI_SDH_I_STAT_EN_CARD_INS_EN_SHIFT                     6
#define SDHCI_SDH_I_STAT_EN_RX_RDY_EN_MASK                        0x0020
#define SDHCI_SDH_I_STAT_EN_RX_RDY_EN_SHIFT                       5
#define SDHCI_SDH_I_STAT_EN_TX_RDY_EN_MASK                        0x0010
#define SDHCI_SDH_I_STAT_EN_TX_RDY_EN_SHIFT                       4
#define SDHCI_SDH_I_STAT_EN_DMA_INT_EN_MASK                       0x0008
#define SDHCI_SDH_I_STAT_EN_DMA_INT_EN_SHIFT                      3
#define SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_MASK                      0x0004
#define SDHCI_SDH_I_STAT_EN_BGAPEVNT_EN_SHIFT                     2
#define SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_MASK                       0x0002
#define SDHCI_SDH_I_STAT_EN_XFRCOMP_EN_SHIFT                      1
#define SDHCI_SDH_I_STAT_EN_CMDCOMP_EN_MASK                       0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_E_STAT_EN Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_E_STAT_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_E_STAT_EN_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_E_STAT_EN_RESERVED_0_MASK      0x0800
#define SDHCI_SDH_E_STAT_EN_RW_MASK              0xF7FF
//Bitfield Constants Register SDH_E_STAT_EN
#define SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_MASK                  0x8000
#define SDHCI_SDH_E_STAT_EN_CRC_STAT_ERR_EN_SHIFT                 15
#define SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_MASK                    0x4000
#define SDHCI_SDH_E_STAT_EN_CPL_TO_ERR_EN_SHIFT                   14
#define SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_MASK                  0x2000
#define SDHCI_SDH_E_STAT_EN_AXI_RESP_ERR_EN_SHIFT                 13
#define SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_MASK                       0x1000
#define SDHCI_SDH_E_STAT_EN_SPI_ERR_EN_SHIFT                      12
#define SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_MASK                    0x0400
#define SDHCI_SDH_E_STAT_EN_TUNING_ERR_EN_SHIFT                   10
#define SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_MASK                      0x0200
#define SDHCI_SDH_E_STAT_EN_ADMA_ERR_EN_SHIFT                     9
#define SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_MASK                      0x0100
#define SDHCI_SDH_E_STAT_EN_AC12_ERR_EN_SHIFT                     8
#define SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_MASK                     0x0080
#define SDHCI_SDH_E_STAT_EN_CURLMTERR_EN_SHIFT                    7
#define SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_MASK                    0x0040
#define SDHCI_SDH_E_STAT_EN_DATAENDERR_EN_SHIFT                   6
#define SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_MASK                    0x0020
#define SDHCI_SDH_E_STAT_EN_DATACRCERR_EN_SHIFT                   5
#define SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_MASK                    0x0010
#define SDHCI_SDH_E_STAT_EN_DATA_TOERR_EN_SHIFT                   4
#define SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_MASK                    0x0008
#define SDHCI_SDH_E_STAT_EN_CMD_IDXERR_EN_SHIFT                   3
#define SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_MASK                    0x0004
#define SDHCI_SDH_E_STAT_EN_CMD_ENDERR_EN_SHIFT                   2
#define SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_MASK                    0x0002
#define SDHCI_SDH_E_STAT_EN_CMD_CRCERR_EN_SHIFT                   1
#define SDHCI_SDH_E_STAT_EN_CMD_TO_ERR_EN_MASK                    0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_I_SIGN_EN Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_I_SIGN_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_I_SIGN_EN_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_I_SIGN_EN_RESERVED_0_MASK      0xE000
#define SDHCI_SDH_I_SIGN_EN_RW_MASK              0x1FFF
//Bitfield Constants Register SDH_I_SIGN_EN
#define SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_MASK                  0x1000
#define SDHCI_SDH_I_SIGN_EN_RETUNE_INT_SGEN_SHIFT                 12
#define SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_MASK                   0x0800
#define SDHCI_SDH_I_SIGN_EN_INT_C_INT_SGEN_SHIFT                  11
#define SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_MASK                   0x0400
#define SDHCI_SDH_I_SIGN_EN_INT_B_INT_SGEN_SHIFT                  10
#define SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_MASK                   0x0200
#define SDHCI_SDH_I_SIGN_EN_INT_A_INT_SGEN_SHIFT                  9
#define SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_MASK                    0x0100
#define SDHCI_SDH_I_SIGN_EN_CARD_INT_SGEN_SHIFT                   8
#define SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_MASK                    0x0080
#define SDHCI_SDH_I_SIGN_EN_CARD_REM_SGEN_SHIFT                   7
#define SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_MASK                    0x0040
#define SDHCI_SDH_I_SIGN_EN_CARD_INS_SGEN_SHIFT                   6
#define SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_MASK                      0x0020
#define SDHCI_SDH_I_SIGN_EN_RX_RDY_SGEN_SHIFT                     5
#define SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_MASK                      0x0010
#define SDHCI_SDH_I_SIGN_EN_TX_RDY_SGEN_SHIFT                     4
#define SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_MASK                     0x0008
#define SDHCI_SDH_I_SIGN_EN_DMA_INT_SGEN_SHIFT                    3
#define SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_MASK                    0x0004
#define SDHCI_SDH_I_SIGN_EN_BGAPEVNT_SGEN_SHIFT                   2
#define SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_MASK                     0x0002
#define SDHCI_SDH_I_SIGN_EN_XFRCOMP_SGEN_SHIFT                    1
#define SDHCI_SDH_I_SIGN_EN_CMDCOMP_SGEN_MASK                     0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_E_SIGN_EN Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_E_SIGN_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_E_SIGN_EN_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_E_SIGN_EN_RESERVED_0_MASK      0x0800
#define SDHCI_SDH_E_SIGN_EN_RW_MASK              0xF7FF
//Bitfield Constants Register SDH_E_SIGN_EN
#define SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_MASK                0x8000
#define SDHCI_SDH_E_SIGN_EN_CRC_STAT_ERR_SGEN_SHIFT               15
#define SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_MASK                  0x4000
#define SDHCI_SDH_E_SIGN_EN_CPL_TO_ERR_SGEN_SHIFT                 14
#define SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_MASK                0x2000
#define SDHCI_SDH_E_SIGN_EN_AXI_RESP_ERR_SGEN_SHIFT               13
#define SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_MASK                     0x1000
#define SDHCI_SDH_E_SIGN_EN_SPI_ERR_SGEN_SHIFT                    12
#define SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_MASK                    0x0400
#define SDHCI_SDH_E_SIGN_EN_TUNE_ERR_SGEN_SHIFT                   10
#define SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_MASK                    0x0200
#define SDHCI_SDH_E_SIGN_EN_ADMA_ERR_SGEN_SHIFT                   9
#define SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_MASK                    0x0100
#define SDHCI_SDH_E_SIGN_EN_AC12_ERR_SGEN_SHIFT                   8
#define SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_MASK                   0x0080
#define SDHCI_SDH_E_SIGN_EN_CURLMTERR_SGEN_SHIFT                  7
#define SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_MASK                  0x0040
#define SDHCI_SDH_E_SIGN_EN_DATAENDERR_SGEN_SHIFT                 6
#define SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_MASK                  0x0020
#define SDHCI_SDH_E_SIGN_EN_DATACRCERR_SGEN_SHIFT                 5
#define SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_MASK                  0x0010
#define SDHCI_SDH_E_SIGN_EN_DATA_TOERR_SGEN_SHIFT                 4
#define SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_MASK                  0x0008
#define SDHCI_SDH_E_SIGN_EN_CMD_IDXERR_SGEN_SHIFT                 3
#define SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_MASK                  0x0004
#define SDHCI_SDH_E_SIGN_EN_CMD_ENDERR_SGEN_SHIFT                 2
#define SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_MASK                  0x0002
#define SDHCI_SDH_E_SIGN_EN_CMD_CRCERR_SGEN_SHIFT                 1
#define SDHCI_SDH_E_SIGN_EN_CMD_TO_ERR_SGEN_MASK                  0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_AUTO_CMD12_ERROR_STATUS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RESERVED_0_MASK      0xFF60
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_SET_TO_CLEAR_MASK    0x001F
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_STC_MASK             0x001F
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_RO_MASK              0x0080
//Bitfield Constants Register SDH_AUTO_CMD12_ERROR_STATUS
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_CMD_NOT_ISSUED_MASK                   0x0080
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_CMD_NOT_ISSUED_SHIFT                  7
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_MASK               0x0010
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_INDEX_ERR_SHIFT              4
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_MASK             0x0008
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_END_BIT_ERR_SHIFT            3
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_MASK                 0x0004
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_CRC_ERR_SHIFT                2
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_MASK             0x0002
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD_TIMEOUT_ERR_SHIFT            1
#define SDHCI_SDH_AUTO_CMD12_ERROR_STATUS_AUTO_CMD12_NOT_EXE_MASK               0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_HOST_CTRL_2 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_HOST_CTRL_2_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_HOST_CTRL_2_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_HOST_CTRL_2_RESERVED_0_MASK      0x3F00
#define SDHCI_SDH_HOST_CTRL_2_RW_MASK              0xC0FF
//Bitfield Constants Register SDH_HOST_CTRL_2
#define SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_MASK                       0x8000
#define SDHCI_SDH_HOST_CTRL_2_PRE_VAL_EN_SHIFT                      15
#define SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_MASK                     0x4000
#define SDHCI_SDH_HOST_CTRL_2_ASYNC_INT_EN_SHIFT                    14
#define SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_MASK                 0x0080
#define SDHCI_SDH_HOST_CTRL_2_SAMPLING_CLK_SEL_SHIFT                7
#define SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_MASK                       0x0040
#define SDHCI_SDH_HOST_CTRL_2_EXE_TUNING_SHIFT                      6
#define SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_MASK                 0x0030
#define SDHCI_SDH_HOST_CTRL_2_DRV_STRENGTH_SEL_SHIFT                4
#define SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_MASK                       0x0008
#define SDHCI_SDH_HOST_CTRL_2_SDH_V18_EN_SHIFT                      3
#define SDHCI_SDH_HOST_CTRL_2_UHS_MODE_SEL_MASK                     0x0007


////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_1 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CAPABILITIES_1_REGISTER_RESET_VALUE 0x0099
#define SDHCI_SDH_CAPABILITIES_1_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_CAPABILITIES_1_RESERVED_0_MASK      0x0040
#define SDHCI_SDH_CAPABILITIES_1_RO_MASK              0xFFBF
//Bitfield Constants Register SDH_CAPABILITIES_1
#define SDHCI_SDH_CAPABILITIES_1_BASE_FREQ_MASK                        0xFF00
#define SDHCI_SDH_CAPABILITIES_1_BASE_FREQ_SHIFT                       8
#define SDHCI_SDH_CAPABILITIES_1_TIMEOUT_UNIT_MASK                     0x0080
#define SDHCI_SDH_CAPABILITIES_1_TIMEOUT_UNIT_SHIFT                    7
#define SDHCI_SDH_CAPABILITIES_1_TIMEOUT_FREQ_MASK                     0x003F


////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_2 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CAPABILITIES_2_REGISTER_RESET_VALUE 0x25EC
#define SDHCI_SDH_CAPABILITIES_2_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_CAPABILITIES_2_RESERVED_0_MASK      0x0800
#define SDHCI_SDH_CAPABILITIES_2_RW_MASK              0xC000
#define SDHCI_SDH_CAPABILITIES_2_RO_MASK              0x37FF
//Bitfield Constants Register SDH_CAPABILITIES_2
#define SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_MASK                    0xC000
#define SDHCI_SDH_CAPABILITIES_2_CFG_SLOT_TYPE_SHIFT                   14
#define SDHCI_SDH_CAPABILITIES_2_ASYNC_INT_SUPP_MASK                   0x2000
#define SDHCI_SDH_CAPABILITIES_2_ASYNC_INT_SUPP_SHIFT                  13
#define SDHCI_SDH_CAPABILITIES_2_SYS_BUS_64_SUPP_MASK                  0x1000
#define SDHCI_SDH_CAPABILITIES_2_SYS_BUS_64_SUPP_SHIFT                 12
#define SDHCI_SDH_CAPABILITIES_2_VLG_18_SUPPORT_MASK                   0x0400
#define SDHCI_SDH_CAPABILITIES_2_VLG_18_SUPPORT_SHIFT                  10
#define SDHCI_SDH_CAPABILITIES_2_VLG_30_SUPPORT_MASK                   0x0200
#define SDHCI_SDH_CAPABILITIES_2_VLG_30_SUPPORT_SHIFT                  9
#define SDHCI_SDH_CAPABILITIES_2_VLG_33_SUPPORT_MASK                   0x0100
#define SDHCI_SDH_CAPABILITIES_2_VLG_33_SUPPORT_SHIFT                  8
#define SDHCI_SDH_CAPABILITIES_2_SUS_RES_SUPPORT_MASK                  0x0080
#define SDHCI_SDH_CAPABILITIES_2_SUS_RES_SUPPORT_SHIFT                 7
#define SDHCI_SDH_CAPABILITIES_2_SDMA_SUPPORT_MASK                     0x0040
#define SDHCI_SDH_CAPABILITIES_2_SDMA_SUPPORT_SHIFT                    6
#define SDHCI_SDH_CAPABILITIES_2_HI_SPEED_SUPPORT_MASK                 0x0020
#define SDHCI_SDH_CAPABILITIES_2_HI_SPEED_SUPPORT_SHIFT                5
#define SDHCI_SDH_CAPABILITIES_2_ADMA1_SUPPORT_MASK                    0x0010
#define SDHCI_SDH_CAPABILITIES_2_ADMA1_SUPPORT_SHIFT                   4
#define SDHCI_SDH_CAPABILITIES_2_ADMA2_SUPPORT_MASK                    0x0008
#define SDHCI_SDH_CAPABILITIES_2_ADMA2_SUPPORT_SHIFT                   3
#define SDHCI_SDH_CAPABILITIES_2_EX_DATA_WIDTH_SUPPORT_MASK            0x0004
#define SDHCI_SDH_CAPABILITIES_2_EX_DATA_WIDTH_SUPPORT_SHIFT           2
#define SDHCI_SDH_CAPABILITIES_2_MAX_BLK_LEN_MASK                      0x0003


////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_3 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CAPABILITIES_3_REGISTER_RESET_VALUE 0xAF77
#define SDHCI_SDH_CAPABILITIES_3_REGISTER_RESET_MASK  0xEF77
#define SDHCI_SDH_CAPABILITIES_3_RO_MASK              0xEF77
//Bitfield Constants Register SDH_CAPABILITIES_3
#define SDHCI_SDH_CAPABILITIES_3_RETUNE_MODES_MASK                     0xC000
#define SDHCI_SDH_CAPABILITIES_3_RETUNE_MODES_SHIFT                    14
#define SDHCI_SDH_CAPABILITIES_3_SDR50_TUNE_MASK                       0x2000
#define SDHCI_SDH_CAPABILITIES_3_SDR50_TUNE_SHIFT                      13
#define SDHCI_SDH_CAPABILITIES_3_TMR_RETUNE_MASK                       0x0F00
#define SDHCI_SDH_CAPABILITIES_3_TMR_RETUNE_SHIFT                      8
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_D_MASK                       0x0040
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_D_SHIFT                      6
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_C_MASK                       0x0020
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_C_SHIFT                      5
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_A_MASK                       0x0010
#define SDHCI_SDH_CAPABILITIES_3_DRV_TYPE_A_SHIFT                      4
#define SDHCI_SDH_CAPABILITIES_3_DDR50_SUPPORT_MASK                    0x0004
#define SDHCI_SDH_CAPABILITIES_3_DDR50_SUPPORT_SHIFT                   2
#define SDHCI_SDH_CAPABILITIES_3_SDR104_SUPPORT_MASK                   0x0002
#define SDHCI_SDH_CAPABILITIES_3_SDR104_SUPPORT_SHIFT                  1
#define SDHCI_SDH_CAPABILITIES_3_SDR50_SUPPORT_MASK                    0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_CAPABILITIES_4 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CAPABILITIES_4_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_CAPABILITIES_4_REGISTER_RESET_MASK  0x00FF
#define SDHCI_SDH_CAPABILITIES_4_RO_MASK              0x00FF
//Bitfield Constants Register SDH_CAPABILITIES_4
#define SDHCI_SDH_CAPABILITIES_4_CLK_MULTIPLIER_MASK                   0x00FF


////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_1 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_MAX_CURRENT_1_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_MAX_CURRENT_1_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_MAX_CURRENT_1_RO_MASK              0xFFFF
//Bitfield Constants Register SDH_MAX_CURRENT_1
#define SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_30_MASK                       0xFF00
#define SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_30_SHIFT                      8
#define SDHCI_SDH_MAX_CURRENT_1_MAX_CUR_33_MASK                       0x00FF


////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_2 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_MAX_CURRENT_2_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_MAX_CURRENT_2_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_MAX_CURRENT_2_RESERVED_0_MASK      0xFF00
#define SDHCI_SDH_MAX_CURRENT_2_RO_MASK              0x00FF
//Bitfield Constants Register SDH_MAX_CURRENT_2
#define SDHCI_SDH_MAX_CURRENT_2_MAX_CUR_18_MASK                       0x00FF


////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_3 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_MAX_CURRENT_3_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_MAX_CURRENT_3_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_MAX_CURRENT_3_RESERVED_0_MASK      0xFFFF
//Bitfield Constants Register SDH_MAX_CURRENT_3


////////////////////////////////////////////////////////////////////
// Register SDH_MAX_CURRENT_4 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_MAX_CURRENT_4_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_MAX_CURRENT_4_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_MAX_CURRENT_4_RESERVED_0_MASK      0xFFFF
//Bitfield Constants Register SDH_MAX_CURRENT_4


////////////////////////////////////////////////////////////////////
// Register SDH_FORCE_EVENT_ACMD12ERR Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_RESERVED_0_MASK      0xFF60
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_WO_MASK              0x009F
//Bitfield Constants Register SDH_FORCE_EVENT_ACMD12ERR
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_MASK               0x0080
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_ISSUE_ERR_SHIFT              7
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_MASK               0x0010
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_INDEX_ERR_SHIFT              4
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_MASK                0x0008
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_EBIT_ERR_SHIFT               3
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_MASK                 0x0004
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_CRC_ERR_SHIFT                2
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_MASK                  0x0002
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_TO_ERR_SHIFT                 1
#define SDHCI_SDH_FORCE_EVENT_ACMD12ERR_F_ACMD12_NEXE_ERR_MASK                0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_FORCE_EVENT_ERROR_STATUS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_RESERVED_0_MASK      0x0C00
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_WO_MASK              0xF3FF
//Bitfield Constants Register SDH_FORCE_EVENT_ERROR_STATUS
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_MASK                 0x8000
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CRC_STATUS_ERR_SHIFT                15
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_MASK                0x4000
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CPL_TIMEOUT_ERR_SHIFT               14
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_MASK                   0x2000
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_AXI_RESP_ERR_SHIFT                  13
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_MASK                        0x1000
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_SPI_ERR_SHIFT                       12
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_MASK                       0x0200
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ADMA_ERR_SHIFT                      9
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_MASK                     0x0100
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_ACMD12_ERR_SHIFT                    8
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_MASK                    0x0080
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CURRENT_ERR_SHIFT                   7
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_MASK                0x0040
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_END_BIT_ERR_SHIFT               6
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_MASK                    0x0020
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_CRC_ERR_SHIFT                   5
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_MASK                     0x0010
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_DAT_TO_ERR_SHIFT                    4
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_MASK                  0x0008
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_INDEX_ERR_SHIFT                 3
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_MASK                0x0004
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_END_BIT_ERR_SHIFT               2
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_MASK                    0x0002
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_CRC_ERR_SHIFT                   1
#define SDHCI_SDH_FORCE_EVENT_ERROR_STATUS_F_CMD_TO_ERR_MASK                     0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_ERROR_STATUS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_ADMA_ERROR_STATUS_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_ADMA_ERROR_STATUS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_ADMA_ERROR_STATUS_RESERVED_0_MASK      0xFFF8
#define SDHCI_SDH_ADMA_ERROR_STATUS_RW_MASK              0x0007
//Bitfield Constants Register SDH_ADMA_ERROR_STATUS
#define SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_MASK                     0x0004
#define SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_LEN_ERR_SHIFT                    2
#define SDHCI_SDH_ADMA_ERROR_STATUS_ADMA_STATE_MASK                       0x0003


////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_SYSADDR0 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_ADMA_SYSADDR0_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_ADMA_SYSADDR0_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_ADMA_SYSADDR0_RW_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_ADMA_SYSADDR0


////////////////////////////////////////////////////////////////////
// Register SDH_ADMA_SYSADDR1 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_ADMA_SYSADDR1_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_ADMA_SYSADDR1_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_ADMA_SYSADDR1_RW_MASK              0xFFFFFFFF
//Bitfield Constants Register SDH_ADMA_SYSADDR1


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_INIT Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_INIT_REGISTER_RESET_VALUE 0x01F4
#define SDHCI_PRESET_VALUE_FOR_INIT_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_INIT_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_INIT_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_INIT
#define SDHCI_PRESET_VALUE_FOR_INIT_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_INIT_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_INIT_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_INIT_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_INIT_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_DS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_DS_REGISTER_RESET_VALUE 0x0008
#define SDHCI_PRESET_VALUE_FOR_DS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_DS_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_DS_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_DS
#define SDHCI_PRESET_VALUE_FOR_DS_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_DS_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_DS_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_DS_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_DS_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_HS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_HS_REGISTER_RESET_VALUE 0x0004
#define SDHCI_PRESET_VALUE_FOR_HS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_HS_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_HS_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_HS
#define SDHCI_PRESET_VALUE_FOR_HS_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_HS_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_HS_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_HS_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_HS_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR12 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_SDR12_REGISTER_RESET_VALUE 0x0008
#define SDHCI_PRESET_VALUE_FOR_SDR12_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_SDR12_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_SDR12_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_SDR12
#define SDHCI_PRESET_VALUE_FOR_SDR12_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_SDR12_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_SDR12_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_SDR12_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_SDR12_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR25 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_SDR25_REGISTER_RESET_VALUE 0x0004
#define SDHCI_PRESET_VALUE_FOR_SDR25_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_SDR25_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_SDR25_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_SDR25
#define SDHCI_PRESET_VALUE_FOR_SDR25_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_SDR25_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_SDR25_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_SDR25_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_SDR25_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR50 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_SDR50_REGISTER_RESET_VALUE 0x0002
#define SDHCI_PRESET_VALUE_FOR_SDR50_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_SDR50_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_SDR50_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_SDR50
#define SDHCI_PRESET_VALUE_FOR_SDR50_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_SDR50_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_SDR50_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_SDR50_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_SDR50_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_SDR104 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_SDR104_REGISTER_RESET_VALUE 0x0001
#define SDHCI_PRESET_VALUE_FOR_SDR104_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_SDR104_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_SDR104_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_SDR104
#define SDHCI_PRESET_VALUE_FOR_SDR104_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_SDR104_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_SDR104_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_SDR104_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_SDR104_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register PRESET_VALUE_FOR_DDR50 Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_PRESET_VALUE_FOR_DDR50_REGISTER_RESET_VALUE 0x0004
#define SDHCI_PRESET_VALUE_FOR_DDR50_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_PRESET_VALUE_FOR_DDR50_RESERVED_0_MASK      0x3800
#define SDHCI_PRESET_VALUE_FOR_DDR50_RO_MASK              0xC7FF
//Bitfield Constants Register PRESET_VALUE_FOR_DDR50
#define SDHCI_PRESET_VALUE_FOR_DDR50_DRV_STRENGTH_VAL_MASK                 0xC000
#define SDHCI_PRESET_VALUE_FOR_DDR50_DRV_STRENGTH_VAL_SHIFT                14
#define SDHCI_PRESET_VALUE_FOR_DDR50_CLKGEN_SEL_VAL_MASK                   0x0400
#define SDHCI_PRESET_VALUE_FOR_DDR50_CLKGEN_SEL_VAL_SHIFT                  10
#define SDHCI_PRESET_VALUE_FOR_DDR50_CLK_FREQ_SEL_VAL_MASK                 0x03FF


////////////////////////////////////////////////////////////////////
// Register SHARED_BUS_CTRL Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SHARED_BUS_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SHARED_BUS_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SHARED_BUS_CTRL_RESERVED_0_MASK      0x808880C8
#define SDHCI_SHARED_BUS_CTRL_RW_MASK              0x7F770000
#define SDHCI_SHARED_BUS_CTRL_RO_MASK              0x00007F37
//Bitfield Constants Register SHARED_BUS_CTRL
#define SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_MASK                    0x7F000000
#define SDHCI_SHARED_BUS_CTRL_BEND_PWR_CTRL_SHIFT                   24
#define SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_MASK                      0x00700000
#define SDHCI_SHARED_BUS_CTRL_INT_PIN_SEL_SHIFT                     20
#define SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_MASK                      0x00070000
#define SDHCI_SHARED_BUS_CTRL_CLK_PIN_SEL_SHIFT                     16
#define SDHCI_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_MASK                 0x00007F00
#define SDHCI_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_SHIFT                8
#define SDHCI_SHARED_BUS_CTRL_NUM_INT_PINS_MASK                     0x00000030
#define SDHCI_SHARED_BUS_CTRL_NUM_INT_PINS_SHIFT                    4
#define SDHCI_SHARED_BUS_CTRL_NUM_CLK_PINS_MASK                     0x00000007


////////////////////////////////////////////////////////////////////
// Register SDH_SLOT_INT_STATUS Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_SLOT_INT_STATUS_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_SLOT_INT_STATUS_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_SLOT_INT_STATUS_RESERVED_0_MASK      0xFFF0
#define SDHCI_SDH_SLOT_INT_STATUS_RO_MASK              0x000F
//Bitfield Constants Register SDH_SLOT_INT_STATUS
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT3_MASK                        0x0008
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT3_SHIFT                       3
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT2_MASK                        0x0004
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT2_SHIFT                       2
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT1_MASK                        0x0002
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT1_SHIFT                       1
#define SDHCI_SDH_SLOT_INT_STATUS_SLOT_INT0_MASK                        0x0001


////////////////////////////////////////////////////////////////////
// Register SDH_CTRL_VER Masks
////////////////////////////////////////////////////////////////////
#define SDHCI_SDH_CTRL_VER_REGISTER_RESET_VALUE 0x0002
#define SDHCI_SDH_CTRL_VER_REGISTER_RESET_MASK  0xFFFF
#define SDHCI_SDH_CTRL_VER_RO_MASK              0xFFFF
//Bitfield Constants Register SDH_CTRL_VER
#define SDHCI_SDH_CTRL_VER_VENDOR_VER_MASK                       0xFF00
#define SDHCI_SDH_CTRL_VER_VENDOR_VER_SHIFT                      8
#define SDHCI_SDH_CTRL_VER_HOST_VER_MASK                         0x00FF

// Unified Register SDH_TP_MONITOE Constants
#define SDHCI_SDH_TP_MONITOE_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_TP_MONITOE_REGISTER_RESET_MASK  0x0007FFFF
#define SDHCI_SDH_TP_MONITOE_RESERVED_0_MASK      0xFFF80000
#define SDHCI_SDH_TP_MONITOE_RW_MASK              0x0007FFFF
//Unified Bitfield Constants Register SDH_TP_MONITOE
#define SDHCI_SDH_TP_MONITOE_TP_SELECT_R_MASK                      0x00070000
#define SDHCI_SDH_TP_MONITOE_TP_SELECT_R_SHIFT                     16
#define SDHCI_SDH_TP_MONITOE_TP_VALUE_R_MASK                       0x0000FFFF

// Unified Register SDH_TP_CTRL Constants
#define SDHCI_SDH_TP_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_TP_CTRL_REGISTER_RESET_MASK  0x000FFFFF
#define SDHCI_SDH_TP_CTRL_RESERVED_0_MASK      0xFFF00000
#define SDHCI_SDH_TP_CTRL_RW_MASK              0x000FFFFF
//Unified Bitfield Constants Register SDH_TP_CTRL
#define SDHCI_SDH_TP_CTRL_BOOTTEST_MODE_MASK                    0x00080000
#define SDHCI_SDH_TP_CTRL_BOOTTEST_MODE_SHIFT                   19
#define SDHCI_SDH_TP_CTRL_TP_SELECT_W_MASK                      0x00070000
#define SDHCI_SDH_TP_CTRL_TP_SELECT_W_SHIFT                     16
#define SDHCI_SDH_TP_CTRL_TP_VALUE_W_MASK                       0x0000FFFF

// Unified Register SDHC_SLOT_AUTO_RETUNING_CTRL Constants
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_REGISTER_RESET_VALUE 0x00000100
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_RESERVED_0_MASK      0x000000F8
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_RW_MASK              0xFFFFFF07
//Unified Bitfield Constants Register SDHC_SLOT_AUTO_RETUNING_CTRL
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_NEXT_DISTANCE_MASK                    0xFF000000
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_NEXT_DISTANCE_SHIFT                   24
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_PREV_DISTANCE_MASK                    0x00FF0000
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_PREV_DISTANCE_SHIFT                   16
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_ADJUSTMENT_MASK         0x0000FF00
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_ADJUSTMENT_SHIFT        8
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_MON_DATA_MASK           0x00000004
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_MON_DATA_SHIFT          2
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_MON_RESP_MASK           0x00000002
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_AUTO_RETUNING_MON_RESP_SHIFT          1
#define SDHCI_SDHC_SLOT_AUTO_RETUNING_CTRL_ENABLE_AUTO_RETUNING_MASK             0x00000001

// Unified Register SDH_SLOT_STROBE_DLY_CTRL Constants
#define SDHCI_SDH_SLOT_STROBE_DLY_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_STROBE_DLY_CTRL_REGISTER_RESET_MASK  0x0000FFFF
#define SDHCI_SDH_SLOT_STROBE_DLY_CTRL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_STROBE_DLY_CTRL_RW_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_STROBE_DLY_CTRL
#define SDHCI_SDH_SLOT_STROBE_DLY_CTRL_STROBE_DELAY_FIXED_MASK               0x0000FFFF

// Unified Register SDH_SLOT_DLL_CTRL Constants
#define SDHCI_SDH_SLOT_DLL_CTRL_REGISTER_RESET_VALUE 0x01000800
#define SDHCI_SDH_SLOT_DLL_CTRL_REGISTER_RESET_MASK  0x03FFFFFF
#define SDHCI_SDH_SLOT_DLL_CTRL_RESERVED_0_MASK      0xFC00E0C0
#define SDHCI_SDH_SLOT_DLL_CTRL_RW_MASK              0x03FF1F3F
//Unified Bitfield Constants Register SDH_SLOT_DLL_CTRL
#define SDHCI_SDH_SLOT_DLL_CTRL_TEST_EN_MASK                          0x02000000
#define SDHCI_SDH_SLOT_DLL_CTRL_TEST_EN_SHIFT                         25
#define SDHCI_SDH_SLOT_DLL_CTRL_GAIN2X_MASK                           0x01000000
#define SDHCI_SDH_SLOT_DLL_CTRL_GAIN2X_SHIFT                          24
#define SDHCI_SDH_SLOT_DLL_CTRL_UNLOCK_THRESHOLD_MASK                 0x00F00000
#define SDHCI_SDH_SLOT_DLL_CTRL_UNLOCK_THRESHOLD_SHIFT                20
#define SDHCI_SDH_SLOT_DLL_CTRL_LOCK_THRESHOLD_MASK                   0x000F0000
#define SDHCI_SDH_SLOT_DLL_CTRL_LOCK_THRESHOLD_SHIFT                  16
#define SDHCI_SDH_SLOT_DLL_CTRL_DELTA_VALUE_MASK                      0x00001F00
#define SDHCI_SDH_SLOT_DLL_CTRL_DELTA_VALUE_SHIFT                     8
#define SDHCI_SDH_SLOT_DLL_CTRL_FAST_LOCK_EN_MASK                     0x00000020
#define SDHCI_SDH_SLOT_DLL_CTRL_FAST_LOCK_EN_SHIFT                    5
#define SDHCI_SDH_SLOT_DLL_CTRL_LOCK_MODE_MASK                        0x00000010
#define SDHCI_SDH_SLOT_DLL_CTRL_LOCK_MODE_SHIFT                       4
#define SDHCI_SDH_SLOT_DLL_CTRL_EXE_LOCK_MASK                         0x00000008
#define SDHCI_SDH_SLOT_DLL_CTRL_EXE_LOCK_SHIFT                        3
#define SDHCI_SDH_SLOT_DLL_CTRL_RESET_DLL_MASK                        0x00000004
#define SDHCI_SDH_SLOT_DLL_CTRL_RESET_DLL_SHIFT                       2
#define SDHCI_SDH_SLOT_DLL_CTRL_ENABLE_DLL_MASK                       0x00000002
#define SDHCI_SDH_SLOT_DLL_CTRL_ENABLE_DLL_SHIFT                      1
#define SDHCI_SDH_SLOT_DLL_CTRL_DLL_SELECT_MASK                       0x00000001

// Unified Register SDH_SLOT_OUTPUT_DLY_CTRL Constants
#define SDHCI_SDH_SLOT_OUTPUT_DLY_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_OUTPUT_DLY_CTRL_REGISTER_RESET_MASK  0x0000FFFF
#define SDHCI_SDH_SLOT_OUTPUT_DLY_CTRL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_OUTPUT_DLY_CTRL_RW_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_OUTPUT_DLY_CTRL
#define SDHCI_SDH_SLOT_OUTPUT_DLY_CTRL_HOST_OUTPUT_DLY_MASK                  0x0000FFFF

// Unified Register PHY_LB_COMP_CNT Constants
#define SDHCI_PHY_LB_COMP_CNT_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_PHY_LB_COMP_CNT_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_PHY_LB_COMP_CNT_RW_MASK              0xFFFFFFFF
//Unified Bitfield Constants Register PHY_LB_COMP_CNT

// Unified Register PHY_LB_ERR_STATUS Constants
#define SDHCI_PHY_LB_ERR_STATUS_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_PHY_LB_ERR_STATUS_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_PHY_LB_ERR_STATUS_RESERVED_0_MASK      0x0E00FE00
#define SDHCI_PHY_LB_ERR_STATUS_RO_MASK              0xF1FF01FF
//Unified Bitfield Constants Register PHY_LB_ERR_STATUS
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_0_DET_ERR_QSN_MASK              0x80000000
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_0_DET_ERR_QSN_SHIFT             31
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_1_DET_ERR_QSN_MASK              0x40000000
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_1_DET_ERR_QSN_SHIFT             30
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_0_DET_ERR_QSP_MASK              0x20000000
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_0_DET_ERR_QSP_SHIFT             29
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_1_DET_ERR_QSP_MASK              0x10000000
#define SDHCI_PHY_LB_ERR_STATUS_STUCK_1_DET_ERR_QSP_SHIFT             28
#define SDHCI_PHY_LB_ERR_STATUS_DM_EVEN_PIN_ERR_MASK                  0x01000000
#define SDHCI_PHY_LB_ERR_STATUS_DM_EVEN_PIN_ERR_SHIFT                 24
#define SDHCI_PHY_LB_ERR_STATUS_DQ_EVEN_PIN_ERR_MASK                  0x00FF0000
#define SDHCI_PHY_LB_ERR_STATUS_DQ_EVEN_PIN_ERR_SHIFT                 16
#define SDHCI_PHY_LB_ERR_STATUS_DM_ODD_PIN_ERR_MASK                   0x00000100
#define SDHCI_PHY_LB_ERR_STATUS_DM_ODD_PIN_ERR_SHIFT                  8
#define SDHCI_PHY_LB_ERR_STATUS_DQ_ODD_PIN_ERR_MASK                   0x000000FF

// Unified Register PHY_LB_FUNC_CONTROL Constants
#define SDHCI_PHY_LB_FUNC_CONTROL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_PHY_LB_FUNC_CONTROL_REGISTER_RESET_MASK  0x8000001F
#define SDHCI_PHY_LB_FUNC_CONTROL_RESERVED_0_MASK      0x7FFFFFE0
#define SDHCI_PHY_LB_FUNC_CONTROL_RW_MASK              0x8000001F
//Unified Bitfield Constants Register PHY_LB_FUNC_CONTROL
#define SDHCI_PHY_LB_FUNC_CONTROL_CLEAR_LB_ERR_STATUS_MASK              0x80000000
#define SDHCI_PHY_LB_FUNC_CONTROL_CLEAR_LB_ERR_STATUS_SHIFT             31
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK0_DET_QSN_MASK             0x00000010
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK0_DET_QSN_SHIFT            4
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK1_DET_QSN_MASK             0x00000008
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK1_DET_QSN_SHIFT            3
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK0_DET_QSP_MASK             0x00000004
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK0_DET_QSP_SHIFT            2
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK1_DET_QSP_MASK             0x00000002
#define SDHCI_PHY_LB_FUNC_CONTROL_START_STUCK1_DET_QSP_SHIFT            1
#define SDHCI_PHY_LB_FUNC_CONTROL_START_LP_TEST_MASK                    0x00000001

// Unified Register PHY_LB_DATA_PATTERN Constants
#define SDHCI_PHY_LB_DATA_PATTERN_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_PHY_LB_DATA_PATTERN_REGISTER_RESET_MASK  0xC003FFFF
#define SDHCI_PHY_LB_DATA_PATTERN_RESERVED_0_MASK      0x3FFC0000
#define SDHCI_PHY_LB_DATA_PATTERN_RW_MASK              0xC003FFFF
//Unified Bitfield Constants Register PHY_LB_DATA_PATTERN
#define SDHCI_PHY_LB_DATA_PATTERN_RESET_PATTERN_MASK                    0x80000000
#define SDHCI_PHY_LB_DATA_PATTERN_RESET_PATTERN_SHIFT                   31
#define SDHCI_PHY_LB_DATA_PATTERN_WRITE_PATTERN_MASK                    0x40000000
#define SDHCI_PHY_LB_DATA_PATTERN_WRITE_PATTERN_SHIFT                   30
#define SDHCI_PHY_LB_DATA_PATTERN_LB_DATA_PATTERN_MASK                  0x0003FFFF

// Unified Register PHY_LB_FUNC_CONFIG Constants
#define SDHCI_PHY_LB_FUNC_CONFIG_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_PHY_LB_FUNC_CONFIG_REGISTER_RESET_MASK  0xFFDFFFFF
#define SDHCI_PHY_LB_FUNC_CONFIG_RESERVED_0_MASK      0x020000FF
#define SDHCI_PHY_LB_FUNC_CONFIG_RW_MASK              0xFDFFFF00
//Unified Bitfield Constants Register PHY_LB_FUNC_CONFIG
#define SDHCI_PHY_LB_FUNC_CONFIG_LB_MODE_EN_MASK                       0x80000000
#define SDHCI_PHY_LB_FUNC_CONFIG_LB_MODE_EN_SHIFT                      31
#define SDHCI_PHY_LB_FUNC_CONFIG_LB_PATTERN_SEL_MASK                   0x40000000
#define SDHCI_PHY_LB_FUNC_CONFIG_LB_PATTERN_SEL_SHIFT                  30
#define SDHCI_PHY_LB_FUNC_CONFIG_NEED_QSP_MASK                         0x20000000
#define SDHCI_PHY_LB_FUNC_CONFIG_NEED_QSP_SHIFT                        29
#define SDHCI_PHY_LB_FUNC_CONFIG_NEED_QSN_MASK                         0x10000000
#define SDHCI_PHY_LB_FUNC_CONFIG_NEED_QSN_SHIFT                        28
#define SDHCI_PHY_LB_FUNC_CONFIG_INV_QSP_MASK                          0x08000000
#define SDHCI_PHY_LB_FUNC_CONFIG_INV_QSP_SHIFT                         27
#define SDHCI_PHY_LB_FUNC_CONFIG_INV_QSN_MASK                          0x04000000
#define SDHCI_PHY_LB_FUNC_CONFIG_INV_QSN_SHIFT                         26
#define SDHCI_PHY_LB_FUNC_CONFIG_CMD_LB_MASK_MASK                      0x01000000
#define SDHCI_PHY_LB_FUNC_CONFIG_CMD_LB_MASK_SHIFT                     24
#define SDHCI_PHY_LB_FUNC_CONFIG_DQ_LB_MASK_MASK                       0x00FF0000
#define SDHCI_PHY_LB_FUNC_CONFIG_DQ_LB_MASK_SHIFT                      16
#define SDHCI_PHY_LB_FUNC_CONFIG_FILTER_BITS_CNT_MASK                  0x0000F000
#define SDHCI_PHY_LB_FUNC_CONFIG_FILTER_BITS_CNT_SHIFT                 12
#define SDHCI_PHY_LB_FUNC_CONFIG_QSO_AFTER_RDRST_MASK                  0x00000F00
#define SDHCI_PHY_LB_FUNC_CONFIG_QSO_AFTER_RDRST_SHIFT                 8

// Unified Register EMMC_LOGIC_TIMING_ADJUST_LOW Constants
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_REGISTER_RESET_VALUE 0x00031F11
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_REGISTER_RESET_MASK  0x000FFFFF
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_RESERVED_0_MASK      0xFFF00000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_RW_MASK              0x000FFFFF
//Unified Bitfield Constants Register EMMC_LOGIC_TIMING_ADJUST_LOW
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_INT_DET_LOW_PARAM_MASK            0x000F0000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_INT_DET_LOW_PARAM_SHIFT           16
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_INT_OFF_LOW_PARAM_MASK            0x0000F000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_INT_OFF_LOW_PARAM_SHIFT           12
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_STOP_CLK_LOW_PARAM_MASK           0x00000F00
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_STOP_CLK_LOW_PARAM_SHIFT          8
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_WR_DLY_LOW_PARAM_MASK             0x000000F0
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_WR_DLY_LOW_PARAM_SHIFT            4
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_LOW_PHY_RESP_DLY_LOW_PARAM_MASK           0x0000000F

// Unified Register EMMC_LOGIC_TIMING_ADJUST Constants
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_REGISTER_RESET_VALUE 0x00AA8877
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_REGISTER_RESET_MASK  0x00FFFFFF
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_RESERVED_0_MASK      0xFF000000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_RW_MASK              0x00FFFFFF
//Unified Bitfield Constants Register EMMC_LOGIC_TIMING_ADJUST
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_RW_PARAM_MASK                     0x00F00000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_RW_PARAM_SHIFT                    20
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_INT_DET_PARAM_MASK                0x000F0000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_INT_DET_PARAM_SHIFT               16
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_RD_CLK_PARAM_MASK                 0x0000F000
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_RD_CLK_PARAM_SHIFT                12
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_STOP_CLK_PARAM_MASK               0x00000F00
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_STOP_CLK_PARAM_SHIFT              8
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_WR_DLY_PARAM_MASK                 0x000000F0
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_WR_DLY_PARAM_SHIFT                4
#define SDHCI_EMMC_LOGIC_TIMING_ADJUST_PHY_RESP_DLY_PARAM_MASK               0x0000000F

// Unified Register EMMC_PHY_DLL_CONTROL Constants
#define SDHCI_EMMC_PHY_DLL_CONTROL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_EMMC_PHY_DLL_CONTROL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_EMMC_PHY_DLL_CONTROL_RESERVED_0_MASK      0x00400000
#define SDHCI_EMMC_PHY_DLL_CONTROL_RW_MASK              0xFFBFFFFF
//Unified Bitfield Constants Register EMMC_PHY_DLL_CONTROL
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_ENABLE_MASK                       0x80000000
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_ENABLE_SHIFT                      31
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_REFCLK_SEL_MASK                   0x40000000
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_REFCLK_SEL_SHIFT                  30
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_PHASE1_MASK                       0x3F000000
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_PHASE1_SHIFT                      24
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_UPDATE_MASK                       0x00800000
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_UPDATE_SHIFT                      23
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_PHASE0_MASK                       0x003F0000
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_PHASE0_SHIFT                      16
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_DELAY_TEST_LOWER_MASK             0x0000FF00
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_DELAY_TEST_LOWER_SHIFT            8
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_DELAY_TEST_UP_MASK                0x000000C0
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_DELAY_TEST_UP_SHIFT               6
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_FAST_LOCK_MASK                    0x00000020
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_FAST_LOCK_SHIFT                   5
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_MC_IDLE_MASK                      0x00000010
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_MC_IDLE_SHIFT                     4
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_GAIN2X_MASK                       0x00000008
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_GAIN2X_SHIFT                      3
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_AUTO_UPDATE_EN_MASK               0x00000004
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_AUTO_UPDATE_EN_SHIFT              2
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_TEST_EN_MASK                      0x00000002
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_TEST_EN_SHIFT                     1
#define SDHCI_EMMC_PHY_DLL_CONTROL_DLL_BYPASS_EN_MASK                    0x00000001

// Unified Register EMMC_PHY_PAD_CONTROL2 Constants
#define SDHCI_EMMC_PHY_PAD_CONTROL2_REGISTER_RESET_VALUE 0x00000F0F
#define SDHCI_EMMC_PHY_PAD_CONTROL2_REGISTER_RESET_MASK  0x00071F1F
#define SDHCI_EMMC_PHY_PAD_CONTROL2_RESERVED_0_MASK      0xFFF8E0E0
#define SDHCI_EMMC_PHY_PAD_CONTROL2_RW_MASK              0x00071F1F
//Unified Bitfield Constants Register EMMC_PHY_PAD_CONTROL2
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ZD_MASK                               0x00040000
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ZD_SHIFT                              18
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ODT_SEL1_MASK                         0x00020000
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ODT_SEL1_SHIFT                        17
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ODT_SEL0_MASK                         0x00010000
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ODT_SEL0_SHIFT                        16
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ZNR_MASK                              0x00001F00
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ZNR_SHIFT                             8
#define SDHCI_EMMC_PHY_PAD_CONTROL2_ZPR_MASK                              0x0000001F

// Unified Register EMMC_PHY_PAD_CONTROL1 Constants
#define SDHCI_EMMC_PHY_PAD_CONTROL1_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_EMMC_PHY_PAD_CONTROL1_REGISTER_RESET_MASK  0x07FF07FF
#define SDHCI_EMMC_PHY_PAD_CONTROL1_RESERVED_0_MASK      0xF800F800
#define SDHCI_EMMC_PHY_PAD_CONTROL1_RW_MASK              0x07FF07FF
//Unified Bitfield Constants Register EMMC_PHY_PAD_CONTROL1
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSN_PU_MASK                        0x04000000
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSN_PU_SHIFT                       26
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSP_PU_MASK                        0x02000000
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSP_PU_SHIFT                       25
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_CMD_PU_MASK                        0x01000000
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_CMD_PU_SHIFT                       24
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_DQ_PU_MASK                         0x00FF0000
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_DQ_PU_SHIFT                        16
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSN_PD_MASK                        0x00000400
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSN_PD_SHIFT                       10
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSP_PD_MASK                        0x00000200
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_QSP_PD_SHIFT                       9
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_CMD_PD_MASK                        0x00000100
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_CMD_PD_SHIFT                       8
#define SDHCI_EMMC_PHY_PAD_CONTROL1_FC_DQ_PD_MASK                         0x000000FF

// Unified Register EMMC_PHY_PAD_CONTROL Constants
#define SDHCI_EMMC_PHY_PAD_CONTROL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_EMMC_PHY_PAD_CONTROL_REGISTER_RESET_MASK  0x7FFFF7FF
#define SDHCI_EMMC_PHY_PAD_CONTROL_RESERVED_0_MASK      0x80FF0800
#define SDHCI_EMMC_PHY_PAD_CONTROL_RW_MASK              0x7F00F7FF
//Unified Bitfield Constants Register EMMC_PHY_PAD_CONTROL
#define SDHCI_EMMC_PHY_PAD_CONTROL_AUTO_RECEN_CTRL_MASK                  0x40000000
#define SDHCI_EMMC_PHY_PAD_CONTROL_AUTO_RECEN_CTRL_SHIFT                 30
#define SDHCI_EMMC_PHY_PAD_CONTROL_OEP_MASK                              0x20000000
#define SDHCI_EMMC_PHY_PAD_CONTROL_OEP_SHIFT                             29
#define SDHCI_EMMC_PHY_PAD_CONTROL_OEN_MASK                              0x10000000
#define SDHCI_EMMC_PHY_PAD_CONTROL_OEN_SHIFT                             28
#define SDHCI_EMMC_PHY_PAD_CONTROL_REC_EN_MASK                           0x0F000000
#define SDHCI_EMMC_PHY_PAD_CONTROL_REC_EN_SHIFT                          24
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_QSN_RCVTYPE_MASK                   0x00008000
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_QSN_RCVTYPE_SHIFT                  15
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_QSP_RCVTYPE_MASK                   0x00004000
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_QSP_RCVTYPE_SHIFT                  14
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_CMD_RCVTYPE_MASK                   0x00002000
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_CMD_RCVTYPE_SHIFT                  13
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_DQ_RCVTYPE_MASK                    0x00001000
#define SDHCI_EMMC_PHY_PAD_CONTROL_FC_DQ_RCVTYPE_SHIFT                   12
#define SDHCI_EMMC_PHY_PAD_CONTROL_QSN_PWRDWN_MASK                       0x00000400
#define SDHCI_EMMC_PHY_PAD_CONTROL_QSN_PWRDWN_SHIFT                      10
#define SDHCI_EMMC_PHY_PAD_CONTROL_QSP_PWRDWN_MASK                       0x00000200
#define SDHCI_EMMC_PHY_PAD_CONTROL_QSP_PWRDWN_SHIFT                      9
#define SDHCI_EMMC_PHY_PAD_CONTROL_CMD_PWRDWN_MASK                       0x00000100
#define SDHCI_EMMC_PHY_PAD_CONTROL_CMD_PWRDWN_SHIFT                      8
#define SDHCI_EMMC_PHY_PAD_CONTROL_DQ_PWRDWN_MASK                        0x000000FF

// Unified Register EMMC_PHY_FUNC_CONTROL Constants
#define SDHCI_EMMC_PHY_FUNC_CONTROL_REGISTER_RESET_VALUE 0x01000031
#define SDHCI_EMMC_PHY_FUNC_CONTROL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_EMMC_PHY_FUNC_CONTROL_RW_MASK              0xFFFFFFFF
//Unified Bitfield Constants Register EMMC_PHY_FUNC_CONTROL
#define SDHCI_EMMC_PHY_FUNC_CONTROL_REG_PDB_CORE_MASK                     0x80000000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_REG_PDB_CORE_SHIFT                    31
#define SDHCI_EMMC_PHY_FUNC_CONTROL_QS_GATE_DLY_CTRL_MASK                 0x7E000000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_QS_GATE_DLY_CTRL_SHIFT                25
#define SDHCI_EMMC_PHY_FUNC_CONTROL_ASYNC_DLL_BYPASS_EN_MASK              0x01000000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_ASYNC_DLL_BYPASS_EN_SHIFT             24
#define SDHCI_EMMC_PHY_FUNC_CONTROL_ASYNC_DDRMODE_MASK                    0x00800000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_ASYNC_DDRMODE_SHIFT                   23
#define SDHCI_EMMC_PHY_FUNC_CONTROL_PHY_RESERVE_MASK                      0x007E0000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_PHY_RESERVE_SHIFT                     17
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_DDRMODE_MASK                      0x00010000
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_DDRMODE_SHIFT                     16
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_DDRMODE_MASK                       0x0000FF00
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_DDRMODE_SHIFT                      8
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_MODE_MASK                         0x00000080
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_MODE_SHIFT                        7
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_MODE_MASK                          0x00000040
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_MODE_SHIFT                         6
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_ASYNC_MODE_MASK                   0x00000020
#define SDHCI_EMMC_PHY_FUNC_CONTROL_CMD_ASYNC_MODE_SHIFT                  5
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_ASYNC_MODE_MASK                    0x00000010
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DQ_ASYNC_MODE_SHIFT                   4
#define SDHCI_EMMC_PHY_FUNC_CONTROL_RFIFO_BYP_MASK                        0x00000008
#define SDHCI_EMMC_PHY_FUNC_CONTROL_RFIFO_BYP_SHIFT                       3
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DSER1X_MASK                           0x00000004
#define SDHCI_EMMC_PHY_FUNC_CONTROL_DSER1X_SHIFT                          2
#define SDHCI_EMMC_PHY_FUNC_CONTROL_FC_WRST_SEL_MASK                      0x00000003

// Unified Register EMMC_PHY_TIMING_ADJUST Constants
#define SDHCI_EMMC_PHY_TIMING_ADJUST_REGISTER_RESET_VALUE 0x01005522
#define SDHCI_EMMC_PHY_TIMING_ADJUST_REGISTER_RESET_MASK  0xBF0FFFFF
#define SDHCI_EMMC_PHY_TIMING_ADJUST_RESERVED_0_MASK      0x40F80000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_RW_MASK              0xBF07FFFF
//Unified Bitfield Constants Register EMMC_PHY_TIMING_ADJUST
#define SDHCI_EMMC_PHY_TIMING_ADJUST_PHY_INITIALIZATION_MASK               0x80000000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_PHY_INITIALIZATION_SHIFT              31
#define SDHCI_EMMC_PHY_TIMING_ADJUST_SLOW_MODE_MASK                        0x20000000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_SLOW_MODE_SHIFT                       29
#define SDHCI_EMMC_PHY_TIMING_ADJUST_SDIO_MODE_MASK                        0x10000000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_SDIO_MODE_SHIFT                       28
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSP_AFTER_RDRST_MASK                  0x0F000000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSP_AFTER_RDRST_SHIFT                 24
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSP_PHASE_SELECT_MASK                 0x00040000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSP_PHASE_SELECT_SHIFT                18
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSN_PHASE_SELECT_MASK                 0x00020000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_QSN_PHASE_SELECT_SHIFT                17
#define SDHCI_EMMC_PHY_TIMING_ADJUST_POLARITY_WR_LDO_LD1_MASK              0x00010000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_POLARITY_WR_LDO_LD1_SHIFT             16
#define SDHCI_EMMC_PHY_TIMING_ADJUST_WAIT_CYCLE_BEFORE_USING_MASK          0x0000F000
#define SDHCI_EMMC_PHY_TIMING_ADJUST_WAIT_CYCLE_BEFORE_USING_SHIFT         12
#define SDHCI_EMMC_PHY_TIMING_ADJUST_FC_SYNC_EN_DURATION_MASK              0x00000F00
#define SDHCI_EMMC_PHY_TIMING_ADJUST_FC_SYNC_EN_DURATION_SHIFT             8
#define SDHCI_EMMC_PHY_TIMING_ADJUST_FC_SYNC_RST_EN_DURATION_MASK          0x000000F0
#define SDHCI_EMMC_PHY_TIMING_ADJUST_FC_SYNC_RST_EN_DURATION_SHIFT         4
#define SDHCI_EMMC_PHY_TIMING_ADJUST_FC_SYNC_RST_DURATION_MASK             0x0000000F

// Unified Register SDH_SLOT_CQ_TASK_INFO Constants
#define SDHCI_SDH_SLOT_CQ_TASK_INFO_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_CQ_TASK_INFO_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_CQ_TASK_INFO_RESERVED_0_MASK      0xFFFFFFE0
#define SDHCI_SDH_SLOT_CQ_TASK_INFO_RW_MASK              0x0000001F
//Unified Bitfield Constants Register SDH_SLOT_CQ_TASK_INFO
#define SDHCI_SDH_SLOT_CQ_TASK_INFO_TASK_FETCH_SLOT_INDEX_MASK            0x0000001F

// Unified Register SDH_SLOT_SUB_CMD_CTRL Constants
#define SDHCI_SDH_SLOT_SUB_CMD_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_SUB_CMD_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_SUB_CMD_CTRL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_SUB_CMD_CTRL_RW_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_SUB_CMD_CTRL
#define SDHCI_SDH_SLOT_SUB_CMD_CTRL_BM_MASK                               0x0000FFFF

// Unified Register SDH_SLOT_STROBE_CUR_DLY_VAL Constants
#define SDHCI_SDH_SLOT_STROBE_CUR_DLY_VAL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_STROBE_CUR_DLY_VAL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_STROBE_CUR_DLY_VAL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_STROBE_CUR_DLY_VAL_RO_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_STROBE_CUR_DLY_VAL
#define SDHCI_SDH_SLOT_STROBE_CUR_DLY_VAL_DVAL_STROBE_MASK                      0x0000FFFF

// Unified Register SDH_SLOT_TUNING_CUR_DLY_VAL Constants
#define SDHCI_SDH_SLOT_TUNING_CUR_DLY_VAL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_TUNING_CUR_DLY_VAL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_TUNING_CUR_DLY_VAL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_TUNING_CUR_DLY_VAL_RO_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_TUNING_CUR_DLY_VAL
#define SDHCI_SDH_SLOT_TUNING_CUR_DLY_VAL_DVAL_TUNING_LINE_MASK                 0x0000FFFF

// Unified Register SDH_SLOT_DLL_CUR_DLY_VAL Constants
#define SDHCI_SDH_SLOT_DLL_CUR_DLY_VAL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_DLL_CUR_DLY_VAL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_DLL_CUR_DLY_VAL_RESERVED_0_MASK      0xFFFF0000
#define SDHCI_SDH_SLOT_DLL_CUR_DLY_VAL_RO_MASK              0x0000FFFF
//Unified Bitfield Constants Register SDH_SLOT_DLL_CUR_DLY_VAL
#define SDHCI_SDH_SLOT_DLL_CUR_DLY_VAL_DVAL_IN_DLL_MASK                      0x0000FFFF

// Unified Register SDH_SLOT_EXT_PRESENT_STATE Constants
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_RESERVED_0_MASK      0xE000FCFE
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_RO_MASK              0x1FFF0301
//Unified Bitfield Constants Register SDH_SLOT_EXT_PRESENT_STATE
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_POS_OR_NEG_SEL_MASK                   0x10000000
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_POS_OR_NEG_SEL_SHIFT                  28
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_SUCCESS_TIMES_MASK                    0x0F000000
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_SUCCESS_TIMES_SHIFT                   24
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_TUNING_STEP_FROM_DLL_MASK             0x00FF0000
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_TUNING_STEP_FROM_DLL_SHIFT            16
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_RESP_IN_ASYNC_FIFO_MASK               0x00000200
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_RESP_IN_ASYNC_FIFO_SHIFT              9
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_DATA_IN_ASYNC_FIFO_MASK               0x00000100
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_DATA_IN_ASYNC_FIFO_SHIFT              8
#define SDHCI_SDH_SLOT_EXT_PRESENT_STATE_LOCK_STATE_MASK                       0x00000001

// Unified Register SDH_SLOT_RETUNING_REQ_CTRL Constants
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_REGISTER_RESET_VALUE 0x00000800
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_RESERVED_0_MASK      0xFFFF00FE
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_RW_MASK              0x0000FF01
//Unified Bitfield Constants Register SDH_SLOT_RETUNING_REQ_CTRL
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_RETUNING_THRESHOLD_MASK               0x0000FF00
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_RETUNING_THRESHOLD_SHIFT              8
#define SDHCI_SDH_SLOT_RETUNING_REQ_CTRL_RETUNING_COMPATIBLE_MASK              0x00000001

// Unified Register SDH_SLOT_EMMC_CTRL Constants
#define SDHCI_SDH_SLOT_EMMC_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_EMMC_CTRL_REGISTER_RESET_MASK  0x06017F73
#define SDHCI_SDH_SLOT_EMMC_CTRL_RESERVED_0_MASK      0xFCFE808C
#define SDHCI_SDH_SLOT_EMMC_CTRL_RW_MASK              0x03017F73
//Unified Bitfield Constants Register SDH_SLOT_EMMC_CTRL
#define SDHCI_SDH_SLOT_EMMC_CTRL_ENABLE_RESP_STROBE_MASK               0x02000000
#define SDHCI_SDH_SLOT_EMMC_CTRL_ENABLE_RESP_STROBE_SHIFT              25
#define SDHCI_SDH_SLOT_EMMC_CTRL_ENABLE_DATA_STROBE_MASK               0x01000000
#define SDHCI_SDH_SLOT_EMMC_CTRL_ENABLE_DATA_STROBE_SHIFT              24
#define SDHCI_SDH_SLOT_EMMC_CTRL_SET_EMMC_RSTN_MASK                    0x00010000
#define SDHCI_SDH_SLOT_EMMC_CTRL_SET_EMMC_RSTN_SHIFT                   16
#define SDHCI_SDH_SLOT_EMMC_CTRL_DISABLE_RD_DATA_CRC_MASK              0x00004000
#define SDHCI_SDH_SLOT_EMMC_CTRL_DISABLE_RD_DATA_CRC_SHIFT             14
#define SDHCI_SDH_SLOT_EMMC_CTRL_DISABLE_CRC_STAT_TOKEN_MASK           0x00002000
#define SDHCI_SDH_SLOT_EMMC_CTRL_DISABLE_CRC_STAT_TOKEN_SHIFT          13
#define SDHCI_SDH_SLOT_EMMC_CTRL_BOOT_DDR_ENABLE_MASK                  0x00001000
#define SDHCI_SDH_SLOT_EMMC_CTRL_BOOT_DDR_ENABLE_SHIFT                 12
#define SDHCI_SDH_SLOT_EMMC_CTRL_BOOT_ACK_ENABLE_MASK                  0x00000800
#define SDHCI_SDH_SLOT_EMMC_CTRL_BOOT_ACK_ENABLE_SHIFT                 11
#define SDHCI_SDH_SLOT_EMMC_CTRL_EXT_ALTER_BOOT_MASK                   0x00000400
#define SDHCI_SDH_SLOT_EMMC_CTRL_EXT_ALTER_BOOT_SHIFT                  10
#define SDHCI_SDH_SLOT_EMMC_CTRL_TERMINATE_DFLT_BOOT_MASK              0x00000200
#define SDHCI_SDH_SLOT_EMMC_CTRL_TERMINATE_DFLT_BOOT_SHIFT             9
#define SDHCI_SDH_SLOT_EMMC_CTRL_EXE_DEFAULT_BOOT_MASK                 0x00000100
#define SDHCI_SDH_SLOT_EMMC_CTRL_EXE_DEFAULT_BOOT_SHIFT                8
#define SDHCI_SDH_SLOT_EMMC_CTRL_EMMC_DRV_STRENGTH_SEL_MASK            0x00000070
#define SDHCI_SDH_SLOT_EMMC_CTRL_EMMC_DRV_STRENGTH_SEL_SHIFT           4
#define SDHCI_SDH_SLOT_EMMC_CTRL_IO_VOLTAGE_SEL_MASK                   0x00000003

// Unified Register SDH_SLOT_FIFO_CTRL Constants
#define SDHCI_SDH_SLOT_FIFO_CTRL_REGISTER_RESET_VALUE 0x00000000
#define SDHCI_SDH_SLOT_FIFO_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_FIFO_CTRL_RESERVED_0_MASK      0xFFFFFE00
#define SDHCI_SDH_SLOT_FIFO_CTRL_RW_MASK              0x000001FF
//Unified Bitfield Constants Register SDH_SLOT_FIFO_CTRL
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PDLVMC_MASK                      0x00000100
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PDLVMC_SHIFT                     8
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PDFVSSM_MASK                     0x00000080
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PDFVSSM_SHIFT                    7
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PWDN_MASK                        0x00000040
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_PWDN_SHIFT                       6
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_CS_MASK                          0x00000020
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_CS_SHIFT                         5
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_CLK_MASK                         0x00000010
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_CLK_SHIFT                        4
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_WTC_MASK                         0x0000000C
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_WTC_SHIFT                        2
#define SDHCI_SDH_SLOT_FIFO_CTRL_FIFO_RTC_MASK                         0x00000003

// Unified Register SDH_SLOT_OP_STATUS_CTRL Constants
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_REGISTER_RESET_VALUE 0x00040000
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_RESERVED_0_MASK      0xFFF80000
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_RW_MASK              0x0007FFFF
//Unified Bitfield Constants Register SDH_SLOT_OP_STATUS_CTRL
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_CONSECUTIVE_TIMES_MASK                0x00070000
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_CONSECUTIVE_TIMES_SHIFT               16
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_TUNING_STEP_MASK                      0x0000F000
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_TUNING_STEP_SHIFT                     12
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_FORCE_SEL_INVERSE_CLK_MASK            0x00000800
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_FORCE_SEL_INVERSE_CLK_SHIFT           11
#define SDHCI_SDH_SLOT_OP_STATUS_CTRL_TUNING_PROG_FIXED_DELAY_MASK          0x000007FF

// Unified Register SDH_SLOT_EXT_ERR_STATUS_EN Constants
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_REGISTER_RESET_MASK  0x0031
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_RW_MASK              0x007F
//Unified Bitfield Constants Register SDH_SLOT_EXT_ERR_STATUS_EN
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_EMMC_BOOT_ACK_ERR_EN_MASK             0x0040
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_EMMC_BOOT_ACK_ERR_EN_SHIFT            6
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_RD_DECERR_EN_MASK           0x0020
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_RD_DECERR_EN_SHIFT          5
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTFI_RD_SLVERR_EN_MASK           0x0010
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTFI_RD_SLVERR_EN_SHIFT          4
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_RD_ICPRESP_EN_MASK          0x0008
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_RD_ICPRESP_EN_SHIFT         3
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_WR_DECERR_EN_MASK           0x0004
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_WR_DECERR_EN_SHIFT          2
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTFI_WR_SLVERR_EN_MASK           0x0002
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTFI_WR_SLVERR_EN_SHIFT          1
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EN_AXI_MSTIF_WR_ICPRESP_EN_MASK          0x0001

// Unified Register SDH_SLOT_EXT_INT_STATUS_EN Constants
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_REGISTER_RESET_MASK  0x0031
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_RW_MASK              0x0031
//Unified Bitfield Constants Register SDH_SLOT_EXT_INT_STATUS_EN
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_UNLOCK_EVENT_EN_MASK                  0x0020
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_UNLOCK_EVENT_EN_SHIFT                 5
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_CLOCK_EVENT_EN_MASK                   0x0010
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_CLOCK_EVENT_EN_SHIFT                  4
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EN_EMMC_BOOT_ACK_RCVD_EN_MASK            0x0001

// Unified Register SDH_SLOT_EXT_ERR_STATUS Constants
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_REGISTER_RESET_VALUE 0x0000
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_REGISTER_RESET_MASK  0x0031
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_STC_MASK             0x007F
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_SET_TO_CLEAR_MASK    0x007F
//Unified Bitfield Constants Register SDH_SLOT_EXT_ERR_STATUS
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EMMC_BOOT_ACK_ERR_MASK                0x0040
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_EMMC_BOOT_ACK_ERR_SHIFT               6
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_RD_DECERR_MASK              0x0020
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_RD_DECERR_SHIFT             5
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTFI_RD_SLVERR_MASK              0x0010
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTFI_RD_SLVERR_SHIFT             4
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_RD_ICPRESP_MASK             0x0008
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_RD_ICPRESP_SHIFT            3
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_WR_DECERR_MASK              0x0004
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_WR_DECERR_SHIFT             2
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTFI_WR_SLVERR_MASK              0x0002
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTFI_WR_SLVERR_SHIFT             1
#define SDHCI_SDH_SLOT_EXT_ERR_STATUS_AXI_MSTIF_WR_ICPRESP_MASK             0x0001

// Unified Register SDH_SLOT_EXT_INT_STATUS Constants
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_REGISTER_RESET_VALUE              0x0000
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_REGISTER_RESET_MASK               0x0031
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_STC_MASK                          0x0031
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_SET_TO_CLEAR_MASK                 0x0031
//Unified Bitfield Constants Register SDH_SLOT_EXT_INT_STATUS
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_UNLOCK_EVENT_MASK                 0x0020
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_UNLOCK_EVENT_SHIFT                5
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_CLOCK_EVENT_MASK                  0x0010
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_CLOCK_EVENT_SHIFT                 4
#define SDHCI_SDH_SLOT_EXT_INT_STATUS_EMMC_BOOT_ACK_RCVD_MASK           0x0001

// Unified Register SDH_TESTOUT_MUXSEL Constants
#define SDHCI_SDH_TESTOUT_MUXSEL_REGISTER_RESET_VALUE               0x00000000
#define SDHCI_SDH_TESTOUT_MUXSEL_REGISTER_RESET_MASK                0xFFFFFFFF
#define SDHCI_SDH_TESTOUT_MUXSEL_RESERVED_0_MASK                    0xFFFFFF00
#define SDHCI_SDH_TESTOUT_MUXSEL_RW_MASK                            0x000000FF
//Unified Bitfield Constants Register SDH_TESTOUT_MUXSEL
#define SDHCI_SDH_TESTOUT_MUXSEL_TEST_MUXSEL_MASK                   0x000000FF

// Unified Register SDH_TEST_OUT Constants
#define SDHCI_SDH_TEST_OUT_REGISTER_RESET_VALUE 0x00B00000
#define SDHCI_SDH_TEST_OUT_REGISTER_RESET_MASK  0xFFFFFFFF
#define SDHCI_SDH_TEST_OUT_RESERVED_0_MASK      0xFFFFFF00
#define SDHCI_SDH_TEST_OUT_RO_MASK              0x000000FF
//Unified Bitfield Constants Register SDH_TEST_OUT
#define SDHCI_SDH_TEST_OUT_TEST_OUTPUT_MASK                         0x000000FF

// Unified Register SDH_SYS_EXT_OP_CTRL Constants
#define SDHCI_SDH_SYS_EXT_OP_CTRL_REGISTER_RESET_VALUE              0x00000000
#define SDHCI_SDH_SYS_EXT_OP_CTRL_REGISTER_RESET_MASK               0xFFFFFFFF
#define SDHCI_SDH_SYS_EXT_OP_CTRL_RESERVED_0_MASK                   0xFFFFFF00
#define SDHCI_SDH_SYS_EXT_OP_CTRL_RW_MASK                           0x000000FF
//Unified Bitfield Constants Register SDH_SYS_EXT_OP_CTRL
#define SDHCI_SDH_SYS_EXT_OP_CTRL_PARALLEL_TRAN_MODE_SLOTS_MASK     0x000000FF

// Unified Register SDH_SYS_OP_CTRL Constants
#define SDHCI_SDH_SYS_OP_CTRL_REGISTER_RESET_VALUE                  0x000F0000
#define SDHCI_SDH_SYS_OP_CTRL_REGISTER_RESET_MASK                   0xFFFFFFFF
#define SDHCI_SDH_SYS_OP_CTRL_RESERVED_0_MASK                       0x00800000
#define SDHCI_SDH_SYS_OP_CTRL_RW_MASK                               0xFF7FFFFF
//Unified Bitfield Constants Register SDH_SYS_OP_CTRL
#define SDHCI_SDH_SYS_OP_CTRL_SLOT_PRIORITY_LEVEL_MASK              0xFF000000
#define SDHCI_SDH_SYS_OP_CTRL_SLOT_PRIORITY_LEVEL_SHIFT             24
#define SDHCI_SDH_SYS_OP_CTRL_DMA_BURST_LEN_MASK                    0x00600000
#define SDHCI_SDH_SYS_OP_CTRL_DMA_BURST_LEN_SHIFT                   21
#define SDHCI_SDH_SYS_OP_CTRL_AUTO_CLKGATE_DISABLE_MASK             0x00100000
#define SDHCI_SDH_SYS_OP_CTRL_AUTO_CLKGATE_DISABLE_SHIFT            20
#define SDHCI_SDH_SYS_OP_CTRL_SDCLK_IDLEOFF_CNTR_MASK               0x000F0000
#define SDHCI_SDH_SYS_OP_CTRL_SDCLK_IDLEOFF_CNTR_SHIFT              16
#define SDHCI_SDH_SYS_OP_CTRL_SDCLK_IDLEOFF_ENABLE_MASK             0x0000FF00
#define SDHCI_SDH_SYS_OP_CTRL_SDCLK_IDLEOFF_ENABLE_SHIFT            8
#define SDHCI_SDH_SYS_OP_CTRL_SLOT_ENABLE_MASK                      0x000000FF

// Unified Register SDH_SYS_CFG_INFO Constants
#define SDHCI_SDH_SYS_CFG_INFO_REGISTER_RESET_VALUE                 0x00010000
#define SDHCI_SDH_SYS_CFG_INFO_REGISTER_RESET_MASK                  0xFFFFFFFF
#define SDHCI_SDH_SYS_CFG_INFO_RW_MASK                              0xFF000000
#define SDHCI_SDH_SYS_CFG_INFO_RO_MASK                              0x00FFFFFF
//Unified Bitfield Constants Register SDH_SYS_CFG_INFO
#define SDHCI_SDH_SYS_CFG_INFO_SDIO_INSERTED_MASK                   0xFF000000
#define SDHCI_SDH_SYS_CFG_INFO_SDIO_INSERTED_SHIFT                  24
#define SDHCI_SDH_SYS_CFG_INFO_SLOT_TYPE_EMMC_MASK                  0x00FF0000
#define SDHCI_SDH_SYS_CFG_INFO_SLOT_TYPE_EMMC_SHIFT                 16
#define SDHCI_SDH_SYS_CFG_INFO_SLOT_TYPE_SD_SDIO_MMC_MASK           0x0000FF00
#define SDHCI_SDH_SYS_CFG_INFO_SLOT_TYPE_SD_SDIO_MMC_SHIFT          8
#define SDHCI_SDH_SYS_CFG_INFO_NUM_SUPPORTED_SLOT_MASK              0x000000FF

// Unified Register SDH_IPID Constants
#define SDHCI_SDH_IPID_REGISTER_RESET_VALUE                         0x12365831
#define SDHCI_SDH_IPID_REGISTER_RESET_MASK                          0xFFFFFFFF
#define SDHCI_SDH_IPID_RO_MASK                                      0xFFFFFFFF
//Unified Bitfield Constants Register SDH_IPID
#define SDHCI_SDH_IPID_SDHC_IPID_IPRELTAG_MASK                      0xFFFFF000
#define SDHCI_SDH_IPID_SDHC_IPID_IPRELTAG_SHIFT                     12
#define SDHCI_SDH_IPID_SDHC_IPID_MAIN_MASK                          0x00000FFF

/////////////////////////////////////
// support functions
/////////////////////////////////////
extern void sdhciAccessLayerRangeError(const char *s, ...);
/*assert for ASRmicro newPHY*/
extern unsigned int sdhciReadSDHC_DLINE_CTRL(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_DLINE_CTRL(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_DLINE_CFG(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_DLINE_CFG(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_CTRL(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_CTRL(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_FUNC(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_FUNC(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_DLLCFG(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_DLLCFG(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_DLLCFG1(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_DLLCFG1(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_DLLSTS(struct mmc_host *pContext);
extern unsigned int sdhciReadSDHC_PHY_DLLSTS1(struct mmc_host *pContext);
extern unsigned int sdhciReadSDHC_PHY_PADCFG(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_PADCFG(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_PADCFG1(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_PADCFG1(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_LBCTRL(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_LBCTRL(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_LBFUNC(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_LBFUNC(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_LBCNT(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_LBCNT(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_PHY_LBSTS(struct mmc_host *pContext);
extern unsigned int sdhciReadSDHC_PHY_LBDATA(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_PHY_LBDATA(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_CQE_CQBDCTRL(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_CQE_CQBDCTRL(struct mmc_host *pContext, unsigned int w32Val);
extern unsigned int sdhciReadSDHC_CQE_CQDINFO(struct mmc_host *pContext);
extern  void sdhciWriteSDHC_CQE_CQDINFO(struct mmc_host *pContext, unsigned int w32Val);

/*assert for ASRmicro sdhc different section from 0x104~0x12c*/
extern  unsigned int sdhciReadSdhc_gpio_cfg_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_gpio_cfg_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_roundtrip_timing_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_roundtrip_timing_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_hwtune_cfg2_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_hwtune_cfg2_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_hwtune_cfg_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_hwtune_cfg_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_tx_cfg_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_tx_cfg_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_rx_cfg_reg(struct mmc_host *pContext);
extern void sdhciWriteSdhc_rx_cfg_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_mmc_ctrl_reg(struct mmc_host *pContext);
extern void sdhciWriteSdhc_mmc_ctrl_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_legacy_ceata_reg(struct mmc_host *pContext);
extern  unsigned int sdhciWriteSdhc_legacy_ceata_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_legacy_ctrl_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_legacy_ctrl_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_op_ext_reg(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_op_ext_reg(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdhc_op_ctrl(struct mmc_host *pContext);
extern  void sdhciWriteSdhc_op_ctrl(struct mmc_host *pContext, unsigned int w32Val);
extern  unsigned int sdhciReadSdh_vid_pid(struct mmc_host *pContext);
extern  unsigned int sdhciReadSdh_slot_fifo_ctrl(struct mmc_host *pContext);

/////////////////////////////////////
//function prototypes for register and bitfield accessors
/////////////////////////////////////
extern       unsigned int   sdhciReadSdh_regs(uintptr_t reg_addr);
extern               void   sdhciWriteSdh_regs(uintptr_t reg_addr, unsigned int w32Val);
extern               void   sdhciWriteSdh_sysaddr(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_sysaddr(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_sysaddrSysaddr(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_sysaddrSysaddr(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_block_size(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_block_size(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_block_sizeHost_dma_bdry(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_block_sizeHost_dma_bdry(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_block_sizeBlock_size(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_block_sizeBlock_size(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_block_count(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_block_count(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_block_countBlock_count(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_block_countBlock_count(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_arg(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_arg(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_argArg(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_argArg(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_mode(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_mode(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_modeMulti_blk_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_modeMulti_blk_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_modeTo_host_dir(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_modeTo_host_dir(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_modeAuto_cmd_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_modeAuto_cmd_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_modeBlk_cnt_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_modeBlk_cnt_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_transfer_modeDma_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_transfer_modeDma_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmd(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmd(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdCmd_index(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdCmd_index(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdCmd_type(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdCmd_type(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdData_present(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdData_present(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdCmd_index_chk_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdCmd_index_chk_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdCmd_crc_chk_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdCmd_crc_chk_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_cmdResp_type(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_cmdResp_type(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_respx(struct mmc_host *pContext,  int iIndex,  unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_respx(struct mmc_host *pContext,  int iIndex);
extern       unsigned int   sdhciReadSdh_respxResp(struct mmc_host *pContext,  int iIndex);
extern               void   sdhciWriteSdh_dp_buffer(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_dp_buffer(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_dp_bufferBfr_data(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_dp_bufferBfr_data(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_present_state_1(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_present_state_1(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Buffer_rd_en(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Buffer_wr_en(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Rx_active(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Tx_active(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Retuning_req(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Dat_active(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Cmd_inhibit_dat(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_1Cmd_inhibit_cmd(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_present_state_2(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_present_state_2(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Cmd_level(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Dat_level(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Write_prot(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Card_detect(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Card_stable(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_present_state_2Card_inserted(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlBus_volt(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlBus_volt(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlBus_power(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlBus_power(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlCard_det_s(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlCard_det_s(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlCard_det_l(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlCard_det_l(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlEx_data_width(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlEx_data_width(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlDma_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlDma_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlHi_speed_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlHi_speed_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlData_width(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlData_width(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrlLed_ctrl(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrlLed_ctrl(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrl(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrl(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlW_removal(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlW_removal(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlW_insertion(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlW_insertion(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlW_card_int(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlW_card_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlInt_blk_gap(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlInt_blk_gap(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlRd_wait_ctl(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlRd_wait_ctl(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlCont_req(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlCont_req(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_blk_gap_ctrlStop_at_block_gap_req(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_blk_gap_ctrlStop_at_block_gap_req(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrl(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrl(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrlFreq_sel_lo(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrlFreq_sel_lo(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrlFreq_sel_hi(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrlFreq_sel_hi(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrlClk_gen_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrlClk_gen_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrlClock_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrlClock_en(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_clock_ctrlInt_clk_stable(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_clock_ctrlInt_clk_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_clock_ctrlInt_clk_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_timeout_ctrl_sw_reset(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_timeout_ctrl_sw_reset(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_dat(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_dat(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_cmd(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_cmd(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_timeout_ctrl_sw_resetSw_rst_all(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_timeout_ctrl_sw_resetSw_rst_all(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_timeout_ctrl_sw_resetTimeout_value(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_timeout_ctrl_sw_resetTimeout_value(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_i_statErr_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statRetuning_int(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statRetuning_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statInt_c(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statInt_c(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statInt_b(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statInt_b(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statInt_a(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statInt_a(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_i_statCard_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statCard_rem_int(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statCard_rem_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statCard_ins_int(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statCard_ins_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statRx_rdy(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statRx_rdy(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statTx_rdy(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statTx_rdy(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statDma_int(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statDma_int(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statBlock_gap_event(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statBlock_gap_event(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statXfer_complete(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statXfer_complete(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_statCmd_complete(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_statCmd_complete(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCrc_status_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCrc_status_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCpl_timeout_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCpl_timeout_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statAxi_resp_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statAxi_resp_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statSpi_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statSpi_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statTune_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statTune_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statAdma_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statAdma_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statAuto_cmd12_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statAuto_cmd12_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCur_limit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCur_limit_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statRd_data_end_bit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statRd_data_end_bit_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statRd_data_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statRd_data_crc_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statData_timeout_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statData_timeout_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCmd_index_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCmd_index_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCmd_end_bit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCmd_end_bit_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCmd_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCmd_crc_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_statCmd_timeout_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_statCmd_timeout_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enRetune_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enRetune_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enInt_c_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enInt_c_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enInt_b_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enInt_b_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enInt_a_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enInt_a_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enCard_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enCard_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enCard_rem_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enCard_rem_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enCard_ins_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enCard_ins_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enRx_rdy_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enRx_rdy_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enTx_rdy_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enTx_rdy_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enDma_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enDma_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enBgapevnt_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enBgapevnt_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enXfrcomp_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enXfrcomp_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_stat_enCmdcomp_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_stat_enCmdcomp_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCrc_stat_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCrc_stat_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCpl_to_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCpl_to_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enAxi_resp_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enAxi_resp_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enSpi_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enSpi_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enTuning_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enTuning_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enAdma_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enAdma_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enAc12_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enAc12_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCurlmterr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCurlmterr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enDataenderr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enDataenderr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enDatacrcerr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enDatacrcerr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enData_toerr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enData_toerr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCmd_idxerr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCmd_idxerr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCmd_enderr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCmd_enderr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCmd_crcerr_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCmd_crcerr_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_stat_enCmd_to_err_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_stat_enCmd_to_err_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enRetune_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enRetune_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enInt_c_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enInt_c_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enInt_b_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enInt_b_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enInt_a_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enInt_a_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enCard_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enCard_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enCard_rem_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enCard_rem_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enCard_ins_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enCard_ins_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enRx_rdy_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enRx_rdy_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enTx_rdy_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enTx_rdy_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enDma_int_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enDma_int_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enBgapevnt_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enBgapevnt_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enXfrcomp_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enXfrcomp_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_i_sign_enCmdcomp_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_i_sign_enCmdcomp_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCrc_stat_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCrc_stat_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCpl_to_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCpl_to_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enAxi_resp_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enAxi_resp_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enSpi_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enSpi_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enTune_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enTune_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enAdma_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enAdma_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enAc12_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enAc12_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCurlmterr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCurlmterr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enDataenderr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enDataenderr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enDatacrcerr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enDatacrcerr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enData_toerr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enData_toerr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCmd_idxerr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCmd_idxerr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCmd_enderr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCmd_enderr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCmd_crcerr_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCmd_crcerr_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_e_sign_enCmd_to_err_sgen(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_e_sign_enCmd_to_err_sgen(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_status(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_status(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusCmd_not_issued(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_index_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_index_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_end_bit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_end_bit_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_crc_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd_timeout_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusAuto_cmd_timeout_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_auto_cmd12_error_statusAuto_cmd12_not_exe(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_auto_cmd12_error_statusAuto_cmd12_not_exe(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Pre_val_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Pre_val_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Async_int_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Async_int_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Sampling_clk_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Sampling_clk_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Exe_tuning(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Exe_tuning(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Drv_strength_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Drv_strength_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Sdh_v18_en(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Sdh_v18_en(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_host_ctrl_2Uhs_mode_sel(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_host_ctrl_2Uhs_mode_sel(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_capabilities_1(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_capabilities_1(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_1Base_freq(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_1Timeout_unit(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_1Timeout_freq(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_capabilities_2(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_capabilities_2(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_capabilities_2Cfg_slot_type(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_capabilities_2Cfg_slot_type(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Async_int_supp(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Sys_bus_64_supp(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Vlg_18_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Vlg_30_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Vlg_33_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Sus_res_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Sdma_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Hi_speed_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Adma1_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Adma2_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Ex_data_width_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_2Max_blk_len(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_capabilities_3(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_capabilities_3(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Retune_modes(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Sdr50_tune(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Tmr_retune(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Drv_type_d(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Drv_type_c(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Drv_type_a(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Ddr50_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Sdr104_support(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_3Sdr50_support(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_capabilities_4(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_capabilities_4(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_capabilities_4Clk_multiplier(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_max_current_1(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_max_current_1(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_max_current_1Max_cur_30(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_max_current_1Max_cur_33(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_max_current_2(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_max_current_2(struct mmc_host *pContext);
extern     unsigned short   sdhciReadSdh_max_current_2Max_cur_18(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_max_current_3(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_max_current_3(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_max_current_4(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_max_current_4(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_force_event_acmd12err(struct mmc_host *pContext,   unsigned short w16Val);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_issue_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_issue_errShadow(void);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_index_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_index_errShadow(void);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_ebit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_ebit_errShadow(void);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_crc_errShadow(void);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_to_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_to_errShadow(void);
extern               void   sdhciWriteSdh_force_event_acmd12errF_acmd12_nexe_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_acmd12errF_acmd12_nexe_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_status(struct mmc_host *pContext,   unsigned short w16Val);
extern               void   sdhciWriteSdh_force_event_error_statusF_crc_status_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_crc_status_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_cpl_timeout_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_cpl_timeout_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_axi_resp_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_axi_resp_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_spi_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_spi_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_adma_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_adma_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_acmd12_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_acmd12_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_current_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_current_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_dat_end_bit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_dat_end_bit_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_dat_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_dat_crc_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_dat_to_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_dat_to_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_cmd_index_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_cmd_index_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_cmd_end_bit_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_cmd_end_bit_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_cmd_crc_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_cmd_crc_errShadow(void);
extern               void   sdhciWriteSdh_force_event_error_statusF_cmd_to_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_force_event_error_statusF_cmd_to_errShadow(void);
extern               void   sdhciWriteSdh_adma_error_status(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_adma_error_status(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_error_statusAdma_len_err(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_adma_error_statusAdma_len_err(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_error_statusAdma_state(struct mmc_host *pContext,   unsigned short w16Val);
extern     unsigned short   sdhciReadSdh_adma_error_statusAdma_state(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_sysaddr0(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_adma_sysaddr0(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_sysaddr0Admasysaddr(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_adma_sysaddr0Admasysaddr(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_sysaddr1(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_adma_sysaddr1(struct mmc_host *pContext);
extern               void   sdhciWriteSdh_adma_sysaddr1Admasysaddr(struct mmc_host *pContext,   unsigned int w32Val);
extern       unsigned int   sdhciReadSdh_adma_sysaddr1Admasysaddr(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_init(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_initDrv_strength_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_initClkgen_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_initClk_freq_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_ds(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_dsDrv_strength_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_dsClkgen_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_dsClk_freq_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_hs(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_hsDrv_strength_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_hsClkgen_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_hsClk_freq_sel_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_sdr12(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_sdr12Drv_strength_val(struct mmc_host *pContext);
extern     unsigned short   sdhciReadPreset_value_for_sdr12Clkgen_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr12Clk_freq_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr25(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr25Drv_strength_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr25Clkgen_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr25Clk_freq_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr50(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr50Drv_strength_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr50Clkgen_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr50Clk_freq_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr104(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr104Drv_strength_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr104Clkgen_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_sdr104Clk_freq_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_ddr50(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_ddr50Drv_strength_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_ddr50Clkgen_sel_val(struct mmc_host *pContext);
extern unsigned short   sdhciReadPreset_value_for_ddr50Clk_freq_sel_val(struct mmc_host *pContext);
extern void   sdhciWriteShared_bus_ctrl(struct mmc_host *pContext,   unsigned int w32Val);
extern unsigned int   sdhciReadShared_bus_ctrl(struct mmc_host *pContext);
extern void   sdhciWriteShared_bus_ctrlBend_pwr_ctrl(struct mmc_host *pContext,   unsigned int w32Val);
extern unsigned int   sdhciReadShared_bus_ctrlBend_pwr_ctrl(struct mmc_host *pContext);
extern void   sdhciWriteShared_bus_ctrlInt_pin_sel(struct mmc_host *pContext,   unsigned int w32Val);
extern unsigned int   sdhciReadShared_bus_ctrlInt_pin_sel(struct mmc_host *pContext);
extern void   sdhciWriteShared_bus_ctrlClk_pin_sel(struct mmc_host *pContext,   unsigned int w32Val);
extern unsigned int   sdhciReadShared_bus_ctrlClk_pin_sel(struct mmc_host *pContext);
extern unsigned int   sdhciReadShared_bus_ctrlBus_width_preset(struct mmc_host *pContext);
extern unsigned int   sdhciReadShared_bus_ctrlNum_int_pins(struct mmc_host *pContext);
extern unsigned int   sdhciReadShared_bus_ctrlNum_clk_pins(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_slot_int_status(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_slot_int_statusSlot_int3(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_slot_int_statusSlot_int2(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_slot_int_statusSlot_int1(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_slot_int_statusSlot_int0(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_ctrl_ver(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_ctrl_verVendor_ver(struct mmc_host *pContext);
extern unsigned short   sdhciReadSdh_ctrl_verHost_ver(struct mmc_host *pContext);

//BES added,2021-12-03
extern unsigned int sdhciReadSdhc_bypass_netlist(struct mmc_host *pContext);
extern void sdhciWriteSdhc_bypass_netlist(struct mmc_host *pContext, unsigned int w32Val);
extern void sdhciWriteSdh_clear_all_regs(struct mmc_host *pContext);
extern unsigned int sdhciReadSdhc_bypass_cfg(struct mmc_host *pContext);
extern void sdhciWriteSdhc_bypass_cfg(struct mmc_host *pContext, unsigned int w32Val);

#if !defined(CHIP_BEST1600)
// reg_00
#define EMMC_DATA_SEL                                       (1 << 0)
#define EMMC_2X_CLK_INV                                     (1 << 1)
#define EMMC_1X_CLK_INV                                     (1 << 2)
#define EMMC_RX_CLK_INV                                     (1 << 3)
#define EMMC_DATA_BYPASS                                    (1 << 4)
#define EMMC_DEBUG_CLR                                      (1 << 5)
#define EMMC_TX_BYPASS                                      (1 << 6)
#define EMMC_RX_BYPASS_DATA                                 (1 << 7)
#define EMMC_RX_BYPASS_CLK                                  (1 << 8)
#define EMMC_RX_BYPASS_CMD                                  (1 << 9)
#define EMMC_1X_CLK_SEL                                     (1 << 10)
#define EMMC_RXIN_CLK_INV                                   (1 << 11)
#define EMMC_RX_BYPASS_CMD_CLK(n)                           (((n) & 0x3) << 12)
#define EMMC_RX_BYPASS_CMD_CLK_MASK                         (0x3 << 12)
#define EMMC_RX_BYPASS_CMD_CLK_SHIFT                        (12)
#define EMMC_RXIN_CMD_CLK_INV                               (1 << 14)
#define EMMC_HS400_DATA0_SEL                                (1 << 15)
#define EMMC_AHB_R_OVERRIDE                                 (1 << 16)
#define EMMC_AHB_W_OVERRIDE                                 (1 << 17)
#define EMMC_AHB_X2H_DECERR_EN                              (1 << 18)
#define EMMC_AHB_X2H_FORCE_INCR                             (1 << 19)
#define EMMC_CLK_EN_BYPASS                                  (1 << 20)
#define EMMC_GLOBAL_SRAM_CLK_ON                             (1 << 21)
#define EMMC_00_RESERVED(n)                                 (((n) & 0x3FF) << 22)
#define EMMC_00_RESERVED_MASK                               (0x3FF << 22)
#define EMMC_00_RESERVED_SHIFT                              (22)

// reg_04
#define EMMC_DEBUG_PORT(n)                                  (((n) & 0xFFFF) << 0)
#define EMMC_DEBUG_PORT_MASK                                (0xFFFF << 0)
#define EMMC_DEBUG_PORT_SHIFT                               (0)
#define EMMC_04_RESERVED(n)                                 (((n) & 0xFFFF) << 16)
#define EMMC_04_RESERVED_MASK                               (0xFFFF << 16)
#define EMMC_04_RESERVED_SHIFT                              (16)

// reg_08
#define EMMC_DEBUG_SEL(n)                                   (((n) & 0x3F) << 0)
#define EMMC_DEBUG_SEL_MASK                                 (0x3F << 0)
#define EMMC_DEBUG_SEL_SHIFT                                (0)
#define EMMC_DEBUG_DATA_CMD(n)                              (((n) & 0x7) << 6)
#define EMMC_DEBUG_DATA_CMD_MASK                            (0x7 << 6)
#define EMMC_DEBUG_DATA_CMD_SHIFT                           (6)
#define EMMC_08_RESERVED(n)                                 (((n) & 0x7FFFFF) << 9)
#define EMMC_08_RESERVED_MASK                               (0x7FFFFF << 9)
#define EMMC_08_RESERVED_SHIFT                              (9)

// reg_0c
#define EMMC_CFG_LOOP_BACK                                  (1 << 0)
#define EMMC_CFG_START_AGAIN                                (1 << 1)
#define EMMC_0C_RESERVED1(n)                                (((n) & 0x3F) << 2)
#define EMMC_0C_RESERVED1_MASK                              (0x3F << 2)
#define EMMC_0C_RESERVED1_SHIFT                             (2)
#define EMMC_CFG_CMD(n)                                     (((n) & 0xFF) << 8)
#define EMMC_CFG_CMD_MASK                                   (0xFF << 8)
#define EMMC_CFG_CMD_SHIFT                                  (8)
#define EMMC_TX_DATA(n)                                     (((n) & 0xFF) << 16)
#define EMMC_TX_DATA_MASK                                   (0xFF << 16)
#define EMMC_TX_DATA_SHIFT                                  (16)
#define EMMC_0C_RESERVED2(n)                                (((n) & 0xFF) << 24)
#define EMMC_0C_RESERVED2_MASK                              (0xFF << 24)
#define EMMC_0C_RESERVED2_SHIFT                             (24)

// reg_10
#define EMMC_RX_CMD(n)                                      (((n) & 0xFF) << 0)
#define EMMC_RX_CMD_MASK                                    (0xFF << 0)
#define EMMC_RX_CMD_SHIFT                                   (0)
#define EMMC_RX_DATA(n)                                     (((n) & 0xFF) << 8)
#define EMMC_RX_DATA_MASK                                   (0xFF << 8)
#define EMMC_RX_DATA_SHIFT                                  (8)
#define EMMC_PHY_DLL_CODE(n)                                (((n) & 0x3F) << 16)
#define EMMC_PHY_DLL_CODE_MASK                              (0x3F << 16)
#define EMMC_PHY_DLL_CODE_SHIFT                             (16)
#define EMMC_PHY_DLL_LK                                     (1 << 22)
#define EMMC_10_RESERVED(n)                                 (((n) & 0x1FF) << 23)
#define EMMC_10_RESERVED_MASK                               (0x1FF << 23)
#define EMMC_10_RESERVED_SHIFT                              (23)

// reg_14
#define EMMC_14_RESERVED(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define EMMC_14_RESERVED_MASK                               (0xFFFFFFFF << 0)
#define EMMC_14_RESERVED_SHIFT                              (0)

// reg_18
#define EMMC_PHY_PCK_REVERSE                                (1 << 0)
#define EMMC_PHY_RD_TUNE_DLY(n)                             (((n) & 0x3F) << 1)
#define EMMC_PHY_RD_TUNE_DLY_MASK                           (0x3F << 1)
#define EMMC_PHY_RD_TUNE_DLY_SHIFT                          (1)
#define EMMC_PHY_LOOP_BACK                                  (1 << 7)
#define EMMC_PHY_CK_SEL(n)                                  (((n) & 0x3) << 8)
#define EMMC_PHY_CK_SEL_MASK                                (0x3 << 8)
#define EMMC_PHY_CK_SEL_SHIFT                               (8)
#define EMMC_PHY_RD_RANGE(n)                                (((n) & 0x3) << 10)
#define EMMC_PHY_RD_RANGE_MASK                              (0x3 << 10)
#define EMMC_PHY_RD_RANGE_SHIFT                             (10)
#define EMMC_PHY_DLL_RANG(n)                                (((n) & 0x3) << 12)
#define EMMC_PHY_DLL_RANG_MASK                              (0x3 << 12)
#define EMMC_PHY_DLL_RANG_SHIFT                             (12)
#define EMMC_PHY_LDO_VRES(n)                                (((n) & 0xF) << 14)
#define EMMC_PHY_LDO_VRES_MASK                              (0xF << 14)
#define EMMC_PHY_LDO_VRES_SHIFT                             (14)
#define EMMC_PHY_PCK_REVERSE2                               (1 << 18)
#define EMMC_PHY_RD_TUNE_DLY2(n)                            (((n) & 0x3F) << 19)
#define EMMC_PHY_RD_TUNE_DLY2_MASK                          (0x3F << 19)
#define EMMC_PHY_RD_TUNE_DLY2_SHIFT                         (19)
#define EMMC_PHY_CK_SEL2(n)                                 (((n) & 0x3) << 25)
#define EMMC_PHY_CK_SEL2_MASK                               (0x3 << 25)
#define EMMC_PHY_CK_SEL2_SHIFT                              (25)
#define EMMC_PHY_RD_RANGE2(n)                               (((n) & 0x3) << 27)
#define EMMC_PHY_RD_RANGE2_MASK                             (0x3 << 27)
#define EMMC_PHY_RD_RANGE2_SHIFT                            (27)
#define EMMC_18_RESERVED(n)                                 (((n) & 0x7) << 29)
#define EMMC_18_RESERVED_MASK                               (0x7 << 29)
#define EMMC_18_RESERVED_SHIFT                              (29)

// reg_1c
#define EMMC_PHY_RPD_REG6(n)                                (((n) & 0xFF) << 0)
#define EMMC_PHY_RPD_REG6_MASK                              (0xFF << 0)
#define EMMC_PHY_RPD_REG6_SHIFT                             (0)
#define EMMC_PHY_RPD_REG5(n)                                (((n) & 0xFF) << 8)
#define EMMC_PHY_RPD_REG5_MASK                              (0xFF << 8)
#define EMMC_PHY_RPD_REG5_SHIFT                             (8)
#define EMMC_PHY_RPD_REG4(n)                                (((n) & 0xFF) << 16)
#define EMMC_PHY_RPD_REG4_MASK                              (0xFF << 16)
#define EMMC_PHY_RPD_REG4_SHIFT                             (16)
#define EMMC_PHY_RPD_REG3(n)                                (((n) & 0xFF) << 24)
#define EMMC_PHY_RPD_REG3_MASK                              (0xFF << 24)
#define EMMC_PHY_RPD_REG3_SHIFT                             (24)

// reg_20
#define EMMC_PHY_RPD_REG2(n)                                (((n) & 0xFF) << 0)
#define EMMC_PHY_RPD_REG2_MASK                              (0xFF << 0)
#define EMMC_PHY_RPD_REG2_SHIFT                             (0)
#define EMMC_PHY_RPD_REG1(n)                                (((n) & 0xFF) << 8)
#define EMMC_PHY_RPD_REG1_MASK                              (0xFF << 8)
#define EMMC_PHY_RPD_REG1_SHIFT                             (8)
#define EMMC_PHY_DLL_REG1(n)                                (((n) & 0xFF) << 16)
#define EMMC_PHY_DLL_REG1_MASK                              (0xFF << 16)
#define EMMC_PHY_DLL_REG1_SHIFT                             (16)
#define EMMC_PHY_DLL_REG2(n)                                (((n) & 0xFF) << 24)
#define EMMC_PHY_DLL_REG2_MASK                              (0xFF << 24)
#define EMMC_PHY_DLL_REG2_SHIFT                             (24)

// reg_24
#define EMMC_PHY_CMD_RX_DLY(n)                              (((n) & 0x1F) << 0)
#define EMMC_PHY_CMD_RX_DLY_MASK                            (0x1F << 0)
#define EMMC_PHY_CMD_RX_DLY_SHIFT                           (0)
#define EMMC_PHY_DS_TX_DLY(n)                               (((n) & 0x1F) << 5)
#define EMMC_PHY_DS_TX_DLY_MASK                             (0x1F << 5)
#define EMMC_PHY_DS_TX_DLY_SHIFT                            (5)
#define EMMC_PHY_DS_RX_DLY(n)                               (((n) & 0x1F) << 10)
#define EMMC_PHY_DS_RX_DLY_MASK                             (0x1F << 10)
#define EMMC_PHY_DS_RX_DLY_SHIFT                            (10)
#define EMMC_PHY_CMD_TX_DLY(n)                              (((n) & 0x1F) << 15)
#define EMMC_PHY_CMD_TX_DLY_MASK                            (0x1F << 15)
#define EMMC_PHY_CMD_TX_DLY_SHIFT                           (15)
#define EMMC_PHY_CK_DLY(n)                                  (((n) & 0x1F) << 20)
#define EMMC_PHY_CK_DLY_MASK                                (0x1F << 20)
#define EMMC_PHY_CK_DLY_SHIFT                               (20)
#define EMMC_24_RESERVED(n)                                 (((n) & 0x7F) << 25)
#define EMMC_24_RESERVED_MASK                               (0x7F << 25)
#define EMMC_24_RESERVED_SHIFT                              (25)

// reg_28
#define EMMC_PHY_DATA_TX_DLY_31_0(n)                        (((n) & 0xFFFFFFFF) << 0)
#define EMMC_PHY_DATA_TX_DLY_31_0_MASK                      (0xFFFFFFFF << 0)
#define EMMC_PHY_DATA_TX_DLY_31_0_SHIFT                     (0)

// reg_2c
#define EMMC_PHY_DATA_TX_DLY_39_32(n)                       (((n) & 0xFF) << 0)
#define EMMC_PHY_DATA_TX_DLY_39_32_MASK                     (0xFF << 0)
#define EMMC_PHY_DATA_TX_DLY_39_32_SHIFT                    (0)
#define EMMC_PHY_REV(n)                                     (((n) & 0xFFFF) << 8)
#define EMMC_PHY_REV_MASK                                   (0xFFFF << 8)
#define EMMC_PHY_REV_SHIFT                                  (8)
#define EMMC_PHY_DATA_RX_DLY_39_32(n)                       (((n) & 0xFF) << 24)
#define EMMC_PHY_DATA_RX_DLY_39_32_MASK                     (0xFF << 24)
#define EMMC_PHY_DATA_RX_DLY_39_32_SHIFT                    (24)

// reg_30
#define EMMC_PHY_DATA_RX_DLY_31_0(n)                        (((n) & 0xFFFFFFFF) << 0)
#define EMMC_PHY_DATA_RX_DLY_31_0_MASK                      (0xFFFFFFFF << 0)
#define EMMC_PHY_DATA_RX_DLY_31_0_SHIFT                     (0)

// reg_34
#define EMMC_PHY_LDO_EN                                     (1 << 0)
#define EMMC_PHY_POWER_UP_SOFT_CNT(n)                       (((n) & 0xF) << 1)
#define EMMC_PHY_POWER_UP_SOFT_CNT_MASK                     (0xF << 1)
#define EMMC_PHY_POWER_UP_SOFT_CNT_SHIFT                    (1)
#define EMMC_PHY_POWER_UP_SOFT_CNT0(n)                      (((n) & 0xF) << 5)
#define EMMC_PHY_POWER_UP_SOFT_CNT0_MASK                    (0xF << 5)
#define EMMC_PHY_POWER_UP_SOFT_CNT0_SHIFT                   (5)
#define EMMC_PHY_POWER_UP_SOFT_CNT1(n)                      (((n) & 0xF) << 9)
#define EMMC_PHY_POWER_UP_SOFT_CNT1_MASK                    (0xF << 9)
#define EMMC_PHY_POWER_UP_SOFT_CNT1_SHIFT                   (9)
#define EMMC_PHY_LDO_PU_DR                                  (1 << 13)
#define EMMC_PHY_LDO_PRECHARGE_DR                           (1 << 14)
#define EMMC_PHY_LDO_PU                                     (1 << 15)
#define EMMC_PHY_LDO_PRECHARGE                              (1 << 16)
#define EMMC_PHY_CK_EN                                      (1 << 17)
#define EMMC_PHY_PHY_EN                                     (1 << 18)
#define EMMC_PHY_CK_EN_DR                                   (1 << 19)
#define EMMC_PHY_PHY_EN_DR                                  (1 << 20)
#define EMMC_PHY_PU_EMMC                                    (1 << 21)
#define EMMC_PHY_PU_EMMC_DR                                 (1 << 22)
#define EMMC_PHY_DLL_REFRESH                                (1 << 23)
#define EMMC_PHY_DLL_REFRESH_DR                             (1 << 24)
#define EMMC_PHY_PU1P8                                      (1 << 25)
#define EMMC_PHY_PU1P8_DR                                   (1 << 26)
#define EMMC_PHY_CORE_PU                                    (1 << 27)
#define EMMC_34_RESERVED(n)                                 (((n) & 0xF) << 28)
#define EMMC_34_RESERVED_MASK                               (0xF << 28)
#define EMMC_34_RESERVED_SHIFT                              (28)

extern void emmcphy_open(struct mmc_host *host);
extern void emmcphy_close(void);
extern void emmcphy_start(int enable);
extern void emmcphy_sleep(void);
extern void emmcphy_wakeup(void);
extern void emmcphy_ddr_mode_enable(void);
extern void emmcphy_ddr_mode_disable(void);
extern void emmcphy_hs_host_mode_enable(void);
extern void emmcphy_hs_host_mode_disable(void);
extern void emmcphy_pin_pull_config(void);
extern void emmcphy_pin_pull_clear_all(void);
extern void emmcphy_pin_pull_down_all(void);
extern void emmcphy_pin_set_io_drv(void);
extern void emmcphy_clk_sel_pck(void);
extern void emmcphy_clk_sel_pck_delay(void);
extern void emmcphy_clk_sel_ds_delay(void);
extern void emmcphy_tuning_correct(void);
extern void emmcphy_pck_reverse(int rev_cmd_en, int rev_data_en);
extern void emmcphy_rd_tune_dly_cali(uint8_t cali_val_cmd, uint8_t cali_val_data);
extern void emmcphy_rd_tune_dly_read(uint8_t *cmd_dly, uint8_t *data_dly);
#endif

#ifdef __cplusplus
}
#endif

#endif
