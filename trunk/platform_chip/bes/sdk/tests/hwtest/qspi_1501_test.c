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
#ifdef QSPI_1501_TEST
#include "string.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "qspi_1501.h"

#define ROW  360
#define COL  360

#define RED    0xF800
#define GREEN  0x07E0
#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define GRAY   0xEF5D	     //0x2410
#define GRAY75 0x39E7
#define GRAY50 0x7BEF
#define GRAY25 0xADB5

POSSIBLY_UNUSED static const uint16_t rgb565_raw_data_360x360[] = {
#include "display_resource/RGB565_unpacked_360x360.txt"
};

uint8_t ALIGNED(4) frame_buf[ROW*COL*2];
uint8_t ALIGNED(4) cmd_buf[50];

void init_9c01()
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;// internal reg enable
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;// internal reg enable
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x80;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;//reg_en for 70\74
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x81;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x70;//reg_en for 7C\7D\7E
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x82;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;//reg_en for 90\93
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x83;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x03;//reg_en for 98\99
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x84;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x20;//reg_en for B5
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x85;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x42;//reg_en for B9\BE
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x86;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xfc;//reg_en for C2~7
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x87;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;//reg_en for C8\CB
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x89;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x10;//reg_en for EC
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8a;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x4f;//reg_en for F0~3\F6
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8c;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x59;//reg_en for 60\63\64\66
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8d;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x51;//reg_en for 68\6C\6E
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xae;//reg_en for A1~3\A5\A7
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8f;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xf3;//reg_en for AC~F\A8\A9
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x36;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x00;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x3a;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x05;// 565 frame
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xec;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x77;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//2COL

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x74;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x80;
    cmd_buf[5] = 0x00;
    cmd_buf[6] = 0x00;
    cmd_buf[7] = 0x00;
    cmd_buf[8] = 0x00;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);//rtn 60Hz

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x98;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3E;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//bvdd 3x

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x99;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3E;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//bvee -2x

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x2A;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//VBP

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc4;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x18;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//VBN

    //*********************????*********************//

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa1;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x04;//SRAM RD OPTION
    qspi_1501_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x04;//SRAM RD OPTION
    qspi_1501_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa9;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x1C;//IREF=9.8uA
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;//VDDMA=1.553V
    cmd_buf[4] = 0x09;//VDDML=1.24V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xb9;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x8A;//RTERM=101O
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa8;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x5E;//VBG_BUF=1.003V, DVDD=1.543V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa7;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x40;//BIAS=10.2uA
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xaf;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x73;//VDDSOU=1.715V ,VDDGM=2.002V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xae;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x44;//VREE=2.475V,VRDD=2.335V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xad;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;//VRGL=1.635V ,VDDSF=2.018V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x5D;//OSC=53.7MHz
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;//VREG_VREF=2.805V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

	cmd_buf[0] = 0;
    cmd_buf[1] = 0xc5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;//VREG1A=5.99V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

	cmd_buf[0] = 0;
    cmd_buf[1] = 0xc6;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0E;//VREG1B=1.505V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

	cmd_buf[0] = 0;
    cmd_buf[1] = 0xc7;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x13;//VREG2A=-2.995V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

	cmd_buf[0] = 0;
    cmd_buf[1] = 0xc8;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0D;//VREG2B=1.497V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xcb;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;//6.09V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);//bvdd ref_ad

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7c;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xB6;
    cmd_buf[4] = 0x26;//13.12V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xac;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x24;//VGLO=-8.35V
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf6;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x80;//EPF=2
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    //*********************????*************************//
    //gip start

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xb5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;//VFP
    cmd_buf[4] = 0x09;//VBP
    qspi_1501_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x60;//STV1&2
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x0B;
    cmd_buf[5] = 0x5B;
    cmd_buf[6] = 0x56;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x63;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3A;
    cmd_buf[4] = 0xE0;//DE
    cmd_buf[5] = 0x5B;//MAX=0x61
    cmd_buf[6] = 0x56;//MAX=0x61
    qspi_1501_send_cmd_data(0x2, cmd_buf, 7);//STV3&4

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x64;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x0D;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0xDD;
    cmd_buf[7] = 0x5B;
    cmd_buf[8] = 0x56;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);//CLK_group1

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x66;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x11;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0xE1;
    cmd_buf[7] = 0x5B;
    cmd_buf[8] = 0x56;
	qspi_1501_send_cmd_data(0x2, cmd_buf, 9);//CLK_group1

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x68;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3B;//FLC12 FREQ
    cmd_buf[4] = 0x08;
    cmd_buf[5] = 0x08;
    cmd_buf[6] = 0x00;
    cmd_buf[7] = 0x08;
    cmd_buf[8] = 0x29;
    cmd_buf[9] = 0x5B;
	qspi_1501_send_cmd_data(0x2, cmd_buf, 10);//FLC&FLV 1~2

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x6e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x00;//gout1_swap_fw[4:0]
    cmd_buf[4] = 0x00;//gout2_swap_fw[4:0]
    cmd_buf[5] = 0x00;//gout3_swap_fw[4:0]
    cmd_buf[6] = 0x07;//gout4_swap_fw[4:0]
    cmd_buf[7] = 0x01;//gout5_swap_fw[4:0]
    cmd_buf[8] = 0x13;//gout6_swap_fw[4:0]
    cmd_buf[9] = 0x11;//gout7_swap_fw[4:0]
    cmd_buf[10] = 0x0B;//gout8_swap_fw[4:0]
    cmd_buf[11] = 0x09;//gout9_swap_fw[4:0]
    cmd_buf[12] = 0x16;//gout10_swap_fw[4:0]
    cmd_buf[13] = 0x15;//gout11_swap_fw[4:0]
    cmd_buf[14] = 0x1D;//gout12_swap_fw[4:0]
    cmd_buf[15] = 0x1E;//gout13_swap_fw[4:0]
    cmd_buf[16] = 0x00;//gout14_swap_fw[4:0]
    cmd_buf[17] = 0x00;//gout15_swap_fw[4:0]
    cmd_buf[18] = 0x00;//gout16_swap_fw[4:0]
    cmd_buf[19] = 0x00;//gout17_swap_fw[4:0]
    cmd_buf[20] = 0x00;//gout18_swap_fw[4:0]
    cmd_buf[21] = 0x00;//gout19_swap_fw[4:0]
    cmd_buf[22] = 0x1E;//gout20_swap_fw[4:0]
    cmd_buf[23] = 0x1D;//gout21_swap_fw[4:0]
    cmd_buf[24] = 0x15;//gout22_swap_fw[4:0]
    cmd_buf[25] = 0x16;//gout23_swap_fw[4:0]
    cmd_buf[26] = 0x0A;//gout24_swap_fw[4:0]
    cmd_buf[27] = 0x0C;//gout25_swap_fw[4:0]
    cmd_buf[28] = 0x12;//gout26_swap_fw[4:0]
    cmd_buf[29] = 0x14;//gout27_swap_fw[4:0]
    cmd_buf[30] = 0x02;//gout28_swap_fw[4:0]
    cmd_buf[31] = 0x08;//gout29_swap_fw[4:0]
    cmd_buf[32] = 0x00;//gout30_swap_fw[4:0]
    cmd_buf[33] = 0x00;//gout31_swap_fw[4:0]
    cmd_buf[34] = 0x00;//gout32_swap_fw[4:0]
    qspi_1501_send_cmd_data(0x2, cmd_buf, 35);//gout_Mapping

    //*********************?????**********************//

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xbe;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;//SOU_BIAS_FIX=1
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x6c;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xCC;
    cmd_buf[4] = 0x0C;
    cmd_buf[5] = 0xCC;
    cmd_buf[6] = 0x84;
    cmd_buf[7] = 0xCC;
    cmd_buf[8] = 0x04;
    cmd_buf[9] = 0x50;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 10);//precharge GATE

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7d;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x72;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;// VGL_BT=1 5X  (BT=0:6X)  RT=0
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x70;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;//avdd_clk
    cmd_buf[4] = 0x03;//avee_clk
    cmd_buf[5] = 0x09;//vdh_clk
    cmd_buf[6] = 0x05;//vgh_clk
    cmd_buf[7] = 0x0C;//vgl_clk
    cmd_buf[8] = 0x06;//vcl_clk
    cmd_buf[9] = 0x09;//vdh_clk_porch  0E
    cmd_buf[10] = 0x05;//vgh_clk_porch  0E
    cmd_buf[11] = 0x0C;//vgl_clk_porch  0E
    cmd_buf[12] = 0x06;//vcl_clk_porch  0E
    qspi_1501_send_cmd_data(0x2, cmd_buf, 13);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x90;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x06;
    cmd_buf[4] = 0x06;
    cmd_buf[5] = 0x05;//bvdd_clk1_ad1
    cmd_buf[6] = 0x06;//bvdd_clk1_ad1
    qspi_1501_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x93;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x45;//bvdd_shut_ad1
    cmd_buf[4] = 0xFF;
    cmd_buf[5] = 0x00;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 6);

    //gamma start
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf0;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x45;
    cmd_buf[4] = 0x09;
    cmd_buf[5] = 0x08;
    cmd_buf[6] = 0x08;
    cmd_buf[7] = 0x26;
    cmd_buf[8] = 0x2A;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf1;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x43;
    cmd_buf[4] = 0x70;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0x36;
    cmd_buf[7] = 0x37;
    cmd_buf[8] = 0x6F;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x45;
    cmd_buf[4] = 0x09;
    cmd_buf[5] = 0x08;
    cmd_buf[6] = 0x08;
    cmd_buf[7] = 0x26;
    cmd_buf[8] = 0x2A;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x43;
    cmd_buf[4] = 0x70;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0x36;
    cmd_buf[7] = 0x37;
    cmd_buf[8] = 0x6F;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x11;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x35;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0;//TE
    qspi_1501_send_cmd_data(0x2, cmd_buf, 4);

    hal_sys_timer_delay(MS_TO_TICKS(100));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x29;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

}

void enter_sleep_9c01(void)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x28;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

	hal_sys_timer_delay(MS_TO_TICKS(2000));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x10;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

}
void exit_sleep_9c01(void)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x11;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

	hal_sys_timer_delay(MS_TO_TICKS(120));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x29;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);

}

uint32_t read_9c01_id()
{
    uint8_t buf[3];

    buf[0] = 0;
    buf[1] = 0x04;
    buf[2] = 0;
    qspi_1501_recv_cmd_data(0x03, buf, 3, buf, 3);

    DUMP8("0x%02x ", buf, 3);
    return buf[2] | (buf[1]<<8);

}

void set_block_addr_9c01(uint32_t x_start, uint32_t x_end, uint32_t y_start, uint32_t y_end)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2a;
    cmd_buf[2] = 0;
    cmd_buf[3] = x_start>>8;
    cmd_buf[4] = x_start&0xff;
    cmd_buf[5] = x_end>>8;
    cmd_buf[6] = x_end&0xff;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2b;
    cmd_buf[2] = 0;
    cmd_buf[3] = y_start>>8;
    cmd_buf[3] = y_start&0xff;
    cmd_buf[4] = y_end>>8;
    cmd_buf[5] = y_end&0xff;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
    cmd_buf[2] = 0;
    qspi_1501_send_cmd_data(0x2, cmd_buf, 3);
}

#if 0
void qspi_1501_color(unsigned int color)
{
    uint8_t *buf = frame_buf;
    uint32_t i, j;
    uint32_t y;

    *buf++ = 0;
    *buf++ = 0x2c;
    *buf++ = 0;
    for (i=0; i<ROW; ++i) {
        for (j=0; j<COL; ++j) {
            *buf++ = color>>8;
            *buf++ = color&0xff;
        }
    }

    for (y = 0; y < ROW; y+=10) {
        set_block_addr_9c01(0,COL-1,y,y+9);
        qspi_1501_send_cmd_qspi_data(0x12, frame_buf, 3+10*COL*2);
    }

	//set_block_addr_9c01(0,COL-1,0,ROW-1);
    //qspi_1501_send_cmd_qspi_data(0x12, frame_buf, 3+ROW*COL*2);
}
#endif

static void display_at_location(uint32_t x, uint32_t x_end, uint32_t y, uint32_t y_end,
    uint8_t *buf, uint32_t len)
{
    set_block_addr_9c01(x, x_end, y, y_end);
    qspi_1501_send_cmd_qspi_data(0x12, buf, len);
}

static uint8_t *dma_buf;
static uint32_t dma_y;

#define DISPLAY_THREAD_STACK_SIZE 0x1000
static osThreadId display_thread_tid;

static void display_thread(void const *argument)
{
    osEvent evt;
    POSSIBLY_UNUSED uint32_t signals = 0;

    while(1)
    {
        //wait any signal
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;

        if(evt.status == osEventSignal)
        {
            dma_y += 10;
            dma_buf += 10*COL*2;
            if (dma_y < ROW)
                display_at_location(0, COL-1, dma_y, dma_y+9, dma_buf, 10*COL*2);
            //else
            //    TRACE(0, "end:%ums", FAST_TICKS_TO_MS(hal_fast_sys_timer_get()));
        }
        else
        {
            TRACE(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}
osThreadDef(display_thread, osPriorityAboveNormal, 1, DISPLAY_THREAD_STACK_SIZE, "display_msg");

void dma_irq_handler_9c01()
{
    TRACE(0, "dma_irq_handler_9c01");
    osSignalSet(display_thread_tid, 1);
}

static void qspi_1501_display_frame_buf(uint8_t *frame)
{
    dma_buf = frame;
    dma_y = 0;
    TRACE(0, "start:%ums", FAST_TICKS_TO_MS(hal_fast_sys_timer_get()));
    display_at_location(0, COL-1, dma_y, dma_y+9, dma_buf, 10*COL*2);
}

void qspi_1501_test(void)
{
    uint32_t id;
    uint32_t src_clk;
    uint32_t speed;
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {HAL_GPIO_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    TRACE(0, "qspi_1501_test");

    display_thread_tid = osThreadCreate(osThread(display_thread), NULL);
    osSignalSet(display_thread_tid, 0x0);
    TRACE_IMM(0, "%s, %d", __func__, __LINE__);

    qspi_1501_set_dma_handler(dma_irq_handler_9c01);

    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_3, HAL_GPIO_DIR_OUT, 1);
    hal_sys_timer_delay(MS_TO_TICKS(10));
    hal_gpio_pin_clr(HAL_GPIO_PIN_P0_3);
    hal_sys_timer_delay(MS_TO_TICKS(50));
    hal_gpio_pin_set(HAL_GPIO_PIN_P0_3);
    hal_sys_timer_delay(MS_TO_TICKS(120));

#ifdef QSPI_LOW_SPEED
    src_clk = 48*1000*1000;
    speed = 24*1000*1000;
#else
    src_clk = 196*1000*1000;
    speed = 98*1000*1000;
#endif
    qspi_1501_init(HAL_FLASH_ID_1, src_clk, speed);

    init_9c01();

    id = read_9c01_id();
    TRACE(0, "Screen id:0x%04x", id);

    uint8_t *buf;
    int i;
    int line = 0;
    while (1) {
        //uint32_t time = hal_fast_sys_timer_get();
        //TRACE(0, "time:%u us",  FAST_TICKS_TO_US(hal_fast_sys_timer_get()-time));
        buf = frame_buf;
        memset(frame_buf, 0, sizeof(frame_buf));
        for (i=COL*line; i<COL*ROW; ++i) {
            *buf++ = rgb565_raw_data_360x360[i]>>8;
            *buf++ = rgb565_raw_data_360x360[i]&0xff;
        }
        for (i=0; i<COL*line; ++i) {
            *buf++ = rgb565_raw_data_360x360[i]>>8;
            *buf++ = rgb565_raw_data_360x360[i]&0xff;
        }
        line++;
        if (line == ROW)
            line = 0;
        qspi_1501_display_frame_buf(frame_buf);
        osDelay(10); //memory copy time is longer than 40ms. so delay time is 10+40ms
    }

    while (1) {
    }
}

#endif
