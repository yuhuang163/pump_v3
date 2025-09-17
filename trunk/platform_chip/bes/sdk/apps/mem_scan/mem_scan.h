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
#ifndef __MEM_SCAN_H__
#define __MEM_SCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MEM_SCAN_INVALID,
    MEM_SCAN_SUCCESS,
    MEM_SCAN_FAILED,
} mscan_result_t;

void mem_scan_start(void);

mscan_result_t mem_scan_check(void);

void mem_scan_check_and_start(void);

#ifdef __cplusplus
}
#endif

#endif /* __MEM_SCAN_H__ */
