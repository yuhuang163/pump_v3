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
#include "flash_tester.h"

#define FLASH_CMD(n)            {TO_STRING(n), CONCAT(flash_tester_, n)},
#define FLASH_NC_OFFSET	        0x04000000
#define FLASH_UNUSED_OFFSET     0x00040000

static enum HAL_FLASH_ID_T id = HAL_FLASH_ID_0;
static const uint32_t len  = FLASH_SECTOR_SIZE_IN_BYTES;
static const uint32_t addr = FLASH_BASE + FLASH_UNUSED_OFFSET;
static uint8_t buffer[FLASH_SECTOR_SIZE_IN_BYTES] = {0,};
static enum HAL_NORFLASH_RET_T ret;

static void flash_tester_erase(void)
{
    CMD_TRACE("erasing flash %d", id);
    ret = hal_norflash_erase(id, addr, len);
    ASSERT(ret == HAL_NORFLASH_OK, "flash erase error [%d]", ret);
	CMD_TRACE("flash erase finished");
}

static void flash_tester_write(void)
{
    CMD_TRACE("writing flash %d", id);
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = i;
    }
    ret = hal_norflash_write(id, addr, buffer, len);
    ASSERT(ret == HAL_NORFLASH_OK, "flash write error [%d]", ret);
	CMD_TRACE("flash write finished");
}

static void flash_tester_read(void)
{
    enum {bufLen = 100};

    CMD_TRACE("reading flash %d", id);
    for (uint32_t i = 0; i < bufLen; i++) {
        TRACE_IMM(0, "[0x%08X] value:\t\t0x%02X", (addr + i),
        *(uint8_t *)(addr - FLASH_NC_OFFSET + i));
    }
}

static const flash_tester_handler_t flash_tester_handler[] = {
    FLASH_CMD(erase)
    FLASH_CMD(write)
    FLASH_CMD(read)
};

static void flash_tester_flash_init(void)
{
    static bool entry[HAL_FLASH_ID_NUM] = {false};
    uint8_t buf[HAL_NORFLASH_DEVICE_ID_LEN];

    /* flash (nor flash) initialization */
	if (hal_norflash_opened(id) && !entry[id]) {
        if (id > HAL_FLASH_ID_0) {
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
            ret = hal_norflash_init(id);
#else
            PARAM_CHK(false);
#endif
        } else {
            ret = hal_norflash_init(id);
        }
        PARAM_CHK(ret == HAL_NORFLASH_OK);
	    hal_norflash_get_id(id, buf, ARRAY_SIZE(buf));
        CMD_TRACE("FLASH_ID:\t\t0x%02X-0x%02X-0x%02X", buf[0], buf[1], buf[2]);

        hal_norflash_show_calib_result(id);

	    ret = hal_norflash_get_open_state(id);
        PARAM_CHK(ret == HAL_NORFLASH_OK);
#if 0
	    DUMP32("0x%08X\n", (uint32_t *)FLASH_BASE, 20);
#endif
        entry[id] = true;
    }
}

void flash_tester(const char **params)
{
    ASSIGN_VAL(id, params[0]);

    PARAM_CHK(id < HAL_FLASH_ID_NUM);

    flash_tester_flash_init();

    CMD_HANDLER(flash_tester, params[1]);
}