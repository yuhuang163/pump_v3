#ifndef __BSP_KNOB_INTERNAL_H__
#define __BSP_KNOB_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bsp_knob.h"

#if IS_SUPPORT_KNOB_GRAY_DECODE
void bsp_knob_gray_init(void);
uint16_t bsp_knob_gray_get_count(uint8_t index);
#else
void bsp_knob_legacy_init(void);
uint16_t bsp_knob_legacy_get_count(uint8_t index);
#endif

#ifdef __cplusplus
}
#endif

#endif
