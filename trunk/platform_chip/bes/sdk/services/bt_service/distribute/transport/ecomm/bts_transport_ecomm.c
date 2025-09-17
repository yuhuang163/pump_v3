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
#include <stdio.h>
#include <stdlib.h>

#include "bt_service.h"
#include "transport_if.h"

#include "ecomm.h"

static transport_if_callback_t g_transport_if_callback = NULL;

// transport related
#define ECOMM_CHANNEL    (3)

#define ECOMM_CHANNEL_TX_PACK_NUM (6)
#define ECOMM_CHANNEL_RX_PACK_NUM (6)

#define ECOMM_CHANNEL_TX_PACK_SIZ (256)
#define ECOMM_CHANNEL_RX_PACK_SIZ (256)

static unsigned char channel_tx_buff[ECOMM_CHANNEL_TX_PACK_NUM*(ECOMM_CHANNEL_TX_PACK_SIZ+ECOMM_CFG_SEND_BUFF_RESV_SIZE)];
static ecomm_pack_t channel_tx_packs[ECOMM_CHANNEL_TX_PACK_NUM];

static unsigned char channel_rx_buff[ECOMM_CHANNEL_RX_PACK_NUM*(ECOMM_CHANNEL_RX_PACK_SIZ+ECOMM_CFG_RECV_BUFF_RESV_SIZE)];
static ecomm_pack_t channel_rx_packs[ECOMM_CHANNEL_RX_PACK_NUM];

#define CASE_ENUM(e) case e: return "["#e"]"; break;

static ecomm_t *g_ecomm = NULL;

static void _process_rx_pack(ecomm_t *ecomm, ecomm_pack_t *pack)
{
    transport_if_event_param_t param;

    param.p.data_indicate.data     = pack->data;
    param.p.data_indicate.data_len = pack->data_len;

    if (g_transport_if_callback != NULL) {
        g_transport_if_callback(TRANSPORT_IF_EVENT_DATA_INDICATE, &param);
    }

    ecomm_free_pack(ecomm, pack, ECOMM_PACK_RECV);
}

static void _ecomm_channel_callback(ecomm_channel_event_t event, ecomm_channel_event_param_t *param)
{
    ecomm_pack_t *pack = NULL;
    ecomm_t *ecomm = param->ecomm;

    (void)pack;
    (void)ecomm;

    switch (event) {
        case ECOMM_CHANNEL_EVENT_FLOW_CONTROL_UPDATE:
        {
        }
        break;
        case ECOMM_CHANNEL_EVENT_SEND_DONE:
        {
            pack = param->p.send_done.pack;
            ecomm_free_pack(ecomm, pack, ECOMM_PACK_SEND);
        }
        break;
        case ECOMM_CHANNEL_EVENT_RECV_INDICATE:
        {
            pack = param->p.recv_indicate.pack;
            _process_rx_pack(ecomm, pack);
        }
        break;
        default:
        break;
    }
}

static int ecomm_setup(void)
{
    ecomm_t *ecomm = NULL;
    g_ecomm = ecomm_get_instance("bes-btwatch-slave-ecomm-1");
    ecomm = g_ecomm;

    ecomm_setup_pack(ecomm, ECOMM_CHANNEL, ECOMM_PACK_SEND, channel_tx_packs,
        channel_tx_buff, sizeof(channel_tx_buff), ECOMM_CHANNEL_TX_PACK_NUM);
    ecomm_setup_pack(ecomm, ECOMM_CHANNEL, ECOMM_PACK_RECV, channel_rx_packs,
        channel_rx_buff, sizeof(channel_rx_buff), ECOMM_CHANNEL_RX_PACK_NUM);

    ecomm_register_channel(ecomm, ECOMM_CHANNEL, _ecomm_channel_callback, NULL);

    return 0;
}

static int ecomm_send_data(unsigned char *data, unsigned int data_len)
{
    ecomm_t *ecomm = g_ecomm;
    ecomm_pack_t *pack = ecomm_alloc_pack(ecomm, ECOMM_CHANNEL, ECOMM_PACK_SEND);

    if (pack != NULL) {
        pack->data_len = data_len;
        memcpy(pack->data, data, data_len);

        ecomm_send(ecomm, pack);
    }

    return 0;
}

// API
static int _init(void)
{
    ecomm_setup();

    return 0;
}

static int _register_callback(transport_if_callback_t cb)
{
    g_transport_if_callback = cb;

    return 0;
}

static int _send(unsigned char *data, int data_len)
{
    return ecomm_send_data(data, data_len);
}

transport_if_t bts_transport_ecomm = {
    .init = _init,
    .send = _send,
    .register_callback = _register_callback,
};