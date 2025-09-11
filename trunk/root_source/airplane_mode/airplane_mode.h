#ifndef __AIRPLANE_MODE_H__
#define __AIRPLANE_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep

typedef enum {
    AIRPLANE_DETECT_STATE_INIT,
    AIRPLANE_DETECT_STATE_FIRST_TIME_IRQ, /// 初次唤醒
    AIRPLANE_DETECT_STATE_CONTINUOUS_MOVING, /// 连续晃动
} airplane_detect_state_e;

#ifdef __cplusplus
}
#endif

#endif 


