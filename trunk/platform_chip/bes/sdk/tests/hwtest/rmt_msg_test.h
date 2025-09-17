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
#ifndef __RMT_MSG_TEST_H__
#define __RMT_MSG_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*RMT_MSG_TEST_SEND_MSG_CB_T)(const void *data, unsigned int len);

typedef void (*RMT_MSG_TEST_GET_MSG_DATA_CB_T)(const void *data, unsigned int len, const void **payload, unsigned int *payload_len);

unsigned int rmt_msg_test_rx_handler(const void *data, unsigned int len);

void rmt_msg_test_tx_handler(const void *data, unsigned int len);

void rmt_msg_test(int is_client, RMT_MSG_TEST_SEND_MSG_CB_T send_msg, RMT_MSG_TEST_GET_MSG_DATA_CB_T get_msg_data);

#ifdef __cplusplus
}
#endif

#endif
