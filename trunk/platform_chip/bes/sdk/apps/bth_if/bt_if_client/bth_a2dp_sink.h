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

#ifndef __BTH_A2DP_SINK_H__
#define __BTH_A2DP_SINK_H__
#include "plat_types.h"
#include "bth_if_cmd_def.h"

#ifdef __cplusplus
extern "C" {
#endif

enum A2DP_SINK_MSG_TYPE {
    A2DP_SINK_MSG_START,
    A2DP_SINK_MSG_VOL_SET,
    A2DP_SINK_MSG_STOP,
    A2DP_SINK_MSG_NUM,
};


typedef struct{
    enum A2DP_SINK_MSG_TYPE msg_type;
    app_bt_a2dp_sink_param_t param;
}a2dp_sink_msg_t;

typedef int (*a2dp_sink_send_msg_callback)(a2dp_sink_msg_t*);

void app_bth_a2dp_sink_msg_send_cb_register(a2dp_sink_send_msg_callback func);

int a2dp_sink_get_packet_data(uint8_t *pdata, uint32_t len);

uint32_t a2dp_sink_get_fifo_len(void);

void a2dp_sink_player_init(void);

#ifdef __cplusplus
}
#endif
#endif //end of __BTH_A2DP_SINK_H__

