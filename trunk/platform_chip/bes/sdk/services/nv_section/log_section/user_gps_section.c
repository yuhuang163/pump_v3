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
#ifdef USER_GPS_TO_FLASH
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "cmsis.h"
#include "user_gps_section.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "app_flash_api.h"

extern uint32_t __user_gps_section_start;
extern uint32_t __user_gps_section_end;

static const uint32_t user_gps_flash_start_addr = (uint32_t)&__user_gps_section_start;
static const uint32_t user_gps_flash_end_addr =  (uint32_t)&__user_gps_section_end;

void user_gps_to_flash_init()
{
    enum HAL_FLASH_ID_T flash_id;

    flash_id = app_flash_get_dev_id_by_addr(user_gps_flash_start_addr);
    app_flash_register_module(NORFLASH_API_MODULE_ID_USER_GPS,
                    flash_id,
                    user_gps_flash_start_addr,
                    user_gps_flash_end_addr - user_gps_flash_start_addr,
                    NULL);
    TRACE(4, "[%s] done", __func__);
}

void user_gps_erase_section()
{
    app_flash_erase(NORFLASH_API_MODULE_ID_USER_GPS,
                    0,
                    user_gps_flash_end_addr - user_gps_flash_start_addr);
    // TRACE(4, "[%s] done", __func__);
}

void user_gps_write(const uint8_t *ptr, uint32_t len, uint32_t offset)
{
    app_flash_program(NORFLASH_API_MODULE_ID_USER_GPS,
                    offset,
                    ptr,
                    len,
                    true);
    // TRACE(4, "[%s] done", __func__);
    return 0;
}

void user_gps_read(uint8_t *ptr, uint32_t len, uint32_t offset)
{
    app_flash_read(NORFLASH_API_MODULE_ID_USER_GPS,
                    offset,
                    ptr,
                    len);
    // TRACE(4, "[%s] done", __func__);
    return 0;
}

#endif
