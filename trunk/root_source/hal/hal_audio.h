#ifndef __HAL_AUDIO_H__
#define __HAL_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/**
 * @brief 描述音频操作码
*/
typedef enum {
    OP_CODE_AUDIO_SET_VOLUME = 1,
    OP_CODE_AUDIO_GET_VOLUE = 2,
    OP_CODE_AUDIO_POWER_AMP_ON = 3,
    OP_CODE_AUDIO_POWER_AMP_OFF = 4,
    OP_CODE_AUDIO_SET_SAMPLE_RATE = 5,
    OP_CODE_AUDIO_IS_READY = 6,
} audio_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
