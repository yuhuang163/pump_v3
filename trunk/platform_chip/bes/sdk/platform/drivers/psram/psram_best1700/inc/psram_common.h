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
#ifndef __PSRAM_COMMON_H__
#define __PSRAM_COMMON_H__

#include "hal_psram.h"
#include "hal_psramip.h"

uint8_t psram_check_wr_valid(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c);

int find_the_largest_window_of_1d_array(bool *array, uint8_t volume);

int find_the_largest_window_of_2d_array(void *array, uint8_t row_column_num, int byte_pos, struct HAL_PSRAM_RECTANGLE_T *rectangle);


#endif /* __PSRAM_COMMON_H__ */
