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
#ifdef BT_CONN_SIMU

#include "plat_addr_map.h"

#ifdef BT_CMU_BASE

#include "hwtest.h"
#include "cmsis.h"
#include "hal_analogif.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_intersys.h"
#include "hal_psc.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#ifndef CHIP_SUBSYS_SENS
#include "pmu.h"
#endif
#include "stdbool.h"
#include "string.h"

//#define BTPCM_TEST

#ifdef BTPCM_TEST
#include "hal_btpcm.h"
#endif

#define BT_RX_WAKE_LOCK                 HAL_CPU_WAKE_LOCK_USER_10
#define BT_TX_WAKE_LOCK                 HAL_CPU_WAKE_LOCK_USER_11
#define BT_TIMER_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_12

enum BT_CONN_STATE_T {
    BT_CONN_WAIT_START,
    BT_CONN_SEND_HCI_RESET,
    BT_CONN_HCI_RESET_ACK,
    BT_CONN_SEND_LINK_POLICY,
    BT_CONN_LINK_POLICY_ACK,
    BT_CONN_SEND_CHANGE_BDADDR,
    BT_CONN_CHANGE_BDADDR_ACK,
    BT_CONN_SEND_SET_SCAN_INTERVAL,
    BT_CONN_SET_SCAN_INTERVAL_ACK,
    BT_CONN_SEND_SCAN,
    BT_CONN_SCAN_ACK,
    BT_CONN_LINK_STATUS,
    BT_CONN_SEND_ACCEPT,
    BT_CONN_ACCEPT_ACK,
    BT_CONN_CONN_STATUS,
    BT_CONN_TEST_EVT,
    BT_CONN_SNIFFER,
    BT_CONN_WAIT_SEND_DATA,
    BT_CONN_SEND_DATA,
    BT_CONN_DATA_ACK,
    BT_CONN_SCO_CONN_REQ,
    BT_CONN_SEND_SCO_CONN_ACCEPT,
    BT_CONN_SCO_CONN_ACCEPT_ACK,
    BT_CONN_CREAT_CONNECT_REQ,
    BT_CONN_CREAT_CONNECT_ACK,
    BT_CONN_CREAT_SCO_CONNECT_REQ,
    BT_CONN_CREAT_SCO_CONNECT_ACK,    
    BT_CONN_WAIT_SCO_CONCMP,
    BT_CONN_SEND_SNIFF_REQ,
    BT_CONN_SEND_SNIFF_ACK,
    BT_CONN_WAIT_MODECHANGE,
    BLE_SET_ADV_DATA_REQ,
    BLE_SET_ADV_DATA_ACK,
    BLE_SET_ADV_PARA_REQ,
    BLE_SET_ADV_PARA_ACK,
    BLE_SET_ADV_EN_REQ,
    BLE_SET_ADV_EN_ACK,
    BLE_CREATE_CONNECT_REQ,
    BLE_CREATE_CONNECT_ACK,
    BLE_WAIT_CONN_CMP,
    BLE_WAIT_DATA_SEND_TIMER,
    BLE_SEND_DATA_REQ,
    BLE_SEND_DATA_ACK,
    BT_CONN_DONE,
};



///test case for simu 
//#define BLE_CONN_TEST
#define BT_CONN_TEST
//#define BT_CONN_SENDDATA_TEST
//#define BT_CONN_ESCO_TEST
//#define BT_CONN_SNIFF_TEST

////test case simu role 
////define means master simu code   no defined means slave simu code
#define SIMU_MASTER_ROLE


///bt conn test master and slave 
#ifdef BT_CONN_TEST
#ifdef SIMU_MASTER_ROLE
#define BT_SIMU_MASTER
#endif

#elif defined(BT_CONN_SENDDATA_TEST)
#ifdef SIMU_MASTER_ROLE
#define BT_SIMU_MASTER
#define ACL_SENDDATA_TEST
#endif

#elif defined(BT_CONN_ESCO_TEST)

#ifdef SIMU_MASTER_ROLE
#define BT_SIMU_MASTER
#define SCO_CONNECT_MASTER
#endif

#elif defined(BT_CONN_SNIFF_TEST)

#ifdef SIMU_MASTER_ROLE
#define BT_SIMU_MASTER
#define BT_SNIFF_MODE_TEST
#endif

#elif defined(BLE_CONN_TEST)

#ifdef SIMU_MASTER_ROLE
#define BLE_CONN_MASTER
#else
#define BLE_CONN_SLAVE
#endif

#else
#error "please define one test case for simu"
#endif






static const enum HAL_INTERSYS_ID_T conn_intersys = HAL_INTERSYS_ID_0;

static enum BT_CONN_STATE_T bt_state = BT_CONN_WAIT_START;
unsigned char bt_conn_handle;
unsigned char ble_conn_handle;

static HWTIMER_ID timer_id;

static volatile bool bt_rx_done = false;
static volatile bool bt_tx_done = false;
static volatile bool timer_evt = false;

static unsigned char bt_rx_data[128];
static unsigned int bt_rx_len = 0;

static unsigned char cmd_data[0x500];
static unsigned int cmd_data_len;


static  unsigned char cmd_hci_reset[] = {
    0x01, 0x03, 0x0c, 0x00,
};

static  unsigned char ack_hci_reset[] = {
    0x04, 0x0e, 0x04, 0x05, 0x03, 0x0c, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char cmd_link_policy[] = {
    0x01, 0x0f, 0x08, 0x02, 0x05, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char ack_link_policy[] = {
    0x04, 0x0e, 0x04, 0x05, 0x0f, 0x08, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char cmd_change_bdaddr[] = {
    0x01, 0x72, 0xfc, 0x07,0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
};
POSSIBLY_UNUSED
static  unsigned char ack_change_bdaddr[] = {
    0x04, 0x0E, 0x04, 0x05, 0x72, 0xFC, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char cmd_set_scan_interval[] = {
    0x01, 0x1c, 0x0c, 0x04, 0x50, 0x00, 0x12, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char ack_set_scan_interval[] = {
    0x04, 0x0e, 0x04, 0x05, 0x1c, 0x0c, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char cmd_scan[] = {
    0x01, 0x1a, 0x0c, 0x01, 0x03,
};
POSSIBLY_UNUSED
static  unsigned char ack_scan[] = {
    0x04, 0x0e, 0x04, 0x05, 0x1a, 0x0c, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char evt_link_status[] = {
    0x04, 0x04, 0x0A, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x00, 0x00, 0x00, 0x01,
};
POSSIBLY_UNUSED
static unsigned char cmd_accept[] = {
    0x01, 0x09, 0x04, 0x07, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x01,
};
POSSIBLY_UNUSED
static  unsigned char ack_accept[] = {
    0x04, 0x0f, 0x04, 0x00, 0x05, 0x09, 0x04,
};
POSSIBLY_UNUSED
static  unsigned char evt_conn_status[] = {
    0x04, 0x03, 0x0B, 0x00, 0x80, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x01, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char evt_conn_attributs[] = {
    0x04, 0x1b, 0x03, 0x80, 0x00, 0x05,
};
POSSIBLY_UNUSED
static  unsigned char evt_sniffer[] = {
    0x04, 0x14, 0x06, 0x00, 0x80, 0x00, 0x02, 0x20, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char ack_data[] = {
    0x04, 0x13, 0x05, 0x01, 0x80, 0x00, 0x01, 0x00,
};
POSSIBLY_UNUSED
static  unsigned char evt_sco_conn_req[] = {
    0x04, 0x04, 0x0A, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x00, 0x00, 0x00, 0x02,
};
POSSIBLY_UNUSED
static  unsigned char cmd_sco_conn_accept[] = {
    0x01, 0x29, 0x04, 0x15, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x40, 0x1f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x00, 0x1e, 0x00, 0x40, 0x00, 0x01, 0x80, 0x03,
};

POSSIBLY_UNUSED
static  unsigned char cmd_creat_connect[] = 
{   
    0x01, 0x05, 0x04, 0x0d, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 
     0x18, 0xcc, 0x02, 0x00, 0x00, 0x00, 0x01, 
};
POSSIBLY_UNUSED
static  unsigned char cmd_creat_connect_ack[] = 
{   
    0x04, 0x0f, 0x04, 0x00, 0x05, 0x05, 0x04, 

};
POSSIBLY_UNUSED
static  unsigned char cmd_sco_conn_creat[] = {
    0x01, 0x28, 0x04, 0x11, 0x80, 0x00, 0x40, 0x1f, 0x00,
    0x00, 0x40, 0x1f, 0x00, 0x00, 0x1e, 0x00, 0x40, 0x00, 0x01, 0x80, 0x03,
};
POSSIBLY_UNUSED
static  unsigned char cmd_sco_connect_ack[] = 
{   
    0x04, 0x0f, 0x04, 0x00, 0x05, 0x28, 0x04, 
};
POSSIBLY_UNUSED
static  unsigned char cmd_sco_connect_cmp_evt[] = 
{   
    0x04, 0x2c, 0x11,
};
POSSIBLY_UNUSED
static  unsigned char cmd_acl_sniff_enter[] = 
{   
    0x01, 0x03, 0x08, 0x0a, 0x80, 0x00, 0x40,0x00,0x40,0x00,0x04,0x00,0x01,0x00 
};
POSSIBLY_UNUSED
static  unsigned char cmd_acl_sniff_ack[] = 
{   
    0x04, 0x0f, 0x04, 0x00, 0x05, 0x03, 0x08, 
};

POSSIBLY_UNUSED
static  unsigned char cmd_change_bdaddr_ble[] = {
    0x01, 0x72, 0xfc, 0x07,0x01, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
};

POSSIBLY_UNUSED
static  unsigned char ack_change_bdaddr_ble[] = {
    0x04, 0x0E, 0x04, 0x05, 0x72, 0xFC, 0x00,
};

POSSIBLY_UNUSED
static  unsigned char set_adv_data[] = {
    0x01, 0x08, 0x20, 0x20,0x1f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15,0x16, 0x17, 0x18,0x19, 0x1a, 0x1b,0x1c, 0x1d, 0x1e,
    0x1f,
};

POSSIBLY_UNUSED
static  unsigned char ack_set_adv_data[] = {
    0x04, 0x0E, 0x04, 0x05, 0x08, 0x20, 0x00,
};

POSSIBLY_UNUSED
static  unsigned char set_adv_para[] = {
    0x01, 0x06, 0x20, 0x0f,
    0x20, 0x00,   //interval min
    0x20, 0x00,  //interval max
    0x00,          //adv type 0: adv_ind 
    0x00,         //own address type  0:pub add
    0x00,        //peer address type
    0x00,0x00,0x00,0x00,0x00,0x00,  //peer address
    0x07,       //chan map
    0x00,      //filter policy
};

POSSIBLY_UNUSED
static  unsigned char ack_set_adv_para[] = {
    0x04, 0x0E, 0x04, 0x05, 0x06, 0x20, 0x00,
};


POSSIBLY_UNUSED
static  unsigned char set_adv_en[] = {
    0x01, 0x0a, 0x20, 0x01,0x01,
};

POSSIBLY_UNUSED
static  unsigned char ack_set_adv_en[] = {
    0x04, 0x0E, 0x04, 0x05, 0x0a, 0x20, 0x00,
};



POSSIBLY_UNUSED
static  unsigned char ble_creat_connect[] = {
    0x01, 0x0d, 0x20, 0x19,
    0x50,0x00,   //scan interval
    0x30,0x00,  //scan window
    0x00,         //init filter policy
    0x00,         //peer address type
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab,  //peer address
    0x00,          //own address type
    0x20,0x00,   //connect interval min
    0x20,0x00,   //connect interval max
    0x00,0x00,         //connect latency
    0x64,0x00,  //connect timeout
    0x10,0x00,   //ce length min
    0x10,0x00,  //ce length max
    
};

POSSIBLY_UNUSED
static  unsigned char ack_ble_creat_connect[] = {
    0x04, 0x0f, 0x04,0x00, 0x05, 0x0d, 0x20
};

POSSIBLY_UNUSED
const unsigned char ble_connect_complete_evt[] = {
    0x04, 0x3E, 0x13, 
    0x01,   //subcode
    0x00,  //status
    0x00, 0x00,  //connect handle
    0x00,      //role
    0x00,    //peer address type
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab,  //peer address
    0x20,0x00,   ///connect interval
    0x00,0x00,  //connect latency
    0x64,0x00,  //supervision timeout
    0x00,        //master clk acc
    
};

///data len:
//br:dm1 11 dh1 27 dm3 121 dh3 183  dm5 224   dh5 339
///edr:2dh1 54  2dh3 367 2dh5 679  3dh1 83 3dh3 552 3dh5 1021
typedef enum
{
    BT_ACL_DM1=0,
    BT_ACL_DM3,
    BT_ACL_DM5,
    BT_ACL_DH1,
    BT_ACL_DH3,
    BT_ACL_DH5,
    BT_ACL_2DH1,
    BT_ACL_2DH3,
    BT_ACL_2DH5,
    BT_ACL_3DH1,
    BT_ACL_3DH3,
    BT_ACL_3DH5,
    BT_MAX_TYPE_ENUM
}BT_ACL_PKT;
unsigned short bt_pkt_max_len[12] = {11,121,224,27,183,339,54,367,679,83,552,1021};
static  unsigned char bt_data[1028];
static  unsigned int bt_data_len;



void bt_pack_data_by_packet(uint8_t conhdl,BT_ACL_PKT type)
{
    if(type>=BT_MAX_TYPE_ENUM)return;
    bt_data_len = bt_pkt_max_len[type];
    bt_data[0] = 0x2;
    bt_data[1] = conhdl;
    bt_data[2] = 0x20;
    bt_data[3] = bt_data_len&0xff;
    bt_data[4] = bt_data_len>>8;
    bt_data_len +=5;
        
}

void ble_pack_data_by_length(uint8_t conhdl,uint8_t length)
{
    if(length>251)return;
    bt_data_len = length;
    bt_data[0] = 0x2;
    bt_data[1] = conhdl;
    bt_data[2] = 0x00;
    bt_data[3] = bt_data_len&0xff;
    bt_data[4] = bt_data_len>>8;
    bt_data_len +=5;
        
}

void bt_pkt_init(void)
{
    bt_data[0] = 0x02;
    bt_data[1] = 0x80;
    bt_data[2] = 0x20;
    bt_data[3] = 0xfd;
    bt_data[4] = 0x03;
    for(unsigned short i=0;i<0x3fd;i++)
    {
        bt_data[5+i] = i;
    }
}

unsigned int byte2int(const unsigned char *data)
{
    return ((*data<<24)|(*(data+1)<<16)|(*(data+2)<<8)|*(data+3));
}

void bt_hci_data_debug(const unsigned char *data,unsigned int len)
{
    hal_cmu_simu_set_val(0x77777777);
    hal_cmu_simu_set_val(len);
    uint8_t wr_len = len>10?10:len;
    for(unsigned char i=0;i<(wr_len);i++)
    {
        hal_cmu_simu_set_val(*(data+i));
    }
        
}

int bt_hci_send(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_MSG_TYPE_T type,
                      const unsigned char *data, unsigned int len)
{
    bt_hci_data_debug(data,len);
   return hal_intersys_send(id,type,data,len);
}

static void timer_expired(void *param)
{
    timer_evt = true;
    hal_cpu_wake_lock(BT_TIMER_WAKE_LOCK);
}

static unsigned int bt_rx(const unsigned char *data, unsigned int len)
{
    unsigned int processed = len;
    bt_hci_data_debug(data,len);
    hal_intersys_stop_recv(conn_intersys);
    if (bt_rx_len + processed > sizeof(bt_rx_data)) {
        TRACE(3,"bt_rx data too long: bt_rx_len=%d, len=%d, limit=%d", bt_rx_len, processed, sizeof(bt_rx_data));
        processed = sizeof(bt_rx_data) - bt_rx_len;
    }
    if (processed > 0) {
        memcpy(bt_rx_data + bt_rx_len, data, processed);
        bt_rx_len += processed;
        bt_rx_done = true;
        hal_cpu_wake_lock(BT_RX_WAKE_LOCK);
    }
    return len;
}

static void bt_tx(const unsigned char *data, unsigned int len)
{
    bt_tx_done = true;
    hal_cpu_wake_lock(BT_TX_WAKE_LOCK);
}

static void handle_timer_evt(void)
{
    int ret;

    TRACE(1,"Recv timer evt: %d", bt_state);

    switch (bt_state) {
        case BT_CONN_WAIT_START:
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_hci_reset, sizeof(cmd_hci_reset));
            ASSERT(ret == 0, "Failed to send hci reset cmd: %d", ret);
            bt_state = BT_CONN_SEND_HCI_RESET;
            break;
        case BT_CONN_WAIT_SEND_DATA:
#if 0            
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_data, cmd_data_len);
            ASSERT(ret == 0, "Failed to send data cmd: %d", ret);
            bt_state = BT_CONN_SEND_DATA;
#endif
            bt_pack_data_by_packet(bt_conn_handle,BT_ACL_2DH5);
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, bt_data, bt_data_len);
            ASSERT(ret == 0, "Failed to send acl data1: %d", ret);
            bt_state = BT_CONN_SEND_DATA;
            break;
///ble send data:
        case BLE_WAIT_DATA_SEND_TIMER:
            ble_pack_data_by_length(ble_conn_handle,251);
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, bt_data, bt_data_len);
            ASSERT(ret == 0, "Failed to send acl data1: %d", ret);
            bt_state = BLE_SEND_DATA_REQ;
            break;
            
        default:
            ASSERT(false, "Invalid state %d in %s", bt_state, __FUNCTION__);
            break;
    }
}

#ifdef BTPCM_TEST

#define AUDIO_BUFFER_SIZE 2048

struct HAL_BTPCM_CONFIG_T btpcm_cfg;
struct HAL_DMA_CH_CFG_T dma_cfg;
struct HAL_DMA_DESC_T play_desc;
struct HAL_DMA_DESC_T record_desc;
uint16_t play_buffer[AUDIO_BUFFER_SIZE / 2] = 
{
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
0xb4c4,0xa57e,0x9873,0x8df4,0x8645,0x8194,0x8001,0x8194,
0x8645,0x8df4,0x9873,0xa57e,0xb4c4,0xc5e4,0xd872,0xebfa,
0x0002,0x1406,0x278e,0x3a1c,0x4b3c,0x5a82,0x678d,0x720c,
0x79bb,0x7e6c,0x7fff,0x7e6c,0x79bb,0x720c,0x678d,0x5a82,
0x4b3c,0x3a1c,0x278e,0x1406,0x0002,0xebfa,0xd872,0xc5e4,
};
uint32_t record_buffer[AUDIO_BUFFER_SIZE / 4];

static void start_btpcm(void)
{
    //start controller  pcm en
    *(volatile uint32_t *)0xd0220680 |= 0x01; //pcm enable

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.src_periph = (enum HAL_DMA_PERIPH_T)0;
    dma_cfg.handler = NULL;
    dma_cfg.dst_periph = HAL_AUDMA_BTPCM_TX;
    dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.dst_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    dma_cfg.src_tsize = AUDIO_BUFFER_SIZE/2; /* cause of half word width */
    dma_cfg.try_burst = 1;

    dma_cfg.src = (uint32_t)play_buffer;
    hal_audma_init_desc(&play_desc, &dma_cfg, &play_desc, 0);
    hal_audma_sg_start(&play_desc, &dma_cfg);

    dma_cfg.dst_periph = (enum HAL_DMA_PERIPH_T)0;
    dma_cfg.handler = NULL;
    dma_cfg.src_periph = HAL_AUDMA_BTPCM_RX;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst = (uint32_t)record_buffer;
    hal_audma_init_desc(&record_desc, &dma_cfg, &record_desc, 0);
    hal_audma_sg_start(&record_desc, &dma_cfg);

    hal_btpcm_open(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK);
    hal_btpcm_open(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE);
    memset(&btpcm_cfg, 0, sizeof(btpcm_cfg));
    btpcm_cfg.use_dma = 1;
    hal_btpcm_setup_stream(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK, &btpcm_cfg);
    hal_btpcm_setup_stream(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE, &btpcm_cfg);
    hal_btpcm_start_stream(HAL_BTPCM_ID_0, AUD_STREAM_PLAYBACK);
    hal_btpcm_start_stream(HAL_BTPCM_ID_0, AUD_STREAM_CAPTURE);
}

#endif // BTPCM_TEST

#ifdef DPD_BT_TEST

static unsigned char dpd_bt_buf[4096] __attribute__((aligned(0x100)));
static struct HAL_DMA_DESC_T dpd_bt_desc;

void dpd_bt_handler(uint8_t chan, uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
#ifdef CHIP_BEST1000
    //REG(DPD_RX_BASE + 0x30) = 0;
    //REG(DPD_RX_BASE) = 0;
#endif
}

void dpd_bt_test(void)
{
    struct HAL_DMA_CH_CFG_T dma_cfg;

    // FM simulation
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    //dma_cfg.dst_periph = HAL_AUDMA_PERIPH_QTY; // useless
    dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.handler = dpd_bt_handler;
    dma_cfg.src = 0; // useless
    dma_cfg.src_bsize = HAL_DMA_BSIZE_4;
    dma_cfg.src_periph = HAL_AUDMA_DPD_RX;
    dma_cfg.src_tsize = sizeof(dpd_bt_buf) / 4;
    dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.try_burst = 1;
    dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    dma_cfg.ch = hal_audma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);

    dma_cfg.dst = (uint32_t)&dpd_bt_buf[0];
    hal_audma_init_desc(&dpd_bt_desc, &dma_cfg, &dpd_bt_desc, 1);
    hal_audma_sg_start(&dpd_bt_desc, &dma_cfg);

    REG(0xD03300f0) |= (1 << 0);
    REG(0xD0350208) |= (1 << 16);
    REG(0xD0350228) |= (1 << 16);

#ifdef CHIP_BEST1000
    REG(DPD_RX_BASE + 0x30) = 1;
    REG(DPD_RX_BASE) = 3;
#endif
}

#endif // DPD_BT_TEST

POSSIBLY_UNUSED
static void handle_rx_done(const unsigned char *data, unsigned int len)
{
    int ret;

    TRACE(1,"Recv rx done evt: %d", bt_state);
    hal_cmu_simu_set_val(bt_state);
    //ignore the dbg trace
    if(*data == 0x04 && *(data+1)==0xff)
    {
        return;
    }
    //ignore the max slot ind
    if(*data == 0x04 && *(data+1)==0x1b)
    {
        return;
    }
    //ignore
    if(*data == 0x04 && *(data+1)==0x07)
    {
        return;
    }
    //ignore acl data rx
    if(*data == 0x02)
        return;
    switch (bt_state) {
        case BT_CONN_HCI_RESET_ACK:
            ASSERT(len == sizeof(ack_hci_reset), "Invalid hci reset ack len: %d", len);
            ret = memcmp(ack_hci_reset, data, len);
            ASSERT(ret == 0, "Recv invalid hci reset ack");
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_link_policy, sizeof(cmd_link_policy));
            ASSERT(ret == 0, "Failed to send link policy cmd: %d", ret);
            bt_state = BT_CONN_SEND_LINK_POLICY;
            break;
        case BT_CONN_LINK_POLICY_ACK:
            ASSERT(len == sizeof(ack_link_policy), "Invalid link policy ack len: %d", len);
            ret = memcmp(ack_link_policy, data, len);
            ASSERT(ret == 0, "Recv invalid link policy ack");
            if (1) {
                ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_change_bdaddr, sizeof(cmd_change_bdaddr));
                ASSERT(ret == 0, "Failed to send change bdaddr cmd: %d", ret);
                bt_state = BT_CONN_SEND_CHANGE_BDADDR;
            } else {
                ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_set_scan_interval, sizeof(cmd_set_scan_interval));
                ASSERT(ret == 0, "Failed to send set scan interval cmd: %d", ret);
                bt_state = BT_CONN_SEND_SET_SCAN_INTERVAL;
            }
            break;
        case BT_CONN_CHANGE_BDADDR_ACK:
            ASSERT(len == sizeof(ack_change_bdaddr), "Invalid change bdaddr ack len: %d", len);
            ret = memcmp(ack_change_bdaddr, data, len);
            ASSERT(ret == 0, "Recv invalid change bdaddr ack");
#ifdef BT_SIMU_MASTER            
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_creat_connect, sizeof(cmd_creat_connect));
            ASSERT(ret == 0, "Failed to send scan cmd: %d", ret);
            bt_state = BT_CONN_CREAT_CONNECT_REQ;
#else
         //   ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_scan, sizeof(cmd_scan));
         //   ASSERT(ret == 0, "Failed to send scan cmd: %d", ret);
        //    bt_state = BT_CONN_SEND_SCAN;
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_set_scan_interval, sizeof(cmd_set_scan_interval));
            ASSERT(ret == 0, "Failed to send set scan interval cmd: %d", ret);
            bt_state = BT_CONN_SEND_SET_SCAN_INTERVAL;
#endif            
            break;
        case BT_CONN_SET_SCAN_INTERVAL_ACK:
            ASSERT(len == sizeof(ack_set_scan_interval), "Invalid set scan interval ack len: %d", len);
            ret = memcmp(ack_set_scan_interval, data, len);
            ASSERT(ret == 0, "Recv invalid set scan interval ack");
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_scan, sizeof(cmd_scan));
            ASSERT(ret == 0, "Failed to send scan cmd: %d", ret);
            bt_state = BT_CONN_SEND_SCAN;
            break;
        case BT_CONN_SCAN_ACK:
            ASSERT(len == sizeof(ack_scan), "Invalid scan ack len: %d", len);
            ret = memcmp(ack_scan, data, len);
            ASSERT(ret == 0, "Recv invalid scan ack");
#ifdef DPD_BT_TEST
            bt_state = BT_CONN_DONE;
            hal_sys_timer_delay(MS_TO_TICKS(50));
            dpd_bt_test();
#else
            bt_state = BT_CONN_LINK_STATUS;
#endif
            break;
        case BT_CONN_CREAT_CONNECT_ACK:
            ASSERT(len == sizeof(cmd_creat_connect_ack), "Invalid create connect status evt len: %d", len);
            ret = memcmp(cmd_creat_connect_ack, data, len);
            ASSERT(ret == 0, "Recv invalid create connect status evt");
         //   ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
       //     ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_CONN_STATUS;  
            break;
        case BT_CONN_LINK_STATUS:
            ASSERT(len == sizeof(evt_link_status), "Invalid connect req evt len: %d", len);
            ret = memcmp(evt_link_status, data, len);
            ASSERT(ret == 0, "Recv invalid link req evt");
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
            ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_SEND_ACCEPT;
            break;
        case BT_CONN_ACCEPT_ACK:
            ASSERT(len == sizeof(ack_accept), "Invalid accept ack len: %d", len);
            ret = memcmp(ack_accept, data, len);
            ASSERT(ret == 0, "Recv invalid accept ack");
            bt_state = BT_CONN_CONN_STATUS;
            break;
        case BT_CONN_CONN_STATUS:
            ASSERT(len == sizeof(evt_conn_status), "Invalid conn status evt len: %d", len);
            ret = memcmp(evt_conn_status, data, len);
            bt_conn_handle = data[4];
            ASSERT(ret == 0, "Recv invalid conn status evt");
#ifdef SCO_CONNECT_MASTER
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_sco_conn_creat, sizeof(cmd_sco_conn_creat));
            ASSERT(ret == 0, "Failed to send sco start cmd: %d", ret);
            bt_state = BT_CONN_CREAT_SCO_CONNECT_REQ;
#elif defined(ACL_SENDDATA_TEST)
            bt_pack_data_by_packet(bt_conn_handle,BT_ACL_2DH5);
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, bt_data, bt_data_len);
            ASSERT(ret == 0, "Failed to send sco start cmd: %d", ret);
            bt_state = BT_CONN_SEND_DATA;
#elif defined(BT_SNIFF_MODE_TEST)            
            cmd_acl_sniff_enter[4]= bt_conn_handle;
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_acl_sniff_enter, sizeof(cmd_acl_sniff_enter));
            ASSERT(ret == 0, "Failed to send sco start cmd: %d", ret);
            bt_state = BT_CONN_SEND_SNIFF_REQ;            
#else
            bt_state = BT_CONN_TEST_EVT;
#endif            

            break;
        case BT_CONN_SEND_SNIFF_ACK:
            ASSERT(len == sizeof(cmd_acl_sniff_ack), "Invalid sniff status evt len: %d", len);
            ret = memcmp(cmd_acl_sniff_ack, data, len);
            ASSERT(ret == 0, "Recv invalid sniff status evt");
         //   ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
       //     ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_WAIT_MODECHANGE;  
            break;
        case  BT_CONN_WAIT_MODECHANGE:
            if(len == sizeof(evt_sniffer) && memcmp(evt_sniffer, data, 4) == 0) {
                // Current state is BT_CONN_SNIFFER
                hwtimer_start(timer_id, MS_TO_TICKS(40));
                bt_state = BT_CONN_WAIT_SEND_DATA;
                hal_cmu_simu_set_val(0x22222222);
            }
            break;

        case BT_CONN_CREAT_SCO_CONNECT_ACK:
            ASSERT(len == sizeof(cmd_sco_connect_ack), "Invalid sco connect status evt len: %d", len);
            ret = memcmp(cmd_sco_connect_ack, data, len);
            ASSERT(ret == 0, "Recv invalid sco connect status evt");
         //   ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
       //     ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_WAIT_SCO_CONCMP;  
            break;
        case BT_CONN_WAIT_SCO_CONCMP:
            ret = memcmp(cmd_sco_connect_cmp_evt, data, sizeof(cmd_sco_connect_cmp_evt));
            ASSERT(ret == 0, "Recv invalid sco link status evt");
         //   ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_accept, sizeof(cmd_accept));
       //     ASSERT(ret == 0, "Failed to send accept cmd: %d", ret);
            bt_state = BT_CONN_DONE;  
#ifdef BTPCM_TEST
            start_btpcm();
#endif

         
            break;
        case BT_CONN_TEST_EVT:
            if (len == sizeof(evt_conn_attributs) && memcmp(evt_conn_attributs, data, len) == 0) {
                TRACE(0,"Recv conn attrib evt while waiting for test evt");
            } else if (len == sizeof(evt_sniffer) && memcmp(evt_sniffer, data, 4) == 0) {
                // Current state is BT_CONN_SNIFFER
     //           hwtimer_start(timer_id, MS_TO_TICKS(60));
     //           bt_state = BT_CONN_WAIT_SEND_DATA;
                    bt_state = BT_CONN_DONE;
                    hal_cmu_simu_pass();     
            } else if (len == sizeof(evt_sco_conn_req) && memcmp(evt_sco_conn_req, data, len) == 0) {
                // Current state is BT_CONN_SCO_CONN_REQ
                ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_sco_conn_accept, sizeof(cmd_sco_conn_accept));
                ASSERT(ret == 0, "Failed to send sync conn accept cmd: %d", ret);
                bt_state = BT_CONN_SEND_SCO_CONN_ACCEPT;
            } else {
                TRACE(3,"Recv invalid test evt: len=%d data={%08x%08x}",
                    len, data[0]<<24|data[1]<<16|data[2]<<8|data[3], data[4]<<24|data[5]<<16|data[6]<<8|data[7]);
            }
            break;
        case BT_CONN_DATA_ACK:
       //     ASSERT(len == sizeof(ack_data), "Invalid data ack len: %d", len);
       //     ret = memcmp(ack_data, data, 4);
        //    ASSERT(ret == 0, "Recv invalid data ack");
            bt_state = BT_CONN_DONE;
            hal_cmu_simu_pass();
            break;
        case BT_CONN_SCO_CONN_ACCEPT_ACK:
            TRACE(4,"SYNC ACK len=%d {%08x%08x%08x}", len,
                data[0]<<24|data[1]<<16|data[2]<<8|data[3],
                data[4]<<24|data[5]<<16|data[6]<<8|data[7],
                data[8]<<24|data[9]<<16|data[10]<<8|data[11]);
            bt_state = BT_CONN_WAIT_SCO_CONCMP;
            hal_cmu_simu_pass();
            break;
        case BT_CONN_DONE:
            TRACE(1,"Rx after test done: len=%d", len);
            break;
        default:
            ASSERT(false, "Invalid state %d in %s", bt_state, __FUNCTION__);
            break;
    }
}

POSSIBLY_UNUSED
static void handle_rx_done_ble(const unsigned char *data, unsigned int len)
{
    int ret;

    TRACE(1,"Recv rx done evt: %d", bt_state);
    hal_cmu_simu_set_val(bt_state);
    //ignore the dbg trace
    if(*data == 0x04 && *(data+1)==0xff)
    {
        return;
    }
    //ignore the max slot ind
    if(*data == 0x04 && *(data+1)==0x1b)
    {
        return;
    }    
    //ignore acl data rx
    if(*data == 0x02)
        return;

    switch (bt_state) {
        case BT_CONN_HCI_RESET_ACK:
            ASSERT(len == sizeof(ack_hci_reset), "Invalid hci reset ack len: %d", len);
            ret = memcmp(ack_hci_reset, data, len);
            ASSERT(ret == 0, "Recv invalid hci reset ack");
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI, cmd_change_bdaddr_ble, sizeof(cmd_change_bdaddr_ble));
            ASSERT(ret == 0, "Failed to send change ble bdaddr cmd: %d", ret);
            bt_state = BT_CONN_SEND_CHANGE_BDADDR;
            break;

        case BT_CONN_CHANGE_BDADDR_ACK:
            ASSERT(len == sizeof(ack_change_bdaddr_ble), "Invalid ble change bdaddr ack len: %d", len);
            ret = memcmp(ack_change_bdaddr_ble, data, len);
            ASSERT(ret == 0, "Recv invalid ble change bdaddr ack");
#if defined(BLE_CONN_SLAVE)
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI,set_adv_data, sizeof(set_adv_data));
            ASSERT(ret == 0, "Failed to set adv data cmd: %d", ret);
            bt_state = BLE_SET_ADV_DATA_REQ;
            
#elif defined(BLE_CONN_MASTER)
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI,ble_creat_connect, sizeof(ble_creat_connect));
            ASSERT(ret == 0, "Failed to creat ble connect cmd: %d", ret);
            bt_state = BLE_CREATE_CONNECT_REQ;

#endif
            break;

/////////////////slave adv process start////////////////////
        case BLE_SET_ADV_DATA_ACK:
            ASSERT(len == sizeof(ack_set_adv_data), "Invalid ble set adv data ack len: %d", len);
            ret = memcmp(ack_set_adv_data, data, len);
            ASSERT(ret == 0, "Recv invalid ble set adv data ack");

            
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI,set_adv_para, sizeof(set_adv_para));
            ASSERT(ret == 0, "Failed to set adv para cmd: %d", ret);
            bt_state = BLE_SET_ADV_PARA_REQ;
            break;
            
        case BLE_SET_ADV_PARA_ACK:
            ASSERT(len == sizeof(ack_set_adv_para), "Invalid ble set adv para ack len: %d", len);
            ret = memcmp(ack_set_adv_para, data, len);
            ASSERT(ret == 0, "Recv invalid ble set adv para ack");

            
            ret = bt_hci_send(conn_intersys, HAL_INTERSYS_MSG_HCI,set_adv_en, sizeof(set_adv_en));
            ASSERT(ret == 0, "Failed to set adv en cmd: %d", ret);
            bt_state = BLE_SET_ADV_EN_REQ;
            break;

        case BLE_SET_ADV_EN_ACK:
            ASSERT(len == sizeof(ack_set_adv_en), "Invalid ble set adv en ack len: %d", len);
            ret = memcmp(ack_set_adv_en, data, len);
            ASSERT(ret == 0, "Recv invalid ble set adv en ack");
            
            ///wait connect complete evt
            bt_state = BLE_WAIT_CONN_CMP;
            break;
/////////////////slave adv process end////////////////////

/////////////////ble master connect process start////////////////////
        case BLE_CREATE_CONNECT_ACK:
            ASSERT(len == sizeof(ack_ble_creat_connect), "Invalid ble create conenct ack len: %d", len);
            ret = memcmp(ack_ble_creat_connect, data, len);
            ASSERT(ret == 0, "Recv invalid ble create conn ack");
            ///wait connect complete evt
            bt_state = BLE_WAIT_CONN_CMP;
            break;
/////////////////ble master connect process end////////////////////
///////////////////ble connect process start///////////////
        case BLE_WAIT_CONN_CMP:
//            ASSERT(len == sizeof(ack_ble_creat_connect), "Invalid ble create conenct ack len: %d", len);
            ret = memcmp(ble_connect_complete_evt, data, 5);
            ASSERT(ret == 0, "Recv invalid ble create conn complete evt");
            ble_conn_handle = data[5];
            hwtimer_start(timer_id, MS_TO_TICKS(40));
            bt_state = BLE_WAIT_DATA_SEND_TIMER;
            break;

        case BLE_SEND_DATA_ACK:
            bt_state = BT_CONN_DONE;
            hal_cmu_simu_pass();
            break;
///////////////////ble connect process end///////////////


        case BT_CONN_DONE:
            TRACE(1,"Rx after test done: len=%d", len);
            break;
            
        default:
            break;
   }

}

static void handle_tx_done(void)
{
    TRACE(1,"Recv tx done evt: %d", bt_state);

    switch (bt_state) {
        case BT_CONN_SEND_HCI_RESET:
            bt_state = BT_CONN_HCI_RESET_ACK;
            break;
        case BT_CONN_SEND_LINK_POLICY:
            bt_state = BT_CONN_LINK_POLICY_ACK;
            break;
        case BT_CONN_SEND_CHANGE_BDADDR:
            bt_state = BT_CONN_CHANGE_BDADDR_ACK;
            break;
        case BT_CONN_SEND_SET_SCAN_INTERVAL:
            bt_state = BT_CONN_SET_SCAN_INTERVAL_ACK;
            break;
        case BT_CONN_SEND_SCAN:
            bt_state = BT_CONN_SCAN_ACK;
            break;
        case BT_CONN_SEND_ACCEPT:
            bt_state = BT_CONN_ACCEPT_ACK;
            break;
        case BT_CONN_SEND_DATA:
            bt_state = BT_CONN_DATA_ACK;
            break;
        case BT_CONN_SEND_SCO_CONN_ACCEPT:
            bt_state = BT_CONN_SCO_CONN_ACCEPT_ACK;
            break;
        case BT_CONN_CREAT_CONNECT_REQ:
            bt_state = BT_CONN_CREAT_CONNECT_ACK;
            break;
        case BT_CONN_CREAT_SCO_CONNECT_REQ:
            bt_state = BT_CONN_CREAT_SCO_CONNECT_ACK;
            break;
        case BT_CONN_SEND_SNIFF_REQ:
            bt_state = BT_CONN_SEND_SNIFF_ACK;
            break;
        case BLE_SET_ADV_DATA_REQ:
            bt_state = BLE_SET_ADV_DATA_ACK;
            break;
        case BLE_SET_ADV_PARA_REQ:
            bt_state = BLE_SET_ADV_PARA_ACK;
            break;
        case BLE_SET_ADV_EN_REQ:
            bt_state = BLE_SET_ADV_EN_ACK;
            break;            
        case BLE_CREATE_CONNECT_REQ:
            bt_state = BLE_CREATE_CONNECT_ACK;
            break;
        case BLE_SEND_DATA_REQ:
            bt_state = BLE_SEND_DATA_ACK;
            break;
            
        default:
            ASSERT(false, "Invalid state %d in %s", bt_state, __FUNCTION__);
            break;
    }
}

void WEAK hal_psc_bt_enable(void) {}
#ifdef AF_SIMU
#include "hal_aud.h"
void af_test(void);
#endif

void bt_conn_simu(void)
{
    int ret;
    int i;
    uint32_t lock;
    bool proc_timer;
    bool proc_rx;
    bool proc_tx;
#if !defined(BTPCM_TEST) && !defined(DPD_BT_TEST)
    POSSIBLY_UNUSED bool sleep;
#endif

#ifdef CHIP_BEST1000
    // Configure BT clock
    hal_cmu_reset_set(HAL_CMU_MOD_BTCPU);
    hal_cmu_reset_pulse(HAL_CMU_MOD_BT);
    REG(0xD0330038) &= ~(1 << 8);
    hal_cmu_reset_clear(HAL_CMU_MOD_BTCPU);
#else
    hal_psc_bt_enable();
    hal_cmu_bt_clock_enable();
    hal_cmu_bt_reset_clear();
#endif

    hal_sys_timer_delay(MS_TO_TICKS(3));

    // Allow sleep
#ifndef CHIP_SUBSYS_SENS
  //  pmu_sleep_en(1);
#endif
  //  hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

    ret = hal_intersys_open(conn_intersys, HAL_INTERSYS_MSG_HCI, bt_rx, bt_tx, false);
    if (ret) {
        ASSERT(false, "Failed to open intersys");
    }
    hal_intersys_start_recv(conn_intersys);

    cmd_data[0] = 0x02;
    cmd_data[1] = 0x80;
    cmd_data[2] = 0x20;
    cmd_data[3] = 0xfd;
    cmd_data[4] = 0x03;
    for (i = 0; i < 0x03fd; i++) {
        cmd_data[5 + i] = i;
    }
    cmd_data_len = 5 + 0x03fd;
    bt_pkt_init();
    timer_id = hwtimer_alloc(timer_expired, NULL);
    ASSERT(timer_id, "Failed to alloc timer_id");
    hwtimer_start(timer_id, MS_TO_TICKS(15));

    TRACE(0,"Start to wait timer ...");
#ifdef AF_SIMU
   af_test(); 
#endif
    while (1) {
        proc_timer = false;
        proc_rx = false;
        proc_tx = false;
#if !defined(BTPCM_TEST) && !defined(DPD_BT_TEST)
        sleep = false;
#endif

        lock = int_lock();
#if !defined(BTPCM_TEST) && !defined(DPD_BT_TEST)
        if (!(timer_evt || bt_rx_done || bt_tx_done)) {
            sleep = true;
        }
#endif
        if (timer_evt) {
            timer_evt = false;
            proc_timer = true;
            hal_cpu_wake_unlock(BT_TIMER_WAKE_LOCK);
        }
        if (bt_rx_done) {
            bt_rx_done = false;
            proc_rx = true;
            hal_cpu_wake_unlock(BT_RX_WAKE_LOCK);
        }
        if (bt_tx_done) {
            bt_tx_done = false;
            proc_tx = true;
            hal_cpu_wake_unlock(BT_TX_WAKE_LOCK);
        }
        int_unlock(lock);

        if (proc_timer) {
            handle_timer_evt();
        }
        if (proc_rx) {
            if (bt_rx_len > 0) {
#ifdef BLE_CONN_TEST                
                handle_rx_done_ble(bt_rx_data, bt_rx_len);
#else
                handle_rx_done(bt_rx_data, bt_rx_len);
#endif
                bt_rx_len = 0;
            }
            hal_intersys_start_recv(conn_intersys);
        }
        if (proc_tx) {
            handle_tx_done();
        }

#if !defined(BTPCM_TEST) && !defined(DPD_BT_TEST)
        if(sleep) {
            if (hal_trace_busy() == 0) {
                uint32_t time;
                enum HAL_SLEEP_STATUS_T status;

                time = hal_sys_timer_get();
                hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
                status = hal_sleep_enter_sleep();
                hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
                
                if (status == HAL_SLEEP_STATUS_DEEP) {
                    TRACE(1,"Deep sleep %u ms", TICKS_TO_MS(hal_sys_timer_get() - time));
                }
            }
        }
#endif
    }
}

#endif
#endif
