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
#if (CHIP_PSRAM_CTRL_VER == 6)

#include "hal_psram.h"
#include "hal_psramip.h"
#include "psram_timing.h"
#include "psram_debug.h"

uint8_t psram_check_wr_valid(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c)
{
    uint32_t i;
    uint32_t val, val0, val1;
    uint32_t val2, val3;
    volatile uint32_t *psram_base,*psram_base1;

    if (HAL_PSRAM_ID_0 == id) {
        psram_base  = (volatile uint32_t *)PSRAM_NC_BASE;
        psram_base1 = (volatile uint32_t *)(PSRAM_NC_BASE + 0x100000);
    } else {
        /// TODO: Assert
    }

    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        *(psram_base + i) = val0;
        *(psram_base1 + i) = val1;
    }
    __DSB();
    hal_psram_mc_wait_write_buffer_empty(id);
    hal_psram_mc_wb_busy_wait(id);
    hal_psram_mc_busy_wait(id);
    for (i = 0; i < 0x100; ++i) {
        val = i & 0xFF;
        val = val | (val << 8) | (val << 16) | (val << 24);
        val0 = (val & 0x00FF00FF) | ((~val) & 0xFF00FF00);
        val1 = (val & 0xFF0000FF) | ((~val) & 0x00FFFF00);
        val2 = *(psram_base + i);
        val3 = *(psram_base1 + i);
        if (val2 != val0) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val2, val0);
            return 0;
        }
        if (val3 != val1) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val3, val0);
            return 0;
        }
    }
    return 1;
}

static uint8_t find_largest_extension_square_of_point(void *array, uint8_t column_row_num, uint8_t x, uint8_t y, int byte_pos)
{
    uint8_t (*p)[PSRAM_WINDOW_SIZE] = (uint8_t (*)[PSRAM_WINDOW_SIZE])array;
    uint8_t range = 0;

    while (1) {
        for (int i = 0; i <= range; i++) {
            if (((p[x + range][y + i] & (1 << byte_pos)) == 0) || ((p[x + i][y + range] & (1 << byte_pos)) == 0)) {
               return (range == 0 ? 0 : range - 1);
            }
        }
        range++;
        if ((x + range >= column_row_num) || (y + range >= column_row_num)) {
            return range - 1;
        }
    }
}

int find_the_largest_window_of_2d_array(void *array, uint8_t row_column_num, int byte_pos, struct HAL_PSRAM_RECTANGLE_T *rectangle)
{
    uint8_t (*p)[PSRAM_WINDOW_SIZE] = (uint8_t (*)[PSRAM_WINDOW_SIZE])array;
    uint8_t square_len = 0, max_square_len = 0;
    uint8_t row_extend = 0, column_extend = 0;
    uint8_t width = 0, max_width = 0, hight = 0, max_hight = 0;
    int i, j, column, row;
    int ret = 0;

    rectangle->x_start = 0;
    rectangle->y_start = 0;
    rectangle->x_end = 0;
    rectangle->y_end = 0;
    for(i = 0; i < row_column_num; i++) {
        for (j = 0; j < row_column_num; j++) {
            if ((p[i][j] & (1 << byte_pos)) == 0) {
                continue;
            }
            square_len = find_largest_extension_square_of_point(array, row_column_num, i, j, byte_pos);
            if (max_square_len <= square_len) {
                max_square_len = square_len;
                ///> row extend
                for(row_extend = i + square_len + 1; row_extend < row_column_num; row_extend++) {
                    for(column = j; column <= j + square_len; column++) {
                        if ((p[row_extend][column] & (1 << byte_pos)) == 0)
                            break;
                    }
                    if (column <= j + square_len)
                        break;
                }
                ///> column extend
                for(column_extend = j + square_len + 1; column_extend < row_column_num; column_extend++) {
                    for(row = i; row <= i + square_len; row++) {
                        if ((p[row][column_extend] & (1 << byte_pos)) == 0)
                            break;
                    }
                    if (row <= i + square_len)
                        break;
                }
                hight = row_extend - i - 1;
                width = column_extend - j - 1;
                if (hight > width) {
                    width = square_len;
                } else {
                    hight = square_len;
                }
                if ((MIN(width, hight) > (MIN(max_width, max_hight))) ||
                        ((MIN(width, hight) == (MIN(max_width, max_hight))) && (MAX(width, hight) > (MAX(max_width, max_hight))))) {
                    rectangle->y_start = i;
                    rectangle->x_start = j;
                    rectangle->y_end = i + hight;
                    rectangle->x_end = j + width;
                    max_width = width;
                    max_hight = hight;
                    ret = square_len;
                }
            }
        }
    }
    return ret;
}

int find_the_largest_window_of_1d_array(bool *array, uint8_t volume)
{
    uint8_t val_cnt = 0;
    uint8_t win_l = 0, win_r = 0;
    uint32_t ret;

    for (int i = 1;i <= volume;i++) {
        if (array[i] == 1 && array[i - 1] == 1) {
            val_cnt++;
            if (i == volume && val_cnt > (win_r - win_l)) {
                win_r = i;
                win_l = i - val_cnt;
                val_cnt = 0;
            }
        } else if (val_cnt > (win_r - win_l)) {
            win_r = i - 1;
            win_l = i - 1 - val_cnt;
            val_cnt = 0;
        } else {
            val_cnt = 0;
        }
    }

    ret = (win_r & 0xFF) | ((win_l & 0xFF) << 8);
    return ret;
}

#endif

