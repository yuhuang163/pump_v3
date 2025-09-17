/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "tgt_hardware.h"
#include "analog.h"

#if !defined(__NuttX__)

const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_pinmux_pwl[CFG_HW_PWL_NUM] = {
#if (CFG_HW_PWL_NUM > 0)
#endif
};

//adckey define
const uint16_t cfg_hw_adckey_map_table[CFG_HW_ADCKEY_NUMBER] = {
#if (CFG_HW_ADCKEY_NUMBER > 0)
    HAL_KEY_CODE_FN9,HAL_KEY_CODE_FN8,HAL_KEY_CODE_FN7,
    HAL_KEY_CODE_FN6,HAL_KEY_CODE_FN5,HAL_KEY_CODE_FN4,
    HAL_KEY_CODE_FN3,HAL_KEY_CODE_FN2,HAL_KEY_CODE_FN1
#endif
};

const uint16_t cfg_hw_adckey_vol_table[CFG_HW_ADCKEY_NUMBER] = {
#if (CFG_HW_ADCKEY_NUMBER > 0)
    100,
    200,
    300,
    400,
    500,
    600,
    700,
    800,
    900
#endif
};

//gpiokey define
const struct HAL_KEY_GPIOKEY_CFG_T cfg_hw_gpio_key_cfg[CFG_HW_GPIOKEY_NUM] = {
#if (CFG_HW_GPIOKEY_NUM > 0)
#endif
};
#endif
//bt config
const char *BT_LOCAL_NAME = TO_STRING(BT_DEV_NAME) "\0";
const char *BLE_DEFAULT_NAME = "BES_BLE";

uint8_t ble_global_addr[6] = {
#ifdef BLE_DEV_ADDR
    BLE_DEV_ADDR
#else
    0xBE,0x99,0x34,0x45,0x56,0x67
#endif
};
uint8_t bt_global_addr[6] = {
#ifdef BT_DEV_ADDR
    BT_DEV_ADDR
#else
    0x1e,0x57,0x34,0x45,0x56,0x67
#endif
};

const CODEC_DAC_VOL_T codec_dac_vol[TGT_VOLUME_LEVEL_QTY] = {
    -99, -45, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -12, -9, -6, -3, 0,
};

#ifndef VMIC_MAP_CFG
#define VMIC_MAP_CFG                        AUD_VMIC_MAP_VMIC1
#endif
#ifndef MAINMIC_MAP_CFG
#define MAINMIC_MAP_CFG                     AUD_CHANNEL_MAP_CH0
#endif
#ifndef LINEIN_MAP_CFG
#if (MAX_ANA_MIC_CH_NUM >= 2)
#define LINEIN_MAP_CFG                      (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
#else
#define LINEIN_MAP_CFG                      AUD_CHANNEL_MAP_CH0
#endif
#endif
#ifndef VADMIC_MAP_CFG
#ifdef ANA_VAD_MIC_CH
#define VADMIC_MAP_CFG                      (ANA_VAD_MIC_CH)
#else
#define VADMIC_MAP_CFG                      (AUD_CHANNEL_MAP_CH0 << (MAX_ANA_MIC_CH_NUM - 1))
#endif
#endif

#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (MAINMIC_MAP_CFG | VMIC_MAP_CFG)
#define CFG_HW_AUD_INPUT_PATH_LINEIN_DEV    (LINEIN_MAP_CFG)
#define CFG_HW_AUD_INPUT_PATH_VADMIC_DEV    (VADMIC_MAP_CFG | VMIC_MAP_CFG)

const struct AUD_IO_PATH_CFG_T cfg_audio_input_path_cfg[CFG_HW_AUD_INPUT_PATH_NUM] = {
    { AUD_INPUT_PATH_MAINMIC, CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV, },
    { AUD_INPUT_PATH_LINEIN,  CFG_HW_AUD_INPUT_PATH_LINEIN_DEV,  },
    { AUD_INPUT_PATH_VADMIC,  CFG_HW_AUD_INPUT_PATH_VADMIC_DEV,  },
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_enable_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_detecter_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_indicator_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
};

#ifdef ANC_APP
#include "aud_section.h"

#define IIR_COUNTER_FF_L (6)
#define IIR_COUNTER_FF_R (6)
#define IIR_COUNTER_FB_L (5)
#define IIR_COUNTER_FB_R (5)

static const struct_anc_cfg AncFirCoef_50p7k_mode0 = {
    .anc_cfg_ff_l = {
#ifdef ANC_PROD_TEST
            .total_gain = 0,
#else
            .total_gain = 350,
#endif
        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={42462788,    -84862242,     42399478},
        .iir_coef[0].coef_a={134217728,   -268358003,    134140286},

        .iir_coef[1].coef_b={135905569,   -267224817,    131334465},
        .iir_coef[1].coef_a={134217728,   -267224817,    133022306},

        .iir_coef[2].coef_b={132936489,   -263935268,    131067941},
        .iir_coef[2].coef_a={134217728,   -263935268,    129786702},

        .iir_coef[3].coef_b={131758190,   -257297054,    126191415},
        .iir_coef[3].coef_a={134217728,   -257297054,    123731878},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*      .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    .anc_cfg_ff_r = {
#ifdef ANC_PROD_TEST
            .total_gain = 0,
#else
            .total_gain = 350,
#endif
        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b={42462788,    -84862242,     42399478},
        .iir_coef[0].coef_a={134217728,   -268358003,    134140286},

        .iir_coef[1].coef_b={135905569,   -267224817,    131334465},
        .iir_coef[1].coef_a={134217728,   -267224817,    133022306},

        .iir_coef[2].coef_b={132936489,   -263935268,    131067941},
        .iir_coef[2].coef_a={134217728,   -263935268,    129786702},

        .iir_coef[3].coef_b={131758190,   -257297054,    126191415},
        .iir_coef[3].coef_a={134217728,   -257297054,    123731878},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*      .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
     .adc_gain_offset=(0)*4,
    },
#endif

/*
Filter1_B=[     27461831,    -54408898,     27001841];
Filter1_A=[    134217728,   -216605724,     82606056];

Filter2_B=[    138294078,   -267600712,    129323227];
Filter2_A=[    134217728,   -267600712,    133399577];

Filter3_B=[    134500015,   -268177932,    133678688];
Filter3_A=[    134217728,   -268177932,    133960975];

Filter4_B=[    133629164,   -264794659,    131257050];
Filter4_A=[    134217728,   -264794659,    130668486];
*/

    .anc_cfg_fb_l = {
#ifdef ANC_PROD_TEST
            .total_gain = 0,
#else
            .total_gain = 350,
#endif
        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b={  27461831,    -54408898,     27001841},
        .iir_coef[0].coef_a={134217728,   -216605724,     82606056},

        .iir_coef[1].coef_b={138294078,   -267600712,    129323227},
        .iir_coef[1].coef_a={134217728,   -267600712,    133399577},

        .iir_coef[2].coef_b={134500015,   -268177932,    133678688},
        .iir_coef[2].coef_a={134217728,   -268177932,    133960975},

        .iir_coef[3].coef_b={133629164,   -264794659,    131257050},
        .iir_coef[3].coef_a={134217728,   -264794659,    130668486},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*      .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    .anc_cfg_fb_r = {
#ifdef ANC_PROD_TEST
            .total_gain = 0,
#else
            .total_gain = 350,
#endif
        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b={  27461831,    -54408898,     27001841},
        .iir_coef[0].coef_a={134217728,   -216605724,     82606056},

        .iir_coef[1].coef_b={138294078,   -267600712,    129323227},
        .iir_coef[1].coef_a={134217728,   -267600712,    133399577},

        .iir_coef[2].coef_b={134500015,   -268177932,    133678688},
        .iir_coef[2].coef_a={134217728,   -268177932,    133960975},

        .iir_coef[3].coef_b={133629164,   -264794659,    131257050},
        .iir_coef[3].coef_a={134217728,   -264794659,    130668486},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*      .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
     .adc_gain_offset=(0)*4,
    },
#endif

    .anc_cfg_mc_l = {
        .total_gain = 0,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b={  27461831,    -54408898,     27001841},
        .iir_coef[0].coef_a={134217728,   -216605724,     82606056},

        .iir_coef[1].coef_b={138294078,   -267600712,    129323227},
        .iir_coef[1].coef_a={134217728,   -267600712,    133399577},

        .iir_coef[2].coef_b={134500015,   -268177932,    133678688},
        .iir_coef[2].coef_a={134217728,   -268177932,    133960975},

        .iir_coef[3].coef_b={133629164,   -264794659,    131257050},
        .iir_coef[3].coef_a={134217728,   -264794659,    130668486},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*      .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};

const struct_anc_cfg * anc_coef_list_50p7k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_50p7k_mode0,
};
#endif
