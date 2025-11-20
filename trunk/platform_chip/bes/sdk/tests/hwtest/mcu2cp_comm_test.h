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
#ifndef __MCU2CP_COMM_TEST_H__
#define __MCU2CP_COMM_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

void cp_subsys_data_process_handle(void);
unsigned int mcu2cp_msg_test_rx_handler(const void *data, unsigned int len);
void mcu2cp_msg_test_tx_done_handler(const void *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
