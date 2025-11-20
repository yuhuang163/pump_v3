#ifndef __BSP_FACTORY_UART_H__
#define __BSP_FACTORY_UART_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void bsp_factory_uart_register_event_cb(uart_recv_event_cb_func cb, rt_device_node_t *dev);
int bsp_factory_uart_send_data(uint8_t *buf, uint32_t len);
int bsp_factory_uart_init(void);
void bsp_factory_uart_close(void);

#ifdef __cplusplus
}
#endif

#endif 
