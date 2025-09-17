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
#ifdef TEST_NORFLASH
#include "cmsis_os.h"
#include "hal_trace.h"
#include "stdlib.h"
#include "crc32_c.h"
#include "hal_norflash.h"

#define FLASH_TESTER_MAGICNUMBER           (0xBE571600)
#define FLASH_TESTER_VER                   (0x00000001)

#define FLASH_TESTER_WRITE_SIZE            (256)

uint8_t flash_cache[FLASH_SECTOR_SIZE_IN_BYTES] = {0,};

typedef struct{
    uint32_t magic;
    uint32_t version;
    uint32_t test_cnt;
    uint32_t crc;
}flash_test_head_t;

static void norflash_test_fill_pattern(void)
{
    uint32_t *p;
    uint32_t fill_byte_size;
    flash_test_head_t *head = (flash_test_head_t *)flash_cache;
    uint32_t i = 0;
    head->magic   = FLASH_TESTER_MAGICNUMBER;
    head->version = FLASH_TESTER_VER;
    head->test_cnt++;
    p = (uint32_t *)(flash_cache+sizeof(flash_test_head_t));
    
    fill_byte_size = FLASH_SECTOR_SIZE_IN_BYTES - sizeof(flash_test_head_t);
    for (i=0; i<fill_byte_size/4; i++){
        p[i] = rand();
    }
    head->crc = crc32_c(0, (uint8_t *)p, fill_byte_size);
    
    TRACE(0, "test_fill pattern magic:%08x crc:%08x test_cnt:%d", head->magic, head->crc, head->test_cnt);
}

static void norflash_test_verify_pattern(uint32_t addr)
{
    flash_test_head_t *head = NULL;
    uint8_t* verify_buf = NULL;
    uint32_t check_crc = 0;
    uint32_t *p;
    uint32_t fill_byte_size;

    TRACE(0, "test_verify pattern addr:%08x", addr);
    verify_buf = (uint8_t*)addr;
    head = (flash_test_head_t *)verify_buf;
    p = (uint32_t *)(verify_buf+sizeof(flash_test_head_t));
    fill_byte_size = FLASH_SECTOR_SIZE_IN_BYTES - sizeof(flash_test_head_t);
    check_crc = head->crc = crc32_c(0, (uint8_t *)p, fill_byte_size);
    TRACE(0, "test_verify pattern magic:%08x crc:%08x test_cnt:%d", head->magic, head->crc, head->test_cnt);
    if (check_crc == head->crc){
        TRACE(0, "test_verify success");
    }else{
        TRACE(0, "test_verify fail crc:%08x vs :%08x", check_crc, head->crc);
    }
}

static void test_norflash_loop(void)
{
    uint32_t *p = NULL;
    TRACE(0, "%s", __func__);
    norflash_test_verify_pattern((uint32_t)SYS_FLASH_NC_BASE);
    norflash_test_fill_pattern();
    norflash_test_verify_pattern((uint32_t)flash_cache);
    hal_norflash_erase(HAL_FLASH_ID_1, 0, FLASH_SECTOR_SIZE_IN_BYTES);
    for (uint32_t offset = 0; offset<FLASH_SECTOR_SIZE_IN_BYTES; offset+=FLASH_TESTER_WRITE_SIZE){
        hal_norflash_write(HAL_FLASH_ID_1, 0+offset, (uint8_t*)flash_cache+offset, FLASH_TESTER_WRITE_SIZE);
    }
    norflash_test_verify_pattern((uint32_t)SYS_FLASH_NC_BASE);
    p = (uint32_t *)SYS_FLASH_NC_BASE;
    DUMP32("%08x ", p, 10);
}

void test_norflash(void)
{
    uint32_t *p = NULL;
    TRACE(0, "%s start.", __func__);
    uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];

    TRACE(0, "TEST FLASH 1");
    norflash_test_verify_pattern((uint32_t)SYS_FLASH_NC_BASE);

    hal_norflash_init(HAL_FLASH_ID_1);
    hal_norflash_get_id(HAL_FLASH_ID_1, flash_id, ARRAY_SIZE(flash_id));
    TRACE(0, "FLASH_ID: %02X-%02X-%02X", flash_id[0], flash_id[1], flash_id[2]);
    hal_norflash_show_calib_result(HAL_FLASH_ID_1);
    ASSERT(hal_norflash_opened(HAL_FLASH_ID_1), "Failed to init flash: %d", hal_norflash_get_open_state(HAL_FLASH_ID_1));

    norflash_test_verify_pattern((uint32_t)SYS_FLASH_NC_BASE);
    norflash_test_fill_pattern();
    norflash_test_verify_pattern((uint32_t)flash_cache);
    hal_norflash_erase(HAL_FLASH_ID_1, 0, FLASH_SECTOR_SIZE_IN_BYTES);
    for (uint32_t offset = 0; offset<FLASH_SECTOR_SIZE_IN_BYTES; offset+=FLASH_TESTER_WRITE_SIZE){
        hal_norflash_write(HAL_FLASH_ID_1, 0+offset, (uint8_t*)flash_cache+offset, FLASH_TESTER_WRITE_SIZE);
    }
    norflash_test_verify_pattern((uint32_t)SYS_FLASH_NC_BASE);
    p = (uint32_t *)SYS_FLASH_NC_BASE;
    DUMP32("%08x ", p, 10);

    do {
        test_norflash_loop();
        osDelay(1000);
    } while(1);
}
#endif


