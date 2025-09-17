/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef PSRAM_WINDOW_TEST

#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_psram.h"

#ifndef PSRAM_ENABLE
#error "PSRAM_ENABLE not defined"
#endif

#ifdef PSRAM_WINDOW_TEST_FT
static uint8_t ft_len0 = 0, ft_width0 = 0;
static uint8_t ft_len1 = 0, ft_width1 = 0;

void psram_window_2d_analysis(void * const array, uint8_t row_column_num, uint8_t inc_delay, int byte_pos, const struct HAL_PSRAM_RECTANGLE_T *new_rectangle);

uint32_t psram_window_scan_test(void)
{
    uint32_t code = 0;

    hal_psram_window_2d_cb_register(psram_window_2d_analysis);
    hal_psram_init();
    TRACE_IMM(0, "psram window%d:(%d, %d)", 0, ft_len0, ft_width0);
    code |= (ft_len0 << 8) | ft_width0;
    TRACE_IMM(0, "psram window%d:(%d, %d)", 1, ft_len1, ft_width1);
    code |= (ft_len1 << 24) | (ft_width1 << 16);

    return code;
}
#endif

#ifdef PSRAM_CALIB_ON_1D
void psram_window_1d_analysis(struct HAL_PSRAM_WINDOW_1D_ANALYSIS_T *analysis)
{
    char *item_str = (analysis->item == PSRAM_CALIB_RX_DQS? "rx_dqs" :
                      analysis->item == PSRAM_CALIB_TX_DQS? "tx_dqs" :
                      analysis->item == PSRAM_CALIB_TX_CLK? "tx_ckl" :
                      analysis->item == PSRAM_CALIB_TX_CEB? "tx_ceb" :
                                                            "unknow");
    if (analysis->item == PSRAM_CALIB_RX_DQS) {
        REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "(range = %1d, T/4 = %2d)  %s: " ,analysis->range, analysis->t_div_2/2, item_str);
    } else {
        REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "                       %s: " ,item_str);
    }
    for (int i = 0; i < PSRAM_WINDOW_SIZE; i++) {
        REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM,"%d",analysis->raw_data[i]);
    }
    REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "-->[%2d, %2d] %s",analysis->boundary->l,analysis->boundary->r,
                          analysis->boundary->is_legal? "":"ilegal");
    REL_TRACE_OUTPUT_LINEFEED();
    if (analysis->item == PSRAM_CALIB_TX_DQS) {
        REL_TRACE_OUTPUT_LINEFEED();
    }
}

#else
void psram_window_2d_analysis(void * const array, uint8_t row_column_num, uint8_t inc_delay, int byte_pos, const struct HAL_PSRAM_RECTANGLE_T *new_rectangle)
{
#ifdef PSRAM_WINDOW_TEST_FT
    if (byte_pos == 0) {
        ft_len0 = (new_rectangle ->x_end - new_rectangle ->x_start) * inc_delay;
        ft_width0 = (new_rectangle ->y_end - new_rectangle ->y_start) * inc_delay;
    } else {
        ft_len1 = (new_rectangle ->x_end - new_rectangle ->x_start) * inc_delay;
        ft_width1 = (new_rectangle ->y_end - new_rectangle ->y_start) * inc_delay;
    }
#else
    uint8_t (*p)[PSRAM_WINDOW_SIZE] = (uint8_t (*)[PSRAM_WINDOW_SIZE])array;
    uint8_t tx_dqs, rx_dqs, range, dly_in;
    uint8_t enable_print_window = 0;
    int square_len = 0;
    static struct HAL_PSRAM_RECTANGLE_T last_rectangle[2] = {{0,0,0,0},{0,0,0,0}};

    if (byte_pos == 0) {
        range = hal_psram_phy_get_range_val();
        dly_in = hal_psram_phy_get_dly_in_val();
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", range = 0x%02x, dly_in = 0x%02x", range, dly_in);
    } else {
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "                             ");
    }
    square_len = MIN((new_rectangle->y_end - new_rectangle->y_start) , (new_rectangle->x_end - new_rectangle->x_start));
    if (square_len == 0) {
        enable_print_window = 1;
    }
    if ((new_rectangle->x_start != last_rectangle[byte_pos].x_start) || (new_rectangle->x_end != last_rectangle[byte_pos].x_end)) {
        last_rectangle[byte_pos].x_start = new_rectangle->x_start;
        last_rectangle[byte_pos].x_end = new_rectangle->x_end;
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", %d-rx_dqs:(%02d~%02d)",
                      byte_pos, new_rectangle->x_start * inc_delay, new_rectangle->x_end * inc_delay);
    } else {
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", %d-++++++++++++++", byte_pos);
    }
    if ((new_rectangle->y_start != last_rectangle[byte_pos].y_start) || (new_rectangle->y_end != last_rectangle[byte_pos].y_end)) {
        last_rectangle[byte_pos].y_start = new_rectangle->y_start;
        last_rectangle[byte_pos].y_end = new_rectangle->y_end;
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", tx_dqs:(%02d~%02d), %s",
                    new_rectangle->y_start * inc_delay, new_rectangle->y_end * inc_delay, enable_print_window? "abnormal" : "normal");
    } else {
        TRACE_IMM(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", --------------, %s", enable_print_window? "abnormal" : "normal");
    }
    TRACE_IMM(TR_ATTR_NO_TS | TR_ATTR_NO_ID, ", vol:%02d, inc_delay:%02d", row_column_num -1, inc_delay);
    if (enable_print_window) {
        enable_print_window = 0;
        //Print original window data
        REL_TRACE_OUTPUT_LINEFEED();
        REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "   rx_dqs ");
        for (tx_dqs = 0; tx_dqs < row_column_num; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, " %2d ", tx_dqs * inc_delay);
        }
        REL_TRACE_OUTPUT_LINEFEED();
        for (tx_dqs = 0; tx_dqs < row_column_num; tx_dqs++) {
            REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "tx_dqs:%2d ", tx_dqs * inc_delay);
            for (rx_dqs = 0; rx_dqs < row_column_num; rx_dqs++) {
                REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "  %d ", !!(p[tx_dqs][rx_dqs] & (1 << byte_pos)));
            }
            REL_TRACE_OUTPUT_LINEFEED();
        }
        REL_TRACE_OUTPUT_LINEFEED();
    }
#endif
}
#endif

void psram_window_test(void)
{
    TRACE(0, "  ");
#ifdef CHIP_BEST2003
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_390M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
#endif
    TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    TRACE(0, "------psram window test started------");
    TRACE(0, "  ");
    hal_psram_snoop_disable();

#ifdef PSRAM_CALIB_ON_1D
    hal_psram_window_1d_cb_register(psram_window_1d_analysis);
#else
    hal_psram_window_2d_cb_register(psram_window_2d_analysis);
#endif
    while (1) {
        hal_psram_calib(PSRAM_SPEED * 1000 * 1000);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}

#endif
