#ifndef __BSP_HYN_H__
#define __BSP_HYN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * 海栎创自容触摸IC驱动 (参考: 海栎创自容IC-按键项目软件应用说明)
 * 适配 BES 平台，实现 bsp_key_internal.h 接口
 ******************************************************************************/

#include <stdint.h>

#define HYN_TOUCH_I2C_ADDR              (0x15)  /// 7位I2C地址
#define HYN_TOUCH_KEY_NUM               (6)     /// 与 SC09B 兼容，映射6个按键

#ifdef __cplusplus
}
#endif

#endif
