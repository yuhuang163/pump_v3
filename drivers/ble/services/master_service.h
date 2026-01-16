#ifndef __MASTER_SERVICE_H__
#define __MASTER_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h"

#define ROOT_MASTER_SRV_UUID_128 \
        0x01,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x90,0x54,0x4F,0x4F,0x52
#define ROOT_MASTER_SRV_TX_CHAR_UUID_128 \
        0x02,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x90,0x54,0x4F,0x4F,0x52
#define ROOT_MASTER_SRV_RX_CHAR_UUID_128 \
        0x03,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x90,0x54,0x4F,0x4F,0x52


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_master_service_init(void);
bool ble_master_service_notify_status_get(void);
int ble_master_service_notify_data_send(uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif 
