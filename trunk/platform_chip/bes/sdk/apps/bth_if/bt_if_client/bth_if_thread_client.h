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

#ifndef __BTH_IF_THREAD_CLIENT_H__
#define __BTH_IF_THREAD_CLIENT_H__
#include "plat_types.h"
#include "bth_if_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bth_server_state_monitor_cb)(uint8_t alive_status);

int32_t bth_if_thread_client_open(app_bth_if_user_t user, uint32_t callback, uint8_t comm_type);
int32_t bth_if_thread_client_close(app_bth_if_user_t user);
int32_t bth_if_thread_client_reset(app_bth_if_user_t user);
int32_t bth_if_thread_client_ctrl(app_bth_if_user_t user, uint32_t ctrl_cmd, uint8_t *buf, uint32_t payload_len);

int32_t app_bth_if_thread_client_result_register(app_bth_if_user_t user, bth_if_user_rsp_result_callback_t func);
int32_t app_bth_if_client_init(bth_server_state_monitor_cb* server_state_cb);
void app_bth_server_ready_notify(uint8_t ready_flag);
#ifdef __cplusplus
}
#endif
#endif //end of __BTH_IF_THREAD_CLIENT_H__

