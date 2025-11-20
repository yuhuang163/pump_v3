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
#include "pmu.h"
#include <stdio.h>
#include <string.h>
#include "crc32_c.h"
#include "tool_msg.h"
#include "hal_wdt.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_norflash.h"
#include "hal_location.h"
#include "cmsis_nvic.h"
#include "ota_boot_handle.h"
#include "mbedtls/md5.h"
#include "mbedtls/error.h"
#if defined(OTA_USE_DIFF)
#include "bsdiff.h"
#endif
#if defined(OTA_BIN_COMPRESSED)
#include "heap_api.h"
#include "bes_lzma_api.h"
#endif
#if defined(BOOT_USE_STORAGE_NOFS)
#include "fs/fs_nofs.h"
#include "fs/stat.h"
#endif

#if (RAM_SIZE < 0x80000)
#error "OTA_BIN_COMPRESS need more memory for malloc(may need set CHIP_HAS_CP=0)"
#endif

#ifdef PSRAM_ENABLE
#define OTA_BOOT_RAM                   PSRAM_BSS_LOC
#else
#define OTA_BOOT_RAM
#endif

#define OTA_FLASH_LOGIC_ADDR            (FLASH_NC_BASE)
#define SRC_FLASH_ID                    HAL_FLASH_ID_0
#define DST_FLASH_ID                    HAL_FLASH_ID_0
#define SRC_FLASH_LOGIC_ADDR            OTA_FLASH_LOGIC_ADDR
#define DST_FLASH_LOGIC_ADDR            OTA_FLASH_LOGIC_ADDR
#define EMMC_BASE_ADDR_MASK             0xFF00000F
#define EMMC_BASE_ADDR                  0x00000003

#ifndef APP_RESTORE_FLASH_ADDR
#define APP_RESTORE_FLASH_ADDR          (FLASH_NC_BASE+0x30000)         //adjust according to the actual situation
#endif

#ifndef APP_RESTORE_EMMC_ADDR
#define APP_RESTORE_EMMC_ADDR           (128*0x100000)                  //adjust according to the actual situation
#endif

#ifndef MAX_RESOURCE_COUNT
#define MAX_RESOURCE_COUNT              (0xFF) // adjust according to the actual situation
#endif

#ifndef FLASH_SECTOR_SIZE_IN_BYTES
#define FLASH_SECTOR_SIZE_IN_BYTES      (4096)
#endif
#ifdef FLASH_DUAL_CHIP
#define BUFFER_SIZE_FOR_BURNING         (FLASH_SECTOR_SIZE_IN_BYTES*2)
#define DUAL_CHIP_BUFFER_SIZE           (128*1024)
#else
#define BUFFER_SIZE_FOR_BURNING         FLASH_SECTOR_SIZE_IN_BYTES
#endif

#ifdef DECODE_BREAKPOINT_OFFSET
#define DECODE_BREAKPOINT_ADDR          (SRC_FLASH_LOGIC_ADDR+DECODE_BREAKPOINT_OFFSET)
#else
#define DECODE_BREAKPOINT_ADDR          (SRC_FLASH_LOGIC_ADDR+OTA_BOOT_INFO_OFFSET+BUFFER_SIZE_FOR_BURNING*2)
#endif
#define LEN_OF_CODED_BLOCK              (64*1024)
#define LEN_OF_VAILD_CHECK              (4*64*1024)
#define LEN_OF_CODED_FILE_HEADER        (4*1024)
#define SIZEOF_MD5                      (16)
#define SIZEOF_CRC                      (4)
#define FEED_DOG_FREQ                   (5)
#define MAX_RETRY_TIMES                 (3)
#define CUSTOM_ADDR_TYPE                (10)

#define MOUNT_DEV                       "/dev/emmc0_2"
#define MOUNT_TRAGET                    "/data/emmc0_2"
#define MOUNT_DEV_OS                    "/dev/emmc0_1"
#define MOUNT_OS_FILE                   "/data/emmc0_1"

#ifndef ssize_t
#define ssize_t                         int
#endif
#ifndef O_RDWR
#define O_RDWR                          (02)
#endif

#define PACKED_TOOL_VER_MAJOR           0x00010200  // 1.2.0
#define PACKED_TOOL_VER_LATEST          0x00010201  // 1.2.1

typedef enum
{
    USE_LFS_MODE,
    USE_EMMC_MODE,
    USE_FLASH_MODE,
} OTA_NV_MODE_E;

typedef enum {
    DECODE_IDLE = 0,
    DECODE_NEXT_FILE = 1,
    DECODE_RESTORING = 2,
} DECODE_STATE_E;

typedef struct {
    uint32_t magicCode;
    uint32_t dataCrc;
    uint32_t resourceCount;
    uint32_t totalLength;               // length of file data exclude OTA_MULTI_FILE_HEAD_T
    uint32_t toolVersion;               // ver >= 0x00010200(1.2.0)
    uint32_t packedType;                // 0x01->diff; 0x02->compress; 0x03->complete
    uint8_t  dataMd5[SIZEOF_MD5];
    uint8_t  reserved[24];
} __attribute__((packed)) OTA_MULTI_FILE_HEAD_T;

typedef struct {
    uint32_t type;
    uint32_t resourceCrc;
    uint32_t resourceOffset;
    uint32_t resourceLength;
    uint32_t destAddr;
} __attribute__((packed)) OTA_FILE_INFO_T;

typedef struct {
    uint32_t magicCode;
    uint32_t oldAppFileLen;
    uint32_t newAppFileLen;
    uint32_t newAppFileCrc;
    uint32_t compressDataCrc;
    uint32_t compressDataLen;
    uint32_t compressBlockLen;
    uint32_t oldAppFileCrc;
    uint8_t  reserved[20];
} __attribute__((packed)) __COMP_FILE_HEADER_T;

typedef struct {
    uint8_t Props[5];
    uint32_t orgCompressLen;
    uint32_t compressDataLen;
} __attribute__((packed)) __COMP_FILE_BLOCK_T;

typedef struct {
    uint16_t decodeStatus;
    uint32_t fileInfoOffset;
    uint32_t curCompFileIndex;
    uint32_t compFileOffset;
    uint32_t compFileBlockNum;
    uint32_t oldFileOffset;
    uint32_t newFileBlockLen;
    uint32_t newFileCrc;
    uint32_t newFileCrcLen;
    uint32_t newFileBlockCrc;   // unused
    uint16_t blockStatus;
    uint32_t crc32;
}__attribute__((packed)) OTA_TASK_CONFIG_T;

typedef int FD;

typedef struct {
    FD newFp;
    FD mmcfsFp;
    uint32_t totalLen;
    uint32_t packedType;
}__attribute__((packed)) OTA_ENV_T;

const FLASH_OTA_BOOT_INFO_T * bootinfoCtrl = NULL;

#ifdef  FLASH_DUAL_CHIP
OTA_BOOT_RAM static uint8_t flashBuf[DUAL_CHIP_BUFFER_SIZE];
#endif

OTA_BOOT_RAM static uint8_t appData[LEN_OF_CODED_BLOCK];
#ifdef OTA_USE_DIFF
OTA_BOOT_RAM static uint8_t newappdata[LEN_OF_CODED_BLOCK];
#endif

OTA_ENV_T otaEnv = {0};

// #ifndef BOOT_USE_STORAGE_NOFS
// WEAK int hal_aos_nofs_open(const char *path, int oflags, ...) {return 0;}
// WEAK int hal_aos_nofs_close(int fd) {return 0;}
// WEAK ssize_t hal_aos_nofs_write(int fd, int offset, void *buf, size_t nbytes) {return 0;}
// WEAK ssize_t hal_aos_nofs_read(int fd, int offset, void *buf, size_t nbytes) {return 0;}
// WEAK int stat(FAR const char *path, FAR struct stat *buf); {return 0;}
// struct stat {
//     uint32_t st_size;
// }
// #endif

void boot_norflash_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len)
{
    // TRACE_IMM(0,"[%s]LINE:%d %d %x %x",__func__, __LINE__, id, start_address, len);
    uint32_t erase_len = len;
    #ifdef FLASH_DUAL_CHIP
    if (len % BUFFER_SIZE_FOR_BURNING)
    {
        erase_len = ((len / BUFFER_SIZE_FOR_BURNING) + 1) * BUFFER_SIZE_FOR_BURNING;
    }
    if ((start_address%BUFFER_SIZE_FOR_BURNING) && HAL_FLASH_ID_0 == id)
    {
        ASSERT(0,"Error Erase, stadt addr:%x", start_address);
    }
    #endif

    uint32_t lock;
    lock = int_lock();
    pmu_flash_write_config();
    hal_norflash_erase(id, start_address, erase_len);
    pmu_flash_read_config();
    int_unlock(lock);
}

void boot_norflash_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len)
{
    // TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x",__func__, __LINE__, id, start_address, buffer, len);
    uint32_t lock;
    enum HAL_NORFLASH_RET_T ret = 0;
    #ifndef FLASH_DUAL_CHIP
    lock = int_lock();
    pmu_flash_write_config();
    ret = hal_norflash_write(id, start_address, buffer, len);
    pmu_flash_read_config();
    int_unlock(lock);
    if (ret)
    {
        TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x ret:%d",__func__, __LINE__, id, start_address, buffer, len, ret);
    }

    #else
    if ((start_address & 0x01 || len & 0x01) && HAL_FLASH_ID_0 == id)
    {
        uint32_t offset = 0, read_len = 0, write_len = 0, new_start_addr = 0;

        offset = start_address % BUFFER_SIZE_FOR_BURNING;
        // BUFFER_SIZE_FOR_BURNING alignment
        new_start_addr = start_address - offset;

        // read previous data
        read_len = (offset & 0x01) ? (offset + 1) : offset;
        memcpy(flashBuf, (void*)new_start_addr, read_len);

        // splice
        memcpy(flashBuf+offset, buffer, len);

        // write
        write_len = ((offset + len) & 0x01) ? (offset + len + 1) : (offset + len);
        lock = int_lock();
        pmu_flash_write_config();
        hal_norflash_erase(id, new_start_addr, BUFFER_SIZE_FOR_BURNING);
        ret = hal_norflash_write(id, new_start_addr, flashBuf, write_len);
        pmu_flash_read_config();
        int_unlock(lock);
        if (ret)
        {
            TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x ret:%d",__func__, __LINE__, id, new_start_addr, flashBuf, write_len, ret);
        }
    }
    else
    {
        lock = int_lock();
        pmu_flash_write_config();
        ret = hal_norflash_write(id, start_address, buffer, len);
        pmu_flash_read_config();
        int_unlock(lock);
        if (ret)
        {
            TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x ret:%d",__func__, __LINE__, id, start_address, buffer, len, ret);
        }
    }
    #endif
}

void boot_norflash_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len)
{
    // TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x",__func__, __LINE__, id, start_address, buffer, len);

    if (HAL_FLASH_ID_0 != id)
    {
        hal_norflash_read(id, start_address, buffer, len);
        return;
    }
    
    #ifndef FLASH_DUAL_CHIP
    memcpy(buffer, (void*)start_address, len);
    #else
    if ((start_address & 0x01 || len & 0x01) && HAL_FLASH_ID_0 == id)
    {
        uint32_t offset = 0, read_len = 0, new_start_addr = 0;

        offset = start_address % BUFFER_SIZE_FOR_BURNING;
        // BUFFER_SIZE_FOR_BURNING alignment
        new_start_addr = start_address - offset;

        // read previous data
        read_len = ((offset + len) & 0x01) ? (offset + len + 1) : (offset + len);
        if (read_len > sizeof(flashBuf))
        {
            ASSERT(0,"Read len is too long, %x > %x", read_len, sizeof(flashBuf));
        }
        memcpy(flashBuf, (void*)new_start_addr, read_len);

        // distribute
        TRACE_IMM(0,"[%s]LINE:%d %d %x %p %x",__func__, __LINE__, id, start_address, buffer, len);
        memcpy(buffer, &flashBuf[offset], len);
    }
    else
    {
        memcpy(buffer, (void*)start_address, len);
    }
    #endif
}


void _watchdog_ping(void)
{
#ifdef __WATCHER_DOG_RESET__
    static uint8_t WdPingCount = 0;

    if (++WdPingCount > FEED_DOG_FREQ)
    {
        hal_wdt_ping(HAL_WDT_ID_0);
        #ifndef CHIP_BEST2000
            pmu_wdt_feed();
        #endif
        WdPingCount = 0;
    }
#endif
}

OTA_NV_MODE_E ota_get_nv_mode(void)
{
#if defined(BOOT_USE_STORAGE_NOFS)
    return USE_EMMC_MODE;
#endif
    return USE_FLASH_MODE;
}

unsigned long crc_update(unsigned long crc, const unsigned char *buf, unsigned int len, bool firstBlock)
{

    if (firstBlock) {
        uint32_t magicNum = 0xFFFFFFFF;
        unsigned long crcTmp = 0;
        crcTmp = crc32_c(crc, (uint8_t *)&magicNum, sizeof(magicNum));
        return crc32_c(crcTmp, (uint8_t *)buf+sizeof(magicNum), len-sizeof(magicNum));
    } else {
        return crc32_c(crc, (uint8_t *)buf, len);
    }

}

static void update_boot_info(FLASH_OTA_BOOT_INFO_T* otaBootInfo)
{
    uint32_t addr = (uint32_t)bootinfoCtrl;

    boot_norflash_erase(DST_FLASH_ID, addr, BUFFER_SIZE_FOR_BURNING);
    boot_norflash_write(DST_FLASH_ID, addr, (uint8_t*)(otaBootInfo), sizeof(FLASH_OTA_BOOT_INFO_T));

    //test
    FLASH_OTA_BOOT_INFO_T tmp = {0};
    boot_norflash_read(DST_FLASH_ID, addr, (uint8_t*)(&tmp), sizeof(FLASH_OTA_BOOT_INFO_T));
    TRACE_IMM(0,"[%s] %x %x %x", __func__, DST_FLASH_LOGIC_ADDR, addr, otaBootInfo->magicNumber);
    DUMP8("%02x ", (uint8_t*)(&tmp), sizeof(FLASH_OTA_BOOT_INFO_T));
}

static void ota_read_coded_data(uint32_t offset, uint8_t *buf, uint32_t len)
{
    switch (ota_get_nv_mode())
    {
    case USE_LFS_MODE:
        break;
#ifdef BOOT_USE_STORAGE_NOFS
    case USE_EMMC_MODE:
        hal_aos_nofs_read(otaEnv.newFp, offset, buf, len);
        break;
#endif
    case USE_FLASH_MODE:
        boot_norflash_read(SRC_FLASH_ID, SRC_FLASH_LOGIC_ADDR + offset, buf, len);
        break;
    default:
        break;
    }
}

static void ota_write_coded_data(uint32_t offset, uint8_t *buf, uint32_t len)
{
    switch (ota_get_nv_mode())
    {
    case USE_LFS_MODE:
        break;
#ifdef BOOT_USE_STORAGE_NOFS
    case USE_EMMC_MODE:
        hal_aos_nofs_write(otaEnv.newFp, offset, buf, len);
        break;
#endif
    case USE_FLASH_MODE:
        boot_norflash_write(SRC_FLASH_ID, SRC_FLASH_LOGIC_ADDR + offset, buf, len);
        break;
    default:
        break;
    }
}

static OTA_TASK_CONFIG_T ota_decode_breakpoint_get(void)
{
    OTA_TASK_CONFIG_T otaConfig = {0};
    uint32_t currentCrcValue;
    uint32_t otaConfigLen = sizeof(OTA_TASK_CONFIG_T);

    boot_norflash_read(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR,
                        (uint8_t*)&otaConfig, otaConfigLen);
    currentCrcValue = crc_update(0, (uint8_t*)&otaConfig, otaConfigLen - 4, false);
    if(currentCrcValue == otaConfig.crc32)
    {
        TRACE_IMM(0, "#######ota_configuse  1");
        return otaConfig;
    }

    boot_norflash_read(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR+BUFFER_SIZE_FOR_BURNING,
                        (uint8_t*)&otaConfig, otaConfigLen);
    currentCrcValue = crc_update(0, (uint8_t*)&otaConfig, otaConfigLen - 4, false);
    if(currentCrcValue == otaConfig.crc32)
    {
        TRACE_IMM(0, "#######otaConfig use 2");
        return otaConfig;
    }
    //test
    // DUMP8("%x ", (uint8_t*)&otaConfig, otaConfigLen);
    // TRACE_IMM(0, "currentCrcValue:%x != otaConfig.crc32:%x", currentCrcValue, otaConfig.crc32);
    // TRACE_IMM(0, "#######otaConfig is null");
    memset((void*)&otaConfig, 0, otaConfigLen);
    return otaConfig;
}

static void ota_decode_breakpoint_set(OTA_TASK_CONFIG_T *otaConfig)
{
    uint32_t otaConfigLen = sizeof(OTA_TASK_CONFIG_T);
    otaConfig->crc32 = (uint32_t)crc_update(0, (uint8_t*)otaConfig, otaConfigLen-sizeof(otaConfig->crc32), false);

    boot_norflash_erase(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR, BUFFER_SIZE_FOR_BURNING);
    boot_norflash_write(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR, (uint8_t*)otaConfig, otaConfigLen);
    TRACE_IMM(0, "#######otaConfig set 1");
    boot_norflash_erase(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR+BUFFER_SIZE_FOR_BURNING, BUFFER_SIZE_FOR_BURNING);
    boot_norflash_write(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR+BUFFER_SIZE_FOR_BURNING, (uint8_t*)otaConfig, otaConfigLen);
    TRACE_IMM(0, "#######otaConfig set 2");

    // //test
    // OTA_TASK_CONFIG_T tmp = {0};
    // boot_norflash_read(DST_FLASH_ID, DECODE_BREAKPOINT_ADDR, (uint8_t*)(&tmp), sizeof(OTA_TASK_CONFIG_T));
    // TRACE_IMM(0,"[%s] %x %x %x", __func__, DST_FLASH_LOGIC_ADDR, DECODE_BREAKPOINT_ADDR, otaConfig->crc32);
    // DUMP8("%02x ", (uint8_t*)(&tmp), sizeof(OTA_TASK_CONFIG_T));
}

static uint32_t ota_backup_area_addr()
{
#ifdef OTA_BACKUP_ADDR
    return OTA_BACKUP_ADDR;
#else
#if defined(BOOT_USE_STORAGE_NOFS)
    struct stat result;
#endif
#if defined(BOOT_USE_STORAGE_NOFS)
    int ret = 0;
#endif
    switch (ota_get_nv_mode())
    {
    case USE_LFS_MODE:
        break;
#if defined(BOOT_USE_STORAGE_NOFS)
    case USE_EMMC_MODE:
        ret = stat(MOUNT_DEV, &result);
        if (!ret) {
            TRACE_IMM(0, "result=%d ret:%d", (uint32_t)result.st_size, ret);
            return (uint32_t)(result.st_size/2);
        }
        break;
#endif
    case USE_FLASH_MODE:
        return hal_norflash_get_flash_total_size(SRC_FLASH_ID)/2;
        break;
    default:
        break;
    }
    return (1024*1024*1024/2);  // 0.5G
#endif
}

static void ota_backup_area_config()
{
    OTA_TASK_CONFIG_T otaConfig = {0};
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    OTA_MULTI_FILE_HEAD_T multiFileHead;
    uint32_t magicNum = BOOT_MAGIC_NUMBER, cleanCrc = 0;

    // get backup area size & crc
    ota_read_coded_data(ota_backup_area_addr(),
                    (uint8_t*)&(multiFileHead),
                    sizeof(OTA_MULTI_FILE_HEAD_T));
    DUMP8("%02x ", (uint8_t*)&(multiFileHead), sizeof(OTA_MULTI_FILE_HEAD_T));

    boot_norflash_read(DST_FLASH_ID, (uint32_t)bootinfoCtrl, (uint8_t *)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));
    otaBootInfo.magicNumber = OTA_COPY_BLOCK_MAGIC;
    otaBootInfo.imageCrc = multiFileHead.dataCrc;
    otaBootInfo.imageSize = multiFileHead.totalLength+sizeof(OTA_MULTI_FILE_HEAD_T);
    otaBootInfo.newImageFlashOffset = ota_backup_area_addr();
    otaBootInfo.bootWord = BOOT_WORD_IDLE;
    update_boot_info(&otaBootInfo);

    // make the backup area effective
    ota_write_coded_data(ota_backup_area_addr(),
                        (uint8_t*)&magicNum, sizeof(magicNum));
    ota_write_coded_data(ota_backup_area_addr()+OFFSETOF(OTA_MULTI_FILE_HEAD_T, dataCrc),
                        (uint8_t*)&cleanCrc, sizeof(cleanCrc));
    ota_read_coded_data(ota_backup_area_addr(),
                    (uint8_t*)&(multiFileHead),
                    sizeof(OTA_MULTI_FILE_HEAD_T));
    DUMP8("%02x ", (uint8_t*)&(multiFileHead), sizeof(OTA_MULTI_FILE_HEAD_T));

    ota_decode_breakpoint_set(&otaConfig);
}

static void ota_backup_area_update_prepare()
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    OTA_MULTI_FILE_HEAD_T multiFileHead;

    boot_norflash_read(DST_FLASH_ID, (uint32_t)bootinfoCtrl, (uint8_t *)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));
    ota_read_coded_data(otaBootInfo.newImageFlashOffset,
                    (uint8_t*)&(multiFileHead),
                    sizeof(OTA_MULTI_FILE_HEAD_T));
    DUMP8("%02x ", (uint8_t*)&(multiFileHead), sizeof(OTA_MULTI_FILE_HEAD_T));
    // make sure the file contains a checksum value
    if (!multiFileHead.dataCrc)
    {
        ota_write_coded_data(otaBootInfo.newImageFlashOffset+OFFSETOF(OTA_MULTI_FILE_HEAD_T, dataCrc),
                            (uint8_t*)&otaBootInfo.imageCrc, sizeof(otaBootInfo.imageCrc));
    }

    if (PACKED_COMPRESS == multiFileHead.packedType && MAX_RESOURCE_COUNT <= multiFileHead.resourceCount)
    {
        otaBootInfo.bootWord = BOOT_WORD_TBCU;
        otaBootInfo.imageSize = multiFileHead.totalLength+sizeof(OTA_MULTI_FILE_HEAD_T);
    }
    else
    {
        otaBootInfo.bootWord = BOOT_WORD_TBCF;
    }
    update_boot_info(&otaBootInfo);
}

#if defined(BOOT_USE_DISPLAY)
static void ota_total_len_calculate(uint32_t startAddr,uint32_t count)
{
    uint32_t fileInfoOffset = sizeof(OTA_MULTI_FILE_HEAD_T), curCompFileIndex = 0;
    OTA_FILE_INFO_T fileInfo = {0};
    for(; curCompFileIndex < count; curCompFileIndex++)
    {
        //get fileInfo;
        ota_read_coded_data(startAddr+fileInfoOffset,
                        (uint8_t*)&(fileInfo),
                        sizeof(OTA_FILE_INFO_T));
        fileInfoOffset += sizeof(OTA_FILE_INFO_T);
        otaEnv.totalLen += fileInfo.resourceLength;
        TRACE_IMM(0,"[%s] %d %d %d", __func__, curCompFileIndex, fileInfo.resourceLength, otaEnv.totalLen);
    }
}

static uint32_t ota_total_len_get(void)
{
    return otaEnv.totalLen;
}

static void ota_progress_update(uint32_t curOffset)
{
    uint32_t len = 0, percent = 0;

    len = ota_total_len_get();

    percent = (curOffset-LEN_OF_CODED_FILE_HEADER) *100/len;
    TRACE_IMM(0,"[%s] percent =  %d*100/%d = %d", __func__, (curOffset-LEN_OF_CODED_FILE_HEADER), len, percent);
    if (percent > 100)
    {
        percent = 100;
    }

    show_ota_progress(percent, true);
}
#endif

static uint32_t ota_check_whole_image_crc(uint32_t srcFlashOffset, uint32_t imageSize)
{
    uint32_t verifiedDataSize = 0;
    uint32_t crc32Value = 0;
    uint32_t verifiedBytes = 0;
    uint8_t *srcBuf = appData;   //reuse buffer

    if (srcBuf == NULL) {
        crc32Value = 0;
        return crc32Value;
    }

    while (verifiedDataSize < imageSize) {
        if (imageSize - verifiedDataSize > BUFFER_SIZE_FOR_BURNING){
            verifiedBytes = BUFFER_SIZE_FOR_BURNING;
        } else {
            verifiedBytes = imageSize - verifiedDataSize;
        }

        ota_read_coded_data(srcFlashOffset+verifiedDataSize, srcBuf, BUFFER_SIZE_FOR_BURNING);

        if (0 == verifiedDataSize) {
            if (*(uint32_t *)srcBuf != BOOT_MAGIC_NUMBER) {
                TRACE_IMM(0,"Wrong magic number =0x%x srcFlashOffset:%x",
                    *(uint32_t *)srcBuf, srcFlashOffset);
                DUMP8("%02x ", srcBuf, 60);
                crc32Value = 0;
                return crc32Value;
            } else {
                *(uint32_t *)srcBuf  = 0xFFFFFFFF;
            }
        }
        crc32Value = crc32_c(crc32Value, srcBuf, verifiedBytes);
        verifiedDataSize += verifiedBytes;
    }
    return crc32Value;
}

static bool ota_check_whole_image_md5(uint32_t srcFlashOffset, uint32_t imageSize)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    OTA_MULTI_FILE_HEAD_T multiFileHead = {0};
    mbedtls_md5_context ctx;
    uint32_t verifiedDataSize = 0;
    uint32_t verifiedBytes = 0;
    uint8_t dataMd5[SIZEOF_MD5] = {0};
    uint8_t zoreMd5[SIZEOF_MD5] = {0};
    uint8_t *srcBuf = appData;   //reuse buffer

    if (srcBuf == NULL) {
        return false;
    }

    //get multiFileHead
    ota_read_coded_data(srcFlashOffset,
                    (uint8_t*)&(multiFileHead),
                    sizeof(OTA_MULTI_FILE_HEAD_T));

    // pc tool not support
    if (!memcmp(zoreMd5, multiFileHead.dataMd5, SIZEOF_MD5))
    {
        return true;
    }

    mbedtls_md5_init(&ctx);

    if ((ret = mbedtls_md5_starts(&ctx)) != 0) {
        goto exit;
    }

    while (verifiedDataSize < imageSize) {
        if (imageSize - verifiedDataSize > BUFFER_SIZE_FOR_BURNING){
            verifiedBytes = BUFFER_SIZE_FOR_BURNING;
        } else {
            verifiedBytes = imageSize - verifiedDataSize;
        }

        ota_read_coded_data(srcFlashOffset+verifiedDataSize, srcBuf, BUFFER_SIZE_FOR_BURNING);

        if (0 == verifiedDataSize) {
            if (*(uint32_t *)srcBuf != BOOT_MAGIC_NUMBER) {
                TRACE_IMM(0,"Wrong magic number =0x%x srcFlashOffset:%x",
                    *(uint32_t *)srcBuf, srcFlashOffset);
                DUMP8("%02x ", srcBuf, 60);
                goto exit;
            } else {
                memset(srcBuf+OFFSETOF(OTA_MULTI_FILE_HEAD_T, magicCode), 0xFF, sizeof(multiFileHead.magicCode));
                memset(srcBuf+OFFSETOF(OTA_MULTI_FILE_HEAD_T, dataCrc), 0x00, sizeof(multiFileHead.dataCrc));
                memset(srcBuf+OFFSETOF(OTA_MULTI_FILE_HEAD_T, dataMd5), 0x00, sizeof(multiFileHead.dataMd5));
            }
        }

        if ((ret = mbedtls_md5_update(&ctx, srcBuf, verifiedBytes)) != 0) {
            goto exit;
        }
        verifiedDataSize += verifiedBytes;
    }

    if ((ret = mbedtls_md5_finish(&ctx, dataMd5)) != 0) {
        goto exit;
    }

exit:
    mbedtls_md5_free(&ctx);

    if (!memcmp(dataMd5, multiFileHead.dataMd5, SIZEOF_MD5))
    {
        return true;
    }

    return false;
}

static void ota_open_type_file(uint32_t startAddr)
{
    if (USE_FLASH_MODE == ota_get_nv_mode())  return;
#ifdef BOOT_USE_STORAGE_NOFS
    if (EMMC_BASE_ADDR == (startAddr&EMMC_BASE_ADDR_MASK) && \
        USE_EMMC_MODE==ota_get_nv_mode()) //emmc
    {
        //open old mmcfs file
        otaEnv.mmcfsFp = hal_aos_nofs_open(MOUNT_OS_FILE, O_RDWR);
        ASSERT(otaEnv.mmcfsFp, "open old file failed");
    }
#endif
}

static void ota_close_type_file(uint32_t startAddr)
{
    if (USE_FLASH_MODE == ota_get_nv_mode())  return;
#if defined(BOOT_USE_STORAGE_NOFS)
    if (EMMC_BASE_ADDR == (startAddr&EMMC_BASE_ADDR_MASK) && \
        USE_EMMC_MODE==ota_get_nv_mode()) //emmc
    {
        //open old mmcfs file
        hal_aos_nofs_close(otaEnv.mmcfsFp);
    }
#endif
}

static void ota_read_old_data(uint32_t startAddr, uint32_t offset, uint8_t *buf, uint32_t len)
{
    TRACE_IMM(0,"[%s]LINE:%d start addr:%x offset:%x len:%x", __func__, __LINE__, startAddr, offset, len);

    if (HAL_FLASH_ID_NUM != hal_norflash_addr_to_id(startAddr))
    {
        boot_norflash_read(hal_norflash_addr_to_id(startAddr), startAddr+offset, buf, len);
    }
#ifdef BOOT_USE_STORAGE_NOFS
    else if (EMMC_BASE_ADDR == (startAddr&EMMC_BASE_ADDR_MASK) && \
            USE_EMMC_MODE==ota_get_nv_mode()) //emmc
    {
        hal_aos_nofs_read(otaEnv.mmcfsFp, offset, buf, len);
    }
#endif
}

static void ota_cover_old_data(uint32_t startAddr, uint32_t offset, uint8_t *buf, uint32_t len)
{
    TRACE_IMM(0,"[%s]LINE:%d start addr:%x offset:%x len:%x", __func__, __LINE__, startAddr, offset, len);

    if (HAL_FLASH_ID_NUM != hal_norflash_addr_to_id(startAddr))
    {
        boot_norflash_erase(hal_norflash_addr_to_id(startAddr), startAddr+offset, len);
        boot_norflash_write(hal_norflash_addr_to_id(startAddr), startAddr+offset, buf, len);
    }
#ifdef BOOT_USE_STORAGE_NOFS
    else if (EMMC_BASE_ADDR == (startAddr&EMMC_BASE_ADDR_MASK) && \
            USE_EMMC_MODE==ota_get_nv_mode()) //emmc
    {
        hal_aos_nofs_write(otaEnv.mmcfsFp, offset, buf, len);
    }
#endif
}

#if defined(OTA_USE_DIFF)
static void d_write_tmp_data(uint8_t *buf, uint32_t len)
{
#ifdef BOOT_USE_STORAGE_NOFS
    FD file = otaEnv.newFp;
#endif
    TRACE_IMM(0,"[%s]LINE:%d len:%d", __func__, __LINE__, len);

    if (USE_EMMC_MODE == ota_get_nv_mode()) {
#ifdef BOOT_USE_STORAGE_NOFS
        hal_aos_nofs_write(file, APP_RESTORE_EMMC_ADDR, buf, len);
#endif
    } else {
        boot_norflash_erase(HAL_FLASH_ID_0, APP_RESTORE_FLASH_ADDR, len);
        boot_norflash_write(HAL_FLASH_ID_0, APP_RESTORE_FLASH_ADDR, buf, len);
    }
}

static void d_read_tmp_data(uint8_t *buf, uint32_t len)
{
#ifdef BOOT_USE_STORAGE_NOFS
    FD file = otaEnv.newFp;
#endif
    TRACE_IMM(0,"[%s]LINE:%d len:%d", __func__, __LINE__, len);

    if (USE_EMMC_MODE == ota_get_nv_mode()) {
#ifdef BOOT_USE_STORAGE_NOFS
        hal_aos_nofs_read(file, APP_RESTORE_EMMC_ADDR, buf, len);
#endif
    } else {
        boot_norflash_read(HAL_FLASH_ID_0, APP_RESTORE_FLASH_ADDR, buf, len);
    }
}

static uint32_t d_get_block_data_len(uint32_t fileLen, uint32_t blockNum)
{
    uint32_t blockLen = 0;
    if(fileLen >= LEN_OF_CODED_BLOCK * (blockNum+1))
    {
        blockLen = LEN_OF_CODED_BLOCK;
    }
    else if(fileLen > LEN_OF_CODED_BLOCK * blockNum)
    {
        blockLen = fileLen % LEN_OF_CODED_BLOCK;
    }

    return blockLen;
}

static bool ota_diff_single_block_decode(uint32_t codedFileStartOffset, uint32_t* compFileOffset,
                                        uint32_t oldAppFileLen, uint32_t newAppFileLen, OTA_FILE_INFO_T* fileInfo)
{
    bool firstBlock = false;
    uint32_t decompFileBlockLen = 0;
    uint32_t currentCrcValue = 0;
    uint32_t compFileBlockLen = 0, compFileBlockNum = 0;
    uint32_t oldFileBlockLen = 0, oldFileOffset = 0;
    uint32_t newFileCrc = 0, newFileBlockLen = 0, newFileBlockCrc = 0, newFileCrcLen = 0;
    static uint32_t retryCount = 0;

    OTA_TASK_CONFIG_T otaConfig = {0};
    __COMP_FILE_BLOCK_T comFileBlock = {0};

    //get configuration
    otaConfig = ota_decode_breakpoint_get();

    // check breakpoint
    if(otaConfig.decodeStatus == DECODE_RESTORING)
    {
        compFileBlockNum = otaConfig.compFileBlockNum;
        newFileCrc = otaConfig.newFileCrc;
        oldFileOffset = otaConfig.oldFileOffset;
        newFileCrcLen = otaConfig.newFileCrcLen;
        newFileBlockLen = otaConfig.newFileBlockLen;
    }

    TRACE_IMM(0,"[%s]LINE:%d compFileOffset 0x%x, oldFileOffset 0x%x",
            __func__, __LINE__, *compFileOffset, oldFileOffset);

    if (otaConfig.blockStatus != 1)
    {
        //get encode block header
        ota_read_coded_data(codedFileStartOffset+*compFileOffset, (uint8_t*)&(comFileBlock), sizeof(__COMP_FILE_BLOCK_T));
        compFileBlockLen = comFileBlock.compressDataLen + sizeof(__COMP_FILE_BLOCK_T);
        decompFileBlockLen = comFileBlock.orgCompressLen;

        //get encode data
        TRACE_IMM(0,"[%s]LINE:%d compressDataLen 0x%x orgCompressLen 0x%x", __func__, __LINE__,
                comFileBlock.compressDataLen, comFileBlock.orgCompressLen);
        void *srcBuf = bes_lzma_malloc(compFileBlockLen);
        memset(srcBuf, 0, compFileBlockLen);
        ota_read_coded_data(codedFileStartOffset+*compFileOffset, srcBuf, compFileBlockLen);
        *compFileOffset += compFileBlockLen;

        //lzma decode
        void *decompressed = bes_lzma_malloc(decompFileBlockLen);
        memset(decompressed, 0xff, decompFileBlockLen);
        bes_lzma_dec_buf(srcBuf, compFileBlockLen, decompressed, &decompFileBlockLen);
        bes_lzma_free(srcBuf);

        //needn't handle crc of encode data, ignore it
        *compFileOffset += SIZEOF_CRC;
        //needn't handle crc of diff data, ignore it
        *compFileOffset += SIZEOF_CRC;

        //Confirmed oldfileblockdata len
        oldFileBlockLen = d_get_block_data_len(oldAppFileLen, compFileBlockNum);
        //Confirmed newfileblockdata len
        newFileBlockLen = d_get_block_data_len(newAppFileLen, compFileBlockNum);

        newFileCrcLen = newFileBlockLen;
        TRACE_IMM(0,"[%s]LINE:%d old/new/decomp appfile block 0x%x 0x%x 0x%x",
                __func__, __LINE__, oldFileBlockLen, newFileBlockLen, decompFileBlockLen);
        if(newFileBlockLen < oldFileBlockLen)
        {
            newFileBlockLen = oldFileBlockLen;
        }

        if(newFileBlockLen % FLASH_SECTOR_SIZE_IN_BYTES)
        {
            newFileBlockLen = (newFileBlockLen / FLASH_SECTOR_SIZE_IN_BYTES + 1) * FLASH_SECTOR_SIZE_IN_BYTES;
        }

        //get oldfileblockdata
        memset(appData, 0xff, sizeof(appData));
        if(oldFileBlockLen != 0)
        {
            ota_read_old_data(fileInfo->destAddr, oldFileOffset, appData, oldFileBlockLen);
        }

        //bspatch
        bspatch((const uint8_t*)(appData), newFileBlockLen, decompressed, decompFileBlockLen, newappdata, newFileBlockLen);
        bes_lzma_free(decompressed);

        //store new file block data in backup area firstly
        if (0 == oldFileOffset && fileInfo->type < CUSTOM_ADDR_TYPE)
        {
            *(uint32_t *)&newappdata = BOOT_MAGIC_NUMBER;
            firstBlock = true;
        }
        d_write_tmp_data(newappdata, newFileBlockLen);
        otaConfig.blockStatus = 1;
        otaConfig.newFileCrcLen = newFileCrcLen;
        otaConfig.compFileOffset = *compFileOffset;
        otaConfig.newFileBlockLen = newFileBlockLen;
        otaConfig.decodeStatus = DECODE_RESTORING;
        TRACE_IMM(0, "firstBlock 0x%x compFileOffset 0x%x", firstBlock, *compFileOffset);
        TRACE_IMM(0, "oldFileOffset 0x%x newFileBlockLen 0x%x", oldFileOffset, newFileBlockLen);
        ota_decode_breakpoint_set(&otaConfig);
    }

    //Confirmed new file block crc
    memset(newappdata, 0, sizeof(newappdata));
    d_read_tmp_data(newappdata, newFileBlockLen);
    currentCrcValue = crc_update(0, (unsigned char*)newappdata, newFileBlockLen, firstBlock);

    ota_read_coded_data(codedFileStartOffset+*compFileOffset, (uint8_t*)&(newFileBlockCrc), SIZEOF_CRC);
    *compFileOffset += SIZEOF_CRC;

    TRACE_IMM(0,"[%s]LINE:%d new block CRC32 0x%x current CRC32 0x%x", __func__, __LINE__, newFileBlockCrc, currentCrcValue);
    if (currentCrcValue != newFileBlockCrc)
    {
        TRACE_IMM(0,"[%s]LINE:%d new file block crc error, ret", __func__, __LINE__);
        //Restore offset and retry
        *compFileOffset = *compFileOffset - compFileBlockLen - SIZEOF_CRC*3;
        otaConfig.blockStatus = 0;
        otaConfig.compFileOffset = *compFileOffset;
        ota_decode_breakpoint_set(&otaConfig);
        if (retryCount++ > MAX_RETRY_TIMES)
        {
            // terminate the unpacking of current file
            return false;
        }
    }

    //save new file block data in running area
    ota_cover_old_data(fileInfo->destAddr, oldFileOffset, newappdata, newFileBlockLen);

    _watchdog_ping();

    //Confirmed new file crc
    memset(newappdata, 0, sizeof(newappdata));
    ota_read_old_data(fileInfo->destAddr, oldFileOffset, newappdata, newFileBlockLen);

    newFileCrc = crc_update(newFileCrc, (unsigned char*)newappdata, newFileCrcLen, firstBlock);
    oldFileOffset += newFileBlockLen;

    //next block
    compFileBlockNum++;
    otaConfig.blockStatus = 0;
    otaConfig.newFileCrc = newFileCrc;
    otaConfig.oldFileOffset = oldFileOffset;
    otaConfig.compFileOffset = *compFileOffset;
    otaConfig.compFileBlockNum = compFileBlockNum;
    TRACE_IMM(0, "compFileBlockNum 0x%x oldFileOffset 0x%x", otaConfig.compFileBlockNum, otaConfig.oldFileOffset);
    ota_decode_breakpoint_set(&otaConfig);
    return true;
}

static bool ota_check_diff_packet_vaild(uint32_t startAddr,uint32_t count)
{
    uint32_t fileInfoOffset = sizeof(OTA_MULTI_FILE_HEAD_T), curCompFileIndex = 0;
    uint32_t currentCrcValue = 0, calculateLen = 0, oldFileOffset = 0;
    OTA_FILE_INFO_T fileInfo = {0};
    __COMP_FILE_HEADER_T compFileHead;
    bool ret = true, firstBlock = false;
    void *buf = bes_lzma_malloc(LEN_OF_VAILD_CHECK);

    for(; curCompFileIndex < count; curCompFileIndex++)
    {
        //get fileInfo;
        ota_read_coded_data(startAddr+fileInfoOffset,
                        (uint8_t*)&(fileInfo),
                        sizeof(OTA_FILE_INFO_T));
        fileInfoOffset += sizeof(OTA_FILE_INFO_T);

        //open current file
        ota_open_type_file(fileInfo.destAddr);

        // get file header
        ota_read_coded_data(startAddr+fileInfo.resourceOffset,
                        (uint8_t*)&(compFileHead),
                        sizeof(__COMP_FILE_HEADER_T));

        // crc calculate
        TRACE_IMM(0,"[%s] oldAppFileLen:%d", __func__, compFileHead.oldAppFileLen);
        while (oldFileOffset < compFileHead.oldAppFileLen)
        {
            firstBlock = false;
            calculateLen = (compFileHead.oldAppFileLen-oldFileOffset) > LEN_OF_VAILD_CHECK ? LEN_OF_VAILD_CHECK : (compFileHead.oldAppFileLen-oldFileOffset);
            ota_read_old_data(fileInfo.destAddr, oldFileOffset, buf, calculateLen);
            if (0 == oldFileOffset && fileInfo.type < CUSTOM_ADDR_TYPE)
            {
                *(uint32_t *)buf = BOOT_MAGIC_NUMBER;
                firstBlock = true;
            }
            currentCrcValue = crc_update(currentCrcValue, (unsigned char*)buf, calculateLen, firstBlock);
            oldFileOffset += calculateLen;
        }

        //close current file
        ota_close_type_file(fileInfo.destAddr);

        TRACE_IMM(0,"[%s] %d %x %x", __func__, curCompFileIndex, compFileHead.oldAppFileCrc, currentCrcValue);
        if (compFileHead.oldAppFileCrc != currentCrcValue)
        {
            ret = false;
            break;
        }
        oldFileOffset = 0;
        currentCrcValue = 0;
    }
    return ret;
}

#endif

#if defined(OTA_BIN_COMPRESSED)
static bool ota_comp_single_block_decode(uint32_t codedFileStartOffset, uint32_t* compFileOffset, OTA_FILE_INFO_T* fileInfo)
{
    bool firstBlock = false;
    uint32_t decompFileBlockLen = 0;
    uint32_t currentCrcValue = 0;
    uint32_t compFileBlockLen = 0;
    uint32_t oldFileOffset = 0;
    uint32_t newFileCrc = 0, newFileBlockCrc = 0;
    static uint32_t retryCount = 0;

    OTA_TASK_CONFIG_T otaConfig = {0};
    __COMP_FILE_BLOCK_T comFileBlock = {0};

    //get configuration
    otaConfig = ota_decode_breakpoint_get();

    // check breakpoint
    if(otaConfig.decodeStatus == DECODE_RESTORING)
    {
        newFileCrc = otaConfig.newFileCrc;
        oldFileOffset = otaConfig.oldFileOffset;
    }

    TRACE_IMM(0,"[%s]LINE:%d compFileOffset 0x%x, oldFileOffset 0x%x",
            __func__, __LINE__, *compFileOffset, oldFileOffset);

    //get encode block header
    ota_read_coded_data(codedFileStartOffset+*compFileOffset, (uint8_t*)&(comFileBlock), sizeof(__COMP_FILE_BLOCK_T));
    compFileBlockLen = comFileBlock.compressDataLen + sizeof(__COMP_FILE_BLOCK_T);
    decompFileBlockLen = comFileBlock.orgCompressLen;

    //get encode data
    TRACE_IMM(0,"[%s]LINE:%d compressDataLen 0x%x orgCompressLen 0x%x", __func__, __LINE__,
            comFileBlock.compressDataLen, comFileBlock.orgCompressLen);
    void *srcBuf = bes_lzma_malloc(compFileBlockLen);
    memset(srcBuf, 0, compFileBlockLen);
    ota_read_coded_data(codedFileStartOffset+*compFileOffset, srcBuf, compFileBlockLen);
    *compFileOffset += compFileBlockLen;

    //lzma decode
    void *decompressed = bes_lzma_malloc(decompFileBlockLen);
    memset(decompressed, 0xff, decompFileBlockLen);
    bes_lzma_dec_buf(srcBuf, compFileBlockLen, decompressed, &decompFileBlockLen);
    bes_lzma_free(srcBuf);

    //needn't handle crc of encode data, ignore it
    *compFileOffset += SIZEOF_CRC;
    //needn't handle crc of diff data, ignore it
    *compFileOffset += SIZEOF_CRC;

    //store new file block data in backup area firstly
    if (0 == oldFileOffset && fileInfo->type < CUSTOM_ADDR_TYPE)
    {
        *(uint32_t *)decompressed = BOOT_MAGIC_NUMBER;
        firstBlock = true;
    }

    //Confirmed new file block crc
    currentCrcValue = crc_update(0, (unsigned char*)decompressed, decompFileBlockLen, firstBlock);

    ota_read_coded_data(codedFileStartOffset+*compFileOffset, (uint8_t*)&(newFileBlockCrc), SIZEOF_CRC);
    *compFileOffset += SIZEOF_CRC;

    TRACE_IMM(0,"[%s]LINE:%d new block CRC32 0x%x current CRC32 0x%x", __func__, __LINE__, newFileBlockCrc, currentCrcValue);
    if (currentCrcValue != newFileBlockCrc)
    {
        TRACE_IMM(0,"[%s]LINE:%d new file block crc error, ret", __func__, __LINE__);
        //Restore offset and retry
        *compFileOffset = *compFileOffset - compFileBlockLen - SIZEOF_CRC*3;
        otaConfig.blockStatus = 0;
        otaConfig.compFileOffset = *compFileOffset;
        ota_decode_breakpoint_set(&otaConfig);
        if (retryCount++ > MAX_RETRY_TIMES)
        {
            // terminate the unpacking of current file
            return false;
        }
    }

    //save new file block data in running area
    ota_cover_old_data(fileInfo->destAddr, oldFileOffset, decompressed, decompFileBlockLen);

    _watchdog_ping();

    //Confirmed new file crc
    memset(decompressed, 0, decompFileBlockLen);
    ota_read_old_data(fileInfo->destAddr, oldFileOffset, decompressed, decompFileBlockLen);

    newFileCrc = crc_update(newFileCrc, (unsigned char*)decompressed, decompFileBlockLen, firstBlock);
    oldFileOffset += decompFileBlockLen;
    bes_lzma_free(decompressed);

    //next block
    otaConfig.newFileCrc = newFileCrc;
    otaConfig.oldFileOffset = oldFileOffset;
    otaConfig.compFileOffset = *compFileOffset;
    otaConfig.decodeStatus = DECODE_RESTORING;
    TRACE_IMM(0, "oldFileOffset 0x%x", otaConfig.oldFileOffset);
    ota_decode_breakpoint_set(&otaConfig);
    return true;
}
#endif

static bool ota_single_block_decode(uint32_t codedFileStartOffset, uint32_t* compFileOffset,
                                    uint32_t newAppFileLen, OTA_FILE_INFO_T* fileInfo)
{
    bool firstBlock = false;
    static uint32_t retryCount = 0;
    uint32_t currentCrcValue = 0;
    uint32_t oldFileOffset = 0;
    uint32_t newFileCrc = 0, newFileBlockCrc = 0;
    uint32_t decodeFileBlockLen = LEN_OF_CODED_BLOCK;

    OTA_TASK_CONFIG_T otaConfig = {0};

    //get configuration
    otaConfig = ota_decode_breakpoint_get();

    // check breakpoint
    if(otaConfig.decodeStatus == DECODE_RESTORING)
    {
        newFileCrc = otaConfig.newFileCrc;
        oldFileOffset = otaConfig.oldFileOffset;
    }

    TRACE_IMM(0,"[%s]LINE:%d compFileOffset 0x%x, oldFileOffset 0x%x",
            __func__, __LINE__, *compFileOffset, oldFileOffset);

    //get encode data
    if (LEN_OF_CODED_BLOCK > newAppFileLen - oldFileOffset)
    {
        decodeFileBlockLen = newAppFileLen - oldFileOffset;
    }
    
    uint8_t *srcBuf = appData;   //reuse buffer
    memset(srcBuf, 0, decodeFileBlockLen);
    ota_read_coded_data(codedFileStartOffset+*compFileOffset, srcBuf, decodeFileBlockLen);
    *compFileOffset += decodeFileBlockLen;

    //needn't handle crc of encode data, ignore it
    *compFileOffset += SIZEOF_CRC;
    //needn't handle crc of diff data, ignore it
    *compFileOffset += SIZEOF_CRC;

    //store new file block data in backup area firstly
    if (0 == oldFileOffset && fileInfo->type < CUSTOM_ADDR_TYPE)
    {
        TRACE_IMM(0,"[%s]LINE:%d ", __func__, __LINE__);
        *(uint32_t *)srcBuf = BOOT_MAGIC_NUMBER;
        firstBlock = true;
    }

    //Confirmed new file block crc
    currentCrcValue = crc_update(0, (unsigned char*)srcBuf, decodeFileBlockLen, firstBlock);

    ota_read_coded_data(codedFileStartOffset+*compFileOffset, (uint8_t*)&(newFileBlockCrc), SIZEOF_CRC);
    *compFileOffset += SIZEOF_CRC;

    TRACE_IMM(0,"[%s]LINE:%d new block CRC32 0x%x current CRC32 0x%x", __func__, __LINE__, newFileBlockCrc, currentCrcValue);
    if (currentCrcValue != newFileBlockCrc)
    {
        TRACE_IMM(0,"[%s]LINE:%d new file block crc error, ret", __func__, __LINE__);
        //Restore offset and retry
        *compFileOffset = *compFileOffset - decodeFileBlockLen - SIZEOF_CRC*3;
        otaConfig.blockStatus = 0;
        otaConfig.compFileOffset = *compFileOffset;
        ota_decode_breakpoint_set(&otaConfig);
        if (retryCount++ > MAX_RETRY_TIMES)
        {
            // terminate the unpacking of current file
            return false;
        }
    }

    //save new file block data in running area
    ota_cover_old_data(fileInfo->destAddr, oldFileOffset, srcBuf, decodeFileBlockLen);

    _watchdog_ping();

    //Confirmed new file crc
    memset(srcBuf, 0, decodeFileBlockLen);
    ota_read_old_data(fileInfo->destAddr, oldFileOffset, srcBuf, decodeFileBlockLen);

    newFileCrc = crc_update(newFileCrc, (unsigned char*)srcBuf, decodeFileBlockLen, firstBlock);
    oldFileOffset += decodeFileBlockLen;

    //next block
    otaConfig.newFileCrc = newFileCrc;
    otaConfig.oldFileOffset = oldFileOffset;
    otaConfig.compFileOffset = *compFileOffset;
    otaConfig.decodeStatus = DECODE_RESTORING;
    TRACE_IMM(0, "oldFileOffset 0x%x", otaConfig.oldFileOffset);
    ota_decode_breakpoint_set(&otaConfig);
    return true;
}

static uint32_t ota_single_file_decode(OTA_FILE_INFO_T* fileInfo)
{
    uint32_t codedFileStartOffset = 0, compFileOffset = 0;
    OTA_TASK_CONFIG_T otaConfig = {0};
    __COMP_FILE_HEADER_T compFileHead;
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    bool ret = false;

    boot_norflash_read(DST_FLASH_ID, (uint32_t)bootinfoCtrl, (uint8_t *)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));

    //get configuration
    otaConfig = ota_decode_breakpoint_get();

    //get start addr
    codedFileStartOffset = otaBootInfo.newImageFlashOffset+fileInfo->resourceOffset;
    TRACE_IMM(0,"[%s]LINE:%d type 0x%x, destAddr 0x%x, resourceOffset 0x%x",
            __func__, __LINE__, fileInfo->type, fileInfo->destAddr, fileInfo->resourceOffset);

    //open current file
    ota_open_type_file(fileInfo->destAddr);

    //get comp file header
    ota_read_coded_data(codedFileStartOffset+compFileOffset, (uint8_t*)&(compFileHead), sizeof(__COMP_FILE_HEADER_T));
    compFileOffset += sizeof(__COMP_FILE_HEADER_T);

    //clear comp block num
    TRACE_IMM(0,"[%s]LINE:%d compressDataLen 0x%x, compressBlockLen 0x%x",
            __func__, __LINE__, compFileHead.compressDataLen, compFileHead.compressBlockLen);
    TRACE_IMM(0,"[%s]LINE:%d newAppFileLen 0x%x, oldAppFileLen 0x%x",
            __func__, __LINE__, compFileHead.newAppFileLen, compFileHead.oldAppFileLen);

    // breakpoint check
    if(otaConfig.decodeStatus == DECODE_RESTORING)
    {
        compFileOffset = otaConfig.compFileOffset;
    }

    while(compFileHead.compressDataLen > compFileOffset)
    {
        switch (otaEnv.packedType)
        {
        #if defined(OTA_USE_DIFF)
        case PACKED_DIFF:
            ret = ota_diff_single_block_decode(codedFileStartOffset, &compFileOffset,
                                compFileHead.oldAppFileLen, compFileHead.newAppFileLen, fileInfo);
            break;
        #endif
        #if defined(OTA_BIN_COMPRESSED)
        case PACKED_COMPRESS:
            ret = ota_comp_single_block_decode(codedFileStartOffset, &compFileOffset, fileInfo);
            break;
        #endif
        case PACKED_COMPLETE:
        default:
            ret = ota_single_block_decode(codedFileStartOffset, &compFileOffset,
                                compFileHead.newAppFileLen, fileInfo);
            break;
        }

        if (!ret)
        {
            break;
        }

        #if defined(BOOT_USE_DISPLAY)
        ota_progress_update(codedFileStartOffset+compFileOffset);
        #endif
    }

    //close current file
    ota_close_type_file(fileInfo->destAddr);
    return compFileHead.newAppFileCrc;
}

static void ota_multi_file_decode(void)
{
    TRACE_IMM(0,"[%s]LINE:%d start", __func__, __LINE__);

    uint32_t fileInfoOffset = 0, currentCrcValue = 0, curCompFileIndex = 0;
    OTA_MULTI_FILE_HEAD_T multiFileHead;
    OTA_FILE_INFO_T fileInfo;
    OTA_TASK_CONFIG_T otaConfig = {0};
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    bool reboot_normal = false;
    bool ota_succeed = false;

    boot_norflash_read(DST_FLASH_ID, (uint32_t)bootinfoCtrl, (uint8_t *)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));

    //get configuration
    otaConfig = ota_decode_breakpoint_get();

    //get ram
    #if defined(OTA_BIN_COMPRESSED)
    syspool_init();
    bes_lzma_dec_init(syspool_start_addr(), syspool_total_size());
    #endif

    //check file crc before decode
    currentCrcValue = ota_check_whole_image_crc(otaBootInfo.newImageFlashOffset, otaBootInfo.imageSize);
    TRACE_IMM(0, "[%s]LINE:%d multiFile Original CRC32 0x%x current CRC32 0x%x.",
            __func__, __LINE__, otaBootInfo.imageCrc, currentCrcValue);
    if (currentCrcValue != otaBootInfo.imageCrc)
    {
        TRACE_IMM(0,"ERROR: The crc value is incorrect and the upgrade file is damaged");
        reboot_normal = true;
        goto _exit;
    }

    //get multiFileHead
    ota_read_coded_data(otaBootInfo.newImageFlashOffset,
                    (uint8_t*)&(multiFileHead),
                    sizeof(OTA_MULTI_FILE_HEAD_T));
    fileInfoOffset += sizeof(OTA_MULTI_FILE_HEAD_T);

    // packeted tool check
    if (PACKED_TOOL_VER_MAJOR > multiFileHead.toolVersion || PACKED_TOOL_VER_LATEST < multiFileHead.toolVersion)
    {
        TRACE_IMM(0,"ERROR: Packing tool version is too low %8x, plase update your tool.", multiFileHead.toolVersion);
        reboot_normal = true;
        goto _exit;
    }

    otaEnv.packedType = multiFileHead.packedType;
    // if get a diff packet, must check old data match or not
    TRACE_IMM(0,"[%s] packedType:%d", __func__, otaEnv.packedType);
    #if defined(OTA_USE_DIFF)
    if (PACKED_DIFF == otaEnv.packedType)
    {
        bool ret = ota_check_diff_packet_vaild(otaBootInfo.newImageFlashOffset, multiFileHead.resourceCount);
        if (!ret)
        {
            TRACE_IMM(0,"ERROR: this diff-packet is not match current burning frimware, plase pack again.");
            reboot_normal = true;
            goto _exit;
        }
    }
    #endif

    if (!ota_check_whole_image_md5(otaBootInfo.newImageFlashOffset, otaBootInfo.imageSize))
    {
        TRACE_IMM(0,"ERROR: MD5 check failed.");
        reboot_normal = true;
        goto _exit;
    }

    //breakpoint check
    TRACE_IMM(0, "[breakpoint]decodeStatus 0x%x blockStatus 0x%x compFileOffset 0x%x ",otaConfig.decodeStatus,otaConfig.blockStatus ,otaConfig.compFileOffset);
    TRACE_IMM(0, "[breakpoint]compFileBlockNum 0x%x fileInfoOffset 0x%x oldFileOffset 0x%x", otaConfig.compFileBlockNum, otaConfig.fileInfoOffset, otaConfig.oldFileOffset);
    if(otaConfig.decodeStatus != DECODE_IDLE)
    {
        fileInfoOffset = otaConfig.fileInfoOffset;
        curCompFileIndex = otaConfig.curCompFileIndex;
    }
    else
    {
        otaConfig.fileInfoOffset = fileInfoOffset;
        ota_decode_breakpoint_set(&otaConfig);
    }

    //get comp file num form multiFileHead
    TRACE_IMM(0,"[%s]LINE:%d resourceCount is 0x%x, max count 0x%x ", __func__, __LINE__, multiFileHead.resourceCount, MAX_RESOURCE_COUNT);
    #if defined(BOOT_USE_DISPLAY)
    ota_total_len_calculate(otaBootInfo.newImageFlashOffset, multiFileHead.resourceCount);
    #endif

    /*******************************************************************************************************************/
    /*DATA FORMAT*******************************************************************************************************/
    /*WHOLE IMAGE: MULTI FILE HEADER(20bytes)+FILE0(x*bytes)+FILE1(y*bytes)+...*****************************************/
    /*EACH FILE:   FILE INFO(52bytes)+BLOCK0(n*bytes)+BLOCK1(m*bytes)+...+"CRC32_OF_IMAGE=0x00000000"*******************/
    /*EACH BLOCK:  BLOCK HEADER(13bytes)+ENCODE DATA(j*bytes)+ENCODE CRC(4bytes)+DIFF CRC(4bytes)+NEW DATA CRC(4bytes)**/
    /*BLOCK HEADER:DEFINED BY LZMA**************************************************************************************/
    /*ENCODE DATA: LZMA(64k DIFF DATA)**********************************************************************************/
    /*******************************************************************************************************************/
    for(; curCompFileIndex < multiFileHead.resourceCount; curCompFileIndex++)
    {
        //get fileInfo;
        ota_read_coded_data(otaBootInfo.newImageFlashOffset+fileInfoOffset,
                        (uint8_t*)&(fileInfo),
                        sizeof(OTA_FILE_INFO_T));
        fileInfoOffset += sizeof(OTA_FILE_INFO_T);

        currentCrcValue = ota_single_file_decode(&fileInfo);

        // decode successfully if crc is correct
        otaConfig = ota_decode_breakpoint_get();
        TRACE_IMM(0,"[%s]LINE:%d new_file Original CRC32 0x%x current CRC32 0x%x",
                __func__, __LINE__, otaConfig.newFileCrc, currentCrcValue);
        if (otaConfig.newFileCrc == currentCrcValue)
        {
            otaConfig.decodeStatus = DECODE_NEXT_FILE;
            otaConfig.fileInfoOffset = fileInfoOffset;
            otaConfig.curCompFileIndex = curCompFileIndex;
            TRACE_IMM(0, "curCompFileIndex 0x%x fileInfoOffset 0x%x", otaConfig.curCompFileIndex, otaConfig.fileInfoOffset);
            ota_decode_breakpoint_set(&otaConfig);
        }
        else
        {
            // enable the backup area
            ota_backup_area_config();
            break;
        }
    }

    // check the last file crc value
    if (otaConfig.newFileCrc == currentCrcValue)
    {
        #if defined(BOOT_USE_DISPLAY)
        // show ota has finished
        show_ota_progress(100, true);
        hal_sys_timer_delay(MS_TO_TICKS(100));
        show_ota_status(true);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        #endif
        ota_succeed = true;
        reboot_normal = true;
        // prepare for the update of backup area
        ota_backup_area_update_prepare();
    }

_exit:

    if (!ota_succeed)
    {
        #if defined(BOOT_USE_DISPLAY)
        // show ota failure
        show_ota_status(false);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        #endif
    }

    if (reboot_normal)
    {
        boot_norflash_read(DST_FLASH_ID, (uint32_t)bootinfoCtrl, (uint8_t *)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));
        otaBootInfo.magicNumber = BOOT_MAGIC_NUMBER;
        update_boot_info(&otaBootInfo);
    }

    memset(&otaConfig, 0, sizeof(otaConfig));
    ota_decode_breakpoint_set(&otaConfig);

    TRACE_IMM(0,"[%s]LINE:%d end", __func__, __LINE__);
}

void copy_new_image(void)
{
    ota_multi_file_decode();
}

void boot2_watch_ota_handler(const FLASH_OTA_BOOT_INFO_T * ctrl)
{
    if (!ctrl) {
        TRACE_IMM(0,"[%s]LINE:%d ERROR PARMA", __func__, __LINE__);
        goto _exit;
    }
    bootinfoCtrl = ctrl;

    TRACE_IMM(0,"[%s]LINE:%d bootWord:%x", __func__, __LINE__,ctrl->bootWord);
    if (ctrl->magicNumber != OTA_COPY_BLOCK_MAGIC) {
        if (ctrl->bootWord == BOOT_WORD_TBCF || ctrl->bootWord == BOOT_WORD_TBCU) {
            FLASH_OTA_BOOT_INFO_T otaBootInfo;
            memcpy(&otaBootInfo, ctrl, sizeof(FLASH_OTA_BOOT_INFO_T));
            otaBootInfo.magicNumber = OTA_COPY_BLOCK_MAGIC;
            update_boot_info(&otaBootInfo);
        } else {
            TRACE_IMM(0,"[%s]LINE:%d magicNumber=%x needn't handle", __func__, __LINE__, ctrl->magicNumber);
        }
        goto _exit;
    }

    pmu_open();
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);

#if defined(BOOT_USE_DISPLAY)
    extern int fb_gpu_test_init(void);
    fb_gpu_test_init();
    TRACE_IMM(0, "[%s]LINE:%d display init done", __func__, __LINE__);
#endif

#if defined(BOOT_USE_STORAGE_NOFS)
    int ret = hal_aos_nofs_mount(MOUNT_DEV_OS, MOUNT_OS_FILE);
    ASSERT(ret == 0, "mount emmc0_1 failed");
    ret = hal_aos_nofs_mount(MOUNT_DEV, MOUNT_TRAGET);
    ASSERT(ret == 0, "mount emmc0_2 failed");
    TRACE_IMM(0, "[%s]LINE:%d nofs init done", __func__, __LINE__);
    if (otaEnv.newFp == 0) {
        otaEnv.newFp = hal_aos_nofs_open(MOUNT_TRAGET, O_RDWR);
        ASSERT(otaEnv.newFp, "open nofs failed");
    }
#endif

    if (ctrl->bootWord == BOOT_WORD_TBCF || ctrl->bootWord == BOOT_WORD_TBCU) {
        TRACE_IMM(0,"[%s]LINE:%d bootWord=%x App bring up failed, enable backup area", __func__, __LINE__, ctrl->bootWord);
        ota_backup_area_config();
        goto _reboot;
    } else {
        copy_new_image();
    }

#if defined(BOOT_USE_STORAGE_NOFS)
    if (otaEnv.newFp != 0) {
        hal_aos_nofs_close(otaEnv.newFp);
    }
    ret = hal_aos_nofs_umount(MOUNT_OS_FILE);
    ASSERT(ret == 0, "unmount emmc0_1 failed");
    ret = hal_aos_nofs_umount(MOUNT_TRAGET);
    ASSERT(ret == 0, "unmount emmc0_2 failed");
    TRACE_IMM(0, "[%s]LINE:%d nofs deinit done", __func__, __LINE__);
#endif

_reboot:
    hal_cmu_sys_reboot();
    while (1);

_exit:
    return;
}
