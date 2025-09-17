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

#ifndef __BTH_IF_THREAD_SERVER_H__
#define __BTH_IF_THREAD_SERVER_H__
#include "bth_if_core.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t bth_if_send_result_to_app(app_bth_if_user_t user, uint8_t *buf, uint32_t len, uint32_t sub_cmd);
int32_t bth_if_thread_server_register(app_bth_if_user_t user, const app_bth_if_ipc_msg_t* bth_if_handle_table);
int32_t bth_if_thread_server_init(void);

#ifdef __cplusplus
}
#endif
#endif //end of __BTH_IF_THREAD_SERVER_H__