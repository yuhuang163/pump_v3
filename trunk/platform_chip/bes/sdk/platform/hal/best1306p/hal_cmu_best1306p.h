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
#ifndef __HAL_CMU_BEST1306P_H__
#define __HAL_CMU_BEST1306P_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FPGA
#define HAL_CMU_DEFAULT_CRYSTAL_FREQ        48000000
#else
#define HAL_CMU_DEFAULT_CRYSTAL_FREQ        24000000
#endif
#define HAL_CMU_VALID_CRYSTAL_FREQ          { 24000000, 26000000, }

#define HAL_CMU_USB_ROM_SELECT_CLOCK_SOURCE

enum HAL_CMU_MOD_ID_T {
    // HCLK/HRST
    HAL_CMU_MOD_H_MCU,          // 0
    HAL_CMU_MOD_H_ROM0,         // 1
    HAL_CMU_MOD_H_ROM1,         // 2
    HAL_CMU_MOD_H_REAL_FLASH1,  // 3
    HAL_CMU_MOD_H_RAM0,         // 4
    HAL_CMU_MOD_H_RAM1,         // 5
    HAL_CMU_MOD_H_RAM2,         // 6
    HAL_CMU_MOD_H_CORDIC,       // 7
    HAL_CMU_MOD_H_AHB0,         // 8
    HAL_CMU_MOD_H_AHB1,         // 9
    HAL_CMU_MOD_H_AH2H_BT,      // 10
    HAL_CMU_MOD_H_ADMA,         // 11
    HAL_CMU_MOD_H_GDMA,         // 12
    HAL_CMU_MOD_H_PSPY,         // 13
    HAL_CMU_MOD_H_REAL_FLASH,   // 14
    HAL_CMU_MOD_H_EMMC,         // 15
    HAL_CMU_MOD_H_USBC,         // 16
    HAL_CMU_MOD_H_CODEC,        // 17
    HAL_CMU_MOD_H_MCU_CP0,      // 18
    HAL_CMU_MOD_H_BT_TPORT,     // 19
    HAL_CMU_MOD_H_USBH,         // 20
    HAL_CMU_MOD_H_LCDC,         // 21
    HAL_CMU_MOD_H_BT_DUMP,      // 22
    HAL_CMU_MOD_H_CP,           // 23
    HAL_CMU_MOD_H_RAM3,         // 24
    HAL_CMU_MOD_H_RAM4,         // 25
    HAL_CMU_MOD_H_RAM5,         // 26
    HAL_CMU_MOD_H_RAM6,         // 27
    HAL_CMU_MOD_H_RAM7,      // 28
    HAL_CMU_MOD_H_CACHE0,       // 29
    HAL_CMU_MOD_H_AH2H_SENS,    // 30
    HAL_CMU_MOD_H_SPI_AHB,      // 31
    // PCLK/PRST
    HAL_CMU_MOD_P_CMU,          // 32
    HAL_CMU_MOD_P_WDT,          // 33
    HAL_CMU_MOD_P_TIMER0,       // 34
    HAL_CMU_MOD_P_TIMER1,       // 35
    HAL_CMU_MOD_P_TIMER2,       // 36
    HAL_CMU_MOD_P_I2C0,         // 37
    HAL_CMU_MOD_P_I2C1,         // 38
    HAL_CMU_MOD_P_SPI,          // 39
    HAL_CMU_MOD_P_TRNG,         // 40
    HAL_CMU_MOD_P_SPI_ITN,      // 41
    HAL_CMU_MOD_P_UART0,        // 42
    HAL_CMU_MOD_P_UART1,        // 43
    HAL_CMU_MOD_P_UART2,        // 44
    HAL_CMU_MOD_P_PCM,          // 45
    HAL_CMU_MOD_P_I2S0,         // 46
    HAL_CMU_MOD_P_SPDIF0,       // 47
    HAL_CMU_MOD_P_I2S1,         // 48
    HAL_CMU_MOD_P_SEC_ENG,      // 49
    HAL_CMU_MOD_P_TZC,          // 50
    HAL_CMU_MOD_P_UART5,        // 51
    HAL_CMU_MOD_P_PSPY,         // 52
    HAL_CMU_MOD_P_I2C2,         // 53
    HAL_CMU_MOD_P_I2C3,         // 54
    HAL_CMU_MOD_P_I2C4,         // 55
    HAL_CMU_MOD_P_I2C5,         // 56
    HAL_CMU_MOD_P_I2C6,         // 57
    HAL_CMU_MOD_P_I2C7,         // 58
    HAL_CMU_MOD_P_UART3,        // 59
    HAL_CMU_MOD_P_SLCD,         // 60
    HAL_CMU_MOD_P_SPI_DPD,      // 61
    HAL_CMU_MOD_P_UART4,        // 62
    HAL_CMU_MOD_P_PPI,          // 63
    // OCLK/ORST
    HAL_CMU_MOD_O_SLEEP,        // 64
    HAL_CMU_MOD_O_REAL_FLASH,   // 65
    HAL_CMU_MOD_O_USB,          // 66
    HAL_CMU_MOD_O_EMMC,         // 67
    HAL_CMU_MOD_O_WDT,          // 68
    HAL_CMU_MOD_O_TIMER0,       // 69
    HAL_CMU_MOD_O_TIMER1,       // 70
    HAL_CMU_MOD_O_TIMER2,       // 71
    HAL_CMU_MOD_O_I2C0,         // 72
    HAL_CMU_MOD_O_I2C1,         // 73
    HAL_CMU_MOD_O_SPI,          // 74
    HAL_CMU_MOD_O_SPI_ITN,      // 75
    HAL_CMU_MOD_O_UART0,        // 76
    HAL_CMU_MOD_O_UART1,        // 77
    HAL_CMU_MOD_O_UART2,        // 78
    HAL_CMU_MOD_O_I2S0,         // 79
    HAL_CMU_MOD_O_SPDIF0,       // 80
    HAL_CMU_MOD_O_PCM,          // 81
    HAL_CMU_MOD_O_USB32K,       // 82
    HAL_CMU_MOD_O_I2S1,         // 83
    HAL_CMU_MOD_O_REAL_FLASH1,  // 84
    HAL_CMU_MOD_O_DISPIX,       // 85
    HAL_CMU_MOD_O_DISDSI,       // 86
    HAL_CMU_MOD_O_I2C2,         // 87
    HAL_CMU_MOD_O_I2C3,         // 88
    HAL_CMU_MOD_O_SDMMC,        // 89
    HAL_CMU_MOD_O_PSRAM,        // 90
    HAL_CMU_MOD_O_DISPN,        // 91
    HAL_CMU_MOD_O_DISTV,        // 92
    HAL_CMU_MOD_O_2DPNTV,       // 93
    HAL_CMU_MOD_O_GPU,          // 94
    HAL_CMU_MOD_O_LCDC_P,       // 95
    // XCLK/XRST
    HAL_CMU_MOD_X_PSRAM,        // 96
    HAL_CMU_MOD_X_SDMMC,        // 97
    HAL_CMU_MOD_X_GPU,          // 98
    HAL_CMU_MOD_X_L2CC,         // 99
    HAL_CMU_MOD_X_VMMU,         // 100
    HAL_CMU_MOD_X_GA2D,         // 101
    HAL_CMU_MOD_X_MISC,         // 102
    HAL_CMU_MOD_X_LCDC,         // 103
    HAL_CMU_MOD_X_AHB2,         // 104
    HAL_CMU_MOD_X_CACHE1,       // 105
    HAL_CMU_MOD_X_CP1,          // 106
    HAL_CMU_MOD_X_CPUDUMP,      // 107
    HAL_CMU_MOD_X_JPEG,         // 108
    HAL_CMU_MOD_X_CRC,          // 109
    HAL_CMU_MOD_X_CHS,          // 110
    HAL_CMU_MOD_X_CRCCHS,       // 111
    // OXCLK/OXRST
    HAL_CMU_MOD_OX_I2C4,        // 112
    HAL_CMU_MOD_OX_I2C5,        // 113
    HAL_CMU_MOD_OX_I2C6,        // 114
    HAL_CMU_MOD_OX_I2C7,        // 115
    HAL_CMU_MOD_OX_UART3,       // 116
    HAL_CMU_MOD_OX_SPI1,        // 117
    HAL_CMU_MOD_OX_SPI2,        // 118
    HAL_CMU_MOD_OX_UART4,       // 119
    HAL_CMU_MOD_OX_UART5,       // 120
    HAL_CMU_MOD_OX_PPI,         // 121
    HAL_CMU_MOD_OX_DISQSPI,     // 122
    // AON ACLK/ARST
    HAL_CMU_AON_A_CMU,          // 123
    HAL_CMU_AON_A_GPIO0,        // 124
    HAL_CMU_AON_A_GPIO0_INT,    // 125
    HAL_CMU_AON_A_WDT,          // 126
    HAL_CMU_AON_A_PWM,          // 127
    HAL_CMU_AON_A_TIMER0,       // 128
    HAL_CMU_AON_A_PSC,          // 129
    HAL_CMU_AON_A_IOMUX,        // 130
    HAL_CMU_AON_A_APBC,         // 131
    HAL_CMU_AON_A_H2H_MCU,      // 132
    HAL_CMU_AON_A_I2C_SLV,      // 133
    HAL_CMU_AON_A_TZC,          // 134
    HAL_CMU_AON_A_GPIO1,        // 135
    HAL_CMU_AON_A_GPIO1_INT,    // 136
    HAL_CMU_AON_A_GPIO2,        // 137
    HAL_CMU_AON_A_GPIO2_INT,    // 138
    HAL_CMU_AON_A_TIMER1,       // 139
    HAL_CMU_AON_A_PWM1,         // 140
    // AON OCLK/ORST
    HAL_CMU_AON_O_WDT,          // 141
    HAL_CMU_AON_O_TIMER0,       // 142
    HAL_CMU_AON_O_GPIO0,        // 143
    HAL_CMU_AON_O_PWM0,         // 144
    HAL_CMU_AON_O_PWM1,         // 145
    HAL_CMU_AON_O_PWM2,         // 146
    HAL_CMU_AON_O_PWM3,         // 147
    HAL_CMU_AON_O_IOMUX,        // 148
    HAL_CMU_AON_O_GPIO1,        // 149
    HAL_CMU_AON_O_BTAON,        // 150
    HAL_CMU_AON_O_PSC,          // 151
    HAL_CMU_AON_O_GPIO2,        // 152
    HAL_CMU_AON_O_TIMER1,       // 153
    HAL_CMU_AON_O_PWM4,         // 154
    HAL_CMU_AON_O_PWM5,         // 155
    HAL_CMU_AON_O_PWM6,         // 156
    HAL_CMU_AON_O_PWM7,         // 157
    HAL_CMU_AON_O_USB,          // 158

    // AON SUBSYS
    HAL_CMU_AON_MCU,            // 158
    HAL_CMU_AON_CODEC,          // 160
    HAL_CMU_AON_SENS,           // 161
    HAL_CMU_AON_BT,             // 162
    HAL_CMU_AON_MCUCPU,         // 163
    HAL_CMU_AON_RESERVED5,      // 164
    HAL_CMU_AON_BTCPU,          // 165
    HAL_CMU_AON_GLOBAL,         // 166

    HAL_CMU_MOD_QTY,

    HAL_CMU_MOD_GLOBAL = HAL_CMU_AON_GLOBAL,
    HAL_CMU_MOD_BT = HAL_CMU_AON_BT,
    HAL_CMU_MOD_BTCPU = HAL_CMU_AON_BTCPU,

    HAL_CMU_MOD_P_PWM = HAL_CMU_AON_A_PWM,
    HAL_CMU_MOD_P_PWM1 = HAL_CMU_AON_A_PWM1,
    HAL_CMU_MOD_O_PWM0 = HAL_CMU_AON_O_PWM0,
    HAL_CMU_MOD_O_PWM1 = HAL_CMU_AON_O_PWM1,
    HAL_CMU_MOD_O_PWM2 = HAL_CMU_AON_O_PWM2,
    HAL_CMU_MOD_O_PWM3 = HAL_CMU_AON_O_PWM3,
    HAL_CMU_MOD_O_PWM4 = HAL_CMU_AON_O_PWM4,
    HAL_CMU_MOD_O_PWM5 = HAL_CMU_AON_O_PWM5,
    HAL_CMU_MOD_O_PWM6 = HAL_CMU_AON_O_PWM6,
    HAL_CMU_MOD_O_PWM7 = HAL_CMU_AON_O_PWM7,
    HAL_CMU_MOD_O_I2C4 = HAL_CMU_MOD_OX_I2C4,
    HAL_CMU_MOD_O_I2C5 = HAL_CMU_MOD_OX_I2C5,
    HAL_CMU_MOD_O_I2C6 = HAL_CMU_MOD_OX_I2C6,
    HAL_CMU_MOD_O_I2C7 = HAL_CMU_MOD_OX_I2C7,
    HAL_CMU_MOD_O_SLCD = HAL_CMU_MOD_OX_SPI1,
    HAL_CMU_MOD_O_SPI_DPD = HAL_CMU_MOD_OX_SPI2,
    HAL_CMU_MOD_O_UART3 = HAL_CMU_MOD_OX_UART3,
    HAL_CMU_MOD_H_PSRAM = HAL_CMU_MOD_X_PSRAM,
    HAL_CMU_MOD_H_DMA2D = HAL_CMU_MOD_O_2DPNTV,
    HAL_CMU_MOD_X_DMA2D = HAL_CMU_MOD_X_GA2D,
    HAL_CMU_MOD_Q_DISPN2D = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_X_DISPB = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_Q_DIS_PN = HAL_CMU_MOD_O_DISPN,
    HAL_CMU_MOD_Q_DIS_TV = HAL_CMU_MOD_O_DISTV,
    HAL_CMU_MOD_Q_DIS_PIX = HAL_CMU_MOD_O_DISPIX,
    HAL_CMU_MOD_Q_DIS_DSI = HAL_CMU_MOD_O_DISDSI,
    HAL_CMU_MOD_Q_LCDC_P  = HAL_CMU_MOD_O_LCDC_P,

    HAL_CMU_MOD_H_SDMMC = HAL_CMU_MOD_X_SDMMC,

    HAL_CMU_MOD_H_ICACHE = HAL_CMU_MOD_H_CACHE0,
    HAL_CMU_MOD_H_DCACHE = HAL_CMU_MOD_X_CACHE1,
    HAL_CMU_MOD_H_ICACHECP = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_H_DCACHECP = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_H_DISPPRE = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_H_DISP = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_X_DISP = HAL_CMU_MOD_QTY,

#ifdef ALT_BOOT_FLASH
    HAL_CMU_MOD_H_FLASH  = HAL_CMU_MOD_H_REAL_FLASH1,
    HAL_CMU_MOD_H_FLASH1 = HAL_CMU_MOD_H_REAL_FLASH,
    HAL_CMU_MOD_O_FLASH  = HAL_CMU_MOD_O_REAL_FLASH1,
    HAL_CMU_MOD_O_FLASH1 = HAL_CMU_MOD_O_REAL_FLASH,
#else
    HAL_CMU_MOD_H_FLASH  = HAL_CMU_MOD_H_REAL_FLASH,
    HAL_CMU_MOD_H_FLASH1 = HAL_CMU_MOD_H_REAL_FLASH1,
    HAL_CMU_MOD_O_FLASH  = HAL_CMU_MOD_O_REAL_FLASH,
    HAL_CMU_MOD_O_FLASH1 = HAL_CMU_MOD_O_REAL_FLASH1,
#endif

    HAL_CMU_MOD_H_JPEG = HAL_CMU_MOD_X_JPEG,
};

enum HAL_CMU_CLOCK_OUT_ID_T {
    HAL_CMU_CLOCK_OUT_AON_32K           = 0x00,
    HAL_CMU_CLOCK_OUT_AON_OSC           = 0x01,
    HAL_CMU_CLOCK_OUT_AON_OSCX2         = 0x02,
    HAL_CMU_CLOCK_OUT_AON_DIG_OSCX2     = 0x03,
    HAL_CMU_CLOCK_OUT_AON_DIG_OSCX4     = 0x04,
    HAL_CMU_CLOCK_OUT_AON_SYS           = 0x05,
    HAL_CMU_CLOCK_OUT_AON_DCDC2         = 0x07,
    HAL_CMU_CLOCK_OUT_AON_PLLBB         = 0x08,
    HAL_CMU_CLOCK_OUT_AON_RCX8          = 0x09,

    HAL_CMU_CLOCK_OUT_BT_NONE           = 0x40,
    HAL_CMU_CLOCK_OUT_BT_32K            = 0x41,
    HAL_CMU_CLOCK_OUT_BT_SYS            = 0x42,
    HAL_CMU_CLOCK_OUT_BT_48M            = 0x43,
    HAL_CMU_CLOCK_OUT_BT_12M            = 0x44,
    HAL_CMU_CLOCK_OUT_BT_ADC            = 0x45,
    HAL_CMU_CLOCK_OUT_BT_ADCD3          = 0x46,
    HAL_CMU_CLOCK_OUT_BT_DAC            = 0x47,
    HAL_CMU_CLOCK_OUT_BT_DACD6          = 0x48,
    HAL_CMU_CLOCK_OUT_BT_OSR12BLE2M     = 0x50,

    HAL_CMU_CLOCK_OUT_MCU_32K           = 0x60,
    HAL_CMU_CLOCK_OUT_MCU_SYS           = 0x61,
    HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH0   = 0x62,
    HAL_CMU_CLOCK_OUT_MCU_USB           = 0x63,
    HAL_CMU_CLOCK_OUT_MCU_PCLK          = 0x64,
    HAL_CMU_CLOCK_OUT_MCU_I2S0          = 0x65,
    HAL_CMU_CLOCK_OUT_MCU_PCM           = 0x66,
    HAL_CMU_CLOCK_OUT_MCU_SPDIF0        = 0x67,
    HAL_CMU_CLOCK_OUT_MCU_SPI2          = 0x69,
    HAL_CMU_CLOCK_OUT_MCU_SPI1          = 0x6A,
    HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH1   = 0x6B,
    HAL_CMU_CLOCK_OUT_MCU_PSRAM         = 0x6C,
    HAL_CMU_CLOCK_OUT_MCU_PIX           = 0x6D,
    HAL_CMU_CLOCK_OUT_MCU_DSI           = 0x6E,
    HAL_CMU_CLOCK_OUT_MCU_SDMMC         = 0x6F,
    HAL_CMU_CLOCK_OUT_MCU_GPU           = 0x70,
    HAL_CMU_CLOCK_OUT_MCU_LCDCP         = 0x71,
    HAL_CMU_CLOCK_OUT_MCU_SPI3          = 0x72,
    HAL_CMU_CLOCK_OUT_MCU_UART3         = 0x74,
    HAL_CMU_CLOCK_OUT_MCU_PPI           = 0x75,
    HAL_CMU_CLOCK_OUT_MCU_DISQSPI       = 0x76,

#ifdef ALT_BOOT_FLASH
    HAL_CMU_CLOCK_OUT_MCU_FLASH0        = HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH1,
    HAL_CMU_CLOCK_OUT_MCU_FLASH1        = HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH0,
#else
    HAL_CMU_CLOCK_OUT_MCU_FLASH0        = HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH0,
    HAL_CMU_CLOCK_OUT_MCU_FLASH1        = HAL_CMU_CLOCK_OUT_MCU_REAL_FLASH1,
#endif

    HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA     = 0x80,
    HAL_CMU_CLOCK_OUT_CODEC_CODEC       = 0x81,
    HAL_CMU_CLOCK_OUT_CODEC_ANC_IIR     = 0x82,
    HAL_CMU_CLOCK_OUT_CODEC_RS_DAC      = 0x83,
    HAL_CMU_CLOCK_OUT_CODEC_RS_ADC      = 0x84,
    HAL_CMU_CLOCK_OUT_CODEC_EQ_IIR      = 0x85,
    HAL_CMU_CLOCK_OUT_CODEC_FIR         = 0x86,
    HAL_CMU_CLOCK_OUT_CODEC_HCLK        = 0x87,
};

enum HAL_CMU_I2S_MCLK_ID_T {
    HAL_CMU_I2S_MCLK_PER                = 0x00,
    HAL_CMU_I2S_MCLK_PLLPCM             = 0x01,
    HAL_CMU_I2S_MCLK_PLLI2S0            = 0x02,
    HAL_CMU_I2S_MCLK_PLLI2S1            = 0x03,
    HAL_CMU_I2S_MCLK_PLLSPDIF0          = 0x04,
};
#define HAL_CMU_I2S_MCLK_ID_T               HAL_CMU_I2S_MCLK_ID_T

enum HAL_PWM_ID_T {
    HAL_PWM_ID_0,
    HAL_PWM_ID_1,
    HAL_PWM_ID_2,
    HAL_PWM_ID_3,
    HAL_PWM1_ID_0,
    HAL_PWM1_ID_1,
    HAL_PWM1_ID_2,
    HAL_PWM1_ID_3,

    HAL_PWM_ID_QTY
};
#define HAL_PWM_ID_T                        HAL_PWM_ID_T

enum HAL_I2S_ID_T {
    HAL_I2S_ID_0 = 0,
    HAL_I2S_ID_1,

    HAL_I2S_ID_QTY,
};
#define HAL_I2S_ID_T                        HAL_I2S_ID_T

enum HAL_CMU_ANC_CLK_USER_T {
    HAL_CMU_ANC_CLK_USER_ANC,

    HAL_CMU_ANC_CLK_USER_QTY
};

enum HAL_CMU_FREQ_T {
    HAL_CMU_FREQ_32K,
    HAL_CMU_FREQ_6P5M,
    HAL_CMU_FREQ_13M,
    HAL_CMU_FREQ_26M,
    HAL_CMU_FREQ_52M,
    HAL_CMU_FREQ_78M,
    HAL_CMU_FREQ_104M,
    HAL_CMU_FREQ_144M,
    HAL_CMU_FREQ_208M,
    HAL_CMU_FREQ_390M,

    HAL_CMU_FREQ_QTY
};
#define HAL_CMU_FREQ_390M                   HAL_CMU_FREQ_390M
#define HAL_CMU_FREQ_T                      HAL_CMU_FREQ_T

enum HAL_CMU_RAM_CFG_SEL_T {
    HAL_CMU_RAM_CFG_SEL_MCU = 0,
    HAL_CMU_RAM_CFG_SEL_BT,
};

enum HAL_CMU_PLL_T {
    HAL_CMU_PLL_BB = 0,
    HAL_CMU_PLL_AUD = HAL_CMU_PLL_BB,
    HAL_CMU_PLL_DSI = HAL_CMU_PLL_BB,
    HAL_CMU_PLL_USB = HAL_CMU_PLL_BB,

    HAL_CMU_PLL_QTY
};
#define HAL_CMU_PLL_T                       HAL_CMU_PLL_T

enum HAL_CMU_PLL_USER_T {
    HAL_CMU_PLL_USER_SYS,
    HAL_CMU_PLL_USER_AUD,
    HAL_CMU_PLL_USER_USB,
    HAL_CMU_PLL_USER_GPU,
    HAL_CMU_PLL_USER_LCDC,
    HAL_CMU_PLL_USER_DSI,
    HAL_CMU_PLL_USER_FLASH,
    HAL_CMU_PLL_USER_FLASH1,
    HAL_CMU_PLL_USER_PSRAM,
    HAL_CMU_PLL_USER_QSPI,

    HAL_CMU_PLL_USER_QTY,
    HAL_CMU_PLL_USER_ALL = HAL_CMU_PLL_USER_QTY,
};
#define HAL_CMU_PLL_USER_T                  HAL_CMU_PLL_USER_T

enum HAL_CMU_DCDC_CLOCK_USER_T {
    HAL_CMU_DCDC_CLOCK_USER_GPADC,
    HAL_CMU_DCDC_CLOCK_USER_ADCKEY,
    HAL_CMU_DCDC_CLOCK_USER_EFUSE,

    HAL_CMU_DCDC_CLOCK_USER_QTY,
};

enum HAL_CMU_PU_OSC_USER_T {
    HAL_CMU_PU_OSC_USER_SEL_OSC     = (1 << 0),
    HAL_CMU_PU_OSC_USER_PLL         = (1 << 1),
    HAL_CMU_PU_OSC_USER_GPADC       = (1 << 2),
};

enum HAL_CMU_BT_TRIGGER_SRC_T {
    HAL_CMU_BT_TRIGGER_SRC_0,
    HAL_CMU_BT_TRIGGER_SRC_1,
    HAL_CMU_BT_TRIGGER_SRC_2,
    HAL_CMU_BT_TRIGGER_SRC_3,

    HAL_CMU_BT_TRIGGER_SRC_QTY,
};

enum HAL_CMU_CLK_SEL_OSC_USER_T {
    HAL_CMU_CLK_SEL_OSC_USER_SYS      = (1 << 0),
    HAL_CMU_CLK_SEL_OSC_USER_CODEC    = (1 << 1),
    HAL_CMU_CLK_SEL_OSC_USER_BTC      = (1 << 2),
};

typedef void (*HAL_CMU_BT_TRIGGER_HANDLER_T)(enum HAL_CMU_BT_TRIGGER_SRC_T src);

int hal_cmu_fast_timer_offline(void);

enum HAL_CMU_PLL_T hal_cmu_get_audio_pll(void);

void hal_cmu_anc_enable(enum HAL_CMU_ANC_CLK_USER_T user);

void hal_cmu_anc_disable(enum HAL_CMU_ANC_CLK_USER_T user);

int hal_cmu_anc_get_status(enum HAL_CMU_ANC_CLK_USER_T user);

void hal_cmu_mcu_pdm_clock_out(uint32_t clk_map);

void hal_cmu_sens_pdm_clock_out(uint32_t clk_map);

uint32_t hal_cmu_get_aon_chip_id(void);

uint32_t hal_cmu_get_aon_revision_id(void);

void hal_cmu_cp_subsys_enable(uint32_t sp);

void hal_cmu_cp_subsys_disable(void);

void hal_cmu_cp_start_cpu(uint32_t sp, uint32_t entry);

void hal_cmu_cp_enable(uint32_t sp, uint32_t entry);

void hal_cmu_cp_disable(void);

uint32_t hal_cmu_cp_get_entry_addr(void);

uint32_t hal_cmu_get_ram_boot_addr(void);

void hal_cmu_ram_cfg_sel_update(uint32_t map, enum HAL_CMU_RAM_CFG_SEL_T sel);

int hal_cmu_sys_is_using_audpll(void);

int hal_cmu_sys_is_using_bbpll(void);

int hal_cmu_flash_is_using_audpll(void);

int hal_cmu_flash_is_using_bbpll(void);

void hal_cmu_beco_enable(void);

void hal_cmu_beco_disable(void);

void hal_cmu_cp_beco_enable(void);

void hal_cmu_cp_beco_disable(void);

void hal_cmu_dcdc_clock_enable(enum HAL_CMU_DCDC_CLOCK_USER_T user);

void hal_cmu_dcdc_clock_disable(enum HAL_CMU_DCDC_CLOCK_USER_T user);

void hal_cmu_pu_osc_enable(enum HAL_CMU_PU_OSC_USER_T user);

void hal_cmu_pu_osc_disable(enum HAL_CMU_PU_OSC_USER_T user);

void hal_cmu_select_rc_clock(enum HAL_CMU_CLK_SEL_OSC_USER_T user);

void hal_cmu_select_osc_clock(enum HAL_CMU_CLK_SEL_OSC_USER_T user);

uint16_t hal_cmu_get_oscx4_locked_time_us(void);

uint32_t hal_cmu_get_osc_switch_overhead(void);

void hal_cmu_boot_dcdc_clock_enable(void);

void hal_cmu_boot_dcdc_clock_disable(void);

int hal_cmu_bt_trigger_set_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src, HAL_CMU_BT_TRIGGER_HANDLER_T hdlr);

int hal_cmu_bt_trigger_enable(enum HAL_CMU_BT_TRIGGER_SRC_T src);

int hal_cmu_bt_trigger_disable(enum HAL_CMU_BT_TRIGGER_SRC_T src);

void hal_cmu_btc_ram_cfg(void);

void hal_cmu_btc_start_cpu(uint32_t sp, uint32_t entry);

void hal_cmu_ispi_access_init(void);

void hal_cmu_lcdc_sleep(void);

void hal_cmu_lcdc_wakeup(void);

void hal_cmu_force_sys_pll_div(uint32_t div);

void hal_cmu_rc_2m_clock_enable(void);

void hal_cmu_jpeg_clock_enable(void);

void hal_cmu_jpeg_clock_disable(void);

int hal_cmu_osc_wakeup_test_reload_wt24m_us(uint32_t us);

void hal_cmu_rom_start_wdt(uint32_t time);

void hal_cmu_rom_stop_wdt(void);

void hal_cmu_rom_startup_global_reset(void);

void hal_cmu_rom_startup_wdt_reset(void);

void hal_cmu_psram_clock_enable(void);

void hal_cmu_psram_clock_disable(void);

void hal_cmu_programmer_chip_init(void);

void hal_cmu_global_reset(void);

void hal_cmu_acquire_x2_x4(void);

void hal_cmu_release_x2_x4(void);

void hal_cmu_low_freq_mode_enable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq);

void hal_cmu_low_freq_mode_disable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq);

#ifdef __cplusplus
}
#endif

#endif
