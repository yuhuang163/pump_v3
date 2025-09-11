#ifndef __APP_ALGO_H__
#define __APP_ALGO_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

// 泵奶结果
typedef struct __algo_result_detail {
    uint8_t coverage;                                         // 总体/平均覆盖率
} algo_result_detail_t;

// 泵奶数据
typedef struct {
    uint8_t mode;
    uint16_t level;
    uint8_t report_flag;                // 刷牙报告是否实时上传
    uint32_t timestamp;                 // 开始刷牙时间
    uint32_t setting_time;              // app设置的单次刷牙时间
    uint32_t target_valid_time;         // 目标有效刷牙时间
    uint32_t target_total_time;         // 目标最大工作时长 电子压感存在时使用
    uint32_t work_time_total;           // 刷牙总时长
    uint32_t work_valid_time;           // 有效刷牙时长
    algo_result_detail_t detail_result; // 跟算法的algo_other_result_t结构对齐转换.
} algo_breath_pump_data_t;

#ifdef __cplusplus
}
#endif

#endif 
