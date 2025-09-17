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


#ifndef __BTH_OTA_RX_HANDLER__
#define __BTH_OTA_RX_HANDLER__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    MSG_OP_CODE,
    MSG_FLASH_DATA,
}FLASH_MSG_TYPE_T;

typedef struct
{
    uint8_t *cmd_buffer;
    uint16_t cmd_len;
} cmd_buffer_t;

typedef struct
{
    uint32_t evt;
    uint32_t arg;
    uint32_t arg1;
    uint32_t system_time;
    cmd_buffer_t cmd_buff;
}FLASH_BLOCK_T;


void app_bth_ota_thread_init(void);
void app_bth_ota_op_code_send(uint8_t* data , uint32_t ilen);
void app_bth_ota_data_send(uint8_t* data , uint32_t ilen);

#ifdef __cplusplus
}
#endif

#endif

