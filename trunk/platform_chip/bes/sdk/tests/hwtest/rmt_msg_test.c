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
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rmt_msg_test.h"
#include "string.h"

#define MAX_TX_CNT                          16

static uint8_t tx_buf[MAX_TX_CNT][8];
static uint32_t tx_word = 0xabcdef12;
static uint32_t tx_cnt;
static RMT_MSG_TEST_SEND_MSG_CB_T send_msg_cb;
static RMT_MSG_TEST_GET_MSG_DATA_CB_T get_msg_data_cb;
static bool client_role;

unsigned int rmt_msg_test_rx_handler(const void *data, unsigned int len)
{
    int ret;
    uint32_t tx_len;
    const uint8_t *rdata = NULL;
    uint32_t rlen;
    bool raw = false;
    bool tx_en = true;
    char str_buf[120];
    uint32_t buf_len;
    uint32_t i;

    if (get_msg_data_cb) {
        get_msg_data_cb(data, len, (const void **)&rdata, &rlen);
    }
    if (rdata == NULL) {
        rdata = data;
        rlen = len;
        raw = true;
    }
    buf_len = 0;
    for (i = 0; i < rlen && buf_len < sizeof(str_buf); i++) {
        buf_len += format_string(&str_buf[buf_len], sizeof(str_buf) - buf_len, "%02X ", rdata[i]);
    }
    str_buf[sizeof(str_buf) - 1] = 0;
    TR_INFO(0, "[%2u] RX=%u %s: %s", tx_cnt, len, raw ? "RAW" : "PAYLOAD", str_buf);

    if (client_role) {
        if (rlen > sizeof(tx_buf[0])) {
            tx_len = sizeof(tx_buf[0]);
        } else {
            tx_len = rlen;
        }

        for (i = 0; i < tx_len; i++) {
            tx_buf[tx_cnt][i] = rdata[i] + 1;
        }
    } else {
        if (tx_cnt < MAX_TX_CNT) {
            tx_len = sizeof(tx_word);
            if (tx_len > sizeof(tx_buf[0])) {
                tx_len = sizeof(tx_buf[0]);
            }
            memcpy(tx_buf[tx_cnt], &tx_word, tx_len);

            tx_word += (1 << (8 * (tx_cnt % 4)));
        } else {
            tx_len = 0;
            tx_en = false;
        }
    }

    if (tx_en) {
        hal_sys_timer_delay_us(500);
        ASSERT(send_msg_cb, "%s: rmt msg test not init yet?", __func__);
        ret = send_msg_cb(tx_buf[tx_cnt], tx_len);
        ASSERT(ret == 0, "rmt send msg failed: %d", ret);
    }

    if (tx_cnt < MAX_TX_CNT) {
        tx_cnt++;
    }

    return len;
}

void rmt_msg_test_tx_handler(const void *data, unsigned int len)
{
    const uint8_t *rdata = NULL;
    uint32_t rlen;
    char str_buf[120];
    uint32_t buf_len;
    uint32_t i;
    uint32_t last_tx_cnt = MAX_TX_CNT;
    bool raw = false;

    for (i = 0; i < MAX_TX_CNT; i++) {
        if (data == tx_buf[i]) {
            last_tx_cnt = i;
            break;
        }
    }

    if (get_msg_data_cb) {
        get_msg_data_cb(data, len, (const void **)&rdata, &rlen);
    }
    if (rdata == NULL) {
        rdata = data;
        rlen = len;
        raw = true;
    }
    buf_len = 0;
    for (i = 0; i < rlen && buf_len < sizeof(str_buf); i++) {
        buf_len += format_string(&str_buf[buf_len], sizeof(str_buf) - buf_len, "%02X ", rdata[i]);
    }
    str_buf[sizeof(str_buf) - 1] = 0;
    TR_INFO(0, "[%2u] TX=%u %s: %s", last_tx_cnt, len, raw ? "RAW" : "PAYLOAD", str_buf);
}

void rmt_msg_test(int is_client, RMT_MSG_TEST_SEND_MSG_CB_T send_msg, RMT_MSG_TEST_GET_MSG_DATA_CB_T get_msg_data)
{
    TR_INFO(0, "%s: %s", __func__, is_client ? "Client" : "Server");

    ASSERT(send_msg, "%s: send msg callback must exist", __func__);

    client_role = !!is_client;
    send_msg_cb = send_msg;
    get_msg_data_cb = get_msg_data;

    if (client_role) {
        int ret;
        uint32_t tx_len;
        const char data[] = "gogo";

        tx_len = sizeof(data) - 1;
        if (tx_len > ARRAY_SIZE(tx_buf)) {
            tx_len = ARRAY_SIZE(tx_buf);
        }
        memcpy(tx_buf, data, tx_len);
        ret = send_msg_cb(tx_buf, tx_len);
        ASSERT(ret == 0, "rmt send msg failed: %d", ret);
        TR_INFO(0, "%s: Send first msg", __func__);
    }
}
