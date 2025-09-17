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
#ifndef _USER_GPS_SECTION_H
#define _USER_GPS_SECTION_H
#define USER_GPS_SECTOR_SIZE             0x1000
#define USER_GPS_SECTOR_SIZE_MASK        0xFFF
#define USER_GPS_BUFFER_LEN              (USER_GPS_SECTOR_SIZE*2)
#define USER_GPS_NORFALSH_BUFFER_LEN     (USER_GPS_BUFFER_LEN)
#if defined(__cplusplus)
extern "C" {
#endif

#include "stdint.h"

void user_gps_to_flash_init();
void user_gps_erase_section();
void user_gps_write(const uint8_t *ptr, uint32_t len, uint32_t offset);
void user_gps_read(uint8_t *ptr, uint32_t len, uint32_t offset);

#if defined(__cplusplus)
}
#endif

#endif

