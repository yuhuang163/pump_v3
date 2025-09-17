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
#ifndef __BTS_HFP_HF_H__
#define __BTS_HFP_HF_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef void bts_hfp_hf_t;

typedef enum bts_hfp_hf_event {
    BTS_HFP_HF_EVENT_SERVICE_CONNECTED = 0,
    BTS_HFP_HF_EVENT_SERVICE_DISCONNECTED,
    BTS_HFP_HF_EVENT_RING_IND,
    BTS_HFP_HF_EVENT_CALLER_ID_IND,
    BTS_HFP_HF_EVENT_AUDIO_CONNECTED,
    BTS_HFP_HF_EVENT_AUDIO_DISCONNECTED,
} bts_hfp_hf_event_t;

typedef struct bts_hfp_hf_event_param {
    bts_hfp_hf_t *hfp_hf;
    union {
        struct {
            bts_dev_addr_t *addr;
        } service_connected;
        struct {
        } service_disconnect;
        struct {
            const char *number;
        } ring_ind;
        struct {
            int number_len;
            const char *number;
        } caller_id_ind;
        struct {
        } audio_connected;
        struct {
        } audio_disconnected;
    } p;
} bts_hfp_hf_event_param_t;

typedef bts_status_t (*bts_hfp_hf_callback_t)(bts_hfp_hf_event_t event, bts_hfp_hf_event_param_t *param);
bts_status_t bts_hfp_hf_init(void);
bts_status_t bts_hfp_hf_register_callback(bts_hfp_hf_callback_t cb);

#if defined(__cplusplus)
}
#endif

#endif /* __BTS_HFP_HF_H__ */