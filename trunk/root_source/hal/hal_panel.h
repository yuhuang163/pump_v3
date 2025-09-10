#ifndef __HAL_PANEL_H__
#define __HAL_PANEL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述面板操作码
*/
typedef enum {
    OP_CODE_PANEL_CLEAR = (1 << 1),
    OP_CODE_PANEL_SET_BL = (1 << 2),
    OP_CODE_PANEL_ADD_READY_CB = (1 << 3),
} panel_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
