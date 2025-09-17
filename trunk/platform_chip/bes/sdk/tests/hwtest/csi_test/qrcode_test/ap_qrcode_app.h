/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __AP_QRCODE_APP_H__
#define __AP_QRCODE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define QRCODE_RESULT_DATA_LEN_MAX      (2953)
#define QRCODE_RPC_MSG_DATA_LEN_MAX     (16)

enum qrcode_rpc_msg_type
{
    QRCODE_OPEN_M33_TO_A7 = 0x20,
    QRCODE_OPEN_A7_TO_M33,
    QRCODE_CLOSE_M33_TO_A7,
    QRCODE_CLOSE_A7_TO_M33,
    QRCODE_START_M33_TO_A7,
    QRCODE_START_A7_TO_M33,
    QRCODE_STOP_M33_TO_A7,
    QRCODE_STOP_A7_TO_M33,
};

struct qrcode_rpc_message {
    uint8_t     msg_type;
    uint8_t     sync;
    uint16_t     data_len;
    union {
        uint8_t     data[QRCODE_RPC_MSG_DATA_LEN_MAX];
        uint32_t    param[QRCODE_RPC_MSG_DATA_LEN_MAX/4];
    }u;
};

void ap_qrcode_rpc_rx_handler(void *data, int size);
int ap_qrcode_task_init(void);
int ap_qrcode_task_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
