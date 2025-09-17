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
#ifdef QSPI_DISPLAY_TEST
#include "string.h"
#if !_BAREMETAL
#include "cmsis.h"
#include "cmsis_os.h"
#endif

#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "hal_dma.h"
#include "hal_norflaship.h"
#include "qspi_1501.h"
#include "gpu_port.h"

#define ROW  360
#define COL  360

#define RED    0xF800
#define GREEN  0x07E0
#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define GRAY   0xEF5D        //0x2410
#define GRAY75 0x39E7
#define GRAY50 0x7BEF
#define GRAY25 0xADB5

//#define SIMU
#define QSPI_LOW_SPEED
#define HAL_FLASH_ID  HAL_FLASH_ID_0

#ifdef SIMU
#define hal_sys_timer_delay(...)
#endif

#define qspi_display_send_cmd_data qspi_1501_send_cmd_data
#define qspi_display_recv_cmd_data qspi_1501_recv_cmd_data
#define qspi_display_send_cmd_qspi_data qspi_1501_send_cmd_qspi_data
#define qspi_display_set_dma_handler qspi_1501_set_dma_handler
#define qspi_display_init qspi_1501_init
#define qspi_display_send_qspi_data qspi_1501_send_data

uint8_t ALIGNED(4) cmd_buf[50];

static void init_9c01()
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;    // internal reg enable
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;    // internal reg enable
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x80;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;    //reg_en for 70\74
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x81;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x70;    //reg_en for 7C\7D\7E
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x82;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;    //reg_en for 90\93
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x83;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x03;    //reg_en for 98\99
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x84;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x20;    //reg_en for B5
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x85;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x42;    //reg_en for B9\BE
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x86;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xfc;    //reg_en for C2~7
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x87;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;    //reg_en for C8\CB
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x89;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x10;    //reg_en for EC
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8a;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x4f;    //reg_en for F0~3\F6
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8c;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x59;    //reg_en for 60\63\64\66
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8d;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x51;    //reg_en for 68\6C\6E
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xae;    //reg_en for A1~3\A5\A7
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x8f;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xf3;    //reg_en for AC~F\A8\A9
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);
#if 0
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x36;
    cmd_buf[2] = 0;
    // cmd_buf[3] = 0x08;
    cmd_buf[3] = 0x00;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);
#endif
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x3a;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x05;    // 565 frame
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xec;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x70;    //0x77;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //2COL

#if 0
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xb1;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x08;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //dtr enable
#endif

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x74;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x80;
    cmd_buf[5] = 0x00;
    cmd_buf[6] = 0x00;
    cmd_buf[7] = 0x00;
    cmd_buf[8] = 0x00;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);    //rtn 60Hz

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x98;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3E;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //bvdd 3x

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x99;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3E;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //bvee -2x

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x2A;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //VBP

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc4;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x18;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //VBN

    //*********************????*********************//
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa1;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x04;    //SRAM RD OPTION
    qspi_display_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x01;
    cmd_buf[4] = 0x04;    //SRAM RD OPTION
    qspi_display_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa9;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x1C;    //IREF=9.8uA
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;    //VDDMA=1.553V
    cmd_buf[4] = 0x09;    //VDDML=1.24V
    qspi_display_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xb9;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x8A;    //RTERM=101O
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa8;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x5E;    //VBG_BUF=1.003V, DVDD=1.543V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa7;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x40;    //BIAS=10.2uA
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xaf;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x73;    //VDDSOU=1.715V ,VDDGM=2.002V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xae;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x44;    //VREE=2.475V,VRDD=2.335V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xad;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;    //VRGL=1.635V ,VDDSF=2.018V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xa3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x5D;    //OSC=53.7MHz
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;    //VREG_VREF=2.805V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;    //VREG1A=5.99V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc6;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0E;    //VREG1B=1.505V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc7;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x13;    //VREG2A=-2.995V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xc8;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0D;    //VREG2B=1.497V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xcb;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;    //6.09V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);    //bvdd ref_ad

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7c;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xB6;
    cmd_buf[4] = 0x26;    //13.12V
    qspi_display_send_cmd_data(0x2, cmd_buf, 5);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xac;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x24;    //VGLO=-8.35V
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf6;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0xc0;
//    cmd_buf[3] = 0x80;//EPF=2
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    //*********************????*************************//
    //gip start

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xb5;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x09;    //VFP
    cmd_buf[4] = 0x09;    //VBP
    cmd_buf[5] = 0x14;    //HBP
    qspi_display_send_cmd_data(0x2, cmd_buf, 6);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x60;    //STV1&2
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x0B;
    cmd_buf[5] = 0x5B;
    cmd_buf[6] = 0x56;
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x63;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3A;
    cmd_buf[4] = 0xE0;    //DE
    cmd_buf[5] = 0x5B;    //MAX=0x61
    cmd_buf[6] = 0x56;    //MAX=0x61
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);    //STV3&4

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x64;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x0D;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0xDD;
    cmd_buf[7] = 0x5B;
    cmd_buf[8] = 0x56;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);    //CLK_group1

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x66;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;
    cmd_buf[4] = 0x11;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0xE1;
    cmd_buf[7] = 0x5B;
    cmd_buf[8] = 0x56;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);    //CLK_group1

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x68;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x3B;    //FLC12 FREQ
    cmd_buf[4] = 0x08;
    cmd_buf[5] = 0x08;
    cmd_buf[6] = 0x00;
    cmd_buf[7] = 0x08;
    cmd_buf[8] = 0x29;
    cmd_buf[9] = 0x5B;
    qspi_display_send_cmd_data(0x2, cmd_buf, 10);    //FLC&FLV 1~2

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x6e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x00;    //gout1_swap_fw[4:0]
    cmd_buf[4] = 0x00;    //gout2_swap_fw[4:0]
    cmd_buf[5] = 0x00;    //gout3_swap_fw[4:0]
    cmd_buf[6] = 0x07;    //gout4_swap_fw[4:0]
    cmd_buf[7] = 0x01;    //gout5_swap_fw[4:0]
    cmd_buf[8] = 0x13;    //gout6_swap_fw[4:0]
    cmd_buf[9] = 0x11;    //gout7_swap_fw[4:0]
    cmd_buf[10] = 0x0B;    //gout8_swap_fw[4:0]
    cmd_buf[11] = 0x09;    //gout9_swap_fw[4:0]
    cmd_buf[12] = 0x16;    //gout10_swap_fw[4:0]
    cmd_buf[13] = 0x15;    //gout11_swap_fw[4:0]
    cmd_buf[14] = 0x1D;    //gout12_swap_fw[4:0]
    cmd_buf[15] = 0x1E;    //gout13_swap_fw[4:0]
    cmd_buf[16] = 0x00;    //gout14_swap_fw[4:0]
    cmd_buf[17] = 0x00;    //gout15_swap_fw[4:0]
    cmd_buf[18] = 0x00;    //gout16_swap_fw[4:0]
    cmd_buf[19] = 0x00;    //gout17_swap_fw[4:0]
    cmd_buf[20] = 0x00;    //gout18_swap_fw[4:0]
    cmd_buf[21] = 0x00;    //gout19_swap_fw[4:0]
    cmd_buf[22] = 0x1E;    //gout20_swap_fw[4:0]
    cmd_buf[23] = 0x1D;    //gout21_swap_fw[4:0]
    cmd_buf[24] = 0x15;    //gout22_swap_fw[4:0]
    cmd_buf[25] = 0x16;    //gout23_swap_fw[4:0]
    cmd_buf[26] = 0x0A;    //gout24_swap_fw[4:0]
    cmd_buf[27] = 0x0C;    //gout25_swap_fw[4:0]
    cmd_buf[28] = 0x12;    //gout26_swap_fw[4:0]
    cmd_buf[29] = 0x14;    //gout27_swap_fw[4:0]
    cmd_buf[30] = 0x02;    //gout28_swap_fw[4:0]
    cmd_buf[31] = 0x08;    //gout29_swap_fw[4:0]
    cmd_buf[32] = 0x00;    //gout30_swap_fw[4:0]
    cmd_buf[33] = 0x00;    //gout31_swap_fw[4:0]
    cmd_buf[34] = 0x00;    //gout32_swap_fw[4:0]
    qspi_display_send_cmd_data(0x2, cmd_buf, 35);    //gout_Mapping

    //*********************?????**********************//

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xbe;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x11;    //SOU_BIAS_FIX=1
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

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
    qspi_display_send_cmd_data(0x2, cmd_buf, 10);    //precharge GATE

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7d;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x72;
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x7e;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x38;    // VGL_BT=1 5X  (BT=0:6X)  RT=0
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x70;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x02;    //avdd_clk
    cmd_buf[4] = 0x03;    //avee_clk
    cmd_buf[5] = 0x09;    //vdh_clk
    cmd_buf[6] = 0x05;    //vgh_clk
    cmd_buf[7] = 0x0C;    //vgl_clk
    cmd_buf[8] = 0x06;    //vcl_clk
    cmd_buf[9] = 0x09;    //vdh_clk_porch  0E
    cmd_buf[10] = 0x05;    //vgh_clk_porch  0E
    cmd_buf[11] = 0x0C;    //vgl_clk_porch  0E
    cmd_buf[12] = 0x06;    //vcl_clk_porch  0E
    qspi_display_send_cmd_data(0x2, cmd_buf, 13);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x90;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x06;
    cmd_buf[4] = 0x06;
    cmd_buf[5] = 0x05;    //bvdd_clk1_ad1
    cmd_buf[6] = 0x06;    //bvdd_clk1_ad1
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x93;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x45;    //bvdd_shut_ad1
    cmd_buf[4] = 0xFF;
    cmd_buf[5] = 0x00;
    qspi_display_send_cmd_data(0x2, cmd_buf, 6);

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
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf1;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x43;
    cmd_buf[4] = 0x70;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0x36;
    cmd_buf[7] = 0x37;
    cmd_buf[8] = 0x6F;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf2;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x45;
    cmd_buf[4] = 0x09;
    cmd_buf[5] = 0x08;
    cmd_buf[6] = 0x08;
    cmd_buf[7] = 0x26;
    cmd_buf[8] = 0x2A;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xf3;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x43;
    cmd_buf[4] = 0x70;
    cmd_buf[5] = 0x72;
    cmd_buf[6] = 0x36;
    cmd_buf[7] = 0x37;
    cmd_buf[8] = 0x6F;
    qspi_display_send_cmd_data(0x2, cmd_buf, 9);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x11;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x35;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x0;    //TE
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x36;
    cmd_buf[2] = 0;
    cmd_buf[3] = 0x00;;    //MADC
    qspi_display_send_cmd_data(0x2, cmd_buf, 4);

    hal_sys_timer_delay(MS_TO_TICKS(100));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x29;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

}

static void enter_sleep_9c01(void)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x28;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    hal_sys_timer_delay(MS_TO_TICKS(2000));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x10;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

}

static void exit_sleep_9c01(void)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0xfe;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0xef;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x11;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

    hal_sys_timer_delay(MS_TO_TICKS(120));

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x29;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);

}

static uint32_t read_9c01_id()
{
    uint8_t buf[3];

    buf[0] = 0;
    buf[1] = 0x04;
    buf[2] = 0;
    qspi_display_recv_cmd_data(0x03, buf, 3, buf, 3);

    DUMP8("0x%02x ", buf, 3);
    return buf[2] | (buf[1] << 8);

}

static void set_block_addr_row_9c01(uint32_t y_start, uint32_t y_end)
{

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2b;
    cmd_buf[2] = 0;
    cmd_buf[3] = y_start >> 8;
    cmd_buf[3] = y_start & 0xff;
    cmd_buf[4] = y_end >> 8;
    cmd_buf[5] = y_end & 0xff;
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);
}

static void set_block_addr_col_9c01(uint32_t x_start, uint32_t x_end)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2a;
    cmd_buf[2] = 0;
    cmd_buf[3] = x_start >> 8;
    cmd_buf[4] = x_start & 0xff;
    cmd_buf[5] = x_end >> 8;
    cmd_buf[6] = x_end & 0xff;
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);
}

static void set_block_addr_9c01(uint32_t x_start, uint32_t x_end, uint32_t y_start,
             uint32_t y_end)
{
    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2a;
    cmd_buf[2] = 0;
    cmd_buf[3] = x_start >> 8;
    cmd_buf[4] = x_start & 0xff;
    cmd_buf[5] = x_end >> 8;
    cmd_buf[6] = x_end & 0xff;
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2b;
    cmd_buf[2] = 0;
    cmd_buf[3] = y_start >> 8;
    cmd_buf[3] = y_start & 0xff;
    cmd_buf[4] = y_end >> 8;
    cmd_buf[5] = y_end & 0xff;
    qspi_display_send_cmd_data(0x2, cmd_buf, 7);

    cmd_buf[0] = 0;
    cmd_buf[1] = 0x2c;
//    cmd_buf[1] = 0x3c;
    cmd_buf[2] = 0;
    qspi_display_send_cmd_data(0x2, cmd_buf, 3);
}


#if !_BAREMETAL


#else

#define  RED_COLOR   RED
#define  BLUE_COLOR  BLUE
#define  GREEN_COLOR GREEN

#define lcm_reset_pin HAL_GPIO_PIN_P9_0
#define lcm_te_pin HAL_GPIO_PIN_P9_1

static volatile int done_flag = 0;
void dma_irq_handler_9c01()
{
    //osSignalSet(display_thread_tid, 1);
    norflaship_set_transcmp(HAL_FLASH_ID, 1);
    norflaship_set_transsize_control(HAL_FLASH_ID, 1);
    done_flag = 1;
    TRACE(3, "%s", __func__);
}

static void wait_send_done(void)
{
    while (1) {
        if (done_flag) {
            done_flag = 0;
            break;
        }
    }
}

#ifdef HW_SWAP_BYTEORDER_TEST
static struct HAL_DMA_CH_CFG_T dma_cfg;
static struct HAL_DMA_DESC_T desc_bank[2];
static volatile bool swap_done_flag;

static void hwswap_wait_complete(void)
{
    while (1) {
        if (swap_done_flag) {
            swap_done_flag = 0;
            break;
        }
    }
}

static void dma_irq_handler_hwswap(void)
{
    swap_done_flag = 1;
    TRACE(3, "%s", __func__);
}


static void hwswap_dma_init(uint8_t *dest)
{
    enum HAL_DMA_PERIPH_T periph;
    uint8_t ch;
    periph = HAL_GPDMA_MEM;
    ch = hal_gpdma_get_chan(periph, HAL_DMA_HIGH_PRIO);
    if (ch == HAL_DMA_CHAN_NONE) {
        return -1;
    }
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.handler = dma_irq_handler_hwswap;
    dma_cfg.dst = (uint32_t)dest;
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.dst_periph = periph;
    dma_cfg.dst_width = HAL_DMA_WIDTH_BYTE;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.src_periph = 0;
    dma_cfg.src_width = HAL_DMA_WIDTH_BYTE;
    dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg.try_burst = 0;
    dma_cfg.ch = ch;
}

#define SWAP_BYTE_ORDER

static uint32_t hw_swap_byte_order(uint8_t * buffer,
                     uint32_t len, uint8_t *dest)
{
    enum HAL_DMA_RET_T ret;
    uint8_t dummy_buf[3];
    uint32_t remain = 0;
    uint32_t dma_frame_sz = HAL_DMA_MAX_DESC_XFER_SIZE & ~0x03;
    struct HAL_DMA_DESC_T *desc = NULL;
    uint32_t desc_cnt = (len - 1) / dma_frame_sz + 1;
    TRACE(3, "%s %d desc_cnt:%d len:%d", __func__, __LINE__, desc_cnt, len);
    if (desc_cnt > 2) {
        ASSERT(false, "%s: alloc memeory(size:%d) fail", __func__,
               sizeof(struct HAL_DMA_DESC_T) * desc_cnt);
    }
    desc = desc_bank;
    if (len % dma_frame_sz) {
        remain = len % dma_frame_sz;
    }
    ASSERT(desc, "%s: alloc memeory(size:%d) fail", __func__,
           sizeof(struct HAL_DMA_DESC_T) * desc_cnt);

    hwswap_dma_init(dest);

    dma_cfg.src = (uint32_t)buffer;
#ifdef SWAP_BYTE_ORDER
    dma_cfg.src += 1;
#endif
    if (desc_cnt == 1) {
        dma_cfg.src_tsize = len;
        ret = hal_dma_init_desc(&desc[0], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }
#ifdef SWAP_BYTE_ORDER
        struct HAL_DMA_2D_CFG_T src_2d;
        src_2d.xmodify = -2;
        src_2d.xcount  = 2;
        src_2d.ymodify = 2;
        src_2d.ycount  = dma_cfg.src_tsize / 2;

        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, &src_2d, NULL);
#else
        ret = hal_dma_sg_start(&desc[0], &dma_cfg);
#endif
        remain = 0;
    } else {
        int i = 0;
        for (i = 0; i < desc_cnt - 1; i++) {
            dma_cfg.src_tsize = dma_frame_sz;
            ret =
                hal_dma_init_desc(&desc[i], &dma_cfg, &desc[i + 1],
                          0);
            if (ret != HAL_DMA_OK) {
                goto _err_exit;
            }
            dma_cfg.src += dma_frame_sz;
        }
        ret = hal_dma_init_desc(&desc[i], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }
#ifdef SWAP_BYTE_ORDER
        struct HAL_DMA_2D_CFG_T src_2d;
        src_2d.xmodify = 2;
        src_2d.xcount = dma_cfg.src_tsize / 2;
        src_2d.ymodify = -1;
        src_2d.ycount = 2;
        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, &src_2d, NULL);
#else
        ret = hal_dma_sg_start(&desc[0], &dma_cfg);
#endif
        hwswap_wait_complete();
    }
_err_exit:
    if (ret != HAL_DMA_OK) {
        TRACE(3, "%s %d", __func__, __LINE__);
        hal_dma_free_chan(dma_cfg.ch);
        return len;
    }
    TRACE(3, "%s %d", __func__, __LINE__);
    return remain;
}
#endif


#define QSPI_TRANS_NOSIZELIMIT

void qspi_display_frame_buf(screen_fb_buffer_t * fb)
{
    uint8_t *dma_buf = fb->memory;
    uint32_t dma_y = 0;
    uint32_t dma_y_end = 0;
    uint32_t frames = 0;
    uint32_t remain = 0;
    uint32_t dma_frame_sz = (HAL_DMA_MAX_DESC_XFER_SIZE & ~0x03);
    uint32_t len = fb->stride * fb->height;
    uint32_t rpf = 0;
    uint32_t time_e = 0;
    uint32_t time_s = 0;
    uint32_t offset_x = (360 - fb->width) / 2;
    uint32_t offset_y = 0;
    dma_y = offset_y;

    while (0 == hal_gpio_pin_get_val(lcm_te_pin)) {
        ;
    }

    dma_frame_sz = dma_frame_sz - dma_frame_sz % fb->stride;
    rpf = dma_frame_sz / fb->stride;
    frames = len / dma_frame_sz;
    remain = len % dma_frame_sz;
#if 0
    for (int i = 0; i < frames; i++) {
        dma_y_end = dma_y + rpf - 1;
        set_block_addr_9c01(offset_x, offset_x + fb->width - 1, dma_y,
                    dma_y_end);
        qspi_display_send_cmd_qspi_data(0x12,
                        dma_buf + i * dma_frame_sz,
                        dma_frame_sz);
#ifdef _BAREMETAL
        hal_sys_timer_delay(MS_TO_TICKS(8));
#else
        wait_send_done();
#endif
        dma_y += rpf;
    }
    if (remain) {
        //TRACE(0,"fn:%d ys:%d ye:%d",frames, dma_y, fb->height -1 );
        set_block_addr_9c01(offset_x, offset_x + fb->width - 1, dma_y,
                    fb->height - 1);
        qspi_display_send_cmd_qspi_data(0x12,
                        dma_buf + frames * dma_frame_sz,
                        remain);
#ifdef _BAREMETAL
        hal_sys_timer_delay(MS_TO_TICKS(8));
#else
        wait_send_done();
#endif

    }
#else
    set_block_addr_9c01(offset_x, offset_x + fb->width - 1, offset_y,
                offset_y + fb->height - 1);
    time_s = hal_sys_timer_get();

#ifdef QSPI_TRANS_NOSIZELIMIT
    remain =
        qspi_display_send_qspi_data(0x12, dma_buf, fb->stride * fb->height);
    wait_send_done();
    if (remain) {
        qspi_display_send_qspi_data(0x12,
                        dma_buf + (fb->stride * fb->height -
                               remain), remain);
        wait_send_done();
    }
#else
    for (int i = 0; i < frames; i++) {
        qspi_display_send_qspi_data(0x12, dma_buf + i * dma_frame_sz,
                        dma_frame_sz);
        wait_send_done();
    }

    while (remain) {
        remain =
            qspi_display_send_qspi_data(0x12,
                        dma_buf + frames * dma_frame_sz,
                        remain);
        wait_send_done();
    }
#endif
    time_e = hal_sys_timer_get();
    TRACE(0, "%s  tpf:%d ms", __func__, TICKS_TO_MS(time_e - time_s));
#endif

}

//static screen_fb_buffer_t fb;

void rgb565_fb_clear(screen_fb_buffer_t * fb, uint16_t clear_color)
{
    uint8_t *buf = fb->memory;
    uint8_t color_depth = 2;
    uint16_t color = clear_color;

    for (int i = 0; i < fb->width * fb->height; ++i) {
        //buf[0] = clear_color>>8;
        //buf[1] = clear_color&0xff;
        memcpy(buf, &color, color_depth);
        buf += color_depth;
    }
}

void argb888_fb_clear(screen_fb_buffer_t * fb, uint32_t clear_color)
{
    uint8_t *buf = fb->memory;
    uint8_t color_depth = fb->stride / fb->width;
    uint32_t color = clear_color;
    for (int i = 0; i < fb->width * fb->height; ++i) {
        memcpy(buf, &color, color_depth);
        buf += color_depth;
    }
}

static screen_fb_buffer_t *display_fb = NULL;

void display_tee_handler(enum HAL_GPIO_PIN_T pin)
{
    static int c = 0;
    if (c++ % 12 != 0) {
        return;
        // TRACE(0,"%s", __func__);
    }

    if (display_fb) {
        qspi_display_frame_buf(display_fb);
        display_fb = 0;
    }
}

void submit_display_task(screen_fb_buffer_t * fb)
{
    display_fb = fb;
}

static void setup_display_tee_handler(void)
{
    struct HAL_GPIO_IRQ_CFG_T cfg;
    cfg.irq_enable = 1;
    cfg.irq_debounce = 1;
    cfg.irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;
    cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    cfg.irq_handler = display_tee_handler;
    hal_gpio_setup_irq(lcm_te_pin, &cfg);
}


static void rgb565_fb_fill_grad(screen_fb_buffer_t * fb)
{
    uint8_t *buf = fb->memory;
    uint8_t color_depth = fb->stride / fb->width;
    uint32_t color = 0x001f;

    //draw blue 32 line

    for (int i = 0; i < fb->width * 32; ++i) {
        color = 0x001f - i / fb->width;
        memcpy(buf, &color, color_depth);
        buf += color_depth;
    }

    //draw green 64 lines
    for (int i = 0; i < fb->width * 64; ++i) {
        color = 0x07e0 - ((i / fb->width) << 5);
        memcpy(buf, &color, color_depth);
        buf += color_depth;
    }

    //draw red 32 lines
    for (int i = 0; i < fb->width * 32; ++i) {
        color = 0xf800 - ((i / fb->width) << 11);
        memcpy(buf, &color, color_depth);
        buf += color_depth;
    }

    // fill other with blue
    int r = fb->height - 128;
    color = 0x001f;
    if (r > 0) {
        for (int i = 0; i < fb->width * r; ++i) {
            //color = 0xf800 - ((i/fb->width) << 11);
            memcpy(buf, &color, color_depth);
            buf += color_depth;
        }
    }
}



void qspi_display_setup(void)
{
    uint32_t id;
    uint32_t src_clk;
    uint32_t speed;
    screen_fb_buffer_t fb;

#ifdef HW_SWAP_BYTEORDER_TEST
    uint32_t *src = (uint8_t *)0x20400000;
    uint32_t *dest =   (uint8_t *)0x20400008;

    src[0] = 0x87654321;
    src[1] = 0x0fedcba9;
    hw_swap_byte_order((uint8_t *)src,8, (uint8_t *)dest);

    TRACE(3, "src:0x%x dest:0x%x",*src,*dest );

    while(1);
#endif

    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {lcm_reset_pin, HAL_IOMUX_FUNC_AS_GPIO,
         HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {lcm_te_pin, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO,
         HAL_IOMUX_PIN_NOPULL},
    };
    TRACE(0, "%s", __func__);

    qspi_display_set_dma_handler(dma_irq_handler_9c01);

    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(lcm_reset_pin, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(lcm_te_pin, HAL_GPIO_DIR_IN, 0);
    hal_sys_timer_delay(MS_TO_TICKS(10));
    hal_gpio_pin_clr(lcm_reset_pin);
    hal_sys_timer_delay(MS_TO_TICKS(50));
    hal_gpio_pin_set(lcm_reset_pin);
    hal_sys_timer_delay(MS_TO_TICKS(120));

#ifdef QSPI_LOW_SPEED
    src_clk = 48*1000*1000;
    speed = 24*1000*1000;
#else
    src_clk = 196*1000*1000;
    speed = 98*1000*1000;
#endif

    qspi_display_init(HAL_FLASH_ID,src_clk , speed);

    init_9c01();

    id = read_9c01_id();
    TRACE(0, "Screen id:0x%04x", id);

    setup_display_tee_handler();

    fb.width = 360;
    fb.height = 360;
    fb.stride = 720;    //rgb565
    fb.memory = (uint8_t *) 0x20400000;    //frame_buf;

    {
        cmd_buf[0] = 0;
        cmd_buf[1] = 0x29;
        cmd_buf[2] = 0;
        qspi_display_send_cmd_data(0x2, cmd_buf, 3);
    }
    rgb565_fb_clear(&fb,0x001f);
    qspi_display_frame_buf(&fb);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    #ifdef QSPI_FILL_GRAD_TEST
    while (1) {
        for (int i = 0; i < 2; i++) {
            if (i%2){
            rgb565_fb_fill_grad(&fb);
            }else{
             rgb565_fb_clear(&fb,0x001f);
            }
            qspi_display_frame_buf(&fb);
            hal_sys_timer_delay(MS_TO_TICKS(5000));
        }
    }
    #endif
}


#endif //!_BAREMETAL
#endif
