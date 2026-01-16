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
#ifndef __REG_LCDC_H__
#define __REG_LCDC_H__

#include "plat_types.h"

struct LCDC_REG_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_0C8;
    __IO uint32_t REG_0CC;
    __IO uint32_t REG_0D0;
    __IO uint32_t REG_0D4;
    __IO uint32_t REG_0D8;
    __IO uint32_t REG_0DC;
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
    __IO uint32_t REG_0F4;
    __IO uint32_t REG_0F8;
    __IO uint32_t REG_0FC;
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_10C;
    __IO uint32_t REG_110;
    __IO uint32_t REG_114;
    __IO uint32_t REG_118;
    __IO uint32_t REG_11C;
    __IO uint32_t REG_120;
    __IO uint32_t REG_124;
    __IO uint32_t REG_128;
    __IO uint32_t REG_12C;
    __IO uint32_t REG_130;
    __IO uint32_t REG_134;
    __IO uint32_t REG_138;
    __IO uint32_t REG_13C;
    __IO uint32_t REG_140;
    __IO uint32_t REG_144;
    __IO uint32_t REG_148;
    __IO uint32_t REG_14C;
    __IO uint32_t REG_150;
    __IO uint32_t REG_154;
    __IO uint32_t REG_158;
    __IO uint32_t REG_15C;
    __IO uint32_t REG_160;
    __IO uint32_t REG_164;
    __IO uint32_t REG_168;
    __IO uint32_t REG_16C;
    __IO uint32_t REG_170;
    __IO uint32_t REG_174;
    __IO uint32_t REG_178;
    __IO uint32_t REG_17C;
    __IO uint32_t REG_180;
    __IO uint32_t REG_184;
    __IO uint32_t REG_188;
    __IO uint32_t REG_18C;
    __IO uint32_t REG_190;
    __IO uint32_t REG_194;
    __IO uint32_t REG_198;
    __IO uint32_t REG_19C;
    __IO uint32_t REG_1A0;
    __IO uint32_t REG_1A4;
    __IO uint32_t REG_1A8;
    __IO uint32_t REG_1AC;
    __IO uint32_t REG_1B0;
    __IO uint32_t REG_1B4;
    __IO uint32_t REG_1B8;
    __IO uint32_t REG_1BC;
    __IO uint32_t REG_1C0;
    __IO uint32_t REG_1C4;
    __IO uint32_t REG_1C8;
    __IO uint32_t REG_1CC;
    __IO uint32_t REG_1D0;
    __IO uint32_t REG_1D4;
    __IO uint32_t REG_1D8;
    __IO uint32_t REG_1DC;
    __IO uint32_t REG_1E0;
    __IO uint32_t REG_1E4;
    __IO uint32_t REG_1E8;
    __IO uint32_t REG_1EC;
    __IO uint32_t REG_1F0;
    __IO uint32_t REG_1F4;
    __IO uint32_t REG_1F8;
    __IO uint32_t REG_1FC;
    __IO uint32_t REG_200;
    __IO uint32_t REG_204;
    __IO uint32_t REG_208;
    __IO uint32_t REG_20C;
    __IO uint32_t REG_210;
    __IO uint32_t REG_214;
    __IO uint32_t REG_218;
    __IO uint32_t REG_21C;
    __IO uint32_t REG_220;
    __IO uint32_t REG_224;
    __IO uint32_t REG_228;
    __IO uint32_t REG_22C;
    __IO uint32_t REG_230;
    __IO uint32_t REG_234;
    __IO uint32_t REG_238;
    __IO uint32_t REG_23C;
    __IO uint32_t REG_240;
    __IO uint32_t REG_244;
    __IO uint32_t REG_248;
    __IO uint32_t REG_24C;
    __IO uint32_t REG_250;
    __IO uint32_t REG_254;
    __IO uint32_t REG_258;
    __IO uint32_t REG_25C;
    __IO uint32_t REG_260;
    __IO uint32_t REG_264;
    __IO uint32_t REG_268;
    __IO uint32_t REG_26C;
    __IO uint32_t REG_270;
    __IO uint32_t REG_274;
    __IO uint32_t REG_278;
    __IO uint32_t REG_27C;
    __IO uint32_t REG_280;
    __IO uint32_t REG_284;
    __IO uint32_t REG_288;
    __IO uint32_t REG_28C;
    __IO uint32_t REG_290;
    __IO uint32_t REG_294;
    __IO uint32_t REG_298;
    __IO uint32_t REG_29C;
    __IO uint32_t REG_2A0;
    __IO uint32_t REG_2A4;
    __IO uint32_t REG_2A8;
    __IO uint32_t REG_2AC;
    __IO uint32_t REG_2B0;
    __IO uint32_t REG_2B4;
    __IO uint32_t REG_2B8;
    __IO uint32_t REG_2BC;
    __IO uint32_t REG_2C0;
    __IO uint32_t REG_2C4;
    __IO uint32_t REG_2C8;
    __IO uint32_t REG_2CC;
    __IO uint32_t REG_2D0;
    __IO uint32_t REG_2D4;
    __IO uint32_t REG_2D8;
    __IO uint32_t REG_2DC;
    __IO uint32_t RES_2E0_328[19];
    __IO uint32_t REG_32C;
    __IO uint32_t REG_330;
    __IO uint32_t REG_334;
    __IO uint32_t REG_338;
    __IO uint32_t REG_33C;
    __IO uint32_t REG_340;
    __IO uint32_t REG_344;
    __IO uint32_t REG_348;
    __IO uint32_t REG_34C;
    __IO uint32_t REG_350;
    __IO uint32_t REG_354;
    __IO uint32_t REG_358;
    __IO uint32_t REG_35C;
    __IO uint32_t REG_360;
    __IO uint32_t REG_364;
    __IO uint32_t REG_368;
    __IO uint32_t REG_36C;
    __IO uint32_t REG_370;
    __IO uint32_t REG_374_4EC[95];
    __IO uint32_t REG_4F0;
    __IO uint32_t REG_4F4_5AC[47];
    __IO uint32_t REG_5B0;
    __IO uint32_t REG_5B4;
    __IO uint32_t REG_5B8;
    __IO uint32_t REG_5BC;
    __IO uint32_t REG_5C0;
    __IO uint32_t REG_5C4;
    __IO uint32_t REG_5C8;
    __IO uint32_t REG_5CC;
    __IO uint32_t REG_5D0;
    __IO uint32_t REG_5D4;
    __IO uint32_t REG_5D8;
    __IO uint32_t REG_5DC;
    __IO uint32_t REG_5E0_5EC[4];
    __IO uint32_t REG_5F0;
    __IO uint32_t REG_5F4;
    __IO uint32_t REG_5F8;
    __IO uint32_t REG_5FC;
    __IO uint32_t REG_600;
    __IO uint32_t REG_604;
    __IO uint32_t REG_608;
    __IO uint32_t REG_60C;
    __IO uint32_t REG_610;
    __IO uint32_t REG_614;
    __IO uint32_t REG_618;
    __IO uint32_t REG_61C;
    __IO uint32_t REG_620;
    __IO uint32_t REG_624;
    __IO uint32_t REG_628;
    __IO uint32_t REG_62C;
    __IO uint32_t REG_630;
    __IO uint32_t REG_634;
    __IO uint32_t REG_638;
    __IO uint32_t REG_63C;
    __IO uint32_t REG_640;
    __IO uint32_t REG_644;
    __IO uint32_t REG_648;
    __IO uint32_t REG_64C;
    __IO uint32_t REG_650;
    __IO uint32_t REG_654;
    __IO uint32_t REG_658;
    __IO uint32_t REG_65C;
    __IO uint32_t REG_660;
    __IO uint32_t REG_664;
    __IO uint32_t REG_668;
    __IO uint32_t REG_66C;
    __IO uint32_t REG_670;
    __IO uint32_t REG_674;
    __IO uint32_t REG_678;
    __IO uint32_t REG_67C;
    __IO uint32_t REG_680;
    __IO uint32_t REG_684;
    __IO uint32_t REG_688;
    __IO uint32_t REG_68C;
    __IO uint32_t REG_690;
    __IO uint32_t REG_694;
    __IO uint32_t REG_698;
    __IO uint32_t REG_69C;
    __IO uint32_t REG_6A0_6B0[5];
    __IO uint32_t REG_6B4;
    __IO uint32_t REG_6B8_6F8[17];
    __IO uint32_t REG_6FC;
    __IO uint32_t REG_700;
    __IO uint32_t REG_704;
    __IO uint32_t REG_708;
    __IO uint32_t REG_70C;
    __IO uint32_t REG_710;
    __IO uint32_t REG_714;
    __IO uint32_t REG_718;
    __IO uint32_t REG_71C;
    __IO uint32_t REG_720;
    __IO uint32_t REG_724;
    __IO uint32_t REG_728;
};

// reg_00
#define LCDC_TVD_SA_Y0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_Y0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_Y0_SHIFT                                    (0)

// reg_04
#define LCDC_TVD_SA_U0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_U0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_U0_SHIFT                                    (0)

// reg_08
#define LCDC_TVD_SA_V0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_V0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_V0_SHIFT                                    (0)

// reg_0c
#define LCDC_TVD_SA_C0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_C0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_C0_SHIFT                                    (0)

// reg_10
#define LCDC_TVD_SA_Y1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_Y1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_Y1_SHIFT                                    (0)

// reg_14
#define LCDC_TVD_SA_U1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_U1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_U1_SHIFT                                    (0)

// reg_18
#define LCDC_TVD_SA_V1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_V1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_V1_SHIFT                                    (0)

// reg_1c
#define LCDC_TVD_SA_C1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVD_SA_C1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVD_SA_C1_SHIFT                                    (0)

// reg_20
#define LCDC_TVD_PITCH_Y(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_TVD_PITCH_Y_MASK                                   (0xFFFF << 0)
#define LCDC_TVD_PITCH_Y_SHIFT                                  (0)
#define LCDC_TVD_PITCH_C(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_TVD_PITCH_C_MASK                                   (0xFFFF << 16)
#define LCDC_TVD_PITCH_C_SHIFT                                  (16)

// reg_24
#define LCDC_TVD_PITCH_U(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_TVD_PITCH_U_MASK                                   (0xFFFF << 0)
#define LCDC_TVD_PITCH_U_SHIFT                                  (0)
#define LCDC_TVD_PITCH_V(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_TVD_PITCH_V_MASK                                   (0xFFFF << 16)
#define LCDC_TVD_PITCH_V_SHIFT                                  (16)

// reg_28
#define LCDC_TVD_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_TVD_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_TVD_OVSA_HPXL_SHIFT                                (0)
#define LCDC_TVDFRM_Y                                           (1 << 12)
#define LCDC_TVDFRM_U                                           (1 << 13)
#define LCDC_TVDFRM_V                                           (1 << 14)
#define LCDC_TVDFRM_C                                           (1 << 15)
#define LCDC_TVD_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVD_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_TVD_OVSA_VLN_SHIFT                                 (16)
#define LCDC_TVDFRM_FIX                                         (1 << 31)

// reg_2c
#define LCDC_TVD_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TVD_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_TVD_H_PIXEL_SHIFT                                  (0)
#define LCDC_TVD_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_TVD_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_TVD_V_LINE_SHIFT                                   (16)

// reg_30
#define LCDC_TVDZM_H_PIXEL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_TVDZM_H_PIXEL_MASK                                 (0xFFF << 0)
#define LCDC_TVDZM_H_PIXEL_SHIFT                                (0)
#define LCDC_TVDZM_V_LINE(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVDZM_V_LINE_MASK                                  (0xFFF << 16)
#define LCDC_TVDZM_V_LINE_SHIFT                                 (16)

// reg_34
#define LCDC_TVG_SA0(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVG_SA0_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_TVG_SA0_SHIFT                                      (0)

// reg_38
#define LCDC_TVG_SA1(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVG_SA1_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_TVG_SA1_SHIFT                                      (0)

// reg_3c
#define LCDC_TVG_PITCH(n)                                       (((n) & 0xFFFF) << 0)
#define LCDC_TVG_PITCH_MASK                                     (0xFFFF << 0)
#define LCDC_TVG_PITCH_SHIFT                                    (0)
#define LCDC_TV_BKLIGHTDIV(n)                                   (((n) & 0xFFF) << 16)
#define LCDC_TV_BKLIGHTDIV_MASK                                 (0xFFF << 16)
#define LCDC_TV_BKLIGHTDIV_SHIFT                                (16)
#define LCDC_TV_BKLIGHTDIV_CTRL(n)                              (((n) & 0xF) << 28)
#define LCDC_TV_BKLIGHTDIV_CTRL_MASK                            (0xF << 28)
#define LCDC_TV_BKLIGHTDIV_CTRL_SHIFT                           (28)

// reg_40
#define LCDC_TVG_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_TVG_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_TVG_OVSA_HPXL_SHIFT                                (0)
#define LCDC_TVGFRM_Y                                           (1 << 12)
#define LCDC_TVG_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVG_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_TVG_OVSA_VLN_SHIFT                                 (16)
#define LCDC_TVGFRM_FIX                                         (1 << 31)

// reg_44
#define LCDC_TVG_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TVG_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_TVG_H_PIXEL_SHIFT                                  (0)
#define LCDC_TVG_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_TVG_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_TVG_V_LINE_SHIFT                                   (16)

// reg_48
#define LCDC_TVGZM_H_PIXEL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_TVGZM_H_PIXEL_MASK                                 (0xFFF << 0)
#define LCDC_TVGZM_H_PIXEL_SHIFT                                (0)
#define LCDC_TVGZM_V_LINE(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVGZM_V_LINE_MASK                                  (0xFFF << 16)
#define LCDC_TVGZM_V_LINE_SHIFT                                 (16)

// reg_4c
#define LCDC_TVC_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_TVC_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_TVC_OVSA_HPXL_SHIFT                                (0)
#define LCDC_TVC_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVC_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_TVC_OVSA_VLN_SHIFT                                 (16)

// reg_50
#define LCDC_TVC_PHXL(n)                                        (((n) & 0xFFF) << 0)
#define LCDC_TVC_PHXL_MASK                                      (0xFFF << 0)
#define LCDC_TVC_PHXL_SHIFT                                     (0)
#define LCDC_TVC_VLN(n)                                         (((n) & 0xFFF) << 16)
#define LCDC_TVC_VLN_MASK                                       (0xFFF << 16)
#define LCDC_TVC_VLN_SHIFT                                      (16)

// reg_54
#define LCDC_TV_H_TOTAL(n)                                      (((n) & 0xFFF) << 0)
#define LCDC_TV_H_TOTAL_MASK                                    (0xFFF << 0)
#define LCDC_TV_H_TOTAL_SHIFT                                   (0)
#define LCDC_TV_V_TOTAL(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_TV_V_TOTAL_MASK                                    (0xFFF << 16)
#define LCDC_TV_V_TOTAL_SHIFT                                   (16)
#define LCDC_BURSTSIZE_64_128_SEL_64                            (1 << 28)
#define LCDC_BOUNDARY_4K_1K_SEL_4K                              (1 << 29)

// reg_58
#define LCDC_TV_H_ACTIVE(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TV_H_ACTIVE_MASK                                   (0xFFF << 0)
#define LCDC_TV_H_ACTIVE_SHIFT                                  (0)
#define LCDC_TV_V_ACTIVE(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_TV_V_ACTIVE_MASK                                   (0xFFF << 16)
#define LCDC_TV_V_ACTIVE_SHIFT                                  (16)

// reg_5c
#define LCDC_TV_H_FPORCH(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TV_H_FPORCH_MASK                                   (0xFFF << 0)
#define LCDC_TV_H_FPORCH_SHIFT                                  (0)
#define LCDC_TV_H_BPORCH(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_TV_H_BPORCH_MASK                                   (0xFFF << 16)
#define LCDC_TV_H_BPORCH_SHIFT                                  (16)

// reg_60
#define LCDC_TV_V_FPORCH(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TV_V_FPORCH_MASK                                   (0xFFF << 0)
#define LCDC_TV_V_FPORCH_SHIFT                                  (0)
#define LCDC_TV_V_BPORCH(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_TV_V_BPORCH_MASK                                   (0xFFF << 16)
#define LCDC_TV_V_BPORCH_SHIFT                                  (16)

// reg_64
#define LCDC_TV_BLANKCOLOR(n)                                   (((n) & 0xFFFFFF) << 0)
#define LCDC_TV_BLANKCOLOR_MASK                                 (0xFFFFFF << 0)
#define LCDC_TV_BLANKCOLOR_SHIFT                                (0)

// reg_68
#define LCDC_TVC_COLOR1(n)                                      (((n) & 0xFFFFFF) << 0)
#define LCDC_TVC_COLOR1_MASK                                    (0xFFFFFF << 0)
#define LCDC_TVC_COLOR1_SHIFT                                   (0)

// reg_6c
#define LCDC_TVC_COLOR2(n)                                      (((n) & 0xFFFFFF) << 0)
#define LCDC_TVC_COLOR2_MASK                                    (0xFFFFFF << 0)
#define LCDC_TVC_COLOR2_SHIFT                                   (0)

// reg_70
#define LCDC_TV_ALPHA_Y(n)                                      (((n) & 0xFF) << 0)
#define LCDC_TV_ALPHA_Y_MASK                                    (0xFF << 0)
#define LCDC_TV_ALPHA_Y_SHIFT                                   (0)
#define LCDC_TV_CKEY_Y(n)                                       (((n) & 0xFF) << 8)
#define LCDC_TV_CKEY_Y_MASK                                     (0xFF << 8)
#define LCDC_TV_CKEY_Y_SHIFT                                    (8)
#define LCDC_TV_CKEY_Y1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_TV_CKEY_Y1_MASK                                    (0xFF << 16)
#define LCDC_TV_CKEY_Y1_SHIFT                                   (16)
#define LCDC_TV_CKEY_Y2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_TV_CKEY_Y2_MASK                                    (0xFF << 24)
#define LCDC_TV_CKEY_Y2_SHIFT                                   (24)

// reg_74
#define LCDC_TV_ALPHA_U(n)                                      (((n) & 0xFF) << 0)
#define LCDC_TV_ALPHA_U_MASK                                    (0xFF << 0)
#define LCDC_TV_ALPHA_U_SHIFT                                   (0)
#define LCDC_TV_CKEY_U(n)                                       (((n) & 0xFF) << 8)
#define LCDC_TV_CKEY_U_MASK                                     (0xFF << 8)
#define LCDC_TV_CKEY_U_SHIFT                                    (8)
#define LCDC_TV_CKEY_U1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_TV_CKEY_U1_MASK                                    (0xFF << 16)
#define LCDC_TV_CKEY_U1_SHIFT                                   (16)
#define LCDC_TV_CKEY_U2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_TV_CKEY_U2_MASK                                    (0xFF << 24)
#define LCDC_TV_CKEY_U2_SHIFT                                   (24)

// reg_78
#define LCDC_TV_ALPHA_V(n)                                      (((n) & 0xFF) << 0)
#define LCDC_TV_ALPHA_V_MASK                                    (0xFF << 0)
#define LCDC_TV_ALPHA_V_SHIFT                                   (0)
#define LCDC_TV_CKEY_V(n)                                       (((n) & 0xFF) << 8)
#define LCDC_TV_CKEY_V_MASK                                     (0xFF << 8)
#define LCDC_TV_CKEY_V_SHIFT                                    (8)
#define LCDC_TV_CKEY_V1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_TV_CKEY_V1_MASK                                    (0xFF << 16)
#define LCDC_TV_CKEY_V1_SHIFT                                   (16)
#define LCDC_TV_CKEY_V2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_TV_CKEY_V2_MASK                                    (0xFF << 24)
#define LCDC_TV_CKEY_V2_SHIFT                                   (24)

// reg_7c
#define LCDC_TV_V_SPXLCNT(n)                                    (((n) & 0xFFF) << 0)
#define LCDC_TV_V_SPXLCNT_MASK                                  (0xFFF << 0)
#define LCDC_TV_V_SPXLCNT_SHIFT                                 (0)
#define LCDC_TV_V_EPXLCNT(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TV_V_EPXLCNT_MASK                                  (0xFFF << 16)
#define LCDC_TV_V_EPXLCNT_SHIFT                                 (16)

// reg_80
#define LCDC_TVG_ENA                                            (1 << 0)
#define LCDC_YUV2RGB_TVG                                        (1 << 1)
#define LCDC_TVG_SWAPYU                                         (1 << 2)
#define LCDC_TVG_SWAPUV                                         (1 << 3)
#define LCDC_TVG_SWAPRB                                         (1 << 4)
#define LCDC_TVG_TSTMODE                                        (1 << 5)
#define LCDC_TVG_HSMOOTH                                        (1 << 6)
#define LCDC_TVG_FTOGGLE                                        (1 << 7)
#define LCDC_TVGFORMAT(n)                                       (((n) & 0xF) << 8)
#define LCDC_TVGFORMAT_MASK                                     (0xF << 8)
#define LCDC_TVGFORMAT_SHIFT                                    (8)
#define LCDC_TVC_1BITENA                                        (1 << 25)
#define LCDC_TVC_1BITMOD                                        (1 << 26)
#define LCDC_TV_PALETTE_ENA                                     (1 << 28)
#define LCDC_TV_CBSH_ENA                                        (1 << 29)
#define LCDC_TV_GAMMA_ENA                                       (1 << 30)
#define LCDC_TV_NOBLENDING                                      (1 << 31)

// reg_84
#define LCDC_TV_NI                                              (1 << 0)
#define LCDC_TV_RESETB                                          (1 << 1)
#define LCDC_TV_TVGOTOPN                                        (1 << 2)
#define LCDC_TV_PNGOTOTV                                        (1 << 3)
#define LCDC_IO_TVGOTOPN                                        (1 << 4)
#define LCDC_IO_PNGOTOTV                                        (1 << 5)
#define LCDC_TMSYNC_ENA                                         (1 << 6)
#define LCDC_TV_ALPHA(n)                                        (((n) & 0xFF) << 8)
#define LCDC_TV_ALPHA_MASK                                      (0xFF << 8)
#define LCDC_TV_ALPHA_SHIFT                                     (8)
#define LCDC_TV_ALPHA_CONST_MODE                                (0 << 16)
#define LCDC_TV_ALPHA_PIXEL_MODE                                (1 << 16)
#define LCDC_TV_CKEY_DMA                                        (1 << 18)
#define LCDC_TV_CKEY_GRA                                        (1 << 19)
#define LCDC_TV_CKEY_TVD                                        (1 << 20)
#define LCDC_TV_CKEY_TVG                                        (1 << 21)
#define LCDC_TV_CARRY                                           (1 << 23)
#define LCDC_TV_CKEY_MODE(n)                                    (((n) & 0x7) << 24)
#define LCDC_TV_CKEY_MODE_MASK                                  (0x7 << 24)
#define LCDC_TV_CKEY_MODE_SHIFT                                 (24)
#define LCDC_TVSYN_INV                                          (1 << 27)

// reg_88
#define LCDC_TV_CONTRAST(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_TV_CONTRAST_MASK                                   (0xFFFF << 0)
#define LCDC_TV_CONTRAST_SHIFT                                  (0)
#define LCDC_TV_BRIGHTNESS(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_TV_BRIGHTNESS_MASK                                 (0xFFFF << 16)
#define LCDC_TV_BRIGHTNESS_SHIFT                                (16)

// reg_8c
#define LCDC_TV_SATURATION(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_TV_SATURATION_MASK                                 (0xFFFF << 0)
#define LCDC_TV_SATURATION_SHIFT                                (0)
#define LCDC_TV_C_MULT_S(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_TV_C_MULT_S_MASK                                   (0xFFFF << 16)
#define LCDC_TV_C_MULT_S_SHIFT                                  (16)

// reg_90
#define LCDC_TV_COS0(n)                                         (((n) & 0xFFFF) << 0)
#define LCDC_TV_COS0_MASK                                       (0xFFFF << 0)
#define LCDC_TV_COS0_SHIFT                                      (0)
#define LCDC_TV_SIN0(n)                                         (((n) & 0xFFFF) << 16)
#define LCDC_TV_SIN0_MASK                                       (0xFFFF << 16)
#define LCDC_TV_SIN0_SHIFT                                      (16)

// reg_94
#define LCDC_TVIF_ENA                                           (1 << 0)
#define LCDC_TV_INV_TCLK                                        (1 << 1)
#define LCDC_TV_INV_HSYNC                                       (1 << 2)
#define LCDC_TV_INV_VSYNC                                       (1 << 3)
#define LCDC_TV_INV_HENA                                        (1 << 4)
#define LCDC_TV_INV_COMPSYNC                                    (1 << 5)
#define LCDC_TV_INV_COMPBLANK                                   (1 << 6)
#define LCDC_TV_REVERSERGB                                      (1 << 7)
#define LCDC_TV_BIASOUT                                         (1 << 8)
#define LCDC_INV_TCLK2X                                         (1 << 9)
#define LCDC_TV_INV_FIELD                                       (1 << 10)
#define LCDC_TVG_PMBURST                                        (1 << 11)
#define LCDC_AFF_WATERMARK(n)                                   (((n) & 0xFF) << 16)
#define LCDC_AFF_WATERMARK_MASK                                 (0xFF << 16)
#define LCDC_AFF_WATERMARK_SHIFT                                (16)
#define LCDC_HDMIMODE(n)                                        (((n) & 0xF) << 24)
#define LCDC_HDMIMODE_MASK                                      (0xF << 24)
#define LCDC_HDMIMODE_SHIFT                                     (24)
#define LCDC_TVIFMODE(n)                                        (((n) & 0xF) << 28)
#define LCDC_TVIFMODE_MASK                                      (0xF << 28)
#define LCDC_TVIFMODE_SHIFT                                     (28)

// reg_98

// reg_9c
#define LCDC_TCLK_DIV(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TCLK_DIV_MASK                                      (0xFFFFFFFF << 0)
#define LCDC_TCLK_DIV_SHIFT                                     (0)

// reg_c0
#define LCDC_DMA_SA_Y0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_Y0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_Y0_SHIFT                                    (0)

// reg_c4
#define LCDC_DMA_SA_U0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_U0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_U0_SHIFT                                    (0)

// reg_c8
#define LCDC_DMA_SA_V0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_V0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_V0_SHIFT                                    (0)

// reg_cc
#define LCDC_DMA_SA_C0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_C0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_C0_SHIFT                                    (0)

// reg_d0
#define LCDC_DMA_SA_Y1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_Y1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_Y1_SHIFT                                    (0)

// reg_d4
#define LCDC_DMA_SA_U1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_U1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_U1_SHIFT                                    (0)

// reg_d8
#define LCDC_DMA_SA_V1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_V1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_V1_SHIFT                                    (0)

// reg_dc
#define LCDC_DMA_SA_C1(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_SA_C1_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_DMA_SA_C1_SHIFT                                    (0)

// reg_e0
#define LCDC_DMA_PITCH_Y(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_DMA_PITCH_Y_MASK                                   (0xFFFF << 0)
#define LCDC_DMA_PITCH_Y_SHIFT                                  (0)
#define LCDC_DMA_PITCH_C(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_DMA_PITCH_C_MASK                                   (0xFFFF << 16)
#define LCDC_DMA_PITCH_C_SHIFT                                  (16)

// reg_e4
#define LCDC_DMA_PITCH_U(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_DMA_PITCH_U_MASK                                   (0xFFFF << 0)
#define LCDC_DMA_PITCH_U_SHIFT                                  (0)
#define LCDC_DMA_PITCH_V(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_DMA_PITCH_V_MASK                                   (0xFFFF << 16)
#define LCDC_DMA_PITCH_V_SHIFT                                  (16)

// reg_e8
#define LCDC_DMA_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_DMA_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_DMA_OVSA_HPXL_SHIFT                                (0)
#define LCDC_DMAFRM_Y                                           (1 << 12)
#define LCDC_DMAFRM_U                                           (1 << 13)
#define LCDC_DMAFRM_V                                           (1 << 14)
#define LCDC_DMAFRM_C                                           (1 << 15)
#define LCDC_DMA_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_DMA_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_DMA_OVSA_VLN_SHIFT                                 (16)
#define LCDC_DMAFRM_FIX                                         (1 << 31)

// reg_ec
#define LCDC_DMA_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_DMA_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_DMA_H_PIXEL_SHIFT                                  (0)
#define LCDC_DMA_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_DMA_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_DMA_V_LINE_SHIFT                                   (16)

// reg_f0
#define LCDC_DMAZM_H_PIXEL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_DMAZM_H_PIXEL_MASK                                 (0xFFF << 0)
#define LCDC_DMAZM_H_PIXEL_SHIFT                                (0)
#define LCDC_DMAZM_V_LINE(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_DMAZM_V_LINE_MASK                                  (0xFFF << 16)
#define LCDC_DMAZM_V_LINE_SHIFT                                 (16)

// reg_f4
#define LCDC_GRA_SA0(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA_SA0_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_GRA_SA0_SHIFT                                      (0)

// reg_f8
#define LCDC_GRA_SA1(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA_SA1_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_GRA_SA1_SHIFT                                      (0)

// reg_fc
#define LCDC_GRA_PITCH(n)                                       (((n) & 0xFFFF) << 0)
#define LCDC_GRA_PITCH_MASK                                     (0xFFFF << 0)
#define LCDC_GRA_PITCH_SHIFT                                    (0)
#define LCDC_PN_BKLIGHTDIV(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_PN_BKLIGHTDIV_MASK                                 (0xFFFF << 16)
#define LCDC_PN_BKLIGHTDIV_SHIFT                                (16)

// reg_100
#define LCDC_GRA_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_GRA_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_GRA_OVSA_HPXL_SHIFT                                (0)
#define LCDC_GRAFRM_Y                                           (1 << 12)
#define LCDC_GRA_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_GRA_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_GRA_OVSA_VLN_SHIFT                                 (16)
#define LCDC_GRAFRM_FIX                                         (1 << 31)

// reg_104
#define LCDC_GRA_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_GRA_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_GRA_H_PIXEL_SHIFT                                  (0)
#define LCDC_GRA_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_GRA_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_GRA_V_LINE_SHIFT                                   (16)

// reg_108
#define LCDC_GRAZM_H_PIXEL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_GRAZM_H_PIXEL_MASK                                 (0xFFF << 0)
#define LCDC_GRAZM_H_PIXEL_SHIFT                                (0)
#define LCDC_GRAZM_V_LINE(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_GRAZM_V_LINE_MASK                                  (0xFFF << 16)
#define LCDC_GRAZM_V_LINE_SHIFT                                 (16)

// reg_10c
#define LCDC_HWC_OVSA_HPXL(n)                                   (((n) & 0xFFF) << 0)
#define LCDC_HWC_OVSA_HPXL_MASK                                 (0xFFF << 0)
#define LCDC_HWC_OVSA_HPXL_SHIFT                                (0)
#define LCDC_HWC_OVSA_VLN(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_HWC_OVSA_VLN_MASK                                  (0xFFF << 16)
#define LCDC_HWC_OVSA_VLN_SHIFT                                 (16)

// reg_110
#define LCDC_HWC_HPXL(n)                                        (((n) & 0xFFF) << 0)
#define LCDC_HWC_HPXL_MASK                                      (0xFFF << 0)
#define LCDC_HWC_HPXL_SHIFT                                     (0)
#define LCDC_HWC_VLN(n)                                         (((n) & 0xFFF) << 16)
#define LCDC_HWC_VLN_MASK                                       (0xFFF << 16)
#define LCDC_HWC_VLN_SHIFT                                      (16)

// reg_114
#define LCDC_PN_H_TOTAL(n)                                      (((n) & 0xFFF) << 0)
#define LCDC_PN_H_TOTAL_MASK                                    (0xFFF << 0)
#define LCDC_PN_H_TOTAL_SHIFT                                   (0)
#define LCDC_PN_V_TOTAL(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_PN_V_TOTAL_MASK                                    (0xFFF << 16)
#define LCDC_PN_V_TOTAL_SHIFT                                   (16)

// reg_118
#define LCDC_PN_H_ACTIVE(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_PN_H_ACTIVE_MASK                                   (0xFFF << 0)
#define LCDC_PN_H_ACTIVE_SHIFT                                  (0)
#define LCDC_PN_V_ACTIVE(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_PN_V_ACTIVE_MASK                                   (0xFFF << 16)
#define LCDC_PN_V_ACTIVE_SHIFT                                  (16)

// reg_11c
#define LCDC_PN_H_FPORCH(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_PN_H_FPORCH_MASK                                   (0xFFF << 0)
#define LCDC_PN_H_FPORCH_SHIFT                                  (0)
#define LCDC_PN_H_BPORCH(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_PN_H_BPORCH_MASK                                   (0xFFF << 16)
#define LCDC_PN_H_BPORCH_SHIFT                                  (16)

// reg_120
#define LCDC_PN_V_FPORCH(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_PN_V_FPORCH_MASK                                   (0xFFF << 0)
#define LCDC_PN_V_FPORCH_SHIFT                                  (0)
#define LCDC_PN_V_BPORCH(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_PN_V_BPORCH_MASK                                   (0xFFF << 16)
#define LCDC_PN_V_BPORCH_SHIFT                                  (16)

// reg_124
#define LCDC_PN_BLANKCOLOR(n)                                   (((n) & 0xFFFFFF) << 0)
#define LCDC_PN_BLANKCOLOR_MASK                                 (0xFFFFFF << 0)
#define LCDC_PN_BLANKCOLOR_SHIFT                                (0)

// reg_128
#define LCDC_HWC_COLOR1(n)                                      (((n) & 0xFFFFFF) << 0)
#define LCDC_HWC_COLOR1_MASK                                    (0xFFFFFF << 0)
#define LCDC_HWC_COLOR1_SHIFT                                   (0)

// reg_12c
#define LCDC_HWC_COLOR2(n)                                      (((n) & 0xFFFFFF) << 0)
#define LCDC_HWC_COLOR2_MASK                                    (0xFFFFFF << 0)
#define LCDC_HWC_COLOR2_SHIFT                                   (0)

// reg_130
#define LCDC_PN_ALPHA_Y(n)                                      (((n) & 0xFF) << 0)
#define LCDC_PN_ALPHA_Y_MASK                                    (0xFF << 0)
#define LCDC_PN_ALPHA_Y_SHIFT                                   (0)
#define LCDC_PN_CKEY_Y(n)                                       (((n) & 0xFF) << 8)
#define LCDC_PN_CKEY_Y_MASK                                     (0xFF << 8)
#define LCDC_PN_CKEY_Y_SHIFT                                    (8)
#define LCDC_PN_CKEY_Y1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_PN_CKEY_Y1_MASK                                    (0xFF << 16)
#define LCDC_PN_CKEY_Y1_SHIFT                                   (16)
#define LCDC_PN_CKEY_Y2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_PN_CKEY_Y2_MASK                                    (0xFF << 24)
#define LCDC_PN_CKEY_Y2_SHIFT                                   (24)

// reg_134
#define LCDC_PN_ALPHA_U(n)                                      (((n) & 0xFF) << 0)
#define LCDC_PN_ALPHA_U_MASK                                    (0xFF << 0)
#define LCDC_PN_ALPHA_U_SHIFT                                   (0)
#define LCDC_PN_CKEY_U(n)                                       (((n) & 0xFF) << 8)
#define LCDC_PN_CKEY_U_MASK                                     (0xFF << 8)
#define LCDC_PN_CKEY_U_SHIFT                                    (8)
#define LCDC_PN_CKEY_U1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_PN_CKEY_U1_MASK                                    (0xFF << 16)
#define LCDC_PN_CKEY_U1_SHIFT                                   (16)
#define LCDC_PN_CKEY_U2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_PN_CKEY_U2_MASK                                    (0xFF << 24)
#define LCDC_PN_CKEY_U2_SHIFT                                   (24)

// reg_138
#define LCDC_PN_ALPHA_V(n)                                      (((n) & 0xFF) << 0)
#define LCDC_PN_ALPHA_V_MASK                                    (0xFF << 0)
#define LCDC_PN_ALPHA_V_SHIFT                                   (0)
#define LCDC_PN_CKEY_V(n)                                       (((n) & 0xFF) << 8)
#define LCDC_PN_CKEY_V_MASK                                     (0xFF << 8)
#define LCDC_PN_CKEY_V_SHIFT                                    (8)
#define LCDC_PN_CKEY_V1(n)                                      (((n) & 0xFF) << 16)
#define LCDC_PN_CKEY_V1_MASK                                    (0xFF << 16)
#define LCDC_PN_CKEY_V1_SHIFT                                   (16)
#define LCDC_PN_CKEY_V2(n)                                      (((n) & 0xFF) << 24)
#define LCDC_PN_CKEY_V2_MASK                                    (0xFF << 24)
#define LCDC_PN_CKEY_V2_SHIFT                                   (24)

// reg_13c
#define LCDC_PN_V_SPXLCNT(n)                                    (((n) & 0xFFF) << 0)
#define LCDC_PN_V_SPXLCNT_MASK                                  (0xFFF << 0)
#define LCDC_PN_V_SPXLCNT_SHIFT                                 (0)
#define LCDC_PN_V_EPXLCNT(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_PN_V_EPXLCNT_MASK                                  (0xFFF << 16)
#define LCDC_PN_V_EPXLCNT_SHIFT                                 (16)
#define LCDC_DBG_RD_INDEX(n)                                    (((n) & 0xF) << 28)
#define LCDC_DBG_RD_INDEX_MASK                                  (0xF << 28)
#define LCDC_DBG_RD_INDEX_SHIFT                                 (28)

// reg_140
#define LCDC_SPI_RXDATA(n)                                      (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SPI_RXDATA_MASK                                    (0xFFFFFFFF << 0)
#define LCDC_SPI_RXDATA_SHIFT                                   (0)

// reg_144
#define LCDC_ISA_RXDATA(n)                                      (((n) & 0xFFFFFFFF) << 0)
#define LCDC_ISA_RXDATA_MASK                                    (0xFFFFFFFF << 0)
#define LCDC_ISA_RXDATA_SHIFT                                   (0)

// reg_148
#define LCDC_PN_IOPAD_I(n)                                      (((n) & 0xFFFFFFF) << 0)
#define LCDC_PN_IOPAD_I_MASK                                    (0xFFFFFFF << 0)
#define LCDC_PN_IOPAD_I_SHIFT                                   (0)

// reg_14c
#define LCDC_DMAVLD_HLEN_Y(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_DMAVLD_HLEN_Y_MASK                                 (0xFFFF << 0)
#define LCDC_DMAVLD_HLEN_Y_SHIFT                                (0)
#define LCDC_TVD_SA_CFLAG                                       (1 << 22)
#define LCDC_DMA_SA_CFLAG                                       (1 << 23)
#define LCDC_TVG_SA_YFLAG                                       (1 << 24)
#define LCDC_TVD_SA_VFLAG                                       (1 << 25)
#define LCDC_TVD_SA_UFLAG                                       (1 << 26)
#define LCDC_TVD_SA_YFLAG                                       (1 << 27)
#define LCDC_GRA_SA_YFLAG                                       (1 << 28)
#define LCDC_DMA_SA_VFLAG                                       (1 << 29)
#define LCDC_DMA_SA_UFLAG                                       (1 << 30)
#define LCDC_DMA_SA_YFLAG                                       (1 << 31)

// reg_150
#define LCDC_DMAVLD_HLEN_U(n)                                   (((n) & 0x3FF) << 0)
#define LCDC_DMAVLD_HLEN_U_MASK                                 (0x3FF << 0)
#define LCDC_DMAVLD_HLEN_U_SHIFT                                (0)
#define LCDC_DMAVLD_HLEN_V(n)                                   (((n) & 0x3FF) << 10)
#define LCDC_DMAVLD_HLEN_V_MASK                                 (0x3FF << 10)
#define LCDC_DMAVLD_HLEN_V_SHIFT                                (10)

// reg_154
#define LCDC_GRAVLD_HLEN(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_GRAVLD_HLEN_MASK                                   (0xFFFF << 0)
#define LCDC_GRAVLD_HLEN_SHIFT                                  (0)
#define LCDC_TVGVLD_HLEN(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_TVGVLD_HLEN_MASK                                   (0xFFFF << 16)
#define LCDC_TVGVLD_HLEN_SHIFT                                  (16)

// reg_158
#define LCDC_HWC_RDDAT(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_HWC_RDDAT_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_HWC_RDDAT_SHIFT                                    (0)

// reg_15c
#define LCDC_PN_GAMMA_RDDAT(n)                                  (((n) & 0xFF) << 0)
#define LCDC_PN_GAMMA_RDDAT_MASK                                (0xFF << 0)
#define LCDC_PN_GAMMA_RDDAT_SHIFT                               (0)

// reg_160
#define LCDC_PN_PALETTE_RDDAT(n)                                (((n) & 0xFFFFFF) << 0)
#define LCDC_PN_PALETTE_RDDAT_MASK                              (0xFFFFFF << 0)
#define LCDC_PN_PALETTE_RDDAT_SHIFT                             (0)

// reg_164
#define LCDC_SLV_DATACNT(n)                                     (((n) & 0xFF) << 4)
#define LCDC_SLV_DATACNT_MASK                                   (0xFF << 4)
#define LCDC_SLV_DATACNT_SHIFT                                  (4)
#define LCDC_SLV_SPACECNT(n)                                    (((n) & 0xFF) << 12)
#define LCDC_SLV_SPACECNT_MASK                                  (0xFF << 12)
#define LCDC_SLV_SPACECNT_SHIFT                                 (12)

// reg_168
#define LCDC_TVDVLD_HLEN_Y(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_TVDVLD_HLEN_Y_MASK                                 (0xFFFF << 0)
#define LCDC_TVDVLD_HLEN_Y_SHIFT                                (0)

// reg_16c
#define LCDC_DMAVLD_HLEN_U(n)                                   (((n) & 0x3FF) << 0)
#define LCDC_DMAVLD_HLEN_U_MASK                                 (0x3FF << 0)
#define LCDC_DMAVLD_HLEN_U_SHIFT                                (0)
#define LCDC_DMAVLD_HLEN_V(n)                                   (((n) & 0x3FF) << 10)
#define LCDC_DMAVLD_HLEN_V_MASK                                 (0x3FF << 10)
#define LCDC_DMAVLD_HLEN_V_SHIFT                                (10)

// reg_170
#define LCDC_TVC_RDDAT(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TVC_RDDAT_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TVC_RDDAT_SHIFT                                    (0)

// reg_174
#define LCDC_TV_GAMMA_RDDAT(n)                                  (((n) & 0xFF) << 0)
#define LCDC_TV_GAMMA_RDDAT_MASK                                (0xFF << 0)
#define LCDC_TV_GAMMA_RDDAT_SHIFT                               (0)

// reg_178
#define LCDC_TV_PALETTE_RDDAT(n)                                (((n) & 0xFFFFFF) << 0)
#define LCDC_TV_PALETTE_RDDAT_MASK                              (0xFFFFFF << 0)
#define LCDC_TV_PALETTE_RDDAT_SHIFT                             (0)

// reg_17c
#define LCDC_DMA_FRAME_CNT(n)                                   (((n) & 0x3) << 0)
#define LCDC_DMA_FRAME_CNT_MASK                                 (0x3 << 0)
#define LCDC_DMA_FRAME_CNT_SHIFT                                (0)
#define LCDC_GRA_FRAME_CNT(n)                                   (((n) & 0x3) << 2)
#define LCDC_GRA_FRAME_CNT_MASK                                 (0x3 << 2)
#define LCDC_GRA_FRAME_CNT_SHIFT                                (2)
#define LCDC_TVD_FRAME_CNT(n)                                   (((n) & 0x3) << 4)
#define LCDC_TVD_FRAME_CNT_MASK                                 (0x3 << 4)
#define LCDC_TVD_FRAME_CNT_SHIFT                                (4)
#define LCDC_TVG_FRAME_CNT(n)                                   (((n) & 0x3) << 6)
#define LCDC_TVG_FRAME_CNT_MASK                                 (0x3 << 6)
#define LCDC_TVG_FRAME_CNT_SHIFT                                (6)
#define LCDC_PN_CCIC_FRAME_CNT(n)                               (((n) & 0x3) << 8)
#define LCDC_PN_CCIC_FRAME_CNT_MASK                             (0x3 << 8)
#define LCDC_PN_CCIC_FRAME_CNT_SHIFT                            (8)
#define LCDC_PN_IRE_FRAME_CNT(n)                                (((n) & 0x3) << 10)
#define LCDC_PN_IRE_FRAME_CNT_MASK                              (0x3 << 10)
#define LCDC_PN_IRE_FRAME_CNT_SHIFT                             (10)
#define LCDC_TV_CCIC_FRAME_CNT(n)                               (((n) & 0x3) << 12)
#define LCDC_TV_CCIC_FRAME_CNT_MASK                             (0x3 << 12)
#define LCDC_TV_CCIC_FRAME_CNT_SHIFT                            (12)
#define LCDC_TV_IRE_FRAME_CNT(n)                                (((n) & 0x3) << 14)
#define LCDC_TV_IRE_FRAME_CNT_MASK                              (0x3 << 14)
#define LCDC_TV_IRE_FRAME_CNT_SHIFT                             (14)
#define LCDC_SP_FRAME_CNT(n)                                    (((n) & 0x3) << 18)
#define LCDC_SP_FRAME_CNT_MASK                                  (0x3 << 18)
#define LCDC_SP_FRAME_CNT_SHIFT                                 (18)
#define LCDC_PN_FRAME_CNT(n)                                    (((n) & 0x3) << 20)
#define LCDC_PN_FRAME_CNT_MASK                                  (0x3 << 20)
#define LCDC_PN_FRAME_CNT_SHIFT                                 (20)
#define LCDC_TV_FRAME_CNT(n)                                    (((n) & 0x3) << 22)
#define LCDC_TV_FRAME_CNT_MASK                                  (0x3 << 22)
#define LCDC_TV_FRAME_CNT_SHIFT                                 (22)

// reg_180
#define LCDC_SPI_START                                          (1 << 0)
#define LCDC_SPI_3W4WB                                          (1 << 1)
#define LCDC_SPI_READ_ENA                                       (1 << 1)
#define LCDC_SPI_SEL                                            (1 << 2)
#define LCDC_SPI_ENA                                            (1 << 3)
#define LCDC_TXBITSTO0                                          (1 << 4)
#define LCDC_RXBITSTO0                                          (1 << 5)
#define LCDC_SPI_KEEPXFER                                       (1 << 6)
#define LCDC_CLKINV                                             (1 << 7)
#define LCDC_TXBITS(n)                                          (((n) & 0xFF) << 8)
#define LCDC_TXBITS_MASK                                        (0xFF << 8)
#define LCDC_TXBITS_SHIFT                                       (8)
#define LCDC_RXBITS(n)                                          (((n) & 0xFF) << 16)
#define LCDC_RXBITS_MASK                                        (0xFF << 16)
#define LCDC_RXBITS_SHIFT                                       (16)
#define LCDC_SCLKCNT(n)                                         (((n) & 0xFF) << 24)
#define LCDC_SCLKCNT_MASK                                       (0xFF << 24)
#define LCDC_SCLKCNT_SHIFT                                      (24)

// reg_184
#define LCDC_SPI_TXDATA(n)                                      (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SPI_TXDATA_MASK                                    (0xFFFFFFFF << 0)
#define LCDC_SPI_TXDATA_SHIFT                                   (0)

// reg_188
#define LCDC_SMPN_ENA                                           (1 << 0)
#define LCDC_KEEPXFER                                           (1 << 1)
#define LCDC_68S80SB                                            (1 << 2)
#define LCDC_SMPN_RSTB                                          (1 << 3)
#define LCDC_SLV_ENA                                            (1 << 4)
#define LCDC_SMPN_SEL                                           (1 << 5)
#define LCDC_SWAPBYTES                                          (1 << 6)
#define LCDC_CMD32OR16B                                         (1 << 7)
#define LCDC_SMPNMODE(n)                                        (((n) & 0xF) << 8)
#define LCDC_SMPNMODE_MASK                                      (0xF << 8)
#define LCDC_SMPNMODE_SHIFT                                     (8)
#define LCDC_SLV_ONLY                                           (1 << 12)
#define LCDC_IORDY_MSK                                          (1 << 13)
#define LCDC_SMPNVSYNC(n)                                       (((n) & 0x3) << 14)
#define LCDC_SMPNVSYNC_MASK                                     (0x3 << 14)
#define LCDC_SMPNVSYNC_SHIFT                                    (14)
#define LCDC_ISA_TXHIGH(n)                                      (((n) & 0xF) << 16)
#define LCDC_ISA_TXHIGH_MASK                                    (0xF << 16)
#define LCDC_ISA_TXHIGH_SHIFT                                   (16)
#define LCDC_ISA_TXLOW(n)                                       (((n) & 0xF) << 20)
#define LCDC_ISA_TXLOW_MASK                                     (0xF << 20)
#define LCDC_ISA_TXLOW_SHIFT                                    (20)
#define LCDC_ISA_RXHIGH(n)                                      (((n) & 0xF) << 24)
#define LCDC_ISA_RXHIGH_MASK                                    (0xF << 24)
#define LCDC_ISA_RXHIGH_SHIFT                                   (24)
#define LCDC_ISA_RXLOW(n)                                       (((n) & 0xF) << 28)
#define LCDC_ISA_RXLOW_MASK                                     (0xF << 28)
#define LCDC_ISA_RXLOW_SHIFT                                    (28)

// reg_18c
#define LCDC_SLVD_COMMAND0_DATA(n)                              (((n) & 0xFF) << 0)
#define LCDC_SLVD_COMMAND0_DATA_MASK                            (0xFF << 0)
#define LCDC_SLVD_COMMAND0_DATA_SHIFT                           (0)
#define LCDC_SLV_COMMAND0_WRITE                                 (1 << 8)
#define LCDC_SLV_COMMAND0_READ                                  (1 << 9)
#define LCDC_32_CMD_FMT_DATA(n)                                 (((n) & 0x1F) << 10)
#define LCDC_32_CMD_FMT_DATA_MASK                               (0x1F << 10)
#define LCDC_32_CMD_FMT_DATA_SHIFT                              (10)
#define LCDC_SLV_COMMAND0_A0                                    (1 << 15)
#define LCDC_SLV_COMMAND1_DATA(n)                               (((n) & 0xFF) << 16)
#define LCDC_SLV_COMMAND1_DATA_MASK                             (0xFF << 16)
#define LCDC_SLV_COMMAND1_DATA_SHIFT                            (16)
#define LCDC_SLV_COMMAND1_WRITE                                 (1 << 24)
#define LCDC_SLV_COMMAND1_READ                                  (1 << 25)
#define LCDC_COMMAND_A_A0                                       (1 << 31)

// reg_190
#define LCDC_DMA_ENA                                            (1 << 0)
#define LCDC_YUV2RGB_DMA                                        (1 << 1)
#define LCDC_DMA_SWAPYU                                         (1 << 2)
#define LCDC_DMA_SWAPUV                                         (1 << 3)
#define LCDC_DMA_SWAPRB                                         (1 << 4)
#define LCDC_DMA_TSTMODE                                        (1 << 5)
#define LCDC_DMA_HSMOOTH                                        (1 << 6)
#define LCDC_DMA_FTOGGLE                                        (1 << 7)
#define LCDC_DMAFORMAT(n)                                       (((n) & 0xF) << 8)
#define LCDC_DMAFORMAT_MASK                                     (0xF << 8)
#define LCDC_DMAFORMAT_SHIFT                                    (8)

// reg_194
#define LCDC_PXLMD(n)                                           (((n) & 0xFF) << 0)
#define LCDC_PXLMD_MASK                                         (0xFF << 0)
#define LCDC_PXLMD_SHIFT                                        (0)
#define LCDC_PN_ALPHA(n)                                        (((n) & 0xFF) << 8)
#define LCDC_PN_ALPHA_MASK                                      (0xFF << 8)
#define LCDC_PN_ALPHA_SHIFT                                     (8)
#define LCDC_PN_ALPHA_CONST_MODE                                (0 << 16)
#define LCDC_PN_ALPHA_PIXEL_MODE                                (1 << 16)
#define LCDC_DSCALE(n)                                          (((n) & 0x3) << 18)
#define LCDC_DSCALE_MASK                                        (0x3 << 18)
#define LCDC_DSCALE_SHIFT                                       (18)
#define LCDC_PWRDN_ENA                                          (1 << 20)
#define LCDC_GATED_ENA                                          (1 << 21)
#define LCDC_LNBUF_ENA                                          (1 << 22)
#define LCDC_PN_CARRY                                           (1 << 23)
#define LCDC_PN_KEY_MODE(n)                                     (((n) & 0x7) << 24)
#define LCDC_PN_KEY_MODE_MASK                                   (0x7 << 24)
#define LCDC_PN_KEY_MODE_SHIFT                                  (24)
#define LCDC_PN_VSYNC_INV                                       (1 << 27)
#define LCDC_PN_VSYNC_MODE(n)                                   (((n) & 0x7) << 28)
#define LCDC_PN_VSYNC_MODE_MASK                                 (0x7 << 28)
#define LCDC_PN_VSYNC_MODE_SHIFT                                (28)
#define LCDC_FRAME_TRIG                                         (1 << 31)

// reg_198
#define LCDC_SRAM_ADDR(n)                                       (((n) & 0xFF) << 0)
#define LCDC_SRAM_ADDR_MASK                                     (0xFF << 0)
#define LCDC_SRAM_ADDR_SHIFT                                    (0)
#define LCDC_SRAM_TYPE(n)                                       (((n) & 0xF) << 8) /* 0: PN GammaR
                                                                                    * 1: PN GammaG
                                                                                    * 2: PN GammaB
                                                                                    * 3: Color RGB
                                                                                    * 4: TV GammaR
                                                                                    * 5: TV GammaG
                                                                                    * 6: TV GammaB
                                                                                    */
#define LCDC_SRAM_TYPE_MASK                                     (0xF << 8)
#define LCDC_SRAM_TYPE_SHIFT                                    (8)
#define LCDC_SRAM_CTRL(n)                                       (((n) & 0x3) << 14) /* 0: Read
                                                                                     * 2: Write
                                                                                     * 3: Init
                                                                                     */
#define LCDC_SRAM_CTRL_MASK                                     (0x3 << 14)
#define LCDC_SRAM_CTRL_SHIFT                                    (14)

// reg_19c
#define LCDC_SRAM_DATA_COLOR(n)                                 (((n) & 0xFFFFFF) << 0)
#define LCDC_SRAM_DATA_COLOR_MASK                               (0xFFFFFF << 0)
#define LCDC_SRAM_DATA_COLOR_SHIFT                              (0)
#define LCDC_SRAM_DATA_GAMMA(n)                                 (((n) & 0xFF) << 0)
#define LCDC_SRAM_DATA_GAMMA_MASK                               (0xFF << 0)
#define LCDC_SRAM_DATA_GAMMA_SHIFT                              (0)

// reg_1a0
#define LCDC_RTC128X66(n)                                       (((n) & 0x3) << 0)
#define LCDC_RTC128X66_MASK                                     (0x3 << 0)
#define LCDC_RTC128X66_SHIFT                                    (0)
#define LCDC_WTC128X66(n)                                       (((n) & 0x3) << 2)
#define LCDC_WTC128X66_MASK                                     (0x3 << 2)
#define LCDC_WTC128X66_SHIFT                                    (2)
#define LCDC_RTC64X66(n)                                        (((n) & 0x3) << 4)
#define LCDC_RTC64X66_MASK                                      (0x3 << 4)
#define LCDC_RTC64X66_SHIFT                                     (4)
#define LCDC_WTC64X66(n)                                        (((n) & 0x3) << 6)
#define LCDC_WTC64X66_MASK                                      (0x3 << 6)
#define LCDC_WTC64X66_SHIFT                                     (6)
#define LCDC_RTC32X66(n)                                        (((n) & 0x3) << 8)
#define LCDC_RTC32X66_MASK                                      (0x3 << 8)
#define LCDC_RTC32X66_SHIFT                                     (8)
#define LCDC_WTC32X66(n)                                        (((n) & 0x3) << 10)
#define LCDC_WTC32X66_MASK                                      (0x3 << 10)
#define LCDC_WTC32X66_SHIFT                                     (10)
#define LCDC_RTC32X32(n)                                        (((n) & 0x3) << 12)
#define LCDC_RTC32X32_MASK                                      (0x3 << 12)
#define LCDC_RTC32X32_SHIFT                                     (12)
#define LCDC_WTC32X32(n)                                        (((n) & 0x3) << 14)
#define LCDC_WTC32X32_MASK                                      (0x3 << 14)
#define LCDC_WTC32X32_SHIFT                                     (14)
#define LCDC_RTC256X8(n)                                        (((n) & 0x3) << 20)
#define LCDC_RTC256X8_MASK                                      (0x3 << 20)
#define LCDC_RTC256X8_SHIFT                                     (20)
#define LCDC_WTC256X8(n)                                        (((n) & 0x3) << 22)
#define LCDC_WTC256X8_MASK                                      (0x3 << 22)
#define LCDC_WTC256X8_SHIFT                                     (22)
#define LCDC_RTC256X24(n)                                       (((n) & 0x3) << 24)
#define LCDC_RTC256X24_MASK                                     (0x3 << 24)
#define LCDC_RTC256X24_SHIFT                                    (24)
#define LCDC_WTC256X24(n)                                       (((n) & 0x3) << 26)
#define LCDC_WTC256X24_MASK                                     (0x3 << 26)
#define LCDC_WTC256X24_SHIFT                                    (26)
#define LCDC_RTC256X32(n)                                       (((n) & 0x3) << 28)
#define LCDC_RTC256X32_MASK                                     (0x3 << 28)
#define LCDC_RTC256X32_SHIFT                                    (28)
#define LCDC_WTC256X32(n)                                       (((n) & 0x3) << 30)
#define LCDC_WTC256X32_MASK                                     (0x3 << 30)
#define LCDC_WTC256X32_SHIFT                                    (30)

// reg_1a4
#define LCDC_PDWN128X66                                         (1 << 0)
#define LCDC_PDWN64X66                                          (1 << 1)
#define LCDC_PDWN32X66                                          (1 << 2)
#define LCDC_PDWN32X32                                          (1 << 3)
#define LCDC_PDWN256X8                                          (1 << 5)
#define LCDC_PDWN256X24                                         (1 << 6)
#define LCDC_PDWN256X32                                         (1 << 7)
#define LCDC_CSB_256X8                                          (1 << 13)
#define LCDC_CSB_256X24                                         (1 << 14)
#define LCDC_CSB_256X32                                         (1 << 15)

// reg_1a8
#define LCDC_CLK_INT_DIV(n)                                     (((n) & 0xFF) << 0)
#define LCDC_CLK_INT_DIV_MASK                                   (0xFF << 0)
#define LCDC_CLK_INT_DIV_SHIFT                                  (0)
#define LCDC_DSI1_BITCLK_DIV(n)                                 (((n) & 0xF) << 8)
#define LCDC_DSI1_BITCLK_DIV_MASK                               (0xF << 8)
#define LCDC_DSI1_BITCLK_DIV_SHIFT                              (8)
#define LCDC_CLK_FRAC_DIV(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_CLK_FRAC_DIV_MASK                                  (0xFFF << 16)
#define LCDC_CLK_FRAC_DIV_SHIFT                                 (16)
#define LCDC_SCLK_DISABLE                                       (1 << 28)
#define LCDC_SCLK_SOURCE_SELECT(n)                              (((n) & 0x3) << 30)
#define LCDC_SCLK_SOURCE_SELECT_MASK                            (0x3 << 30)
#define LCDC_SCLK_SOURCE_SELECT_SHIFT                           (30)

// reg_1ac
#define LCDC_PN_CONTRAST(n)                                     (((n) & 0xFFFF) << 0)
#define LCDC_PN_CONTRAST_MASK                                   (0xFFFF << 0)
#define LCDC_PN_CONTRAST_SHIFT                                  (0)
#define LCDC_PN_BRIGHTNESS(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_PN_BRIGHTNESS_MASK                                 (0xFFFF << 16)
#define LCDC_PN_BRIGHTNESS_SHIFT                                (16)

// reg_1b0
#define LCDC_PN_SATURATION(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_PN_SATURATION_MASK                                 (0xFFFF << 0)
#define LCDC_PN_SATURATION_SHIFT                                (0)
#define LCDC_PN_C_MULT_S(n)                                     (((n) & 0xFFFF) << 16)
#define LCDC_PN_C_MULT_S_MASK                                   (0xFFFF << 16)
#define LCDC_PN_C_MULT_S_SHIFT                                  (16)

// reg_1b4
#define LCDC_PN_COS0(n)                                         (((n) & 0xFFFF) << 0)
#define LCDC_PN_COS0_MASK                                       (0xFFFF << 0)
#define LCDC_PN_COS0_SHIFT                                      (0)
#define LCDC_PN_SIN0(n)                                         (((n) & 0xFFFF) << 16)
#define LCDC_PN_SIN0_MASK                                       (0xFFFF << 16)
#define LCDC_PN_SIN0_SHIFT                                      (16)

// reg_1b8
#define LCDC_DUMB_ENA                                           (1 << 0)
#define LCDC_PN_INV_PCLK                                        (1 << 1)
#define LCDC_PN_INV_HSYNC                                       (1 << 2)
#define LCDC_PN_INV_VSYNC                                       (1 << 3)
#define LCDC_PN_INV_HENA                                        (1 << 4)
#define LCDC_PN_INV_COMPSYNC                                    (1 << 5)
#define LCDC_PN_INV_COMPBLANK                                   (1 << 6)
#define LCDC_PN_REVERSE_RGB                                     (1 << 7)
#define LCDC_PN_BIASOUT                                         (1 << 8)
#define LCDC_PN_NO_IOPAD                                        (1 << 9)
#define LCDC_GRA_PMBURST                                        (1 << 11)
#define LCDC_LCDGPIO_ENA(n)                                     (((n) & 0xFF) << 12)
#define LCDC_LCDGPIO_ENA_MASK                                   (0xFF << 12)
#define LCDC_LCDGPIO_ENA_SHIFT                                  (12)
#define LCDC_LCDGPIO_O(n)                                       (((n) & 0xFF) << 20)
#define LCDC_LCDGPIO_O_MASK                                     (0xFF << 20)
#define LCDC_LCDGPIO_O_SHIFT                                    (20)
#define LCDC_DUMBMODE(n)                                        (((n) & 0xF) << 28)
#define LCDC_DUMBMODE_MASK                                      (0xF << 28)
#define LCDC_DUMBMODE_SHIFT                                     (28)

// reg_1bc
#define LCDC_IOPADMODE(n)                                       (((n) & 0xF) << 0)
#define LCDC_IOPADMODE_MASK                                     (0xF << 0)
#define LCDC_IOPADMODE_SHIFT                                    (0)
#define LCDC_AXI_CTRL(n)                                        (((n) & 0xF) << 4)
#define LCDC_AXI_CTRL_MASK                                      (0xF << 4)
#define LCDC_AXI_CTRL_SHIFT                                     (4)
#define LCDC_PN_CSC(n)                                          (((n) & 0x3) << 8)
#define LCDC_PN_CSC_MASK                                        (0x3 << 8)
#define LCDC_PN_CSC_SHIFT                                       (8)
#define LCDC_TV_CSC(n)                                          (((n) & 0x3) << 10)
#define LCDC_TV_CSC_MASK                                        (0x3 << 10)
#define LCDC_TV_CSC_SHIFT                                       (10)
#define LCDC_CMD_VM_ENA                                         (1 << 12)
#define LCDC_DMA_VM_ENA                                         (1 << 13)
#define LCDC_GRA_VM_ENA                                         (1 << 15)
#define LCDC_TVD_VM_ENA                                         (1 << 17)
#define LCDC_TVG_VM_ENA                                         (1 << 19)
#define LCDC_SA_ENABLE                                          (1 << 20)
#define LCDC_TVD_SA_CMSK                                        (1 << 22)
#define LCDC_DMA_SA_CMSK                                        (1 << 23)
#define LCDC_TVG_SA_YMSK                                        (1 << 24)
#define LCDC_TVD_SA_VMSK                                        (1 << 25)
#define LCDC_TVD_SA_UMSK                                        (1 << 26)
#define LCDC_TVD_SA_YMSK                                        (1 << 27)
#define LCDC_GRA_SA_YMSK                                        (1 << 28)
#define LCDC_DMA_SA_VMSK                                        (1 << 29)
#define LCDC_DMA_SA_UMSK                                        (1 << 30)
#define LCDC_DMA_SA_YMSK                                        (1 << 31)

// reg_1c0
#define LCDC_TVIF_FRAMEDONE_ENA                                 (1 << 8)
#define LCDC_TVG_FF_UNDERFLOW_ENA                               (1 << 9)
#define LCDC_TVG_FRAME_IRQ1_ENA                                 (1 << 10)
#define LCDC_TVG_FRAME_IRQ0_ENA                                 (1 << 11)
#define LCDC_TVSYN_IRQ_ENA                                      (1 << 12)
#define LCDC_WB_FF_UNDERRUN_ENA                                 (1 << 13)
#define LCDC_WB_FRAMEDONE_ENA                                   (1 << 14)
#define LCDC_WB_FF_OVERRUN_ENA                                  (1 << 15)
#define LCDC_ERR_IRQ_ENA                                        (1 << 16)
#define LCDC_PWRND_IRQ_ENA                                      (1 << 17)
#define LCDC_SPI_IRQ_ENA                                        (1 << 18)
#define LCDC_SLV_IRQ_ENA                                        (1 << 19)
#define LCDC_SPI_FRAMEDONE_ENA                                  (1 << 20)
#define LCDC_TXC_FRAMEDONE_ENA                                  (1 << 21)
#define LCDC_DUMB_FRAMEDONE_ENA                                 (1 << 22)
#define LCDC_VSYNC_IRQ_ENA                                      (1 << 23)
#define LCDC_TVC_FRAMEDONE_ENA                                  (1 << 24)
#define LCDC_GRA_FF_UNDERFLOW_ENA                               (1 << 25)
#define LCDC_GRA_FRAME_IRQ1_ENA                                 (1 << 26)
#define LCDC_GRA_FRAME_IRQ0_ENA                                 (1 << 27)
#define LCDC_M2A_IRQ_ENA                                        (1 << 28)
#define LCDC_DMA_FF_UNDERFLOW_ENA                               (1 << 29)
#define LCDC_DMA_FRAME_IRQ1_ENA                                 (1 << 30)
#define LCDC_DMA_FRAME_IRQ0_ENA                                 (1 << 31)

// reg_1c0 macros for 2009
#define LCDC_FRAME_HALT_FLAG_ENA                                (1 << 8)
#define LCDC_ROTATE_FF_EMPTY_ENA                                (1 << 24)

// reg_1c0 macros for 1701
#define LCDC_TVD_FRAME_IRQ0_ENA                                 (1 << 0)
#define LCDC_TVD_FRAME_IRQ1_ENA                                 (1 << 1)
#define LCDC_TVD_FF_UNDERFLOW_ENA                               (1 << 2)
#define LCDC_TFBC_FF_UNDERFLOW_ENA                              (1 << 3)
#define LCDC_TFBC_CRC_MISMATCH_ENA                              (1 << 4)
#define LCDC_SMT_FF_UNDERFLOW_ENA                               (1 << 5)
#define LCDC_SMT_FRAME_IRQ1_ENA                                 (1 << 6)
#define LCDC_SMT_FRAME_IRQ0_ENA                                 (1 << 7)
#define LCDC_DEC_ERR_IRQ_ENA                                    (1 << 8)

// reg_1c4
#define LCDC_TVG_FF_EMPTY                                       (1 << 0)
#define LCDC_TVD_FF_EMPTY                                       (1 << 1)
#define LCDC_GRA_FF_EMPTY                                       (1 << 2)
#define LCDC_DMA_FF_EMPTY                                       (1 << 3)
#define LCDC_TVG_FRAME_CNT_0                                    (1 << 4)
#define LCDC_TVD_FRAME_CNT_0                                    (1 << 5)
#define LCDC_GRA_FRAME_CNT_0                                    (1 << 6)
#define LCDC_DMA_FRAME_CNT_0                                    (1 << 7)
#define LCDC_TVIF_FRAMEDONE                                     (1 << 8)
#define LCDC_TVG_FF_UNDERFLOW                                   (1 << 9)
#define LCDC_TVG_FRAME_IRQ1                                     (1 << 10)
#define LCDC_TVG_FRAME_IRQ0                                     (1 << 11)
#define LCDC_TV_VSYNC_IRQ                                       (1 << 12)
#define LCDC_WB_FF_UNDERRUN                                     (1 << 13)
#define LCDC_WB_FRAMEDONE                                       (1 << 14)
#define LCDC_WB_FF_OVERRUN                                      (1 << 15)
#define LCDC_ERR_IRQ                                            (1 << 16)
#define LCDC_PWRDN_IRQ                                          (1 << 17)
#define LCDC_SPI_IRQ                                            (1 << 18)
#define LCDC_SLV_IRQ                                            (1 << 19)
#define LCDC_SPI_FRAMEDONE                                      (1 << 20)
#define LCDC_TXC_FRAMEDONE                                      (1 << 21)
#define LCDC_DUMB_FRAMEDONE                                     (1 << 22)
#define LCDC_PN_VSYNC_IRQ                                       (1 << 23)
#define LCDC_TVC_FRAMEDONE                                      (1 << 24)
#define LCDC_GRA_FF_UNDERFLOW                                   (1 << 25)
#define LCDC_GRA_FRAME_IRQ1                                     (1 << 26)
#define LCDC_GRA_FRAME_IRQ0                                     (1 << 27)
#define LCDC_M2A_IRQ                                            (1 << 28)
#define LCDC_DMA_FF_UNDERFLOW                                   (1 << 29)
#define LCDC_DMA_FRAME_IRQ1                                     (1 << 30)
#define LCDC_DMA_FRAME_IRQ0                                     (1 << 31)

// reg_1c4 macros for 2009
#define LCDC_FRAME_HALT_FLAG                                    (1 << 8)
#define LCDC_ROTATE_FF_EMPTY                                    (1 << 24)

// reg_1c4 macros for 1701
#define LCDC_TVD_FF_UNDERFLOW                                   (1 << 0)
#define LCDC_TVD_FRAME_IRQ1                                     (1 << 1)
#define LCDC_TVD_FRAME_IRQ0                                     (1 << 2)
#define LCDC_TFBC_FF_UNDERFLOW                                  (1 << 3)
#define LCDC_TFBC_CRC_MISMATCH                                  (1 << 4)
#define LCDC_SMT_FF_UNDERFLOW                                   (1 << 5)
#define LCDC_SMT_FRAME_IRQ1                                     (1 << 6)
#define LCDC_SMT_FRAME_IRQ0                                     (1 << 7)
#define LCDC_DEC_ERR_IRQ                                        (1 << 8)

// reg_1c8
#define LCDC_RSR(n)                                             (((n) & 0xFFFFFF) << 8)
#define LCDC_RSR_MASK                                           (0xFFFFFF << 8)
#define LCDC_RSR_SHIFT                                          (8)

// reg_1cc
#define LCDC_GRA_CUTHPXL1(n)                                    (((n) & 0xFFF) << 0)
#define LCDC_GRA_CUTHPXL1_MASK                                  (0xFFF << 0)
#define LCDC_GRA_CUTHPXL1_SHIFT                                 (0)
#define LCDC_GRA_CUTCOLOR_3_0(n)                                (((n) & 0xF) << 12)
#define LCDC_GRA_CUTCOLOR_3_0_MASK                              (0xF << 12)
#define LCDC_GRA_CUTCOLOR_3_0_SHIFT                             (12)
#define LCDC_GRA_CUTHPXL2(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_GRA_CUTHPXL2_MASK                                  (0xFFF << 16)
#define LCDC_GRA_CUTHPXL2_SHIFT                                 (16)
#define LCDC_GRA_CUTCOLOR_7_4(n)                                (((n) & 0xF) << 28)
#define LCDC_GRA_CUTCOLOR_7_4_MASK                              (0xF << 28)
#define LCDC_GRA_CUTCOLOR_7_4_SHIFT                             (28)

// reg_1d0
#define LCDC_GRA_CUTVLN1(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_GRA_CUTVLN1_MASK                                   (0xFFF << 0)
#define LCDC_GRA_CUTVLN1_SHIFT                                  (0)
#define LCDC_GRA_CUTCOLOR_11_8(n)                               (((n) & 0xF) << 12)
#define LCDC_GRA_CUTCOLOR_11_8_MASK                             (0xF << 12)
#define LCDC_GRA_CUTCOLOR_11_8_SHIFT                            (12)
#define LCDC_GRA_CUTVLN2(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_GRA_CUTVLN2_MASK                                   (0xFFF << 16)
#define LCDC_GRA_CUTVLN2_SHIFT                                  (16)
#define LCDC_GRA_CUTCOLOR_15_12(n)                              (((n) & 0xF) << 28)
#define LCDC_GRA_CUTCOLOR_15_12_MASK                            (0xF << 28)
#define LCDC_GRA_CUTCOLOR_15_12_SHIFT                           (28)

// reg_1d4
#define LCDC_TVG_CUTHPXL1(n)                                    (((n) & 0xFFF) << 0)
#define LCDC_TVG_CUTHPXL1_MASK                                  (0xFFF << 0)
#define LCDC_TVG_CUTHPXL1_SHIFT                                 (0)
#define LCDC_TVG_CUTCOLOR_3_0(n)                                (((n) & 0xF) << 12)
#define LCDC_TVG_CUTCOLOR_3_0_MASK                              (0xF << 12)
#define LCDC_TVG_CUTCOLOR_3_0_SHIFT                             (12)
#define LCDC_TVG_CUTHPXL2(n)                                    (((n) & 0xFFF) << 16)
#define LCDC_TVG_CUTHPXL2_MASK                                  (0xFFF << 16)
#define LCDC_TVG_CUTHPXL2_SHIFT                                 (16)
#define LCDC_TVG_CUTCOLOR_7_4(n)                                (((n) & 0xF) << 28)
#define LCDC_TVG_CUTCOLOR_7_4_MASK                              (0xF << 28)
#define LCDC_TVG_CUTCOLOR_7_4_SHIFT                             (28)

// reg_1d8
#define LCDC_TVG_CUTVLN1(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_TVG_CUTVLN1_MASK                                   (0xFFF << 0)
#define LCDC_TVG_CUTVLN1_SHIFT                                  (0)
#define LCDC_TVG_CUTCOLOR_11_8(n)                               (((n) & 0xF) << 12)
#define LCDC_TVG_CUTCOLOR_11_8_MASK                             (0xF << 12)
#define LCDC_TVG_CUTCOLOR_11_8_SHIFT                            (12)
#define LCDC_TVG_CUTVLN2(n)                                     (((n) & 0xFFF) << 16)
#define LCDC_TVG_CUTVLN2_MASK                                   (0xFFF << 16)
#define LCDC_TVG_CUTVLN2_SHIFT                                  (16)
#define LCDC_TVG_CUTCOLOR_15_12(n)                              (((n) & 0xF) << 28)
#define LCDC_TVG_CUTCOLOR_15_12_MASK                            (0xF << 28)
#define LCDC_TVG_CUTCOLOR_15_12_SHIFT                           (28)

// reg_1dc
#define LCDC_AHB_WRCNT(n)                                       (((n) & 0x3) << 4)
#define LCDC_AHB_WRCNT_MASK                                     (0x3 << 4)
#define LCDC_AHB_WRCNT_SHIFT                                    (4)
#define LCDC_AHB_RDCNT(n)                                       (((n) & 0x3) << 6)
#define LCDC_AHB_RDCNT_MASK                                     (0x3 << 6)
#define LCDC_AHB_RDCNT_SHIFT                                    (6)
#define LCDC_DMA_BURST0TO3(n)                                   (((n) & 0x3) << 8)
#define LCDC_DMA_BURST0TO3_MASK                                 (0x3 << 8)
#define LCDC_DMA_BURST0TO3_SHIFT                                (8)
#define LCDC_GRA_BURST0TO3(n)                                   (((n) & 0x3) << 10)
#define LCDC_GRA_BURST0TO3_MASK                                 (0x3 << 10)
#define LCDC_GRA_BURST0TO3_SHIFT                                (10)
#define LCDC_TVD_BURST0TO3(n)                                   (((n) & 0x3) << 12)
#define LCDC_TVD_BURST0TO3_MASK                                 (0x3 << 12)
#define LCDC_TVD_BURST0TO3_SHIFT                                (12)
#define LCDC_TVG_BURST0TO3(n)                                   (((n) & 0x3) << 14)
#define LCDC_TVG_BURST0TO3_MASK                                 (0x3 << 14)
#define LCDC_TVG_BURST0TO3_SHIFT                                (14)
#define LCDC_PN_SWAPATH_ENA                                     (1 << 16)
#define LCDC_PN_SWAPATH                                         (1 << 17)
#define LCDC_TV_SWAPATH_ENA                                     (1 << 18)
#define LCDC_TV_SWAPATH                                         (1 << 19)
#define LCDC_MERGE_SCLKTCLK                                     (1 << 20)
#define LCDC_T2S_S2TB                                           (1 << 21)
#define LCDC_ALL2PNORTV(n)                                      (((n) & 0x3) << 22)
#define LCDC_ALL2PNORTV_MASK                                    (0x3 << 22)
#define LCDC_ALL2PNORTV_SHIFT                                   (22)
#define LCDC_MIPIMODE(n)                                        (((n) & 0xF) << 24)
#define LCDC_MIPIMODE_MASK                                      (0xF << 24)
#define LCDC_MIPIMODE_SHIFT                                     (24)
#define LCDC_INVERT_IOPAD_DENA                                  (1 << 28)
#define LCDC_INVERT_IOPAD_PCLK                                  (1 << 29)
#define LCDC_INVERT_IOPAD_HSYNC                                 (1 << 30)
#define LCDC_INVERT_IOPAD_VSYNC                                 (1 << 31)

// reg_1e0
#define LCDC_SQULN1_ENA                                         (1 << 0)
#define LCDC_SQULN1_SIZE(n)                                     (((n) & 0x1F) << 1)
#define LCDC_SQULN1_SIZE_MASK                                   (0x1F << 1)
#define LCDC_SQULN1_SIZE_SHIFT                                  (1)
#define LCDC_SQULN1_SA(n)                                       (((n) & 0x3FFFFFF) << 6)
#define LCDC_SQULN1_SA_MASK                                     (0x3FFFFFF << 6)
#define LCDC_SQULN1_SA_SHIFT                                    (6)

// reg_1e4
#define LCDC_SQULN2_ENA                                         (1 << 0)
#define LCDC_SQULN2_SIZE(n)                                     (((n) & 0x1F) << 1)
#define LCDC_SQULN2_SIZE_MASK                                   (0x1F << 1)
#define LCDC_SQULN2_SIZE_SHIFT                                  (1)
#define LCDC_SQULN2_SA(n)                                       (((n) & 0x3FFFFFF) << 6)
#define LCDC_SQULN2_SA_MASK                                     (0x3FFFFFF << 6)
#define LCDC_SQULN2_SA_SHIFT                                    (6)

// reg_1e8
#define LCDC_OVTOP(n)                                           (((n) & 0x3) << 0)
#define LCDC_OVTOP_MASK                                         (0x3 << 0)
#define LCDC_OVTOP_SHIFT                                        (0)
#define LCDC_OVNXT(n)                                           (((n) & 0x3) << 2)
#define LCDC_OVNXT_MASK                                         (0x3 << 2)
#define LCDC_OVNXT_SHIFT                                        (2)
#define LCDC_OVTOP_TVC                                          (1 << 4)
#define LCDC_DMATVD_AMOD(n)                                     (((n) & 0x3) << 8)
#define LCDC_DMATVD_AMOD_MASK                                   (0x3 << 8)
#define LCDC_DMATVD_AMOD_SHIFT                                  (8)
#define LCDC_DMATVG_AMOD(n)                                     (((n) & 0x3) << 10)
#define LCDC_DMATVG_AMOD_MASK                                   (0x3 << 10)
#define LCDC_DMATVG_AMOD_SHIFT                                  (10)
#define LCDC_GRATVD_AMOD(n)                                     (((n) & 0x3) << 12)
#define LCDC_GRATVD_AMOD_MASK                                   (0x3 << 12)
#define LCDC_GRATVD_AMOD_SHIFT                                  (12)
#define LCDC_GRATVG_AMOD(n)                                     (((n) & 0x3) << 14)
#define LCDC_GRATVG_AMOD_MASK                                   (0x3 << 14)
#define LCDC_GRATVG_AMOD_SHIFT                                  (14)
#define LCDC_VSMTH_DMATVD(n)                                    (((n) & 0x3) << 16)
#define LCDC_VSMTH_DMATVD_MASK                                  (0x3 << 16)
#define LCDC_VSMTH_DMATVD_SHIFT                                 (16)
#define LCDC_VSMTH_GRATVG(n)                                    (((n) & 0x3) << 18)
#define LCDC_VSMTH_GRATVG_MASK                                  (0x3 << 18)
#define LCDC_VSMTH_GRATVG_SHIFT                                 (18)
#define LCDC_DMA_2ZMDN                                          (1 << 20)
#define LCDC_GRA_2ZMDN                                          (1 << 21)
#define LCDC_TVD_2ZMDN                                          (1 << 22)
#define LCDC_TVG_2ZMDN                                          (1 << 23)

// reg_1ec
#define LCDC_DITHER_EN_PN                                       (1 << 0)
#define LCDC_DITHER_4X8_PN                                      (1 << 1)
#define LCDC_DITHER_MODE_PN(n)                                  (((n) & 0x7) << 4)
#define LCDC_DITHER_MODE_PN_MASK                                (0x7 << 4)
#define LCDC_DITHER_MODE_PN_SHIFT                               (4)
#define LCDC_DITHER_EN_TV                                       (1 << 8)
#define LCDC_DITHER_4X8_TV                                      (1 << 9)
#define LCDC_DITHER_MODE_TV(n)                                  (((n) & 0x7) << 12)
#define LCDC_DITHER_MODE_TV_MASK                                (0x7 << 12)
#define LCDC_DITHER_MODE_TV_SHIFT                               (12)
#define LCDC_DITHER_TABLE_INDEX_SEL(n)                          (((n) & 0x3) << 16)
#define LCDC_DITHER_TABLE_INDEX_SEL_MASK                        (0x3 << 16)
#define LCDC_DITHER_TABLE_INDEX_SEL_SHIFT                       (16)

// reg_1f0
#define LCDC_DITHER_TBL_DATA(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DITHER_TBL_DATA_MASK                               (0xFFFFFFFF << 0)
#define LCDC_DITHER_TBL_DATA_SHIFT                              (0)

// reg_1f4
#define LCDC_DSI1_SEL                                           (1 << 0)
#define LCDC_DSI2_SEL                                           (1 << 1)
#define LCDC_MASTER_ENH_PN                                      (1 << 4)
#define LCDC_MASTER_ENH_TV                                      (1 << 5)
#define LCDC_MASTER_ENV_PN                                      (1 << 6)
#define LCDC_MASTER_ENV_TV                                      (1 << 7)
#define LCDC_DSI_START_PN_SEL                                   (1 << 8)
#define LCDC_DSI_START_TV_SEL                                   (1 << 9)
#define LCDC_HSTART_PN_SEL                                      (1 << 10)
#define LCDC_VSTART_PN_SEL                                      (1 << 11)
#define LCDC_HSTART_TV_SEL                                      (1 << 12)
#define LCDC_VSTART_TV_SEL                                      (1 << 13)
#define LCDC_RTC_DSI(n)                                         (((n) & 0x3) << 14)
#define LCDC_RTC_DSI_MASK                                       (0x3 << 14)
#define LCDC_RTC_DSI_SHIFT                                      (14)
#define LCDC_WTC_DSI(n)                                         (((n) & 0x3) << 16)
#define LCDC_WTC_DSI_MASK                                       (0x3 << 16)
#define LCDC_WTC_DSI_SHIFT                                      (16)
#define LCDC_PDWN_DSI                                           (1 << 18)

// reg_1f8
#define LCDC_SPI_HW_CTRL                                        (1 << 0)
#define LCDC_SMPN_SWAP_RB                                       (1 << 1)
#define LCDC_SPI_2LINE_MODE                                     (1 << 2)
#define LCDC_SPI_1LINE_MODE                                     (1 << 3)
#define LCDC_SMPN2SPI_MODE(n)                                   (((n) & 0x3) << 4)
#define LCDC_SMPN2SPI_MODE_MASK                                 (0x3 << 4)
#define LCDC_SMPN2SPI_MODE_SHIFT                                (4)
#define LCDC_SPI_4LINE_MODE                                     (1 << 6)

// reg_200
#define LCDC_WDMA_ENA                                           (1 << 0)
#define LCDC_WDMA_SEL_DSI                                       (1 << 1)
#define LCDC_WDMA_PIX_FMT(n)                                    (((n) & 0x3) << 4)
#define LCDC_WDMA_PIX_FMT_MASK                                  (0x3 << 4)
#define LCDC_WDMA_PIX_FMT_SHIFT                                 (4)
#define LCDC_WDMA_BURST_LEN(n)                                  (((n) & 0x1F) << 8)
#define LCDC_WDMA_BURST_LEN_MASK                                (0x1F << 8)
#define LCDC_WDMA_BURST_LEN_SHIFT                               (8)
#define LCDC_WDMA_IMG_PITCH(n)                                  (((n) & 0xFFFF) << 16)
#define LCDC_WDMA_IMG_PITCH_MASK                                (0xFFFF << 16)
#define LCDC_WDMA_IMG_PITCH_SHIFT                               (16)

// reg_204
#define LCDC_WDMA_IMG_WIDTH(n)                                  (((n) & 0x1FFF) << 0)
#define LCDC_WDMA_IMG_WIDTH_MASK                                (0x1FFF << 0)
#define LCDC_WDMA_IMG_WIDTH_SHIFT                               (0)
#define LCDC_WDMA_IMG_HEIGHT(n)                                 (((n) & 0xFFF) << 16)
#define LCDC_WDMA_IMG_HEIGHT_MASK                               (0xFFF << 16)
#define LCDC_WDMA_IMG_HEIGHT_SHIFT                              (16)
#define LCDC_WDMA_FPT_SET(n)                                    (((n) & 0xF) << 28)
#define LCDC_WDMA_FPT_SET_MASK                                  (0xF << 28)
#define LCDC_WDMA_FPT_SET_SHIFT                                 (28)

// reg_208
#define LCDC_WDMA_BASE_ADDR(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define LCDC_WDMA_BASE_ADDR_MASK                                (0xFFFFFFFF << 0)
#define LCDC_WDMA_BASE_ADDR_SHIFT                               (0)

// reg_20c
#define LCDC_DMAC_CTRL(n)                                       (((n) & 0xFFFFF) << 0)
#define LCDC_DMAC_CTRL_MASK                                     (0xFFFFF << 0)
#define LCDC_DMAC_CTRL_SHIFT                                    (0)
#define LCDC_WR_FAST_ENA                                        (1 << 20)
#define LCDC_SPLIT_WLAST_MASK                                   (1 << 21)

// reg_210
#define LCDC_VSYNC_TRIG_DISABLE_S                               (1 << 0)
#define LCDC_VSYNC_TRIG_DISABLE_A                               (1 << 1)
#define LCDC_TE_BYPASS                                          (1 << 2)
#define LCDC_TE_SEL                                             (1 << 3)
#define LCDC_EDGE_SEL                                           (1 << 4)
#define LCDC_TE_MODE                                            (1 << 5)
#define LCDC_NFRAME_ACTIVE(n)                                   (((n) & 0xFF) << 6)
#define LCDC_NFRAME_ACTIVE_MASK                                 (0xFF << 6)
#define LCDC_NFRAME_ACTIVE_SHIFT                                (6)
#define LCDC_EXTRA_DELAY(n)                                     (((n) & 0x3FF) << 14)
#define LCDC_EXTRA_DELAY_MASK                                   (0x3FF << 14)
#define LCDC_EXTRA_DELAY_SHIFT                                  (14)
#define LCDC_TE_CLR                                             (1 << 24)
#define LCDC_TE_MASK                                            (1 << 25)
#define LCDC_DSI_DR                                             (1 << 26)
#define LCDC_BACKLIGHT_EN                                       (1 << 27)
#define LCDC_BACKLIGHT_DUMB_MASK                                (1 << 28)
#define LCDC_REG_GEN_FRAME                                      (1 << 29)
#define LCDC_DSI_RD_MEM                                         (1 << 30)
#define LCDC_DUMB_SOURCE_SEL                                    (1 << 31)

// reg_214
#define LCDC_HSYNC_LENGTH(n)                                    (((n) & 0xFFFF) << 0)
#define LCDC_HSYNC_LENGTH_MASK                                  (0xFFFF << 0)
#define LCDC_HSYNC_LENGTH_SHIFT                                 (0)
#define LCDC_DEL_NLINE(n)                                       (((n) & 0x3FF) << 16)
#define LCDC_DEL_NLINE_MASK                                     (0x3FF << 16)
#define LCDC_DEL_NLINE_SHIFT                                    (16)
#define LCDC_HWC_ENA                                            (1 << 26)
#define LCDC_HWC_1BITENA                                        (1 << 27)
#define LCDC_HWC_1BITMOD                                        (1 << 28)
#define LCDC_SPI_CMD_START                                      (1 << 29)
#define LCDC_DSI_RD_MEM_CMD                                     (1 << 30)
#define LCDC_TE_INV                                             (1 << 31)

// reg_218
#define LCDC_STBY_LENGTH(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_STBY_LENGTH_MASK                                   (0xFFF << 0)
#define LCDC_STBY_LENGTH_SHIFT                                  (0)
#define LCDC_VSYNC_START_LINE(n)                                (((n) & 0xFF) << 12)
#define LCDC_VSYNC_START_LINE_MASK                              (0xFF << 12)
#define LCDC_VSYNC_START_LINE_SHIFT                             (12)
#define LCDC_VSYNC_END_LINE(n)                                  (((n) & 0xFF) << 20)
#define LCDC_VSYNC_END_LINE_MASK                                (0xFF << 20)
#define LCDC_VSYNC_END_LINE_SHIFT                               (20)
#define LCDC_ACMD_BYPASS                                        (1 << 28)

// reg_21c
#define LCDC_H_FRONT_PORCH_SMPN(n)                              (((n) & 0xFFF) << 0)
#define LCDC_H_FRONT_PORCH_SMPN_MASK                            (0xFFF << 0)
#define LCDC_H_FRONT_PORCH_SMPN_SHIFT                           (0)
#define LCDC_H_BACK_PORCH_SMPN(n)                               (((n) & 0xFFF) << 12)
#define LCDC_H_BACK_PORCH_SMPN_MASK                             (0xFFF << 12)
#define LCDC_H_BACK_PORCH_SMPN_SHIFT                            (12)
#define LCDC_VSYNC_NLINE(n)                                     (((n) & 0xFF) << 24)
#define LCDC_VSYNC_NLINE_MASK                                   (0xFF << 24)
#define LCDC_VSYNC_NLINE_SHIFT                                  (24)

// reg_220
#define LCDC_VSYNC_START_PIX(n)                                 (((n) & 0xFFFF) << 0)
#define LCDC_VSYNC_START_PIX_MASK                               (0xFFFF << 0)
#define LCDC_VSYNC_START_PIX_SHIFT                              (0)
#define LCDC_VSYNC_END_PIX(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_VSYNC_END_PIX_MASK                                 (0xFFFF << 16)
#define LCDC_VSYNC_END_PIX_SHIFT                                (16)

// reg_224
#define LCDC_GEN_FRAME_START(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GEN_FRAME_START_MASK                               (0xFFFFFFFF << 0)
#define LCDC_GEN_FRAME_START_SHIFT                              (0)

// reg_228
#define LCDC_FRAME_HALT_EN                                      (1 << 0)
#define LCDC_FRAME_HALT                                         (1 << 1)
#define LCDC_TEST_PATTERN_EN                                    (1 << 2)
#define LCDC_PATTERN_MODE(n)                                    (((n) & 0x7F) << 3)
#define LCDC_PATTERN_MODE_MASK                                  (0x7F << 3)
#define LCDC_PATTERN_MODE_SHIFT                                 (3)
#define LCDC_PATTERN_PIX_NUM(n)                                 (((n) & 0x7FF) << 10)
#define LCDC_PATTERN_PIX_NUM_MASK                               (0x7FF << 10)
#define LCDC_PATTERN_PIX_NUM_SHIFT                              (10)
#define LCDC_PATTERN_RGB_ORDER(n)                               (((n) & 0x7) << 21)
#define LCDC_PATTERN_RGB_ORDER_MASK                             (0x7 << 21)
#define LCDC_PATTERN_RGB_ORDER_SHIFT                            (21)
#define LCDC_PATTERN_IS_CIR                                     (1 << 24)
#define LCDC_DSI_DATA_SWAP(n)                                   (((n) & 0x7) << 25)
#define LCDC_DSI_DATA_SWAP_MASK                                 (0x7 << 25)
#define LCDC_DSI_DATA_SWAP_SHIFT                                (25)
#define LCDC_DSI_RGB888                                         (1 << 28)
#define LCDC_DSI_RGB666                                         (1 << 29)
#define LCDC_DSI_RGB565                                         (1 << 30)
#define LCDC_DSI_RGB101010                                      (1 << 31)

// reg_22c
#define LCDC_TXDATA_HEAD_24B(n)                                 (((n) & 0xFFFFFF) << 0)
#define LCDC_TXDATA_HEAD_24B_MASK                               (0xFFFFFF << 0)
#define LCDC_TXDATA_HEAD_24B_SHIFT                              (0)
#define LCDC_SPI_HEAD_TYPE(n)                                   (((n) & 0x3) << 24)
#define LCDC_SPI_HEAD_TYPE_MASK                                 (0x3 << 24)
#define LCDC_SPI_HEAD_TYPE_SHIFT                                (24)
#define LCDC_USE_FRAME_HEAD                                     (1 << 26)
#define LCDC_TXBIT_MINUS1                                       (1 << 27)
#define LCDC_SPI_4LINE_888                                      (1 << 28)
#define LCDC_SPI_4LINE_666                                      (1 << 29)
#define LCDC_SPI_1LN_RGB666_ORDER                               (1 << 30)
#define LCDC_SPI_4LINE_565                                      (1 << 31)

// reg_230
#define LCDC_USE_MMU                                            (1 << 0)
#define LCDC_MMU_ENA                                            (1 << 1)
#define LCDC_MMU_BYPASS                                         (1 << 2)
#define LCDC_SPI_DCX_DR                                         (1 << 3)
#define LCDC_SPI_DCX                                            (1 << 4)
#define LCDC_SPI_DCX_OENB                                       (1 << 5)
#define LCDC_SPI_XCS_DR                                         (1 << 6)
#define LCDC_SPI_XCS                                            (1 << 7)
#define LCDC_SPI_HEAD_NUM(n)                                    (((n) & 0x3F) << 8)
#define LCDC_SPI_HEAD_NUM_MASK                                  (0x3F << 8)
#define LCDC_SPI_HEAD_NUM_SHIFT                                 (8)
#define LCDC_DCX_VALID_SP_TXCMD                                 (1 << 18)
#define LCDC_SPI_4W_MODEL                                       (1 << 24)

// reg_234
#define LCDC_DMA_WDATA_H(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_WDATA_H_MASK                                   (0xFFFFFFFF << 0)
#define LCDC_DMA_WDATA_H_SHIFT                                  (0)

// reg_238
#define LCDC_DMA_WDATA_L(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DMA_WDATA_L_MASK                                   (0xFFFFFFFF << 0)
#define LCDC_DMA_WDATA_L_SHIFT                                  (0)

// reg_23c
#define LCDC_GRA_WDATA_H(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA_WDATA_H_MASK                                   (0xFFFFFFFF << 0)
#define LCDC_GRA_WDATA_H_SHIFT                                  (0)

// reg_240
#define LCDC_GRA_WDATA_L(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA_WDATA_L_MASK                                   (0xFFFFFFFF << 0)
#define LCDC_GRA_WDATA_L_SHIFT                                  (0)

// reg_244
#define LCDC_GRA2_WDATA_H(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA2_WDATA_H_MASK                                  (0xFFFFFFFF << 0)
#define LCDC_GRA2_WDATA_H_SHIFT                                 (0)

// reg_248
#define LCDC_GRA2_WDATA_L(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define LCDC_GRA2_WDATA_L_MASK                                  (0xFFFFFFFF << 0)
#define LCDC_GRA2_WDATA_L_SHIFT                                 (0)

// reg_24c
#define LCDC_DMA_MAP_ADDR(n)                                    (((n) & 0x1FF) << 0)
#define LCDC_DMA_MAP_ADDR_MASK                                  (0x1FF << 0)
#define LCDC_DMA_MAP_ADDR_SHIFT                                 (0)
#define LCDC_GRA_MAP_ADDR(n)                                    (((n) & 0x1FF) << 9)
#define LCDC_GRA_MAP_ADDR_MASK                                  (0x1FF << 9)
#define LCDC_GRA_MAP_ADDR_SHIFT                                 (9)
#define LCDC_GRA2_MAP_ADDR(n)                                   (((n) & 0x1FF) << 18)
#define LCDC_GRA2_MAP_ADDR_MASK                                 (0x1FF << 18)
#define LCDC_GRA2_MAP_ADDR_SHIFT                                (18)

// reg_250
#define LCDC_MMU_UNVALID_DATA(n)                                (((n) & 0xFFFFFFFF) << 0)
#define LCDC_MMU_UNVALID_DATA_MASK                              (0xFFFFFFFF << 0)
#define LCDC_MMU_UNVALID_DATA_SHIFT                             (0)

// reg_254
#define LCDC_TE_DB_TARGET(n)                                    (((n) & 0xFF) << 0)
#define LCDC_TE_DB_TARGET_MASK                                  (0xFF << 0)
#define LCDC_TE_DB_TARGET_SHIFT                                 (0)
#define LCDC_TE_DB_BYPASS                                       (1 << 8)
#define LCDC_TE_USE_SW                                          (1 << 9)
#define LCDC_WAIT_TIME(n)                                       (((n) & 0xFFF) << 16)
#define LCDC_WAIT_TIME_MASK                                     (0xFFF << 16)
#define LCDC_WAIT_TIME_SHIFT                                    (16)

// reg_258
#define LCDC_FRAME_VALID(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define LCDC_FRAME_VALID_MASK                                   (0xFFFFFFFF << 0)
#define LCDC_FRAME_VALID_SHIFT                                  (0)

// reg_260
#define LCDC_TVD_ENA                                            (1 << 0)
#define LCDC_YUV2RGB_TVD                                        (1 << 1)
#define LCDC_TVD_SWAPYU                                         (1 << 2)
#define LCDC_TVD_SWAPUV                                         (1 << 3)
#define LCDC_TVD_SWAPRB                                         (1 << 4)
#define LCDC_TVD_TSTMODE                                        (1 << 5)
#define LCDC_TVD_HSMOOTH                                        (1 << 6)
#define LCDC_TVD_FTOGGLE                                        (1 << 7)
#define LCDC_TVDFORMAT(n)                                       (((n) & 0xF) << 8)
#define LCDC_TVDFORMAT_MASK                                     (0xF << 8)
#define LCDC_TVDFORMAT_SHIFT                                    (8)
#define LCDC_FORCE_BLANKCOLOR                                   (1 << 24)
#define LCDC_DLY_CS_ENA                                         (1 << 25)
#define LCDC_ARBFAST_ENA                                        (1 << 27)
#define LCDC_PN_PALETTE_ENA                                     (1 << 28)
#define LCDC_PN_CBSH_ENA                                        (1 << 29)
#define LCDC_PN_GAMMA_ENA                                       (1 << 30)
#define LCDC_PN_NOBLENDING                                      (1 << 31)

// reg_264
#define LCDC_GRA_ENA                                            (1 << 0)
#define LCDC_YUV2RGB_GRA                                        (1 << 1)
#define LCDC_GRA_SWAPYU                                         (1 << 2)
#define LCDC_GRA_SWAPUV                                         (1 << 3)
#define LCDC_GRA_SWAPRB                                         (1 << 4)
#define LCDC_GRA_TSTMODE                                        (1 << 5)
#define LCDC_GRA_HSMOOTH                                        (1 << 6)
#define LCDC_GRA_FTOGGLE                                        (1 << 7)
#define LCDC_GRAFORMAT(n)                                       (((n) & 0xF) << 8)
#define LCDC_GRAFORMAT_MASK                                     (0xF << 8)
#define LCDC_GRAFORMAT_SHIFT                                    (8)
#define LCDC_YUV_PLANAR_FAST                                    (1 << 15)
#define LCDC_YUV_PLANAR_RDS(n)                                  (((n) & 0xFFFF) << 16)
#define LCDC_YUV_PLANAR_RDS_MASK                                (0xFFFF << 16)
#define LCDC_YUV_PLANAR_RDS_SHIFT                               (16)

// reg_26c
#define LCDC_WIDTH_SM_BURST_LENGTH_DMA                          (1 << 25)
#define LCDC_WIDTH_SM_BURST_LENGTH_TVD                          (1 << 26)
#define LCDC_FRAME_HALT_FROM_LPAP                               (1 << 28)

// reg_27c
#define LCDC_WAIT_RDCNT(n)                                      (((n) & 0xFFFF) << 0)
#define LCDC_WAIT_RDCNT_MASK                                    (0xFFFF << 0)
#define LCDC_WAIT_RDCNT_SHIFT                                   (0)
#define LCDC_READ_CMD_NUM(n)                                    (((n) & 0xFFFF) << 16)
#define LCDC_READ_CMD_NUM_MASK                                  (0xFFFF << 16)
#define LCDC_READ_CMD_NUM_SHIFT                                 (16)

// reg_280
#define LCDC_CMD_ENA                                            (1 << 0)
#define LCDC_PARAM_ENA                                          (1 << 1)
#define LCDC_PARAM_CNT(n)                                       (((n) & 0x1FF) << 4)
#define LCDC_PARAM_CNT_MASK                                     (0x1FF << 4)
#define LCDC_PARAM_CNT_SHIFT                                    (4)

// reg_284
#define LCDC_CMD(n)                                             (((n) & 0xFFFFFFFF) << 0)
#define LCDC_CMD_MASK                                           (0xFFFFFFFF << 0)
#define LCDC_CMD_SHIFT                                          (0)

// reg_288 ~ reg_2a4
// maxium 256bits parameters

// reg_2a8
#define LCDC_QSPI_USE0                                          (1 << 0)
#define LCDC_QSPI_USE1                                          (1 << 1)
#define LCDC_DCX_VALID_SP_CMD_STOP                              (1 << 3)
#define LCDC_DCX_VALID_SP_HEAD                                  (1 << 4)
#define LCDC_DATA_DCX_MODE                                      (1 << 5)
#define LCDC_READ_MUX_DATA1                                     (1 << 6)
#define LCDC_READ_MUX_DATA0                                     (0 << 6)
#define LCDC_CMDSIZE(n)                                         (((n) & 0x3F) << 8)
#define LCDC_CMDSIZE_MASK                                       (0x3F << 8)
#define LCDC_CMDSIZE_SHIFT                                      (8)
#define LCDC_SDR_EN                                             (1 << 14)
#define LCDC_DTR_EN                                             (1 << 15)
#define LCDC_HEAD_HIGH_8B(n)                                    (((n) & 0xFF) << 16)
#define LCDC_HEAD_HIGH_8B_MASK                                  (0xFF << 16)
#define LCDC_HEAD_HIGH_8B_SHIFT                                 (16)
#define LCDC_LINE_MODEL                                         (1 << 24)
#define LCDC_SPI_4LINE_444                                      (1 << 25)
#define LCDC_SPI_4LINE_444_SWAP                                 (1 << 26)
#define LCDC_CMD_DCX                                            (1 << 27)
#define LCDC_QSPI_FAST_MODE                                     (1 << 28)
#define LCDC_QSPI_DUAL_DISP                                     (1 << 30)
#define LCDC_QSPI_VSYNC_START_SEL                               (1 << 31)

// reg_2b0
#define LCDC_SPI_2LINE_MODE0                                    (1 << 0)
#define LCDC_SPI_2LINE_MODE1                                    (1 << 1)

// reg_2c0
#define LCDC_GRA_PREPXL(n)                                      (((n) & 0xF) << 0)
#define LCDC_GRA_PREPXL_MASK                                    (0xF << 0)
#define LCDC_GRA_PREPXL_SHIFT                                   (0)
#define LCDC_TVGRA_PREPXL(n)                                    (((n) & 0xF) << 4)
#define LCDC_TVGRA_PREPXL_MASK                                  (0xF << 4)
#define LCDC_TVGRA_PREPXL_SHIFT                                 (4)
#define LCDC_LCDC_TE_SEL                                        (1 << 8)
#define LCDC_DSI0_TE_SEL                                        (1 << 9)
#define LCDC_DSI1_TE_SEL                                        (1 << 10)
#define LCDC_PARALLEL_CLK_INV                                   (1 << 11)
#define LCDC_PARALLEL_CLK_ENA                                   (1 << 12)
#define LCDC_LCDC_PARA0_ENA                                     (1 << 13)
#define LCDC_LCDC_PARA1_ENA                                     (1 << 14)
#define LCDC_TRIGGER_SEL_FOR_VIDEO(n)                           (((n) & 0x3) << 15)
#define LCDC_TRIGGER_SEL_FOR_VIDEO_MASK                         (0x3 << 15)
#define LCDC_TRIGGER_SEL_FOR_VIDEO_SHIFT                        (15)
#define LCDC_TRIGGER_MUX_EDGE_SEL                               (1 << 17)
#define LCDC_TRIGGER_ENA_DIS                                    (1 << 18)
#define LCDC_TRIGGER_MUX_ENA                                    (1 << 19)
#define LCDC_LCD_IRQ_FOR_BTC(n)                                 (((n) & 0x3) << 20)
#define LCDC_LCD_IRQ_FOR_BTC_MASK                               (0x3 << 20)
#define LCDC_LCD_IRQ_FOR_BTC_SHIFT                              (20)
#define LCDC_GRA_IN_VLD_STEREO_L                                (1 << 22)
#define LCDC_DMA_IN_VLD_STEREO_L                                (1 << 23)
#define LCDC_TVG_IN_VLD_STEREO_L                                (1 << 24)
#define LCDC_GRA_IN_VLD_STEREO_R                                (1 << 25)
#define LCDC_DMA_IN_VLD_STEREO_R                                (1 << 26)
#define LCDC_TVG_IN_VLD_STEREO_R                                (1 << 27)
#define LCDC_DSI0_DATA1_SEL                                     (1 << 28)
#define LCDC_DSI1_DATA1_SEL                                     (1 << 29)
#define LCDC_PARALLEL_CLK_INV_1                                 (1 << 30)
#define LCDC_PARALLEL_CLK_ENA_1                                 (1 << 31)

//reg_2c4
#define LCDC_ARQOS_FIX_RDQOS                                    (1 << 0)
#define LCDC_ARQOS_RDQOS_RANGE                                  (1 << 1)
#define LCDC_ARQOS_URGENT_MUX                                   (1 << 2)
#define LCDC_ARQOS_RDQOS_VALUE(n)                               (((n) & 0xF) << 4)
#define LCDC_ARQOS_RDQOS_VALUE_MASK                             (0xF << 4)
#define LCDC_ARQOS_RDQOS_VALUE_SHIFT                            (4)
#define LCDC_ARQOS_RDQOS_MIN(n)                                 (((n) & 0xF) << 8)
#define LCDC_ARQOS_RDQOS_MIN_MASK                               (0xF << 8)
#define LCDC_ARQOS_RDQOS_MIN_SHIFT                              (8)
#define LCDC_ARQOS_RDQOS_MAX(n)                                 (((n) & 0xF) << 12)
#define LCDC_ARQOS_RDQOS_MAX_MASK                               (0xF << 12)
#define LCDC_ARQOS_RDQOS_MAX_SHIFT                              (12)

/* reg_2c8 */
#define LCDC_DATA1_PN_ALPHA(n)                                  (((n) & 0xFF) << 0)
#define LCDC_DATA1_PN_ALPHA_MASK                                (0xFF << 0)
#define LCDC_DATA1_PN_ALPHA_SHIFT                               (0)
#define LCDC_DATA1_TV_ALPHA(n)                                  (((n) & 0xFF) << 8)
#define LCDC_DATA1_TV_ALPHA_MASK                                (0xFF << 8)
#define LCDC_DATA1_TV_ALPHA_SHIFT                               (8)
#define LCDC_DATA1_BLEND_MODE(n)                                (((n) & 0x3) << 16)
#define LCDC_DATA1_BLEND_MODE_MASK                              (0x3 << 16)
#define LCDC_DATA1_BLEND_MODE_SHIFT                             (16)
#define LCDC_DATA1_PN_ALPHA_CONST_MODE                          (0 << 18)
#define LCDC_DATA1_PN_ALPHA_PIXEL_MODE                          (1 << 18)
#define LCDC_DATA1_TV_ALPHA_CONST_MODE                          (0 << 20)
#define LCDC_DATA1_TV_ALPHA_PIXEL_MODE                          (1 << 20)
#define LCDC_PARALLEL_VSYNC_POL_INV                             (1 << 22)
#define LCDC_PARALLEL_HSYNC_POL_INV                             (1 << 23)
#define LCDC_PARALLEL_DEN_POL_INV                               (1 << 24)
#define LCDC_TVD_ALPHA_CONST_MODE                               (0 << 25)
#define LCDC_TVD_ALPHA_PIXEL_MODE                               (1 << 25)
#define LCDC_TVD_IN_VLD_STEREO_L                                (1 << 26)
#define LCDC_SMT_IN_VLD_STEREO_L                                (1 << 27)
#define LCDC_TVD_IN_VLD_STEREO_R                                (1 << 28)
#define LCDC_SMT_IN_VLD_STEREO_R                                (1 << 29)
#define LCDC_ALPHA_BLEND_ROUND_ENA                              (1 << 30)

/* reg_2cc */
#define LCDC_PN_BLANKCOLOR_STEREO(n)                            (((n) & 0xFFFFFF) << 0)
#define LCDC_PN_BLANKCOLOR_STEREO_MASK                          (0xFFFFFF << 0)
#define LCDC_PN_BLANKCOLOR_STEREO_SHIFT                         (0)
#define LCDC_TVD_ALPHA(n)                                       (((n) & 0xFF) << 24)
#define LCDC_TVD_ALPHA_MASK                                     (0xFF << 24)
#define LCDC_TVD_ALPHA_SHIFT                                    (24)

/* reg_2d0 */
#define LCDC_DEC_ENA                                            (1 << 0)
#define LCDC_DEC_SWAP_RB                                        (1 << 1)
#define LCDC_DEC_TILE_MODE(n)                                   (((n) & 0x1) << 2) /* 0: linear, 1: tiled */
#define LCDC_DEC_TILE_MODE_MASK                                 (1 << 2)
#define LCDC_DEC_TILE_MODE_SHIFT                                (2)
#define LCDC_DEC_BYPASS                                         (1 << 3)
#define LCDC_DEC_COMPRESS_MODE(n)                               (((n) & 0x3) << 4) /* 0: non sample,
                                                                                    * 1: h-sample,
                                                                                    * 2: hv-sample */
#define LCDC_DEC_COMPRESS_MODE_MASK                             (0x3 << 4)
#define LCDC_DEC_COMPRESS_MODE_SHIFT                            (4)
#define LCDC_DEC_BURST_LEN_SEL(n)                               (((n) & 0x3) << 6) /* 0: 16 bytes,
                                                                                    * 1: 24 bytes,
                                                                                    * 2: 32 bytes,
                                                                                    * 3: 40 bytes */
#define LCDC_DEC_BURST_LEN_SEL_MASK                             (0x3 << 6)
#define LCDC_DEC_BURST_LEN_SEL_SHIFT                            (6)
#define LCDC_DEC_FORMAT(n)                                      (((n) & 0xF) << 8)  /* 0: ARGB, 1: XRGB */
#define LCDC_DEC_FORMAT_MASK                                    (0xF << 8)
#define LCDC_DEC_FORMAT_SHIFT                                   (8)
#define LCDC_DEC_HLEN_DR                                        (1 << 12) /* 0: hardware auto set hlen,
                                                                           * 1: software set hlen in bit28:16 */
#define LCDC_DEC_SYNC_FIFO_FLUSH                                (1 << 13)
#define LCDC_DEC_HLEN(n)                                        (((n) & 0x1FFF) << 16) /* compressed size in bytes of one line, set by sw */
#define LCDC_DEC_HLEN_MASK                                      (0x1FFF << 16)
#define LCDC_DEC_HLEN_SHIFT                                     (16)
#define LCDC_DEC_DATA_DELAY(n)                                  (((n) & 0x3) << 29)
#define LCDC_DEC_DATA_DELAY_MASK                                (0x3 << 29)
#define LCDC_DEC_DATA_DELAY_SHIFT                               (29)

/* reg_2d0 for 2009 */
#define LCDC_HMIRRO_ENA                                         (1 << 0)
#define LCDC_HMIRRO_V_LINE_SHIFT                                (1)
#define LCDC_HMIRRO_V_LINE(n)                                   (((n) & 0x3FF) << LCDC_HMIRRO_V_LINE_SHIFT)
#define LCDC_HMIRRO_V_LINE_MASK                                 (0x3FF << LCDC_HMIRRO_V_LINE_SHIFT)
#define LCDC_HMIRRO_H_PIXEL_SHIFT                               (11)
#define LCDC_HMIRRO_H_PIXEL(n)                                  (((n) & 0x3FF) << LCDC_HMIRRO_H_PIXEL_SHIFT)
#define LCDC_HMIRRO_H_PIXEL_MASK                                (0x3FF << LCDC_HMIRRO_H_PIXEL_SHIFT)

// reg_2d4
#define LCDC_DEC_BUF_H_PIXEL(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_DEC_BUF_H_PIXEL_MASK                               (0xFFF << 0)
#define LCDC_DEC_BUF_H_PIXEL_SHIFT                              (0)
#define LCDC_DEC_BUF_V_LINE(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_DEC_BUF_V_LINE_MASK                                (0xFFF << 16)
#define LCDC_DEC_BUF_V_LINE_SHIFT                               (16)
#define LCDC_DEC_UNDERFLOW_START_CNT(n)                         (((n) & 0xF) << 28)
#define LCDC_DEC_UNDERFLOW_START_CNT_MASK                       (0xF << 28)
#define LCDC_DEC_UNDERFLOW_START_CNT_SHIFT                      (28)

// reg_2d8
#define LCDC_DEC_SA0(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_DEC_SA0_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_DEC_SA0_SHIFT                                      (0)

// reg_2dc
#define LCDC_DEC_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_DEC_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_DEC_H_PIXEL_SHIFT                                  (0)
#define LCDC_DEC_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_DEC_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_DEC_V_LINE_SHIFT                                   (16)

// reg_32c
#define LCDC_TILED_ENA                                          (1 << 0)
#define LCDC_TILED_HLEN_DR                                      (1 << 1)
#define LCDC_TILED_REQBURST                                     (1 << 2)
#define LCDC_TILED_FORMAT(n)                                    (((n) & 0x3) << 3)
#define LCDC_TILED_FORMAT_MASK                                  (0x3 << 3)
#define LCDC_TILED_FORMAT_SHIFT                                 (3)
#define LCDC_TILED_BURST_LEN(n)                                 (((n) & 0x7) << 5)
#define LCDC_TILED_BURST_LEN_MASK                               (0x7 << 5)
#define LCDC_TILED_BURST_LEN_SHIFT                              (5)
#define LCDC_TILED_SYNC_FIFO_FLUSH                              (1 << 8)
#define LCDC_TILED_SWAP_RB                                      (1 << 9)
#define LCDC_TILED_DATA_DELAY(n)                                (((n) & 0x3) << 10)
#define LCDC_TILED_DATA_DELAY_MASK                              (0x3 << 10)
#define LCDC_TILED_DATA_DELAY_SHIFT                             (10)
#define LCDC_TILED_32B                                          (1 << 12)
#define LCDC_TILED_24B                                          (1 << 13)
#define LCDC_TILED_16B                                          (1 << 14)
#define LCDC_TILED_HLEN(n)                                      (((n) & 0x1FFF) << 16)
#define LCDC_TILED_HLEN_MASK                                    (0x1FFF << 16)
#define LCDC_TILED_HLEN_SHIFT                                   (16)
#define LCDC_TILED_LINE_END_MASK                                (1 << 30)
#define LCDC_TILED_USE_QSPI                                     (1 << 31)

// reg_330
#define LCDC_TILED_BUF_H_PIXEL(n)                               (((n) & 0x7FF) << 0)
#define LCDC_TILED_BUF_H_PIXEL_MASK                             (0x7FF << 0)
#define LCDC_TILED_BUF_H_PIXEL_SHIFT                            (0)
#define LCDC_TILED_BUF_V_LINE(n)                                (((n) & 0x7FF) << 16)
#define LCDC_TILED_BUF_V_LINE_MASK                              (0x7FF << 16)
#define LCDC_TILED_BUF_V_LINE_SHIFT                             (16)
#define LCDC_TILED_UNDERFLOW_START_CNT(n)                       (((n) & 0xF) << 28)
#define LCDC_TILED_UNDERFLOW_START_CNT_MASK                     (0xF << 28)
#define LCDC_TILED_UNDERFLOW_START_CNT_SHIFT                    (28)

// reg_334
#define LCDC_TILED_SA0(n)                                       (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TILED_SA0_MASK                                     (0xFFFFFFFF << 0)
#define LCDC_TILED_SA0_SHIFT                                    (0)

// reg_338
#define LCDC_TILED_H_PIXEL(n)                                   (((n) & 0x7FF) << 0)
#define LCDC_TILED_H_PIXEL_MASK                                 (0x7FF << 0)
#define LCDC_TILED_H_PIXEL_SHIFT                                (0)
#define LCDC_TILED_V_LINE(n)                                    (((n) & 0x7FF) << 16)
#define LCDC_TILED_V_LINE_MASK                                  (0x7FF << 16)
#define LCDC_TILED_V_LINE_SHIFT                                 (16)

//reg_340
#define LCDC_SCALER_H_ACTIVE_IN(n)                              (((n) & 0xFFF) << 0)
#define LCDC_SCALER_H_ACTIVE_IN_MASK                            (0xFFF << 0)
#define LCDC_SCALER_H_ACTIVE_IN_SHIFT                           (0)
#define LCDC_SCALER_V_ACTIVE_IN(n)                              (((n) & 0xFFF) << 16)
#define LCDC_SCALER_V_ACTIVE_IN_MASK                            (0xFFF << 16)
#define LCDC_SCALER_V_ACTIVE_IN_SHIFT                           (16)

//reg_344
#define LCDC_SCALER_VSYNC_NLINE(n)                              (((n) & 0xFF) << 0)
#define LCDC_SCALER_VSYNC_NLINE_MASK                            (0xFF << 0)
#define LCDC_SCALER_VSYNC_NLINE_SHIFT                           (0)
#define LCDC_SCALER_START_LINE(n)                               (((n) & 0xFF) << 8)
#define LCDC_SCALER_START_LINE_MASK                             (0xFF << 8)
#define LCDC_SCALER_START_LINE_SHIFT                            (8)
#define LCDC_SCALER_END_LINE(n)                                 (((n) & 0xFF) << 16)
#define LCDC_SCALER_END_LINE_MASK                               (0xFF << 16)
#define LCDC_SCALER_END_LINE_SHIFT                              (16)

//reg_348
#define LCDC_SCALER_START_PIX(n)                                (((n) & 0xFFFF) << 0)
#define LCDC_SCALER_START_PIX_MASK                              (0xFFFF << 0)
#define LCDC_SCALER_START_PIX_SHIFT                             (0)
#define LCDC_SCALER_END_PIX(n)                                  (((n) & 0xFFFF) << 16)
#define LCDC_SCALER_END_PIX_MASK                                (0xFFFF << 16)
#define LCDC_SCALER_END_PIX_SHIFT                               (16)

#ifdef CHIP_BEST1701
//reg_34c
#define LCDC_SCALER_H_FRONT_PORCH(n)                            (((n) & 0xFFFF) << 0)
#define LCDC_SCALER_H_FRONT_PORCH_MASK                          (0xFFFF << 0)
#define LCDC_SCALER_H_FRONT_PORCH_SHIFT                         (0)
#define LCDC_SCALER_H_BACK_PORCH(n)                             (((n) & 0xFFFF) << 16)
#define LCDC_SCALER_H_BACK_PORCH_MASK                           (0xFFFF << 16)
#define LCDC_SCALER_H_BACK_PORCH_SHIFT                          (16)

//reg_350
#define LCDC_SCALER_STBY_LEN(n)                                 (((n) & 0xFFFF) << 0)
#define LCDC_SCALER_STBY_LEN_MASK                               (0xFFFF << 0)
#define LCDC_SCALER_STBY_LEN_SHIFT                              (0)
#else

//reg_34c
#define LCDC_SCALER_H_FRONT_PORCH(n)                            (((n) & 0xFFF) << 0)
#define LCDC_SCALER_H_FRONT_PORCH_MASK                          (0xFFF << 0)
#define LCDC_SCALER_H_FRONT_PORCH_SHIFT                         (0)
#define LCDC_SCALER_H_BACK_PORCH(n)                             (((n) & 0xFFF) << 16)
#define LCDC_SCALER_H_BACK_PORCH_MASK                           (0xFFF << 16)
#define LCDC_SCALER_H_BACK_PORCH_SHIFT                          (16)

//reg_350
#define LCDC_SCALER_STBY_LEN(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SCALER_STBY_LEN_MASK                               (0xFFF << 0)
#define LCDC_SCALER_STBY_LEN_SHIFT                              (0)
#endif

// reg_354
#define LCDC_SCALER_ENA                                         (1 << 0)
#define LCDC_SCALER_FORMAT(n)                                   (((n) & 0x3) << 1)
#define LCDC_SCALER_FORMAT_MASK                                 (0x3 << 1)
#define LCDC_SCALER_FORMAT_SHIFT                                (1)
#define LCDC_SCALER_PRECISION(n)                                (((n) & 0x3) << 3)
#define LCDC_SCALER_PRECISION_MASK                              (0x3 << 3)
#define LCDC_SCALER_PRECISION_SHIFT                             (3)
#define LCDC_SCALER_FORCE_CLK_ON                                (1 << 5)
#define LCDC_SCALER_R_HORI_EN                                   (1 << 6)
#define LCDC_SCALER_R_VERT_EN                                   (1 << 7)
#define LCDC_SCALER_START_PTR(n)                                (((n) & 0x1FFFF) << 8)
#define LCDC_SCALER_START_PTR_MASK                              (0x1FFFF << 8)
#define LCDC_SCALER_START_PTR_SHIFT                             (8)
#define LCDC_SCALER_VLD_TX_CTRL_SEL                             (1 << 25)
#define LCDC_SCALER_QSPI_INTR                                   (1 << 26)

// reg_358
#define LCDC_SCALER_IMG_IN_HTOTAL(n)                            (((n) & 0xFFFF) << 0)
#define LCDC_SCALER_IMG_IN_HTOTAL_MASK                          (0xFFFF << 0)
#define LCDC_SCALER_IMG_IN_HTOTAL_SHIFT                         (0)
#define LCDC_SCALER_IMG_OUT_HTOTAL(n)                           (((n) & 0xFFFF) << 16)
#define LCDC_SCALER_IMG_OUT_HTOTAL_MASK                         (0xFFFF << 16)
#define LCDC_SCALER_IMG_OUT_HTOTAL_SHIFT                        (16)

// reg_35c
#define LCDC_SCALER_IMG_IN_WIDTH(n)                             (((n) & 0xFFF) << 0)
#define LCDC_SCALER_IMG_IN_WIDTH_MASK                           (0xFFF << 0)
#define LCDC_SCALER_IMG_IN_WIDTH_SHIFT                          (0)
#define LCDC_SCALER_IMG_IN_HEIGHT(n)                            (((n) & 0xFFF) << 16)
#define LCDC_SCALER_IMG_IN_HEIGHT_MASK                          (0xFFF << 16)
#define LCDC_SCALER_IMG_IN_HEIGHT_SHIFT                         (16)

// reg_360
#define LCDC_SCALER_IMG_OUT_WIDTH(n)                            (((n) & 0xFFF) << 0)
#define LCDC_SCALER_IMG_OUT_WIDTH_MASK                          (0xFFF << 0)
#define LCDC_SCALER_IMG_OUT_WIDTH_SHIFT                         (0)
#define LCDC_SCALER_IMG_OUT_HEIGHT(n)                           (((n) & 0xFFF) << 16)
#define LCDC_SCALER_IMG_OUT_HEIGHT_MASK                         (0xFFF << 16)
#define LCDC_SCALER_IMG_OUT_HEIGHT_SHIFT                        (16)

// reg_364
#define LCDC_SCALER_R_HORI_PHASE_INCR(n)                        (((n) & 0x1FFFFF) << 0)
#define LCDC_SCALER_R_HORI_PHASE_INCR_MASK                      (0x1FFFFF << 0)
#define LCDC_SCALER_R_HORI_PHASE_INCR_SHIFT                     (0)
#define LCDC_SCALER_R_VERT_UP_CUT(n)                            (((n) & 0x7FF) << 21)
#define LCDC_SCALER_R_VERT_UP_CUT_MASK                          (0x7FF << 21)
#define LCDC_SCALER_R_VERT_UP_CUT_SHIFT                         (21)

// reg_368
#define LCDC_SCALER_R_VERT_PHASE_INCR(n)                        (((n) & 0x1FFFFF) << 0)
#define LCDC_SCALER_R_VERT_PHASE_INCR_MASK                      (0x1FFFFF << 0)
#define LCDC_SCALER_R_VERT_PHASE_INCR_SHIFT                     (0)
#define LCDC_SCALER_R_VERT_BOT_CUT(n)                           (((n) & 0x7FF) << 21)
#define LCDC_SCALER_R_VERT_BOT_CUT_MASK                         (0x7FF << 21)
#define LCDC_SCALER_R_VERT_BOT_CUT_SHIFT                        (21)

// reg_36c
#define LCDC_SCALER_R_HORI_INIT(n)                              (((n) & 0x7FFFFFFF) << 0)
#define LCDC_SCALER_R_HORI_INIT_MASK                            (0x7FFFFFFF << 0)
#define LCDC_SCALER_R_HORI_INIT_SHIFT                           (0)

// reg_370 ~ reg_4ec
// scaler hori phase tables

// reg_4f0 ~ reg_5ac
// scaler vert phase tables

// reg_5b0
#define LCDC_SRC_ALPHA_SEL_FIRST_L(n)                           (((n) & 0x3) << 0)
#define LCDC_SRC_ALPHA_SEL_FIRST_L_MASK                         (0x3 << 0)
#define LCDC_SRC_ALPHA_SEL_FIRST_L_SHIFT                        (0)
#define LCDC_DST_ALPHA_SEL_FIRST_L(n)                           (((n) & 0x3) << 2)
#define LCDC_DST_ALPHA_SEL_FIRST_L_MASK                         (0x3 << 2)
#define LCDC_DST_ALPHA_SEL_FIRST_L_SHIFT                        (2)
#define LCDC_BOTH_PIX_SEL_FIRST_L(n)                            (((n) & 0x3) << 4)
#define LCDC_BOTH_PIX_SEL_FIRST_L_MASK                          (0x3 << 4)
#define LCDC_BOTH_PIX_SEL_FIRST_L_SHIFT                         (4)
#define LCDC_SRC_PIX_SEL_FIRST_L                                (1 << 6)
#define LCDC_DST_PIX_SEL_FIRST_L                                (1 << 8)
#define LCDC_ALPHA_SRC_VLD_FIRST_L                              (1 << 10)
#define LCDC_ALPHA_DST_VLD_FIRST_L                              (1 << 11)
#define LCDC_ALPHA_BOTH_VLD_FIRST_L                             (1 << 12)
#define LCDC_SRC_ALPHA_SEL_SECOND_L(n)                          (((n) & 0x3) << 16)
#define LCDC_SRC_ALPHA_SEL_SECOND_L_MASK                        (0x3 << 16)
#define LCDC_SRC_ALPHA_SEL_SECOND_L_SHIFT                       (16)
#define LCDC_DST_ALPHA_SEL_SECOND_L(n)                          (((n) & 0x3) << 18)
#define LCDC_DST_ALPHA_SEL_SECOND_L_MASK                        (0x3 << 18)
#define LCDC_DST_ALPHA_SEL_SECOND_L_SHIFT                       (18)
#define LCDC_BOTH_PIX_SEL_SECOND_L(n)                           (((n) & 0x3) << 20)
#define LCDC_BOTH_PIX_SEL_SECOND_L_MASK                         (0x3 << 20)
#define LCDC_BOTH_PIX_SEL_SECOND_L_SHIFT                        (20)
#define LCDC_SRC_PIX_SEL_SECOND_L                               (1 << 22)
#define LCDC_DST_PIX_SEL_SECOND_L                               (1 << 24)
#define LCDC_ALPHA_SRC_VLD_SECOND_L                             (1 << 26)
#define LCDC_ALPHA_DST_VLD_SECOND_L                             (1 << 27)
#define LCDC_ALPHA_BOTH_VLD_SECOND_L                            (1 << 28)

// reg_5b4
#define LCDC_SRC_ALPHA_SEL_THIRD_L(n)                           (((n) & 0x3) << 0)
#define LCDC_SRC_ALPHA_SEL_THIRD_L_MASK                         (0x3 << 0)
#define LCDC_SRC_ALPHA_SEL_THIRD_L_SHIFT                        (0)
#define LCDC_DST_ALPHA_SEL_THIRD_L(n)                           (((n) & 0x3) << 2)
#define LCDC_DST_ALPHA_SEL_THIRD_L_MASK                         (0x3 << 2)
#define LCDC_DST_ALPHA_SEL_THIRD_L_SHIFT                        (2)
#define LCDC_BOTH_PIX_SEL_THIRD_L(n)                            (((n) & 0x3) << 4)
#define LCDC_BOTH_PIX_SEL_THIRD_L_MASK                          (0x3 << 4)
#define LCDC_BOTH_PIX_SEL_THIRD_L_SHIFT                         (4)
#define LCDC_SRC_PIX_SEL_THIRD_L                                (1 << 6)
#define LCDC_DST_PIX_SEL_THIRD_L                                (1 << 8)
#define LCDC_ALPHA_SRC_VLD_THIRD_L                              (1 << 10)
#define LCDC_ALPHA_DST_VLD_THIRD_L                              (1 << 11)
#define LCDC_ALPHA_BOTH_VLD_THIRD_L                             (1 << 12)
#define LCDC_SRC_ALPHA_SEL_FOURTH_L(n)                          (((n) & 0x3) << 16)
#define LCDC_SRC_ALPHA_SEL_FOURTH_L_MASK                        (0x3 << 16)
#define LCDC_SRC_ALPHA_SEL_FOURTH_L_SHIFT                       (16)
#define LCDC_DST_ALPHA_SEL_FOURTH_L(n)                          (((n) & 0x3) << 18)
#define LCDC_DST_ALPHA_SEL_FOURTH_L_MASK                        (0x3 << 18)
#define LCDC_DST_ALPHA_SEL_FOURTH_L_SHIFT                       (18)
#define LCDC_BOTH_PIX_SEL_FOURTH_L(n)                           (((n) & 0x3) << 20)
#define LCDC_BOTH_PIX_SEL_FOURTH_L_MASK                         (0x3 << 20)
#define LCDC_BOTH_PIX_SEL_FOURTH_L_SHIFT                        (20)
#define LCDC_SRC_PIX_SEL_FOURTH_L                               (1 << 22)
#define LCDC_DST_PIX_SEL_FOURTH_L                               (1 << 24)
#define LCDC_ALPHA_SRC_VLD_FOURTH_L                             (1 << 26)
#define LCDC_ALPHA_DST_VLD_FOURTH_L                             (1 << 27)
#define LCDC_ALPHA_BOTH_VLD_FOURTH_L                            (1 << 28)

// reg_5b8
#define LCDC_SRC_ALPHA_SEL_FIRST_R(n)                           (((n) & 0x3) << 0)
#define LCDC_SRC_ALPHA_SEL_FIRST_R_MASK                         (0x3 << 0)
#define LCDC_SRC_ALPHA_SEL_FIRST_R_SHIFT                        (0)
#define LCDC_DST_ALPHA_SEL_FIRST_R(n)                           (((n) & 0x3) << 2)
#define LCDC_DST_ALPHA_SEL_FIRST_R_MASK                         (0x3 << 2)
#define LCDC_DST_ALPHA_SEL_FIRST_R_SHIFT                        (2)
#define LCDC_BOTH_PIX_SEL_FIRST_R(n)                            (((n) & 0x3) << 4)
#define LCDC_BOTH_PIX_SEL_FIRST_R_MASK                          (0x3 << 4)
#define LCDC_BOTH_PIX_SEL_FIRST_R_SHIFT                         (4)
#define LCDC_SRC_PIX_SEL_FIRST_R                                (1 << 6)
#define LCDC_DST_PIX_SEL_FIRST_R                                (1 << 8)
#define LCDC_ALPHA_SRC_VLD_FIRST_R                              (1 << 10)
#define LCDC_ALPHA_DST_VLD_FIRST_R                              (1 << 11)
#define LCDC_ALPHA_BOTH_VLD_FIRST_R                             (1 << 12)
#define LCDC_SRC_ALPHA_SEL_SECOND_R(n)                          (((n) & 0x3) << 16)
#define LCDC_SRC_ALPHA_SEL_SECOND_R_MASK                        (0x3 << 16)
#define LCDC_SRC_ALPHA_SEL_SECOND_R_SHIFT                       (16)
#define LCDC_DST_ALPHA_SEL_SECOND_R(n)                          (((n) & 0x3) << 18)
#define LCDC_DST_ALPHA_SEL_SECOND_R_MASK                        (0x3 << 18)
#define LCDC_DST_ALPHA_SEL_SECOND_R_SHIFT                       (18)
#define LCDC_BOTH_PIX_SEL_SECOND_R(n)                           (((n) & 0x3) << 20)
#define LCDC_BOTH_PIX_SEL_SECOND_R_MASK                         (0x3 << 20)
#define LCDC_BOTH_PIX_SEL_SECOND_R_SHIFT                        (20)
#define LCDC_SRC_PIX_SEL_SECOND_R                               (1 << 22)
#define LCDC_DST_PIX_SEL_SECOND_R                               (1 << 24)
#define LCDC_ALPHA_SRC_VLD_SECOND_R                             (1 << 26)
#define LCDC_ALPHA_DST_VLD_SECOND_R                             (1 << 27)
#define LCDC_ALPHA_BOTH_VLD_SECOND_R                            (1 << 28)

// reg_5bc
#define LCDC_SRC_ALPHA_SEL_THIRD_R(n)                           (((n) & 0x3) << 0)
#define LCDC_SRC_ALPHA_SEL_THIRD_R_MASK                         (0x3 << 0)
#define LCDC_SRC_ALPHA_SEL_THIRD_R_SHIFT                        (0)
#define LCDC_DST_ALPHA_SEL_THIRD_R(n)                           (((n) & 0x3) << 2)
#define LCDC_DST_ALPHA_SEL_THIRD_R_MASK                         (0x3 << 2)
#define LCDC_DST_ALPHA_SEL_THIRD_R_SHIFT                        (2)
#define LCDC_BOTH_PIX_SEL_THIRD_R(n)                            (((n) & 0x3) << 4)
#define LCDC_BOTH_PIX_SEL_THIRD_R_MASK                          (0x3 << 4)
#define LCDC_BOTH_PIX_SEL_THIRD_R_SHIFT                         (4)
#define LCDC_SRC_PIX_SEL_THIRD_R                                (1 << 6)
#define LCDC_DST_PIX_SEL_THIRD_R                                (1 << 8)
#define LCDC_ALPHA_SRC_VLD_THIRD_R                              (1 << 10)
#define LCDC_ALPHA_DST_VLD_THIRD_R                              (1 << 11)
#define LCDC_ALPHA_BOTH_VLD_THIRD_R                             (1 << 12)
#define LCDC_SRC_ALPHA_SEL_FOURTH_R(n)                          (((n) & 0x3) << 16)
#define LCDC_SRC_ALPHA_SEL_FOURTH_R_MASK                        (0x3 << 16)
#define LCDC_SRC_ALPHA_SEL_FOURTH_R_SHIFT                       (16)
#define LCDC_DST_ALPHA_SEL_FOURTH_R(n)                          (((n) & 0x3) << 18)
#define LCDC_DST_ALPHA_SEL_FOURTH_R_MASK                        (0x3 << 18)
#define LCDC_DST_ALPHA_SEL_FOURTH_R_SHIFT                       (18)
#define LCDC_BOTH_PIX_SEL_FOURTH_R(n)                           (((n) & 0x3) << 20)
#define LCDC_BOTH_PIX_SEL_FOURTH_R_MASK                         (0x3 << 20)
#define LCDC_BOTH_PIX_SEL_FOURTH_R_SHIFT                        (20)
#define LCDC_SRC_PIX_SEL_FOURTH_R                               (1 << 22)
#define LCDC_DST_PIX_SEL_FOURTH_R                               (1 << 24)
#define LCDC_ALPHA_SRC_VLD_FOURTH_R                             (1 << 26)
#define LCDC_ALPHA_DST_VLD_FOURTH_R                             (1 << 27)
#define LCDC_ALPHA_BOTH_VLD_FOURTH_R                            (1 << 28)

// reg_5c0
#define LCDC_ALPHA_BLEND_Z_ORDER_L(n)                           (((n) & 0x1F) << 0)
#define LCDC_ALPHA_BLEND_Z_ORDER_L_MASK                         (0x1F << 0)
#define LCDC_ALPHA_BLEND_Z_ORDER_L_SHIFT                        (0)
#define LCDC_ALPHA_BLEND_Z_ORDER_R(n)                           (((n) & 0x1F) << 5)
#define LCDC_ALPHA_BLEND_Z_ORDER_R_MASK                         (0x1F << 5)
#define LCDC_ALPHA_BLEND_Z_ORDER_R_SHIFT                        (5)
#define LCDC_ALPHA_BLEND_USE_NEW_ALG                            (1 << 10)

// reg_5c8
#define LCDC_GLOBAL_ALPHA_FIRST_L(n)                            (((n) & 0xFF) << 0)
#define LCDC_GLOBAL_ALPHA_FIRST_L_MASK                          (0xFF << 0)
#define LCDC_GLOBAL_ALPHA_FIRST_L_SHIFT                         (0)
#define LCDC_GLOBAL_ALPHA_SECOND_L(n)                           (((n) & 0xFF) << 8)
#define LCDC_GLOBAL_ALPHA_SECOND_L_MASK                         (0xFF << 8)
#define LCDC_GLOBAL_ALPHA_SECOND_L_SHIFT                        (8)
#define LCDC_GLOBAL_ALPHA_THIRD_L(n)                            (((n) & 0xFF) << 16)
#define LCDC_GLOBAL_ALPHA_THIRD_L_MASK                          (0xFF << 16)
#define LCDC_GLOBAL_ALPHA_THIRD_L_SHIFT                         (16)
#define LCDC_GLOBAL_ALPHA_FOURTH_L(n)                           (((n) & 0xFF) << 24)
#define LCDC_GLOBAL_ALPHA_FOURTH_L_MASK                         (0xFF << 24)
#define LCDC_GLOBAL_ALPHA_FOURTH_L_SHIFT                        (24)

// reg_5cc
#define LCDC_GLOBAL_ALPHA_FIRST_R(n)                            (((n) & 0xFF) << 0)
#define LCDC_GLOBAL_ALPHA_FIRST_R_MASK                          (0xFF << 0)
#define LCDC_GLOBAL_ALPHA_FIRST_R_SHIFT                         (0)
#define LCDC_GLOBAL_ALPHA_SECOND_R(n)                           (((n) & 0xFF) << 8)
#define LCDC_GLOBAL_ALPHA_SECOND_R_MASK                         (0xFF << 8)
#define LCDC_GLOBAL_ALPHA_SECOND_R_SHIFT                        (8)
#define LCDC_GLOBAL_ALPHA_THIRD_R(n)                            (((n) & 0xFF) << 16)
#define LCDC_GLOBAL_ALPHA_THIRD_R_MASK                          (0xFF << 16)
#define LCDC_GLOBAL_ALPHA_THIRD_R_SHIFT                         (16)
#define LCDC_GLOBAL_ALPHA_FOURTH_R(n)                           (((n) & 0xFF) << 24)
#define LCDC_GLOBAL_ALPHA_FOURTH_R_MASK                         (0xFF << 24)
#define LCDC_GLOBAL_ALPHA_FOURTH_R_SHIFT                        (24)

// reg_5d0
#define LCDC_ALPHA_BLEND_PREMULTI_FIRST_L                       (1 << 0)
#define LCDC_ALPHA_BLEND_PREMULTI_SECOND_L                      (1 << 1)
#define LCDC_ALPHA_BLEND_PREMULTI_THIRD_L                       (1 << 2)
#define LCDC_ALPHA_BLEND_PREMULTI_FOURTH_L                      (1 << 3)
#define LCDC_ALPHA_BLEND_PREMULTI_FIRST_R                       (1 << 4)
#define LCDC_ALPHA_BLEND_PREMULTI_SECOND_R                      (1 << 5)
#define LCDC_ALPHA_BLEND_PREMULTI_THIRD_R                       (1 << 6)
#define LCDC_ALPHA_BLEND_PREMULTI_FOURTH_R                      (1 << 7)
#define LCDC_ALPHA_BLEND_COVERAGE_FIRST_L                       (1 << 8)
#define LCDC_ALPHA_BLEND_COVERAGE_SECOND_L                      (1 << 9)
#define LCDC_ALPHA_BLEND_COVERAGE_THIRD_L                       (1 << 10)
#define LCDC_ALPHA_BLEND_COVERAGE_FOURTH_L                      (1 << 11)
#define LCDC_ALPHA_BLEND_COVERAGE_FIRST_R                       (1 << 12)
#define LCDC_ALPHA_BLEND_COVERAGE_SECOND_R                      (1 << 13)
#define LCDC_ALPHA_BLEND_COVERAGE_THIRD_R                       (1 << 14)
#define LCDC_ALPHA_BLEND_COVERAGE_FOURTH_R                      (1 << 15)

// reg_5d4
#define LCDC_SRC_ALPHA_SEL_OUT_FIRST_L(n)                       (((n) & 0x3) << 0)
#define LCDC_DST_ALPHA_SEL_OUT_FIRST_L(n)                       (((n) & 0x3) << 2)
#define LCDC_SRC_ALPHA_SEL_OUT_SECOND_L(n)                      (((n) & 0x3) << 4)
#define LCDC_DST_ALPHA_SEL_OUT_SECOND_L(n)                      (((n) & 0x3) << 6)
#define LCDC_SRC_ALPHA_SEL_OUT_THIRD_L(n)                       (((n) & 0x3) << 8)
#define LCDC_DST_ALPHA_SEL_OUT_THIRD_L(n)                       (((n) & 0x3) << 10)
#define LCDC_SRC_ALPHA_SEL_OUT_FOURTH_L(n)                      (((n) & 0x3) << 12)
#define LCDC_DST_ALPHA_SEL_OUT_FOURTH_L(n)                      (((n) & 0x3) << 14)
#define LCDC_SRC_ALPHA_SEL_OUT_FIRST_R(n)                       (((n) & 0x3) << 16)
#define LCDC_DST_ALPHA_SEL_OUT_FIRST_R(n)                       (((n) & 0x3) << 18)
#define LCDC_SRC_ALPHA_SEL_OUT_SECOND_R(n)                      (((n) & 0x3) << 20)
#define LCDC_DST_ALPHA_SEL_OUT_SECOND_R(n)                      (((n) & 0x3) << 22)
#define LCDC_SRC_ALPHA_SEL_OUT_THIRD_R(n)                       (((n) & 0x3) << 24)
#define LCDC_DST_ALPHA_SEL_OUT_THIRD_R(n)                       (((n) & 0x3) << 26)
#define LCDC_SRC_ALPHA_SEL_OUT_FOURTH_R(n)                      (((n) & 0x3) << 28)
#define LCDC_DST_ALPHA_SEL_OUT_FOURTH_R(n)                      (((n) & 0x3) << 30)

// reg_5d8
#define LCDC_SWITCH_FRAME_START(n)                              (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SWITCH_FRAME_START_MASK                            (0xFFFFFFFF << 0)
#define LCDC_SWITCH_FRAME_START_SHIFT                           (0)

// reg_5dc
#define LCDC_V_SWITCH_START(n)                                  (((n) & 0xFFF) << 0)
#define LCDC_V_SWITCH_START_MASK                                (0xFFF << 0)
#define LCDC_V_SWITCH_START_SHIFT                               (0)
#define LCDC_H_SWITCH_START(n)                                  (((n) & 0xFFF) << 0)
#define LCDC_H_SWITCH_START_MASK                                (0xFFF << 16)
#define LCDC_H_SWITCH_START_SHIFT                               (16)
#define LCDC_GEN_SWITCH_START_SEL                               (1 << 28)

// reg_5f0
#define LCDC_SCALER_R_VERT_INIT(n)                              (((n) & 0x7FFFFFFF) << 0)
#define LCDC_SCALER_R_VERT_INIT_MASK                            (0x7FFFFFFF << 0)
#define LCDC_SCALER_R_VERT_INIT_SHIFT                           (0)

// reg_5f4
#define LCDC_SCALER_V_ACTIVE(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SCALER_V_ACTIVE_MASK                               (0xFFF << 0)
#define LCDC_SCALER_V_ACTIVE_SHIFT                              (0)
#define LCDC_SCALER_OVSA_VLN(n)                                 (((n) & 0xFFF) << 16)
#define LCDC_SCALER_OVSA_VLN_MASK                               (0xFFF << 16)
#define LCDC_SCALER_OVSA_VLN_SHIFT                              (16)

// reg_5f8
#define LCDC_SMT_OVSA_HPXL_SP0(n)                               (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVSA_HPXL_SP0_MASK                             (0xFFF << 0)
#define LCDC_SMT_OVSA_HPXL_SP0_SHIFT                            (0)
#define LCDC_SMT_OVSA_VLN_SP0(n)                                (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVSA_VLN_SP0_MASK                              (0xFFF << 16)
#define LCDC_SMT_OVSA_VLN_SP0_SHIFT                             (16)

// reg_5fc
#define LCDC_SMT_OVLEN_HPXL_SP0(n)                              (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVLEN_HPXL_SP0_MASK                            (0xFFF << 0)
#define LCDC_SMT_OVLEN_HPXL_SP0_SHIFT                           (0)
#define LCDC_SMT_OVLEN_VLN_SP0(n)                               (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVLEN_VLN_SP0_MASK                             (0xFFF << 16)
#define LCDC_SMT_OVLEN_VLN_SP0_SHIFT                            (16)

// reg_600
#define LCDC_SMT_OVSA_HPXL_SP1(n)                               (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVSA_HPXL_SP1_MASK                             (0xFFF << 0)
#define LCDC_SMT_OVSA_HPXL_SP1_SHIFT                            (0)
#define LCDC_SMT_OVSA_VLN_SP1(n)                                (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVSA_VLN_SP1_MASK                              (0xFFF << 16)
#define LCDC_SMT_OVSA_VLN_SP1_SHIFT                             (16)

// reg_604
#define LCDC_SMT_OVLEN_HPXL_SP1(n)                              (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVLEN_HPXL_SP1_MASK                            (0xFFF << 0)
#define LCDC_SMT_OVLEN_HPXL_SP1_SHIFT                           (0)
#define LCDC_SMT_OVLEN_VLN_SP1(n)                               (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVLEN_VLN_SP1_MASK                             (0xFFF << 16)
#define LCDC_SMT_OVLEN_VLN_SP1_SHIFT                            (16)

// reg_608
#define LCDC_SMT_OVSA_HPXL_SP2(n)                               (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVSA_HPXL_SP2_MASK                             (0xFFF << 0)
#define LCDC_SMT_OVSA_HPXL_SP2_SHIFT                            (0)
#define LCDC_SMT_OVSA_VLN_SP2(n)                                (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVSA_VLN_SP2_MASK                              (0xFFF << 16)
#define LCDC_SMT_OVSA_VLN_SP2_SHIFT                             (16)

// reg_60c
#define LCDC_SMT_OVLEN_HPXL_SP2(n)                              (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVLEN_HPXL_SP2_MASK                            (0xFFF << 0)
#define LCDC_SMT_OVLEN_HPXL_SP2_SHIFT                           (0)
#define LCDC_SMT_OVLEN_VLN_SP2(n)                               (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVLEN_VLN_SP2_MASK                             (0xFFF << 16)
#define LCDC_SMT_OVLEN_VLN_SP2_SHIFT                            (16)

// reg_610
#define LCDC_SMT_OVSA_HPXL_SP3(n)                               (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVSA_HPXL_SP3_MASK                             (0xFFF << 0)
#define LCDC_SMT_OVSA_HPXL_SP3_SHIFT                            (0)
#define LCDC_SMT_OVSA_VLN_SP3(n)                                (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVSA_VLN_SP3_MASK                              (0xFFF << 16)
#define LCDC_SMT_OVSA_VLN_SP3_SHIFT                             (16)

// reg_614
#define LCDC_SMT_OVLEN_HPXL_SP3(n)                              (((n) & 0xFFF) << 0)
#define LCDC_SMT_OVLEN_HPXL_SP3_MASK                            (0xFFF << 0)
#define LCDC_SMT_OVLEN_HPXL_SP3_SHIFT                           (0)
#define LCDC_SMT_OVLEN_VLN_SP3(n)                               (((n) & 0xFFF) << 16)
#define LCDC_SMT_OVLEN_VLN_SP3_MASK                             (0xFFF << 16)
#define LCDC_SMT_OVLEN_VLN_SP3_SHIFT                            (16)

// reg_618
#define LCDC_SMT_SA0(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SMT_SA0_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_SMT_SA0_SHIFT                                      (0)

// reg_61c
#define LCDC_SMT_SA1(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SMT_SA1_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_SMT_SA1_SHIFT                                      (0)

// reg_620
#define LCDC_SMT_SA2(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SMT_SA2_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_SMT_SA2_SHIFT                                      (0)

// reg_624
#define LCDC_SMT_SA3(n)                                         (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SMT_SA3_MASK                                       (0xFFFFFFFF << 0)
#define LCDC_SMT_SA3_SHIFT                                      (0)

// reg_628
#define LCDC_SMT_H_PIXEL_SA0(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SMT_H_PIXEL_SA0_MASK                               (0xFFF << 0)
#define LCDC_SMT_H_PIXEL_SA0_SHIFT                              (0)
#define LCDC_SMT_V_LINE_SA0(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_SMT_V_LINE_SA0_MASK                                (0xFFF << 16)
#define LCDC_SMT_V_LINE_SA0_SHIFT                               (16)

// reg_62c
#define LCDC_SMT_H_PIXEL_SA1(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SMT_H_PIXEL_SA1_MASK                               (0xFFF << 0)
#define LCDC_SMT_H_PIXEL_SA1_SHIFT                              (0)
#define LCDC_SMT_V_LINE_SA1(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_SMT_V_LINE_SA1_MASK                                (0xFFF << 16)
#define LCDC_SMT_V_LINE_SA1_SHIFT                               (16)

// reg_630
#define LCDC_SMT_H_PIXEL_SA2(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SMT_H_PIXEL_SA2_MASK                               (0xFFF << 0)
#define LCDC_SMT_H_PIXEL_SA2_SHIFT                              (0)
#define LCDC_SMT_V_LINE_SA2(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_SMT_V_LINE_SA2_MASK                                (0xFFF << 16)
#define LCDC_SMT_V_LINE_SA2_SHIFT                               (16)

// reg_634
#define LCDC_SMT_H_PIXEL_SA3(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_SMT_H_PIXEL_SA3_MASK                               (0xFFF << 0)
#define LCDC_SMT_H_PIXEL_SA3_SHIFT                              (0)
#define LCDC_SMT_V_LINE_SA3(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_SMT_V_LINE_SA3_MASK                                (0xFFF << 16)
#define LCDC_SMT_V_LINE_SA3_SHIFT                               (16)

// reg_63c
#define LCDC_SMT_NOVLAP_SPLIT_ENA                               (1 << 0)
#define LCDC_SMT_PMBURST                                        (1 << 1)
#define LCDC_SMT_BURST0TO3(n)                                   (((n) & 0x3) << 2)
#define LCDC_SMT_BURST0TO3_MASK                                 (0x3 << 2)
#define LCDC_SMT_BURST0TO3_SHIFT                                (2)
#define LCDC_SMT_NOBLEND                                        (1 << 4)
#define LCDC_SMT_CKEY                                           (1 << 5)
#define LCDC_SMT_TSTMODE                                        (1 << 6)
#define LCDC_SMT_ENA                                            (1 << 7)
#define LCDC_SMTFORMAT(n)                                       (((n) & 0xF) << 8)
#define LCDC_SMTFORMAT_MASK                                     (0xF << 8)
#define LCDC_SMTFORMAT_SHIFT                                    (8)
#define LCDC_SMT_SWAPYU                                         (1 << 12)
#define LCDC_SMT_CARRY                                          (1 << 13)
#define LCDC_SMT_ALPHA(n)                                       (((n) & 0xFF) << 16)
#define LCDC_SMT_ALPHA_MASK                                     (0xFF << 16)
#define LCDC_SMT_ALPHA_SHIFT                                    (16)
#define LCDC_SMT_ALPHA_CONST_MODE                               (0 << 24)
#define LCDC_SMT_ALPHA_PIXEL_MODE                               (1 << 24)

// reg_640
#define LCDC_SMT_PITCH_SP0(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_SMT_PITCH_SP0_MASK                                 (0xFFFF << 0)
#define LCDC_SMT_PITCH_SP0_SHIFT                                (0)
#define LCDC_SMT_PITCH_SP1(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_SMT_PITCH_SP1_MASK                                 (0xFFFF << 16)
#define LCDC_SMT_PITCH_SP1_SHIFT                                (16)

// reg_644
#define LCDC_SMT_PITCH_SP2(n)                                   (((n) & 0xFFFF) << 0)
#define LCDC_SMT_PITCH_SP2_MASK                                 (0xFFFF << 0)
#define LCDC_SMT_PITCH_SP2_SHIFT                                (0)
#define LCDC_SMT_PITCH_SP3(n)                                   (((n) & 0xFFFF) << 16)
#define LCDC_SMT_PITCH_SP3_MASK                                 (0xFFFF << 16)
#define LCDC_SMT_PITCH_SP3_SHIFT                                (16)

// reg_648
#define LCDC_SMT_SA(n)                                          (((n) & 0xFFFFFFFF) << 0)
#define LCDC_SMT_SA_MASK                                        (0xFFFFFFFF << 0)
#define LCDC_SMT_SA_SHIFT                                       (0)

// reg_64c
#define LCDC_SMT_PITCH(n)                                       (((n) & 0xFFFF) << 0)
#define LCDC_SMT_PITCH_MASK                                     (0xFFFF << 0)
#define LCDC_SMT_PITCH_SHIFT                                    (0)

// reg_650
#define LCDC_SMT_H_PIXEL(n)                                     (((n) & 0xFFF) << 0)
#define LCDC_SMT_H_PIXEL_MASK                                   (0xFFF << 0)
#define LCDC_SMT_H_PIXEL_SHIFT                                  (0)
#define LCDC_SMT_V_LINE(n)                                      (((n) & 0xFFF) << 16)
#define LCDC_SMT_V_LINE_MASK                                    (0xFFF << 16)
#define LCDC_SMT_V_LINE_SHIFT                                   (16)

// reg_658 ~ reg_684
// scaler-2 copy from scaler-1 reg_340 ~ reg_36c

// reg_688 ~ reg_68c
// scaler-2 copy from scaler-1 reg_5f0 ~ reg_5f4

// reg_690
#define LCDC_TFBDC_ENA                                          (1 << 0)
#define LCDC_TFBDC_H_WIDTH_L(n)                                 (((n) & 0x3FF) << 1)
#define LCDC_TFBDC_H_WIDTH_L_MASK                               (0x3FF << 1)
#define LCDC_TFBDC_H_WIDTH_L_SHIFT                              (1)
#define LCDC_TFBDC_ACTIVE_H_WIDTH_L(n)                          (((n) & 0x3FF) << 11)
#define LCDC_TFBDC_ACTIVE_H_WIDTH_L_MASK                        (0x3FF << 11)
#define LCDC_TFBDC_ACTIVE_H_WIDTH_L_SHIFT                       (11)
#define LCDC_TFBDC_UNDERFLOW_START_CNT(n)                       (((n) & 0xF) << 21)
#define LCDC_TFBDC_UNDERFLOW_START_CNT_MASK                     (0xF << 21)
#define LCDC_TFBDC_UNDERFLOW_START_CNT_SHIFT                    (21)
#define LCDC_TFBDC_FORMAT(n)                                    (((n) & 0x3) << 25)
#define LCDC_TFBDC_FORMAT_MASK                                  (0x3 << 25)
#define LCDC_TFBDC_FORMAT_SHIFT                                 (25)
#define LCDC_TFBDC_32B                                          (1 << 27)
#define LCDC_TFBDC_24B                                          (1 << 28)
#define LCDC_TFBDC_16B                                          (1 << 29)
#define LCDC_TFBDC_LINEEND_MASK                                 (1 << 30)
#define LCDC_TFBDC_LINEEND_MASK_0                               (1 << 31)

// reg_694
#define LCDC_TFBDC_USE_QSPI                                     (1 << 0)
#define LCDC_TFBDC_SWAP_RB                                      (1 << 1)
#define LCDC_TFBDC_DATA_DELAY(n)                                (((n) & 0x3) << 2)
#define LCDC_TFBDC_DATA_DELAY_MASK                              (0x3 << 2)
#define LCDC_TFBDC_DATA_DELAY_SHIFT                             (2)
#define LCDC_TFBDC_REQ_LOSSY(n)                                 (((n) & 0x3) << 4)
#define LCDC_TFBDC_REQ_LOSSY_MASK                               (0x3 << 4)
#define LCDC_TFBDC_REQ_LOSSY_SHIFT                              (4)
#define LCDC_TFBDC_REQ_FORMAT(n)                                (((n) & 0x7F) << 6)
#define LCDC_TFBDC_REQ_FORMAT_MASK                              (0x7F << 6)
#define LCDC_TFBDC_REQ_FORMAT_SHIFT                             (6)
#define LCDC_TFBDC_ARQOS(n)                                     (((n) & 0xF) << 13)
#define LCDC_TFBDC_ARQOS_MASK                                   (0xF << 13)
#define LCDC_TFBDC_ARQOS_SHIFT                                  (13)
#define LCDC_TFBDC_REQ_TILE_LINENUM(n)                          (((n) & 0xFF) << 17)
#define LCDC_TFBDC_REQ_TILE_LINENUM_MASK                        (0xFF << 17)
#define LCDC_TFBDC_REQ_TILE_LINENUM_SHIFT                       (17)
#define LCDC_TFBDC_V_MIRROR                                     (1 << 26)
#define LCDC_TFBDC_UNMSK_VSYNC_INACLK                           (1 << 27)
#define LCDC_TFBDC_H_WIDTH_H                                    (1 << 28)
#define LCDC_TFBDC_ACTIVE_H_WIDTH_H                             (1 << 29)
#define LCDC_TFBDC_UNMSK_TRIG_INACLK                            (1 << 30)

// reg_698
#define LCDC_TFBDC_REQ_TILE_NUM(n)                              (((n) & 0x7FFFFF) << 0)
#define LCDC_TFBDC_REQ_TILE_NUM_MASK                            (0x7FFFFF << 0)
#define LCDC_TFBDC_REQ_TILE_NUM_SHIFT                           (0)
#define LCDC_TFBDC_REQ_BASE_ADDR_MSB                            (1 << 23)
#define LCDC_TFBDC_VMIRROR_OVERHEAD(n)                          (((n) & 0xFF) << 24)
#define LCDC_TFBDC_VMIRROR_OVERHEAD_MASK                        (0xFF << 24)
#define LCDC_TFBDC_VMIRROR_OVERHEAD_SHIFT                       (24)

// reg_69c
#define LCDC_TFBDC_REQ_BASE_ADDR(n)                              (((n) & 0xFFFFFFFF) << 0)
#define LCDC_TFBDC_REQ_BASE_ADDR_MASK                            (0xFFFFFFFF << 0)
#define LCDC_TFBDC_REQ_BASE_ADDR_SHIFT                           (0)

// reg_6b4
#define LCDC_WB_CF_HBLANK(n)                                    (((n) & 0xFFF) << 0)
#define LCDC_WB_CF_HBLANK_MASK                                  (0xFFF << 0)
#define LCDC_WB_CF_HBLANK_SHIFT                                 (0)
#define LCDC_WB_CF_ENABLE                                       (1 << 16)
#define LCDC_WDMA_SEL_ALPHA_TV_PN                               (1 << 17)
#define LCDC_WDMA_SEL_DATA_TV_PN                                (1 << 18)

// reg_6fc
#define LCDC_TFBDC_REQ_START_TILE(n)                            (((n) & 0x7FFFFF) << 0)
#define LCDC_TFBDC_REQ_START_TILE_MASK                          (0x7FFFFF << 0)
#define LCDC_TFBDC_REQ_START_TILE_SHIFT                         (0)

// reg_700
#define LCDC_ALPHA_DMA_NEW_ALG_SEL_GLOBAL_L                     (1 << 0)
#define LCDC_ALPHA_GRA_NEW_ALG_SEL_GLOBAL_L                     (1 << 1)
#define LCDC_ALPHA_TVD_NEW_ALG_SEL_GLOBAL_L                     (1 << 2)
#define LCDC_ALPHA_TVG_NEW_ALG_SEL_GLOBAL_L                     (1 << 3)
#define LCDC_ALPHA_SMT_NEW_ALG_SEL_GLOBAL_L                     (1 << 4)
#define LCDC_ALPHA_DMA_NEW_ALG_SEL_GLOBAL_R                     (1 << 5)
#define LCDC_ALPHA_GRA_NEW_ALG_SEL_GLOBAL_R                     (1 << 6)
#define LCDC_ALPHA_TVD_NEW_ALG_SEL_GLOBAL_R                     (1 << 7)
#define LCDC_ALPHA_TVG_NEW_ALG_SEL_GLOBAL_R                     (1 << 8)
#define LCDC_ALPHA_SMT_NEW_ALG_SEL_GLOBAL_R                     (1 << 9)

// reg_704
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_L(n)                      (((n) & 0xFF) << 0)
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_L_MASK                    (0xFF << 0)
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_L_SHIFT                   (0)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_L(n)                      (((n) & 0xFF) << 8)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_L_MASK                    (0xFF << 8)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_L_SHIFT                   (8)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_L(n)                      (((n) & 0xFF) << 16)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_L_MASK                    (0xFF << 16)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_L_SHIFT                   (16)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_L(n)                      (((n) & 0xFF) << 24)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_L_MASK                    (0xFF << 24)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_L_SHIFT                   (24)

// reg_708
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_R(n)                      (((n) & 0xFF) << 0)
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_R_MASK                    (0xFF << 0)
#define LCDC_ALPHA_DMA_NEW_ALG_GLOBAL_R_SHIFT                   (0)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_R(n)                      (((n) & 0xFF) << 8)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_R_MASK                    (0xFF << 8)
#define LCDC_ALPHA_GRA_NEW_ALG_GLOBAL_R_SHIFT                   (8)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_R(n)                      (((n) & 0xFF) << 16)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_R_MASK                    (0xFF << 16)
#define LCDC_ALPHA_TVD_NEW_ALG_GLOBAL_R_SHIFT                   (16)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_R(n)                      (((n) & 0xFF) << 24)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_R_MASK                    (0xFF << 24)
#define LCDC_ALPHA_TVG_NEW_ALG_GLOBAL_R_SHIFT                   (24)

// reg_70c
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_L(n)                      (((n) & 0xFF) << 0)
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_L_MASK                    (0xFF << 0)
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_L_SHIFT                   (0)
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_R(n)                      (((n) & 0xFF) << 8)
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_R_MASK                    (0xFF << 8)
#define LCDC_ALPHA_SMT_NEW_ALG_GLOBAL_R_SHIFT                   (8)
#define LCDC_GAMMA_IN_ALPHA_SEL_PN(n)                           (((n) & 0x7) << 16)
#define LCDC_GAMMA_IN_ALPHA_SEL_PN_MASK                         (0x7 << 16)
#define LCDC_GAMMA_IN_ALPHA_SEL_PN_SHIFT                        (16)
#define LCDC_GAMMA_IN_ALPHA_SEL_TV(n)                           (((n) & 0x7) << 19)
#define LCDC_GAMMA_IN_ALPHA_SEL_TV_MASK                         (0x7 << 19)
#define LCDC_GAMMA_IN_ALPHA_SEL_TV_SHIFT                        (19)
#define LCDC_PRE_COV_SEL_INVERT_FIRST_L                         (1 << 22)
#define LCDC_PRE_COV_SEL_INVERT_SECOND_L                        (1 << 23)
#define LCDC_PRE_COV_SEL_INVERT_THIRD_L                         (1 << 24)
#define LCDC_PRE_COV_SEL_INVERT_FOURTH_L                        (1 << 25)
#define LCDC_PRE_COV_SEL_INVERT_FIRST_R                         (1 << 26)
#define LCDC_PRE_COV_SEL_INVERT_SECOND_R                        (1 << 27)
#define LCDC_PRE_COV_SEL_INVERT_THIRD_R                         (1 << 28)
#define LCDC_PRE_COV_SEL_INVERT_FOURTH_R                        (1 << 29)

// reg_710
#define LCDC_LINE_NUM_MIRROR(n)                                 (((n) & 0x3FF) << 0)
#define LCDC_LINE_NUM_MIRROR_MASK                               (0x3FF << 0)
#define LCDC_LINE_NUM_MIRROR_SHIFT                              (0)
#define LCDC_LINE_WIDTH_MIRROR(n)                               (((n) & 0x3FF) << 12)
#define LCDC_LINE_WIDTH_MIRROR_MASK                             (0x3FF << 12)
#define LCDC_LINE_WIDTH_MIRROR_SHIFT                            (12)
#define LCDC_SPU_MIRROR_EN                                      (1 << 24)

// reg_714 (reg_2d4 for 2009)
#define LCDC_GRA_H_PIXEL_ROT(n)                                 (((n) & 0xFFF) << 0)
#define LCDC_GRA_H_PIXEL_ROT_MASK                               (0xFFF << 0)
#define LCDC_GRA_H_PIXEL_ROT_SHIFT                              (0)
#define LCDC_GRA_V_LINE_ROT(n)                                  (((n) & 0xFFF) << 16)
#define LCDC_GRA_V_LINE_ROT_MASK                                (0xFFF << 16)
#define LCDC_GRA_V_LINE_ROT_SHIFT                               (16)

// reg_718 (reg_2d8 for 2009)
#define LCDC_GRARA_ENA                                          (1 << 0)
#define LCDC_GRA_ROT_90                                         (1 << 1)
#define LCDC_GRA_ROT_270                                        (1 << 2)
#define LCDC_LONG_WIDTH_ROT                                     (1 << 3)
#define LCDC_GRARA_ENA_1                                        (1 << 4)
#define LCDC_TILED_ROT_ENA                                      (1 << 5)
#define LCDC_GRA_H_PIXEL_ROT_ACTIVE(n)                          (((n) & 0xFFF) << 16)
#define LCDC_GRA_H_PIXEL_ROT_ACTIVE_MASK                        (0xFFF << 16)
#define LCDC_GRA_H_PIXEL_ROT_ACTIVE_SHIFT                       (16)

// reg_71c
#define LCDC_READ_DELAY_CNT(n)                                  (((n) & 0x3FF) << 0)
#define LCDC_READ_DELAY_CNT_MASK                                (0x3FF << 0)
#define LCDC_READ_DELAY_CNT_SHIFT                               (0)
#define LCDC_WRITE_WAIT                                         (1 << 11)
#define LCDC_WRITE_DELAY_CNT(n)                                 (((n) & 0xFFF) << 12)
#define LCDC_WRITE_DELAY_CNT_MASK                               (0xFFF << 12)
#define LCDC_WRITE_DELAY_CNT_SHIFT                              (12)

// reg_720
#define LCDC_GRA_CFF_FULL_MARK                                  (1 << 0)
#define LCDC_SMT_CFF_FULL_MARK                                  (1 << 1)
#define LCDC_TVG_CFF_FULL_MARK                                  (1 << 2)
#define LCDC_DMA_CFF_FULL_MARK                                  (1 << 3)
#define LCDC_TVD_CFF_FULL_MARK                                  (1 << 4)
#define LCDC_ALPHA_BLEND_SEL_FG_L1_L                            (1 << 5)
#define LCDC_ALPHA_BLEND_SEL_FG_L1_R                            (1 << 6)
#define LCDC_V_SWITCH_AP_LP(n)                                  (((n) & 0xFFF) << 8)
#define LCDC_V_SWITCH_AP_LP_MASK                                (0xFFF << 8)
#define LCDC_V_SWITCH_AP_LP_SHIFT                               (8)
#define LCDC_H_SWITCH_AP_LP(n)                                  (((n) & 0xFFF) << 20)
#define LCDC_H_SWITCH_AP_LP_MASK                                (0xFFF << 20)
#define LCDC_H_SWITCH_AP_LP_SHIFT                               (20)

// reg_724
#define LCDC_TFBDC_VMIRROR_REMAIN_TILES(n)                      (((n) & 0x3FF) << 0)
#define LCDC_TFBDC_VMIRROR_REMAIN_TILES_MASK                    (0x3FF << 0)
#define LCDC_TFBDC_VMIRROR_REMAIN_TILES_SHIFT                   (0)
#define LCDC_YUV420P_16_LOL_BASE(n)                             (((n) & 0x3F) << 10)
#define LCDC_YUV420P_16_LOL_BASE_MASK                           (0x3F << 10)
#define LCDC_YUV420P_16_LOL_BASE_SHIFT                          (10)
#define LCDC_64TOPIXEL_RD_CNT_DMA(n)                            (((n) & 0x3F) << 16)
#define LCDC_64TOPIXEL_RD_CNT_DMA_MASK                          (0x3F << 16)
#define LCDC_64TOPIXEL_RD_CNT_DMA_SHIFT                         (16)
#define LCDC_SCA_64TOPIXEL_RD_CNT_DMA(n)                        (((n) & 0x3F) << 22)
#define LCDC_SCA_64TOPIXEL_RD_CNT_DMA_MASK                      (0x3F << 22)
#define LCDC_SCA_64TOPIXEL_RD_CNT_DMA_SHIFT                     (22)

// reg_728
#define LCDC_64TOPIXEL_RD_CNT_TVD(n)                            (((n) & 0x3F) << 0)
#define LCDC_64TOPIXEL_RD_CNT_TVD_MASK                          (0x3F << 0)
#define LCDC_64TOPIXEL_RD_CNT_TVD_SHIFT                         (0)
#define LCDC_SCA_64TOPIXEL_RD_CNT_TVD(n)                        (((n) & 0x3F) << 6)
#define LCDC_SCA_64TOPIXEL_RD_CNT_TVD_MASK                      (0x3F << 6)
#define LCDC_SCA_64TOPIXEL_RD_CNT_TVD_SHIFT                     (6)


#endif /* __REG_LCDC_H__ */

