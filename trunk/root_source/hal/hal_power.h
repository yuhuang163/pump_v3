#ifndef __HAL_POWER_H__
#define __HAL_POWER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

typedef enum {
    OP_CODE_POWER_GO_CHIP_MODE = 1,
    OP_CODE_PCBA_TEST_LEVEL_GET = 2,
} power_dev_op_code_e;

#ifdef __cplusplus
}
#endif

#endif 
