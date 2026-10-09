/*
 * 旋钮公共接口。解码实现二选一，由 bsp_knob.h 中 IS_SUPPORT_KNOB_GRAY_DECODE 切换：
 *   0 → bsp_knob_legacy.c
 *   1 → bsp_knob_gray.c
 */
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_knob.h" // IWYU pragma: keep
#include "bsp_knob_internal.h" // IWYU pragma: keep

#if IS_SUPPORT_DEVICE_KNOB

void bsp_knob_init(void)
{
#if IS_SUPPORT_KNOB_GRAY_DECODE
    bsp_knob_gray_init();
#else
    bsp_knob_legacy_init();
#endif
}

uint16_t bsp_knob_get_count(uint8_t index)
{
#if IS_SUPPORT_KNOB_GRAY_DECODE
    return bsp_knob_gray_get_count(index);
#else
    return bsp_knob_legacy_get_count(index);
#endif
}

#endif /* IS_SUPPORT_DEVICE_KNOB */
