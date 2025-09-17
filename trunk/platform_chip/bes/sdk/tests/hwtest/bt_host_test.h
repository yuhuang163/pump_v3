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
#ifndef __BT_HOST_TEST_H__
#define __BT_HOST_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

enum BTH_RX_DATA_HDLR_ID_T {
    BTH_RX_DATA_HDLR_MT,
    BTH_RX_DATA_HDLR_BT,

    BTH_RX_DATA_HDLR_QTY
};

typedef unsigned int (*BTH_RX_DATA_HDLR_T)(const void *data, unsigned int len);

void bt_host_setup_rx_data_handler(enum BTH_RX_DATA_HDLR_ID_T id, BTH_RX_DATA_HDLR_T handler);

void bt_host_test(void);

#ifdef __cplusplus
}
#endif

#endif
