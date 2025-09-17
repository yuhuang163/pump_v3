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
#include "tgt_hardware.h"
#include "iir_process.h"

const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_pinmux_pwl[CFG_HW_PWL_NUM] = {
};

//adckey define
const uint16_t cfg_hw_adckey_map_table[CFG_HW_ADCKEY_NUMBER] = {
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
#define CFG_HW_GPIOKEY_DOWN_LEVEL          (0)
#define CFG_HW_GPIOKEY_UP_LEVEL            (1)
const struct HAL_KEY_GPIOKEY_CFG_T cfg_hw_gpio_key_cfg[CFG_HW_GPIOKEY_NUM] = {
};

//BT config
const char *BT_LOCAL_NAME = "BES_FPGA_BT\0";
uint8_t bt_global_addr[6] = {
#ifdef BT_DEV_ADDR
    BT_DEV_ADDR
#else
#ifdef FPGA_A2DP_SINK
    0x77,0x77,0x33,0x22,0x11,0x11
#else
    0xF8,0x33,0x66,0x22,0x11,0x11
#endif
#endif
};
//BLE config
const char *BLE_DEFAULT_NAME = "BES_FPGA_BT_BLE";
uint8_t ble_global_addr[6] = {
#ifdef BLE_DEV_ADDR
    BLE_DEV_ADDR
#else
    0xBE,0x99,0x34,0x45,0x56,0x67
#endif
};

const CODEC_DAC_VOL_T codec_dac_vol[TGT_VOLUME_LEVEL_QTY] = {
    -99, -45, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -12, -9, -6, -3, 0,
};

#if SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2
#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | AUD_VMIC_MAP_VMIC1)
#else
#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH4 | AUD_VMIC_MAP_VMIC1)
#endif
#define CFG_HW_AUD_INPUT_PATH_LINEIN_DEV    (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
const struct AUD_IO_PATH_CFG_T cfg_audio_input_path_cfg[CFG_HW_AUD_INPUT_PATH_NUM] = {
    { AUD_INPUT_PATH_MAINMIC, CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV, },
    { AUD_INPUT_PATH_LINEIN,  CFG_HW_AUD_INPUT_PATH_LINEIN_DEV, },
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

//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_dac_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 20,
    .param = {
        {IIR_TYPE_PEAK, -3,   20.0,   0.3},
        {IIR_TYPE_PEAK, -3,   50.0,   0.3},
        {IIR_TYPE_PEAK, -3,   100.0,   0.3},
        {IIR_TYPE_PEAK, -3,   128.0,   0.3},
        {IIR_TYPE_PEAK, -3,   400.0,   0.3},
        {IIR_TYPE_PEAK, -3,   512.0,   0.3},
        {IIR_TYPE_PEAK, -3,   600.0,   0.3},
        {IIR_TYPE_PEAK, -3,   800.0,   0.3},
        {IIR_TYPE_PEAK, -3,   1000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   2000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   3000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   4000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   5000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   6000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   7000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   8000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   9000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   10000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   15000.0,   0.3},
        {IIR_TYPE_PEAK, -3,   17000.0,   0.3}
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM]={
    &audio_eq_hw_dac_iir_cfg,
};
