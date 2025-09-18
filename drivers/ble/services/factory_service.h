#ifndef __FACTORY_SERVICE_H__
#define __FACTORY_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_common.h"

#define ROOT_FACTORY_SRV_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x02,0x00,0x01
#define ROOT_FACTORY_SRV_TX_CHAR_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x02,0x00,0x02
#define ROOT_FACTORY_SRV_RX_CHAR_UUID_128 \
        0x52,0x4F,0x4F,0x54,0x90,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x02,0x00,0x03


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void ble_factory_service_init(void);
bool ble_factory_service_notify_status_get(void);
int ble_factory_service_notify_data_send(uint8_t* data, uint32_t len);
#ifdef __cplusplus
}
#endif

#endif 
