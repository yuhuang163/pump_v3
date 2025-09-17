/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef BT_TESTER_EXT_H
#define BT_TESTER_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_tester.h"

#define bt_CMD_HELP_INFO                                                   "\n\
        help - an interface to the bt test command manual                   \n\
EXAMPLES                                                                    \n\
        bt,help                                                             \n\
        bt,HCI_RESET                                                        \n\
        bt,WR_BTADDR,1234567890ab                                           \n\
        bt,NOSIG_EDR_RX,0,2-DH1,PRBS9,0,555555555555                        \n\
        bt,BLE_RX_TEST_V2,0,LE_1M,STANDARD                                  \n\
        bt,RD_PSCAN_ACT                                                     \n\
        bt,BT_CARRIER_START,0                                               \n\
        bt,SIG_MODE                                                         \n\
        bt,BT_SCAN_CONTROL,3                                              \n\n\
"

/**
 * \brief bt test function
 * \param [in]  params  parameters received as strings from uart port
 */
void bt_tester(const char **params);

#ifdef __cplusplus
}
#endif

#endif /* BT_TESTER_EXT_H */