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
#include "psap_freq_psap.h"

static float CalibFlt[6]=
{
    1.f,
};

PsapFreqPsapConfig psap_freq_psap_cfg = {
    .bypass = 0,
    .debug_en = 0,

    .bf_enable = 1,
    .dehowling_enable = 0,
    .howling_af_enable = 0,
    .ns_enable = 1,
    .wdrc_enable = 1,
    .eq_enable = 1,
    .reverberator_enable = 0,
    .limiter_enable = 1,

    .bf_cfg = {
        .init_enable = 1,
        .dist = 0.020,
        .calib_filter = CalibFlt,
        .calib_filter_len = ARRAY_SIZE(CalibFlt),
    },

    .dehowling_cfg = {
        .onoff = 1,
        .debug_en = 0,
        .power_thd = 0.1,       // energy threshold
        .factor = 100,
        .attact_cnt = 30,       // attact time
        .release_cnt = 5000,    // attact time
    },

    .howling_af_cfg = {
        .delay = 7,
    },

    .ns_cfg = {
        .init_status = 2,
        .ns_power_thd = 0.1,
    },

    .wdrc_cfg = {
        .total_gain_l = 0.0,
        .total_gain_r = 0.0,
        .psap_band_num = 13,
        .PSAP_CS = {0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999},
        .PSAP_CT = {-40, -25,-10, -10, -10, -10, -10, -10, -10, -10, -10, -10},
        .PSAP_WS = {0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999, 0.9999},
        .PSAP_WT = {-20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0, -20.0},
        .PSAP_ES = {-0.9999,  -0.9999,  -0.9999,  -0.9999,   -0.9999, -0.9999,  -0.9999,  -0.9999,  -0.9999,  -0.9999,  -0.9999, -0.9999, -0.9999},
        .PSAP_ET = {-60, -60, -60, -60, -60, -60, -60, -60, -60, -60, -60, -60, -60},
        .PSAP_AT = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25},
        .PSAP_RT = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25},
        .PSAP_TA = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
        .PSAP_GAIN = {  0,    0,   0,   0,   0,    0,   0,   0,    0,    0,     0,    0,    0},
        .PSAP_FREQ = { 375, 750, 1125, 2250, 3000, 3750, 4500, 4875, 5250, 6000, 8000, 10000},
    },

    .eq_cfg = {
        // left
        {
            .bypass = 0,
            .gain   = 0.f,
            .num    = 1,
            .params = {
                {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
            },
        },
        // right
        {
            .bypass = 0,
            .gain   = 0.f,
            .num    = 1,
            .params = {
                {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
            },
        },
    },

    .reverberator_cfg = {
        //left
        {
            .bypass = 0,
            .reverberance = 50,
            .hf_damping = 100,
            .room_scale = 100,
            .pre_delay_ms = 10,
            .wet_gain_dB = 0,
            .comb_lengths = { 1116/2, 1188/2, 1277/2, 1356/2, 1422/2, 1491/2, 1557/2, 1617/2},
            .comb_used = 8,
            .allpass_lengths = { 225/2, 341/2, 441/2, 556/2 },
            .allpass_used = 4,
        },
        //right
        {
            .bypass = 0,
            .reverberance = 50,
            .hf_damping = 100,
            .room_scale = 100,
            .pre_delay_ms = 10,
            .wet_gain_dB = 0,
            .comb_lengths = { 1116/2, 1188/2, 1277/2, 1356/2, 1422/2, 1491/2, 1557/2, 1617/2},
            .comb_used = 8,
            .allpass_lengths = { 225/2, 341/2, 441/2, 556/2 },
            .allpass_used = 4,
        },
    },

    .limiter_cfg = {
        .knee = 2,
        .look_ahead_time = 10,
        .threshold = -1,
        .makeup_gain = 0,
        .ratio = 1000,
        .attack_time = 3,
        .release_time = 50,
    },
};
