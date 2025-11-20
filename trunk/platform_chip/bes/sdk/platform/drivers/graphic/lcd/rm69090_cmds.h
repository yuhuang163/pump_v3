/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifndef __RM69090_CMDS_H__
#define __RM69090_CMDS_H__

static struct lcd_command lcd_init_cmds[] = {
    {0xFE, 1, {0x00}, 0},
    {0x35, 1, {0x00}, 0},
    {0x51, 1, {0xCF}, 0},
    {0x2A, 4, {0x00, 0x0E, 0x1, 0xD3}, 0},
    {0x2B, 4, {0x00, 0x00, 0x1, 0xC5}, 0},
    {0x11, 0, {0x00}, 240},
    {0x29, 0, {0x00}, 150},
};

#endif /* __RM69090_CMDS_H__ */
