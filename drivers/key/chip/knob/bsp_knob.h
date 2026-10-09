#ifndef __BSP_KNOB_H__
#define __BSP_KNOB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

/*
 * 旋钮解码二选一（驱动内部，公共接口在 bsp_knob.c）：
 *   0 → bsp_knob_legacy.c  旧 A↓/B↑ 半正交
 *   1 → bsp_knob_gray.c    Gray 码双沿
 */
#ifndef IS_SUPPORT_KNOB_GRAY_DECODE
#define IS_SUPPORT_KNOB_GRAY_DECODE                      (1)
#endif
void bsp_knob_init(void);
uint16_t bsp_knob_get_count(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif
