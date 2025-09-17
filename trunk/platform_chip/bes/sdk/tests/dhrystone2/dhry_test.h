/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#ifndef __DHRY_TEST_H__
#define __DHRY_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

void dhrystone_main(void);

void dhrystone_test(void);

unsigned dhrystone_get_freq_factor(void);

void dhrystone_event_begin(void);

void dhrystone_event_end(void);

#ifdef __cplusplus
}
#endif

#endif
