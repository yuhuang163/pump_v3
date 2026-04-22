#ifndef __BSP_HYN_H__
#define __BSP_HYN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * 海栎创自容触摸IC驱动 (参考: 海栎创自容IC-按键项目软件应用说明)
 * 适配 BES 平台，实现 bsp_key_internal.h 接口
 ******************************************************************************/

#include <stdint.h> // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep

#define HYN_TOUCH_I2C_ADDR              (0x15)  /// 7位I2C地址
#define HYN_TOUCH_KEY_NUM               (6)     /// 与 SC09B 兼容，映射6个按键

/// 周期读 0x61 差分/信号量并打印 (CST812T/CST820 等订制品参考)；0=关闭
#ifndef HYN_ENABLE_DIFF_SIGNAL_DUMP
#define HYN_ENABLE_DIFF_SIGNAL_DUMP     (0)
#endif
#ifndef HYN_DIFF_SIGNAL_PERIOD_MS
#define HYN_DIFF_SIGNAL_PERIOD_MS       (100)
#endif

#if IS_SUPPORT_TOUCH_IC_OTA
/// 按键开机后触摸芯片固件自动化升级开关
#ifndef BSP_TOUCH_IC_FW_OTA_AUTO
#define BSP_TOUCH_IC_FW_OTA_AUTO        (0)    /// 0: 非压测版本，按正常升级流程 1: 压测版本，忽略版本差异并循环强制升级
#endif

#if (BSP_TOUCH_IC_FW_OTA_AUTO == 1)
/// 自动化压测模式下每轮升级间隔
#ifndef HYN_TOUCH_OTA_STRESS_INTERVAL_MS
#define HYN_TOUCH_OTA_STRESS_INTERVAL_MS (4000)
#endif
#endif

#if (BSP_TOUCH_IC_FW_OTA_AUTO == 0)
/// 为 1 时在 bsp_key_init() 内开机即尝试触摸 OTA（默认关，仅在调试阶段打开）
#ifndef BSP_TOUCH_IC_FW_OTA_AT_BOOT
#define BSP_TOUCH_IC_FW_OTA_AT_BOOT     (0)
#endif
#endif

#define HYN_TOUCH_BOOT_I2C_ADDR         (0x6A)  /// 7位I2C地址
#define HYN_TRUE	(0)
#define HYN_FAULT	(1)

#define PER_LEN	512

#define REG_LEN_1B  1
#define REG_LEN_2B  2
#endif

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void bsp_touch_key_reset(void);
uint8_t touch_fw_update_handler(void);
uint8_t touch_fw_update_handler_force(bool force_update);
bool touch_firmware_is_lower_than_image(int current_version);
int touch_firmware_target_version(void);
void bsp_touch_rst_pin_output_set(uint8_t high_level);
void bsp_touch_key_ota_event_cb(key_ota_event_e event, uint8_t progress);
#ifdef __cplusplus
}
#endif

#endif
