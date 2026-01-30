#ifndef __ADAPTIVE_DIMMING_H__
#define __ADAPTIVE_DIMMING_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#define ADAP_DIM_DATA_SIZE                1  /// 自适应背光数据大小

/// 自适应背光lux 阈值定义
#if 0
#define ADAP_DIM_LUX_MAX                  1500    /// lux 最大值
#define ADAP_DIM_LUX_THR_VDARK            10      /// 非常暗环境阈值 (lux < 10)
#define ADAP_DIM_LUX_THR_DARK             50      /// 较暗环境阈值 (10 <= lux < 50)
#define ADAP_DIM_LUX_THR_MEDIUM           200     /// 中等亮度环境阈值 (50 <= lux < 200)
#define ADAP_DIM_LUX_THR_BRIGHT           500     /// 较亮环境阈值 (200 <= lux < 500)
#define ADAP_DIM_LUX_THR_VBRIGHT          1000    /// 很亮环境阈值 (500 <= lux < 1000)
#else
#define ADAP_DIM_LUX_MAX                  100     /// lux 最大值
#define ADAP_DIM_LUX_THR_VDARK            10      /// 非常暗环境阈值 (lux < 10)
#define ADAP_DIM_LUX_THR_DARK             50      /// 较暗环境阈值 (10 <= lux < 50)
#define ADAP_DIM_LUX_THR_MEDIUM           60     /// 中等亮度环境阈值 (50 <= lux < 200)
#define ADAP_DIM_LUX_THR_BRIGHT           70     /// 较亮环境阈值 (200 <= lux < 500)
#define ADAP_DIM_LUX_THR_VBRIGHT          90     /// 很亮环境阈值 (500 <= lux < 1000)
#endif

/// 自适应亮度等级定义
#define ADAP_DIM_LED_LVL_VDARK            100     /// 非常暗环境时的LED亮度 (最高)
#define ADAP_DIM_LED_LVL_DARK_MIN         80      /// 较暗环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_MEDIUM_MIN       50      /// 中等亮度环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_BRIGHT_MIN       20      /// 较亮环境时的最小LED亮度
#define ADAP_DIM_LED_LVL_VBRIGHT          10      /// 很亮环境时的LED亮度 (最低)
#define ADAP_DIM_LED_LVL_OFF              0       /// 极亮环境时关闭LED (lux >= 1000)

/// 滞后阈值定义
#define ADAP_DIM_HYST_THR_MIN             10       /// 滞后阈值最小值 (lux)
#define ADAP_DIM_HYST_THR_RATIO           10       /// 滞后阈值比例 (lux值的百分比，即10%)
#define ADAP_DIM_HYST_THR_BASE            100      /// 滞后阈值计算的基准值 (lux)

typedef struct {
    uint8_t is_acquire_complete; /// 采集完成标志
    uint8_t index; /// 采集索引
    uint16_t buff[ADAP_DIM_DATA_SIZE];
} adaptive_dimming_capture_data_t;

typedef struct {
    adaptive_dimming_capture_data_t capture_data;
} adaptive_dimming_info_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void adaptive_dimming_init(void);

#ifdef __cplusplus
}
#endif

#endif 







