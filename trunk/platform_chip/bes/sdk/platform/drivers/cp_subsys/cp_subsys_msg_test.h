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
#ifndef __CP_SUBSYS_MSG_TEST_H__
#define __CP_SUBSYS_MSG_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

void cp_subsys_msg_release(const void *data, unsigned int len);

int cp_subsys_msg_test_send(const void *data, unsigned int len);

void cp_subsys_get_msg_test_data(const void *data, unsigned int len, const void **payload, unsigned int *payload_len);

#ifdef __cplusplus
}
#endif

#endif

