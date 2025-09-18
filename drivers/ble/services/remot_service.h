#ifndef __REMOT_SERVICE_H__
#define __REMOT_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h"

#define ROOT_REMOT_SRV_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x01
#define ROOT_REMOT_SRV_TX_CHAR_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x02
#define ROOT_REMOT_SRV_RX_CHAR_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x03


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_remot_service_init(void);
bool ble_remot_service_notify_status_get(void);
int ble_remot_service_notify_data_send(uint8_t* data, uint32_t len);
#ifdef __cplusplus
}
#endif

#endif 
