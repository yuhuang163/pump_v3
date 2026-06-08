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
#define ADAP_DIM_DATA_SIZE                4

/// lux 低通与快速跟踪：大变化时直接跟上环境，小波动用 EMA 平滑
#define ADAP_DIM_LUX_EMA_NUM              7    /// EMA: filt = (filt*NUM + raw) / DEN
#define ADAP_DIM_LUX_EMA_DEN              8
#define ADAP_DIM_LUX_FAST_TRACK_DIFF      180  /// |raw - filt| 超过此值则跳过 EMA，直接采用 raw

/// 自适应背光 lux 阈值默认值（IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB 时由工厂 calib[0..3] 覆盖）
#if 1
#define ADAP_DIM_LUX_MAX                  1500    /// lux 最大值（固定宏，calib[4] 预留未用）
#define ADAP_DIM_LUX_THR_VDARK            30      /// 非常暗环境阈值（默认，工厂 calib[0]）
#define ADAP_DIM_LUX_THR_DARK             145     /// 较暗环境阈值（默认，工厂 calib[1]）
#define ADAP_DIM_LUX_THR_MEDIUM           312     /// 中等亮度环境阈值（默认，工厂 calib[2]）
#define ADAP_DIM_LUX_THR_BRIGHT           839     /// 较亮环境阈值（默认，工厂 calib[3]）
// #define ADAP_DIM_LUX_THR_VBRIGHT          1200    /// 很亮环境阈值 (839 <= lux < 1200)
#else
#define ADAP_DIM_LUX_MAX                  100     /// lux 最大值
#define ADAP_DIM_LUX_THR_VDARK            10      /// 非常暗环境阈值 (lux < 10)
#define ADAP_DIM_LUX_THR_DARK             50      /// 较暗环境阈值 (10 <= lux < 50)
#define ADAP_DIM_LUX_THR_MEDIUM           60     /// 中等亮度环境阈值 (50 <= lux < 200)
#define ADAP_DIM_LUX_THR_BRIGHT           70     /// 较亮环境阈值 (200 <= lux < 500)
#define ADAP_DIM_LUX_THR_VBRIGHT          90     /// 很亮环境阈值 (500 <= lux < 1000)
#endif

/// 自适应亮度等级定义
#define ADAP_DIM_LED_LVL_VDARK            17    /// 非常暗环境时的LED亮度
#define ADAP_DIM_LED_LVL_DARK_MIN         29    /// 较暗环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_MEDIUM_MIN       49    /// 中等亮度环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_BRIGHT_MIN       79    /// 较亮环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_ON               100   /// 极亮环境时LED亮度

/// 滞后阈值定义
#define ADAP_DIM_HYST_THR_MIN             10       /// 滞后阈值最小值 (lux)
#define ADAP_DIM_HYST_THR_RATIO           10       /// 滞后阈值比例 (lux值的百分比，即10%)
#define ADAP_DIM_HYST_THR_BASE            100      /// 滞后阈值计算的基准值 (lux)

/// 与 adaptive_dimming_init 里 add_misc_task(..., interval, ...) 的周期一致（仅 MISC 节拍，不用系统时间）
#define ADAP_DIM_MISC_POLL_MS             200u    /// MISC 节拍周期 (ms)

#if IS_SUPPORT_ADAPTIVE_DIMMING
/**
 * 左旋/右旋时短时屏蔽自动调光（独立开关，默认关）。
 * 与 IS_SUPPORT_ADAPTIVE_DIMMING 解耦：仅控制旋钮屏蔽与 add_keyevent_handler 注册，不改变分段调光/滞环等原有逻辑。
 */
#ifndef ADAP_DIM_USE_ENCODER_KNOB_SHIELD
#define ADAP_DIM_USE_ENCODER_KNOB_SHIELD              (1)
#endif

#if ADAP_DIM_USE_ENCODER_KNOB_SHIELD
/// 旋钮操作后：剩余多少次 MISC 回调内不执行自动调光（约 2000ms）。ADAP_DIM_MISC_POLL_MS 须与 add_misc_task 的 interval 一致。
#define ADAP_DIM_ENCODER_SUPPRESS_TICKS \
    ((2000u + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
#endif
/**
 * 光感「相对上次已下发亮度」出现变化后的延时确认
 * 默认开启；置 0 可关闭，仅保留滞环/EMA 等，不经 settling 门控。
 */
#ifndef ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
#define ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE      (1)
#endif

#if ADAP_DIM_USE_LUX_SIGNIFICANT_SETTLE_GATE
/// 出现变化后：需在候选 lux 附近连续通过多少次 MISC 周期才允许下发（由 polling 内 lux_stable_misc_tick 累加）
#define ADAP_DIM_LUX_STABLE_MISC_TICKS \
    ((1000u + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
/// 与 s_last_applied_lux 相差超过该值视为「明显变化」，进入 settling；否则直接放行
#define ADAP_DIM_LUX_SIGNIFICANT_DELTA        150u
/// settling 内当前 lux 与候选相差不超过该值时认为同一段，用于 misc 节拍累加
#define ADAP_DIM_LUX_SETTLE_MATCH_EPS         40u
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







