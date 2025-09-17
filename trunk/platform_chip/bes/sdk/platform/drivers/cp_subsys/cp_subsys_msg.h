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
#ifndef __CP_SUBSYS_MSG_H__
#define __CP_SUBSYS_MSG_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum CP_SUBSYS_MSG_ID_T {
    CP_SUBSYS_MSG_ID_SET_FREQ,
    CP_SUBSYS_MSG_ID_DEMO_PROCESS_START,
    CP_SUBSYS_MSG_ID_DEMO_PROCESS_STOP,
    CP_SUBSYS_MSG_ID_DEMO_PROCESS_RET,
    CP_SUBSYS_MSG_ID_TEST,
};

struct CP_SUBSYS_MSG_HDR_T {
    uint16_t id     : 15;
    uint16_t reply  : 1;
};

struct CP_SUBSYS_MSG_T {
    struct CP_SUBSYS_MSG_HDR_T hdr;
    uint16_t seq;
    uint32_t param[3];
};

#define CP_SUBSYS_PROC_BUF_MAX_SIZE     (32)
typedef struct {
    uint32_t    in_buf[CP_SUBSYS_PROC_BUF_MAX_SIZE];
    uint32_t    in_size;
    uint32_t    out_buf[CP_SUBSYS_PROC_BUF_MAX_SIZE];
    uint32_t    out_size;
} cp_subsys_proc_buf_t;

#ifdef __cplusplus
}
#endif

#endif

