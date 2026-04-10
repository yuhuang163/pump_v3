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

/// 自适应背光lux 阈值定义
#if 1
#define ADAP_DIM_LUX_MAX                  1500    /// lux 最大值
#define ADAP_DIM_LUX_THR_VDARK            30      /// 非常暗环境阈值 (lux < 30)
#define ADAP_DIM_LUX_THR_DARK             145      /// 较暗环境阈值 (30 <= lux < 145)
#define ADAP_DIM_LUX_THR_MEDIUM           312     /// 中等亮度环境阈值 (145 <= lux < 312)
#define ADAP_DIM_LUX_THR_BRIGHT           839     /// 较亮环境阈值 (312 <= lux < 839)
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
#define ADAP_DIM_LED_LVL_VDARK            2     /// 非常暗环境时的LED亮度 (最高)
#define ADAP_DIM_LED_LVL_DARK_MIN         6      /// 较暗环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_MEDIUM_MIN       21      /// 中等亮度环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_BRIGHT_MIN       60      /// 较亮环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_ON               100       /// 极亮环境时关闭LED (lux >= 1000)
// #define ADAP_DIM_LED_LVL_VBRIGHT          2      /// 很亮环境时的LED亮度 (最低)
#define ADAP_DIM_LED_LVL_OFF              100       /// 极亮环境时关闭LED (lux >= 1000)

/// 滞后阈值定义
#define ADAP_DIM_HYST_THR_MIN             10       /// 滞后阈值最小值 (lux)
#define ADAP_DIM_HYST_THR_RATIO           10       /// 滞后阈值比例 (lux值的百分比，即10%)
#define ADAP_DIM_HYST_THR_BASE            100      /// 滞后阈值计算的基准值 (lux)

/// 与 adaptive_dimming_init 里 add_misc_task(..., interval, ...) 的周期一致
#define ADAP_DIM_MISC_POLL_MS                 200u

#if IS_SUPPORT_ADAPTIVE_DIMMING
/*
 * 以下为「旋钮屏蔽」与「光感明显变化后延时确认」两套独立策略的参数，均按 MISC 任务周期计数，
 * 与 get_timestamp 无关；ADAP_DIM_MISC_POLL_MS 须与 adaptive_dimming_init 中 add_misc_task 的 interval 一致。
 */
/// 旋钮操作后：剩余多少次 MISC 回调内不执行自动调光（约 2000ms）。由 key_input 在 dispatch_key_event_client() 中调用 adaptive_dimming_notify_encoder_activity() 置满。
#define ADAP_DIM_ENCODER_SUPPRESS_TICKS \
    ((2000u + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
/// 光感相对上次已生效亮度变化超过 ADAP_DIM_LUX_SIGNIFICANT_DELTA 后：需在候选附近稳定累计多少次 MISC 周期才允许调光（约 3000ms）。
#define ADAP_DIM_LUX_STABLE_MISC_TICKS \
    ((1000u + ADAP_DIM_MISC_POLL_MS - 1u) / ADAP_DIM_MISC_POLL_MS)
/// 与 s_last_applied_lux 相差超过该值视为「明显变化」，须先经 misc 周期累计与候选一致判稳，再走 duty 计算。
#define ADAP_DIM_LUX_SIGNIFICANT_DELTA        30u
/// 明显变化场景下，当前滤波 lux 与候选 lux 相差不超过该值时认为仍在「同一段稳定区」，用于 misc_tick 累加或 gate 判通过。
#define ADAP_DIM_LUX_SETTLE_MATCH_EPS         30u
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
#if IS_SUPPORT_ADAPTIVE_DIMMING
/// 用户旋转编码器时调用：屏蔽自动调光 ADAP_DIM_ENCODER_SUPPRESS_TICKS 个 MISC 周期，并复位光感稳定状态机。（由按键输入模块在分发至各窗口前统一调用，勿在业务窗口重复调用。）
void adaptive_dimming_notify_encoder_activity(void);
#endif
void light_sensor_sampling_start(void);
void light_sensor_sampling_stop(void);
bool light_sensor_change_sampling_interval(uint16_t interval);
bool light_sensor_data_report(uint8_t *data);
#ifdef __cplusplus
}
#endif

#endif 







