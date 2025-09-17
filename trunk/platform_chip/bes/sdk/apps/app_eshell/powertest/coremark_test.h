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

#ifndef __COREMARK_TEST_H__
#define __COREMARK_TEST_H__

int coremark_main(void);
uint32_t get_coremark_start_info(void);
uint32_t get_wfi_start_info(void);
uint16_t get_sys_freq(void);

#endif //__COREMARK_TEST_H__
