/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __BTS_A2DP_SOURCE_H__
#define __BTS_A2DP_SOURCE_H__

#include "bts_defines.h"
#include "bts_status.h"
#include "bts_device.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void bts_a2dp_source_t;

typedef enum bts_a2dp_source_event {
    BTS_A2DP_SOURCE_EVENT_SEARCH_RESULT = 0,
    BTS_A2DP_SOURCE_EVENT_SEARCH_COMPLETE,
    BTS_A2DP_SOURCE_EVENT_CONNECT_FAIL,
    BTS_A2DP_SOURCE_EVENT_STREAM_OPEN,
    BTS_A2DP_SOURCE_EVENT_STREAM_CLOSE,
} bts_a2dp_source_event_t;

typedef struct bts_a2dp_source_event_param {
    bts_a2dp_source_t *a2dp_source;
    union {
        struct {
            bts_dev_addr_t *addr;
            const char *name;
            int name_len;
        } search_result;
        struct {
        } search_complete;
        struct {
            bts_dev_addr_t *addr;
        } connect_fail;
        struct {
            bts_dev_addr_t *addr;
        } stream_open;
        struct {
            bts_dev_addr_t *addr;
        } stream_close;
    } p;
} bts_a2dp_source_event_param_t;

typedef bts_status_t (*bts_a2dp_source_callback_t)(bts_a2dp_source_event_t event, bts_a2dp_source_event_param_t *param);

bts_status_t bts_a2dp_source_init(void);
bts_status_t bts_a2dp_source_register_callback(bts_a2dp_source_callback_t cb);

bts_status_t bts_a2dp_source_search(void);
bts_status_t bts_a2dp_source_connect(bts_dev_addr_t *addr);
bts_status_t bts_a2dp_source_disconnect(bts_a2dp_source_t *a2dp_source);
bts_status_t bts_a2dp_source_start_stream(bts_a2dp_source_t *a2dp_source);
bts_status_t bts_a2dp_source_write_pcm(bts_a2dp_source_t *a2dp_source, unsigned char *pcm, unsigned int pcm_len);

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_A2DP_SOURCE_H__ */