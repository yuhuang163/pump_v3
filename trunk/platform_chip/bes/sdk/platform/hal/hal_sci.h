/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __HAL_SCI_H__
#define __HAL_SCI_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"

#define T0_APDU_SW1SW2_LEN                   2
#define T0_APDU_COMMAND_HEADER_LEN           4
#define T0_APDU_COMMAND_HEADER_AND_LE_LEN    5
#define T0_APDU_COMMAND_HEADER_AND_LC_LEN    5
#define T0_APDU_COMMAND_HEADER_AND_LC_LE_LEN 6

#define T0_TPDU_COMMAND_HEADER_LEN           5

#define T0_TPDU_COMMAND_CLA_INDEX            0
#define T0_TPDU_COMMAND_INS_INDEX            1
#define T0_TPDU_COMMAND_P1_INDEX             2
#define T0_TPDU_COMMAND_P2_INDEX             3
#define T0_TPDU_COMMAND_P3_INDEX             4

int32_t hal_sci_open(void);
void hal_sci_reset_card(void);
uint32_t hal_sci_get_response(uint8_t *buf, uint32_t rxlen);
uint8_t hal_sci_send_command(uint8_t *buf, uint8_t txlen);

#ifdef __cplusplus
}
#endif

#endif
