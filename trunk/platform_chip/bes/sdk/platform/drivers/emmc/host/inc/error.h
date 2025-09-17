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
#ifndef __MMC_ERROR_H__
#define __MMC_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MMC_NO_ERR                          0
#define MMC_INVALID_ERR                     1
#define MMC_MEM_ALLOC_ERR                   2
#define MMC_DATA_VERIFY_ERR                 3
#define MMC_INT_CLK_ERR                     4
#define MMC_CMD_ERR                         5
#define MMC_DATA_XFER_ERR                   6
#define MMC_DETECT_CARD_ERR                 7
#define MMC_CARD_STATUS_ERR                 8
#define MMC_BUS_TEST_ERR                    9
#define MMC_SWITCH_ERR                      10
#define MMC_TUNING_ERROR                    11
#define MMC_PHY_LB_ERR                      12
#define MMC_REG_RW_ERR                      13
#define MMC_OFFSET_ERR                      14
#define MMC_RW_SIZE_ERR                     15  //Read and write over the range
#define MMC_SEND_STOP_ERR                   16  //if send stop fail, you can try again
#define MMC_IN_USE                          17
#define MMC_DEVICE_BUSY                     18
#define MMC_RPMB_KEY_ERR                    19
#define MMC_BLOCK_COUNT_ERR                 20
#define MMC_NOT_SUPPORT                     21
#define MMC_SLEEP_TIMEOUT                   22
#define MMC_AWAKE_TIMEOUT                   23
#define MMC_BUS_IDLE_TIMEOUT                24
#define MMC_ERR_NULL_POINTER                25
#define MMC_ERASE_TIMEOUT                   26

#define MMC_DATA_PATTERN_INCREMENT          0
#define MMC_DATA_PATTERN_DECREMENT          1
#define MMC_DATA_PATTERN_INCREMENT_INVERT   2
#define MMC_DATA_PATTERN_RANDOM             3
#define MMC_DATA_PATTERN_CONSTANT           4
#define MMC_DATA_PATTERN_TUNING_4B          5
#define MMC_DATA_PATTERN_TUNING_8B          6

#ifdef __cplusplus
}
#endif

#endif
