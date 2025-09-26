#ifndef __HAL_LED_H__
#define __HAL_LED_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述LED操作码
*/
typedef enum {
    OP_CODE_LED_REFRESH = 1,
} led_dev_op_code_e;

/**
 * @brief 描述RGB LED ID
*/
typedef enum {
    RGB_LED_ID_0 = 0,
    RGB_LED_ID_1,
    RGB_LED_ID_2,
    RGB_LED_ID_3,
    RGB_LED_ID_4,

    RGB_ALL_LED_ID = 0xFF,
} rgb_led_id_e;

#ifdef __cplusplus
}
#endif

#endif 
