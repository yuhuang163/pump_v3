/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef INTERSYS_COMM_TEST

#include "plat_addr_map.h"
#include "hwtest.h"
#include "hal_intersys.h"
#ifdef AON_PSC_BASE
#include "hal_psc.h"
#endif
#include "hal_trace.h"
#include "hal_uart.h"
#include "string.h"

#define btdrv_delay(ms) hal_sys_timer_delay(MS_TO_TICKS(ms))

typedef struct
{
    uint8_t patch_index;   //patch position
    uint8_t patch_state;   //is patch active
    uint16_t patch_length;     ///patch length 0:one instrution replace  other:jump to ram to run more instruction
    uint32_t patch_remap_address;   //patch occured address
    uint32_t patch_remap_value;      //patch replaced instuction
    uint32_t patch_start_address;    ///ram patch address for lenth>0
    uint8_t *patch_data;                  //ram patch date for length >0

}BTDRV_PATCH_STRUCT;

static volatile uint32_t btdrv_tx_flag = 1;

#define BTDRV_PATCH_ACT   0x1
#define BTDRV_PATCH_INACT   0x0

/// 8 bit access types
#define _8_Bit                              8
/// 16 bit access types
#define _16_Bit                             16
/// 32 bit access types
#define _32_Bit                             32

///enable m4 patch func
#define BTDRV_PATCH_EN_REG   0xe0002000

//set m4 patch remap adress
#define BTDRV_PATCH_REMAP_REG    0xe0002004

////instruction patch compare src address
#define BTDRV_PATCH_INS_COMP_ADDR_START   0xe0002008

#define BTDRV_PATCH_INS_REMAP_ADDR_START   0xc0000100

////data patch compare src address
#define BTDRV_PATCH_DATA_COMP_ADDR_START   0xe00020e8

#define BTDRV_PATCH_DATA_REMAP_ADDR_START   0xc00001e0

static enum HAL_INTERSYS_ID_T br_intersys = HAL_INTERSYS_ID_0;

static POSSIBLY_UNUSED volatile bool uart_rx_done = false;
static POSSIBLY_UNUSED volatile bool uart_tx_done = false;
static volatile bool bt_rx_done = false;
static volatile bool bt_tx_done = false;

unsigned char uart_rx_data[1024];
volatile unsigned int uart_rx_len = 0;

unsigned char bt_rx_data[1024];
volatile unsigned int bt_rx_len = 0;

uint8_t bt_rx_buff[4096];

static unsigned int bt_rx(const unsigned char *data, unsigned int len)
{
    memcpy(bt_rx_buff, data, len);

    for (int i = 0; i < len; ++i) {
        *((volatile uint32_t *)0xd0220064) = data[i];
    }
    bt_rx_done = true;

#if 0
    hal_intersys_stop_recv(br_intersys);
#endif
    return len;
}

static void bt_tx(const unsigned char *data, unsigned int len)
{
    bt_tx_done = true;
    btdrv_tx_flag = 1;
}

unsigned char rsp_1[] = {0x04,0x0e,0x04,0x05,0x02,0xfc,0x00};
unsigned char req_1[] = {0x01,0x02,0xfc,0x0a,0xe4,0x01,0x22,0xd0,0x20,0x04,0x00,0x55,0x1f,0x00};

// 01 02 fc 0e 00 20 00 e0 20 08 00 00 02 2f 00 01 00 20
void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length)
{
    uint8_t buff[256];
    if(length ==0 || length >128)
        return;
    buff[0] = 0x01;
    buff[1] = 0x02;
    buff[2] = 0xfc;
    buff[3] = length + 6;
    buff[4] = address & 0xff;
    buff[5] = (address &0xff00)>>8;
    buff[6] = (address &0xff0000)>>16;
    buff[7] = address>>24;
    buff[8] = wr_type;
    buff[9] = length;
    memcpy(&buff[10],value,length);

    btdrv_tx_flag = 0;
    for (int i = 0; i < length+10; ++i) {
        *((volatile uint32_t *)0xd0220050) = buff[i];
    }
    hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, length+10);
    while( btdrv_tx_flag == 0);

    btdrv_delay(2);
}

void btdrv_send_data(const uint8_t *buff, uint8_t length)
{
    btdrv_tx_flag = 0;
    for (int i = 0; i < length; ++i) {
        *((volatile uint32_t *)0xd0220050) = buff[i];
    }
    hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, length);
    while( btdrv_tx_flag == 0);

    btdrv_delay(2);
}

#if defined(CHIP_BEST1400)
/*
const uint32_t bes1400_patch0_ins_data[] =
{
    0x601a4b01,
    0xbbe2f600,
    0x4000007c
};

const BTDRV_PATCH_STRUCT bes1400_ins_patch0 =
{
    0,
    BTDRV_PATCH_ACT,
    sizeof(bes1400_patch0_ins_data),
    0x00006ec8,
    0xbc1af1ff,
    0xc0006700,
    (uint8_t *)bes1400_patch0_ins_data
};//clear intersys interrupt
*/

const uint32_t bes1400_patch0_ins_data[] =
{
    0x601a4b01,
    0xbbe2f600,
    0x4000007C
};

const BTDRV_PATCH_STRUCT bes1400_ins_patch0 =
{
    0,
    BTDRV_PATCH_ACT,
    sizeof(bes1400_patch0_ins_data),
    0x00006ec8,
    0xbc1af1ff,
    0xc0006700,
    (uint8_t *)bes1400_patch0_ins_data
};//clear intersys interrupt

const uint32_t bes1400_patch1_ins_data[] =
{
    0x601a4b01,
    0xbbf8f600,
    0x4000007C
};

const BTDRV_PATCH_STRUCT bes1400_ins_patch1 =
{
    1,
    BTDRV_PATCH_ACT,
    sizeof(bes1400_patch1_ins_data),
    0x00006f04,
    0xbc04f1ff,
    0xc0006710,
    (uint8_t *)bes1400_patch1_ins_data
};//clear intersys interrupt

static const uint32_t best1400_ins_patch_config[] =
{
    2,
    (uint32_t)&bes1400_ins_patch0,
    (uint32_t)&bes1400_ins_patch1,
};


void btdrv_patch_en(uint8_t en)
{
    uint32_t value[2];

    //set patch enable
    value[0] = 0x2f02 | en;
    //set patch remap address  to 0xc0000100
    value[1] = 0x20000100;
    btdrv_write_memory(_32_Bit,BTDRV_PATCH_EN_REG,(uint8_t *)&value,8);
}

void btdrv_ins_patch_write(BTDRV_PATCH_STRUCT *ins_patch_p)
{
    uint32_t remap_addr;
    uint8_t i=0;
    remap_addr =   ins_patch_p->patch_remap_address | 1;
    btdrv_write_memory(_32_Bit,(BTDRV_PATCH_INS_REMAP_ADDR_START + ins_patch_p->patch_index*4),
                                            (uint8_t *)&ins_patch_p->patch_remap_value,4);
    if(ins_patch_p->patch_length != 0)  //have ram patch data
    {
        for( ;i<(ins_patch_p->patch_length)/128;i++)
        {
                    btdrv_write_memory(_32_Bit,ins_patch_p->patch_start_address+i*128,
                                                            (ins_patch_p->patch_data+i*128),128);
        }

        btdrv_write_memory(_32_Bit,ins_patch_p->patch_start_address+i*128,ins_patch_p->patch_data+i*128,
                                                ins_patch_p->patch_length%128);
    }

    btdrv_write_memory(_32_Bit,(BTDRV_PATCH_INS_COMP_ADDR_START + ins_patch_p->patch_index*4),
                                            (uint8_t *)&remap_addr,4);

}

void btdrv_ins_patch_init_1400(void)
{
    btdrv_patch_en(0);
    const BTDRV_PATCH_STRUCT *ins_patch_p;
    {
        for(uint8_t i=0;i<best1400_ins_patch_config[0];i++)
        {
            ins_patch_p = (BTDRV_PATCH_STRUCT *)best1400_ins_patch_config[i+1];
            if(ins_patch_p->patch_state ==BTDRV_PATCH_ACT)
                btdrv_ins_patch_write((BTDRV_PATCH_STRUCT *)best1400_ins_patch_config[i+1]);
        }
    }
    btdrv_patch_en(1);
}
#endif

void patch_test_start(void)
{
#ifdef AON_PSC_BASE
    hal_psc_bt_enable();
#endif
    hal_cmu_bt_clock_enable();
    hal_cmu_bt_reset_clear();
    hal_cmu_bt_module_init();

    btdrv_delay(20);

#if defined(CHIP_BEST1400)
    btdrv_ins_patch_init_1400();
#endif

    btdrv_delay(4);
}

unsigned char cmd1[] = {0x01, 0x32, 0xfc, 0x06, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
unsigned char cmd2[] = {0x01, 0x0f, 0x08, 0x02, 0x05, 0x00};
unsigned char cmd3[] = {0x01, 0x1c, 0x0c, 0x04, 0x50, 0x00, 0x12, 0x00};
unsigned char cmd4[] = {0x01, 0x1a, 0x0c, 0x01, 0x02};

void intersys_comm_test(void)
{
    int ret;

    ret = hal_intersys_open(br_intersys, HAL_INTERSYS_MSG_HCI, bt_rx, bt_tx, false);
    if (ret) {
        TRACE(0,"Failed to open intersys");
        return;
    }
    hal_intersys_start_recv(br_intersys);

    patch_test_start();

    bt_rx_done = false;
    btdrv_send_data(cmd1, sizeof(cmd1));
    while (!bt_rx_done);
    btdrv_delay(5);

    bt_rx_done = false;
    btdrv_send_data(cmd2, sizeof(cmd2));
    while (!bt_rx_done);
    btdrv_delay(5);

    bt_rx_done = false;
    btdrv_send_data(cmd3, sizeof(cmd3));
    while (!bt_rx_done);
    btdrv_delay(5);

    bt_rx_done = false;
    btdrv_send_data(cmd4, sizeof(cmd4));
    while (!bt_rx_done);
    btdrv_delay(5);

    while (1);
}

#endif
