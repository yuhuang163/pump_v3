#ifndef __ADAPTIVE_DIMMING_H__
#define __ADAPTIVE_DIMMING_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/// 连续 N 次光感上报取平均（抑制抖动）。凑满一批的时长 ≈ N × LIGHT_SENSOR_POLLING_PERIOD
/// （见 light_sensor_module.h，默认 100ms），与 MISC 自适应任务 200ms 无直接乘算关系。
/// 遮挡变暗需较快响应，由 4 改为 2（约 200ms/批），减轻「混合批」造成的两段台阶。
#define ADAP_DIM_DATA_SIZE                      2

/// lux 低通与快速跟踪：大变化时直接跟上环境，小波动用 EMA 平滑
#define ADAP_DIM_LUX_EMA_NUM                    7    /// EMA: filt = (filt*NUM + raw) / DEN
#define ADAP_DIM_LUX_EMA_DEN                    8
#define ADAP_DIM_LUX_FAST_TRACK_DIFF            90  /// |raw - filt| 超过此值则跳过 EMA，直接采用 raw（仅变亮侧）

/// 变暗侧：批均值低于 filt 时直接采用 raw，不走 EMA，避免「先暗→回亮→再暗」
#ifndef ADAP_DIM_DIM_LUX_FAST_PATH
#define ADAP_DIM_DIM_LUX_FAST_PATH              (1)
#endif
/// 屏/键背光 ALS：lux 分段阈值默认值（运行时由工厂 calib[0..3] 覆盖，无效时回退）
#define ADAP_DIM_LUX_MAX                        1500u
#define ADAP_DIM_LUX_THR_VDARK                  30u
#define ADAP_DIM_LUX_THR_DARK                   145u
#define ADAP_DIM_LUX_THR_MEDIUM                 312u
#define ADAP_DIM_LUX_THR_BRIGHT                 839u
/// 工厂光感校准表中用作分段阈值的点数（calib[0..3]，calib[4] 预留）
#define ADAP_DIM_LUX_CALIB_THR_POINTS           4
/// 产测 S2：写完最后一个阈值点（point == 本宏）后再 reload 生效
#define ADAP_DIM_LUX_CALIB_THR_RELOAD_POINT     (ADAP_DIM_LUX_CALIB_THR_POINTS - 1)

/// 屏/键背光 lux→duty knots（adaptive_dimming.c / s_back_lux_duty_knots）
#define ADAP_DIM_LED_LEVEL1_MIN                 26u  /// 对应PWM表5%位置
#define ADAP_DIM_LED_LEVEL1_MAX                 35u  /// 对应PWM表10%位置
#define ADAP_DIM_LED_LEVEL2_MIN                 35u  /// 对应PWM表10%位置
#define ADAP_DIM_LED_LEVEL2_MAX                 48u  /// 对应PWM表20%位置
#define ADAP_DIM_LED_LEVEL3_MIN                 48u  /// 对应PWM表20%位置 //53u  /// 对应PWM表25%位置
#define ADAP_DIM_LED_LEVEL3_MAX                 66u  /// 对应PWM表40%位置
#define ADAP_DIM_LED_LEVEL4_MIN                 66u  /// 对应PWM表40%位置 //69u  /// 对应PWM表45%位置
#define ADAP_DIM_LED_LEVEL4_MAX                 79u  /// 对应PWM表60%位置
#define ADAP_DIM_LED_LEVEL5_MIN                 79u  /// 对应PWM表60%位置
#define ADAP_DIM_LED_LEVEL5_MAX                 89u  /// 对应PWM表80%位置


/// 光感批处理 + 去抖计数（与传感器 2 点批对齐，约 200ms/批）
#define ADAP_DIM_MISC_POLL_MS                   200u

/// slew 任务周期（对齐 misc_manager；屏/夜灯仅 BSP 爬坡）
#ifndef ADAP_DIM_SLEW_POLL_MS
#define ADAP_DIM_SLEW_POLL_MS                   30u
#endif

/*
 * 夜灯禁止 software slew（s_nl_duty_slew 随 target 直写）；按键保留 slew_one。
 * 夜灯 ALS 在 night_light_hook 内直写 BSP，勿叠 MISC 二次写灯。
 */

#if IS_SUPPORT_ADAPTIVE_DIMMING
/** 左旋/右旋后短时屏蔽 ALS（约 2s），并复位去抖状态机。 */
#ifndef ADAP_DIM_USE_ENCODER_KNOB_SHIELD
#define ADAP_DIM_USE_ENCODER_KNOB_SHIELD        (1)
#endif

#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
/// 旋钮操作后：剩余多少次 MISC 回调内不执行自动调光（约 2000ms）。ADAP_DIM_MISC_POLL_MS 须与 add_misc_task 的 interval 一致。
#define ADAP_DIM_ENCODER_SUPPRESS_TICKS \
    ((2000u + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
#endif
/**
 * 手机式：滞回触发一次去抖确认，确认时用当前 lux 定目标；
 * 仅按键 software slew；屏/夜灯 target 直推 hook，由 BSP 单层爬坡（LCD 25ms/级、夜灯 35ms/级）。
 * 置 0 则每批直接更新目标（仍走 gate 时）。
 */
#ifndef ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
#define ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE      (1)
#endif

/// 每个 ADAP_DIM_SLEW_POLL_MS 周期按键向 target 逼近的 duty 步进（屏/夜灯见 BSP 爬坡）
#ifndef ADAP_DIM_DUTY_SLEW_STEP_UP
#define ADAP_DIM_DUTY_SLEW_STEP_UP                  1u
#endif
#ifndef ADAP_DIM_DUTY_SLEW_STEP_DOWN
#define ADAP_DIM_DUTY_SLEW_STEP_DOWN                2u
#endif

#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
#ifndef ADAP_DIM_LUX_DEBOUNCE_MS
#define ADAP_DIM_LUX_DEBOUNCE_MS                    200u
#endif
#define ADAP_DIM_LUX_STABLE_MISC_TICKS \
    ((ADAP_DIM_LUX_DEBOUNCE_MS + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
#define ADAP_DIM_LUX_SETTLE_MATCH_EPS               20u
#endif
#endif



typedef struct {
    uint8_t is_acquire_complete; /// 采集完成标志
    uint8_t index; /// 采集索引
    uint16_t buff[ADAP_DIM_DATA_SIZE];
} adaptive_dimming_capture_data_t;

typedef struct {
    adaptive_dimming_capture_data_t capture_data;
    uint8_t is_sampling;
} adaptive_dimming_info_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void adaptive_dimming_init(void);

/// 从工厂存储重载 calib[0..3] 到运行时阈值并重建 knots（产测写齐后 / 调试热生效）
int adaptive_dimming_lux_thr_reload_from_factory(void);

/// 最近一次 EMA 滤波后的 lux（供夜灯开灯时立即对齐背光目标）
uint16_t adaptive_dimming_get_filtered_lux(void);

#if IS_SUPPORT_NIGHT_LIGHT

/// 夜灯 ALS 曲线 pwm_table[] 索引
#define ADAP_DIM_NL_PWM_IDX_OFF                    0
#define ADAP_DIM_NL_PWM_IDX_L1                     25u
#define ADAP_DIM_NL_PWM_IDX_L2                     45u
#define ADAP_DIM_NL_PWM_IDX_L3                     65u

/// adc/lux → pwm_table[] 索引（s_nl_adc_duty_knots 插值；lux >= 运行时 medium thr 返回 OFF）
uint8_t adaptive_dimming_night_light_duty_from_lux(uint16_t lux_avg);

/// 夜灯 ALS 路径开启（由 led_hook 在 turn_on 时调用，内部处理缓存失效与立即推送）
void adaptive_dimming_on_night_light_open(led_manager_context_t *led);

/// 夜灯关闭（由 led_hook 在 turn_off 时调用）
void adaptive_dimming_on_night_light_close(void);

/// 夜灯已亮时 switch 从关恢复（由 led_hook 在 set_night_light_switch(1) 时调用）
void adaptive_dimming_on_night_light_switch_restored(led_manager_context_t *led);

#endif

#define PANEL_BACKLIGHT_APP_RATE_MIN        1u//1u
#define PANEL_BACKLIGHT_APP_RATE_MAX        100u//80u//100u
#define PANEL_BACKLIGHT_APP_RATE_DEFAULT    100u//80u//100u


#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
/// 旋钮活动时调用：置屏蔽计数；若开启光感延时确认策略则同时复位其状态机。（由本模块注册的左旋/右旋回调触发。）
void adaptive_dimming_notify_encoder_activity(void);
#endif
void light_sensor_sampling_start(void);
void light_sensor_sampling_stop(void);
bool light_sensor_sampling_is_sampling(void);
bool light_sensor_change_sampling_interval(uint16_t interval);
bool light_sensor_data_report(uint8_t *data);
#ifdef __cplusplus
}
#endif

#endif
