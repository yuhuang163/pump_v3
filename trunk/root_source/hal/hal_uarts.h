#ifndef __HAL_UARTS_H__
#define __HAL_UARTS_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述UART操作码
*/
typedef enum {
    OP_CODE_UART_ADD_EVENT_CB = 1,
} uart_dev_op_code_e;

typedef void (*uart_recv_event_cb_func)(void *ctx, uint8_t *p_data, uint16_t ulLen);

#ifdef __cplusplus
}
#endif

#endif 
