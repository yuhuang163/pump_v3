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
#ifdef NOAPP

#include "plat_types.h"
#include "cmsis.h"
#include "cp_subsys.h"
#include "cp_subsys_msg.h"
#include "cp_subsys_msg_test.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "string.h"

SYNC_FLAGS_LOC
static struct CP_SUBSYS_MSG_T cp_msg[10];
static bool cp_msg_used[ARRAY_SIZE(cp_msg)];

int cp_subsys_msg_send(unsigned short id, unsigned short reply, const void *data, unsigned int len)
{
    uint32_t lock;
    int i;

    lock = int_lock();
    for (i = 0; i < ARRAY_SIZE(cp_msg); i++) {
        if (!cp_msg_used[i]) {
            break;
        }
    }
    ASSERT(i < ARRAY_SIZE(cp_msg), "%s: Failed to find empty cp_msg index", __func__);
    cp_msg_used[i] = true;
    int_unlock(lock);

    memset(&cp_msg[i], 0, sizeof(cp_msg[i]));
    cp_msg[i].hdr.id = id;
    cp_msg[i].hdr.reply = !!reply;
    cp_msg[i].seq = i;
    cp_msg[i].param[0] = (uint32_t)data;
    cp_msg[i].param[1] = len;
    return cp_subsys_send(&cp_msg[i], sizeof(cp_msg[i]));
}

void cp_subsys_msg_release(const void *data, unsigned int len)
{
    const struct CP_SUBSYS_MSG_T *msg = (const struct CP_SUBSYS_MSG_T *)data;
    int i;

    if (&cp_msg[0] <= msg && msg < &cp_msg[ARRAY_SIZE(cp_msg)]) {
        for (i = 0; i < ARRAY_SIZE(cp_msg); i++) {
            if (msg == &cp_msg[i]) {
                break;
            }
        }
        ASSERT(i < ARRAY_SIZE(cp_msg), "%s: Failed to find cp_msg index: msg=%p", __func__, msg);
        ASSERT(i == msg->seq, "%s: Bad msg seq %u (expecting %u)", __func__, msg->seq, i);
        cp_msg_used[i] = false;
    }
}

int cp_subsys_msg_test_send(const void *data, unsigned int len)
{
    return cp_subsys_msg_send(CP_SUBSYS_MSG_ID_TEST, 0, data, len);
}

void cp_subsys_get_msg_test_data(const void *data, unsigned int len, const void **payload, unsigned int *payload_len)
{
    const struct CP_SUBSYS_MSG_T *msg = (const struct CP_SUBSYS_MSG_T *)data;

    if (len == sizeof(*msg) && msg->hdr.id == CP_SUBSYS_MSG_ID_TEST && !msg->hdr.reply) {
        *payload = (const void *)msg->param[0];
        *payload_len = msg->param[1];
    } else {
        *payload = NULL;
        *payload_len = 0;
    }
}

#endif
