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
#if (defined TEST_USER_GPS_TO_FLASH)

#include "user_gps_section.h"
#include "hal_trace.h"
#include "string.h"

#define BUF_LEN 1024
static uint8_t test_write_buf[BUF_LEN] = {0};
static uint8_t test_read_buf[BUF_LEN] = {0};

void test_user_gps_to_flash(void)
{
    user_gps_to_flash_init();

    for (uint16_t i=0; i<BUF_LEN; i++) {
        test_write_buf[i] = i % 0xFF;
    }

    user_gps_erase_section();
    for (uint8_t i=0; i<60; i++) {
        user_gps_write(test_write_buf, BUF_LEN, i*BUF_LEN);

        memset(test_read_buf, 0, BUF_LEN);
        user_gps_read(test_read_buf, BUF_LEN, i*BUF_LEN);
        if (memcmp(test_write_buf, test_read_buf, BUF_LEN) != 0) {
            TRACE(0, "gps data check fail! cnt:%d", i);
        }
    }
    TRACE(0, "gps data check done\n");
}


#endif

