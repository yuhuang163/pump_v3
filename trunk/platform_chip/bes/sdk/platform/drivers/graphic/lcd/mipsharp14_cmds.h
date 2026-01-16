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

#ifndef __MIPSHARP12_CMDS_H__
#define __MIPSHARP12_CMDS_H__

#define MIP_FREQ_KHZ    1200
#define MIP_ROW_CNT     (LCD_HEIGHT + 4)
#define MIP_COL_CNT     (LCD_WIDTH)

static struct lcd_command lcd_init_cmds[] = {
    { 0xFF, 64,
      {
          (MIP_FREQ_KHZ & 0xff), ((MIP_FREQ_KHZ >> 8) & 0xff),
          (MIP_COL_CNT & 0xff),  ((MIP_COL_CNT >> 8) & 0xff),
          (MIP_ROW_CNT & 0xff),  ((MIP_ROW_CNT >> 8) & 0xff),
      },
     0},
};

#endif /* __MIPSHARP12_CMDS_H__ */
