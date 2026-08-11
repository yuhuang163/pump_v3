#ifndef __PROJECT_SUPPORT_OVERLAY_H__
#define __PROJECT_SUPPORT_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "product_info.h" // IWYU pragma: keep

/// 模块支持
#define IS_SUPPORT_MODULE_PBUS                           (1)
#define IS_SUPPORT_MODULE_PM                             (1)
#define IS_SUPPORT_MODULE_STORAGE                        (1)
#define IS_SUPPORT_MODULE_TIME_SYSTEM                    (1)
#define IS_SUPPORT_MODULE_WM                             (1)
#define IS_SUPPORT_MODULE_UI                             (1)
#define IS_SUPPORT_MODULE_KEY_INPUT                      (1)
#define IS_SUPPORT_MODULE_SENSOR                         (1)
#define IS_SUPPORT_MODULE_BLE                            (1)
#define IS_SUPPORT_MODULE_OTA                            (1)
#define IS_SUPPORT_MODULE_MISC                           (1)
#define IS_SUPPORT_MODULE_MOTOR                          (1)
#define IS_SUPPORT_MODULE_BREAST_PUMP                    (1)
#define IS_SUPPORT_MODULE_TESTS                          (1)
#define IS_SUPPORT_MODULE_US_ALGO                        (1)
#define IS_SUPPORT_MODULE_AUDIO                          (0)
#define IS_SUPPORT_MODULE_AIRPLANE_MODE                  (1)
#define IS_SUPPORT_MODULE_DATA_BURIAL                    (1)
#define IS_SUPPORT_WEEKLY_STAT                           (0)
#define IS_SUPPORT_MODULE_PROTOCOL                       (1)
#define IS_SUPPORT_MODULE_SERVERS                        (1)
#define IS_SUPPORT_MODULE_BATTERY                        (1)
#define IS_SUPPORT_MODULE_LED                            (1)
#define IS_SUPPORT_MODULE_SYS_ABNORMAL                   (1)

/// 设备支持
#if IS_SUPPORT_MODULE_BATTERY
#define IS_SUPPORT_DEVICE_BATTERY                        (1)
#else
#define IS_SUPPORT_DEVICE_BATTERY                        (0)
#endif
#if IS_SUPPORT_MODULE_LED
#define IS_SUPPORT_DEVICE_LED                            (1)
#else
#define IS_SUPPORT_DEVICE_LED                            (0)
#endif
#define IS_SUPPORT_DEVICE_IRQ                            (1)
#define IS_SUPPORT_DEVICE_FLASH                          (1)
#define IS_SUPPORT_DEVICE_DATABASE                       (1)
#define IS_SUPPORT_DEVICE_INPUT_KEY                      (1)
#define IS_SUPPORT_DEVICE_TOUCH_KEY                      (1)
#define IS_SUPPORT_DEVICE_POWER                          (1)
#define IS_SUPPORT_DEVICE_PMU                            (1)
#define IS_SUPPORT_DEVICE_IMU                            (0)
#define IS_SUPPORT_DEVICE_FSENSOR                        (0)
#define IS_SUPPORT_DEVICE_PRESSURE_KEY                   (0)
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define IS_SUPPORT_DEVICE_AIR_PRESSURE                   (1)
#else
#define IS_SUPPORT_DEVICE_AIR_PRESSURE                   (0)
#endif
#define IS_SUPPORT_DEVICE_MOTOR                          (1)
#define IS_SUPPORT_DEVICE_AUDIO                          (0)
#define IS_SUPPORT_DEVICE_LCD                            (1)
#define IS_SUPPORT_DEVICE_BLE                            (1)
#define IS_SUPPORT_DEVICE_FS                             (0)
#define IS_SUPPORT_DEVICE_WIFI                           (0)

/// 叠图功能
#define IS_SUPPORT_OVERLAY_IMG                           (1)
/// 振动马达震动功能
#define IS_SUPPORT_MOTOR_SHAKE                           (0)
/// 使用在线陀螺仪校准
#define IS_USE_GYRO_ONLINE_CALI                          (0) 
/// 使用IMU FIFO
#define IS_SUPPORT_IMU_FIFO                              (0)
/// 测试支持
#define IS_SUPPORT_PRESSURE_TEST                         (1)
/// 支持爬升策略
#define IS_SUPPORT_CLIMB_POLICY                          (1)
/// 支持暂停/吸奶结束补吸策略
#define IS_SUPPORT_MAKEUP_SUCTION                        (0)
/// 支持长按软复位
#define IS_SUPPORT_KEY_SOFT_RESET                        (1)

/// 支持LVGL
#if CFG_LVGL_ENABLE
#define IS_SUPPORT_LVGL                                  (1)
#else
#define IS_SUPPORT_LVGL                                  (0)
#endif

/// 支持PD充电
#define IS_SUPPORT_PD_CHARGE                             (1)
/// 支持PD充电适配器不匹配
#define IS_SUPPORT_PD_CHARGE_ADAPTER_MISMATCH            (1)
/// 支持旋钮
#define IS_SUPPORT_DEVICE_KNOB                           (1)
/// 支持电池直采
#define IS_SUPPORT_BATTERY_DIRECT_SAMPLING               (0)
/// 支持UART
#define IS_SUPPORT_DEVICE_UART                           (1)
/// 支持光传感器
#define IS_SUPPORT_DEVICE_LIGHT_SENSOR                   (1)
/// 支持光传感器校准
#define IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB             (1)
/// 支持资源管理
#define IS_SUPPORT_MODULE_RES                            (1)
/// 支持自适应背光  
#define IS_SUPPORT_ADAPTIVE_DIMMING                      (1)
/// 设备端电源键 toggle 夜灯（暂关，保留 hook 接口供后续启用）
#ifndef IS_SUPPORT_DEVICE_NIGHT_LIGHT_TOGGLE
#define IS_SUPPORT_DEVICE_NIGHT_LIGHT_TOGGLE             (0)
#endif
/// 支持堵转检测
#define IS_SUPPORT_MOTOR_STALL_DETECT                    (1)
/// 支持睡眠关机
#define IS_SUPPORT_SLEEP_POWER_OFF                       (1)

/// 支持关机提醒
#if IS_SUPPORT_SLEEP_POWER_OFF
#define IS_SUPPORT_POWER_OFF_REMINDER                    (0)
#else
#define IS_SUPPORT_POWER_OFF_REMINDER                    (0)
#endif

/// 支持泵奶时间累计
#define IS_SUPPORT_MODE_WORK_TIME_ACCUMULATE             (1)
/// 支持泵奶步数使用相同设置
#define IS_PUMP_STEPS_USE_SAME_SETTING                   (0)
/// 支持多时间策略组合的韵律中其同类型的时间策略使用同一套设置
#define IS_PUMP_SETTING_SYNC_BY_MODE_TYPE                (1)
/// 同类型时间策略同步范围：0=仅当前模式内同步，1=所有模式中同 mode_type 同步
#define IS_PUMP_SETTING_SYNC_MODE_TYPE_ALL_MODES         (1)
/// 支持泵奶全局参数设置
#define IS_SUPPORT_PUMP_GLOBAL_PARAM_SETTING             (0)
/// 支持泵奶档位实时调节
#define IS_SUPPORT_PUMP_LEVEL_REAL_TIME_ADJUST           (1)
/// 支持泵奶韵律模式选择
#define IS_SUPPORT_PUMP_RHYTHM_MODE_SELECT               (1)

/// 硬件差异配置
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
/// 支持漏气提醒
#define IS_SUPPORT_AIR_LEAKAGE_WARNING                   (0)
/// 支持算法采集
#define IS_SUPPORT_ALGO_SAMPLING                         (1)
/// 支持充电
#define IS_SUPPORT_DEVICE_CHARGE                         (1)
/// 支持泵占空比动态调整
#define IS_SUPPORT_PUMP_DUTY_ADJUST                      (1)
/// 支持插电自动开机
#define IS_SUPPORT_PWR_ON_WHEN_PLUG_IN                   (1)
/// 支持夜灯
#define IS_SUPPORT_NIGHT_LIGHT                           (1)
/// 支持关机充电
#define IS_SUPPORT_POWER_OFF_WHEN_CHARGING               (1)
/// 支持低电量自动结束泵奶
#define IS_SUPPORT_LOW_POWER_AUTO_STOP_PUMP              (1)

/// V3Pro 关闭：插电/短按未满足开机时长不进 dummy 长按开机引导
#ifndef IS_SUPPORT_POWER_ON_GUIDE_UI
#define IS_SUPPORT_POWER_ON_GUIDE_UI                     (0)
#endif

#else
/// 支持漏气提醒
#define IS_SUPPORT_AIR_LEAKAGE_WARNING                   (0)
/// 支持算法采集
#define IS_SUPPORT_ALGO_SAMPLING                         (0)
/// 支持充电
#define IS_SUPPORT_DEVICE_CHARGE                         (0)
/// 支持泵占空比动态调整
#define IS_SUPPORT_PUMP_DUTY_ADJUST                      (0)
/// 支持夜灯
#define IS_SUPPORT_NIGHT_LIGHT                           (0)
/// 支持关机充电
#define IS_SUPPORT_POWER_OFF_WHEN_CHARGING               (0)
/// 支持低电量自动结束泵奶
#define IS_SUPPORT_LOW_POWER_AUTO_STOP_PUMP              (0)
/// V3 开启：插电/短按未满足开机时长时进入 dummy 长按开机引导（关闭则保持原关机逻辑）
#ifndef IS_SUPPORT_POWER_ON_GUIDE_UI
#define IS_SUPPORT_POWER_ON_GUIDE_UI                     (0)
#endif
#endif

/// 支持LOGO界面背景颜色测试
#define IS_SUPPORT_LOGO_UI_TEST                          (0)
/// 支持LOGO界面按键背光自适应测试
#define IS_SUPPORT_LOGO_KEY_LIGHT_ADAPTIVE_TEST          (0)    
/// 开机优化：logo 亮屏后再分步预解码
#ifndef IS_SUPPORT_BOOT_DEFER_UI_SNAP_PREPARE
#define IS_SUPPORT_BOOT_DEFER_UI_SNAP_PREPARE            (1)
#endif
/// 支持引导界面
#define IS_SUPPORT_GUIDE_WINDOW                          (1)    
/// 支持主题选择界面
#define IS_SUPPORT_THEME_GUIDE_WINDOW                    (0)    

/// 支持触摸芯片固件升级
#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V3 || V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
#define IS_SUPPORT_TOUCH_IC_OTA                          (0)
#else
#define IS_SUPPORT_TOUCH_IC_OTA                          (1)
#endif

/// 支持力度调节闪烁
#define IS_SUPPORT_PUMP_LEVEL_ADJUST_BLINK               (1)

/// 支持泵奶策略快速测试
#define IS_SUPPORT_PUMP_MODE_FAST_TEST                   (0)

/// 支持主题切换
#define IS_SUPPORT_THEME_SWITCH                          (0)

/// 使用新的Spectra兼容吸力曲线参数表 // 0 不使用 1 使用
#if (ROOT_UI_VERSION == ROOT_UI_V7)
#ifndef IS_USE_SPECTRA_SUCTION_CURVE
#define IS_USE_SPECTRA_SUCTION_CURVE                     (1)
#endif
#endif

/// 支持旋钮按键触发后的电源键去抖（抑制误触）
#define IS_SUPPORT_KNOB_POWER_KEY_DEBOUNCE               (1)

/// 泵同步方式：同档同步 / 同频同步（二者互斥，优先同频）
#define PUMP_LEVEL_SYNC_ENABLE                           (0)
#define PUMP_FREQ_SYNC_ENABLE                            (1)

#ifdef __cplusplus
}
#endif

#endif


