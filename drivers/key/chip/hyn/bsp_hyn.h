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

/// 周期读 0x61 差分/信号量并打印 (CST812T/CST820 等订制品参考)；0=关闭
#ifndef HYN_ENABLE_DIFF_SIGNAL_DUMP
#define HYN_ENABLE_DIFF_SIGNAL_DUMP     (0)
#endif
#ifndef HYN_DIFF_SIGNAL_PERIOD_MS
#define HYN_DIFF_SIGNAL_PERIOD_MS       (100)
#endif

#if IS_SUPPORT_TOUCH_IC_OTA
/// 为 1 时在 bsp_key_init() 内开机即尝试触摸 OTA（默认关，仅在调试阶段打开）
#ifndef BSP_TOUCH_IC_FW_OTA_AT_BOOT
#define BSP_TOUCH_IC_FW_OTA_AT_BOOT     (0)
#endif

#define HYN_TOUCH_BOOT_I2C_ADDR         (0x6A)  /// 7位I2C地址
#define HYN_TRUE	(0)
#define HYN_FAULT	(1)

#define PER_LEN	512

#define REG_LEN_1B  1
#define REG_LEN_2B  2

uint8_t touch_fw_update(void);

/// 显式执行触摸固件升级（封装 touch_fw_update，返回 0 成功，非 0 失败）；未开启 IS_SUPPORT_TOUCH_IC_OTA 时恒返回 -1
int touch_ic_fw_update_execute(void);

/// 读取触摸 IC 固件版本号（应用模式寄存器 0xFF），与已验证接口一致
uint8_t get_touch_ic_fw_current_version(void);

/// 嵌入固件（hyn_firmware.h / app_bin）对应的目标版本号；勿在其它 .c 中直接包含 hyn_firmware.h，避免 app_bin 重复定义
uint8_t get_touch_ic_fw_target_version(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
