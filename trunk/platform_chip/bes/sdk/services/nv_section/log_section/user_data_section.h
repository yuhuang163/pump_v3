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
#ifndef _USER_DATA_SECTION_H
#define _USER_DATA_SECTION_H
#define USER_DATA_SECTOR_SIZE             0x1000
#define USER_DATA_SECTOR_SIZE_MASK        0xFFF
#define USER_DATA_BUFFER_LEN              (USER_DATA_SECTOR_SIZE*2)
#define USER_DATA_NORFALSH_BUFFER_LEN     (USER_DATA_BUFFER_LEN)
#if defined(__cplusplus)
extern "C" {
#endif

void user_data_to_flash_init();
void user_data_erase_section();
int32_t user_data_write(const uint8_t* ptr,uint32_t len);
int32_t user_data_read(uint8_t *ptr, uint32_t len, uint32_t offset);

#if defined(__cplusplus)
}
#endif

#endif
