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
#ifdef PSRAM_CTRL_ENABLE

#include "plat_types.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "reg_psram_mc_v6.h"
#include "reg_psram_phy_v6.h"
#include "string.h"
#include "hal_psram.h"
#include "psram_drv.h"

struct psram_package_info {
    enum PSRAM_DATA_BUS_T bus;
    enum PSRAM_TYPE_T type;
    // int die_code;
};

#ifdef PSRAM_POLLING
#if (defined(CHIP_BEST1502P) || defined(CHIP_BEST1306P) || defined(CHIP_BEST2007H))
static struct psram_package_info psram_polling_list[] = {
    {PSRAM_DATA_BUS_X8, PSRAM_TYPE_XOPI},
    {PSRAM_DATA_BUS_X8, PSRAM_TYPE_OPI},
    {PSRAM_DATA_BUS_X8, PSRAM_TYPE_HYPERRAM}
    };
#elif 0
    // add another chip's polling here
#else
    #error "chip do not support polling yet!!!"
#endif
#endif

void psram_mr_setting(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        psram_opi_mr_set(id, psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        psram_xopi_mr_set(id, psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM) {
        psram_hyper_mr_set(id, psram_cfg);
    }
}

void psram_mr_show(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        psram_opi_mr_show(id, psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        psram_xopi_mr_show(id, psram_cfg);
    } else { }
}

int psram_die_check(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    int ret;

    if (psram_cfg->device_func_cfg->psram_check_die == NULL) {
        PSRAM_TRACE(0, "WARINING: func psram_check_die is not registered yet");
        ret = 1;
    } else {
        hal_psram_phy_rx_timeout_enable(id);
        ret = psram_cfg->device_func_cfg->psram_check_die(id);
        hal_psram_phy_rx_timeout_disable(id);
    }

    return ret;
}

void SRAM_TEXT_LOC psram_mr_get(struct PSRAM_CFG_T *psram_cfg, uint8_t *reg, uint8_t *val, uint32_t clk)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        psram_xopi_mr_get(psram_cfg, reg, val, clk);
    } else { }
}

void psram_struct_init(struct PSRAM_CFG_T *psram_cfg)
{
    if (psram_cfg->psram_type == PSRAM_TYPE_OPI) {
        PSRAM_TRACE(0, "**PSRAM_TYPE_OPI");
        psram_opi_struct_init(psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_XOPI) {
        PSRAM_TRACE(0, "**PSRAM_TYPE_XOPI");
        psram_xopi_struct_init(psram_cfg);
    } else if (psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM) {
        PSRAM_TRACE(0, "**PSRAM_TYPE_HYPERBUS");
        psram_hyper_struct_init(psram_cfg);
    } else {
    }
}

int psram_type_bus_init(struct PSRAM_T *psram)
{
    uint32_t type, bus, phy_type;

    type = PSRAM_TYPE_XOPI;
    bus = PSRAM_DATA_BUS_X16;
    phy_type = PHY_P2S_BYPASS;

#ifdef PSRAM_POLLING
    static int index = 0;

    if (index < ARRAY_SIZE(psram_polling_list)) {
        type = psram_polling_list[index].type;
        bus = psram_polling_list[index].bus;
        index++;
    } else {
        return 1;
    }
#else
#if defined(PSRAM_CLASSIFY_XOPI)
    type = PSRAM_TYPE_XOPI;
#elif defined(PSRAM_CLASSIFY_HYPERBUS)
    type = PSRAM_TYPE_HYPERRAM;
#elif defined(PSRAM_CLASSIFY_OPI)
    type = PSRAM_TYPE_OPI;
#endif

#if defined(PSRAM_X8_MODE)
    bus = PSRAM_DATA_BUS_X8;
#elif defined(PSRAM_X16_MODE)
    bus = PSRAM_DATA_BUS_X16;
#elif defined(PSRAM_DUAL_8BIT)
    bus = PSRAM_DATA_BUS_2X8;
#elif defined(PSRAM_2X16_MODE)
    bus = PSRAM_DATA_BUS_2X16;
#endif
#endif

#if !defined(FPGA) && !defined(SIMU)
    phy_type = PHY_P2S_BYPASS;
#else
    phy_type = PHY_ANA_BYPASS;
#endif

    psram->psram_type[HAL_PSRAM_ID_0] = type;
    psram->psram_data_bus[HAL_PSRAM_ID_0] = bus;
    psram->phy_bypass_cfg[HAL_PSRAM_ID_0] = phy_type;
    PSRAM_TRACE(1, "psram cfg type = %d, data bus = %d, phy type = %d", type, bus, phy_type);

    return 0;
}

void psram_device_sleep(struct PSRAM_CFG_T *psram_cfg)
{
    enum HAL_PSRAM_ID_T id = HAL_PSRAM_ID_0;
    uint32_t val = 0;

    val = hal_psram_mc_mrs_pre_operation(id, psram_cfg);

    hal_psram_mc_set_cmd_pd(id);

    hal_psram_mc_mrs_post_operation(id, psram_cfg, val);
}

uint8_t psram_check_wr_valid(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c)
{
    uint32_t i;
    uint32_t val, val0, val1;
    uint32_t val2, val3;
    volatile uint32_t *psram_base,*psram_base1;
#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
    static const uint32_t mask_l = 0x00FF00FF;
    static const uint32_t mask_h = 0xFF00FF00;
    bool valid_l = true;
    bool valid_h = true;
#endif

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
#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
        if (valid_l && ((val2 & mask_l) != (val0 & mask_l))) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val2, val0, mask_l);
            valid_l = false;
        }
        if (valid_h && ((val2 & mask_h) != (val0 & mask_h))) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val2, val0, mask_h);
            valid_h = false;
        }
        if (!valid_l && !valid_h) {
            return 0;
        }
#else
        if (val2 != val0) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val2, val0);
            return 0;
        }
#endif

#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
        if (valid_l && ((val3 & mask_l) != (val1 & mask_l))) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val3, val1, mask_l);
            valid_l = false;
        }
        if (valid_h && ((val3 & mask_h) != (val1 & mask_h))) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x m=0x%08x", __FUNCTION__, i, val3, val1, mask_h);
            valid_h = false;
        }
        if (!valid_l && !valid_h) {
            return 0;
        }
#else
        if (val3 != val1) {
            PSRAM_TRACE(3, "%s:0: i=%d 0x%08x 0x%08x", __FUNCTION__, i, val3, val1);
            return 0;
        }
#endif
    }

#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
    uint8_t ret = 0;

    if (valid_l) {
        ret |= (1 << 0);
    }
    if (valid_h) {
        ret |= (1 << 1);
    }
    return ret;
#else
    return 1;
#endif
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

POSSIBLY_UNUSED static void psram_calib_on_1d(enum HAL_PSRAM_ID_T id, enum HAL_PSRAM_PHY_CHANNEL_T c, uint32_t clk)
{
    uint16_t volume;
    uint8_t windows_l = 0, windows_r = 0;
    uint32_t calib_val;
    uint32_t byte_pos;

    POSSIBLY_UNUSED uint16_t tx_ceb, tx_dqs, tx_clk, rx_dqs;
    uint8_t valid_square[PSRAM_WINDOW_SIZE];
    uint8_t valid_cnt[PSRAM_WINDOW_SIZE];

    memset(valid_square, 0, sizeof(valid_square));
    memset(valid_cnt, 0, sizeof(valid_cnt));

    for(rx_dqs = 0; rx_dqs <= 0xf; rx_dqs++) {
        hal_psram_anaphy_set_rxdqs_dly(rx_dqs);
#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
        hal_psram_anaphy1_set_rxdqs_dly(rx_dqs);
#endif
        __DSB();
        valid_square[rx_dqs] = psram_check_wr_valid(id, c);
    }

#if (defined(PSRAMPHY_DUAL_8BIT) && defined(PSRAM_X16_MODE)) || defined(PSRAM_DUAL_8BIT)
    static const int calib_num = 2;
#else
    static const int calib_num = 1;
#endif
    bool cali_valid[PSRAM_WINDOW_SIZE];

    volume = 0xf;
    for (byte_pos = 0; byte_pos < calib_num; byte_pos++) {
        PSRAM_TRACE_LINEFEED();
        for (int i = 0; i <= volume; i++){
            cali_valid[i] = !!(valid_square[i] & (1 << byte_pos));
            PSRAM_TRACENOCRLF_NOTS(0, "  %d", cali_valid[i]);
        }
        PSRAM_TRACE_LINEFEED();

        //find the longest rx_dqs window
        calib_val = find_the_largest_window_of_1d_array(cali_valid, volume);
        windows_l = (calib_val >> 8) & 0xFF;
        windows_r = calib_val & 0xFF;

        PSRAM_TRACE(2,"windows_l: %d , windows_r: %d",windows_l, windows_r);
        if ((windows_r - windows_l) < 2) {
            PSRAM_ASSERT(0, "%s : dqs window is too small to ensure data access accurately ", __func__);
        }
        rx_dqs = (windows_r + windows_l) >> 1;

        PSRAM_TRACE(2,"rx_dqs: %d", rx_dqs);
        if (byte_pos == 0) {
            hal_psram_anaphy_set_rxdqs_dly(rx_dqs);
        } else {
            hal_psram_anaphy1_set_rxdqs_dly(rx_dqs);
        }
    }
}

void psram_anaphy_calib(const struct PSRAM_T *psram)
{
    if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_OPI) {
        psram_calib_on_1d(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_0, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
    } else if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_XOPI) {
        if (psram->psram_data_bus[HAL_PSRAM_ID_0] == PSRAM_DATA_BUS_X8) {
            psram_calib_on_1d(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_0, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        } else {
            psram_calib_on_1d(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_ALL, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
        }
    } else if (psram->psram_type[HAL_PSRAM_ID_0] == PSRAM_TYPE_HYPERRAM) {
        psram_calib_on_1d(HAL_PSRAM_ID_0, HAL_PSRAM_PHY_CHANNEL_0, psram->run_freq_mhz[HAL_PSRAM_ID_0] * 1000000);
    } else {
        ///assert
    }
}

void psram_calib(const struct PSRAM_T *psram)
{
    psram_anaphy_calib(psram);
}

void psram_reset(enum HAL_PSRAM_ID_T id, struct PSRAM_CFG_T *psram_cfg)
{
    uint32_t val = 0;
    uint32_t mr, md;

    if (psram_cfg->psram_type == PSRAM_TYPE_HYPERRAM) {
        mr = 2;
        md = (0xa << 12);
        val = hal_psram_mc_mrs_pre_operation(id, psram_cfg);
        hal_psram_mc_var_lat_en(id, false);
        hal_psram_device_mr_write(id, mr, PSRAM_SWAP8BITS(md) | (PSRAM_SWAP8BITS(md) << 16));
        hal_psram_mc_busy_wait(id);
        hal_sys_timer_delay_us(3);
        hal_psram_mc_var_lat_en(id, true);
        hal_psram_mc_mrs_post_operation(id, psram_cfg, val);
    } else {
        hal_psram_mc_set_cmd_rst(id);
    }
}

#endif
