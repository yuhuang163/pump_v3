/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#ifndef __OTA_MULTI_IMAGE_OTA_UPGRADE__
#define __OTA_MULTI_IMAGE_OTA_UPGRADE__
#include "bth_if_cmd_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  OP_CODE_WRITE  0XAABBCCDD
#define  OP_CODE_READ   0X00112233
#define  OP_CODE_ERASE  0X55667788



typedef struct{
    uint32_t op_code;
    uint32_t address;
    uint32_t ilength;
}PARAM_M55_FLASH_OP;

void ota_write_m55_flash_done_handle(void);
int ota_read_m55_flash_done(uint8_t *p_buff , uint32_t ilength);

void ota_multi_image_upgrade_init(void);
int ota_multi_image_m55_flash_write(uint32_t address , uint8_t *p_buff , uint32_t ilength);
int ota_multi_image_m55_flash_read(uint32_t address , uint32_t ilength , uint8_t *p_buff);
int ota_multi_image_m55_flash_erase(uint32_t address , uint32_t ilength);

#ifdef __cplusplus
}
#endif
#endif

