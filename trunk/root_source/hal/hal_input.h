#ifndef __HAL_INPUT_H__
#define __HAL_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述输入操作码
*/
typedef enum {
    OP_CODE_INPUT_ADD_CALLBACK = 1,
    OP_CODE_INPUT_IS_TRIGGERED = 2,
    OP_CODE_INPUT_GET_KEY_STATE = 3,
    OP_CODE_INPUT_SET_KEY_SENSITIVITY = 4,
    OP_CODE_INPUT_IS_READY = 5,
    OP_CODE_INPUT_TOUCH_IS_READY = 6,
    OP_CODE_INPUT_TOUCH_SIGNAL_READ = 7,
} input_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
