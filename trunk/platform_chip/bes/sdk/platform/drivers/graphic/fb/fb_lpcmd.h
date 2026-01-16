/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#ifndef __FB_LPCMD_H__
#define __FB_LPCMD_H__

#ifdef cpluscplus
extern "C" {
#endif
#include <stdint.h>

#define LPCMDS_MAX_PARAMS_NUM 4
#define LPCMDS_MAX_CMDS_NUM 20
#define LPCMDS_MAX_GROUP_NUM 128

#define LPCMDS_NONBLOCK 1
#define LPCMDS_BLOCK    2
#define LPCMDS_FLUSH    4

enum lpcmd_type_e
{
    LPCMD_TYPE_WRITE,
    LPCMD_TYPE_READ,
    LPCMD_TYPE_DELAY,
};

typedef struct lpcmd_node_s
{
  uint8_t cmd;
  uint8_t count;
  uint8_t optype;
  uint8_t params[LPCMDS_MAX_PARAMS_NUM];
} lpcmd_node_t;

/**
 * @brief lpcmds_register_cmds - register cmds to cache, will send at framedone irq
 *
 * @param cmds reference struct lpcmd_node_s *
 * @param nums count
 * @return int success 0; failed -1;
 */
int lpcmds_register_cmds(void *cmds, int nums, int flag);

/**
 * @brief lpcmds_flush - send all cmds at cache
 *
 * @return int - success 0; failed -1;
 */
int lpcmds_flush(void);

/**
 * @brief lpcmds_clear_buffer - clear lpcmds buffer
 *
 */
void lpcmds_clear_buffer(void);

#ifdef cplusplus
}
#endif

#endif /* __FB_LPCMD_H__ */
