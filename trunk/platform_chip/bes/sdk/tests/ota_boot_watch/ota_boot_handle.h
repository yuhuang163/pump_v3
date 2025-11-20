/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __OTA_BOOT_HANDLE_H__
#define __OTA_BOOT_HANDLE_H__

#ifdef __cplusplus
    extern "C" {
#endif

#define OTA_COPY_BLOCK_MAGIC            0x5A5A5A5A
#define BOOT_WORD_A                     0xAAAAAAAA
#define BOOT_WORD_B                     0xBBBBBBBB
#define BOOT_WORD_IDLE                  0x49444c45  // IDLE
#define BOOT_WORD_TBCF                  0x54424346  // To Be Confirmed
#define BOOT_WORD_TBCU                  0x54424355  // To Be Confirmed And Updated Backup Area
#define BOOT_WORD_UIBA                  0x55494241  // Updating Backup Area

#ifndef SENS_FLASH_BOOT_STRUCT_ADDR
#define SENS_FLASH_BOOT_STRUCT_ADDR     SENS_FLASH_NC_BASE
#endif

#ifndef BTH_FLASH_BOOT_STRUCT_ADDR
#define BTH_FLASH_BOOT_STRUCT_ADDR      FLASH1_NC_BASE
#endif

typedef enum
{
    PACKED_DIFF = 1,
    PACKED_COMPRESS,
    PACKED_COMPLETE,
} FILE_PACKED_TYPE_E;

typedef struct
{
    uint32_t magicNumber;           // NORMAL_BOOT or COPY_NEW_IMAGE
    uint32_t imageSize;
    uint32_t imageCrc;
    uint32_t newImageFlashOffset;
    uint32_t bootWord;
} FLASH_OTA_BOOT_INFO_T;

void boot2_watch_ota_handler(const FLASH_OTA_BOOT_INFO_T * ctrl);

#ifdef __cplusplus
    }
#endif

#endif

