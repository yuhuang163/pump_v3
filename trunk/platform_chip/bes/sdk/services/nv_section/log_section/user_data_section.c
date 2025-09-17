/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifdef USER_DATA_TO_FLASH
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "cmsis.h"
#include "user_data_section.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "app_flash_api.h"

extern uint32_t __user_data_section_start;
extern uint32_t __user_data_section_end;

static const uint32_t user_data_flash_start_addr = (uint32_t)&__user_data_section_start;
static const uint32_t user_data_flash_end_addr =  (uint32_t)&__user_data_section_end;

static uint32_t user_data_flash_addr_offset = 0;

void user_data_to_flash_init()
{
    enum HAL_FLASH_ID_T flash_id;

    flash_id = app_flash_get_dev_id_by_addr(user_data_flash_start_addr);
    app_flash_register_module(NORFLASH_API_MODULE_ID_USER_DATA,
                    flash_id,
                    user_data_flash_start_addr,
                    user_data_flash_end_addr - user_data_flash_start_addr,
                    NULL);
    user_data_flash_addr_offset = 0;
    TRACE(4, "[%s] done", __func__);
}

void user_data_erase_section()
{
    user_data_flash_addr_offset = 0;
    app_flash_erase(NORFLASH_API_MODULE_ID_USER_DATA,
                    user_data_flash_addr_offset,
                    user_data_flash_end_addr - user_data_flash_start_addr);
    TRACE(4, "[%s] done", __func__);
}

int32_t user_data_write(const uint8_t *ptr, uint32_t len)
{
    enum NORFLASH_API_RET_T ret;
    uint32_t write_len;
    uint32_t remain_len = len;

    user_data_flash_addr_offset = user_data_flash_start_addr;

    //TRACE(4,"%s offset = 0x%x ptr = 0x%x len=0x%x",__func__, user_data_flash_addr_offset, ptr, len);
    do
    {
        if (user_data_flash_addr_offset % USER_DATA_SECTOR_SIZE != 0)
        {
            write_len = USER_DATA_SECTOR_SIZE - (user_data_flash_addr_offset - (user_data_flash_addr_offset & ~USER_DATA_SECTOR_SIZE_MASK));
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_USER_DATA,
                        user_data_flash_addr_offset,
                        ptr,
                        write_len,
                        false);
            if(ret != NORFLASH_API_OK)
            {
                TRACE(4,"%s: addr = 0x%x,write_len = 0x%x,ret = %d.",
                                __func__, user_data_flash_addr_offset, write_len, ret);
                return (int32_t)ret;
            }
            user_data_flash_addr_offset += write_len;
            ptr += write_len;
            remain_len -= write_len;
        }

        if (remain_len < USER_DATA_SECTOR_SIZE)
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_USER_DATA,
                        user_data_flash_addr_offset,
                        ptr,
                        remain_len,
                        false);
            if(ret != NORFLASH_API_OK)
            {
                TRACE(4,"%s: addr = 0x%x, remain_len = 0x%x, ret = %d.",
                                __func__, user_data_flash_addr_offset, remain_len, ret);
                return (int32_t)ret;
            }
            user_data_flash_addr_offset += remain_len;
            ptr += remain_len;
            remain_len = 0;
        }
        else
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_USER_DATA,
                        user_data_flash_addr_offset,
                        ptr,
                        USER_DATA_SECTOR_SIZE,
                        false);
            if(ret != NORFLASH_API_OK)
            {
                TRACE(3,"%s: addr = 0x%x, ret = %d.",
                                __func__, user_data_flash_addr_offset,ret);
                return (int32_t)ret;
            }
            user_data_flash_addr_offset += USER_DATA_SECTOR_SIZE;
            ptr += USER_DATA_SECTOR_SIZE;
            remain_len -= USER_DATA_SECTOR_SIZE;
        }
    } while (remain_len > 0);
    //TRACE(3,"%s offset = 0x%x ptr = 0x%x",__func__, user_data_flash_addr_offset, ptr);
    return 0;
}

int32_t user_data_read(uint8_t *ptr, uint32_t len, uint32_t offset)
{
    app_flash_read(NORFLASH_API_MODULE_ID_USER_DATA,
                    offset,
                    ptr,
                    len);
    TRACE(4, "[%s] done", __func__);
    return 0;
}

#endif
