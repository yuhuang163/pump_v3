/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prtransportr written
 * permisstransportn of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary informattransportn of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#ifndef __TRANSPORT_IF_H__
#define __TRANSPORT_IF_H__

#if defined(__cplusplus)
extern "C" {
#endif

struct transport_if;

typedef enum transport_if_event {
    TRANSPORT_IF_EVENT_DATA_INDICATE = 0,
} transport_if_event_t;

typedef struct transport_if_event_param {
    struct transport_if *transport_if;
    union {
        struct {
            int data_len;
            unsigned char *data;
        } data_indicate;
    } p;
} transport_if_event_param_t;

typedef int (*transport_if_callback_t)(transport_if_event_t event, transport_if_event_param_t *param);

typedef struct transport_if {
    int (*init)(void);
    int (*register_callback)(transport_if_callback_t cb);
    int (*send)(unsigned char *data, int data_len);
} transport_if_t;

#if defined(__cplusplus)
}
#endif

#endif /* __TRANSPORT_IF_H__ */
